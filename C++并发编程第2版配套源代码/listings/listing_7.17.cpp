#include <memory>
#include <atomic>
template<typename T>
class lock_free_queue
{
private:
#include "node.hpp"
    struct counted_node_ptr
    {
        int external_count;
        node* ptr;
    };
    
    std::atomic<counted_node_ptr> head;
    std::atomic<counted_node_ptr> tail;
public:
    std::unique_ptr<T> pop()
    {
        counted_node_ptr old_head=head.load(std::memory_order_relaxed);
        for(;;)
        {
            increase_external_count(head,old_head);
            node* const ptr=old_head.ptr;
            if(ptr==tail.load().ptr)
            {
                ptr->release_ref();
                return std::unique_ptr<T>();
            }
            if(head.compare_exchange_strong(old_head,ptr->next))
            {
                T* const res=ptr->data.exchange(nullptr);
                free_external_counter(old_head);
                return std::unique_ptr<T>(res);
            }
            ptr->release_ref();
        }
    }
};
