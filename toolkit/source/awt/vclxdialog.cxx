#include "vclxdialog.hxx"
#include "toolkit/helper/property.hxx"
#include <com/sun/star/awt/ScrollBarOrientation.hpp>


#include <tools/debug.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/dialog.hxx>

#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/macros.hxx>

#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>
#include <toolkit/helper/vclunohelper.hxx>

#include <com/sun/star/awt/PosSize.hpp>

#include "../layout/timer.hxx"

using namespace toolkit;
//........................................................................
namespace layoutimpl
{
//........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

//====================================================================
//= VCLXDialog
//====================================================================
DBG_NAME( VCLXDialog )
//--------------------------------------------------------------------
VCLXDialog::VCLXDialog()
: VCLXWindow()
    , VCLXDialog_Base()
    , Bin()
    , bRealized( false ), bResizeSafeguard( false )
{
    DBG_CTOR( VCLXDialog, NULL );

/*        mxLayoutUnit = uno::Reference< awt::XLayoutUnit >( new LayoutUnit() );
          assert(mxLayoutUnit.is());*/
}

//--------------------------------------------------------------------
VCLXDialog::~VCLXDialog()
{
    DBG_DTOR( VCLXDialog, NULL );
}

//--------------------------------------------------------------------
IMPLEMENT_2_FORWARD_XINTERFACE2( VCLXDialog, VCLXWindow, Bin, VCLXDialog_Base )

//--------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXDialog, VCLXWindow, VCLXDialog_Base )

//--------------------------------------------------------------------
void SAL_CALL VCLXDialog::dispose( ) throw(RuntimeException)
{
    {
        ::vos::OGuard aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = *this;
//            maTabListeners.disposeAndClear( aDisposeEvent );
    }

    VCLXWindow::dispose();
}

void VCLXDialog::resizedCb()
{
    queueResize();
}

void SAL_CALL VCLXDialog::allocateArea( const css::awt::Rectangle &rArea )
    throw (css::uno::RuntimeException)
{
    awt::Size reqSize = Bin::getMinimumSize();
    reqSize.Height = getHeightForWidth( rArea.Width );

    if ( !bRealized )
    {
        setPosSize( 0, 0, reqSize.Width, reqSize.Height, PosSize::SIZE );
        bRealized = true;
        setVisible( true );
    }
    else
    {
        awt::Size curSize = getSize();
        if ( reqSize.Width > curSize.Width )
            setPosSize( 0, 0, reqSize.Width, 0, PosSize::WIDTH );
        if ( reqSize.Height > curSize.Height )
            setPosSize( 0, 0, 0, reqSize.Height, PosSize::HEIGHT );
    }

    awt::Size size = getSize();
    maAllocation.Width = size.Width;
    maAllocation.Height = size.Height;

    Bin::allocateArea( maAllocation );
}

//--------------------------------------------------------------------
void VCLXDialog::ProcessWindowEvent( const VclWindowEvent& _rVclWindowEvent )
{
    ::vos::OClearableGuard aGuard( GetMutex() );

    switch ( _rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_RESIZE:
            resizedCb();
        default:
            aGuard.clear();
            VCLXWindow::ProcessWindowEvent( _rVclWindowEvent );
            break;
    }
}

//--------------------------------------------------------------------
void SAL_CALL VCLXDialog::setProperty( const ::rtl::OUString& PropertyName, const Any &Value ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
                VCLXWindow::setProperty( PropertyName, Value );
        }
    }
}

//--------------------------------------------------------------------
Any SAL_CALL VCLXDialog::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Any aReturn;
    if ( GetWindow() )
    {
        sal_uInt16 nPropertyId = GetPropertyId( PropertyName );
        switch ( nPropertyId )
        {
            default:
                aReturn = VCLXWindow::getProperty( PropertyName );
        }
    }
    return aReturn;
}

//---------------------------------------------------------------------

::com::sun::star::uno::Any VCLXDialog::getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aRet;
    Window* pWindow = VCLXWindow::GetWindow();
    if ( pWindow )
    {
        const SystemEnvData* pSysData = ((SystemWindow *)pWindow)->GetSystemData();
        if ( pSysData )
        {
#if (defined WNT)
            if ( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_WIN32 )
            {
                aRet <<= (sal_Int32)pSysData->hWnd;
            }
#elif (defined OS2)
            if ( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_OS2 )
            {
                aRet <<= (sal_Int32)pSysData->hWnd;
            }
#elif (defined QUARTZ)
            if ( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_MAC )
            {
                aRet <<= (sal_IntPtr)pSysData->rWindow;
            }
#elif (defined UNX)
            if ( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW )
            {
                ::com::sun::star::awt::SystemDependentXWindow aSD;
                aSD.DisplayPointer = sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(pSysData->pDisplay));
                aSD.WindowHandle = pSysData->aWindow;
                aRet <<= aSD;
            }
#endif
        }
    }
    return aRet;
}


void VCLXDialog::addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
#if 1
    ::vos::OGuard aGuard( GetMutex() );

    GetTopWindowListeners().addInterface( rxListener );
#else
#endif
}

void VCLXDialog::removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
#if 1
    ::vos::OGuard aGuard( GetMutex() );

    GetTopWindowListeners().removeInterface( rxListener );
#endif
}

void VCLXDialog::toFront(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        ((WorkWindow*)pWindow)->ToTop( TOTOP_RESTOREWHENMIN );
}

void VCLXDialog::toBack(  ) throw(::com::sun::star::uno::RuntimeException)
{
/* Not possible in VCL...

   ::vos::OGuard aGuard( GetMutex() );

   Window* pWindow = GetWindow();
   if ( pWindow )
   {
   ((WorkWindow*)pWindow)->ToBack();
   }
*/
}

void VCLXDialog::setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& /*rxMenu*/ ) throw(::com::sun::star::uno::RuntimeException)
{
#if 0
    ::vos::OGuard aGuard( GetMutex() );

    SystemWindow* pWindow = (SystemWindow*) GetWindow();
    if ( pWindow )
    {
        pWindow->SetMenuBar( NULL );
        if ( rxMenu.is() )
        {
            VCLXMenu* pMenu = VCLXMenu::GetImplementation( rxMenu );
            if ( pMenu && !pMenu->IsPopupMenu() )
                pWindow->SetMenuBar( (MenuBar*) pMenu->GetMenu() );
        }
    }
    mxMenuBar = rxMenu;
#endif
}

void VCLXDialog::setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Title );
}

::rtl::OUString VCLXDialog::getTitle() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aTitle;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aTitle = pWindow->GetText();
    return aTitle;
}

sal_Int16 VCLXDialog::execute() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int16 nRet = 0;
    if ( GetWindow() )
    {
        Dialog* pDlg = (Dialog*) GetWindow();
        Window* pParent = pDlg->GetWindow( WINDOW_PARENTOVERLAP );
        Window* pOldParent = NULL;
        if ( pParent && !pParent->IsReallyVisible() )
        {
            pOldParent = pDlg->GetParent();
            Window* pFrame = pDlg->GetWindow( WINDOW_FRAME );
            if ( pFrame != pDlg )
                pDlg->SetParent( pFrame );
        }
        nRet = pDlg->Execute();
        if ( pOldParent )
            pDlg->SetParent( pOldParent );
    }
    return nRet;
}

void VCLXDialog::endDialog( sal_Int32 nResult ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( nResult == BUTTONID_HELP )
    {
        // UGH: c&p button.cxx
        ::Window* pFocusWin = Application::GetFocusWindow();
        if ( !pFocusWin )
            pFocusWin = GetWindow();

        HelpEvent aEvt( pFocusWin->GetPointerPosPixel(), HELPMODE_CONTEXT );
        pFocusWin->RequestHelp( aEvt );
        return;
    }

    Dialog* pDlg = (Dialog*) GetWindow();
    if ( pDlg )
        pDlg->EndDialog( nResult );
}

void VCLXDialog::endExecute() throw(::com::sun::star::uno::RuntimeException)
{
    endDialog( 0 );
}

//........................................................................
} // namespace toolkit
//........................................................................
