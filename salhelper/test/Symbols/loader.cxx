

/*
This small test program tests whether all necessary symbols are exported from
isalhelper.lib. If this linker runs without errors than it is ok.

(public)
static class salhelper::ORealDynamicLoader * __cdecl salhelper::ORealDynamicLoader::newInstance(class salhelper::ORealDynamicLoader * *,class rtl::OUString const &,class rtl::OUString const &)

unsigned long __cdecl salhelper::ORealDynamicLoader::acquire(void)

unsigned long __cdecl salhelper::ORealDynamicLoader::release(void)

void * __cdecl salhelper::ORealDynamicLoader::getApi(void)const

(protected)
salhelper::ORealDynamicLoader::ORealDynamicLoader( ORealDynamicLoader ** ppSetToZeroInDestructor,
                        const ::rtl::OUString& strModuleName,
                        const ::rtl::OUString& strInitFunction,
                        void* pApi,
                        oslModule pModule );


virtual salhelper::ORealDynamicLoader::~ORealDynamicLoader();

*/

#include  <salhelper/dynload.hxx>
#include <rtl/ustring>

using namespace salhelper;
using namespace rtl;

class Loader:public ORealDynamicLoader
{
public:
    Loader(){}

    void deletethis()
    {
        delete this;
    }
};

int main( int argc, char *argv[ ], char *envp[ ] )
{
    ORealDynamicLoader* pLoader= ORealDynamicLoader::newInstance( NULL, OUString(), OUString());

    pLoader->acquire();

    pLoader->getApi();
    pLoader->release();

    Loader* pLoader2= new Loader();
    pLoader2->deletethis();

    return 0;
}
