#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef INCLUDED_WW8_DOCUMENT_HXX
#include <doctok/WW8Document.hxx>
#endif

namespace doctok
{
using namespace ::com::sun::star;

class WW8StreamImpl : public WW8Stream
{
    uno::Reference<uno::XComponentContext> mrComponentContext;
    uno::Reference<io::XInputStream> mrStream;
    uno::Reference<container::XNameContainer> xOLESimpleStorage;
    uno::Reference<lang::XMultiComponentFactory> xFactory;

public:
    WW8StreamImpl(uno::Reference<uno::XComponentContext> rContext,
                  uno::Reference<io::XInputStream> rStream);
    virtual ~WW8StreamImpl() {}

    virtual WW8Stream::Pointer_t getSubStream(const ::rtl::OUString & rId);

    virtual Sequence get(sal_uInt32 nOffset, sal_uInt32 nCount)
        const;

    //virtual bool put(sal_uInt32 nOffset, const Sequence & rSeq);

    virtual string getSubStreamNames() const;
    virtual uno::Sequence<rtl::OUString> getSubStreamUNames() const;
};
}
