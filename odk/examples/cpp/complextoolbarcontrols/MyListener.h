#ifndef _MyListener_HXX
#define _MyListener_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#define MYLISTENER_IMPLEMENTATIONNAME  "vnd.My.impl.NewDocListener"
#define MYLISTENER_SERVICENAME         "vnd.My.NewDocListener"

namespace css = ::com::sun::star;

/*---------------------------------------------------
 * Registriert sich in der Office Konfiguration als Job.
 * Dieser wird dann für alle neu geöffneten Dokumente automatisch
 * gerufen. Man bekommt eine Reference auf das geöffnete Dokument
 * überreicht und kann dann prüfen, ob es ein unterstütztes Format
 * hat. (Wir interessieren uns ja schließlich nur für Writer/Calc Dokumente.)
 *
 * @see CalcListener
 * @see WriterListener
 */
class MyListener : public cppu::WeakImplHelper2< css::task::XJob         ,
                                                      css::lang::XServiceInfo >
{
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

    public:
                 MyListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~MyListener();

    // XJob
    virtual css::uno::Any SAL_CALL execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
        throw (css::lang::IllegalArgumentException,
               css::uno::Exception,
               css::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& sServiceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException);

    public:

    static css::uno::Reference< css::uno::XInterface > st_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

class CalcListener : public cppu::WeakImplHelper1< css::document::XEventListener >
{
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;


    public:
        CalcListener(const css::uno::Reference< css::lang::XMultiServiceFactory > &rxMSF)
            : mxMSF( rxMSF )
        {}

        virtual ~CalcListener()
        {}

    // document.XEventListener
    virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
        throw (css::uno::RuntimeException);
};

class WriterListener : public cppu::WeakImplHelper1< css::document::XEventListener >
{
    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;

    public:
        WriterListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMSF);

        virtual ~WriterListener()
        {}

        // document.XEventListener
    virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
        throw (css::uno::RuntimeException);
};

#endif // _MyListener_HXX
