#include <stdio.h>

#include <cppuhelper/weak.hxx>

class MyOWeakObject : public ::cppu::OWeakObject
{
public:
    MyOWeakObject() { fprintf( stdout, "constructed\n" ); }
    ~MyOWeakObject() { fprintf( stdout, "destructed\n" ); }
};


void simple_object_creation_and_destruction()
{
    // create the UNO object
    com::sun::star::uno::XInterface * p = new MyOWeakObject();

    // acquire it, refcount becomes one
    p->acquire();

    fprintf( stdout, "before release\n" );

    // release it, refcount drops to zero
    p->release();

    fprintf( stdout, "after release\n" );
}


int main( char * argv[] )
{
    simple_object_creation_and_destruction();
    return 0;
}
