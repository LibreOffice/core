/*************************************************************************
 *
 *  $RCSfile: unoiface.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mm $ $Date: 2001-02-22 18:22:48 $
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

#define _SVT_UNOIFACE_CXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <svmedit.hxx>
#ifndef _SVT_UNOIFACE_HXX
#include <unoiface.hxx>
#endif
#include <filedlg.hxx>
#include <filectrl.hxx>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include <toolkit/helper/property.hxx>
#endif

#ifndef _PRODUCE_HXX
#include <imgprod.hxx>
#endif

#ifndef _FMTFIELD_HXX_
#include <fmtfield.hxx>
#endif

#ifndef _NUMUNO_HXX
#include <numuno.hxx>
#endif

#ifndef _CALENDAR_HXX
#include <calendar.hxx>
#endif

//  ----------------------------------------------------
//  class ExtUnoWrapper
//  ----------------------------------------------------

ExtUnoWrapper::ExtUnoWrapper()
    : UnoWrapper( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>() )
{}

void ExtUnoWrapper::RegisterUnoServices()
{
/*
    UnoWrapper::RegisterUnoServices();

    // ImageProducer registrieren...
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xMSF = ::usr::getProcessServiceManager();

    ::rtl::OUString aServiceName( L"stardiv.uno.awt.ImageProducer" );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aServiceNames( &aServiceName, 1 );
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >  xSSF = ::usr::createOneInstanceFactory
            ( xMSF, L"ImageProducer", ImageProducer_CreateInstance, aServiceNames );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XSet >  xS( xMSF, UNO_QUERY );
    ::com::sun::star::uno::Any aAny( &xSSF, ::getCppuType((const ::com::sun::star::container::XSet*)0) );
    xS->insert( aAny );
*/
}

//  ----------------------------------------------------
//  class ExtVCLXToolkit
//  ----------------------------------------------------
Window* ExtVCLXToolkit::CreateComponent( VCLXWindow** ppNewComp, const ::com::sun::star::awt::WindowDescriptor& rDescriptor, Window* pParent, sal_uInt32 nWinBits )
{
    Window* pWindow = NULL;
    String aServiceName( rDescriptor.WindowServiceName );
    if ( aServiceName.EqualsIgnoreCaseAscii( "MultiLineEdit" ) )
    {
        if ( pParent )
        {
            pWindow = new MultiLineEdit( pParent, nWinBits|WB_IGNORETAB);
            *ppNewComp = new VCLXMultiLineEdit;
        }
        else
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }
    // irgendwas muss ich mir noch fuer 'Custom-Bits' einfallen lassen,
    // jetzt erstmal verschiedene Service-Namen:
//  else if ( aServiceName.ICompare( "FileDialog_open" ) == COMPARE_EQUAL )
//  {
//      pWindow = new FileDialog( pParent, nWinBits|WB_OPEN );
//      *ppNewComp = new VCLXFileDialog;
//  }
//  else if ( aServiceName.ICompare( "FileDialog_save" ) == COMPARE_EQUAL )
//  {
//      pWindow = new FileDialog( pParent, nWinBits|WB_SAVEAS );
//      *ppNewComp = new VCLXFileDialog;
//  }
    else if ( aServiceName.EqualsIgnoreCaseAscii( "FileControl" ) )
    {
        if ( pParent )
        {
            pWindow = new FileControl( pParent, nWinBits );
            *ppNewComp = new VCLXFileControl;
        }
        else
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }
    else if (aServiceName.EqualsIgnoreCaseAscii("FormattedField") )
    {
        pWindow = new FormattedField(pParent, nWinBits);
        *ppNewComp = new SVTXFormattedField;
    }
    else if (aServiceName.EqualsIgnoreCaseAscii("NumericField") )
    {
        pWindow = new DoubleNumericField( pParent, nWinBits );
        *ppNewComp = new SVTXNumericField;
    }
    else if (aServiceName.EqualsIgnoreCaseAscii("LongCurrencyField") )
    {
        pWindow = new DoubleCurrencyField( pParent, nWinBits );
        *ppNewComp = new SVTXCurrencyField;
    }
    else if (aServiceName.EqualsIgnoreCaseAscii("datefield") )
    {
        pWindow = new CalendarField( pParent, nWinBits);
        static_cast<CalendarField*>(pWindow)->EnableToday();
        static_cast<CalendarField*>(pWindow)->EnableNone();
        static_cast<CalendarField*>(pWindow)->EnableEmptyFieldValue( TRUE );
        *ppNewComp = new VCLXDateField;
        ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(DateField*)pWindow );
    }

    if ( !pWindow )
        pWindow = VCLXToolkit::CreateComponent( ppNewComp, rDescriptor, pParent, nWinBits );

    return pWindow;
}

//  ----------------------------------------------------
//  INIT
//  ----------------------------------------------------
void InitExtVclToolkit()
{
    ExtUnoWrapper* pWrapper = new ExtUnoWrapper;
//  pWrapper->SetToolkitCreateFunction( ExtVCLXToolkit_CreateInstance );
    Application::SetUnoWrapper( pWrapper );
}


// ----------------------------------------------------
//  class VCLXMultiLineEdit
//  ----------------------------------------------------
VCLXMultiLineEdit::VCLXMultiLineEdit() : maTextListeners( *this )
{
}

VCLXMultiLineEdit::~VCLXMultiLineEdit()
{
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        pEdit->SetModifyHdl( Link() );
}

::com::sun::star::uno::Any VCLXMultiLineEdit::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextComponent*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextArea*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMultiLineEdit )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextArea>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXMultiLineEdit::SetWindow( Window* pWindow )
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pPrevMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pPrevMultiLineEdit )
        pPrevMultiLineEdit->SetModifyHdl( Link() );

    MultiLineEdit* pNewMultiLineEdit = (MultiLineEdit*) pWindow;
    if ( pNewMultiLineEdit )
        pNewMultiLineEdit->SetModifyHdl( LINK( this, VCLXMultiLineEdit, ModifyHdl ) );

    VCLXWindow::SetWindow( pWindow );
}

void VCLXMultiLineEdit::addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    maTextListeners.addInterface( l );
}

void VCLXMultiLineEdit::removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    maTextListeners.removeInterface( l );
}

void VCLXMultiLineEdit::setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
    {
        pEdit->SetText( aText );

        // In JAVA wird auch ein textChanged ausgeloest, in VCL nicht.
        // ::com::sun::star::awt::Toolkit soll JAVA-komform sein...
        ModifyHdl( NULL );
    }
}

void VCLXMultiLineEdit::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
    {
        setSelection( rSel );
        pEdit->ReplaceSelected( aText );
    }
}

::rtl::OUString VCLXMultiLineEdit::getText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
        aText = pEdit->GetText();
    return aText;
}

::rtl::OUString VCLXMultiLineEdit::getSelectedText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit)
        aText = pMultiLineEdit->GetSelected();
    return aText;

}

void VCLXMultiLineEdit::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit )
    {
        pMultiLineEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
    }
}

::com::sun::star::awt::Selection VCLXMultiLineEdit::getSelection() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Selection aSel;
    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit )
    {
        aSel.Min = pMultiLineEdit->GetSelection().Min();
        aSel.Max = pMultiLineEdit->GetSelection().Max();
    }
    return aSel;
}

sal_Bool VCLXMultiLineEdit::isEditable() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    return ( pMultiLineEdit && !pMultiLineEdit->IsReadOnly() && pMultiLineEdit->IsEnabled() ) ? sal_True : sal_False;
}

void VCLXMultiLineEdit::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetReadOnly( !bEditable );
}

void VCLXMultiLineEdit::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXMultiLineEdit::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    return pMultiLineEdit ? pMultiLineEdit->GetMaxTextLen() : 0;
}

::rtl::OUString VCLXMultiLineEdit::getTextLines() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
        aText = pEdit->GetTextLines();
    return aText;
}

::com::sun::star::awt::Size VCLXMultiLineEdit::getMinimumSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz;
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcMinimumSize());
    return aSz;
}

::com::sun::star::awt::Size VCLXMultiLineEdit::getPreferredSize() throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXMultiLineEdit::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz = rNewSize;
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcAdjustedSize( VCLSize(rNewSize )));
    return aSz;
}

::com::sun::star::awt::Size VCLXMultiLineEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz;
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcSize( nCols, nLines ));
    return aSz;
}

void VCLXMultiLineEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    nCols = nLines = 0;
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
    {
        sal_uInt16 nC, nL;
        pEdit->GetMaxVisColumnsAndLines( nC, nL );
        nCols = nC;
        nLines = nL;
    }
}

IMPL_LINK( VCLXMultiLineEdit, ModifyHdl, MultiLineEdit*, EMPTYARG )
{
    ::com::sun::star::awt::TextEvent aEvent;
    aEvent.Source = (::cppu::OWeakObject*)this;
    maTextListeners.textChanged( aEvent );

    return 1;
}

void VCLXMultiLineEdit::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*)GetWindow();
    if ( pMultiLineEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_READONLY:
            {
                sal_Bool b;
                if ( Value >>= b )
                    pMultiLineEdit->SetReadOnly( b );
            }
            break;
            case BASEPROPERTY_MAXTEXTLEN:
            {
                sal_Int16 n;
                if ( Value >>= n )
                    pMultiLineEdit->SetMaxTextLen( n );
            }
            break;
            case BASEPROPERTY_FOCUSSELECTIONHIDE:
            {
                sal_Bool b;
                if ( Value >>= b )
                    pMultiLineEdit->EnableFocusSelectionHide( b );
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXMultiLineEdit::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*)GetWindow();
    if ( pMultiLineEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_READONLY:
            {
                aProp <<= pMultiLineEdit->IsReadOnly();
            }
            break;
            case BASEPROPERTY_MAXTEXTLEN:
            {
                aProp <<= (sal_Int16) pMultiLineEdit->GetMaxTextLen();
            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//  ----------------------------------------------------
//  class VCLXFileDialog
//  ----------------------------------------------------
/*
VCLXFileDialog::VCLXFileDialog()
{
}

VCLXFileDialog::~VCLXFileDialog()
{
}

::com::sun::star::uno::Any VCLXFileDialog::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XXX*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFileDialog )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XXX>* ) NULL )
IMPL_XTYPEPROVIDER_END

void VCLXFileDialog::setPath( const ::rtl::OUString& rPath )
{
    ::vos::OGuard aGuard( GetMutex() );

    FileDialog* pDlg = (FileDialog*)GetWindow();
    if ( pDlg )
        pDlg->SetPath( ::rtl::OUStringToOString( rPath, CHARSET_SYSTEM ) );
}

::rtl::OUString VCLXFileDialog::getPath()
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aPath;
    FileDialog* pDlg = (FileDialog*)GetWindow();
    if ( pDlg )
        aPath = StringToOUString( pDlg->GetPath(), CHARSET_SYSTEM );
    return aPath;
}

void VCLXFileDialog::setFilters( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rFilterNames, const ::com::sun::star::uno::Sequence< ::rtl::OUString>& rMasks )
{
    ::vos::OGuard aGuard( GetMutex() );

    FileDialog* pDlg = (FileDialog*)GetWindow();
    if ( pDlg )
    {
        sal_uInt32 nFlts = rFilterNames.getLength();
        for ( sal_uInt32 n = 0; n < nFlts; n++ )
            pDlg->AddFilter(
                    ::rtl::OUStringToOString( rFilterNames.getConstArray()[n], CHARSET_SYSTEM ),
                    ::rtl::OUStringToOString( rMasks.getConstArray()[n], CHARSET_SYSTEM ) );
    }
}

void VCLXFileDialog::setCurrentFilter( const ::rtl::OUString& rFilterName )
{
    ::vos::OGuard aGuard( GetMutex() );

    FileDialog* pDlg = (FileDialog*)GetWindow();
    if ( pDlg )
        pDlg->SetCurFilter( ::rtl::OUStringToOString( rFilterName, CHARSET_SYSTEM ) );
}

::rtl::OUString VCLXFileDialog::getCurrentFilter()
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aFilter;
    FileDialog* pDlg = (FileDialog*)GetWindow();
    if ( pDlg )
        aFilter = StringToOUString( pDlg->GetCurFilter(), CHARSET_SYSTEM );
    return aFilter;
}
*/

//  ----------------------------------------------------
//  class VCLXFileControl
//  ----------------------------------------------------
VCLXFileControl::VCLXFileControl() : maTextListeners( *this )
{
}

VCLXFileControl::~VCLXFileControl()
{
    FileControl* pControl = (FileControl*) GetWindow();
    if ( pControl )
        pControl->GetEdit().SetModifyHdl( Link() );
}

::com::sun::star::uno::Any VCLXFileControl::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextComponent*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFileControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END


void VCLXFileControl::SetWindow( Window* pWindow )
{
    FileControl* pPrevFileControl = (FileControl*) GetWindow();
    if ( pPrevFileControl )
        pPrevFileControl->GetEdit().SetModifyHdl( Link() );

    FileControl* pNewFileControl = (FileControl*) pWindow;
    if ( pNewFileControl )
        pNewFileControl->GetEdit().SetModifyHdl( LINK( this, VCLXFileControl, ModifyHdl ) );

    VCLXWindow::SetWindow( pWindow );
}

void VCLXFileControl::addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    maTextListeners.addInterface( l );
}

void VCLXFileControl::removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    maTextListeners.removeInterface( l );
}

void VCLXFileControl::setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetText( aText );

        // In JAVA wird auch ein textChanged ausgeloest, in VCL nicht.
        // ::com::sun::star::awt::Toolkit soll JAVA-komform sein...
        ModifyHdl( NULL );
    }
}

void VCLXFileControl::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
    {
        pFileControl->GetEdit().SetSelection( Selection( rSel.Min, rSel.Max ) );
        pFileControl->GetEdit().ReplaceSelected( aText );
    }
}

::rtl::OUString VCLXFileControl::getText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

::rtl::OUString VCLXFileControl::getSelectedText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl)
        aText = pFileControl->GetEdit().GetSelected();
    return aText;

}

void VCLXFileControl::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
        pFileControl->GetEdit().SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

::com::sun::star::awt::Selection VCLXFileControl::getSelection() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Selection aSel;
    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
    {
        aSel.Min = pFileControl->GetEdit().GetSelection().Min();
        aSel.Max = pFileControl->GetEdit().GetSelection().Max();
    }
    return aSel;
}

sal_Bool VCLXFileControl::isEditable() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FileControl* pFileControl = (FileControl*) GetWindow();
    return ( pFileControl && !pFileControl->GetEdit().IsReadOnly() && pFileControl->GetEdit().IsEnabled() ) ? sal_True : sal_False;
}

void VCLXFileControl::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
        pFileControl->GetEdit().SetReadOnly( !bEditable );
}

void VCLXFileControl::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
        pFileControl->GetEdit().SetMaxTextLen( nLen );
}

sal_Int16 VCLXFileControl::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FileControl* pFileControl = (FileControl*) GetWindow();
    return pFileControl ? pFileControl->GetEdit().GetMaxTextLen() : 0;
}


IMPL_LINK( VCLXFileControl, ModifyHdl, Edit*, EMPTYARG )
{
    ::com::sun::star::awt::TextEvent aEvent;
    aEvent.Source = (::cppu::OWeakObject*)this;
    maTextListeners.textChanged( aEvent );

    return 1;
}

::com::sun::star::awt::Size VCLXFileControl::getMinimumSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz;
    FileControl* pControl = (FileControl*) GetWindow();
    if ( pControl )
    {
        Size aTmpSize = pControl->GetEdit().CalcMinimumSize();
        aTmpSize.Width() += pControl->GetButton().CalcMinimumSize().Width();
        aSz = AWTSize(pControl->CalcWindowSize( aTmpSize ));
    }
    return aSz;
}

::com::sun::star::awt::Size VCLXFileControl::getPreferredSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Size aSz = getMinimumSize();
    aSz.Height += 4;
    return aSz;
}

::com::sun::star::awt::Size VCLXFileControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz =rNewSize;
    FileControl* pControl = (FileControl*) GetWindow();
    if ( pControl )
    {
        ::com::sun::star::awt::Size aMinSz = getMinimumSize();
        if ( aSz.Height != aMinSz.Height )
            aSz.Height = aMinSz.Height;
    }
    return aSz;
}

::com::sun::star::awt::Size VCLXFileControl::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz;
    FileControl* pControl = (FileControl*) GetWindow();
    if ( pControl )
    {
        aSz = AWTSize(pControl->GetEdit().CalcSize( nCols ));
        aSz.Width += pControl->GetButton().CalcMinimumSize().Width();
    }
    return aSz;
}

void VCLXFileControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    nCols = 0;
    nLines = 1;
    FileControl* pControl = (FileControl*) GetWindow();
    if ( pControl )
        nCols = (sal_Int16) pControl->GetEdit().GetMaxVisChars();
}

//  ----------------------------------------------------
//  class SVTXFormattedField
//  ----------------------------------------------------
// --------------------------------------------------------------------------------------
SVTXFormattedField::SVTXFormattedField()
    :m_pCurrentSupplier(NULL)
    ,bIsStandardSupplier(sal_True)
    ,nKeyToSetDelayed(-1)
{
}

// --------------------------------------------------------------------------------------
SVTXFormattedField::~SVTXFormattedField()
{
    if (m_pCurrentSupplier)
    {
        m_pCurrentSupplier->release();
        m_pCurrentSupplier = NULL;
    }
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::SetWindow( Window* _pWindow )
{
    VCLXSpinField::SetWindow(_pWindow);
    if (GetFormattedField())
        GetFormattedField()->SetAutoColor(TRUE);
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_EFFECTIVE_MIN:
            case BASEPROPERTY_VALUEMIN_DOUBLE:
                SetMinValue(Value);
                break;

            case BASEPROPERTY_EFFECTIVE_MAX:
            case BASEPROPERTY_VALUEMAX_DOUBLE:
                SetMaxValue(Value);
                break;

            case BASEPROPERTY_EFFECTIVE_DEFAULT:
                SetDefaultValue(Value);
                break;

            case BASEPROPERTY_TREATASNUMBER:
                sal_Bool b;
                if ( Value >>= b )
                    SetTreatAsNumber(b);
                break;

            case BASEPROPERTY_FORMATSSUPPLIER:
                if (!Value.hasValue())
                    setFormatsSupplier(::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > (NULL));
                else
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > xNFS;
                    if ( Value >>= xNFS )
                        setFormatsSupplier(xNFS);
                }
                break;
            case BASEPROPERTY_FORMATKEY:
                if (!Value.hasValue())
                    setFormatKey(0);
                else
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                        setFormatKey(n);
                }
                break;

            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                const ::com::sun::star::uno::TypeClass rTC = Value.getValueType().getTypeClass();
                if ((rTC != ::com::sun::star::uno::TypeClass_VOID) && (rTC != ::com::sun::star::uno::TypeClass_STRING) && (rTC != ::com::sun::star::uno::TypeClass_DOUBLE))
                    throw ::com::sun::star::lang::IllegalArgumentException();
                SetValue(Value);
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                double d;
                if ( Value >>= d )
                     pField->SetSpinSize( d );
                else
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                         pField->SetSpinSize( n );
                }
            }
            break;
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int32 n;
                if ( Value >>= n )
                     pField->SetDecimalDigits( n );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                    sal_Bool b;
                    if ( Value >>= b )
                     pField->SetThousandsSep( b );
            }
            break;

            default:
                VCLXSpinField::setProperty( PropertyName, Value );
        }
    }
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXFormattedField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aReturn;

    FormattedField* pField = GetFormattedField();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_EFFECTIVE_MIN:
            case BASEPROPERTY_VALUEMIN_DOUBLE:
                aReturn <<= GetMinValue();
                break;

            case BASEPROPERTY_EFFECTIVE_MAX:
            case BASEPROPERTY_VALUEMAX_DOUBLE:
                aReturn <<= GetMaxValue();
                break;

            case BASEPROPERTY_EFFECTIVE_DEFAULT:
                aReturn <<= GetDefaultValue();
                break;

            case BASEPROPERTY_TREATASNUMBER:
                aReturn <<= GetTreatAsNumber();
                break;

            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_VALUE_DOUBLE:
                aReturn <<= GetValue();
                break;

            case BASEPROPERTY_VALUESTEP_DOUBLE:
                aReturn <<= pField->GetSpinSize();
                break;

            case BASEPROPERTY_DECIMALACCURACY:
                aReturn <<= pField->GetDecimalDigits();
                break;

            case BASEPROPERTY_FORMATSSUPPLIER:
            {
                if (!bIsStandardSupplier)
                {   // ansonsten void
                    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = getFormatsSupplier();
                    aReturn <<= xSupplier;
                }
            }
            break;

            case BASEPROPERTY_FORMATKEY:
            {
                if (!bIsStandardSupplier)
                    aReturn <<= getFormatKey();
            }
            break;

            default:
                aReturn <<= VCLXSpinField::getProperty(PropertyName);
        }
    }
    return aReturn;
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXFormattedField::convertEffectiveValue(const ::com::sun::star::uno::Any& rValue)
{
    ::com::sun::star::uno::Any aReturn;

    FormattedField* pField = GetFormattedField();
    if (!pField)
        return aReturn;

    switch (rValue.getValueType().getTypeClass())
    {
        case ::com::sun::star::uno::TypeClass_DOUBLE:
            if (pField->TreatingAsNumber())
            {
                double d;
                rValue >>= d;
                aReturn <<= d;
            }
            else
            {
                SvNumberFormatter* pFormatter = pField->GetFormatter();
                if (!pFormatter)
                    pFormatter = pField->StandardFormatter();
                    // should never fail

                Color* pDummy;
                double d;
                rValue >>= d;
                String sConverted;
                pFormatter->GetOutputString(d, 0, sConverted, &pDummy);
                aReturn <<= ::rtl::OUString( sConverted );
            }
            break;
        case ::com::sun::star::uno::TypeClass_STRING:
        {
            ::rtl::OUString aStr;
            rValue >>= aStr;
            String sValue = aStr;
            if (pField->TreatingAsNumber())
            {
                SvNumberFormatter* pFormatter = pField->GetFormatter();
                if (!pFormatter)
                    pFormatter = pField->StandardFormatter();

                double dVal;
                sal_uInt32 nTestFormat(0);
                if (!pFormatter->IsNumberFormat(sValue, nTestFormat, dVal))
                    aReturn.clear();
                aReturn <<=dVal;
            }
            else
                aReturn <<= aStr;
        }
        break;
        default:
            aReturn.clear();
            break;
    }
    return aReturn;
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::SetMinValue(const ::com::sun::star::uno::Any& rValue)
{
    FormattedField* pField = GetFormattedField();
    if (!pField)
        return;

    switch (rValue.getValueType().getTypeClass())

    {
        case ::com::sun::star::uno::TypeClass_DOUBLE:
            double d;
            rValue >>= d;
            pField->SetMinValue(d);
            break;
        default:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID, "SVTXFormattedField::SetMinValue : invalid argument (an exception will be thrown) !");
            if ( rValue.getValueType().getTypeClass() != ::com::sun::star::uno::TypeClass_VOID )

            {
                throw ::com::sun::star::lang::IllegalArgumentException();
            }
            pField->ClearMinValue();
            break;
    }
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXFormattedField::GetMinValue()
{
    FormattedField* pField = GetFormattedField();
    if (!pField || !pField->HasMinValue())
        return ::com::sun::star::uno::Any();

    ::com::sun::star::uno::Any aReturn;
    aReturn <<= pField->GetMinValue();
    return aReturn;
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::SetMaxValue(const ::com::sun::star::uno::Any& rValue)
{
    FormattedField* pField = GetFormattedField();
    if (!pField)
        return;

    switch (rValue.getValueType().getTypeClass())

    {
        case ::com::sun::star::uno::TypeClass_DOUBLE:
            double d;
            rValue >>= d;
            pField->SetMaxValue(d);
            break;
        default:
            if (rValue.getValueType().getTypeClass() != ::com::sun::star::uno::TypeClass_VOID)

            {
                throw ::com::sun::star::lang::IllegalArgumentException();
            }
            pField->ClearMaxValue();
            break;
    }
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXFormattedField::GetMaxValue()
{
    FormattedField* pField = GetFormattedField();
    if (!pField || !pField->HasMaxValue())
        return ::com::sun::star::uno::Any();

    ::com::sun::star::uno::Any aReturn;
    aReturn <<= pField->GetMaxValue();
    return aReturn;
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::SetDefaultValue(const ::com::sun::star::uno::Any& rValue)
{
    FormattedField* pField = GetFormattedField();
    if (!pField)
        return;

    ::com::sun::star::uno::Any aConverted = convertEffectiveValue(rValue);

    switch (aConverted.getValueType().getTypeClass())

    {
        case ::com::sun::star::uno::TypeClass_DOUBLE:
        {
            double d;
            aConverted >>= d;
            pField->SetDefaultValue(d);
        }
        break;
        case ::com::sun::star::uno::TypeClass_STRING:
        {
            ::rtl::OUString aStr;
            aConverted >>= aStr;
            pField->SetDefaultText( aStr );
        }
        break;
        default:
            pField->EnableEmptyField(sal_True);
                // nur noch void erlaubt
            break;
    }
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXFormattedField::GetDefaultValue()
{
    FormattedField* pField = GetFormattedField();
    if (!pField || pField->IsEmptyFieldEnabled())
        return ::com::sun::star::uno::Any();

    ::com::sun::star::uno::Any aReturn;
    if (pField->TreatingAsNumber())
        aReturn <<= pField->GetDefaultValue();
    else
        aReturn <<= ::rtl::OUString( pField->GetDefaultText() );
    return aReturn;
}

// --------------------------------------------------------------------------------------
sal_Bool SVTXFormattedField::GetTreatAsNumber()
{
    FormattedField* pField = GetFormattedField();
    if (pField)
        return pField->TreatingAsNumber();

    return sal_True;
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::SetTreatAsNumber(sal_Bool bSet)
{
    FormattedField* pField = GetFormattedField();
    if (pField)
        pField->TreatAsNumber(bSet);
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXFormattedField::GetValue()
{
    FormattedField* pField = GetFormattedField();
    if (!pField)
        return ::com::sun::star::uno::Any();

    ::com::sun::star::uno::Any aReturn;
    if (!pField->TreatingAsNumber())
    {
        ::rtl::OUString sText = pField->GetTextValue();
        aReturn <<= sText;
    }
    else
    {
        if (pField->GetText().Len())    // empty wird erst mal standardmaessig als void nach draussen gereicht
            aReturn <<= pField->GetValue();
    }

    return aReturn;
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::SetValue(const ::com::sun::star::uno::Any& rValue)
{
    FormattedField* pField = GetFormattedField();
    if (!pField)
        return;

    if (!rValue.hasValue())
    {
        pField->SetText(String());
    }
    else
    {
        if (rValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_DOUBLE )
        {
            double d;
            rValue >>= d;
            pField->SetValue(d);
        }
        else
        {
            DBG_ASSERT(rValue.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_STRING, "SVTXFormattedField::SetValue : invalid argument !");

            ::rtl::OUString sText;
            rValue >>= sText;
            String aStr( sText );
            if (!pField->TreatingAsNumber())
                pField->SetTextFormatted(aStr);
            else
                pField->SetTextValue(aStr);
        }
    }
//  NotifyTextListeners();
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  SVTXFormattedField::getFormatsSupplier(void) const
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > ((::com::sun::star::util::XNumberFormatsSupplier*)m_pCurrentSupplier);
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::setFormatsSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xSupplier)
{
    FormattedField* pField = GetFormattedField();

    SvNumberFormatsSupplierObj* pNew = NULL;
    if (!xSupplier.is())
    {
        if (pField)
        {
            pNew = new SvNumberFormatsSupplierObj(pField->StandardFormatter());
            bIsStandardSupplier = sal_True;
        }
    }
    else
    {
        pNew = SvNumberFormatsSupplierObj::getImplementation(xSupplier);
        bIsStandardSupplier = sal_False;
    }

    if (!pNew)
        return;     // TODO : wie das behandeln ?

    if (m_pCurrentSupplier)
        m_pCurrentSupplier->release();
    m_pCurrentSupplier = pNew;
    m_pCurrentSupplier->acquire();

    if (pField)
    {
        // den aktuellen Value mit hinueberretten
        ::com::sun::star::uno::Any aCurrent = GetValue();
        pField->SetFormatter(m_pCurrentSupplier->GetNumberFormatter(), sal_False);
        if (nKeyToSetDelayed != -1)
        {
            pField->SetFormatKey(nKeyToSetDelayed);
            nKeyToSetDelayed = -1;
        }
        SetValue(aCurrent);
        NotifyTextListeners();
    }
}

// --------------------------------------------------------------------------------------
sal_Int32 SVTXFormattedField::getFormatKey(void) const
{
    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetFormatKey() : 0;
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::setFormatKey(sal_Int32 nKey)
{
    FormattedField* pField = GetFormattedField();
    if (pField)
    {
        if (pField->GetFormatter())
            pField->SetFormatKey(nKey);
        else
        {   // Wahrscheinlich bin ich gerade in einem Block, in dem erst der Key und dann der Formatter gesetzt
            // wird, das passiert initial mit ziemlicher Sicherheit, da die Properties in alphabetischer Reihenfolge
            // gesetzt werden, und der FormatsSupplier nun mal vor dem FormatKey kommt
            nKeyToSetDelayed = nKey;
        }
        NotifyTextListeners();
    }
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::NotifyTextListeners()
{
    if ( GetTextListeners().getLength() )
    {
        ::com::sun::star::awt::TextEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        GetTextListeners().textChanged( aEvent );
    }
}

// ----------------------------------------------------
//  class SVTXNumericField
//  ----------------------------------------------------
SVTXNumericField::SVTXNumericField()
{
}

SVTXNumericField::~SVTXNumericField()
{
}

::com::sun::star::uno::Any SVTXNumericField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XNumericField*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( SVTXNumericField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XNumericField>* ) NULL ),
    SVTXFormattedField::getTypes()
IMPL_XTYPEPROVIDER_END


void SVTXNumericField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetValue( Value );
}

double SVTXNumericField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetValue() : 0;
}

void SVTXNumericField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMinValue( Value );
}

double SVTXNumericField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMinValue() : 0;
}

void SVTXNumericField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMaxValue( Value );
}

double SVTXNumericField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMaxValue() : 0;
}

void SVTXNumericField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinFirst( Value );
}

double SVTXNumericField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinFirst() : 0;
}

void SVTXNumericField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinLast( Value );
}

double SVTXNumericField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinLast() : 0;
}

void SVTXNumericField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinSize( Value );
}

double SVTXNumericField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinSize() : 0;
}

void SVTXNumericField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetDecimalDigits( Value );
}

sal_Int16 SVTXNumericField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetDecimalDigits() : 0;
}

void SVTXNumericField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetStrictFormat( bStrict );
}

sal_Bool SVTXNumericField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->IsStrictFormat() : sal_False;
}


// ----------------------------------------------------
//  class SVTXCurrencyField
//  ----------------------------------------------------
SVTXCurrencyField::SVTXCurrencyField()
{
}

SVTXCurrencyField::~SVTXCurrencyField()
{
}

::com::sun::star::uno::Any SVTXCurrencyField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XCurrencyField*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( SVTXCurrencyField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCurrencyField>* ) NULL ),
    SVTXFormattedField::getTypes()
IMPL_XTYPEPROVIDER_END

void SVTXCurrencyField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetValue( Value );
}

double SVTXCurrencyField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetValue() : 0;
}

void SVTXCurrencyField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMinValue( Value );
}

double SVTXCurrencyField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMinValue() : 0;
}

void SVTXCurrencyField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMaxValue( Value );
}

double SVTXCurrencyField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMaxValue() : 0;
}

void SVTXCurrencyField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinFirst( Value );
}

double SVTXCurrencyField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinFirst() : 0;
}

void SVTXCurrencyField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinLast( Value );
}

double SVTXCurrencyField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinLast() : 0;
}

void SVTXCurrencyField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinSize( Value );
}

double SVTXCurrencyField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinSize() : 0;
}

void SVTXCurrencyField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetDecimalDigits( Value );
}

sal_Int16 SVTXCurrencyField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetDecimalDigits() : 0;
}

void SVTXCurrencyField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetStrictFormat( bStrict );
}

sal_Bool SVTXCurrencyField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormattedField* pField = GetFormattedField();
    return pField ? pField->IsStrictFormat() : sal_False;
}

void SVTXCurrencyField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aReturn;

    DoubleCurrencyField* pField = (DoubleCurrencyField*)GetFormattedField();
    if ( pField )
    {
#ifdef DBG_UTIL
        String sAssertion( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "SVTXCurrencyField::setProperty(" ) ) );
        sAssertion += String( PropertyName );
        sAssertion.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ") : invalid value !" ) );
#endif
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                ::rtl::OUString aStr;
                Value >>= aStr;
                pField->setCurrencySymbol( aStr );
            }
            break;
            case BASEPROPERTY_CURSYM_POSITION:
            {
                sal_Bool b;
                Value >>= b;
                pField->setPrependCurrSym(b);
            }
            break;

            default:
                SVTXFormattedField::setProperty(PropertyName, Value);
        }
    }
    else
        SVTXFormattedField::setProperty(PropertyName, Value);
}

::com::sun::star::uno::Any SVTXCurrencyField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aReturn;

    DoubleCurrencyField* pField = (DoubleCurrencyField*)GetFormattedField();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                aReturn <<= ::rtl::OUString( pField->getCurrencySymbol() );
            }
            break;
            case BASEPROPERTY_CURSYM_POSITION:
            {
                aReturn <<= pField->getPrependCurrSym();
            }
            break;
            default:
                return SVTXFormattedField::getProperty(PropertyName);
        }
    }
    return SVTXFormattedField::getProperty(PropertyName);
}

