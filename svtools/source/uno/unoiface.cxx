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
#include <vcl/svapp.hxx>
#include <svl/zforlist.hxx>
#include <svmedit.hxx>
#include <unoiface.hxx>
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/helper/property.hxx>
#include <svl/numuno.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
