#include "helper.hxx"

#include <assert.h>
#include <list>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <tools/debug.hxx>

#include "proplist.hxx"
#include "layoutcore.hxx"

#if TEST_LAYOUT && !defined( DBG_UTIL )
#include <stdio.h>
#undef DBG_ERROR
#define DBG_ERROR printf
#undef DBG_ERROR1
#define DBG_ERROR1 printf
#undef DBG_ERROR2
#define DBG_ERROR2 printf
#endif /* TEST_LAYOUT && !DBG_UTIL */

namespace layoutimpl
{
using namespace com::sun::star;
using rtl::OUString;

uno::Reference< awt::XWindowPeer >
getParent( uno::Reference< uno::XInterface > xRef )
{
    do
    {
        uno::Reference< awt::XWindowPeer > xPeer( xRef, uno::UNO_QUERY );
        if ( xPeer.is() )
            return xPeer;

        uno::Reference< awt::XLayoutContainer > xCont( xRef, uno::UNO_QUERY );
        if ( xCont.is() )
            xRef = xCont->getParent();
    }
    while ( xRef.is() );

    return uno::Reference< awt::XWindowPeer >();
}

#if 0
static uno::Reference< awt::XWindowPeer >
getToplevel( uno::Reference< uno::XInterface > xRef )
{
    uno::Reference< awt::XWindowPeer > xTop, i;
    while ( ( i = uno::Reference< awt::XWindowPeer >( xRef, uno::UNO_QUERY ) ).is() )
    {
        xTop = i;

        uno::Reference< awt::XLayoutContainer > xCont( xRef, uno::UNO_QUERY );
        if ( xCont.is() )
            xRef = xCont->getParent();
        else
            xRef = uno::Reference< awt::XWindowPeer >();
    }

    return xTop;
}
#endif

}

#include "dialogbuttonhbox.hxx"
#include "bin.hxx"
#include "box.hxx"
#include "table.hxx"
#include "flow.hxx"

namespace layoutimpl
{

static uno::Reference< awt::XLayoutConstrains > ImplCreateWindow(
    uno::Reference< uno::XInterface > xParent,
    OUString aName, long WindowAttributes );

uno::Reference< awt::XLayoutContainer >
createContainer( const OUString &rName )
{
    uno::Reference< awt::XLayoutContainer > xPeer;

    if ( rName.equalsAscii( "hbox" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new HBox() );
    else if ( rName.equalsAscii( "vbox" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new VBox() );
    else if ( rName.equalsAscii( "table" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Table() );
    else if ( rName.equalsAscii( "flow" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Flow() );
    else if ( rName.equalsAscii( "bin" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Bin() );
    else if ( rName.equalsAscii( "min-size" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new MinSize() );
    else if ( rName.equalsAscii( "align" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Align() );
    else if ( rName.equalsAscii( "dialogbuttonhbox" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new DialogButtonHBox() );

    return xPeer;
}

static uno::Reference< awt::XLayoutConstrains >
createToolkitWidget( uno::Reference< awt::XToolkit > xToolkit,
                     uno::Reference< uno::XInterface > xParent,
                     const OUString &rName, long nProps )
{
    uno::Reference< awt::XLayoutConstrains > xPeer;
    bool bToplevel = !xParent.is();

    // UNO Control Widget
    awt::WindowDescriptor desc;
    if ( bToplevel )
        desc.Type = awt::WindowClass_TOP;
    else
    {
        desc.Type = awt::WindowClass_SIMPLE;

        // top container -- a wrapper for framewindow -- is de-coupled
        // from awt::XWindowPeer. So, getParent() fails at it.
//            uno::Reference< awt::XWindowPeer > xWinParent = getParent( xParent );

        uno::Reference< awt::XWindowPeer > xWinParent( xParent, uno::UNO_QUERY );

        assert( xParent.is() );
        assert( xWinParent.is() );
        desc.Parent = xWinParent;
    }

    desc.ParentIndex = 0;
    // debugging help ...
    desc.Bounds.X = 0;
    desc.Bounds.Y = 0;
    desc.Bounds.Width = 300;
    desc.Bounds.Height = 200;

    desc.WindowAttributes = nProps;
    desc.WindowServiceName = rName;

    uno::Reference< awt::XWindowPeer > xWinPeer;
    try
    {
        xWinPeer = xToolkit->createWindow( desc );
        if ( !xWinPeer.is() )
            throw uno::RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot create peer" ) ),
                uno::Reference< uno::XInterface >() );
        xPeer = uno::Reference< awt::XLayoutConstrains >( xWinPeer, uno::UNO_QUERY );
    }
    catch( uno::Exception &ex )
    {
        DBG_ERROR1( "Warning:  %s is not a recognized type", OUSTRING_CSTR( rName ) );
        return uno::Reference< awt::XLayoutConstrains >();
    }

    // default to visible, let then people change it on properties
    if ( ! bToplevel )
    {
        uno::Reference< awt::XWindow> xWindow( xPeer, uno::UNO_QUERY );
        if ( xWindow.is() )
            xWindow->setVisible( true );
    }
    return xPeer;
}

uno::Reference< awt::XLayoutConstrains >
createWidget( uno::Reference< awt::XToolkit > xToolkit,
              uno::Reference< uno::XInterface > xParent,
              const OUString &rName, long nProps )
{
    uno::Reference< awt::XLayoutConstrains > xPeer;

    xPeer = uno::Reference< awt::XLayoutConstrains >(
        createContainer( rName ), uno::UNO_QUERY );
    if ( xPeer.is() )
        return xPeer;

    xPeer = ImplCreateWindow( xParent, rName, nProps );
    if ( xPeer.is() )
        return xPeer;

    xPeer = createInternalWidget( xToolkit, xParent, rName, nProps );
    if ( xPeer.is() )
        return xPeer;

#if FIXED_INFO
    OUString tName = rName;
    // FIXME
    if ( rName.equalsAscii( "fixedinfo" ) )
        tName = OUString::createFromAscii( "fixedtext" );
    xPeer = createToolkitWidget( xToolkit, xParent, tName, nProps );
#else
    xPeer = createToolkitWidget( xToolkit, xParent, rName, nProps );
#endif
    return xPeer;
}

PropHelper::PropHelper() : LockHelper()
                         , cppu::OPropertySetHelper( maBrdcstHelper )
                         , pHelper( NULL )
{
}

void
PropHelper::addProp (const char *pName, sal_Int32 nNameLen, rtl_TextEncoding e,
                     uno::Type aType, void *pPtr)
{
    // this sucks rocks for effiency ...
    PropDetails aDetails;
    aDetails.aName = rtl::OUString::intern( pName, nNameLen, e );
    aDetails.aType = aType;
    aDetails.pValue = pPtr;
    maDetails.push_back( aDetails );
}

cppu::IPropertyArrayHelper & SAL_CALL
PropHelper::getInfoHelper()
{
    if ( ! pHelper )
    {
        uno::Sequence< beans::Property > aProps( maDetails.size() );
        for( unsigned int i = 0; i < maDetails.size(); i++)
        {
            aProps[i].Name = maDetails[i].aName;
            aProps[i].Type = maDetails[i].aType;
            aProps[i].Handle = i;
            aProps[i].Attributes = 0;
        }
        pHelper = new cppu::OPropertyArrayHelper( aProps, false /* fixme: faster ? */ );

    }
    return *pHelper;
}

sal_Bool SAL_CALL
PropHelper::convertFastPropertyValue(
    uno::Any & rConvertedValue,
    uno::Any & rOldValue,
    sal_Int32 nHandle,
    const uno::Any& rValue )
    throw (lang::IllegalArgumentException)
{
    OSL_ASSERT( nHandle >= 0 && nHandle < (sal_Int32) maDetails.size() );

    // FIXME: no Any::getValue ...
    getFastPropertyValue( rOldValue, nHandle );
    if ( rOldValue != rValue )
    {
        rConvertedValue = rValue;
        return sal_True; // changed
    }
    else
    {
        rConvertedValue.clear();
        rOldValue.clear();
    }
    return sal_False;
}


void SAL_CALL
PropHelper::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle,
                                              const uno::Any& rValue )
    throw (uno::Exception)
{
    OSL_ASSERT( nHandle >= 0 && nHandle < (sal_Int32) maDetails.size() );

    const PropDetails &rInfo = maDetails[ nHandle ];

    uno_type_assignData( rInfo.pValue, rInfo.aType.getTypeLibType(),
                         rValue.pData, rValue.pType,
                         0, 0, 0 );

    if ( mpListener )
        mpListener->propertiesChanged();
}

void SAL_CALL
PropHelper::getFastPropertyValue( uno::Any& rValue,
                                  sal_Int32 nHandle ) const
{
    OSL_ASSERT( nHandle >= 0 && nHandle < (sal_Int32) maDetails.size() );
    const PropDetails &rInfo = maDetails[ nHandle ];
#if 0
    switch ( rInfo.aType.getTypeClass() )
    {
#define MAP(classtype,ctype)                        \
        case uno::TypeClass_##classtype:       \
            rValue <<= *(ctype *)(rInfo.pValue);    \
        break
        MAP( DOUBLE, double );
        MAP( SHORT, sal_Int16 );
        MAP( LONG,  sal_Int32 );
        MAP( UNSIGNED_SHORT, sal_uInt16 );
        MAP( UNSIGNED_LONG, sal_uInt32 );
        MAP( STRING, ::rtl::OUString );
        default:
            DBG_ERROR( "ERROR: unknown type to map!" );
            break;
    }
#undef MAP
#endif
    rValue.setValue( rInfo.pValue, rInfo.aType );
}

::com::sun::star::uno::Any
PropHelper::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    return OPropertySetHelper::queryInterface( rType );
}

} // namespace layoutimpl

#include "../awt/vclxdialog.hxx"
#include "../awt/vclxfixedline.hxx"
#include "../awt/vclxproxy.hxx"
#include "../awt/vclxscroller.hxx"
#include "../awt/vclxsplitter.hxx"
#include "../awt/vclxtabcontrol.hxx"
#include <toolkit/awt/vclxtoolkit.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

namespace layoutimpl
{

uno::Reference< awt::XLayoutConstrains > ImplCreateWindow(
    uno::Reference< uno::XInterface > xParent,
    OUString aName, long WindowAttributes )
{
    VCLXWindow *pNewComp = NULL;
    Window     *pNewWindow = NULL;
    Window     *pParent = NULL;
    uno::Reference< awt::XLayoutConstrains > xRef;

    VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( xParent );
    if ( pParentComponent )
        pParent = pParentComponent->GetWindow();

    if ( aName.equalsAscii( "dialog" ) )
    {
        if ( pParent == NULL )
            // DIALOG_NO_PARENT == 0xffff
            // it would crash otherwise
// FIXME: check for crash
            pParent = DIALOG_NO_PARENT; /*DIALOG_NO_PARENT == 0xffff ?! :P*/
        pNewWindow = new Dialog( pParent,
                                 ImplGetWinBits( WindowAttributes, 0 ) );
        pNewComp = new layoutimpl::VCLXDialog();

        WindowAttributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( aName.equalsAscii( "modaldialog" ) )
    {
        if ( pParent == NULL )
            pParent = DIALOG_NO_PARENT;
        pNewWindow = new ModalDialog( pParent,
                                      ImplGetWinBits( WindowAttributes, 0 ) );
        pNewComp = new layoutimpl::VCLXDialog();

        WindowAttributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( aName.equalsAscii( "tabcontrol" ) )
    {
        pNewWindow = new TabControl( pParent,
                                     ImplGetWinBits( WindowAttributes, WINDOW_TABCONTROL ) );
        pNewComp = new layoutimpl::VCLXTabControl();
    }
    else if ( aName.equalsAscii( "scroller" ) )
    {
        // used FixedImage because I just want some empty non-intrusive widget
        pNewWindow = new FixedImage( pParent, ImplGetWinBits( WindowAttributes, 0 ) );
        pNewComp = new layoutimpl::VCLXScroller();
    }
    else if ( aName.equalsAscii( "hsplitter" ) || aName.equalsAscii( "vsplitter" ) )
    {
        pNewWindow = new FixedImage( pParent, ImplGetWinBits( WindowAttributes, 0 ) );
        pNewComp = new layoutimpl::VCLXSplitter( aName.equalsAscii( "hsplitter" ) );
    }
    else if ( aName.equalsAscii( "hfixedline" ) || aName.equalsAscii( "vfixedline" ) )
    {
        WinBits nStyle = ImplGetWinBits( WindowAttributes, 0 );
        nStyle ^= WB_HORZ;
        if ( aName.equalsAscii( "hfixedline" ) )
            nStyle |= WB_HORZ;
        else
            nStyle |= WB_VERT;
        pNewWindow = new FixedLine( pParent, nStyle );
        pNewComp = new layoutimpl::VCLXFixedLine();
    }

    if ( !pNewWindow )
        return xRef;

    pNewWindow->SetCreatedWithToolkit( sal_True );
    pNewComp->SetCreatedWithToolkit( TRUE );
    xRef = pNewComp;
    pNewWindow->SetComponentInterface( pNewComp );
    if ( WindowAttributes & awt::WindowAttribute::SHOW )
        pNewWindow->Show();

    return xRef;
}

} // namespace layoutimpl

// Avoid polluting the rest of the code with vcl linkage pieces ...

#include <vcl/imagerepository.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graph.hxx>

namespace layoutimpl
{

uno::Reference< graphic::XGraphic > loadGraphic( const char *pName )
{
    BitmapEx aBmp;

    OUString aStr( pName, strlen( pName ), RTL_TEXTENCODING_ASCII_US );
    if ( aStr.compareToAscii( ".uno:" ) == 0 )
        aStr = aStr.copy( 5 ).toAsciiLowerCase();

    if ( !vcl::ImageRepository::loadImage( OUString::createFromAscii( pName ), aBmp, true ) )
        return uno::Reference< graphic::XGraphic >();

    return Graphic( aBmp ).GetXGraphic();
}

} // namespace layoutimpl
