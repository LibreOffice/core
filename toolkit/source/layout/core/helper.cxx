/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "helper.hxx"

#include <assert.h>
#include <list>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <tools/debug.hxx>

#include "proplist.hxx"

#if TEST_LAYOUT && !defined( DBG_UTIL )
#undef DBG_ERROR
#define DBG_ERROR OSL_TRACE
#undef DBG_ERROR1
#define DBG_ERROR1 OSL_TRACE
#undef DBG_ERROR2
#define DBG_ERROR2 OSL_TRACE
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

#include "bin.hxx"
#include "box.hxx"
#include "dialogbuttonhbox.hxx"
#include "flow.hxx"
#include "localized-string.hxx"
#include "table.hxx"

namespace layoutimpl
{

oslModule WidgetFactory::mSfx2Library = 0;
WindowCreator WidgetFactory::mSfx2CreateWidget = 0;

uno::Reference <awt::XLayoutContainer> WidgetFactory::createContainer (OUString const& name)
{
    uno::Reference< awt::XLayoutContainer > xPeer;

    if ( name.equalsAscii( "hbox" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new HBox() );
    else if ( name.equalsAscii( "vbox" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new VBox() );
    else if ( name.equalsAscii( "table" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Table() );
    else if ( name.equalsAscii( "flow" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Flow() );
    else if ( name.equalsAscii( "bin" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Bin() );
    else if ( name.equalsAscii( "min-size" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new MinSize() );
    else if ( name.equalsAscii( "align" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new Align() );
    else if ( name.equalsAscii( "dialogbuttonhbox" ) )
        xPeer = uno::Reference< awt::XLayoutContainer >( new DialogButtonHBox() );

    return xPeer;
}

uno::Reference <awt::XLayoutConstrains> WidgetFactory::toolkitCreateWidget (uno::Reference <awt::XToolkit> xToolkit, uno::Reference <uno::XInterface> xParent, OUString const& name, long properties)
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

#if 0
        // top container -- a wrapper for framewindow -- is de-coupled
        // from awt::XWindowPeer. So, getParent() fails at it.
        uno::Reference< awt::XWindowPeer > xWinParent = getParent( xParent );
#else
        uno::Reference< awt::XWindowPeer > xWinParent( xParent, uno::UNO_QUERY );
#endif
        assert( xParent.is() );
        assert( xWinParent.is() );
        /*
          With the new three layer instarr/rpath feature, when
          prepending toolkit/unxlngx6.pro/lib or $SOLARVER/lib to
          LD_LIBRARY_PATH, VCLXWindow::GetImplementation returns 0x0
          vclxtoolkit::ImplCreateWindow failing to create any widget;
          although it succeeds here.

          While developing, one now must copy libtlx.so to
          $OOO_INSTALL_PREFIX/openoffice.org/basis3.0/program/libtklx.so
          each time.
        */
        VCLXWindow* parentComponent = VCLXWindow::GetImplementation( xWinParent );
        if ( !parentComponent )
            throw uno::RuntimeException(
                OUString::createFromAscii( "parent has no implementation" ),
                uno::Reference< uno::XInterface >() );
        desc.Parent = xWinParent;
    }

    desc.ParentIndex = 0;
    // debugging help ...
    desc.Bounds.X = 0;
    desc.Bounds.Y = 0;
    desc.Bounds.Width = 300;
    desc.Bounds.Height = 200;

    desc.WindowAttributes = properties;
    desc.WindowServiceName = name;

    uno::Reference< awt::XWindowPeer > xWinPeer;
    try
    {
        OSL_TRACE("Asking toolkit: %s", OUSTRING_CSTR( desc.WindowServiceName ) );
        xWinPeer = xToolkit->createWindow( desc );
        if ( !xWinPeer.is() )
            throw uno::RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "Cannot create peer" ) ),
                uno::Reference< uno::XInterface >() );
        xPeer = uno::Reference< awt::XLayoutConstrains >( xWinPeer, uno::UNO_QUERY );
    }
    catch( uno::Exception & )
    {
        DBG_ERROR1( "Warning: %s is not a recognized type\n", OUSTRING_CSTR( name ) );
        return uno::Reference< awt::XLayoutConstrains >();
    }

#if 0 // This shadows the show="false" property and seems otherwise
      // unnecessary

    // default to visible, let then people change it on properties
    if ( ! bToplevel )
    {
        uno::Reference< awt::XWindow> xWindow( xPeer, uno::UNO_QUERY );
        if ( xWindow.is() )
            xWindow->setVisible( true );
    }
#endif

    return xPeer;
}

uno::Reference< awt::XLayoutConstrains >
WidgetFactory::createWidget (uno::Reference< awt::XToolkit > xToolkit, uno::Reference< uno::XInterface > xParent, OUString const& name, long properties)
{
    uno::Reference< awt::XLayoutConstrains > xPeer;

    xPeer = uno::Reference <awt::XLayoutConstrains> (createContainer (name), uno::UNO_QUERY);
    if ( xPeer.is() )
        return xPeer;

    xPeer = implCreateWidget (xParent, name, properties);
    if (xPeer.is ())
        return xPeer;

#define FIXED_INFO 1
#if FIXED_INFO
    OUString tName = name;
    // FIXME
    if ( name.equalsAscii( "fixedinfo" ) )
        tName = OUString::createFromAscii( "fixedtext" );
    xPeer = toolkitCreateWidget (xToolkit, xParent, tName, properties);
#else
    xPeer = toolkitCreateWidget (xToolkit, xParent, name, properties);
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
        for ( unsigned int i = 0; i < maDetails.size(); i++)
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

uno::Reference <beans::XPropertySetInfo> SAL_CALL PropHelper::getPropertySetInfo () throw (uno::RuntimeException)
{
    return css::uno::Reference <css::beans::XPropertySetInfo> (createPropertySetInfo (getInfoHelper ()));
}

} // namespace layoutimpl

#include <awt/vclxbutton.hxx>
#include <awt/vclxdialog.hxx>
#include <awt/vclxfixedline.hxx>
#include <awt/vclxplugin.hxx>
#include <awt/vclxscroller.hxx>
#include <awt/vclxsplitter.hxx>
#include <awt/vclxtabcontrol.hxx>
#include <awt/vclxtabpage.hxx>
#include <toolkit/awt/vclxtoolkit.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/unohelp.hxx>

#include <layout/layout.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <vcl/lstbox.hxx>
#include <vcl.hxx>

#include <typeinfo>

namespace layoutimpl
{

uno::Reference <awt::XLayoutConstrains> WidgetFactory::implCreateWidget (uno::Reference <uno::XInterface> xParent, OUString name, long attributes)
{
    Window* parent = 0;

    if (VCLXWindow* parentComponent = VCLXWindow::GetImplementation (xParent))
        parent = parentComponent->GetWindow ();

    VCLXWindow* component = 0;
    Window* window = 0; //sfx2CreateWindow (&component, parent, name, attributes);
    if (!window)
        window = layoutCreateWindow (&component, parent, name, attributes);

    uno::Reference <awt::XLayoutConstrains> reference;
    if (window)
    {
        window->SetCreatedWithToolkit( sal_True );
        if ( component )
            component->SetCreatedWithToolkit( true );
        reference = component;
        window->SetComponentInterface( component );
        if ( attributes & awt::WindowAttribute::SHOW )
            window->Show();
    }

    return reference;
}

extern "C" { static void SAL_CALL thisModule() {} }

Window* WidgetFactory::sfx2CreateWindow (VCLXWindow** component, Window* parent, OUString const& name, long& attributes)
{
    OSL_TRACE("Asking sfx2: %s", OUSTRING_CSTR (name));

    if (!mSfx2Library)
    {
        OUString libraryName = ::vcl::unohelper::CreateLibraryName ("sfx", sal_True);
        mSfx2Library = osl_loadModuleRelative (&thisModule, libraryName.pData, SAL_LOADMODULE_DEFAULT);
        if (mSfx2Library)
        {
            OUString functionName (RTL_CONSTASCII_USTRINGPARAM ("CreateWindow"));
            mSfx2CreateWidget = (WindowCreator) osl_getFunctionSymbol (mSfx2Library, functionName.pData);
        }
    }

    if (mSfx2CreateWidget)
        return mSfx2CreateWidget (component, name, parent, attributes);

    return 0;
}

Window* WidgetFactory::layoutCreateWindow (VCLXWindow** component, Window *parent, OUString const& name, long& attributes)
{
    Window* window = 0;

    if (0)
    {
        ;
    }
    if ( name.equalsAscii( "dialog" ) )
    {
        if ( parent == NULL )
            parent = DIALOG_NO_PARENT;
        window = new Dialog( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXDialog();

        attributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( name.equalsAscii( "modaldialog" ) )
    {
        if ( parent == NULL )
            parent = DIALOG_NO_PARENT;
        window = new ModalDialog( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXDialog();

        attributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( name.equalsAscii( "modelessdialog" ) )
    {
        if ( parent == NULL )
            parent = DIALOG_NO_PARENT;
        window = new ModelessDialog (parent, ImplGetWinBits (attributes, 0));
        *component = new layoutimpl::VCLXDialog();

        attributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( name.equalsAscii( "sfxdialog" ) )
    {
        if ( parent == NULL )
            parent = DIALOG_NO_PARENT;
        window = new ClosingDialog (parent, ImplGetWinBits (attributes, 0));
        *component = new layoutimpl::VCLXDialog();

        attributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( name.equalsAscii( "sfxmodaldialog" ) )
    {
        if ( parent == NULL )
            parent = DIALOG_NO_PARENT;
        window = new ClosingModalDialog( parent,
                                         ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXDialog();

        attributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( name.equalsAscii( "sfxmodelessdialog" ) )
    {
        if ( parent == NULL )
            parent = DIALOG_NO_PARENT;
        window = new ClosingModelessDialog (parent, ImplGetWinBits (attributes, 0));
        *component = new layoutimpl::VCLXDialog();

        attributes ^= awt::WindowAttribute::SHOW;
    }
    else if ( name.equalsAscii( "tabcontrol" ) )
    {
        window = new TabControl( parent, ImplGetWinBits( attributes, WINDOW_TABCONTROL ) );
        *component = new layoutimpl::VCLXTabControl();
    }
    else if ( name.equalsAscii( "scroller" ) )
    {
        // used FixedImage because I just want some empty non-intrusive widget
        window = new FixedImage( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXScroller();
    }
    else if ( name.equalsAscii( "hsplitter" ) || name.equalsAscii( "vsplitter" ) )
    {
        window = new FixedImage( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXSplitter( name.equalsAscii( "hsplitter" ) );
    }
    else if ( name.equalsAscii( "hfixedline" ) || name.equalsAscii( "vfixedline" ) )
    {
        WinBits nStyle = ImplGetWinBits( attributes, 0 );
        nStyle ^= WB_HORZ;
        if ( name.equalsAscii( "hfixedline" ) )
            nStyle |= WB_HORZ;
        else
            nStyle |= WB_VERT;
        window = new FixedLine( parent, nStyle );
        *component = new layoutimpl::VCLXFixedLine();
    }
    else if ( name.equalsAscii( "okbutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXOKButton( window );
        window->SetType (WINDOW_OKBUTTON);
    }
    else if ( name.equalsAscii( "cancelbutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXCancelButton( window );
        window->SetType (WINDOW_CANCELBUTTON);
    }
    else if ( name.equalsAscii( "yesbutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXYesButton( window );
        window->SetType (WINDOW_OKBUTTON);
    }
    else if ( name.equalsAscii( "nobutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        window->SetType (WINDOW_CANCELBUTTON);
        *component = new layoutimpl::VCLXNoButton( window );
    }
    else if ( name.equalsAscii( "retrybutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXRetryButton( window );
    }
    else if ( name.equalsAscii( "ignorebutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXIgnoreButton( window );
    }
    else if ( name.equalsAscii( "resetbutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXResetButton( window );
    }
    else if ( name.equalsAscii( "applybutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXApplyButton( window );
    }
    else if ( name.equalsAscii( "helpbutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXHelpButton( window );
        window->SetType (WINDOW_HELPBUTTON);
    }
    else if ( name.equalsAscii( "morebutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXMoreButton( window );
        window->SetType (WINDOW_MOREBUTTON);
    }
    else if ( name.equalsAscii( "advancedbutton" ) )
    {
        window = new PushButton( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::VCLXAdvancedButton( window );
    }
    else if ( name.equalsAscii( "plugin" ) )
    {
        window = new Control( parent, ImplGetWinBits( attributes, 0 ) );
#ifndef __SUNPRO_CC
        OSL_TRACE( "%s: parent=%p (%s)\n", __FUNCTION__, parent, typeid( *parent ).name() );
#endif
        *component = new layoutimpl::VCLXPlugin( window, ImplGetWinBits( attributes, 0 ) );
    }
    else if ( name.equalsAscii( "tabpage" ) )
    {
#if 0
        if ( !parent )
            parent = layout::TabPage::global_parent;
#else
        if (layout::TabPage::global_parent)
            parent = layout::TabPage::global_parent;
        layout::TabPage::global_parent = 0;
#endif
        //window = new TabPage( parent, ImplGetWinBits( attributes, 0 ) );
        attributes ^= awt::WindowAttribute::SHOW;
        WinBits nStyle = ImplGetWinBits( attributes, 0 );
        nStyle |= WB_HIDE;

        if (!parent)
        {
            window = new Dialog( parent, nStyle );
            *component = new VCLXDialog();
        }
        else
        {
            window = new TabPage( parent, nStyle );
            *component = new VCLXTabPage( window );
        }
    }
    else if ( name.equalsAscii( "string" ) )
    {
        // FIXME: move <string>s.text to simple map<string> in root?
        attributes &= ~awt::WindowAttribute::SHOW;
        window = new Window( parent, ImplGetWinBits( attributes, 0 ) );
        *component = new layoutimpl::LocalizedString();
    }
#if 0 // parent paranoia
    else if ( name.equalsAscii( "listbox" ) )
    {
        window = new ListBox (parent, ImplGetWinBits (attributes, 0));
        *component = new VCLXListBox ();
    }
#endif
    else if (name.equalsAscii ("svxfontlistbox")
             || name.equalsAscii ("svxlanguagebox"))
    {
        window = new ListBox (parent, ImplGetWinBits (attributes, 0));
        *component = new VCLXListBox ();
    }
    return window;
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
