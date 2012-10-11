/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <svtools/svmedit.hxx>
#include <unoiface.hxx>
#include <svtools/filectrl.hxx>
#include <svtools/roadmap.hxx>
#include <svtools/fixedhyper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/property.hxx>
#include <svtools/fmtfield.hxx>
#include <svl/numuno.hxx>
#include <svtools/calendar.hxx>
#include <svtools/prgsbar.hxx>
#include <svtools/treelistbox.hxx>
#include "treecontrolpeer.hxx"
#include "svtxgridcontrol.hxx"
#include <svtools/table/tablecontrol.hxx>

namespace
{
    static void lcl_setWinBits( Window* _pWindow, WinBits _nBits, sal_Bool _bSet )
    {
        WinBits nStyle = _pWindow->GetStyle();
        if ( _bSet )
            nStyle |= _nBits;
        else
            nStyle &= ~_nBits;
        _pWindow->SetStyle( nStyle );
    }
}

//  ----------------------------------------------------
//  help function for the toolkit...
//  ----------------------------------------------------

extern "C" {

SAL_DLLPUBLIC_EXPORT Window* CreateWindow( VCLXWindow** ppNewComp, const ::com::sun::star::awt::WindowDescriptor* pDescriptor, Window* pParent, WinBits nWinBits )
{
    Window* pWindow = NULL;
    String aServiceName( pDescriptor->WindowServiceName );
    if ( aServiceName.EqualsIgnoreCaseAscii( "MultiLineEdit" ) )
    {
        if ( pParent )
        {
            pWindow = new MultiLineEdit( pParent, nWinBits|WB_IGNORETAB);
            static_cast< MultiLineEdit* >( pWindow )->DisableSelectionOnFocus();
            *ppNewComp = new VCLXMultiLineEdit;
        }
        else
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }
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
        pWindow = new FormattedField( pParent, nWinBits );
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
        static_cast<CalendarField*>(pWindow)->EnableEmptyFieldValue( sal_True );
        *ppNewComp = new SVTXDateField;
        ((VCLXFormattedSpinField*)*ppNewComp)->SetFormatter( (FormatterBase*)(DateField*)pWindow );
    }
    else if (aServiceName.EqualsIgnoreCaseAscii("roadmap") )
    {
        pWindow = new ::svt::ORoadmap( pParent, WB_TABSTOP );
        *ppNewComp = new SVTXRoadmap;
    }
    else if ( aServiceName.EqualsIgnoreCaseAscii( "ProgressBar" ) )
    {
        if ( pParent )
        {
            pWindow = new ProgressBar( pParent, nWinBits );
            *ppNewComp = new VCLXProgressBar;
        }
        else
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }
    else if ( aServiceName.EqualsIgnoreCaseAscii( "Tree" ) )
    {
        TreeControlPeer* pPeer = new TreeControlPeer;
        *ppNewComp = pPeer;
        pWindow = pPeer->createVclControl( pParent, nWinBits );
    }
    else if ( aServiceName.EqualsIgnoreCaseAscii( "FixedHyperlink" ) )
    {
        if ( pParent )
        {
            pWindow = new ::svt::FixedHyperlink( pParent, nWinBits );
            *ppNewComp = new VCLXFixedHyperlink;
        }
        else
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }
    else if ( aServiceName.EqualsIgnoreCaseAscii( "Grid" ) )
    {
        if ( pParent )
        {
            pWindow = new ::svt::table::TableControl(pParent, nWinBits);
            *ppNewComp = new SVTXGridControl;
        }
        else
        {
            *ppNewComp = NULL;
            return NULL;
        }
    }
    return pWindow;
}

}   // extern "C"

//  ----------------------------------------------------
//  class VCLXMultiLineEdit
//  ----------------------------------------------------
VCLXMultiLineEdit::VCLXMultiLineEdit()
    :maTextListeners( *this )
    ,meLineEndType( LINEEND_LF )    // default behavior before introducing this property: LF (unix-like)
{
}

VCLXMultiLineEdit::~VCLXMultiLineEdit()
{
}

::com::sun::star::uno::Any VCLXMultiLineEdit::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XTextComponent* >(this)),
                                        (static_cast< ::com::sun::star::awt::XTextArea* >(this)),
                                        (static_cast< ::com::sun::star::awt::XTextLayoutConstrains* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMultiLineEdit )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextArea>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

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
    SolarMutexGuard aGuard;

    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
    {
        pEdit->SetText( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void VCLXMultiLineEdit::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
    {
        setSelection( rSel );
        pEdit->ReplaceSelected( aText );
    }
}

::rtl::OUString VCLXMultiLineEdit::getText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
        aText = pEdit->GetText( meLineEndType );
    return aText;
}

::rtl::OUString VCLXMultiLineEdit::getSelectedText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit)
        aText = pMultiLineEdit->GetSelected( meLineEndType );
    return aText;

}

void VCLXMultiLineEdit::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit )
    {
        pMultiLineEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
    }
}

::com::sun::star::awt::Selection VCLXMultiLineEdit::getSelection() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    return ( pMultiLineEdit && !pMultiLineEdit->IsReadOnly() && pMultiLineEdit->IsEnabled() ) ? sal_True : sal_False;
}

void VCLXMultiLineEdit::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetReadOnly( !bEditable );
}

void VCLXMultiLineEdit::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXMultiLineEdit::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*) GetWindow();
    return pMultiLineEdit ? (sal_Int16)pMultiLineEdit->GetMaxTextLen() : (sal_Int16)0;
}

::rtl::OUString VCLXMultiLineEdit::getTextLines() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    MultiLineEdit* pEdit = (MultiLineEdit*)GetWindow();
    if ( pEdit )
        aText = pEdit->GetTextLines( meLineEndType );
    return aText;
}

::com::sun::star::awt::Size VCLXMultiLineEdit::getMinimumSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz = rNewSize;
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcAdjustedSize( VCLSize(rNewSize )));
    return aSz;
}

::com::sun::star::awt::Size VCLXMultiLineEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::awt::Size aSz;
    MultiLineEdit* pEdit = (MultiLineEdit*) GetWindow();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcSize( nCols, nLines ));
    return aSz;
}

void VCLXMultiLineEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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

void VCLXMultiLineEdit::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_EDIT_MODIFY:
        {
            if ( maTextListeners.getLength() )
            {
                ::com::sun::star::awt::TextEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                maTextListeners.textChanged( aEvent );
            }
        }
        break;
        default:
        {
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
        }
        break;
    }
}

void VCLXMultiLineEdit::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*)GetWindow();
    if ( pMultiLineEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINE_END_FORMAT:
            {
                sal_Int16 nLineEndType = ::com::sun::star::awt::LineEndFormat::LINE_FEED;
                OSL_VERIFY( Value >>= nLineEndType );
                switch ( nLineEndType )
                {
                case ::com::sun::star::awt::LineEndFormat::CARRIAGE_RETURN:           meLineEndType = LINEEND_CR; break;
                case ::com::sun::star::awt::LineEndFormat::LINE_FEED:                 meLineEndType = LINEEND_LF; break;
                case ::com::sun::star::awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED: meLineEndType = LINEEND_CRLF; break;
                default: OSL_FAIL( "VCLXMultiLineEdit::setProperty: invalid line end value!" ); break;
                }
            }
            break;

            case BASEPROPERTY_READONLY:
            {
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                    pMultiLineEdit->SetReadOnly( b );
            }
            break;
            case BASEPROPERTY_MAXTEXTLEN:
            {
                sal_Int16 n = sal_Int16();
                if ( Value >>= n )
                    pMultiLineEdit->SetMaxTextLen( n );
            }
            break;
            case BASEPROPERTY_HIDEINACTIVESELECTION:
            {
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                {
                    pMultiLineEdit->EnableFocusSelectionHide( b );
                    lcl_setWinBits( pMultiLineEdit, WB_NOHIDESELECTION, !b );
                }
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
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    MultiLineEdit* pMultiLineEdit = (MultiLineEdit*)GetWindow();
    if ( pMultiLineEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINE_END_FORMAT:
            {
                sal_Int16 nLineEndType = ::com::sun::star::awt::LineEndFormat::LINE_FEED;
                switch ( meLineEndType )
                {
                case LINEEND_CR:   nLineEndType = ::com::sun::star::awt::LineEndFormat::CARRIAGE_RETURN; break;
                case LINEEND_LF:   nLineEndType = ::com::sun::star::awt::LineEndFormat::LINE_FEED; break;
                case LINEEND_CRLF: nLineEndType = ::com::sun::star::awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED; break;
                default: OSL_FAIL( "VCLXMultiLineEdit::getProperty: invalid line end value!" ); break;
                }
                aProp <<= nLineEndType;
            }
            break;

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

void SAL_CALL VCLXMultiLineEdit::setFocus(  ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // don't grab the focus if we already have it. Reason is that the only thing which the edit
    // does is forwarding the focus to it's text window. This text window then does a "select all".
    // So if the text window already has the focus, and we give the focus to the multi line
    // edit, then all which happens is that everything is selected.
    // #i27072#
    if ( GetWindow() && !GetWindow()->HasChildPathFocus() )
        GetWindow()->GrabFocus();
}

void VCLXMultiLineEdit::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     // FIXME: elide duplication ?
                     BASEPROPERTY_LINE_END_FORMAT,
                     BASEPROPERTY_READONLY,
                     BASEPROPERTY_MAXTEXTLEN,
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );

}
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
                                        (static_cast< ::com::sun::star::awt::XTextComponent* >(this)),
                                        (static_cast< ::com::sun::star::awt::XTextLayoutConstrains* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFileControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void SAL_CALL VCLXFileControl::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FileControl* pControl = (FileControl*)GetWindow();
    if ( pControl )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
        case BASEPROPERTY_HIDEINACTIVESELECTION:
        {
            sal_Bool bValue( sal_False );
            OSL_VERIFY( Value >>= bValue );

            lcl_setWinBits( pControl, WB_NOHIDESELECTION, !bValue );
            lcl_setWinBits( &pControl->GetEdit(), WB_NOHIDESELECTION, !bValue );
        }
        break;

        default:
            VCLXWindow::setProperty( PropertyName, Value );
            break;
        }
    }
}

void VCLXFileControl::SetWindow( Window* pWindow )
{
    FileControl* pPrevFileControl = dynamic_cast<FileControl*>( GetWindow() );
    if ( pPrevFileControl )
        pPrevFileControl->GetEdit().SetModifyHdl( Link() );

    FileControl* pNewFileControl = dynamic_cast<FileControl*>( pWindow );
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
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
    {
        pFileControl->GetEdit().SetSelection( Selection( rSel.Min, rSel.Max ) );
        pFileControl->GetEdit().ReplaceSelected( aText );
    }
}

::rtl::OUString VCLXFileControl::getText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

::rtl::OUString VCLXFileControl::getSelectedText() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString aText;
    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl)
        aText = pFileControl->GetEdit().GetSelected();
    return aText;

}

void VCLXFileControl::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
        pFileControl->GetEdit().SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

::com::sun::star::awt::Selection VCLXFileControl::getSelection() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

    FileControl* pFileControl = (FileControl*) GetWindow();
    return ( pFileControl && !pFileControl->GetEdit().IsReadOnly() && pFileControl->GetEdit().IsEnabled() ) ? sal_True : sal_False;
}

void VCLXFileControl::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
        pFileControl->GetEdit().SetReadOnly( !bEditable );
}

void VCLXFileControl::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FileControl* pFileControl = (FileControl*) GetWindow();
    if ( pFileControl )
        pFileControl->GetEdit().SetMaxTextLen( nLen );
}

sal_Int16 VCLXFileControl::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FileControl* pFileControl = (FileControl*) GetWindow();
    return pFileControl ? pFileControl->GetEdit().GetMaxTextLen() : 0;
}


IMPL_LINK_NOARG(VCLXFileControl, ModifyHdl)
{
    ::com::sun::star::awt::TextEvent aEvent;
    aEvent.Source = (::cppu::OWeakObject*)this;
    maTextListeners.textChanged( aEvent );

    return 1;
}

::com::sun::star::awt::Size VCLXFileControl::getMinimumSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

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

::com::sun::star::awt::Size VCLXFileControl::getMinimumSize( sal_Int16 nCols, sal_Int16 ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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
    SolarMutexGuard aGuard;

    nCols = 0;
    nLines = 1;
    FileControl* pControl = (FileControl*) GetWindow();
    if ( pControl )
        nCols = (sal_Int16) pControl->GetEdit().GetMaxVisChars();
}

void VCLXFileControl::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     // FIXME: elide duplication ?
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
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
        GetFormattedField()->SetAutoColor(sal_True);
}

// --------------------------------------------------------------------------------------
void SVTXFormattedField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                sal_Bool bEnable( sal_True );
                if ( Value >>= bEnable )
                    pField->EnableNotANumber( !bEnable );
            }
            break;

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
            {
                sal_Bool b = sal_Bool();
                if ( Value >>= b )
                    SetTreatAsNumber(b);
            }
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
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                        setFormatKey(n);
                }
                break;

            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                const ::com::sun::star::uno::TypeClass rTC = Value.getValueType().getTypeClass();
                if (rTC != ::com::sun::star::uno::TypeClass_STRING)
                    // no string
                    if (rTC != ::com::sun::star::uno::TypeClass_DOUBLE)
                        // no double
                        if (Value.hasValue())
                        {   // but a value
                            // try if it is something converitble
                            sal_Int32 nValue = 0;
                            if (!(Value >>= nValue))
                                throw ::com::sun::star::lang::IllegalArgumentException();
                            SetValue(::com::sun::star::uno::makeAny((double)nValue));
                            break;
                        }

                SetValue(Value);
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                double d = 0.0;
                if ( Value >>= d )
                     pField->SetSpinSize( d );
                else
                {
                    sal_Int32 n = 0;
                    if ( Value >>= n )
                         pField->SetSpinSize( n );
                }
            }
            break;
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int32 n = 0;
                if ( Value >>= n )
                     pField->SetDecimalDigits( (sal_uInt16)n );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                    sal_Bool b = sal_Bool();
                    if ( Value >>= b )
                     pField->SetThousandsSep( b );
            }
            break;

            default:
                VCLXSpinField::setProperty( PropertyName, Value );
        }

        if (BASEPROPERTY_TEXTCOLOR == nPropType)
        {   // after setting a new text color, think again about the AutoColor flag of the control
            // 17.05.2001 - 86859 - frank.schoenheit@germany.sun.com
            pField->SetAutoColor(!Value.hasValue());
        }
    }
    else
        VCLXSpinField::setProperty( PropertyName, Value );
}

// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXFormattedField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

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
                double d = 0.0;
                rValue >>= d;
                aReturn <<= d;
            }
            else
            {
                SvNumberFormatter* pFormatter = pField->GetFormatter();
                if (!pFormatter)
                    pFormatter = pField->StandardFormatter();
                    // should never fail

                Color* pDum;
                double d = 0.0;
                rValue >>= d;
                String sConverted;
                pFormatter->GetOutputString(d, 0, sConverted, &pDum);
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
        {
            double d = 0.0;
            rValue >>= d;
            pField->SetMinValue(d);
        }
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
        {
            double d = 0.0;
            rValue >>= d;
            pField->SetMaxValue(d);
        }
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
            double d = 0.0;
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
            double d = 0.0;
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

void SVTXFormattedField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     // FIXME: elide duplication ?
                     BASEPROPERTY_EFFECTIVE_MIN,
                     BASEPROPERTY_VALUEMIN_DOUBLE,
                     BASEPROPERTY_EFFECTIVE_MAX,
                     BASEPROPERTY_VALUEMAX_DOUBLE,
                     BASEPROPERTY_EFFECTIVE_DEFAULT,
                     BASEPROPERTY_TREATASNUMBER,
                     BASEPROPERTY_EFFECTIVE_VALUE,
                     BASEPROPERTY_VALUE_DOUBLE,
                     BASEPROPERTY_VALUESTEP_DOUBLE,
                     BASEPROPERTY_DECIMALACCURACY,
                     BASEPROPERTY_FORMATSSUPPLIER,
                     BASEPROPERTY_NUMSHOWTHOUSANDSEP,
                     BASEPROPERTY_FORMATKEY,
                     BASEPROPERTY_TREATASNUMBER,
                     BASEPROPERTY_ENFORCE_FORMAT,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
    VCLXSpinField::ImplGetPropertyIds( rIds );
}


//  ----------------------------------------------------
//  class SVTXRoadmap
//  ----------------------------------------------------

using namespace svt;

// --------------------------------------------------------------------------------------
SVTXRoadmap::SVTXRoadmap() : maItemListeners( *this )
{
}

// --------------------------------------------------------------------------------------
SVTXRoadmap::~SVTXRoadmap()
{
}

void SVTXRoadmap::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_ROADMAP_ITEMSELECTED:
        {
            SolarMutexGuard aGuard;
            ::svt::ORoadmap* pField = GetRoadmap();
            if ( pField )
            {
                sal_Int16 CurItemID = pField->GetCurrentRoadmapItemID();
                ::com::sun::star::awt::ItemEvent aEvent;
                aEvent.Selected = CurItemID;
                aEvent.Highlighted = CurItemID;
                aEvent.ItemId = CurItemID;
                maItemListeners.itemStateChanged( aEvent );
            }
        }
        break;
        default:
            SVTXRoadmap_Base::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}


void SVTXRoadmap::propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ::svt::ORoadmap* pField = GetRoadmap();
    if ( pField )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xRoadmapItem;
        xRoadmapItem = evt.Source;
        sal_Int32 nID = 0;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropertySet( xRoadmapItem, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Any aValue = xPropertySet->getPropertyValue(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ID" )));
        aValue >>= nID;

        ::com::sun::star::uno::Any rVal = evt.NewValue;
        evt.NewValue >>= rVal;
        ::rtl::OUString sPropertyName = evt.PropertyName;
        if ( sPropertyName == "Enabled" )
        {
            sal_Bool bEnable = false;
            evt.NewValue >>= bEnable;
            pField->EnableRoadmapItem( (RoadmapTypes::ItemId)nID , bEnable );
        }
        else if ( sPropertyName == "Label" )
        {
            ::rtl::OUString sLabel;
            evt.NewValue >>= sLabel;
            pField->ChangeRoadmapItemLabel( (RoadmapTypes::ItemId)nID , sLabel );
        }
        else if  ( sPropertyName == "ID" )
        {
            sal_Int32 nNewID = 0;
            evt.NewValue >>= nNewID;
            evt.OldValue >>= nID;
            pField->ChangeRoadmapItemID( (RoadmapTypes::ItemId)nID, (RoadmapTypes::ItemId)nNewID );
        }
    //    else
            // Todo: handle Interactive appropriately
    }
}


void SVTXRoadmap::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void SVTXRoadmap::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener >& l ) throw (::com::sun::star::uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

RMItemData SVTXRoadmap::GetRMItemData( const ::com::sun::star::container::ContainerEvent& _rEvent )
{
    RMItemData aCurRMItemData;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xRoadmapItem;
    _rEvent.Element >>= xRoadmapItem;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropertySet( xRoadmapItem, ::com::sun::star::uno::UNO_QUERY );
    if ( xPropertySet.is() )
    {
        ::com::sun::star::uno::Any aValue = xPropertySet->getPropertyValue(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" )));
        aValue >>= aCurRMItemData.Label;
        aValue = xPropertySet->getPropertyValue(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ID" )));
        aValue >>= aCurRMItemData.n_ID;
        aValue = xPropertySet->getPropertyValue(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enabled" )));
        aValue >>= aCurRMItemData.b_Enabled;
    }
    return aCurRMItemData;
}

void SVTXRoadmap::elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent )throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ::svt::ORoadmap* pField = GetRoadmap();
    if ( pField )
    {
        RMItemData CurItemData = GetRMItemData(  _rEvent );
        sal_Int32 InsertIndex = 0;
        _rEvent.Accessor >>= InsertIndex;
        pField->InsertRoadmapItem( InsertIndex, CurItemData.Label, (RoadmapTypes::ItemId)CurItemData.n_ID, CurItemData.b_Enabled );
    }
}

void SVTXRoadmap::elementRemoved( const ::com::sun::star::container::ContainerEvent& _rEvent )throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ::svt::ORoadmap* pField = GetRoadmap();
    if ( pField )
    {
        sal_Int32 DelIndex = 0;
        _rEvent.Accessor >>= DelIndex;
        pField->DeleteRoadmapItem(DelIndex);
    }
}

void SVTXRoadmap::elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent )throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    ::svt::ORoadmap* pField = GetRoadmap();
    if ( pField )
    {
        RMItemData CurItemData = GetRMItemData(  _rEvent );
        sal_Int32 ReplaceIndex = 0;
        _rEvent.Accessor >>= ReplaceIndex;
        pField->ReplaceRoadmapItem( ReplaceIndex, CurItemData.Label, (RoadmapTypes::ItemId)CurItemData.n_ID, CurItemData.b_Enabled );
    }
}



// --------------------------------------------------------------------------------------
void SVTXRoadmap::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::svt::ORoadmap* pField = GetRoadmap();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_COMPLETE:
            {
                sal_Bool b = false;
                Value >>= b;
                pField->SetRoadmapComplete( b);
            }
            break;

            case BASEPROPERTY_ACTIVATED:
            {
                   sal_Bool b = false;
                Value >>= b;
                pField->SetRoadmapInteractive( b);
            }
            break;

            case BASEPROPERTY_CURRENTITEMID:
            {
                   sal_Int32 nId = 0;
                Value >>= nId;
                pField->SelectRoadmapItemByID( (RoadmapTypes::ItemId)nId );
            }
            break;

            case BASEPROPERTY_TEXT:
            {
                   ::rtl::OUString aStr;
                Value >>= aStr;
                pField->SetText( aStr );
                pField->Invalidate();
            }
            break;

            default:
                SVTXRoadmap_Base::setProperty( PropertyName, Value );
                break;
        }

    }
    else
        SVTXRoadmap_Base::setProperty( PropertyName, Value );
}


// --------------------------------------------------------------------------------------
::com::sun::star::uno::Any SVTXRoadmap::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aReturn;

    ::svt::ORoadmap* pField = GetRoadmap();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_COMPLETE:
                aReturn <<= pField->IsRoadmapComplete();
                break;
            case BASEPROPERTY_ACTIVATED:
                aReturn <<= pField->IsRoadmapInteractive();
                break;
            case BASEPROPERTY_CURRENTITEMID:
                aReturn <<= pField->GetCurrentRoadmapItemID();
                break;
            default:
                aReturn = SVTXRoadmap_Base::getProperty(PropertyName);
                break;
        }
    }
    return aReturn;
}

void SVTXRoadmap::ImplSetNewImage()
{
    OSL_PRECOND( GetWindow(), "SVTXRoadmap::ImplSetNewImage: window is required to be not-NULL!" );
    ::svt::ORoadmap* pButton = static_cast< ::svt::ORoadmap* >( GetWindow() );
    pButton->SetRoadmapBitmap( GetImage().GetBitmapEx() );
}

void SVTXRoadmap::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_COMPLETE,
                     BASEPROPERTY_ACTIVATED,
                     BASEPROPERTY_CURRENTITEMID,
                     BASEPROPERTY_TEXT,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
    VCLXGraphicControl::ImplGetPropertyIds( rIds );
}

//  ----------------------------------------------------
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
                                        (static_cast< ::com::sun::star::awt::XNumericField* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( SVTXNumericField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XNumericField>* ) NULL ),
    SVTXFormattedField::getTypes()
IMPL_XTYPEPROVIDER_END


void SVTXNumericField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetValue( Value );
}

double SVTXNumericField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetValue() : 0;
}

void SVTXNumericField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMinValue( Value );
}

double SVTXNumericField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMinValue() : 0;
}

void SVTXNumericField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMaxValue( Value );
}

double SVTXNumericField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMaxValue() : 0;
}

void SVTXNumericField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinFirst( Value );
}

double SVTXNumericField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinFirst() : 0;
}

void SVTXNumericField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinLast( Value );
}

double SVTXNumericField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinLast() : 0;
}

void SVTXNumericField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinSize( Value );
}

double SVTXNumericField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinSize() : 0;
}

void SVTXNumericField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetDecimalDigits( Value );
}

sal_Int16 SVTXNumericField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetDecimalDigits() : 0;
}

void SVTXNumericField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetStrictFormat( bStrict );
}

sal_Bool SVTXNumericField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->IsStrictFormat() : sal_False;
}

void SVTXNumericField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    SVTXFormattedField::ImplGetPropertyIds( rIds );
}

//  ----------------------------------------------------
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
                                        (static_cast< ::com::sun::star::awt::XCurrencyField* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( SVTXCurrencyField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCurrencyField>* ) NULL ),
    SVTXFormattedField::getTypes()
IMPL_XTYPEPROVIDER_END

void SVTXCurrencyField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetValue( Value );
}

double SVTXCurrencyField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetValue() : 0;
}

void SVTXCurrencyField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMinValue( Value );
}

double SVTXCurrencyField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMinValue() : 0;
}

void SVTXCurrencyField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetMaxValue( Value );
}

double SVTXCurrencyField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetMaxValue() : 0;
}

void SVTXCurrencyField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinFirst( Value );
}

double SVTXCurrencyField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinFirst() : 0;
}

void SVTXCurrencyField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinLast( Value );
}

double SVTXCurrencyField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinLast() : 0;
}

void SVTXCurrencyField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetSpinSize( Value );
}

double SVTXCurrencyField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetSpinSize() : 0;
}

void SVTXCurrencyField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetDecimalDigits( Value );
}

sal_Int16 SVTXCurrencyField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->GetDecimalDigits() : 0;
}

void SVTXCurrencyField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    if ( pField )
        pField->SetStrictFormat( bStrict );
}

sal_Bool SVTXCurrencyField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    FormattedField* pField = GetFormattedField();
    return pField ? pField->IsStrictFormat() : sal_False;
}

void SVTXCurrencyField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aReturn;

    DoubleCurrencyField* pField = (DoubleCurrencyField*)GetFormattedField();
    if ( pField )
    {
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
                sal_Bool b = false;
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
    SolarMutexGuard aGuard;

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

void SVTXCurrencyField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_CURRENCYSYMBOL,
                     BASEPROPERTY_CURSYM_POSITION,
                     0);
    SVTXFormattedField::ImplGetPropertyIds( rIds );
}


//  ----------------------------------------------------
//  class VCLXProgressBar
//  ----------------------------------------------------

VCLXProgressBar::VCLXProgressBar()
            :m_nValue(0)
            ,m_nValueMin(0)
            ,m_nValueMax(100)
{
}

VCLXProgressBar::~VCLXProgressBar()
{
}

void VCLXProgressBar::ImplUpdateValue()
{
    ProgressBar* pProgressBar = (ProgressBar*) GetWindow();
    if ( pProgressBar )
    {
        sal_Int32 nVal;
        sal_Int32 nValMin;
        sal_Int32 nValMax;

        // check min and max
        if (m_nValueMin < m_nValueMax)
        {
            nValMin = m_nValueMin;
            nValMax = m_nValueMax;
        }
        else
        {
            nValMin = m_nValueMax;
            nValMax = m_nValueMin;
        }

        // check value
        if (m_nValue < nValMin)
        {
            nVal = nValMin;
        }
        else if (m_nValue > nValMax)
        {
            nVal = nValMax;
        }
        else
        {
            nVal = m_nValue;
        }

        // calculate percent
        sal_Int32 nPercent;
        if (nValMin != nValMax)
        {
            nPercent = 100 * (nVal - nValMin) / (nValMax - nValMin);
        }
        else
        {
            nPercent = 0;
        }

        // set progressbar value
        pProgressBar->SetValue( (sal_uInt16) nPercent );
    }
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXProgressBar::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< ::com::sun::star::awt::XProgressBar* >(this)),
                                        (static_cast< ::com::sun::star::lang::XTypeProvider* >(this)) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXProgressBar )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XProgressBar>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::awt::XProgressBar
void VCLXProgressBar::setForegroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Color aColor( nColor );
        pWindow->SetControlForeground( aColor );
    }
}

void VCLXProgressBar::setBackgroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Color aColor( nColor );
        pWindow->SetBackground( aColor );
        pWindow->SetControlBackground( aColor );
        pWindow->Invalidate();
    }
}

void VCLXProgressBar::setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    m_nValue = nValue;
    ImplUpdateValue();
}

void VCLXProgressBar::setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    if ( nMin < nMax )
    {
        // take correct min and max
        m_nValueMin = nMin;
        m_nValueMax = nMax;
    }
    else
    {
        // change min and max
        m_nValueMin = nMax;
        m_nValueMax = nMin;
    }

    ImplUpdateValue();
}

sal_Int32 VCLXProgressBar::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    return m_nValue;
}

// ::com::sun::star::awt::VclWindowPeer
void VCLXProgressBar::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ProgressBar* pProgressBar = (ProgressBar*)GetWindow();
    if ( pProgressBar )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_PROGRESSVALUE:
            {
                if ( Value >>= m_nValue )
                    ImplUpdateValue();
            }
            break;
            case BASEPROPERTY_PROGRESSVALUE_MIN:
            {
                if ( Value >>= m_nValueMin )
                    ImplUpdateValue();
            }
            break;
            case BASEPROPERTY_PROGRESSVALUE_MAX:
            {
                if ( Value >>= m_nValueMax )
                    ImplUpdateValue();
            }
            break;
            case BASEPROPERTY_FILLCOLOR:
            {
                Window* pWindow = GetWindow();
                if ( pWindow )
                {
                    sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

                    if ( bVoid )
                    {
                        pWindow->SetControlForeground();
                    }
                    else
                    {
                        sal_Int32 nColor = 0;
                        if ( Value >>= nColor )
                        {
                            Color aColor( nColor );
                            pWindow->SetControlForeground( aColor );
                        }
                    }
                }
            }
            break;
            default:
                VCLXWindow::setProperty( PropertyName, Value );
                break;
        }
    }
}

::com::sun::star::uno::Any VCLXProgressBar::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    ::com::sun::star::uno::Any aProp;
    ProgressBar* pProgressBar = (ProgressBar*)GetWindow();
    if ( pProgressBar )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_PROGRESSVALUE:
            {
                 aProp <<= m_nValue;
            }
            break;
            case BASEPROPERTY_PROGRESSVALUE_MIN:
            {
                 aProp <<= m_nValueMin;
            }
            break;
            case BASEPROPERTY_PROGRESSVALUE_MAX:
            {
                 aProp <<= m_nValueMax;
            }
            break;
            default:
                aProp <<= VCLXWindow::getProperty( PropertyName );
                break;
        }
    }
    return aProp;
}

void VCLXProgressBar::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_PROGRESSVALUE,
                     BASEPROPERTY_PROGRESSVALUE_MIN,
                     BASEPROPERTY_PROGRESSVALUE_MAX,
                     BASEPROPERTY_FILLCOLOR,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}


//  ----------------------------------------------------
//  class SVTXDateField
//  ----------------------------------------------------
SVTXDateField::SVTXDateField()
    :VCLXDateField()
{
}

SVTXDateField::~SVTXDateField()
{
}

void SAL_CALL SVTXDateField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXDateField::setProperty( PropertyName, Value );

    // some properties need to be forwarded to the sub edit, too
    Edit* pSubEdit = GetWindow() ? static_cast< Edit* >( GetWindow() )->GetSubEdit() : NULL;
    if ( !pSubEdit )
        return;

    switch ( GetPropertyId( PropertyName ) )
    {
    case BASEPROPERTY_TEXTLINECOLOR:
        if ( !Value.hasValue() )
            pSubEdit->SetTextLineColor();
        else
        {
            sal_Int32 nColor = 0;
            if ( Value >>= nColor )
                pSubEdit->SetTextLineColor( Color( nColor ) );
        }
        break;
    }
}

void SVTXDateField::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_TEXTLINECOLOR,
                     0);
    VCLXDateField::ImplGetPropertyIds( rIds );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
