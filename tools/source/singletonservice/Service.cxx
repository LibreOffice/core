#include <svx/svdobj.hxx>
#include <singletonservice/Service.hxx>
namespace tools
{
namespace silverdev
{
S::S()
    : zoomings()
    , notificationCenter()
    , isFirstZoom(true)
{
}
SAL_DLLPUBLIC_EXPORT S& getInstance2()
{
    static S instance;
    return instance;
}
SAL_DLLPUBLIC_EXPORT bool S::hasZooms() { return !this->zoomings.empty(); }
}
}