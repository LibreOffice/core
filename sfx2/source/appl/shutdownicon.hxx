
#ifndef __SHUTDOWNICON_HXX__
#define __SHUTDOWNICON_HXX__

#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _RTL_STRING_HXX
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

class ResMgr;

typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::frame::XTerminateListener, ::com::sun::star::lang::XServiceInfo > ShutdownIconServiceBase;

class ShutdownIcon :    public ShutdownIconServiceBase
{
        ::osl::Mutex    m_aMutex;
        bool            m_bVeto;
        ResMgr          *m_pResMgr;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;

        static ShutdownIcon *pShutdownIcon; // one instance

#ifdef WNT
        void initSystray();
        void deInitSystray();
        static void SetAutostartW32( const ::rtl::OUString& aShortcutName, bool bActivate );
        static bool GetAutostartW32( const ::rtl::OUString& aShortcutName );
#endif

    public:
        ShutdownIcon( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > aSMgr );

        virtual ~ShutdownIcon();

        SFX_DECL_XSERVICEINFO


        static ShutdownIcon* getInstance();
        static void terminateDesktop();
        static void addTerminateListener();

        static void FileOpen();
        static void OpenURL( ::rtl::OUString& aURL );
        static void FromTemplate();

        static void SetAutostart( bool bActivate );
        static bool GetAutostart();

        static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
                    GetWrapperFactory( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSMgr );
        static ::rtl::OUString  GetImplementationName_static();

        ::rtl::OUString GetResString( int id );
        ::rtl::OUString GetUrlDescription( const ::rtl::OUString& aUrl );

        void SetVeto( bool bVeto )  { m_bVeto = bVeto;}
        bool GetVeto()              { return m_bVeto; }

        // Component Helper - force override
        virtual void SAL_CALL disposing();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
            throw(::com::sun::star::uno::RuntimeException);

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
            throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
            throw(::com::sun::star::uno::RuntimeException);

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop > m_xDesktop;
};

#endif
