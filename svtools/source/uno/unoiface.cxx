/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <tools/debug.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/prgsbar.hxx>
#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>
#include <svtools/svmedit.hxx>
#include <unoiface.hxx>
#include <svtools/filectrl.hxx>
#include <roadmap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/property.hxx>
#include <vcl/fmtfield.hxx>
#include <svl/numuno.hxx>
#include <svtools/calendar.hxx>
#include <vcl/treelistbox.hxx>
#include "treecontrolpeer.hxx"
#include "svtxgridcontrol.hxx"
#include <table/tablecontrol.hxx>

namespace
{
    void lcl_setWinBits( vcl::Window* _pWindow, WinBits _nBits, bool _bSet )
    {
        WinBits nStyle = _pWindow->GetStyle();
        if ( _bSet )
            nStyle |= _nBits;
        else
            nStyle &= ~_nBits;
        _pWindow->SetStyle( nStyle );
    }
}


//  help function for the toolkit...


extern "C" {

SAL_DLLPUBLIC_EXPORT vcl::Window* CreateWindow( VCLXWindow** ppNewComp, const css::awt::WindowDescriptor* pDescriptor, vcl::Window* pParent, WinBits nWinBits )
{
    vcl::Window* pWindow = nullptr;
    OUString aServiceName( pDescriptor->WindowServiceName );
    if ( aServiceName.equalsIgnoreAsciiCase( "MultiLineEdit" ) )
    {
        if ( pParent )
        {
            pWindow = VclPtr<MultiLineEdit>::Create( pParent, nWinBits|WB_IGNORETAB);
            static_cast< MultiLineEdit* >( pWindow )->DisableSelectionOnFocus();
            *ppNewComp = new VCLXMultiLineEdit;
        }
        else
        {
            *ppNewComp = nullptr;
            return nullptr;
        }
    }
    else if ( aServiceName.equalsIgnoreAsciiCase( "FileControl" ) )
    {
        if ( pParent )
        {
            pWindow = VclPtr<FileControl>::Create( pParent, nWinBits );
            *ppNewComp = new VCLXFileControl;
        }
        else
        {
            *ppNewComp = nullptr;
            return nullptr;
        }
    }
    else if (aServiceName.equalsIgnoreAsciiCase("FormattedField") )
    {
        pWindow = VclPtr<FormattedField>::Create( pParent, nWinBits );
        *ppNewComp = new SVTXFormattedField;
    }
    else if (aServiceName.equalsIgnoreAsciiCase("NumericField") )
    {
        pWindow = VclPtr<DoubleNumericField>::Create( pParent, nWinBits );
        *ppNewComp = new SVTXNumericField;
    }
    else if (aServiceName.equalsIgnoreAsciiCase("LongCurrencyField") )
    {
        pWindow = VclPtr<DoubleCurrencyField>::Create( pParent, nWinBits );
        *ppNewComp = new SVTXCurrencyField;
    }
    else if (aServiceName.equalsIgnoreAsciiCase("datefield") )
    {
        pWindow = VclPtr<CalendarField>::Create( pParent, nWinBits);
        static_cast<CalendarField*>(pWindow)->EnableToday();
        static_cast<CalendarField*>(pWindow)->EnableNone();
        static_cast<CalendarField*>(pWindow)->EnableEmptyFieldValue( true );
        SVTXDateField * newComp = new SVTXDateField;
        *ppNewComp = newComp;
        newComp->SetFormatter( static_cast<FormatterBase*>(static_cast<DateField*>(pWindow)) );
    }
    else if (aServiceName.equalsIgnoreAsciiCase("roadmap") )
    {
        pWindow = VclPtr< ::svt::ORoadmap>::Create( pParent, WB_TABSTOP );
        *ppNewComp = new SVTXRoadmap;
    }
    else if ( aServiceName.equalsIgnoreAsciiCase( "ProgressBar" ) )
    {
        if ( pParent )
        {
            pWindow = VclPtr<ProgressBar>::Create( pParent, nWinBits );
            *ppNewComp = new VCLXProgressBar;
        }
        else
        {
            *ppNewComp = nullptr;
            return nullptr;
        }
    }
    else if ( aServiceName.equalsIgnoreAsciiCase( "Tree" ) )
    {
        TreeControlPeer* pPeer = new TreeControlPeer;
        *ppNewComp = pPeer;
        pWindow = pPeer->createVclControl( pParent, nWinBits );
    }
    else if ( aServiceName.equalsIgnoreAsciiCase( "FixedHyperlink" ) )
    {
        if ( pParent )
        {
            pWindow = VclPtr<FixedHyperlink>::Create( pParent, nWinBits );
            *ppNewComp = new VCLXFixedHyperlink;
        }
        else
        {
            *ppNewComp = nullptr;
            return nullptr;
        }
    }
    else if ( aServiceName.equalsIgnoreAsciiCase( "Grid" ) )
    {
        if ( pParent )
        {
            pWindow = VclPtr< ::svt::table::TableControl>::Create(pParent, nWinBits);
            *ppNewComp = new SVTXGridControl;
        }
        else
        {
            *ppNewComp = nullptr;
            return nullptr;
        }
    }
    return pWindow;
}

}   // extern "C"


//  class VCLXMultiLineEdit

VCLXMultiLineEdit::VCLXMultiLineEdit()
    :maTextListeners( *this )
    ,meLineEndType( LINEEND_LF )    // default behavior before introducing this property: LF (unix-like)
{
}

VCLXMultiLineEdit::~VCLXMultiLineEdit()
{
}

css::uno::Any VCLXMultiLineEdit::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XTextComponent* >(this),
                                        static_cast< css::awt::XTextArea* >(this),
                                        static_cast< css::awt::XTextLayoutConstrains* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXMultiLineEdit )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXMultiLineEdit::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XTextComponent>::get(),
        cppu::UnoType<css::awt::XTextArea>::get(),
        cppu::UnoType<css::awt::XTextLayoutConstrains>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

void VCLXMultiLineEdit::addTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.addInterface( l );
}

void VCLXMultiLineEdit::removeTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.removeInterface( l );
}

void VCLXMultiLineEdit::setText( const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
    {
        pEdit->SetText( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( true );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( false );
    }
}

void VCLXMultiLineEdit::insertText( const css::awt::Selection& rSel, const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
    {
        setSelection( rSel );
        pEdit->ReplaceSelected( aText );
    }
}

OUString VCLXMultiLineEdit::getText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aText = pEdit->GetText( meLineEndType );
    return aText;
}

OUString VCLXMultiLineEdit::getSelectedText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit)
        aText = pMultiLineEdit->GetSelected( meLineEndType );
    return aText;

}

void VCLXMultiLineEdit::setSelection( const css::awt::Selection& aSelection )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
    {
        pMultiLineEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
    }
}

css::awt::Selection VCLXMultiLineEdit::getSelection()
{
    SolarMutexGuard aGuard;

    css::awt::Selection aSel;
    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
    {
        aSel.Min = pMultiLineEdit->GetSelection().Min();
        aSel.Max = pMultiLineEdit->GetSelection().Max();
    }
    return aSel;
}

sal_Bool VCLXMultiLineEdit::isEditable()
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    return pMultiLineEdit && !pMultiLineEdit->IsReadOnly() && pMultiLineEdit->IsEnabled();
}

void VCLXMultiLineEdit::setEditable( sal_Bool bEditable )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetReadOnly( !bEditable );
}

void VCLXMultiLineEdit::setMaxTextLen( sal_Int16 nLen )
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
        pMultiLineEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXMultiLineEdit::getMaxTextLen()
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    return pMultiLineEdit ? static_cast<sal_Int16>(pMultiLineEdit->GetMaxTextLen()) : sal_Int16(0);
}

OUString VCLXMultiLineEdit::getTextLines()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aText = pEdit->GetTextLines( meLineEndType );
    return aText;
}

css::awt::Size VCLXMultiLineEdit::getMinimumSize()
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcMinimumSize());
    return aSz;
}

css::awt::Size VCLXMultiLineEdit::getPreferredSize()
{
    return getMinimumSize();
}

css::awt::Size VCLXMultiLineEdit::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz = rNewSize;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcAdjustedSize( VCLSize(rNewSize )));
    return aSz;
}

css::awt::Size VCLXMultiLineEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
    if ( pEdit )
        aSz = AWTSize(pEdit->CalcBlockSize( nCols, nLines ));
    return aSz;
}

void VCLXMultiLineEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
{
    SolarMutexGuard aGuard;

    nCols = nLines = 0;
    VclPtr< MultiLineEdit > pEdit = GetAs< MultiLineEdit >();
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
        case VclEventId::EditModify:
        {
            if ( maTextListeners.getLength() )
            {
                css::awt::TextEvent aEvent;
                aEvent.Source = static_cast<cppu::OWeakObject*>(this);
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

void VCLXMultiLineEdit::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( !pMultiLineEdit )
        return;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
        case BASEPROPERTY_LINE_END_FORMAT:
        {
            sal_Int16 nLineEndType = css::awt::LineEndFormat::LINE_FEED;
            OSL_VERIFY( Value >>= nLineEndType );
            switch ( nLineEndType )
            {
            case css::awt::LineEndFormat::CARRIAGE_RETURN:           meLineEndType = LINEEND_CR; break;
            case css::awt::LineEndFormat::LINE_FEED:                 meLineEndType = LINEEND_LF; break;
            case css::awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED: meLineEndType = LINEEND_CRLF; break;
            default: OSL_FAIL( "VCLXMultiLineEdit::setProperty: invalid line end value!" ); break;
            }
        }
        break;

        case BASEPROPERTY_READONLY:
        {
            bool b;
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
            bool b;
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

css::uno::Any VCLXMultiLineEdit::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    VclPtr< MultiLineEdit > pMultiLineEdit = GetAs< MultiLineEdit >();
    if ( pMultiLineEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINE_END_FORMAT:
            {
                sal_Int16 nLineEndType = css::awt::LineEndFormat::LINE_FEED;
                switch ( meLineEndType )
                {
                case LINEEND_CR:   nLineEndType = css::awt::LineEndFormat::CARRIAGE_RETURN; break;
                case LINEEND_LF:   nLineEndType = css::awt::LineEndFormat::LINE_FEED; break;
                case LINEEND_CRLF: nLineEndType = css::awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED; break;
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
                aProp <<= static_cast<sal_Int16>(pMultiLineEdit->GetMaxTextLen());
            }
            break;
            default:
            {
                aProp = VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void SAL_CALL VCLXMultiLineEdit::setFocus(  )
{
    SolarMutexGuard aGuard;

    // don't grab the focus if we already have it. Reason is that the only thing which the edit
    // does is forwarding the focus to its text window. This text window then does a "select all".
    // So if the text window already has the focus, and we give the focus to the multi line
    // edit, then all which happens is that everything is selected.
    // #i27072#
    if ( GetWindow() && !GetWindow()->HasChildPathFocus() )
        GetWindow()->GrabFocus();
}

void VCLXMultiLineEdit::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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

//  class VCLXFileControl

VCLXFileControl::VCLXFileControl() : maTextListeners( *this )
{
}

VCLXFileControl::~VCLXFileControl()
{
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
        pControl->GetEdit().SetModifyHdl( Link<Edit&,void>() );
}

css::uno::Any VCLXFileControl::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XTextComponent* >(this),
                                        static_cast< css::awt::XTextLayoutConstrains* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXFileControl )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXFileControl::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XTextComponent>::get(),
        cppu::UnoType<css::awt::XTextLayoutConstrains>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

void SAL_CALL VCLXFileControl::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( !pControl )
        return;

    sal_uInt16 nPropType = GetPropertyId( PropertyName );
    switch ( nPropType )
    {
    case BASEPROPERTY_HIDEINACTIVESELECTION:
    {
        bool bValue(false);
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

void VCLXFileControl::SetWindow( const VclPtr< vcl::Window > &pWindow )
{
    VclPtr< FileControl > pPrevFileControl = GetAsDynamic< FileControl >();
    if ( pPrevFileControl )
        pPrevFileControl->SetEditModifyHdl( Link<Edit&,void>() );

    FileControl* pNewFileControl = dynamic_cast<FileControl*>( pWindow.get() );
    if ( pNewFileControl )
        pNewFileControl->SetEditModifyHdl( LINK( this, VCLXFileControl, ModifyHdl ) );

    VCLXWindow::SetWindow( pWindow );
}

void VCLXFileControl::addTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.addInterface( l );
}

void VCLXFileControl::removeTextListener( const css::uno::Reference< css::awt::XTextListener > & l )
{
    maTextListeners.removeInterface( l );
}

void VCLXFileControl::setText( const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetText( aText );

        // also in Java a textChanged is triggered, not in VCL.
        // css::awt::Toolkit should be JAVA-compliant...
        ModifyHdl();
    }
}

void VCLXFileControl::insertText( const css::awt::Selection& rSel, const OUString& aText )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
    {
        pFileControl->GetEdit().SetSelection( Selection( rSel.Min, rSel.Max ) );
        pFileControl->GetEdit().ReplaceSelected( aText );
    }
}

OUString VCLXFileControl::getText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

OUString VCLXFileControl::getSelectedText()
{
    SolarMutexGuard aGuard;

    OUString aText;
    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl)
        aText = pFileControl->GetEdit().GetSelected();
    return aText;

}

void VCLXFileControl::setSelection( const css::awt::Selection& aSelection )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
        pFileControl->GetEdit().SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

css::awt::Selection VCLXFileControl::getSelection()
{
    SolarMutexGuard aGuard;

    css::awt::Selection aSel;
    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
    {
        aSel.Min = pFileControl->GetEdit().GetSelection().Min();
        aSel.Max = pFileControl->GetEdit().GetSelection().Max();
    }
    return aSel;
}

sal_Bool VCLXFileControl::isEditable()
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    return pFileControl && !pFileControl->GetEdit().IsReadOnly() && pFileControl->GetEdit().IsEnabled();
}

void VCLXFileControl::setEditable( sal_Bool bEditable )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
        pFileControl->GetEdit().SetReadOnly( !bEditable );
}

void VCLXFileControl::setMaxTextLen( sal_Int16 nLen )
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    if ( pFileControl )
        pFileControl->GetEdit().SetMaxTextLen( nLen );
}

sal_Int16 VCLXFileControl::getMaxTextLen()
{
    SolarMutexGuard aGuard;

    VclPtr< FileControl > pFileControl = GetAs< FileControl >();
    return pFileControl ? pFileControl->GetEdit().GetMaxTextLen() : 0;
}


IMPL_LINK_NOARG(VCLXFileControl, ModifyHdl, Edit&, void)
{
    ModifyHdl();
}

void VCLXFileControl::ModifyHdl()
{
    css::awt::TextEvent aEvent;
    aEvent.Source = static_cast<cppu::OWeakObject*>(this);
    maTextListeners.textChanged( aEvent );
}

css::awt::Size VCLXFileControl::getMinimumSize()
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
    {
        Size aTmpSize = pControl->GetEdit().CalcMinimumSize();
        aTmpSize.AdjustWidth(pControl->GetButton().CalcMinimumSize().Width() );
        aSz = AWTSize(pControl->CalcWindowSize( aTmpSize ));
    }
    return aSz;
}

css::awt::Size VCLXFileControl::getPreferredSize()
{
    css::awt::Size aSz = getMinimumSize();
    aSz.Height += 4;
    return aSz;
}

css::awt::Size VCLXFileControl::calcAdjustedSize( const css::awt::Size& rNewSize )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz =rNewSize;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
    {
        css::awt::Size aMinSz = getMinimumSize();
        if ( aSz.Height != aMinSz.Height )
            aSz.Height = aMinSz.Height;
    }
    return aSz;
}

css::awt::Size VCLXFileControl::getMinimumSize( sal_Int16 nCols, sal_Int16 )
{
    SolarMutexGuard aGuard;

    css::awt::Size aSz;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
    {
        aSz = AWTSize(pControl->GetEdit().CalcSize( nCols ));
        aSz.Width += pControl->GetButton().CalcMinimumSize().Width();
    }
    return aSz;
}

void VCLXFileControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines )
{
    SolarMutexGuard aGuard;

    nCols = 0;
    nLines = 1;
    VclPtr< FileControl > pControl = GetAs< FileControl >();
    if ( pControl )
        nCols = pControl->GetEdit().GetMaxVisChars();
}

void VCLXFileControl::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     // FIXME: elide duplication ?
                     BASEPROPERTY_HIDEINACTIVESELECTION,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}


//  class SVTXFormattedField


SVTXFormattedField::SVTXFormattedField()
    :bIsStandardSupplier(true)
    ,nKeyToSetDelayed(-1)
{
}


SVTXFormattedField::~SVTXFormattedField()
{
}


void SVTXFormattedField::SetWindow( const VclPtr< vcl::Window > &_pWindow )
{
    VCLXSpinField::SetWindow(_pWindow);
    if (GetAs< FormattedField >())
        GetAs< FormattedField >()->SetAutoColor(true);
}


void SVTXFormattedField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_ENFORCE_FORMAT:
            {
                bool bEnable( true );
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
                bool b;
                if ( Value >>= b )
                    SetTreatAsNumber(b);
            }
            break;

            case BASEPROPERTY_FORMATSSUPPLIER:
                if (!Value.hasValue())
                    setFormatsSupplier(css::uno::Reference< css::util::XNumberFormatsSupplier > (nullptr));
                else
                {
                    css::uno::Reference< css::util::XNumberFormatsSupplier > xNFS;
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
                const css::uno::TypeClass rTC = Value.getValueType().getTypeClass();
                if (rTC != css::uno::TypeClass_STRING)
                    // no string
                    if (rTC != css::uno::TypeClass_DOUBLE)
                        // no double
                        if (Value.hasValue())
                        {   // but a value
                            // try if it is something convertible
                            sal_Int32 nValue = 0;
                            if (!(Value >>= nValue))
                                throw css::lang::IllegalArgumentException();
                            SetValue(css::uno::makeAny(static_cast<double>(nValue)));
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
                     pField->SetDecimalDigits( static_cast<sal_uInt16>(n) );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                    bool b;
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


css::uno::Any SVTXFormattedField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aReturn;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_EFFECTIVE_MIN:
            case BASEPROPERTY_VALUEMIN_DOUBLE:
                aReturn = GetMinValue();
                break;

            case BASEPROPERTY_EFFECTIVE_MAX:
            case BASEPROPERTY_VALUEMAX_DOUBLE:
                aReturn = GetMaxValue();
                break;

            case BASEPROPERTY_EFFECTIVE_DEFAULT:
                aReturn = GetDefaultValue();
                break;

            case BASEPROPERTY_TREATASNUMBER:
                aReturn <<= GetTreatAsNumber();
                break;

            case BASEPROPERTY_EFFECTIVE_VALUE:
            case BASEPROPERTY_VALUE_DOUBLE:
                aReturn = GetValue();
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
                    css::uno::Reference< css::util::XNumberFormatsSupplier >  xSupplier = m_xCurrentSupplier.get();
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
                aReturn = VCLXSpinField::getProperty(PropertyName);
        }
    }
    return aReturn;
}

css::uno::Any SVTXFormattedField::convertEffectiveValue(const css::uno::Any& rValue)
{
    css::uno::Any aReturn;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return aReturn;

    switch (rValue.getValueType().getTypeClass())
    {
        case css::uno::TypeClass_DOUBLE:
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
                OUString sConverted;
                pFormatter->GetOutputString(d, 0, sConverted, &pDum);
                aReturn <<= sConverted;
            }
            break;
        case css::uno::TypeClass_STRING:
        {
            OUString aStr;
            rValue >>= aStr;
            if (pField->TreatingAsNumber())
            {
                SvNumberFormatter* pFormatter = pField->GetFormatter();
                if (!pFormatter)
                    pFormatter = pField->StandardFormatter();

                double dVal;
                sal_uInt32 nTestFormat(0);
                if (!pFormatter->IsNumberFormat(aStr, nTestFormat, dVal))
                    aReturn.clear();
                aReturn <<= dVal;
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


void SVTXFormattedField::SetMinValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    switch (rValue.getValueType().getTypeClass())

    {
        case css::uno::TypeClass_DOUBLE:
        {
            double d = 0.0;
            rValue >>= d;
            pField->SetMinValue(d);
        }
            break;
        default:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == css::uno::TypeClass_VOID, "SVTXFormattedField::SetMinValue : invalid argument (an exception will be thrown) !");
            if ( rValue.getValueType().getTypeClass() != css::uno::TypeClass_VOID )

            {
                throw css::lang::IllegalArgumentException();
            }
            pField->ClearMinValue();
            break;
    }
}


css::uno::Any SVTXFormattedField::GetMinValue()
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField || !pField->HasMinValue())
        return css::uno::Any();

    css::uno::Any aReturn;
    aReturn <<= pField->GetMinValue();
    return aReturn;
}


void SVTXFormattedField::SetMaxValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    switch (rValue.getValueType().getTypeClass())

    {
        case css::uno::TypeClass_DOUBLE:
        {
            double d = 0.0;
            rValue >>= d;
            pField->SetMaxValue(d);
        }
            break;
        default:
            if (rValue.getValueType().getTypeClass() != css::uno::TypeClass_VOID)

            {
                throw css::lang::IllegalArgumentException();
            }
            pField->ClearMaxValue();
            break;
    }
}


css::uno::Any SVTXFormattedField::GetMaxValue()
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField || !pField->HasMaxValue())
        return css::uno::Any();

    css::uno::Any aReturn;
    aReturn <<= pField->GetMaxValue();
    return aReturn;
}


void SVTXFormattedField::SetDefaultValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    css::uno::Any aConverted = convertEffectiveValue(rValue);

    switch (aConverted.getValueType().getTypeClass())

    {
        case css::uno::TypeClass_DOUBLE:
        {
            double d = 0.0;
            aConverted >>= d;
            pField->SetDefaultValue(d);
        }
        break;
        case css::uno::TypeClass_STRING:
        {
            OUString aStr;
            aConverted >>= aStr;
            pField->SetDefaultText( aStr );
        }
        break;
        default:
            pField->EnableEmptyField(true);
                // only void accepted
            break;
    }
}


css::uno::Any SVTXFormattedField::GetDefaultValue()
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField || pField->IsEmptyFieldEnabled())
        return css::uno::Any();

    css::uno::Any aReturn;
    if (pField->TreatingAsNumber())
        aReturn <<= pField->GetDefaultValue();
    else
        aReturn <<= pField->GetDefaultText();
    return aReturn;
}


bool SVTXFormattedField::GetTreatAsNumber()
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (pField)
        return pField->TreatingAsNumber();

    return true;
}


void SVTXFormattedField::SetTreatAsNumber(bool bSet)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (pField)
        pField->TreatAsNumber(bSet);
}


css::uno::Any SVTXFormattedField::GetValue()
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return css::uno::Any();

    css::uno::Any aReturn;
    if (!pField->TreatingAsNumber())
    {
        OUString sText = pField->GetTextValue();
        aReturn <<= sText;
    }
    else
    {
        if (!pField->GetText().isEmpty())    // empty is returned as void by default
            aReturn <<= pField->GetValue();
    }

    return aReturn;
}


void SVTXFormattedField::SetValue(const css::uno::Any& rValue)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    if (!rValue.hasValue())
    {
        pField->SetText("");
    }
    else
    {
        if (rValue.getValueType().getTypeClass() == css::uno::TypeClass_DOUBLE )
        {
            double d = 0.0;
            rValue >>= d;
            pField->SetValue(d);
        }
        else
        {
            DBG_ASSERT(rValue.getValueType().getTypeClass() == css::uno::TypeClass_STRING, "SVTXFormattedField::SetValue : invalid argument !");

            OUString sText;
            rValue >>= sText;
            if (!pField->TreatingAsNumber())
                pField->SetTextFormatted(sText);
            else
                pField->SetTextValue(sText);
        }
    }
//  NotifyTextListeners();
}


void SVTXFormattedField::setFormatsSupplier(const css::uno::Reference< css::util::XNumberFormatsSupplier > & xSupplier)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();

    SvNumberFormatsSupplierObj* pNew = nullptr;
    if (!xSupplier.is())
    {
        if (pField)
        {
            pNew = new SvNumberFormatsSupplierObj(pField->StandardFormatter());
            bIsStandardSupplier = true;
        }
    }
    else
    {
        pNew = SvNumberFormatsSupplierObj::getImplementation(xSupplier);
        bIsStandardSupplier = false;
    }

    if (!pNew)
        return;     // TODO : how to process ?

    m_xCurrentSupplier = pNew;
    if (!pField)
        return;

    // save the actual value
    css::uno::Any aCurrent = GetValue();
    pField->SetFormatter(m_xCurrentSupplier->GetNumberFormatter(), false);
    if (nKeyToSetDelayed != -1)
    {
        pField->SetFormatKey(nKeyToSetDelayed);
        nKeyToSetDelayed = -1;
    }
    SetValue(aCurrent);
    NotifyTextListeners();
}


sal_Int32 SVTXFormattedField::getFormatKey() const
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetFormatKey() : 0;
}


void SVTXFormattedField::setFormatKey(sal_Int32 nKey)
{
    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if (!pField)
        return;

    if (pField->GetFormatter())
        pField->SetFormatKey(nKey);
    else
    {
        // probably I am in a block, in which first the key and next the formatter will be set,
        // initially this happens quite certain, as the properties are set in alphabetic sequence,
        // and the FormatsSupplier is processed before the FormatKey
        nKeyToSetDelayed = nKey;
    }
    NotifyTextListeners();
}


void SVTXFormattedField::NotifyTextListeners()
{
    if ( GetTextListeners().getLength() )
    {
        css::awt::TextEvent aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        GetTextListeners().textChanged( aEvent );
    }
}

void SVTXFormattedField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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


//  class SVTXRoadmap

using namespace svt;

SVTXRoadmap::SVTXRoadmap() : maItemListeners( *this )
{
}

SVTXRoadmap::~SVTXRoadmap()
{
}

void SVTXRoadmap::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::RoadmapItemSelected:
        {
            SolarMutexGuard aGuard;
            VclPtr<::svt::ORoadmap> pField = GetAs< svt::ORoadmap >();
            if ( pField )
            {
                sal_Int16 CurItemID = pField->GetCurrentRoadmapItemID();
                css::awt::ItemEvent aEvent;
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


void SVTXRoadmap::propertyChange( const css::beans::PropertyChangeEvent& evt )
{
    SolarMutexGuard aGuard;
    VclPtr<::svt::ORoadmap> pField = GetAs< svt::ORoadmap >();
    if ( !pField )
        return;

    css::uno::Reference< css::uno::XInterface > xRoadmapItem;
    xRoadmapItem = evt.Source;
    sal_Int32 nID = 0;
    css::uno::Reference< css::beans::XPropertySet > xPropertySet( xRoadmapItem, css::uno::UNO_QUERY );
    css::uno::Any aValue = xPropertySet->getPropertyValue("ID");
    aValue >>= nID;

    OUString sPropertyName = evt.PropertyName;
    if ( sPropertyName == "Enabled" )
    {
        bool bEnable = false;
        evt.NewValue >>= bEnable;
        pField->EnableRoadmapItem( static_cast<RoadmapTypes::ItemId>(nID) , bEnable );
    }
    else if ( sPropertyName == "Label" )
    {
        OUString sLabel;
        evt.NewValue >>= sLabel;
        pField->ChangeRoadmapItemLabel( static_cast<RoadmapTypes::ItemId>(nID) , sLabel );
    }
    else if  ( sPropertyName == "ID" )
    {
        sal_Int32 nNewID = 0;
        evt.NewValue >>= nNewID;
        evt.OldValue >>= nID;
        pField->ChangeRoadmapItemID( static_cast<RoadmapTypes::ItemId>(nID), static_cast<RoadmapTypes::ItemId>(nNewID) );
    }
//    else
        // TODO handle Interactive appropriately
}


void SVTXRoadmap::addItemListener( const css::uno::Reference< css::awt::XItemListener >& l )
{
    maItemListeners.addInterface( l );
}

void SVTXRoadmap::removeItemListener( const css::uno::Reference< css::awt::XItemListener >& l )
{
    maItemListeners.removeInterface( l );
}

RMItemData SVTXRoadmap::GetRMItemData( const css::container::ContainerEvent& _rEvent )
{
    RMItemData aCurRMItemData;
    css::uno::Reference< css::uno::XInterface > xRoadmapItem;
    _rEvent.Element >>= xRoadmapItem;
    css::uno::Reference< css::beans::XPropertySet > xPropertySet( xRoadmapItem, css::uno::UNO_QUERY );
    if ( xPropertySet.is() )
    {
        css::uno::Any aValue = xPropertySet->getPropertyValue("Label");
        aValue >>= aCurRMItemData.Label;
        aValue = xPropertySet->getPropertyValue("ID");
        aValue >>= aCurRMItemData.n_ID;
        aValue = xPropertySet->getPropertyValue("Enabled");
        aValue >>= aCurRMItemData.b_Enabled;
    }
    else
    {
        aCurRMItemData.b_Enabled = false;
        aCurRMItemData.n_ID = 0;
    }
    return aCurRMItemData;
}

void SVTXRoadmap::elementInserted( const css::container::ContainerEvent& _rEvent )
{
    SolarMutexGuard aGuard;
    VclPtr<::svt::ORoadmap> pField = GetAs< svt::ORoadmap >();
    if ( pField )
    {
        RMItemData CurItemData = GetRMItemData(  _rEvent );
        sal_Int32 InsertIndex = 0;
        _rEvent.Accessor >>= InsertIndex;
        pField->InsertRoadmapItem( InsertIndex, CurItemData.Label, static_cast<RoadmapTypes::ItemId>(CurItemData.n_ID), CurItemData.b_Enabled );
    }
}

void SVTXRoadmap::elementRemoved( const css::container::ContainerEvent& _rEvent )
{
    SolarMutexGuard aGuard;
    VclPtr<::svt::ORoadmap> pField = GetAs< svt::ORoadmap >();
    if ( pField )
    {
        sal_Int32 DelIndex = 0;
        _rEvent.Accessor >>= DelIndex;
        pField->DeleteRoadmapItem(DelIndex);
    }
}

void SVTXRoadmap::elementReplaced( const css::container::ContainerEvent& _rEvent )
{
    SolarMutexGuard aGuard;
    VclPtr<::svt::ORoadmap> pField = GetAs< svt::ORoadmap >();
    if ( pField )
    {
        RMItemData CurItemData = GetRMItemData(  _rEvent );
        sal_Int32 ReplaceIndex = 0;
        _rEvent.Accessor >>= ReplaceIndex;
        pField->ReplaceRoadmapItem( ReplaceIndex, CurItemData.Label, static_cast<RoadmapTypes::ItemId>(CurItemData.n_ID), CurItemData.b_Enabled );
    }
}


void SVTXRoadmap::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr<::svt::ORoadmap> pField = GetAs< svt::ORoadmap >();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_COMPLETE:
            {
                bool b = false;
                Value >>= b;
                pField->SetRoadmapComplete( b);
            }
            break;

            case BASEPROPERTY_ACTIVATED:
            {
                bool b = false;
                Value >>= b;
                pField->SetRoadmapInteractive( b);
            }
            break;

            case BASEPROPERTY_CURRENTITEMID:
            {
                sal_Int32 nId = 0;
                Value >>= nId;
                pField->SelectRoadmapItemByID( static_cast<RoadmapTypes::ItemId>(nId) );
            }
            break;

            case BASEPROPERTY_TEXT:
            {
                OUString aStr;
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


css::uno::Any SVTXRoadmap::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aReturn;

    VclPtr<::svt::ORoadmap> pField = GetAs< svt::ORoadmap >();
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
    VclPtr< ::svt::ORoadmap > pButton = GetAs< ::svt::ORoadmap >();
    pButton->SetRoadmapBitmap( GetImage().GetBitmapEx() );
}

void SVTXRoadmap::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
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


//  class SVTXNumericField

SVTXNumericField::SVTXNumericField()
{
}

SVTXNumericField::~SVTXNumericField()
{
}

css::uno::Any SVTXNumericField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XNumericField* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( SVTXNumericField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > SVTXNumericField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XNumericField>::get(),
        SVTXFormattedField::getTypes()
    );
    return aTypeList.getTypes();
}


void SVTXNumericField::setValue( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetValue( Value );
}

double SVTXNumericField::getValue()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetValue() : 0;
}

void SVTXNumericField::setMin( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetMinValue( Value );
}

double SVTXNumericField::getMin()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetMinValue() : 0;
}

void SVTXNumericField::setMax( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetMaxValue( Value );
}

double SVTXNumericField::getMax()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetMaxValue() : 0;
}

void SVTXNumericField::setFirst( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetSpinFirst( Value );
}

double SVTXNumericField::getFirst()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetSpinFirst() : 0;
}

void SVTXNumericField::setLast( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetSpinLast( Value );
}

double SVTXNumericField::getLast()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetSpinLast() : 0;
}

void SVTXNumericField::setSpinSize( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetSpinSize( Value );
}

double SVTXNumericField::getSpinSize()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetSpinSize() : 0;
}

void SVTXNumericField::setDecimalDigits( sal_Int16 Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetDecimalDigits( Value );
}

sal_Int16 SVTXNumericField::getDecimalDigits()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetDecimalDigits() : 0;
}

void SVTXNumericField::setStrictFormat( sal_Bool bStrict )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetStrictFormat( bStrict );
}

sal_Bool SVTXNumericField::isStrictFormat()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField && pField->IsStrictFormat();
}

void SVTXNumericField::GetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    SVTXFormattedField::ImplGetPropertyIds( rIds );
}


//  class SVTXCurrencyField

SVTXCurrencyField::SVTXCurrencyField()
{
}

SVTXCurrencyField::~SVTXCurrencyField()
{
}

css::uno::Any SVTXCurrencyField::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XCurrencyField* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : SVTXFormattedField::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( SVTXCurrencyField )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > SVTXCurrencyField::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XCurrencyField>::get(),
        SVTXFormattedField::getTypes()
    );
    return aTypeList.getTypes();
}

void SVTXCurrencyField::setValue( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetValue( Value );
}

double SVTXCurrencyField::getValue()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetValue() : 0;
}

void SVTXCurrencyField::setMin( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetMinValue( Value );
}

double SVTXCurrencyField::getMin()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetMinValue() : 0;
}

void SVTXCurrencyField::setMax( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetMaxValue( Value );
}

double SVTXCurrencyField::getMax()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetMaxValue() : 0;
}

void SVTXCurrencyField::setFirst( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetSpinFirst( Value );
}

double SVTXCurrencyField::getFirst()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetSpinFirst() : 0;
}

void SVTXCurrencyField::setLast( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetSpinLast( Value );
}

double SVTXCurrencyField::getLast()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetSpinLast() : 0;
}

void SVTXCurrencyField::setSpinSize( double Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetSpinSize( Value );
}

double SVTXCurrencyField::getSpinSize()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetSpinSize() : 0;
}

void SVTXCurrencyField::setDecimalDigits( sal_Int16 Value )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetDecimalDigits( Value );
}

sal_Int16 SVTXCurrencyField::getDecimalDigits()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField ? pField->GetDecimalDigits() : 0;
}

void SVTXCurrencyField::setStrictFormat( sal_Bool bStrict )
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    if ( pField )
        pField->SetStrictFormat( bStrict );
}

sal_Bool SVTXCurrencyField::isStrictFormat()
{
    SolarMutexGuard aGuard;

    VclPtr<FormattedField> pField = GetAs< FormattedField >();
    return pField && pField->IsStrictFormat();
}

void SVTXCurrencyField::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< DoubleCurrencyField > pField = GetAs< DoubleCurrencyField >();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                OUString aStr;
                Value >>= aStr;
                pField->setCurrencySymbol( aStr );
            }
            break;
            case BASEPROPERTY_CURSYM_POSITION:
            {
                bool b = false;
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

css::uno::Any SVTXCurrencyField::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aReturn;

    VclPtr< DoubleCurrencyField > pField = GetAs< DoubleCurrencyField >();
    if ( pField )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch (nPropType)
        {
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                aReturn <<= pField->getCurrencySymbol();
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

void SVTXCurrencyField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_CURRENCYSYMBOL,
                     BASEPROPERTY_CURSYM_POSITION,
                     0);
    SVTXFormattedField::ImplGetPropertyIds( rIds );
}


//  class VCLXProgressBar


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
    VclPtr< ProgressBar > pProgressBar = GetAs< ProgressBar >();
    if ( !pProgressBar )
        return;

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
    pProgressBar->SetValue( static_cast<sal_uInt16>(nPercent) );
}

// css::uno::XInterface
css::uno::Any VCLXProgressBar::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = ::cppu::queryInterface( rType,
                                        static_cast< css::awt::XProgressBar* >(this),
                                        static_cast< css::lang::XTypeProvider* >(this) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

IMPL_IMPLEMENTATION_ID( VCLXProgressBar )

// css::lang::XTypeProvider
css::uno::Sequence< css::uno::Type > VCLXProgressBar::getTypes()
{
    static const ::cppu::OTypeCollection aTypeList(
        cppu::UnoType<css::lang::XTypeProvider>::get(),
        cppu::UnoType<css::awt::XProgressBar>::get(),
        VCLXWindow::getTypes()
    );
    return aTypeList.getTypes();
}

// css::awt::XProgressBar
void VCLXProgressBar::setForegroundColor( sal_Int32 nColor )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->SetControlForeground( Color(nColor) );
    }
}

void VCLXProgressBar::setBackgroundColor( sal_Int32 nColor )
{
    SolarMutexGuard aGuard;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        Color aColor = Color( nColor );
        pWindow->SetBackground( aColor );
        pWindow->SetControlBackground( aColor );
        pWindow->Invalidate();
    }
}

void VCLXProgressBar::setValue( sal_Int32 nValue )
{
    SolarMutexGuard aGuard;

    m_nValue = nValue;
    ImplUpdateValue();
}

void VCLXProgressBar::setRange( sal_Int32 nMin, sal_Int32 nMax )
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

sal_Int32 VCLXProgressBar::getValue()
{
    SolarMutexGuard aGuard;

    return m_nValue;
}

// css::awt::VclWindowPeer
void VCLXProgressBar::setProperty( const OUString& PropertyName, const css::uno::Any& Value)
{
    SolarMutexGuard aGuard;

    VclPtr< ProgressBar > pProgressBar = GetAs< ProgressBar >();
    if ( !pProgressBar )
        return;

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
            VclPtr<vcl::Window> pWindow = GetWindow();
            if ( pWindow )
            {
                bool bVoid = Value.getValueType().getTypeClass() == css::uno::TypeClass_VOID;

                if ( bVoid )
                {
                    pWindow->SetControlForeground();
                }
                else
                {
                    sal_Int32 nColor = 0;
                    if ( Value >>= nColor )
                    {
                        Color aColor = Color( nColor );
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

css::uno::Any VCLXProgressBar::getProperty( const OUString& PropertyName )
{
    SolarMutexGuard aGuard;

    css::uno::Any aProp;
    VclPtr< ProgressBar > pProgressBar = GetAs< ProgressBar >();
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
                aProp = VCLXWindow::getProperty( PropertyName );
                break;
        }
    }
    return aProp;
}

void VCLXProgressBar::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_PROGRESSVALUE,
                     BASEPROPERTY_PROGRESSVALUE_MIN,
                     BASEPROPERTY_PROGRESSVALUE_MAX,
                     BASEPROPERTY_FILLCOLOR,
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}


//  class SVTXDateField

SVTXDateField::SVTXDateField()
    :VCLXDateField()
{
}

SVTXDateField::~SVTXDateField()
{
}

void SAL_CALL SVTXDateField::setProperty( const OUString& PropertyName, const css::uno::Any& Value )
{
    VCLXDateField::setProperty( PropertyName, Value );

    // some properties need to be forwarded to the sub edit, too
    SolarMutexGuard g;
    VclPtr< Edit > pSubEdit = GetWindow() ? static_cast< Edit* >( GetWindow().get() )->GetSubEdit() : nullptr;
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

void SVTXDateField::ImplGetPropertyIds( std::vector< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
                     BASEPROPERTY_TEXTLINECOLOR,
                     0);
    VCLXDateField::ImplGetPropertyIds( rIds );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
