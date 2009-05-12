#ifndef __FRAMEWORK_CLASSES_EVENTSCONFIGURATION_HXX_
#define __FRAMEWORK_CLASSES_EVENTSCONFIGURATION_HXX_

#include <svtools/svarray.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// #110897#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

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
