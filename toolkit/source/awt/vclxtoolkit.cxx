/*************************************************************************
 *
 *  $RCSfile: vclxtoolkit.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_WINDOWCLASS_HPP_
#include <com/sun/star/awt/WindowClass.hpp>
#endif

#include <cppuhelper/typeprovider.hxx>
#include <rtl/memory.h>
#include <rtl/uuid.h>

#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/awt/vclxsystemdependentwindow.hxx>
#include <toolkit/awt/vclxregion.hxx>
#include <toolkit/awt/vclxtoolkit.hxx>
#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/convert.hxx>

#include <vcl/btndlg.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/group.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/spin.hxx>
#include <vcl/split.hxx>
#include <vcl/splitwin.hxx>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <vcl/syschild.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>

#include <tools/debug.hxx>



#define VCLWINDOW_FRAMEWINDOW               0x1000
#define VCLWINDOW_SYSTEMCHILDWINDOW         0x1001


sal_uInt32 ImplGetWinBits( sal_uInt32 nComponentAttribs, sal_uInt16 nCompType )
{
    sal_uInt32 nWinBits = 0;

    sal_Bool bMessBox = sal_False;
    if ( ( nCompType == WINDOW_INFOBOX ) ||
         ( nCompType == WINDOW_MESSBOX ) ||
         ( nCompType == WINDOW_QUERYBOX ) ||
         ( nCompType == WINDOW_WARNINGBOX ) ||
         ( nCompType == WINDOW_ERRORBOX ) )
    {
        bMessBox = sal_True;
    }

    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::BORDER )
        nWinBits |= WB_BORDER;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::NOBORDER )
        nWinBits |= WB_NOBORDER;
    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::SIZEABLE )
        nWinBits |= WB_SIZEABLE;
    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::MOVEABLE )
        nWinBits |= WB_MOVEABLE;
    if( nComponentAttribs & ::com::sun::star::awt::WindowAttribute::CLOSEABLE )
        nWinBits |= WB_CLOSEABLE;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::HSCROLL )
        nWinBits |= WB_HSCROLL;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::VSCROLL )
        nWinBits |= WB_VSCROLL;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::LEFT )
        nWinBits |= WB_LEFT;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::CENTER )
        nWinBits |= WB_CENTER;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::RIGHT )
        nWinBits |= WB_RIGHT;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::SPIN )
        nWinBits |= WB_SPIN;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::SORT )
        nWinBits |= WB_SORT;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DROPDOWN )
        nWinBits |= WB_DROPDOWN;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEFBUTTON )
        nWinBits |= WB_DEFBUTTON;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::READONLY )
        nWinBits |= WB_READONLY;
    if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::CLIPCHILDREN )
        nWinBits |= WB_CLIPCHILDREN;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::GROUP )
            nWinBits |= WB_GROUP;

    // These bits are not uniqe
    if ( bMessBox )
    {
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::OK )
            nWinBits |= WB_OK;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::OK_CANCEL )
            nWinBits |= WB_OK_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::YES_NO )
            nWinBits |= WB_YES_NO;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::YES_NO_CANCEL )
            nWinBits |= WB_YES_NO_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::RETRY_CANCEL )
            nWinBits |= WB_RETRY_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_OK )
            nWinBits |= WB_DEF_OK;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_CANCEL )
            nWinBits |= WB_DEF_CANCEL;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_RETRY )
            nWinBits |= WB_DEF_RETRY;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_YES )
            nWinBits |= WB_DEF_YES;
        if( nComponentAttribs & ::com::sun::star::awt::VclWindowPeerAttribute::DEF_NO )
            nWinBits |= WB_DEF_NO;
    }

    return nWinBits;
}

struct ComponentInfo
{
    const char*     pName;
    WindowType      nWinType;
};

static ComponentInfo __FAR_DATA aComponentInfos [] =
{
    { "buttondialog",       WINDOW_BUTTONDIALOG },
    { "cancelbutton",       WINDOW_CANCELBUTTON },
    { "checkbox",           WINDOW_CHECKBOX },
    { "combobox",           WINDOW_COMBOBOX },
    { "control",            WINDOW_CONTROL },
    { "currencybox",        WINDOW_CURRENCYBOX },
    { "currencyfield",      WINDOW_CURRENCYFIELD },
    { "datebox",            WINDOW_DATEBOX },
    { "datefield",          WINDOW_DATEFIELD },
    { "dialog",             WINDOW_DIALOG },
    { "dockingwindow",      WINDOW_DOCKINGWINDOW },
    { "edit",               WINDOW_EDIT },
    { "errorbox",           WINDOW_ERRORBOX },
    { "fixedbitmap",        WINDOW_FIXEDBITMAP },
    { "fixedimage",         WINDOW_FIXEDIMAGE },
    { "fixedline",          WINDOW_FIXEDLINE },
    { "fixedtext",          WINDOW_FIXEDTEXT },
    { "floatingwindow",     WINDOW_FLOATINGWINDOW },
    { "framewindow",        VCLWINDOW_FRAMEWINDOW },
    { "groupbox",           WINDOW_GROUPBOX },
    { "helpbutton",         WINDOW_HELPBUTTON },
    { "imagebutton",        WINDOW_IMAGEBUTTON },
    { "imageradiobutton",   WINDOW_IMAGERADIOBUTTON },
    { "infobox",            WINDOW_INFOBOX },
    { "listbox",            WINDOW_LISTBOX },
    { "longcurrencybox",    WINDOW_LONGCURRENCYBOX },
    { "longcurrencyfield",  WINDOW_LONGCURRENCYFIELD },
    { "menubutton",         WINDOW_MENUBUTTON },
    { "messbox",            WINDOW_MESSBOX },
    { "metricbox",          WINDOW_METRICBOX },
    { "metricfield",        WINDOW_METRICFIELD },
    { "modaldialog",        WINDOW_MODALDIALOG },
    { "modelessdialog",     WINDOW_MODELESSDIALOG },
    { "morebutton",         WINDOW_MOREBUTTON },
    { "multilistbox",       WINDOW_MULTILISTBOX },
    { "numericbox",         WINDOW_NUMERICBOX },
    { "numericfield",       WINDOW_NUMERICFIELD },
    { "okbutton",           WINDOW_OKBUTTON },
    { "patternbox",         WINDOW_PATTERNBOX },
    { "patternfield",       WINDOW_PATTERNFIELD },
    { "pushbutton",         WINDOW_PUSHBUTTON },
    { "querybox",           WINDOW_QUERYBOX },
    { "radiobutton",        WINDOW_RADIOBUTTON },
    { "scrollbar",          WINDOW_SCROLLBAR },
    { "scrollbarbox",       WINDOW_SCROLLBARBOX },
    { "spinbutton",         WINDOW_SPINBUTTON },
    { "spinfield",          WINDOW_SPINFIELD },
    { "splitter",           WINDOW_SPLITTER },
    { "splitwindow",        WINDOW_SPLITWINDOW },
    { "statusbar",          WINDOW_STATUSBAR },
    { "systemchildwindow",  VCLWINDOW_SYSTEMCHILDWINDOW },
    { "tabcontrol",         WINDOW_TABCONTROL },
    { "tabdialog",          WINDOW_TABDIALOG },
    { "tabpage",            WINDOW_TABPAGE },
    { "timebox",            WINDOW_TIMEBOX },
    { "timefield",          WINDOW_TIMEFIELD },
    { "toolbox",            WINDOW_TOOLBOX },
    { "tristatebox",        WINDOW_TRISTATEBOX },
    { "warningbox",         WINDOW_WARNINGBOX },
    { "window",             WINDOW_WINDOW },
    { "workwindow",         WINDOW_WORKWINDOW }
};

static int
#if defined( WNT )
 __cdecl
#endif
#if defined( ICC ) && defined( OS2 )
_Optlink
#endif
     ComponentInfoCompare( const void* pFirst, const void* pSecond)
{
    return( strcmp( ((ComponentInfo*)pFirst)->pName,
                    ((ComponentInfo*)pSecond)->pName ) );
}

sal_uInt16 ImplGetComponentType( const String& rServiceName )
{
    static sal_Bool bSorted = sal_False;
    if( !bSorted )
    {
        qsort(  (void*) aComponentInfos,
                sizeof( aComponentInfos ) / sizeof( ComponentInfo ),
                sizeof( ComponentInfo ),
                ComponentInfoCompare );
        bSorted = sal_True;
    }


    ComponentInfo aSearch;
    ByteString aServiceName( rServiceName, gsl_getSystemTextEncoding() );
    aServiceName.ToLowerAscii();
    if ( aServiceName.Len() )
        aSearch.pName = aServiceName.GetBuffer();
    else
        aSearch.pName = "window";

    ComponentInfo* pInf = (ComponentInfo*) bsearch( &aSearch,
                        (void*) aComponentInfos,
                        sizeof( aComponentInfos ) / sizeof( ComponentInfo ),
                        sizeof( ComponentInfo ),
                        ComponentInfoCompare );

    return pInf ? pInf->nWinType : 0;
}


//  ----------------------------------------------------
//  class VCLXToolkit
//  ----------------------------------------------------

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXToolkit::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XToolkit*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXToolkit )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>* ) NULL )
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::getDesktopWindow(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xRef;
    // 07/00: AppWindow doesn't exist anymore...
    return xRef;
}

::com::sun::star::awt::Rectangle VCLXToolkit::getWorkArea(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Rectangle aRect;
    // 07/00: AppWindow doesn't exist anymore...
    return aRect;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > VCLXToolkit::createWindow( const ::com::sun::star::awt::WindowDescriptor& rDescriptor ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xRef;

    Window* pParent = NULL;
    if ( rDescriptor.Parent.is() )
    {
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation( rDescriptor.Parent );
        DBG_ASSERT( pParentComponent, "ParentComponent not valid" );
        if ( pParentComponent )
            pParent = pParentComponent->GetWindow();
    }

    sal_uInt32 nWinBits = ImplGetWinBits( rDescriptor.WindowAttributes,
        ImplGetComponentType( rDescriptor.WindowServiceName ) );

    VCLXWindow* pNewComp = NULL;
    Window* pNewWindow = CreateComponent( &pNewComp, rDescriptor, pParent, nWinBits );

    DBG_ASSERT( pNewWindow, "createComponent: Unknown Component!" );
    DBG_ASSERTWARNING( pNewComp, "createComponent: No special Interface!" );

    if ( pNewWindow )
    {
        pNewWindow->SetCreatedWithToolkit( sal_True );
        pNewWindow->SetPosPixel( Point() );

        if ( rDescriptor.WindowAttributes & ::com::sun::star::awt::WindowAttribute::MINSIZE )
        {
            pNewWindow->SetSizePixel( Size() );
        }
        else if ( rDescriptor.WindowAttributes & ::com::sun::star::awt::WindowAttribute::FULLSIZE )
        {
            if ( pParent )
                pNewWindow->SetSizePixel( pParent->GetOutputSizePixel() );
        }
        else if ( !VCLUnoHelper::IsZero( rDescriptor.Bounds ) )
        {
            Rectangle aRect = VCLRectangle( rDescriptor.Bounds );
            pNewWindow->SetPosSizePixel( aRect.TopLeft(), aRect.GetSize() );
        }

        if ( rDescriptor.WindowAttributes & ::com::sun::star::awt::WindowAttribute::SHOW )
            pNewWindow->Show();

        if ( !pNewComp )
        {
            // Default-Interface
            xRef = pNewWindow->GetComponentInterface( sal_True );
        }
        else
        {
            pNewWindow->SetComponentInterface( pNewComp );
            xRef = pNewComp;
        }
    }

    return xRef;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > > VCLXToolkit::createWindows( const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::WindowDescriptor >& rDescriptors ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    sal_uInt32 nComponents = rDescriptors.getLength();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > > aSeq( nComponents );
    for ( sal_uInt32 n = 0; n < nComponents; n++ )
    {
        ::com::sun::star::awt::WindowDescriptor aDescr = rDescriptors.getConstArray()[n];

        if ( aDescr.ParentIndex == (-1) )
            aDescr.Parent = NULL;
        else if ( ( aDescr.ParentIndex >= 0 ) && ( aDescr.ParentIndex < (short)n ) )
            aDescr.Parent = aSeq.getConstArray()[aDescr.ParentIndex];
        aSeq.getArray()[n] = createWindow( aDescr );
    }
    return aSeq;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > VCLXToolkit::createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice > xRef;
    VCLXVirtualDevice* pVDev = new VCLXVirtualDevice;
    VirtualDevice* pV = new VirtualDevice;
    pV->SetOutputSizePixel( Size( Width, Height ) );
    pVDev->SetVirtualDevice( pV );

    xRef = pVDev;
    return xRef;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion > VCLXToolkit::createRegion(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >  xRef = new VCLXRegion;
    return xRef;
}

Window* VCLXToolkit::CreateComponent( VCLXWindow** ppNewComp,
    const ::com::sun::star::awt::WindowDescriptor& rDescriptor, Window* pParent, sal_uInt32 nWinBits )
{
    String aServiceName( rDescriptor.WindowServiceName );
    aServiceName.ToLowerAscii();

    Window* pNewWindow = NULL;
    sal_uInt16 nType = ImplGetComponentType( aServiceName );

    if ( !pParent )
    {
        // Wenn die Component einen Parent braucht, dann NULL zurueckgeben,
        // spaeter mal ::com::sun::star::uno::Exception...
        sal_Bool bException = sal_True;
        if ( ( nType == WINDOW_DIALOG ) || ( nType == WINDOW_MODALDIALOG ) || ( nType == WINDOW_MODELESSDIALOG ) )
            bException = sal_False;
        else if ( ( nType == WINDOW_WINDOW ) ||
                  ( nType == WINDOW_WORKWINDOW ) ||
                  ( nType == VCLWINDOW_FRAMEWINDOW ) )
        {
            if ( rDescriptor.Type == ::com::sun::star::awt::WindowClass_TOP )
                bException = sal_False;
        }

        if ( bException )
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }

    if ( nType )
    {
        NAMESPACE_VOS(OGuard) aVclGuard( Application::GetSolarMutex()  );
        switch ( (WindowType)nType )
        {
            case WINDOW_CANCELBUTTON:
                pNewWindow = new CancelButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_CHECKBOX:
                 pNewWindow = new CheckBox( pParent, nWinBits );
                *ppNewComp = new VCLXCheckBox;
            break;
            case WINDOW_COMBOBOX:
                pNewWindow = new ComboBox( pParent, nWinBits );
                ((ComboBox*)pNewWindow)->EnableAutoSize( sal_False );
                *ppNewComp = new VCLXComboBox;
            break;
            case WINDOW_CURRENCYBOX:
                pNewWindow = new CurrencyBox( pParent, nWinBits );
            break;
            case WINDOW_CURRENCYFIELD:
                pNewWindow = new CurrencyField( pParent, nWinBits );
                static_cast<CurrencyField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
                *ppNewComp = new VCLXNumericField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(CurrencyField*)pNewWindow );
            break;
            case WINDOW_DATEBOX:
                pNewWindow = new DateBox( pParent, nWinBits );
            break;
            case WINDOW_DATEFIELD:
                pNewWindow = new DateField( pParent, nWinBits );
                static_cast<DateField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
                *ppNewComp = new VCLXDateField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(DateField*)pNewWindow );
            break;
            case WINDOW_DIALOG:
                pNewWindow = new Dialog( pParent, nWinBits );
                *ppNewComp = new VCLXDialog;
            break;
            case WINDOW_EDIT:
                pNewWindow = new Edit( pParent, nWinBits );
                *ppNewComp = new VCLXEdit;
            break;
            case WINDOW_ERRORBOX:
                pNewWindow = new ErrorBox( pParent, nWinBits, String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_FIXEDBITMAP:
                pNewWindow = new FixedBitmap( pParent, nWinBits );
            break;
            case WINDOW_FIXEDIMAGE:
                pNewWindow = new ImageControl( pParent, nWinBits );
                *ppNewComp = new VCLXImageControl;
            break;
            case WINDOW_FIXEDLINE:
                pNewWindow = new FixedLine( pParent, nWinBits );
            break;
            case WINDOW_FIXEDTEXT:
                pNewWindow = new FixedText( pParent, nWinBits );
                *ppNewComp = new VCLXFixedText;
            break;
            case WINDOW_FLOATINGWINDOW:
                pNewWindow = new FloatingWindow( pParent, nWinBits );
            break;
            case WINDOW_GROUPBOX:
                pNewWindow = new GroupBox( pParent, nWinBits );
            break;
            case WINDOW_HELPBUTTON:
                pNewWindow = new HelpButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_IMAGEBUTTON:
                 pNewWindow = new ImageButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_IMAGERADIOBUTTON:
                pNewWindow = new ImageRadioButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_INFOBOX:
                pNewWindow = new InfoBox( pParent, String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_LISTBOX:
                pNewWindow = new ListBox( pParent, nWinBits );
                ((ListBox*)pNewWindow)->EnableAutoSize( sal_False );
                *ppNewComp = new VCLXListBox;
            break;
            case WINDOW_LONGCURRENCYBOX:
                pNewWindow = new LongCurrencyBox( pParent, nWinBits );
            break;
            case WINDOW_LONGCURRENCYFIELD:
                pNewWindow = new LongCurrencyField( pParent, nWinBits );
                *ppNewComp = new VCLXCurrencyField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(LongCurrencyField*)pNewWindow );
            break;
            case WINDOW_MENUBUTTON:
                pNewWindow = new MenuButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_MESSBOX:
                pNewWindow = new MessBox( pParent, nWinBits, String(), String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_METRICBOX:
                pNewWindow = new MetricBox( pParent, nWinBits );
            break;
            case WINDOW_METRICFIELD:
                pNewWindow = new MetricField( pParent, nWinBits );
            break;
            case WINDOW_MODALDIALOG:
            case WINDOW_MODELESSDIALOG:
                // Modal/Modeless nur durch Show/Execute
                pNewWindow = new Dialog( pParent, nWinBits );
                *ppNewComp = new VCLXDialog;
            break;
            case WINDOW_MOREBUTTON:
                pNewWindow = new MoreButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_MULTILISTBOX:
                pNewWindow = new MultiListBox( pParent, nWinBits );
                *ppNewComp = new VCLXListBox;
            break;
            case WINDOW_NUMERICBOX:
                pNewWindow = new NumericBox( pParent, nWinBits );
            break;
            case WINDOW_NUMERICFIELD:
                pNewWindow = new NumericField( pParent, nWinBits );
                static_cast<NumericField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
                *ppNewComp = new VCLXNumericField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(NumericField*)pNewWindow );
            break;
            case WINDOW_OKBUTTON:
                pNewWindow = new OKButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_PATTERNBOX:
                pNewWindow = new PatternBox( pParent, nWinBits );
            break;
            case WINDOW_PATTERNFIELD:
                pNewWindow = new PatternField( pParent, nWinBits );
                *ppNewComp = new VCLXPatternField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(PatternField*)pNewWindow );
            break;
            case WINDOW_PUSHBUTTON:
                pNewWindow = new PushButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_QUERYBOX:
                pNewWindow = new QueryBox( pParent, nWinBits, String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_RADIOBUTTON:
                pNewWindow = new RadioButton( pParent, nWinBits );
                *ppNewComp = new VCLXRadioButton;
            break;
            case WINDOW_SCROLLBAR:
                pNewWindow = new ScrollBar( pParent, nWinBits );
                *ppNewComp = new VCLXScrollBar;
            break;
            case WINDOW_SCROLLBARBOX:
                pNewWindow = new ScrollBarBox( pParent, nWinBits );
            break;
            case WINDOW_SPINBUTTON:
                pNewWindow = new SpinButton( pParent, nWinBits );
                *ppNewComp = new VCLXButton;
            break;
            case WINDOW_SPINFIELD:
                pNewWindow = new SpinField( pParent, nWinBits );
                *ppNewComp = new VCLXNumericField;
            break;
            case WINDOW_SPLITTER:
                pNewWindow = new Splitter( pParent, nWinBits );
            break;
            case WINDOW_SPLITWINDOW:
                pNewWindow = new SplitWindow( pParent, nWinBits );
            break;
            case WINDOW_STATUSBAR:
                pNewWindow = new StatusBar( pParent, nWinBits );
            break;
            case VCLWINDOW_SYSTEMCHILDWINDOW:
                pNewWindow = new SystemChildWindow( pParent, nWinBits );
                *ppNewComp = new VCLXSystemDependentWindow();
            break;
            case WINDOW_TABCONTROL:
                pNewWindow = new TabControl( pParent, nWinBits );
            break;
            case WINDOW_TABDIALOG:
                pNewWindow = new TabDialog( pParent, nWinBits );
            break;
            case WINDOW_TABPAGE:
                pNewWindow = new TabPage( pParent, nWinBits );
            break;
            case WINDOW_TIMEBOX:
                pNewWindow = new TimeBox( pParent, nWinBits );
            break;
            case WINDOW_TIMEFIELD:
                pNewWindow = new TimeField( pParent, nWinBits );
                static_cast<TimeField*>(pNewWindow)->EnableEmptyFieldValue( TRUE );
                *ppNewComp = new VCLXTimeField;
                ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(TimeField*)pNewWindow );
            break;
            case WINDOW_TOOLBOX:
                pNewWindow = new ToolBox( pParent, nWinBits );
            break;
            case WINDOW_TRISTATEBOX:
                pNewWindow = new TriStateBox( pParent, nWinBits );
            break;
            case WINDOW_WARNINGBOX:
                pNewWindow = new WarningBox( pParent, nWinBits, String() );
                *ppNewComp = new VCLXMessageBox;
            break;
            case WINDOW_WORKWINDOW:
            case WINDOW_WINDOW:
            case VCLWINDOW_FRAMEWINDOW:
            case WINDOW_DOCKINGWINDOW:
                if ( rDescriptor.Type == ::com::sun::star::awt::WindowClass_TOP )
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = new DockingWindow( pParent, nWinBits );
                    else
                        pNewWindow = new WorkWindow( pParent, nWinBits );
                    *ppNewComp = new VCLXTopWindow;
                }
                else if ( rDescriptor.Type == ::com::sun::star::awt::WindowClass_CONTAINER )
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = new DockingWindow( pParent, nWinBits );
                    else
                        pNewWindow = new Window( pParent, nWinBits );
                    *ppNewComp = new VCLXContainer;
                }
                else
                {
                    if (nType == WINDOW_DOCKINGWINDOW )
                        pNewWindow = new DockingWindow( pParent, nWinBits );
                    else
                        pNewWindow = new Window( pParent, nWinBits );
                    *ppNewComp = new VCLXWindow;
                }
            break;
            default:    DBG_ERROR( "UNO3!" );
        }
    }

    return pNewWindow;
}



