
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <services.h>
#include <uielement/panelwrapper.hxx>
#include <threadhelp/resetableguard.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/panelwindow.hxx>
#include <services/modelwinservice.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/awt/XMenuBar.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <toolkit/unohlp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::awt;
using namespace ::com::sun::star::ui;

namespace framework
{

PanelWrapper::PanelWrapper( const Reference< XMultiServiceFactory >& xServiceManager ) :
    UIElementWrapperBase( UIElementType::DOCKINGWINDOW ),
    m_xServiceManager( xServiceManager ),
    m_bNoClose(false)
{
}

PanelWrapper::~PanelWrapper()
{
}

// XInterface
void SAL_CALL PanelWrapper::acquire() throw()
{
    UIElementWrapperBase::acquire();
}

void SAL_CALL PanelWrapper::release() throw()
{
    UIElementWrapperBase::release();
}

uno::Any SAL_CALL PanelWrapper::queryInterface( const uno::Type & rType )
throw( ::com::sun::star::uno::RuntimeException )
{
    return UIElementWrapperBase::queryInterface( rType );
}

// XComponent
void SAL_CALL PanelWrapper::dispose() throw ( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );
    Reference< XMultiServiceFactory > xSMGR( m_xServiceManager );
    Reference< XWindow > xWindow;

    {
        ResetableGuard aLock( m_aLock );
        if ( m_bDisposed )
            return;
        xSMGR = m_xServiceManager;
    }

    com::sun::star::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    rtl::OUString aModelWinService( SERVICENAME_MODELWINSERVICE );
    Reference< XNameAccess > xNameAccess( xSMGR->createInstance( aModelWinService ), UNO_QUERY );
    if ( xNameAccess.is() )
    {
        ModelWinService* pService = dynamic_cast< ModelWinService* >( xNameAccess.get() );
        if ( pService != 0 )
        {
            vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
            PanelWindow* pPanelWindow = dynamic_cast< PanelWindow* >( m_xPanelWindow.get() );
            if ( pPanelWindow != NULL )
            {
                xWindow = VCLUnoHelper::GetInterface( pPanelWindow->getContentWindow() );
                pService->deregisterModelForXWindow( xWindow );
            }
        }
    }

    ResetableGuard aLock( m_aLock );
    m_xPanelWindow.clear();
    m_bDisposed = sal_True;
}

// XInitialization
void SAL_CALL PanelWrapper::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_bInitialized )
    {
        UIElementWrapperBase::initialize( aArguments );

        sal_Bool bPopupMode( sal_False );
        Reference< XWindow > xContentWindow;
        for ( sal_Int32 i = 0; i < aArguments.getLength(); i++ )
        {
            PropertyValue aPropValue;
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAsciiL( "PopupMode", 9 ))
                    aPropValue.Value >>= bPopupMode;
                else if ( aPropValue.Name.equalsAsciiL( "ContentWindow", 13 ))
                    aPropValue.Value >>= xContentWindow;
            }
        }

        Reference< XFrame > xFrame( m_xWeakFrame );
        if ( xFrame.is() )
        {
            PanelWindow* pPanelWindow(0);
            Window*      pContentWindow(0);
            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
                pContentWindow  = VCLUnoHelper::GetWindow( xContentWindow );
                if ( pWindow )
                {
                    sal_uInt32 nStyles = WB_LINESPACING | WB_BORDER | WB_SCROLL | WB_MOVEABLE | WB_3DLOOK | WB_DOCKABLE | WB_SIZEABLE | WB_CLOSEABLE;

                    pPanelWindow = new PanelWindow( pWindow, nStyles );
                    m_xPanelWindow = VCLUnoHelper::GetInterface( pPanelWindow );
                    pPanelWindow->setResourceURL( m_aResourceURL );
                    pPanelWindow->setContentWindow( pContentWindow );
                }
            }

            try
            {
            }
            catch ( NoSuchElementException& )
            {
            }
        }
    }
}

// XEventListener
void SAL_CALL PanelWrapper::disposing( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
{
    // nothing todo
}

// XUpdatable
void SAL_CALL PanelWrapper::update() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();
}

// XUIElement interface
Reference< XInterface > SAL_CALL PanelWrapper::getRealInterface(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    return m_xPanelWindow;
}

void SAL_CALL PanelWrapper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const com::sun::star::uno::Any&  aValue ) throw( com::sun::star::uno::Exception )
{
    ResetableGuard aLock( m_aLock );
    sal_Bool bNoClose( m_bNoClose );
    aLock.unlock();

    UIElementWrapperBase::setFastPropertyValue_NoBroadcast( nHandle, aValue );

    aLock.lock();

    sal_Bool bNewNoClose( m_bNoClose );
    if ( m_xPanelWindow.is() && !m_bDisposed && ( bNewNoClose != bNoClose ))
    {
        PanelWindow* pPanelWindow = dynamic_cast< PanelWindow* >( VCLUnoHelper::GetWindow( m_xPanelWindow ) );
        if ( pPanelWindow )
        {
            if ( bNewNoClose )
            {
                pPanelWindow->SetStyle( pPanelWindow->GetStyle() & ~WB_CLOSEABLE );
                pPanelWindow->SetFloatStyle( pPanelWindow->GetFloatStyle() & ~WB_CLOSEABLE );
            }
            else
            {
                pPanelWindow->SetStyle( pPanelWindow->GetStyle() | WB_CLOSEABLE );
                pPanelWindow->SetFloatStyle( pPanelWindow->GetFloatStyle() | WB_CLOSEABLE );
            }
        }
    }
}

} // namespace framework
