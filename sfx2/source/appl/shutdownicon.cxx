
#include <shutdownicon.hxx>

#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

ShutdownIcon* ShutdownIcon::pShutdownIcon = NULL;

// determines if vetoexception is thrown
// default is NOT to throw an exception, ie,  office terminates normally
static bool bTerminationVeto = false;


// Windows message handling stuff
#ifdef WNT
#include <windows.h>
#define LISTENER_WINDOWCLASS    "SO Listener Class"
#define LISTENER_WINDOWNAME     "SO Listener Window"
#define TERMINATIONVETO_MESSAGE "SO TerminationVeto"
#define TERMINATE_MESSAGE       "SO Terminate"

static HWND aListenerWindow  = NULL;
static UINT aTerminationVetoMessage = 0x7FFF;
static UINT aTerminateMessage = 0x7FFF;

LRESULT CALLBACK listenerWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if( uMsg == aTerminationVetoMessage )
    {
        bTerminationVeto = wParam ? true : false;
    }
    else if( uMsg == aTerminateMessage )
    {
        ShutdownIcon::terminateDesktop();
    }
    else
        switch (uMsg)
        {
            case WM_DESTROY:
                return 0;
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    return 0;
}
#endif

ShutdownIcon::ShutdownIcon( Reference< XDesktop >& aDesktop ) :
    m_xDesktop( aDesktop )
{
#ifdef WNT
    // register windowclass and create window
    //  for communication with systray icon

    WNDCLASS listenerClass;

    listenerClass.style         = 0; //CS_HREDRAW|CS_VREDRAW;
    listenerClass.lpfnWndProc   = listenerWndProc;
    listenerClass.cbClsExtra    = 0;
    listenerClass.cbWndExtra    = 0;
    listenerClass.hInstance     = (HINSTANCE) GetModuleHandle( NULL ); //myInstance;
    listenerClass.hIcon         = NULL;
    listenerClass.hCursor       = NULL;
    listenerClass.hbrBackground = NULL;
    listenerClass.lpszMenuName  = NULL;
    listenerClass.lpszClassName = LISTENER_WINDOWCLASS;

    RegisterClass(&listenerClass);

    aTerminationVetoMessage = RegisterWindowMessage( TERMINATIONVETO_MESSAGE );
    aTerminateMessage       = RegisterWindowMessage( TERMINATE_MESSAGE );

    aListenerWindow = CreateWindowEx(0,
        LISTENER_WINDOWCLASS,       // registered class name
        LISTENER_WINDOWNAME,        // window name
        0, //WS_CHILD,              // window style
        CW_USEDEFAULT,              // horizontal position of window
        CW_USEDEFAULT,              // vertical position of window
        CW_USEDEFAULT,              // window width
        CW_USEDEFAULT,              // window height
        (HWND) NULL,                // handle to parent or owner window
        NULL,                       // menu handle or child identifier
        (HINSTANCE) GetModuleHandle( NULL ),    // handle to application instance
        NULL                        // window-creation data
        );
#endif
}


ShutdownIcon::~ShutdownIcon()
{
#ifdef WNT
    if( IsWindow( aListenerWindow ) )
    {
        DestroyWindow( aListenerWindow );
        aListenerWindow = NULL;
    }
    UnregisterClass( LISTENER_WINDOWCLASS, GetModuleHandle( NULL ) );
#endif
    m_xDesktop = Reference < XDesktop > ();
}

// one instance stuff
void ShutdownIcon::create( Reference< XDesktop >& aDesktop )
{
    if( !pShutdownIcon )
    {
        pShutdownIcon = new ShutdownIcon( aDesktop );
        pShutdownIcon->acquire();
    }
}

void ShutdownIcon::destroy()
{
    if( pShutdownIcon )
    {
        pShutdownIcon->release();
        pShutdownIcon = NULL;
    }
}

ShutdownIcon* ShutdownIcon::getInstance()
{
    return pShutdownIcon;
}

void ShutdownIcon::terminateDesktop()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        // always remove ourselves as listener
        getInstance()->m_xDesktop->removeTerminateListener( getInstance() );

        // terminate desktop only if no tasks exist
        Reference < XTasksSupplier > xTasksSupplier( getInstance()->m_xDesktop, UNO_QUERY );
        if( xTasksSupplier.is() )
        {
            Reference < XEnumerationAccess > xEnum = xTasksSupplier->getTasks();
            if( xEnum.is() )
            {
                Reference < XElementAccess > xElement ( xEnum, UNO_QUERY );
                if ( xElement.is() )
                {
                    if( !xElement->hasElements() )
                        getInstance()->m_xDesktop->terminate();
                }
            }
        }
    }
}

// XInterface
void SAL_CALL ShutdownIcon::acquire()
throw( ::com::sun::star::uno::RuntimeException )
{
    OWeakObject::acquire();
}

void SAL_CALL ShutdownIcon::release()
throw( ::com::sun::star::uno::RuntimeException )
{
    OWeakObject::release();
}

Any SAL_CALL ShutdownIcon::queryInterface( const Type & rType ) throw( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST(
                ::com::sun::star::frame::XTerminateListener* , this ),
                SAL_STATIC_CAST(
                ::com::sun::star::lang::XEventListener*, this ));
    if( a.hasValue() )
    {
        return a;
    }

    return OWeakObject::queryInterface( rType );
}


// XEventListener
void SAL_CALL ShutdownIcon::disposing( const ::com::sun::star::lang::EventObject& Source )
    throw(::com::sun::star::uno::RuntimeException)
{
}

// XTerminateListener
void SAL_CALL ShutdownIcon::queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard  aGuard( m_aMutex );

    if ( bTerminationVeto )
        throw ::com::sun::star::frame::TerminationVetoException();
}


void SAL_CALL ShutdownIcon::notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
throw(::com::sun::star::uno::RuntimeException)
{
}

