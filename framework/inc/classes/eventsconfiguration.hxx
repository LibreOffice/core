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
        static sal_Bool LoadEventsConfig( SvStream& rInStream, EventsConfig& aItems );
        static sal_Bool StoreEventsConfig( SvStream& rOutStream, const EventsConfig& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_CLASSES_EVENTSCONFIGURATION_HXX_
