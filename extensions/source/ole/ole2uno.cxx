#include "osl/getglobalmutex.hxx"
#include "rtl/instance.hxx"
#include "ole2uno.hxx"

using namespace osl;
namespace ole_adapter
{

struct MutexInit
{
    Mutex * operator () ()
    {
        static Mutex aInstance;
        return &aInstance;
    }
};


Mutex * getBridgeMutex()
{
    return rtl_Instance< Mutex, MutexInit, ::osl::MutexGuard,
        ::osl::GetGlobalMutex >::create(
            MutexInit(), ::osl::GetGlobalMutex());
}


// Mutex* getBridgeMutex()
// {
//  static Mutex* pMutex= NULL;

//  if( ! pMutex)
//  {
//      MutexGuard guard( Mutex::getGlobalMutex() );
//      if( !pMutex)
//      {
//          static Mutex aMutex;
//          pMutex= &aMutex;
//      }
//  }
//  return pMutex;
// }

}
