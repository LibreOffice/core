
#ifndef __FRAMEWORK_RECORDING_DISPATCHRECORDER_HXX_
#define __FRAMEWORK_RECORDING_DISPATCHRECORDER_HXX_

// own includes
#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRECORDER_HPP_
#include <com/sun/star/frame/XDispatchRecorder.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.HPP>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HDL_
#include <com/sun/star/uno/RuntimeException.hdl>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include <vector>

namespace framework{

struct DispatchStatement;
typedef ::std::vector < DispatchStatement > DispatchStatementList;

class DispatchRecorder
    : private ThreadHelpBase
    , public  css::lang::XTypeProvider
    , public  css::lang::XServiceInfo
    , public  css::frame::XDispatchRecorder
    , public  ::cppu::OWeakObject
{
    // private member
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR        ;
        DispatchStatementList                                  m_aStatements;
        sal_Int32                                              m_nRecordingID ;

    // public interface
    public:
        DispatchRecorder( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
        ~DispatchRecorder();

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XDispatchRecorder
        virtual void SAL_CALL            startRecording         ( const css::uno::Reference< css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL            recordDispatch         ( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL            recordDispatchAsComment( const css::util::URL& aURL, const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException );
        virtual void SAL_CALL            endRecording           () throw( css::uno::RuntimeException );
        virtual ::rtl::OUString SAL_CALL getRecordedMacro       () throw( css::uno::RuntimeException );

    // private functions
    private:
        void SAL_CALL implts_recordMacro( const ::rtl::OUString& aURL,
                                          const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                                sal_Bool bAsComment, ::rtl::OUStringBuffer& );
}; // class DispatcRecorder

} // namespace framework

#endif // define __FRAMEWORK...
