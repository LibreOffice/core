
#include <osl/mutex.hxx>
#include "ole2uno.hxx"

using namespace osl;
namespace ole_adapter
{

Mutex* getBridgeMutex()
{
    static Mutex* pMutex= NULL;

    if( ! pMutex)
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pMutex)
        {
            static Mutex aMutex;
            pMutex= &aMutex;
        }
    }
    return pMutex;
}

}
