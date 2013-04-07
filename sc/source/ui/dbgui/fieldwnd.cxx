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
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 ************************************************************************/

#include "fieldwnd.hxx"

#include <comphelper/string.hxx>
#include <vcl/decoview.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/mnemonic.hxx>

#include "pvlaydlg.hxx"
#include "dpuiglobal.hxx"
#include "dpmacros.hxx"
#include "AccessibleDataPilotControl.hxx"
#include "scresid.hxx"
#include "pivot.hrc"

using namespace com::sun::star;
using ::std::vector;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::WeakReference;
using ::com::sun::star::accessibility::XAccessible;

const size_t PIVOTFIELD_INVALID = static_cast< size_t >(-1);
const size_t INVALID_INDEX = static_cast<size_t>(-1);

#if DEBUG_PIVOT_TABLE
using std::cout;
using std::endl;
#endif

namespace {

#if DEBUG_PIVOT_TABLE
void DumpAllFuncData(const ScDPFieldControlBase::FuncDataType& rData)
{
    cout << "---" << endl;
    ScDPFieldControlBase::FuncDataType::const_iterator it = rData.begin(), itEnd = rData.end();
    for (; it != itEnd; ++it)
        it->Dump();
}
#endif

}

ScDPFieldControlBase::FieldName::FieldName(const OUString& rText, bool bFits, sal_uInt8 nDupCount) :
    maText(rText), mbFits(bFits), mnDupCount(nDupCount) {}

ScDPFieldControlBase::FieldName::FieldName(const FieldName& r) :
    maText(r.maText), mbFits(r.mbFits), mnDupCount(r.mnDupCount) {}

OUString ScDPFieldControlBase::FieldName::getDisplayedText() const
{
    OUStringBuffer aBuf(maText);
    if (mnDupCount > 0)
        aBuf.append(static_cast<sal_Int32>(mnDupCount+1));
    return aBuf.makeStringAndClear();
}

ScDPFieldControlBase::ScrollBar::ScrollBar(Window* pParent, WinBits nStyle) :
    ::ScrollBar(pParent, nStyle),
    mpParent(pParent)
{
}

void ScDPFieldControlBase::ScrollBar::Command( const CommandEvent& rCEvt )
{
    mpParent->Command(rCEvt);
}

ScDPFieldControlBase::AccessRef::AccessRef( const com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessible > & rAccessible ) : mxRef( rAccessible ) {}

// easy, safe access to the backing accessible for the lifetime of AccessRef
ScAccessibleDataPilotControl *ScDPFieldControlBase::AccessRef::operator -> () const
{
    if (!mxRef.is())
        return NULL;
    return static_cast< ScAccessibleDataPilotControl * >( mxRef.get() );
}

ScDPFieldControlBase::ScDPFieldControlBase(
    ScPivotLayoutDlg* pParent, const ResId& rResId, FixedText* pCaption, const char* pcHelpId) :
    Control(pParent, rResId),
    mpDlg(pParent),
    mpCaption(pCaption),
    mnFieldSelected(0)
{
    SetHelpId( pcHelpId );

    if (pCaption)
        maName = MnemonicGenerator::EraseAllMnemonicChars( pCaption->GetText() );
}

ScDPFieldControlBase::~ScDPFieldControlBase()
{
    AccessRef aRef( mxAccessible );
    if( aRef.is() )
        aRef->dispose();
}

OUString ScDPFieldControlBase::GetName() const
{
    return maName;
}

void ScDPFieldControlBase::SetName(const OUString& rName)
{
    maName = rName;
}

bool ScDPFieldControlBase::IsExistingIndex( size_t nIndex ) const
{
    return nIndex < maFieldNames.size();
}

void ScDPFieldControlBase::AppendField( const OUString& rText, const ScPivotFuncData& rFunc )
{
    size_t nNewIndex = maFieldNames.size();

    sal_uInt8 nDupCount = GetNextDupCount(rText);
    maFieldNames.push_back(FieldName(rText, true, nDupCount));
    maFuncData.push_back(new ScPivotFuncData(rFunc));

    AccessRef xRef(mxAccessible);
    if ( xRef.is() )
        xRef->AddField(nNewIndex);
}

size_t ScDPFieldControlBase::AddField(
    const OUString& rText, const Point& rPos, const ScPivotFuncData& rFunc)
{
    size_t nNewIndex = GetFieldIndex(rPos);
    if (nNewIndex == PIVOTFIELD_INVALID)
        return PIVOTFIELD_INVALID;

    if (nNewIndex > maFieldNames.size())
        nNewIndex = maFieldNames.size();

    sal_uInt8 nDupCount = GetNextDupCount(rText);
    maFieldNames.insert(maFieldNames.begin() + nNewIndex, FieldName(rText, true, nDupCount));

    maFuncData.insert(maFuncData.begin() + nNewIndex, new ScPivotFuncData(rFunc));
    maFuncData.back().mnDupCount = nDupCount;

    mnFieldSelected = nNewIndex;
    ResetScrollBar();
    Invalidate();

    AccessRef xRef( mxAccessible );
    if ( xRef.is() )
        xRef->AddField(nNewIndex);

    return nNewIndex;
}

bool ScDPFieldControlBase::MoveField(size_t nCurPos, const Point& rPos, size_t& rnIndex)
{
    if (nCurPos >= maFieldNames.size())
        // out-of-bound
        return false;

    size_t nNewIndex = GetFieldIndex(rPos);
    if (nNewIndex == PIVOTFIELD_INVALID)
        return false;

    if (nNewIndex == nCurPos)
        // Nothing to do.
        return true;

    FieldName aName = maFieldNames[nCurPos];
    ScPivotFuncData aFunc = maFuncData[nCurPos];
    if (nNewIndex >= maFieldNames.size())
    {
        // Move to the back.
        maFieldNames.erase(maFieldNames.begin()+nCurPos);
        maFieldNames.push_back(aName);
        maFuncData.erase(maFuncData.begin()+nCurPos);
        maFuncData.push_back(new ScPivotFuncData(aFunc));
        rnIndex = maFieldNames.size()-1;
    }
    else
    {
        maFieldNames.erase(maFieldNames.begin()+nCurPos);
        maFuncData.erase(maFuncData.begin()+nCurPos);
        size_t nTmp = nNewIndex; // we need to keep the original index for accessible.
        if (nNewIndex > nCurPos)
            --nTmp;

        maFieldNames.insert(maFieldNames.begin()+nTmp, aName);
        maFuncData.insert(maFuncData.begin()+nTmp, new ScPivotFuncData(aFunc));
        rnIndex = nTmp;
    }

    ResetScrollBar();
    Invalidate();

    AccessRef xRef( mxAccessible );
    if ( xRef.is() )
        xRef->MoveField(nCurPos, nNewIndex);

    return true;
}

void ScDPFieldControlBase::DeleteFieldByIndex( size_t nIndex )
{
    if (!IsExistingIndex(nIndex))
        // Nothing to delete.
        return;

    AccessRef xRef(mxAccessible);
    if (xRef.is())
        xRef->RemoveField(nIndex);


    maFieldNames.erase(maFieldNames.begin() + nIndex);
    if (mnFieldSelected >= maFieldNames.size())
        mnFieldSelected = maFieldNames.size() - 1;

    maFuncData.erase(maFuncData.begin() + nIndex);

    ResetScrollBar();
    Invalidate();
}

size_t ScDPFieldControlBase::GetFieldCount() const
{
    return maFieldNames.size();
}

bool ScDPFieldControlBase::IsEmpty() const
{
    return maFieldNames.empty();
}

void ScDPFieldControlBase::ClearFields()
{
    AccessRef xRef( mxAccessible );
    if ( xRef.is() )
        for( size_t nIdx = maFieldNames.size(); nIdx > 0; --nIdx )
            xRef->RemoveField( nIdx - 1 );

    maFieldNames.clear();
    maFuncData.clear();
}

void ScDPFieldControlBase::SetFieldText(const OUString& rText, size_t nIndex, sal_uInt8 nDupCount)
{
    if( IsExistingIndex( nIndex ) )
    {
        maFieldNames[nIndex] = FieldName(rText, true, nDupCount);
        Invalidate();

        AccessRef xRef( mxAccessible );
        if ( xRef.is() )
            xRef->FieldNameChange(nIndex);
    }
}

OUString ScDPFieldControlBase::GetFieldText( size_t nIndex ) const
{
    if( IsExistingIndex( nIndex ) )
        return maFieldNames[nIndex].maText;
    return OUString();
}

void ScDPFieldControlBase::GetExistingIndex( const Point& rPos, size_t& rnIndex )
{
    if (maFieldNames.empty() || GetFieldType() == PIVOTFIELDTYPE_SELECT)
    {
        rnIndex = 0;
        return;
    }

    rnIndex = GetFieldIndex(rPos);
    if (rnIndex == PIVOTFIELD_INVALID)
        rnIndex = 0;
}

size_t ScDPFieldControlBase::GetSelectedField() const
{
    return mnFieldSelected;
}

vector<ScDPFieldControlBase::FieldName>& ScDPFieldControlBase::GetFieldNames()
{
    return maFieldNames;
}

const vector<ScDPFieldControlBase::FieldName>& ScDPFieldControlBase::GetFieldNames() const
{
    return maFieldNames;
}

void ScDPFieldControlBase::Paint( const Rectangle& /* rRect */ )
{
    // hiding the caption is now done from StateChanged
    Redraw();
}

void ScDPFieldControlBase::StateChanged( StateChangedType nStateChange )
{
    Control::StateChanged( nStateChange );

    if( nStateChange == STATE_CHANGE_INITSHOW )
    {
        /*  After the fixed text associated to this control has received its
            unique mnemonic from VCL dialog initialization code, put this text
            into the field windows.
            #124828# Hiding the FixedTexts and clearing the tab stop style bits
            has to be done after assigning the mnemonics, but Paint() is too
            late, because the test tool may send key events to the dialog when
            it isn't visible. Mnemonics are assigned in Dialog::StateChanged()
            for STATE_CHANGE_INITSHOW, so this can be done immediately
            afterwards. */

        if ( mpCaption )
        {
            SetText( mpCaption->GetText() );
            mpCaption->Hide();
        }
    }
}

void ScDPFieldControlBase::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        Invalidate();
}

void ScDPFieldControlBase::Command( const CommandEvent& rCEvt )
{
    if (rCEvt.GetCommand() == COMMAND_WHEEL)
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if (pData->GetMode() == COMMAND_WHEEL_SCROLL && !pData->IsHorz())
        {
            // Handle vertical mouse wheel scrolls.
            long nNotch = pData->GetNotchDelta(); // positive => up; negative => down
            HandleWheelScroll(nNotch);
        }
    }
}

void ScDPFieldControlBase::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        size_t nNewSelectIndex = GetFieldIndex( rMEvt.GetPosPixel() );
        if (nNewSelectIndex != PIVOTFIELD_INVALID && IsExistingIndex(nNewSelectIndex))
        {
            // grabbing after GetFieldIndex() will prevent to focus empty window
            GrabFocusAndSelect( nNewSelectIndex );

            if( rMEvt.GetClicks() == 1 )
            {
                PointerStyle ePtr = mpDlg->NotifyMouseButtonDown( GetFieldType(), nNewSelectIndex );
                CaptureMouse();
                SetPointer( Pointer( ePtr ) );
            }
            else
                mpDlg->NotifyDoubleClick( GetFieldType(), nNewSelectIndex );
        }
    }
}

void ScDPFieldControlBase::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        if( rMEvt.GetClicks() == 1 )
        {
            Point aScrPos = OutputToScreenPixel(rMEvt.GetPosPixel());
            ScPivotFieldType eToType = mpDlg->GetFieldTypeAtPoint(aScrPos);

            mpDlg->DropFieldItem(aScrPos, eToType);
            SetPointer( Pointer( POINTER_ARROW ) );
        }

        if( IsMouseCaptured() )
            ReleaseMouse();
    }
}

void ScDPFieldControlBase::MouseMove( const MouseEvent& rMEvt )
{
    if( IsMouseCaptured() )
    {
        Point aScrPos = OutputToScreenPixel(rMEvt.GetPosPixel());
        ScPivotFieldType eFieldType = mpDlg->GetFieldTypeAtPoint(aScrPos);
        PointerStyle ePtr = mpDlg->GetPointerStyleAtPoint(aScrPos, eFieldType);
        SetPointer( Pointer( ePtr ) );
    }
    const FieldNames& rFields = GetFieldNames();
    size_t nIndex = GetFieldIndex(rMEvt.GetPosPixel());
    // does the string not fit on the screen ? show a helpful helptext instead
    if (nIndex != PIVOTFIELD_INVALID && (nIndex < rFields.size()) && !rFields[nIndex].mbFits)
    {
        Point aPos = OutputToScreenPixel( rMEvt.GetPosPixel() );
        Rectangle   aRect( aPos, GetSizePixel() );
        String aHelpText = GetFieldText(nIndex);
        Help::ShowQuickHelp( this, aRect, aHelpText );
    }
}

void ScDPFieldControlBase::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKeyCode.GetCode();


    const FieldNames& rFields = GetFieldNames();
    bool bFieldMove = ( rKeyCode.IsMod1() && (GetFieldType() != PIVOTFIELDTYPE_SELECT) );
    bool bKeyEvaluated = true;
    void (ScDPFieldControlBase::*pMoveXY) (SCsCOL nDX, SCsROW nDY);
    if (bFieldMove)
        pMoveXY = &ScDPFieldControlBase::MoveFieldRel;
    else
        pMoveXY = &ScDPFieldControlBase::MoveSelection;
    switch( nCode )
    {
    case KEY_UP:    (this->*pMoveXY)(  0, -1 ); break;
    case KEY_DOWN:  (this->*pMoveXY)(  0,  1 ); break;
    case KEY_LEFT:  (this->*pMoveXY)( -1,  0 ); break;
    case KEY_RIGHT: (this->*pMoveXY)(  1,  0 ); break;
    case KEY_HOME:
        if (bFieldMove)
            MoveField( 0 );
        else
        {
            if( !rFields.empty() )
                MoveSelection( 0 );
        }
        break;
    case KEY_END:
        if (bFieldMove)
            MoveField( rFields.size() - 1 );
        else
        {
            if( !rFields.empty() )
                MoveSelection( rFields.size() - 1 );
        }
        break;
    default:
        if ( !bFieldMove && nCode == KEY_DELETE )
            mpDlg->NotifyRemoveField( GetFieldType(), mnFieldSelected );
        else
            bKeyEvaluated = false;
        break;
    }

    if (bKeyEvaluated)
    {
        ScrollToShowSelection();
        Invalidate();
    }
    else
        Control::KeyInput( rKEvt );
}

void ScDPFieldControlBase::GetFocus()
{
    Control::GetFocus();
    Invalidate();
    if( GetGetFocusFlags() & GETFOCUS_MNEMONIC )
    {
        size_t nOldCount = GetFieldCount();
        mpDlg->NotifyMoveFieldToEnd( GetFieldType() );
        if (GetFieldCount() > nOldCount)
            // Scroll to the end only when a new field is inserted.
            ScrollToEnd();
    }
    else // notify change focus
        mpDlg->NotifyFieldFocus( GetFieldType(), true );

    AccessRef xRef( mxAccessible );
    if( xRef.is() )
        xRef->GotFocus();
}

void ScDPFieldControlBase::LoseFocus()
{
    Control::LoseFocus();
    Invalidate();
    mpDlg->NotifyFieldFocus( GetFieldType(), false );

    AccessRef xRef( mxAccessible );
    if( xRef.is() )
        xRef->LostFocus();
}

Reference<XAccessible> ScDPFieldControlBase::CreateAccessible()
{
    com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessible > xReturn(new ScAccessibleDataPilotControl(GetAccessibleParentWindow()->GetAccessible(), this));

    mxAccessible = xReturn;
    AccessRef xRef( mxAccessible );
    xRef->Init();

    return xReturn;
}

void ScDPFieldControlBase::FieldFocusChanged(size_t nOldSelected, size_t nFieldSelected)
{
    AccessRef xRef( mxAccessible );
    if ( xRef.is() )
        xRef->FieldFocusChange(nOldSelected, nFieldSelected);
}

void ScDPFieldControlBase::UpdateStyle()
{
    WinBits nMask = ~(WB_TABSTOP | WB_NOTABSTOP);
    SetStyle( (GetStyle() & nMask) | (IsEmpty() ? WB_NOTABSTOP : WB_TABSTOP) );
}

void ScDPFieldControlBase::DrawBackground( OutputDevice& rDev )
{
    const StyleSettings& rStyleSet = GetSettings().GetStyleSettings();
    Color aFaceColor = rStyleSet.GetFaceColor();
    Color aWinColor = rStyleSet.GetWindowColor();
    Color aWinTextColor = rStyleSet.GetWindowTextColor();

    Point aPos0;
    Size aSize( GetSizePixel() );

    if (mpCaption)
    {
        rDev.SetLineColor( aWinTextColor );
        rDev.SetFillColor( aWinColor );
    }
    else
    {
        rDev.SetLineColor( aFaceColor );
        rDev.SetFillColor( aFaceColor );
    }
    rDev.DrawRect( Rectangle( aPos0, aSize ) );

    rDev.SetTextColor( aWinTextColor );

    /*  Draw the caption text. This needs some special handling, because we
        support hard line breaks here. This part will draw each line of the
        text for itself. */

    xub_StrLen nTokenCnt = comphelper::string::getTokenCount(GetText(), '\n');
    long nY = (aSize.Height() - nTokenCnt * rDev.GetTextHeight()) / 2;
    sal_Int32 nStringIx = 0;
    for( xub_StrLen nToken = 0; nToken < nTokenCnt; ++nToken )
    {
        String aLine( GetText().getToken( 0, '\n', nStringIx ) );
        Point aLinePos( (aSize.Width() - rDev.GetCtrlTextWidth( aLine )) / 2, nY );
        rDev.DrawCtrlText( aLinePos, aLine );
        nY += rDev.GetTextHeight();
    }
}

void ScDPFieldControlBase::DrawField(
        OutputDevice& rDev, const Rectangle& rRect, FieldName& rText, bool bFocus )
{
    const StyleSettings& rStyleSet = GetSettings().GetStyleSettings();
    Color aTextColor = rStyleSet.GetButtonTextColor();

    VirtualDevice aVirDev( rDev );
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );

    OUString aText = rText.getDisplayedText();

    Size aDevSize( rRect.GetSize() );
    long    nWidth       = aDevSize.Width();
    long    nHeight      = aDevSize.Height();
    long    nLabelWidth  = rDev.GetTextWidth( aText );
    long    nLabelHeight = rDev.GetTextHeight();

    // #i31600# if text is too long, cut and add ellipsis
    rText.mbFits = nLabelWidth + 6 <= nWidth;
    if (!rText.mbFits)
    {
        sal_Int32 nMinLen = 0;
        sal_Int32 nMaxLen = aText.getLength();
        bool bFits = false;
        do
        {
            sal_Int32 nCurrLen = (nMinLen + nMaxLen) / 2;
            OUStringBuffer aBuf(rText.maText.copy(0, nCurrLen));
            aBuf.appendAscii("...");
            aText = aBuf.makeStringAndClear();
            nLabelWidth = rDev.GetTextWidth( aText );
            bFits = nLabelWidth + 6 <= nWidth;
            (bFits ? nMinLen : nMaxLen) = nCurrLen;
        }
        while( !bFits || (nMinLen + 1 < nMaxLen) );
    }
    Point aLabelPos( (nWidth - nLabelWidth) / 2, ::std::max< long >( (nHeight - nLabelHeight) / 2, 3 ) );

    aVirDev.SetOutputSizePixel( aDevSize );
    aVirDev.SetFont( rDev.GetFont() );
    DecorationView aDecoView( &aVirDev );
    aDecoView.DrawButton( Rectangle( Point( 0, 0 ), aDevSize ), bFocus ? BUTTON_DRAW_DEFAULT : 0 );
    aVirDev.SetTextColor( aTextColor );
    aVirDev.DrawText( aLabelPos, aText );
    rDev.DrawBitmap( rRect.TopLeft(), aVirDev.GetBitmap( Point( 0, 0 ), aDevSize ) );
}

void ScDPFieldControlBase::AppendPaintable(Window* p)
{
    maPaintables.push_back(p);
}

void ScDPFieldControlBase::DrawPaintables()
{
    Rectangle aRect(GetPosPixel(), GetSizePixel());
    Paintables::iterator itr = maPaintables.begin(), itrEnd = maPaintables.end();
    for (; itr != itrEnd; ++itr)
    {
        Window* p = *itr;
        if (!p->IsVisible())
            continue;

        p->Paint(aRect);
    }
}

void ScDPFieldControlBase::DrawInvertSelection()
{
    if (!HasFocus())
        return;

    if (mnFieldSelected >= maFieldNames.size())
        return;

    size_t nPos = GetDisplayPosition(mnFieldSelected);
    if (nPos == INVALID_INDEX)
        return;

    Size aFldSize = GetFieldSize();
    long nFldWidth = aFldSize.Width();
    long nSelWidth = std::min<long>(
        GetTextWidth(maFieldNames[mnFieldSelected].getDisplayedText()) + 4, nFldWidth - 6);

    Point aPos = GetFieldPosition(nPos);
    aPos += Point((nFldWidth - nSelWidth) / 2, 3);
    Size aSize(nSelWidth, aFldSize.Height() - 6);

    Rectangle aSel(aPos, aSize);
    InvertTracking(aSel, SHOWTRACK_SMALL | SHOWTRACK_WINDOW);
}

Size ScDPFieldControlBase::GetStdFieldBtnSize() const
{
    return mpDlg->GetStdFieldBtnSize();
}

void ScDPFieldControlBase::MoveField( size_t nDestIndex )
{
    if (nDestIndex != mnFieldSelected)
    {
        std::swap(maFieldNames[nDestIndex], maFieldNames[mnFieldSelected]);
        std::swap(maFuncData[nDestIndex], maFuncData[mnFieldSelected]);
        mnFieldSelected = nDestIndex;
    }
}

void ScDPFieldControlBase::MoveFieldRel( SCsCOL nDX, SCsROW nDY )
{
    MoveField( CalcNewFieldIndex( nDX, nDY ) );
}

void ScDPFieldControlBase::MoveSelection(size_t nIndex)
{
    FieldNames& rFields = GetFieldNames();
    if (rFields.empty())
        return;

    if (nIndex >= rFields.size())
        // Prevent it from going out-of-bound.
        nIndex = rFields.size() - 1;

    if( mnFieldSelected != nIndex )
    {
        size_t nOldSelected = mnFieldSelected;
        mnFieldSelected = nIndex;
        Invalidate();

        if (HasFocus())
            FieldFocusChanged(nOldSelected, mnFieldSelected);
    }

    ScrollToShowSelection();
}

void ScDPFieldControlBase::MoveSelection(SCsCOL nDX, SCsROW nDY)
{
    size_t nNewIndex = CalcNewFieldIndex( nDX, nDY );
    MoveSelection( nNewIndex );
}

sal_uInt8 ScDPFieldControlBase::GetNextDupCount(const OUString& rFieldText) const
{
    sal_uInt8 nMax = 0;
    FieldNames::const_iterator it = maFieldNames.begin(), itEnd = maFieldNames.end();
    for (; it != itEnd; ++it)
    {
        if (it->maText != rFieldText)
            continue;

        sal_uInt8 nNextUp = it->mnDupCount + 1;
        if (nMax < nNextUp)
            nMax = nNextUp;
    }
    return nMax;
}

sal_uInt8 ScDPFieldControlBase::GetNextDupCount(const ScPivotFuncData& rData, size_t nSelfIndex) const
{
    sal_uInt8 nDupCount = 0;
    bool bFound = false;
    for (size_t i = 0, n = maFuncData.size(); i < n; ++i)
    {
        if (i == nSelfIndex)
            // Skip itself.
            continue;

        const ScPivotFuncData& r = maFuncData[i];

        if (r.mnCol != rData.mnCol || r.mnFuncMask != rData.mnFuncMask)
            continue;

        bFound = true;
        if (r.mnDupCount > nDupCount)
            nDupCount = r.mnDupCount;
    }

    return bFound ? nDupCount + 1 : 0;
}

void ScDPFieldControlBase::SelectNext()
{
    MoveSelection(mnFieldSelected + 1);
}

void ScDPFieldControlBase::GrabFocusAndSelect( size_t nIndex )
{
    MoveSelection( nIndex );
    if( !HasFocus() )
        GrabFocus();
}

const ScPivotFuncData& ScDPFieldControlBase::GetFuncData(size_t nIndex) const
{
    return maFuncData.at(nIndex);
}

ScPivotFuncData& ScDPFieldControlBase::GetFuncData(size_t nIndex)
{
    return maFuncData.at(nIndex);
}

namespace {

class PushFuncItem : std::unary_function<ScPivotFuncData, void>
{
    std::vector<ScDPFieldControlBase::FuncItem>& mrItems;
public:
    PushFuncItem(std::vector<ScDPFieldControlBase::FuncItem>& rItems) : mrItems(rItems) {}

    void operator() (const ScPivotFuncData& r)
    {
        ScDPFieldControlBase::FuncItem aItem;
        aItem.mnCol = r.mnCol;
        aItem.mnFuncMask = r.mnFuncMask;
        mrItems.push_back(aItem);
    }
};

}

void ScDPFieldControlBase::GetAllFuncItems(std::vector<FuncItem>& rItems) const
{
    std::for_each(maFuncData.begin(), maFuncData.end(), PushFuncItem(rItems));
}

namespace {

class PivotFieldInserter : public ::std::unary_function<ScPivotFuncData, void>
{
    vector<ScPivotField>& mrFields;
public:
    explicit PivotFieldInserter(vector<ScPivotField>& r, size_t nSize) : mrFields(r)
    {
        mrFields.reserve(nSize);
    }

    PivotFieldInserter(const PivotFieldInserter& r) : mrFields(r.mrFields) {}

    void operator() (const ScPivotFuncData& r)
    {
        ScPivotField aField;
        aField.nCol = r.mnCol;
        aField.mnOriginalDim = r.mnOriginalDim;
        aField.mnDupCount = r.mnDupCount;
        aField.nFuncMask = r.mnFuncMask;
        aField.maFieldRef = r.maFieldRef;
        mrFields.push_back(aField);
    }
};

}

void ScDPFieldControlBase::ConvertToPivotArray(std::vector<ScPivotField>& rArray) const
{
    for_each(maFuncData.begin(), maFuncData.end(), PivotFieldInserter(rArray, maFuncData.size()));
}

namespace {

class EqualByDimOnly : std::unary_function<ScPivotFuncData, bool>
{
    const ScPivotFuncData& mrData;
    long mnDim;

public:
    EqualByDimOnly(const ScPivotFuncData& rData) : mrData(rData)
    {
        mnDim = rData.mnCol;
        if (rData.mnOriginalDim >= 0)
            mnDim = rData.mnOriginalDim;
    }
    bool operator() (const ScPivotFuncData& rData) const
    {
        long nDim = rData.mnCol;
        if (rData.mnOriginalDim >= 0)
            nDim = rData.mnOriginalDim;

        return nDim == mnDim;
    }
};

}

size_t ScDPFieldControlBase::GetFieldIndexByData( const ScPivotFuncData& rData ) const
{
    FuncDataType::const_iterator it = std::find_if(maFuncData.begin(), maFuncData.end(), EqualByDimOnly(rData));
    return it == maFuncData.end() ? PIVOTFIELD_INVALID : std::distance(maFuncData.begin(), it);
}

//=============================================================================

ScDPHorFieldControl::ScDPHorFieldControl(
    ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId) :
    ScDPFieldControlBase(pDialog, rResId, pCaption, pcHelpId),
    maScroll(this, WB_HORZ | WB_DRAG),
    mnFieldBtnRowCount(0),
    mnFieldBtnColCount(0)
{
    maScroll.SetScrollHdl( LINK(this, ScDPHorFieldControl, ScrollHdl) );
    maScroll.SetEndScrollHdl( LINK(this, ScDPHorFieldControl, EndScrollHdl) );
    maScroll.Hide();

    AppendPaintable(&maScroll);
}

ScDPHorFieldControl::~ScDPHorFieldControl()
{
}

Point ScDPHorFieldControl::GetFieldPosition( size_t nIndex )
{
    Point aPos;
    Size aSize;
    GetFieldBtnPosSize(nIndex, aPos, aSize);
    return aPos;
}

Size ScDPHorFieldControl::GetFieldSize() const
{
    return GetStdFieldBtnSize();
}

size_t ScDPHorFieldControl::GetFieldIndex( const Point& rPos )
{
    if (rPos.X() < 0 || rPos.Y() < 0)
        return PIVOTFIELD_INVALID;

    Size aWndSize = GetSizePixel();
    if (rPos.X() > aWndSize.Width() || rPos.Y() > aWndSize.Height())
        return PIVOTFIELD_INVALID;

    size_t nX = rPos.X();
    size_t nY = rPos.Y();
    size_t nW = aWndSize.Width();
    size_t nH = aWndSize.Height();

    Size aFldSize = GetFieldSize();
    size_t nCurX = OUTER_MARGIN_HOR + aFldSize.Width() + ROW_FIELD_BTN_GAP/2;
    size_t nCurY = OUTER_MARGIN_VER + aFldSize.Height() + ROW_FIELD_BTN_GAP/2;
    size_t nCol = 0;
    size_t nRow = 0;
    while (nX > nCurX && nCurX <= nW)
    {
        nCurX += aFldSize.Width() + ROW_FIELD_BTN_GAP;
        ++nCol;
    }
    while (nY > nCurY && nCurY <= nH)
    {
        nCurY += aFldSize.Height() + ROW_FIELD_BTN_GAP;
        ++nRow;
    }

    size_t nOffset = maScroll.GetThumbPos();
    nCol += nOffset; // convert to logical column ID.
    size_t nIndex = nCol * mnFieldBtnRowCount + nRow;
    size_t nFldCount = GetFieldCount();
    if (nIndex > nFldCount)
        nIndex = nFldCount;
    return IsValidIndex(nIndex) ? nIndex : PIVOTFIELD_INVALID;
}

void ScDPHorFieldControl::Redraw()
{
    VirtualDevice   aVirDev;
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );

    Point           aPos0;
    Size            aSize( GetSizePixel() );
    Font            aFont( GetFont() );         // Font vom Window
    aFont.SetTransparent( true );
    aVirDev.SetFont( aFont );
    aVirDev.SetOutputSizePixel( aSize );

    DrawBackground( aVirDev );

    FieldNames& rFields = GetFieldNames();
    {
        long nScrollOffset = maScroll.GetThumbPos();
        FieldNames::iterator itr = rFields.begin(), itrEnd = rFields.end();
        if (nScrollOffset)
            ::std::advance(itr, nScrollOffset*mnFieldBtnRowCount);

        for (size_t i = 0; itr != itrEnd; ++itr, ++i)
        {
            Point aFldPt;
            Size aFldSize;
            if (!GetFieldBtnPosSize(i, aFldPt, aFldSize))
                break;

            size_t nField = i + nScrollOffset*mnFieldBtnRowCount;
            bool bFocus = HasFocus() && (nField == GetSelectedField());
            DrawField(aVirDev, Rectangle(aFldPt, aFldSize), *itr, bFocus);
        }
    }

    DrawBitmap( aPos0, aVirDev.GetBitmap( aPos0, aSize ) );
    DrawPaintables();
    DrawInvertSelection();
    UpdateStyle();
}

void ScDPHorFieldControl::CalcSize()
{
    Size aWndSize = GetSizePixel();

    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    maScroll.SetSizePixel(Size(aWndSize.Width() - OUTER_MARGIN_HOR*2, nScrollSize));
    maScroll.SetPosPixel(Point(OUTER_MARGIN_HOR, aWndSize.Height() - OUTER_MARGIN_VER - nScrollSize));

    long nTotalH = aWndSize.Height() - nScrollSize - OUTER_MARGIN_VER*2;
    long nTotalW = aWndSize.Width() - OUTER_MARGIN_HOR*2;
    mnFieldBtnRowCount = nTotalH / (GetFieldSize().Height() + ROW_FIELD_BTN_GAP);
    mnFieldBtnColCount = (nTotalW + ROW_FIELD_BTN_GAP) / (GetFieldSize().Width() + ROW_FIELD_BTN_GAP);

    maScroll.SetLineSize(1);
    maScroll.SetVisibleSize(mnFieldBtnColCount);
    maScroll.SetPageSize(mnFieldBtnColCount);
    maScroll.SetRange(Range(0, mnFieldBtnColCount));
}

bool ScDPHorFieldControl::IsValidIndex(size_t /*nIndex*/) const
{
    return true;
}

size_t ScDPHorFieldControl::CalcNewFieldIndex(SCsCOL nDX, SCsROW nDY) const
{
    size_t nSel = GetSelectedField();
    size_t nFldCount = GetFieldCount();
    SCsROW nRow = nSel % mnFieldBtnRowCount;
    SCsCOL nCol = nSel / mnFieldBtnRowCount;
    SCsCOL nColUpper = static_cast<SCsCOL>(ceil(
        static_cast<double>(nFldCount) / static_cast<double>(mnFieldBtnRowCount)) - 1);
    SCsROW nRowUpper = mnFieldBtnRowCount - 1;

    nCol += nDX;
    if (nCol < 0)
        nCol = 0;
    else if (nColUpper < nCol)
        nCol = nColUpper;
    nRow += nDY;
    if (nRow < 0)
        nRow = 0;
    else if (nRowUpper < nRow)
        nRow = nRowUpper;

    nSel = nCol*mnFieldBtnRowCount + nRow;
    if (nSel >= nFldCount)
        nSel = nFldCount - 1;

    return nSel;
}

size_t ScDPHorFieldControl::GetDisplayPosition(size_t nIndex) const
{
    size_t nColFirst = maScroll.GetThumbPos();
    size_t nColLast = nColFirst + mnFieldBtnColCount - 1;
    size_t nCol = nIndex / mnFieldBtnRowCount;
    size_t nRow = nIndex % mnFieldBtnRowCount;
    if (nCol < nColFirst || nColLast < nCol)
        // index is outside the visible area.
        return INVALID_INDEX;

    size_t nPos = (nCol - nColFirst)*mnFieldBtnRowCount + nRow;
    return nPos;
}

String ScDPHorFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_COL_DESCR);
}

void ScDPHorFieldControl::ScrollToEnd()
{
    maScroll.DoScroll(maScroll.GetRangeMax());
}

void ScDPHorFieldControl::ScrollToShowSelection()
{
    size_t nLower = maScroll.GetThumbPos();
    size_t nUpper = nLower + mnFieldBtnColCount - 1;
    size_t nCol = GetSelectedField() / mnFieldBtnRowCount;
    if (nCol < nLower)
    {
        // scroll to left.
        maScroll.DoScroll(nCol);
    }
    else if (nUpper < nCol)
    {
        // scroll to right.
        maScroll.DoScroll(nCol - mnFieldBtnColCount + 1);
    }
}

void ScDPHorFieldControl::ResetScrollBar()
{
    long nOldMax = maScroll.GetRangeMax();
    long nNewMax = static_cast<long>(ceil(
        static_cast<double>(GetFieldCount()) / static_cast<double>(mnFieldBtnRowCount)));

    if (nOldMax != nNewMax)
    {
        maScroll.SetRangeMax(nNewMax);
        bool bShow = mnFieldBtnColCount*mnFieldBtnRowCount < GetFieldCount();
        maScroll.Show(bShow);
    }
}

void ScDPHorFieldControl::HandleWheelScroll(long /*nNotch*/)
{
    // not handled for horizontal field controls.
}

bool ScDPHorFieldControl::GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize)
{
    if (nPos >= mnFieldBtnColCount*mnFieldBtnRowCount)
        return false;

    Point aPos = Point(OUTER_MARGIN_HOR, OUTER_MARGIN_VER);
    size_t nRow = nPos % mnFieldBtnRowCount;
    size_t nCol = nPos / mnFieldBtnRowCount;

    aPos.X() += nCol*(GetFieldSize().Width() + ROW_FIELD_BTN_GAP);
    aPos.Y() += nRow*(GetFieldSize().Height() + ROW_FIELD_BTN_GAP);

    rPos = aPos;
    rSize = GetFieldSize();
    return true;
}

void ScDPHorFieldControl::HandleScroll()
{
    Redraw();
}

IMPL_LINK_NOARG(ScDPHorFieldControl, ScrollHdl)
{
    HandleScroll();
    return 0;
}

IMPL_LINK_NOARG(ScDPHorFieldControl, EndScrollHdl)
{
    HandleScroll();
    return 0;
}

//=============================================================================

ScDPPageFieldControl::ScDPPageFieldControl(
    ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId) :
    ScDPHorFieldControl(pDialog, rResId, pCaption, pcHelpId)
{
}

ScDPPageFieldControl::~ScDPPageFieldControl()
{
}

ScPivotFieldType ScDPPageFieldControl::GetFieldType() const
{
    return PIVOTFIELDTYPE_PAGE;
}

String ScDPPageFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_PAGE_DESCR);
}

//=============================================================================

ScDPColFieldControl::ScDPColFieldControl(
    ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId) :
    ScDPHorFieldControl(pDialog, rResId, pCaption, pcHelpId)
{
}

ScDPColFieldControl::~ScDPColFieldControl()
{
}

ScPivotFieldType ScDPColFieldControl::GetFieldType() const
{
    return PIVOTFIELDTYPE_COL;
}

String ScDPColFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_COL_DESCR);
}

//=============================================================================

ScDPRowFieldControl::ScDPRowFieldControl(
    ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId) :
    ScDPFieldControlBase(pDialog, rResId, pCaption, pcHelpId),
    maScroll(this, WB_VERT | WB_DRAG),
    mnColumnBtnCount(0)
{
    maScroll.SetScrollHdl( LINK(this, ScDPRowFieldControl, ScrollHdl) );
    maScroll.SetEndScrollHdl( LINK(this, ScDPRowFieldControl, EndScrollHdl) );
    maScroll.Show(false);

    AppendPaintable(&maScroll);
}

ScDPRowFieldControl::~ScDPRowFieldControl()
{
}

//-------------------------------------------------------------------

Point ScDPRowFieldControl::GetFieldPosition(size_t nIndex)
{
    Point aPos;
    Size aSize;
    GetFieldBtnPosSize(nIndex, aPos, aSize);
    return aPos;
}

Size ScDPRowFieldControl::GetFieldSize() const
{
    return GetStdFieldBtnSize();
}

size_t ScDPRowFieldControl::GetFieldIndex( const Point& rPos )
{
    if (rPos.X() < 0 || rPos.Y() < 0)
        return PIVOTFIELD_INVALID;

    long nFldH = GetFieldSize().Height();
    long nThreshold = OUTER_MARGIN_VER + nFldH + ROW_FIELD_BTN_GAP / 2;

    size_t nIndex = 0;
    for (; nIndex < mnColumnBtnCount; ++nIndex)
    {
        if (rPos.Y() < nThreshold)
            break;

        nThreshold += nFldH + ROW_FIELD_BTN_GAP;
    }

    if (nIndex >= mnColumnBtnCount)
        nIndex = mnColumnBtnCount - 1;

    nIndex += maScroll.GetThumbPos();
    return IsValidIndex(nIndex) ? nIndex : PIVOTFIELD_INVALID;
}

void ScDPRowFieldControl::Redraw()
{
    VirtualDevice   aVirDev;
    // #i97623# VirtualDevice is always LTR while other windows derive direction from parent
    aVirDev.EnableRTL( IsRTLEnabled() );
    aVirDev.SetMapMode( MAP_PIXEL );

    Point aPos0;
    Size aWndSize = GetSizePixel();
    Font aFont = GetFont();
    aFont.SetTransparent(true);
    aVirDev.SetFont(aFont);
    aVirDev.SetOutputSizePixel(aWndSize);

    DrawBackground(aVirDev);

    FieldNames& rFields = GetFieldNames();
    {
        long nScrollOffset = maScroll.GetThumbPos();
        FieldNames::iterator itr = rFields.begin(), itrEnd = rFields.end();
        if (nScrollOffset)
            ::std::advance(itr, nScrollOffset);

        for (size_t i = 0; itr != itrEnd; ++itr, ++i)
        {
            Point aFldPt;
            Size aFldSize;
            if (!GetFieldBtnPosSize(i, aFldPt, aFldSize))
                break;

            size_t nField = i + nScrollOffset;
            bool bFocus = HasFocus() && (nField == GetSelectedField());
            DrawField(aVirDev, Rectangle(aFldPt, aFldSize), *itr, bFocus);
        }
    }

    // Create a bitmap from the virtual device, and place that bitmap onto
    // this control.
    DrawBitmap(aPos0, aVirDev.GetBitmap(aPos0, aWndSize));

    DrawPaintables();
    DrawInvertSelection();
    UpdateStyle();
}

void ScDPRowFieldControl::CalcSize()
{
    Size aWndSize = GetSizePixel();

    long nTotal = aWndSize.Height() - OUTER_MARGIN_VER;
    mnColumnBtnCount = nTotal / (GetFieldSize().Height() + ROW_FIELD_BTN_GAP);

    long nScrollSize = GetSettings().GetStyleSettings().GetScrollBarSize();

    maScroll.SetSizePixel(Size(nScrollSize, aWndSize.Height() - OUTER_MARGIN_VER*2));
    maScroll.SetPosPixel(Point(aWndSize.Width() - nScrollSize - OUTER_MARGIN_HOR, OUTER_MARGIN_VER));
    maScroll.SetLineSize(1);
    maScroll.SetVisibleSize(mnColumnBtnCount);
    maScroll.SetPageSize(mnColumnBtnCount);
    maScroll.SetRange(Range(0, mnColumnBtnCount));
    maScroll.DoScroll(0);

}

bool ScDPRowFieldControl::IsValidIndex(size_t /*nIndex*/) const
{
    // This method is here in case we decide to impose an arbitrary upper
    // boundary on the number of fields.
    return true;
}

size_t ScDPRowFieldControl::CalcNewFieldIndex(SCsCOL /*nDX*/, SCsROW nDY) const
{
    size_t nNewField = GetSelectedField();
    nNewField += nDY;
    return IsExistingIndex(nNewField) ? nNewField : GetSelectedField();
}

size_t ScDPRowFieldControl::GetDisplayPosition(size_t nIndex) const
{
    size_t nLower = maScroll.GetThumbPos();
    size_t nUpper = nLower + mnColumnBtnCount;
    if (nLower <= nIndex && nIndex <= nUpper)
        return nIndex - nLower;

    return INVALID_INDEX;
}

//-------------------------------------------------------------------

String ScDPRowFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_ROW_DESCR);
}

ScPivotFieldType ScDPRowFieldControl::GetFieldType() const
{
    return PIVOTFIELDTYPE_ROW;
}

void ScDPRowFieldControl::ScrollToEnd()
{
    maScroll.DoScroll(maScroll.GetRangeMax());
}

void ScDPRowFieldControl::ScrollToShowSelection()
{
    size_t nLower = maScroll.GetThumbPos();
    size_t nUpper = nLower + mnColumnBtnCount - 1;
    size_t nSel = GetSelectedField();
    if (nSel < nLower)
    {
        // scroll up
        maScroll.DoScroll(nSel);
    }
    else if (nUpper < nSel)
    {
        // scroll down
        size_t nD = nSel - nUpper;
        maScroll.DoScroll(nLower + nD);
    }
}

void ScDPRowFieldControl::ResetScrollBar()
{
    long nOldMax = maScroll.GetRangeMax();
    long nNewMax = std::max<long>(mnColumnBtnCount, GetFieldCount());

    if (nOldMax != nNewMax)
    {
        maScroll.SetRangeMax(nNewMax);
        maScroll.Show(GetFieldCount() > mnColumnBtnCount);
    }
}

void ScDPRowFieldControl::HandleWheelScroll(long nNotch)
{
    maScroll.DoScroll(maScroll.GetThumbPos() - nNotch);
}

bool ScDPRowFieldControl::GetFieldBtnPosSize(size_t nPos, Point& rPos, Size& rSize)
{
    if (nPos >= mnColumnBtnCount)
        return false;

    size_t nOffset = maScroll.GetThumbPos();
    if (nPos + nOffset >= GetFieldCount())
        return false;

    rSize = GetFieldSize();
    rPos = Point(OUTER_MARGIN_HOR, OUTER_MARGIN_VER);
    rPos.Y() += nPos * (rSize.Height() + ROW_FIELD_BTN_GAP);
    return true;
}

void ScDPRowFieldControl::HandleScroll()
{
    Redraw();
}

IMPL_LINK_NOARG(ScDPRowFieldControl, ScrollHdl)
{
    HandleScroll();
    return 0;
}

IMPL_LINK_NOARG(ScDPRowFieldControl, EndScrollHdl)
{
    HandleScroll();
    return 0;
}

//=============================================================================

ScDPSelectFieldControl::ScDPSelectFieldControl(
        ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId) :
    ScDPHorFieldControl(pDialog, rResId, pCaption, pcHelpId)
{
    SetName(String(ScResId(STR_SELECT)));
}

ScDPSelectFieldControl::~ScDPSelectFieldControl()
{
}

ScPivotFieldType ScDPSelectFieldControl::GetFieldType() const
{
    return PIVOTFIELDTYPE_SELECT;
}

String ScDPSelectFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_SEL_DESCR);
}

//=============================================================================

ScDPDataFieldControl::ScDPDataFieldControl(
    ScPivotLayoutDlg* pDialog, const ResId& rResId, FixedText* pCaption, const char* pcHelpId) :
    ScDPHorFieldControl(pDialog, rResId, pCaption, pcHelpId)
{
}

ScDPDataFieldControl::~ScDPDataFieldControl()
{
}

ScPivotFieldType ScDPDataFieldControl::GetFieldType() const
{
    return PIVOTFIELDTYPE_DATA;
}

Size ScDPDataFieldControl::GetFieldSize() const
{
    Size aWndSize = GetSizePixel();
    long nFieldObjWidth = static_cast<long>(aWndSize.Width() / 2.0 - OUTER_MARGIN_HOR - DATA_FIELD_BTN_GAP/2);
    Size aFieldSize(nFieldObjWidth, FIELD_BTN_HEIGHT);
    return aFieldSize;
}

String ScDPDataFieldControl::GetDescription() const
{
    return ScResId(STR_ACC_DATAPILOT_DATA_DESCR);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
