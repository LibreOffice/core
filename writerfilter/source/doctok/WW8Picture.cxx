#include <resources.hxx>
#include <WW8DocumentImpl.hxx>

namespace doctok
{

doctok::Reference<Properties>::Pointer_t
WW8PICF::get_DffRecord()
{
    return doctok::Reference<Properties>::Pointer_t
        (new DffBlock(this, get_cbHeader(), getCount() - get_cbHeader(), 0));
}

doctok::Reference<Properties>::Pointer_t
WW8FSPA::get_shape()
{
    return getDocument()->getShape(get_spid());
}
}

