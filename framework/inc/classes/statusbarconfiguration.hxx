#ifndef __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif

// #110897#
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace framework
{

struct StatusBarItemDescriptor
{
    String  aURL;                       // URL command to dispatch
    long    nItemBits;                  // properties for this statusbar item (WinBits)
    long    nWidth;                     // width of a statusbar item
    long    nOffset;                    // offset

    public:

        StatusBarItemDescriptor() : nItemBits( SIB_CENTER | SIB_IN )
                                    ,nWidth( 0 )
                                    ,nOffset( STATUSBAR_OFFSET ) {}
};

typedef StatusBarItemDescriptor* StatusBarItemDescriptorPtr;
SV_DECL_PTRARR_DEL( StatusBarDescriptor, StatusBarItemDescriptorPtr, 10, 2)

class StatusBarConfiguration
{
    public:
        // #110897#
        static sal_Bool LoadStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rInStream, StatusBarDescriptor& aItems );

        // #110897#
        static sal_Bool StoreStatusBar(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rOutStream, const StatusBarDescriptor& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_STATUSBARCONFIGURATION_HXX_
