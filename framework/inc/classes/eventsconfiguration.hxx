#ifndef __FRAMEWORK_CLASSES_EVENTSCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_EVENTSCONFIGURATION_HXX_

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

// #110897#
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

namespace framework
{

struct EventsConfig
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >              aEventNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >   aEventsProperties;
};

class EventsConfiguration
{
    public:

        // #110897#
        static sal_Bool LoadEventsConfig(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rInStream, EventsConfig& aItems );

        // #110897#
        static sal_Bool StoreEventsConfig(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rOutStream, const EventsConfig& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_EVENTSCONFIGURATION_HXX_
