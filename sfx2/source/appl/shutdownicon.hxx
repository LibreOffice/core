
#ifndef __SHUTDOWNICON_HXX__
#define __SHUTDOWNICON_HXX__

#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HDL_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _RTL_STRING_HXX
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

class ResMgr;

class ShutdownIcon :    public ::com::sun::star::frame::XTerminateListener,
                        public ::cppu::OWeakObject
{
        ::osl::Mutex    m_aMutex;
        bool            m_bVeto;
        ResMgr          *m_pResMgr;

        static ShutdownIcon *pShutdownIcon; // one instance
        ShutdownIcon( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop >& aDesktop,
                      ResMgr *aResMgr );

#ifdef WNT
        void initSystray();
        void deInitSystray();
        static void SetAutostartW32( bool bActivate );
        static bool GetAutostartW32( );
#endif

    public:
        virtual ~ShutdownIcon();

        static void create( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop >& aDesktop,
                            ResMgr *aResMgr );
        static ShutdownIcon* getInstance();

        static void destroy();
        static void terminateDesktop();

        static void FileOpen();
        static void OpenURL( ::rtl::OUString& aURL );

        static void SetAutostart( bool bActivate );
        static bool GetAutostart();

        ::rtl::OUString GetResString( int id );
        ::rtl::OUString GetUrlDescription( const ::rtl::OUString& aUrl );

        void SetVeto( bool bVeto )  { m_bVeto = bVeto;}
        bool GetVeto()              { return m_bVeto; }

        // XInterface
        virtual void SAL_CALL acquire()
            throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL release()
            throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
            throw( ::com::sun::star::uno::RuntimeException );

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
