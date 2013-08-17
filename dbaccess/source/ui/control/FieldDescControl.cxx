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

#include "FieldDescControl.hxx"
#include "FieldControls.hxx"
#include <tools/diagnose_ex.h>
#include "TableDesignHelpBar.hxx"
#include <vcl/scrbar.hxx>
#include <vcl/button.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>
#include <vcl/msgbox.hxx>
#include <vector>
#include "FieldDescriptions.hxx"
#include "dlgattr.hxx"
#include <svx/numfmtsh.hxx>
#include <svx/svxids.hrc>
#include <svx/algitem.hxx>
#include <svl/itempool.hxx>
#include <svl/zforlist.hxx>
#include <svl/rngitem.hxx>
#include <svl/intitem.hxx>
#include <svl/numuno.hxx>
#include <svtools/transfer.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "QEnumTypes.hxx"
#include "dbaccess_helpid.hrc"
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/string.hxx>
#include "UITools.hxx"
#include <memory>
#include "dbu_control.hrc"
#include "dbu_tbl.hrc"
#include <osl/diagnose.h>

using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;

// For the Controls on the OFieldDescGenPage
#define CONTROL_SPACING_X   18  // 6
#define CONTROL_SPACING_Y   4
#define CONTROL_WIDTH_1     160 // 100
#define CONTROL_WIDTH_2     100 // 60
#define CONTROL_WIDTH_3     250
#define CONTROL_WIDTH_4     (CONTROL_WIDTH_3 - 20 - 5)

#define HSCROLL_STEP        20

namespace
{

    template< typename T1, typename T2> void lcl_HideAndDeleteControl(short& _nPos,T1** _pControl,T2** _pControlText)
    {
        if ( *_pControl )
        {
            --_nPos;
            (*_pControl)->Hide();
            (*_pControlText)->Hide();
            delete *_pControl;
            delete *_pControlText;
            (*_pControl) = NULL;
            (*_pControlText) = NULL;
        }
    }

}

// class OFieldDescControl

DBG_NAME(OFieldDescControl)

OFieldDescControl::OFieldDescControl( Window* pParent, const ResId& rResId, OTableDesignHelpBar* pHelpBar)
    :TabPage( pParent, rResId )
    ,pHelp( pHelpBar )
    ,pLastFocusWindow(NULL)
    ,m_pActFocusWindow(NULL)
    ,pDefaultText(NULL)
    ,pRequiredText(NULL)
    ,pAutoIncrementText(NULL)
    ,pTextLenText(NULL)
    ,pNumTypeText(NULL)
    ,pLengthText(NULL)
    ,pScaleText(NULL)
    ,pFormatText(NULL)
    ,pBoolDefaultText(NULL)
    ,m_pColumnNameText(NULL)
    ,m_pTypeText(NULL)
    ,m_pAutoIncrementValueText(NULL)
    ,pRequired(NULL)
    ,pNumType(NULL)
    ,pAutoIncrement(NULL)
    ,pDefault(NULL)
    ,pTextLen(NULL)
    ,pLength(NULL)
    ,pScale(NULL)
    ,pFormatSample(NULL)
    ,pBoolDefault(NULL)
    ,m_pColumnName(NULL)
    ,m_pType(NULL)
    ,m_pAutoIncrementValue(NULL)
    ,pFormat(NULL)
    ,m_pVertScroll( NULL )
    ,m_pHorzScroll( NULL )
    ,m_pPreviousType()
    ,m_nPos(-1)
    ,aYes(ModuleRes(STR_VALUE_YES))
    ,aNo(ModuleRes(STR_VALUE_NO))
    ,m_nOldVThumb( 0 )
    ,m_nOldHThumb( 0 )
    ,m_nWidth(50)
    ,m_bAdded(sal_False)
    ,m_bRightAligned(false)
    ,pActFieldDescr(NULL)
{
    DBG_CTOR(OFieldDescControl,NULL);

    Contruct();
}

OFieldDescControl::OFieldDescControl( Window* pParent, OTableDesignHelpBar* pHelpBar )
    :TabPage( pParent, WB_3DLOOK | WB_DIALOGCONTROL )
    ,pHelp( pHelpBar )
    ,pLastFocusWindow(NULL)
    ,m_pActFocusWindow(NULL)
    ,pDefaultText(NULL)
    ,pRequiredText(NULL)
    ,pAutoIncrementText(NULL)
    ,pTextLenText(NULL)
    ,pNumTypeText(NULL)
    ,pLengthText(NULL)
    ,pScaleText(NULL)
    ,pFormatText(NULL)
    ,pBoolDefaultText(NULL)
    ,m_pColumnNameText(NULL)
    ,m_pTypeText(NULL)
    ,m_pAutoIncrementValueText(NULL)
    ,pRequired(NULL)
    ,pNumType(NULL)
    ,pAutoIncrement(NULL)
    ,pDefault(NULL)
    ,pTextLen(NULL)
    ,pLength(NULL)
    ,pScale(NULL)
    ,pFormatSample(NULL)
    ,pBoolDefault(NULL)
    ,m_pColumnName(NULL)
    ,m_pType(NULL)
    ,m_pAutoIncrementValue(NULL)
    ,pFormat(NULL)
    ,m_pVertScroll( NULL )
    ,m_pHorzScroll( NULL )
    ,m_pPreviousType()
    ,m_nPos(-1)
    ,aYes(ModuleRes(STR_VALUE_YES))
    ,aNo(ModuleRes(STR_VALUE_NO))
    ,m_nOldVThumb( 0 )
    ,m_nOldHThumb( 0 )
    ,m_nWidth(50)
    ,m_bAdded(sal_False)
    ,m_bRightAligned(false)
    ,pActFieldDescr(NULL)
{
    DBG_CTOR(OFieldDescControl,NULL);
    Contruct();
}

void OFieldDescControl::Contruct()
{
    m_pVertScroll = new ScrollBar(this, WB_VSCROLL | WB_REPEAT | WB_DRAG);
    m_pHorzScroll = new ScrollBar(this, WB_HSCROLL | WB_REPEAT | WB_DRAG);
    m_pVertScroll->SetScrollHdl(LINK(this, OFieldDescControl, OnScroll));
    m_pHorzScroll->SetScrollHdl(LINK(this, OFieldDescControl, OnScroll));
    m_pVertScroll->Show();
    m_pHorzScroll->Show();

    m_pVertScroll->EnableClipSiblings();
    m_pHorzScroll->EnableClipSiblings();

    m_pVertScroll->SetLineSize(1);
    m_pVertScroll->SetPageSize(1);
    m_pHorzScroll->SetLineSize(1);
    m_pHorzScroll->SetPageSize(1);

    m_nOldVThumb = m_nOldHThumb = 0;
}

OFieldDescControl::~OFieldDescControl()
{
    DBG_DTOR(OFieldDescControl,NULL);

    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pVertScroll);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pVertScroll    = NULL;
    }
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pHorzScroll);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pHorzScroll    = NULL;
    }
    if ( m_bAdded )
        ::dbaui::notifySystemWindow(this,this,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    pLastFocusWindow = NULL;

    // Destroy children
    DeactivateAggregate( tpDefault );
    DeactivateAggregate( tpRequired );
    DeactivateAggregate( tpTextLen );
    DeactivateAggregate( tpNumType );
    DeactivateAggregate( tpScale );
    DeactivateAggregate( tpLength );
    DeactivateAggregate( tpFormat );
    DeactivateAggregate( tpAutoIncrement );
    DeactivateAggregate( tpBoolDefault );
    DeactivateAggregate( tpColumnName );
    DeactivateAggregate( tpType );
    DeactivateAggregate( tpAutoIncrementValue );
}

String OFieldDescControl::BoolStringPersistent(const String& rUIString) const
{
    if (rUIString == aNo)
        return OUString('0');
    if (rUIString == aYes)
        return OUString('1');
    return OUString();
}

String OFieldDescControl::BoolStringUI(const String& rPersistentString) const
{
    // Older versions may store a language dependend string as a default
    if (rPersistentString.Equals(aYes) || rPersistentString.Equals(aNo))
        return rPersistentString;

    if (comphelper::string::equals(rPersistentString, '0'))
        return aNo;
    if (comphelper::string::equals(rPersistentString, '1'))
        return aYes;

    return ModuleRes(STR_VALUE_NONE).toString();
}

void OFieldDescControl::Init()
{
    Reference< ::com::sun::star::util::XNumberFormatter > xFormatter = GetFormatter();
    ::dbaui::setEvalDateFormatForFormatter(xFormatter);
}

IMPL_LINK(OFieldDescControl, OnScroll, ScrollBar*, /*pBar*/)
{
    ScrollAllAggregates();
    return 0;
}

namespace
{
    void getMaxXPosition(Window* _pWindow,long& _rnMaxXPosition)
    {
        if (_pWindow)
        {
            long nTemp = _pWindow->GetSizePixel().Width() + _pWindow->GetPosPixel().X();
            _rnMaxXPosition = ::std::max(_rnMaxXPosition, nTemp);
        }
    }
}

void OFieldDescControl::CheckScrollBars()
{
    // Calculate the ScrollBars' new position
    Size szOverallSize = GetSizePixel();
    long nHScrollHeight = m_pHorzScroll->GetSizePixel().Height();
    long nVScrollWidth = m_pVertScroll->GetSizePixel().Width();

    long nNewHWidth = szOverallSize.Width() - nVScrollWidth;
    long nNewVHeight = szOverallSize.Height() - nHScrollHeight;

    sal_Bool bNeedHScrollBar(sal_False), bNeedVScrollBar(sal_False);

    // Adjust the areas
    // Do I actually need ScrollBars?
    // horizontal :
    long lMaxXPosition = 0;
    Control* ppAggregates[] = { pRequired, pNumType, pAutoIncrement, pDefault, pTextLen, pLength, pScale, pFormat, m_pColumnName, m_pType,m_pAutoIncrementValue};
    for (sal_uInt16 i=0; i<sizeof(ppAggregates)/sizeof(ppAggregates[0]); ++i)
        getMaxXPosition(ppAggregates[i],lMaxXPosition);

    if (m_pHorzScroll)
        lMaxXPosition += m_pHorzScroll->GetThumbPos() * HSCROLL_STEP;

    long lMaxXAvailable = szOverallSize.Width();
    bNeedHScrollBar = lMaxXPosition > lMaxXAvailable;
        // Might change

    // Vertical
    // How many Controls do I have?
    sal_uInt16 nActive = CountActiveAggregates();
    // Which one is the last one that fits?
    sal_uInt16 nLastVisible;
    const sal_Int32 nControlHeight = GetMaxControlHeight();
    const sal_Int32 nControl_Spacing_y = LogicToPixel(Size(0, CONTROL_SPACING_Y),MAP_APPFONT).Height();
    if (bNeedHScrollBar)
        nLastVisible = static_cast<sal_uInt16>((szOverallSize.Height() - nControl_Spacing_y - nHScrollHeight) / (nControl_Spacing_y + nControlHeight));
    else
        nLastVisible = static_cast<sal_uInt16>((szOverallSize.Height() - nControl_Spacing_y) / (nControl_Spacing_y + nControlHeight));
    bNeedVScrollBar = nActive>nLastVisible;

    if (bNeedVScrollBar)
    {
        // When originally calculating lMaxXAvailable we did not take into account that we have a VScrollBar, so we need to do that now
        lMaxXAvailable -= nVScrollWidth;
        if (!bNeedHScrollBar && (lMaxXPosition > lMaxXAvailable))
        {
            // The vertical one now necessitates a horizontal one
            bNeedHScrollBar = sal_True;
            // Adjust nLastVisible
            nLastVisible = static_cast<sal_uInt16>((szOverallSize.Height() - nControl_Spacing_y - nHScrollHeight) / (nControl_Spacing_y + nControlHeight));
                // bNeedVScrollBar does NOT change: it's already set to sal_True and nLastVisible will only decrease
        }
    }

    // Now we can really position them and set their parameters
    if (bNeedVScrollBar)
    {
        m_pVertScroll->Show();
        m_pVertScroll->SetRangeMax(nActive - nLastVisible);

        m_pVertScroll->SetPosSizePixel( Point(nNewHWidth, 0), Size(nVScrollWidth, szOverallSize.Height()) );
    }
    else
    {
        m_pVertScroll->Hide();
        m_pVertScroll->SetRangeMax(0);
        m_pVertScroll->SetThumbPos(0);
    }

    if (bNeedHScrollBar)
    {
        m_pHorzScroll->Show();
        m_pHorzScroll->SetRangeMax((lMaxXPosition - lMaxXAvailable + HSCROLL_STEP - 1 )/HSCROLL_STEP);

        m_pHorzScroll->SetPosSizePixel( Point(0, nNewVHeight), Size(bNeedVScrollBar ? nNewHWidth : szOverallSize.Width(), nHScrollHeight) );
    }
    else
    {
        m_pHorzScroll->Hide();
        m_pHorzScroll->SetRangeMax(0);
        m_pHorzScroll->SetThumbPos(0);
    }
}

void OFieldDescControl::Resize()
{
    CheckScrollBars();
    ScrollAllAggregates();
}

inline void OFieldDescControl::ScrollAggregate(Control* pText, Control* pInput, Control* pButton, long nDeltaX, long nDeltaY)
{
    if  (!pText)
        return;
    pText->SetPosPixel(pText->GetPosPixel() + Point(nDeltaX, nDeltaY));
    pInput->SetPosPixel(pInput->GetPosPixel() + Point(nDeltaX, nDeltaY));
    if (pButton)
        pButton->SetPosPixel(pButton->GetPosPixel() + Point(nDeltaX, nDeltaY));
}

void OFieldDescControl::ScrollAllAggregates()
{
    long nDeltaX = 0, nDeltaY = 0;
    if (m_nOldHThumb != m_pHorzScroll->GetThumbPos())
    {
        nDeltaX = (m_nOldHThumb - m_pHorzScroll->GetThumbPos()) * HSCROLL_STEP;
        m_nOldHThumb = m_pHorzScroll->GetThumbPos();
    }

    if (m_nOldVThumb != m_pVertScroll->GetThumbPos())
    {
        const sal_Int32 nControlHeight = GetMaxControlHeight();
        const sal_Int32 nControl_Spacing_y = LogicToPixel(Size(0, CONTROL_SPACING_Y),MAP_APPFONT).Height();
        nDeltaY = (m_nOldVThumb - m_pVertScroll->GetThumbPos()) * (nControl_Spacing_y + nControlHeight);
        m_nOldVThumb = m_pVertScroll->GetThumbPos();
    }

    if (nDeltaX || nDeltaY)
    {
        Control* ppAggregates[]     = {   pRequired, pNumType
                                        , pAutoIncrement, pDefault
                                        , pTextLen, pLength
                                        , pScale, m_pColumnName
                                        , m_pType, m_pAutoIncrementValue};
        Control* ppAggregatesText[] = {   pRequiredText, pNumTypeText
                                        , pAutoIncrementText, pDefaultText
                                        , pTextLenText, pLengthText
                                        , pScaleText, m_pColumnNameText
                                        , m_pTypeText, m_pAutoIncrementValueText};
        OSL_ENSURE(sizeof(ppAggregates)/sizeof(ppAggregates[0]) == sizeof(ppAggregatesText)/sizeof(ppAggregatesText[0]),"Lists are not identical!");

        for (sal_uInt16 i=0; i<sizeof(ppAggregates)/sizeof(ppAggregates[0]); ++i)
            ScrollAggregate(ppAggregatesText[i],ppAggregates[i],NULL,nDeltaX, nDeltaY);

        ScrollAggregate(pFormatText,pFormatSample,pFormat,nDeltaX, nDeltaY);
    }
}

sal_uInt16 OFieldDescControl::CountActiveAggregates() const
{
    Control* ppAggregates[] = { pRequired, pNumType, pAutoIncrement, pDefault, pTextLen, pLength, pScale, pFormat, m_pColumnName, m_pType,m_pAutoIncrementValue};
    sal_uInt16 nVisibleAggregates = 0;
    for (sal_uInt16 i=0; i<sizeof(ppAggregates)/sizeof(ppAggregates[0]); ++i)
        if (ppAggregates[i])
            ++nVisibleAggregates;
    return nVisibleAggregates;
}

sal_Int32 OFieldDescControl::GetMaxControlHeight() const
{
    Size aHeight;
    Control* ppAggregates[] = { pRequired, pNumType, pAutoIncrement, pDefault, pTextLen, pLength, pScale, pFormat, m_pColumnName, m_pType,m_pAutoIncrementValue};
    for (sal_uInt16 i=0; i<sizeof(ppAggregates)/sizeof(ppAggregates[0]); ++i)
    {
        if ( ppAggregates[i] )
        {
            const Size aTemp(ppAggregates[i]->GetOptimalSize());
            if ( aTemp.Height() > aHeight.Height() )
                aHeight.Height() = aTemp.Height();
        }
    }

    return aHeight.Height();
}

void OFieldDescControl::SetReadOnly( sal_Bool bReadOnly )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // Enable/disable Controls
    Control* ppAggregates[]     = {   pRequired, pNumType
                                        , pAutoIncrement, pDefault
                                        , pTextLen, pLength
                                        , pScale, m_pColumnName
                                        , m_pType, m_pAutoIncrementValue
                                        , pFormat};
    Control* ppAggregatesText[] = {   pRequiredText, pNumTypeText
                                        , pAutoIncrementText, pDefaultText
                                        , pTextLenText, pLengthText
                                        , pScaleText, m_pColumnNameText
                                        , m_pTypeText, m_pAutoIncrementValueText
                                        , pFormatText};

    OSL_ENSURE(sizeof(ppAggregates)/sizeof(ppAggregates[0]) == sizeof(ppAggregatesText)/sizeof(ppAggregatesText[0]),"Lists are not identical!");

    for (sal_uInt16 i=0; i<sizeof(ppAggregates)/sizeof(ppAggregates[0]); ++i)
    {
        if ( ppAggregatesText[i] )
            ppAggregatesText[i]->Enable( !bReadOnly );
        if ( ppAggregates[i] )
            ppAggregates[i]->Enable( !bReadOnly );
    }
}

String OFieldDescControl::GetControlText( sal_uInt16 nControlId )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // Read out the Controls' texts
    switch( nControlId )
    {
        case FIELD_PROPERTY_BOOL_DEFAULT:
            if (pBoolDefault)
                return pBoolDefault->GetSelectEntry();
            break;
        case FIELD_PROPERTY_DEFAULT:
            if (pDefault)
                return pDefault->GetText();
            break;
        case FIELD_PROPERTY_REQUIRED:
            if (pRequired)
                return pRequired->GetSelectEntry();
            break;
        case FIELD_PROPERTY_TEXTLEN:
            if (pTextLen)
                return OUString::number(pTextLen->GetValue());
        case FIELD_PROPERTY_NUMTYPE:
            if (pNumType)
                return pNumType->GetSelectEntry();
            break;
        case FIELD_PROPERTY_AUTOINC:
            if (pAutoIncrement)
                return pAutoIncrement->GetSelectEntry();
            break;
        case FIELD_PROPERTY_LENGTH:
            if (pLength)
                return pLength->GetText();
            break;
        case FIELD_PROPERTY_SCALE:
            if (pScale)
                return pScale->GetText();
            break;
        case FIELD_PROPERTY_FORMAT:
            if (pFormatSample)
                return pFormatSample->GetText();
            break;
        case FIELD_PRPOERTY_COLUMNNAME:
            if(m_pColumnName)
                return m_pColumnName->GetText();
        case FIELD_PRPOERTY_TYPE:
            if(m_pType)
                return m_pType->GetSelectEntry();
            break;
        case FIELD_PRPOERTY_AUTOINCREMENT:
            if(m_pAutoIncrementValue)
                return m_pAutoIncrementValue->GetText();
    }

    return String();
}

void OFieldDescControl::SetControlText( sal_uInt16 nControlId, const String& rText )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // Set the Controls' texts
    switch( nControlId )
    {
        case FIELD_PROPERTY_BOOL_DEFAULT:
            if (pBoolDefault)
            {
                String sOld = pBoolDefault->GetSelectEntry();
                pBoolDefault->SelectEntry(rText);
                if (!sOld.Equals(rText))
                    LINK(this, OFieldDescControl, ChangeHdl).Call(pBoolDefault);
            }
            break;
        case FIELD_PROPERTY_DEFAULT:
            if (pDefault)
            {
                pDefault->SetText(rText);
                UpdateFormatSample(pActFieldDescr);
            }
            break;

        case FIELD_PROPERTY_REQUIRED:
            if (pRequired)
                pRequired->SelectEntry(rText);
            break;

        case FIELD_PROPERTY_TEXTLEN:
            if (pTextLen)
                pTextLen->SetText(rText);
            break;

        case FIELD_PROPERTY_NUMTYPE:
            if (pNumType)
                pNumType->SelectEntry(rText);
            break;

        case FIELD_PROPERTY_AUTOINC:
            if (pAutoIncrement)
            {
                String sOld = pAutoIncrement->GetSelectEntry();
                pAutoIncrement->SelectEntry(rText);
                if (!sOld.Equals(rText))
                    LINK(this, OFieldDescControl, ChangeHdl).Call(pAutoIncrement);
            }
            break;

        case FIELD_PROPERTY_LENGTH:
            if (pLength)
                pLength->SetText(rText);
            break;

        case FIELD_PROPERTY_SCALE:
            if (pScale)
                pScale->SetText(rText);
            break;

        case FIELD_PROPERTY_FORMAT:
            if (pActFieldDescr)
                UpdateFormatSample(pActFieldDescr);
            break;
        case FIELD_PRPOERTY_COLUMNNAME:
            if(m_pColumnName)
                m_pColumnName->SetText(rText);
            break;
        case FIELD_PRPOERTY_TYPE:
            if(m_pType)
                m_pType->SelectEntry(rText);
            break;
        case FIELD_PRPOERTY_AUTOINCREMENT:
            if(m_pAutoIncrementValue)
                m_pAutoIncrementValue->SetText(rText);
            break;
    }
}

IMPL_LINK( OFieldDescControl, FormatClickHdl, Button *, /*pButton*/ )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // Create temporary Column, which is used for data exchange with Dialog
    if( !pActFieldDescr )
        return 0;

    sal_Int32 nOldFormatKey(pActFieldDescr->GetFormatKey());
    SvxCellHorJustify rOldJustify = pActFieldDescr->GetHorJustify();
    Reference< XNumberFormatsSupplier >  xSupplier = GetFormatter()->getNumberFormatsSupplier();
    SvNumberFormatsSupplierObj* pSupplierImpl = SvNumberFormatsSupplierObj::getImplementation( xSupplier );

    SvNumberFormatter* pFormatter = pSupplierImpl->GetNumberFormatter();
    sal_uInt16 nFlags;
    if(::dbaui::callColumnFormatDialog(this,pFormatter,pActFieldDescr->GetType(),nOldFormatKey,rOldJustify,nFlags,sal_True))
    {
        sal_Bool bModified = sal_False;
        if(nOldFormatKey != pActFieldDescr->GetFormatKey())
        {
            pActFieldDescr->SetFormatKey( nOldFormatKey );
            bModified = sal_True;
        }
        if(rOldJustify != pActFieldDescr->GetHorJustify())
        {
            pActFieldDescr->SetHorJustify( rOldJustify );
            bModified = sal_True;
        }

        if(bModified)
        {
            SetModified(sal_True);
            UpdateFormatSample(pActFieldDescr);
        }
    }
    return 0;
}

void OFieldDescControl::SetModified(sal_Bool /*bModified*/)
{
}

IMPL_LINK( OFieldDescControl, ChangeHdl, ListBox *, pListBox )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    if ( !pActFieldDescr )
        return 0;

    if ( pListBox->GetSavedValue() != pListBox->GetSelectEntryPos() )
        SetModified(sal_True);

    // Special treatment for Boold fields
    if(pListBox == pRequired && pBoolDefault )
    {
        // If pRequired = sal_True then the sal_Bool field must NOT contain <<none>>
        String sDef = BoolStringUI(::comphelper::getString(pActFieldDescr->GetControlDefault()));

        if(pRequired->GetSelectEntryPos() == 0) // Yes
        {
            pBoolDefault->RemoveEntry(String(ModuleRes(STR_VALUE_NONE)));
            if (!sDef.Equals(aYes) && !sDef.Equals(aNo))
                pBoolDefault->SelectEntryPos(1);  // No as a default
            else
                pBoolDefault->SelectEntry(sDef);
        }
        else if(pBoolDefault->GetEntryCount() < 3)
        {
            pBoolDefault->InsertEntry(String(ModuleRes(STR_VALUE_NONE)));
            pBoolDefault->SelectEntry(sDef);
        }
    }

    // A special treatment only for AutoIncrement
    if (pListBox == pAutoIncrement)
    {
        if(pListBox->GetSelectEntryPos() == 1)
        { // no
            DeactivateAggregate( tpAutoIncrementValue );
            if(pActFieldDescr->IsPrimaryKey())
                DeactivateAggregate( tpRequired );
            else if( pActFieldDescr->getTypeInfo()->bNullable )
            {
                ActivateAggregate( tpRequired );
                if(pRequired)
                {
                    if( pActFieldDescr->IsNullable() )
                        pRequired->SelectEntryPos( 1 ); // no
                    else
                        pRequired->SelectEntryPos( 0 ); // yes
                }
            }
            ActivateAggregate( tpDefault );
        }
        else
        {
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
            ActivateAggregate( tpAutoIncrementValue );
        }
        // Move all up
        ArrangeAggregates();
    }

    if(pListBox == m_pType)
    {
        TOTypeInfoSP pTypeInfo = getTypeInfo(m_pType->GetSelectEntryPos());
        pActFieldDescr->FillFromTypeInfo(pTypeInfo,sal_True,sal_False); // SetType(pTypeInfo);

        DisplayData(pActFieldDescr);
        CellModified(-1, m_pType->GetPos());
    }

    return 0;
}

// Rearrange all Controls, such that they are in fixed order and really on top
// of the DescriptionPage
void OFieldDescControl::ArrangeAggregates()
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // A Control's description
    struct AGGREGATE_DESCRIPTION
    {
        Control*    pctrlInputControl;  // The actual Control for input
        Control*    pctrlTextControl;   // The corresponding Label
        sal_uInt16      nPosSizeArgument;   // The second argument for SetPosSize
    };
    AGGREGATE_DESCRIPTION adAggregates[] = {
        { m_pColumnName, m_pColumnNameText, 1},
        { m_pType, m_pTypeText, 1},
        { pAutoIncrement, pAutoIncrementText, 1 },
        { m_pAutoIncrementValue, m_pAutoIncrementValueText, 3 },
        { pNumType, pNumTypeText, 1 },
        { pRequired, pRequiredText, 1 },
        { pTextLen, pTextLenText, 1 },
        { pLength, pLengthText, 1 },
        { pScale, pScaleText, 1 },
        { pDefault, pDefaultText, 3 },
        { pFormatSample, pFormatText, 4 },
        { pBoolDefault, pBoolDefaultText, 1 },
    };

    long nMaxWidth = 0;
    for (size_t i=0; i<sizeof(adAggregates)/sizeof(adAggregates[0]); i++)
    {
        if (adAggregates[i].pctrlTextControl)
        {
            nMaxWidth = ::std::max<long>(OutputDevice::GetTextWidth(adAggregates[i].pctrlTextControl->GetText()),nMaxWidth);
        }
    }

    OSL_ENSURE(nMaxWidth != 0,"Invalid width!");

    // And go ...
    int nCurrentControlPos = 0;
    Control* pZOrderPredecessor = NULL;
    for (size_t i=0; i<sizeof(adAggregates)/sizeof(adAggregates[0]); i++)
    {
        if (adAggregates[i].pctrlInputControl)
        {
            SetPosSize(&adAggregates[i].pctrlTextControl, nCurrentControlPos, 0);
            SetPosSize(&adAggregates[i].pctrlInputControl, nCurrentControlPos, adAggregates[i].nPosSizeArgument);

            // Set the z-order in a way such that the Controls can be traversed in the same sequence in which they have been arranged here
            adAggregates[i].pctrlTextControl->SetZOrder(pZOrderPredecessor, pZOrderPredecessor ? WINDOW_ZORDER_BEHIND : WINDOW_ZORDER_FIRST);
            adAggregates[i].pctrlInputControl->SetZOrder(adAggregates[i].pctrlTextControl, WINDOW_ZORDER_BEHIND );
            pZOrderPredecessor = adAggregates[i].pctrlInputControl;

            if (adAggregates[i].pctrlInputControl == pFormatSample)
            {
                pFormat->SetZOrder(pZOrderPredecessor, WINDOW_ZORDER_BEHIND);
                pZOrderPredecessor = pFormat;
            }

            ++nCurrentControlPos;
        }
    }

    // Special treatment for the Format Controls
    if (pFormat)
    {
        Point ptSamplePos(pFormatSample->GetPosPixel());
        Size szSampleSize(pFormatSample->GetSizePixel());
        pFormat->SetPosPixel(Point(ptSamplePos.X() + szSampleSize.Width() + 5, ptSamplePos.Y()));
    }

    // Finally, put the ScrollBars at the top of the z-order
    m_pVertScroll->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
    m_pHorzScroll->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
}

void OFieldDescControl::ActivateAggregate( EControlType eType )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // Create Controls
    switch( eType )
    {
    case tpDefault:
        if( pDefault )
            return;
        m_nPos++;
        pDefaultText = CreateText(STR_DEFAULT_VALUE);
        pDefault = new OPropEditCtrl( this, STR_HELP_DEFAULT_VALUE, FIELD_PROPERTY_DEFAULT, WB_BORDER );
        InitializeControl(pDefault,HID_TAB_ENT_DEFAULT,false);
        break;
    case tpAutoIncrementValue:
        if( m_pAutoIncrementValue || !isAutoIncrementValueEnabled() )
            return;
        m_nPos++;
        m_pAutoIncrementValueText = CreateText(STR_AUTOINCREMENT_VALUE);
        m_pAutoIncrementValue = new OPropEditCtrl( this, STR_HELP_AUTOINCREMENT_VALUE, FIELD_PRPOERTY_AUTOINCREMENT, WB_BORDER );
        m_pAutoIncrementValue->SetText( getAutoIncrementValue() );
        InitializeControl(m_pAutoIncrementValue,HID_TAB_AUTOINCREMENTVALUE,false);
        break;

    case tpRequired:
    {
        if( pRequired )
            return;
        Reference< XDatabaseMetaData> xMetaData = getMetaData();

        if(xMetaData.is() && xMetaData->supportsNonNullableColumns())
        {
            m_nPos++;
            pRequiredText = CreateText(STR_FIELD_REQUIRED);
            pRequired = new OPropListBoxCtrl( this, STR_HELP_FIELD_REQUIRED, FIELD_PROPERTY_REQUIRED, WB_DROPDOWN);

            pRequired->InsertEntry( aYes );
            pRequired->InsertEntry( aNo );
            pRequired->SelectEntryPos(1);

            InitializeControl(pRequired,HID_TAB_ENT_REQUIRED,true);
        }
    }
    break;
    case tpAutoIncrement:
    {
        if( pAutoIncrement )
            return;
        m_nPos++;
        pAutoIncrementText = CreateText(STR_FIELD_AUTOINCREMENT);
        pAutoIncrement = new OPropListBoxCtrl( this, STR_HELP_AUTOINCREMENT, FIELD_PROPERTY_AUTOINC, WB_DROPDOWN );
        pAutoIncrement->InsertEntry( aYes );
        pAutoIncrement->InsertEntry( aNo );
        pAutoIncrement->SelectEntryPos(0);
        InitializeControl(pAutoIncrement,HID_TAB_ENT_AUTOINCREMENT,true);
    }
    break;
    case tpTextLen:
        if( pTextLen )
            return;
        m_nPos++;
        pTextLenText = CreateText(STR_TEXT_LENGTH);
        pTextLen = CreateNumericControl(STR_HELP_TEXT_LENGTH, FIELD_PROPERTY_TEXTLEN,HID_TAB_ENT_TEXT_LEN);
        break;

    case tpType:
        if( m_pType)
            return;
        m_nPos++;
        m_pTypeText = CreateText(STR_TAB_FIELD_DATATYPE);
        m_pType = new OPropListBoxCtrl( this, STR_HELP_AUTOINCREMENT, FIELD_PRPOERTY_TYPE, WB_DROPDOWN );
        m_pType->SetDropDownLineCount(20);
        {
            const OTypeInfoMap* pTypeInfo = getTypeInfo();
            OTypeInfoMap::const_iterator aIter = pTypeInfo->begin();
            OTypeInfoMap::const_iterator aEnd = pTypeInfo->end();
            for(;aIter != aEnd;++aIter)
                m_pType->InsertEntry( aIter->second->aUIName );
        }
        m_pType->SelectEntryPos(0);
        InitializeControl(m_pType,HID_TAB_ENT_TYPE,true);
        break;
    case tpColumnName:
        if( m_pColumnName )
            return;
        m_nPos++;
        {
            sal_uInt32 nMax = EDIT_NOLIMIT;
            OUString aTmpString;
            try
            {
                Reference< XDatabaseMetaData> xMetaData = getMetaData();
                if ( xMetaData.is() )
                {
                    nMax =  xMetaData->getMaxColumnNameLength();
                    aTmpString = xMetaData->getExtraNameCharacters();
                }
            }
            catch(Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            m_pColumnNameText = CreateText(STR_TAB_FIELD_NAME);
            m_pColumnName = new OPropColumnEditCtrl( this,
                                                    aTmpString,
                                                    STR_HELP_DEFAULT_VALUE,
                                                    FIELD_PRPOERTY_COLUMNNAME,
                                                    WB_BORDER );
            m_pColumnName->SetMaxTextLen(xub_StrLen( nMax ? nMax : EDIT_NOLIMIT));
            m_pColumnName->setCheck( isSQL92CheckEnabled(getConnection()) );
        }

        InitializeControl(m_pColumnName,HID_TAB_ENT_COLUMNNAME,false);
        break;
    case tpNumType:
        if( pNumType )
            return;
        m_nPos++;
        pNumTypeText = CreateText(STR_NUMERIC_TYPE);

        pNumType = new OPropListBoxCtrl( this, STR_HELP_NUMERIC_TYPE, FIELD_PROPERTY_NUMTYPE, WB_DROPDOWN );
        pNumType->SetDropDownLineCount(5);

        pNumType->InsertEntry( OUString("Byte") );
        pNumType->InsertEntry( OUString("SmallInt") );
        pNumType->InsertEntry( OUString("Integer") );
        pNumType->InsertEntry( OUString("Single") );
        pNumType->InsertEntry( OUString("Double") );
        pNumType->SelectEntryPos(2);
        InitializeControl(pNumType,HID_TAB_ENT_NUMTYP,true);
        break;

    case tpLength:
        if( pLength )
            return;
        m_nPos++;
        pLengthText = CreateText(STR_LENGTH);
        pLength = CreateNumericControl(STR_HELP_LENGTH, FIELD_PROPERTY_LENGTH,HID_TAB_ENT_LEN);
        break;

    case tpScale:
        if( pScale )
            return;
        m_nPos++;
        pScaleText = CreateText(STR_SCALE);
        pScale = CreateNumericControl(STR_HELP_SCALE, FIELD_PROPERTY_SCALE,HID_TAB_ENT_SCALE);
        break;

    case tpFormat:
        if (!pFormat)
        {
            m_nPos++;
            pFormatText = CreateText(STR_FORMAT);

            pFormatSample = new OPropEditCtrl( this, STR_HELP_FORMAT_CODE, -1, WB_BORDER );
            pFormatSample->SetReadOnly(sal_True);
            pFormatSample->Enable(sal_False);
            InitializeControl(pFormatSample,HID_TAB_ENT_FORMAT_SAMPLE,false);

            pFormat = new PushButton( this, ModuleRes(PB_FORMAT) );
            const sal_Int32 nControlHeight = GetMaxControlHeight();
            pFormat->SetSizePixel(Size(nControlHeight, nControlHeight));
            pFormat->SetClickHdl( LINK( this, OFieldDescControl, FormatClickHdl ) );
            InitializeControl(pFormat,HID_TAB_ENT_FORMAT,false);
        }

        UpdateFormatSample(pActFieldDescr);
        break;
    case tpBoolDefault:
        if (pBoolDefault)
            return;

        m_nPos++;
        pBoolDefaultText = CreateText(STR_DEFAULT_VALUE);
        pBoolDefault = new OPropListBoxCtrl( this, STR_HELP_BOOL_DEFAULT, FIELD_PROPERTY_BOOL_DEFAULT, WB_DROPDOWN );
        pBoolDefault->SetDropDownLineCount(3);
        pBoolDefault->InsertEntry(String(ModuleRes(STR_VALUE_NONE)));
        pBoolDefault->InsertEntry(aYes);
        pBoolDefault->InsertEntry(aNo);

        InitializeControl(pBoolDefault,HID_TAB_ENT_BOOL_DEFAULT,false);
        break;
    }
}

void OFieldDescControl::InitializeControl(Control* _pControl,const OString& _sHelpId,bool _bAddChangeHandler)
{
    _pControl->SetHelpId(_sHelpId);
    if ( _bAddChangeHandler )
        ((OPropListBoxCtrl*)_pControl)->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

    _pControl->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
    _pControl->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));
    _pControl->EnableClipSiblings();
}

FixedText* OFieldDescControl::CreateText(sal_uInt16 _nTextRes)
{
    FixedText* pFixedText = new FixedText( this );
    pFixedText->SetText( ModuleRes(_nTextRes) );
    pFixedText->EnableClipSiblings();
    return pFixedText;
}

OPropNumericEditCtrl* OFieldDescControl::CreateNumericControl(sal_uInt16 _nHelpStr,short _nProperty,const OString& _sHelpId)
{
    OPropNumericEditCtrl* pControl = new OPropNumericEditCtrl( this, _nHelpStr, _nProperty, WB_BORDER );
    pControl->SetDecimalDigits(0);
    pControl->SetMin(0);
    pControl->SetMax(0x7FFFFFFF);   // Should be changed outside, if needed
    pControl->SetStrictFormat(sal_True);

    InitializeControl(pControl,_sHelpId,false);

    return pControl;
}

void OFieldDescControl::DeactivateAggregate( EControlType eType )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    pLastFocusWindow = NULL;
    // Destroy Controls
    switch( eType )
    {
    case tpDefault:
        lcl_HideAndDeleteControl(m_nPos,&pDefault,&pDefaultText);
        break;

    case tpAutoIncrementValue:
        lcl_HideAndDeleteControl(m_nPos,&m_pAutoIncrementValue,&m_pAutoIncrementValueText);
        break;

    case tpColumnName:
        lcl_HideAndDeleteControl(m_nPos,&m_pColumnName,&m_pColumnNameText);
        break;

    case tpType:
        lcl_HideAndDeleteControl(m_nPos,&m_pType,&m_pTypeText);
        break;

    case tpAutoIncrement:
        lcl_HideAndDeleteControl(m_nPos,&pAutoIncrement,&pAutoIncrementText);
        break;

    case tpRequired:
        lcl_HideAndDeleteControl(m_nPos,&pRequired,&pRequiredText);
        break;

    case tpTextLen:
        lcl_HideAndDeleteControl(m_nPos,&pTextLen,&pTextLenText);
        break;

    case tpNumType:
        lcl_HideAndDeleteControl(m_nPos,&pNumType,&pNumTypeText);
        break;

    case tpLength:
        lcl_HideAndDeleteControl(m_nPos,&pLength,&pLengthText);
        break;

    case tpScale:
        lcl_HideAndDeleteControl(m_nPos,&pScale,&pScaleText);
        break;

    case tpFormat:
        // TODO: we have to check if we have to increment m_nPos again
        lcl_HideAndDeleteControl(m_nPos,&pFormat,&pFormatText);
        if ( pFormatSample )
        {
            pFormatSample->Hide();
            delete pFormatSample;
            pFormatSample = NULL;
        }
        break;
    case tpBoolDefault:
        lcl_HideAndDeleteControl(m_nPos,&pBoolDefault,&pBoolDefaultText);
        break;
    }
}

void OFieldDescControl::SetPosSize( Control** ppControl, long nRow, sal_uInt16 nCol )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);

    // Calculate size
    const sal_Int32 nControlHeight = GetMaxControlHeight();
    Size aSize(0,nControlHeight);
    if ( isRightAligned() && nCol )
        aSize.Width() = LogicToPixel(Size(m_nWidth, 0),MAP_APPFONT).Width();
    else
    {
        switch( nCol )
        {
        case 0:
        default:
            aSize.Width()  = CONTROL_WIDTH_1;
            break;
        case 1:
            aSize.Width()  = CONTROL_WIDTH_2;
            break;
        case 3:
            aSize.Width()  = CONTROL_WIDTH_3;
            break;
        case 4:
            aSize.Width()  = CONTROL_WIDTH_4;
            break;
        }
    }

    // Calculate Position
    Point aPosition;
    switch( nCol )
    {
    case 0:
        aPosition.X() = 0;
        aPosition.Y() = 1;
        break;
    case 1:
    case 3:
    case 4:
        if ( isRightAligned() )
        {
            Size aOwnSize = GetSizePixel();
            aPosition.X() = aOwnSize.Width() - aSize.Width();
        }
        else
            aPosition.X() = CONTROL_WIDTH_1 + CONTROL_SPACING_X;
        break;
    default:
        aPosition.X() = 0;
    }

    (*ppControl)->SetSizePixel( aSize );
    aSize = (*ppControl)->GetSizePixel( );

    const sal_Int32 nControl_Spacing_y = LogicToPixel(Size(0, CONTROL_SPACING_Y),MAP_APPFONT).Height();
    aPosition.Y() += ((nRow+1)*nControl_Spacing_y) +
                    (nRow*nControlHeight);

    // Display Control
    (*ppControl)->SetPosSizePixel( aPosition, aSize );
    aSize = (*ppControl)->GetSizePixel();

    (*ppControl)->Show();
}

void OFieldDescControl::DisplayData(OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    pActFieldDescr = pFieldDescr;
    if(!pFieldDescr)
    {
        DeactivateAggregate( tpDefault );
        DeactivateAggregate( tpRequired );
        DeactivateAggregate( tpTextLen );
        DeactivateAggregate( tpNumType );
        DeactivateAggregate( tpScale );
        DeactivateAggregate( tpLength );
        DeactivateAggregate( tpFormat );
        DeactivateAggregate( tpAutoIncrement );
        DeactivateAggregate( tpBoolDefault );
        DeactivateAggregate( tpColumnName );
        DeactivateAggregate( tpType );
        DeactivateAggregate( tpAutoIncrementValue );
        m_pPreviousType = TOTypeInfoSP();
        // Reset the saved focus' pointer
        pLastFocusWindow = NULL;
        if ( m_bAdded )
        {
            ::dbaui::notifySystemWindow(this,this,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
            m_bAdded = sal_False;
        }
        return;
    }

    if ( !m_bAdded )
    {
        ::dbaui::notifySystemWindow(this,this,::comphelper::mem_fun(&TaskPaneList::AddWindow));
        m_bAdded = sal_True;
    }

    TOTypeInfoSP pFieldType;
    if( pFieldDescr )
        pFieldType = pFieldDescr->getTypeInfo();

    ActivateAggregate( tpColumnName );
    ActivateAggregate( tpType );

    OSL_ENSURE(pFieldType.get(),"We need a type information here!");
    // If the type has changed, subsitute Controls
    if( m_pPreviousType != pFieldType )
    {
        // Reset the saved focus' pointer
        pLastFocusWindow = NULL;

        // Controls, which must NOT be displayed again
        DeactivateAggregate( tpNumType );

        // determine which controls we should show and which not

        // 1. the required control
        if ( pFieldType->bNullable )
            ActivateAggregate( tpRequired );
        else
            DeactivateAggregate( tpRequired );

        // 2. the autoincrement
        if ( pFieldType->bAutoIncrement )
        {
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
            ActivateAggregate( tpAutoIncrement );
            ActivateAggregate( tpAutoIncrementValue );
        }
        else
        {
            DeactivateAggregate( tpAutoIncrement );
            DeactivateAggregate( tpAutoIncrementValue );
            if(pFieldType->bNullable)
                ActivateAggregate( tpRequired );
            else
                DeactivateAggregate( tpRequired );
            ActivateAggregate( tpDefault );
        }
        // 3. the scale and precision
        if (pFieldType->nPrecision)
        {
            ActivateAggregate( tpLength );
            pLength->SetMax(::std::max<sal_Int32>(pFieldType->nPrecision,pFieldDescr->GetPrecision()));
            pLength->SetSpecialReadOnly(pFieldType->aCreateParams.isEmpty());
        }
        else
            DeactivateAggregate( tpLength );

        if (pFieldType->nMaximumScale)
        {
            ActivateAggregate( tpScale );
            pScale->SetMax(::std::max<sal_Int32>(pFieldType->nMaximumScale,pFieldDescr->GetScale()));
            pScale->SetMin(pFieldType->nMinimumScale);
            static const OUString s_sPRECISION("PRECISION");
            pScale->SetSpecialReadOnly(pFieldType->aCreateParams.isEmpty() || pFieldType->aCreateParams == s_sPRECISION);
        }
        else
            DeactivateAggregate( tpScale );

        // and now look for type specific things
        switch( pFieldType->nType )
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                DeactivateAggregate( tpLength );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpDefault );
                ActivateAggregate( tpFormat );
                if (pFieldType->nPrecision)
                {
                    ActivateAggregate( tpTextLen );
                    pTextLen->SetMax(::std::max<sal_Int32>(pFieldType->nPrecision,pFieldDescr->GetPrecision()));
                    pTextLen->SetSpecialReadOnly(pFieldType->aCreateParams.isEmpty());
                }
                else
                    DeactivateAggregate( tpTextLen );
                break;
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
                DeactivateAggregate( tpLength ); // we don't need a length for date types
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpDefault );
                ActivateAggregate( tpFormat );
                break;
            case DataType::BIT:
                if ( !pFieldType->aCreateParams.isEmpty() )
                {
                    DeactivateAggregate( tpFormat );
                    DeactivateAggregate( tpTextLen );
                    DeactivateAggregate( tpBoolDefault );
                    break;
                }
                // run through
            case DataType::BOOLEAN:
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpFormat );
                DeactivateAggregate( tpDefault );

                ActivateAggregate( tpBoolDefault );
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::BIGINT:
            case DataType::FLOAT:
            case DataType::DOUBLE:
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::REAL:
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpFormat );
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
                DeactivateAggregate( tpDefault );
                DeactivateAggregate( tpRequired );
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                ActivateAggregate( tpFormat );
                break;
            case DataType::LONGVARBINARY:
            case DataType::SQLNULL:
            case DataType::OBJECT:
            case DataType::DISTINCT:
            case DataType::STRUCT:
            case DataType::ARRAY:
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::REF:
            case DataType::OTHER:
                DeactivateAggregate( tpFormat );
                DeactivateAggregate( tpTextLen );
                DeactivateAggregate( tpBoolDefault );

                break;
            default:
                OSL_FAIL("Unknown type");
        }
        m_pPreviousType = pFieldType;
    }
    if(pFieldDescr)
    {
        if(pFieldDescr->IsPrimaryKey())
        {
            DeactivateAggregate( tpRequired );
        }
        else if ( !pAutoIncrement && pFieldType.get() )
        {
            if ( pFieldType->bNullable )
                ActivateAggregate( tpRequired );
            else
                DeactivateAggregate( tpRequired );
        }
    }
    // Initialize Controls
    if( pAutoIncrement )
    {
        if ( pFieldDescr->IsAutoIncrement() )
        {
            pAutoIncrement->SelectEntryPos( 0 ); // yes
            ActivateAggregate( tpAutoIncrementValue );
            if ( m_pAutoIncrementValue )
                m_pAutoIncrementValue->SetText(pFieldDescr->GetAutoIncrementValue());
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
        }
        else
        {
            // disable autoincrement value because it should only be visible when autoincrement is to true
            DeactivateAggregate( tpAutoIncrementValue );
            pAutoIncrement->SelectEntryPos( 1 );        // no
            ActivateAggregate( tpDefault );
            // Affects pRequired
            if(!pFieldDescr->IsPrimaryKey())
                ActivateAggregate( tpRequired );
        }
    }

    if( pDefault )
    {
        pDefault->SetText( getControlDefault(pFieldDescr) );
        pDefault->ClearModifyFlag();
    }

    if( pBoolDefault )
    {
        // If pRequired = sal_True then the sal_Bool field must NOT contain <<none>>
        OUString sValue;
        pFieldDescr->GetControlDefault() >>= sValue;
        String sDef = BoolStringUI(sValue);

        // Make sure that <<none>> is only present if the field can be NULL
        if ( ( pFieldType.get() && !pFieldType->bNullable ) || !pFieldDescr->IsNullable() )
        {
            pFieldDescr->SetIsNullable(ColumnValue::NO_NULLS); // The type says so

            pBoolDefault->RemoveEntry(String(ModuleRes(STR_VALUE_NONE)));
            if ( !sDef.Equals(aYes) && !sDef.Equals(aNo) )
                pBoolDefault->SelectEntryPos(1);  // No as a default
            else
                pBoolDefault->SelectEntry(sDef);

            pFieldDescr->SetControlDefault(makeAny(OUString(BoolStringPersistent(pBoolDefault->GetSelectEntry()))));
        }
        else if(pBoolDefault->GetEntryCount() < 3)
        {
            pBoolDefault->InsertEntry(String(ModuleRes(STR_VALUE_NONE)));
            pBoolDefault->SelectEntry(sDef);
        }
        else
            pBoolDefault->SelectEntry(sDef);
    }

    if( pRequired )
    {
        if( pFieldDescr->IsNullable() )
            pRequired->SelectEntryPos( 1 ); // no
        else
            pRequired->SelectEntryPos( 0 ); // yes
    }

    if( pTextLen )
    {
        pTextLen->SetText( OUString::number(pFieldDescr->GetPrecision()) );
        pTextLen->ClearModifyFlag();
    }

    if( pNumType )
    {
        OSL_FAIL("OFieldDescControl::DisplayData: invalid num type!");
    }

    if( pLength )
        pLength->SetText( OUString::number(pFieldDescr->GetPrecision()) );

    if( pScale )
        pScale->SetText( OUString::number(pFieldDescr->GetScale()) );

    if( pFormat )
        UpdateFormatSample(pFieldDescr);

    if(m_pColumnName)
        m_pColumnName->SetText(pFieldDescr->GetName());

    if(m_pType)
    {
        sal_uInt16 nPos = pFieldType.get() ? m_pType->GetEntryPos(String(pFieldDescr->getTypeInfo()->aUIName)) : LISTBOX_ENTRY_NOTFOUND;
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
        {
            const OTypeInfoMap* pMap = getTypeInfo();
            OTypeInfoMap::const_iterator aIter = pMap->find(pFieldType.get() ? pFieldDescr->getTypeInfo()->nType : pFieldDescr->GetType());
            if(aIter == pMap->end() && !pMap->empty())
            {
                aIter = pMap->begin();
                if(pFieldDescr->GetPrecision() > aIter->second->nPrecision)
                    pFieldDescr->SetPrecision(aIter->second->nPrecision);
                if(pFieldDescr->GetScale() > aIter->second->nMaximumScale)
                    pFieldDescr->SetScale(0);
                if(!aIter->second->bNullable && pFieldDescr->IsNullable())
                    pFieldDescr->SetIsNullable(ColumnValue::NO_NULLS);
                if(!aIter->second->bAutoIncrement && pFieldDescr->IsAutoIncrement())
                    pFieldDescr->SetAutoIncrement(sal_False);
            }
            if ( aIter != pMap->end() )
            {
                pFieldDescr->SetType(aIter->second);
            }
        }
        m_pType->SelectEntry(pFieldDescr->getTypeInfo()->aUIName);
    }

    // Enable/disable Controls
    sal_Bool bRead(IsReadOnly());

    ArrangeAggregates();
    CheckScrollBars();
    ScrollAllAggregates();

    SetReadOnly( bRead );
}

IMPL_LINK(OFieldDescControl, OnControlFocusGot, Control*, pControl )
{
    String strHelpText;
    OPropNumericEditCtrl* pNumeric = dynamic_cast< OPropNumericEditCtrl* >( pControl );
    if ( pNumeric )
    {
        pNumeric->SaveValue();
        strHelpText = pNumeric->GetHelp();
    }

    OPropColumnEditCtrl* pColumn = dynamic_cast< OPropColumnEditCtrl* >( pControl );
    if ( pColumn )
    {
        pColumn->SaveValue();
        strHelpText = pColumn->GetHelp();
    }

    OPropEditCtrl* pEdit = dynamic_cast< OPropEditCtrl* >( pControl );
    if ( pEdit )
    {
        pEdit->SaveValue();
        strHelpText = pEdit->GetHelp();
    }

    OPropListBoxCtrl* pListBox = dynamic_cast< OPropListBoxCtrl* >( pControl );
    if ( pListBox )
    {
        pListBox->SaveValue();
        strHelpText = pListBox->GetHelp();
    }

    if (pControl == pFormat)
        strHelpText  =String(ModuleRes(STR_HELP_FORMAT_BUTTON));

    if (strHelpText.Len() && (pHelp != NULL))
        pHelp->SetHelpText(strHelpText);

    m_pActFocusWindow = pControl;

    return 0L;
}

IMPL_LINK(OFieldDescControl, OnControlFocusLost, Control*, pControl )
{
    if ((pControl == pLength) || (pControl == pTextLen) || (pControl == pScale))
    {
        OPropNumericEditCtrl* pConverted = (OPropNumericEditCtrl*)pControl;
        if (pConverted->IsModified())
            CellModified(-1, pConverted->GetPos());
    }
    if(pControl == m_pColumnName)
    {
        OPropColumnEditCtrl* pConverted = (OPropColumnEditCtrl*)pControl;
        if (pConverted->IsModified())
            CellModified(-1, pConverted->GetPos());
    }
    else if ((pControl == pDefault) || (pControl == pFormatSample) || (pControl == m_pAutoIncrementValue) )
    {
        OPropEditCtrl* pConverted = (OPropEditCtrl*)pControl;
        if (pConverted->IsModified())
            CellModified(-1, pConverted->GetPos());
    }
    else if ((pControl == pRequired) || (pControl == pNumType) || (pControl == pAutoIncrement) || (pControl == pBoolDefault) || (pControl == m_pType))
    {
        OPropListBoxCtrl* pConverted = (OPropListBoxCtrl*)pControl;
        if (pConverted->IsModified())
            CellModified(-1, pConverted->GetPos());
    }

    if (pControl == pDefault)
        UpdateFormatSample(pActFieldDescr);

    implFocusLost(pControl);

    return 0L;
}

void OFieldDescControl::SaveData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    if( !pFieldDescr )
        return;

    // Read out Controls
    OUString sDefault;
    if (pDefault)
    {
        sDefault = pDefault->GetText();
    }
    else if (pBoolDefault)
    {
        sDefault = BoolStringPersistent(pBoolDefault->GetSelectEntry());
    }

    if ( !sDefault.isEmpty() )
        pFieldDescr->SetControlDefault(makeAny(sDefault));
    else
        pFieldDescr->SetControlDefault(Any());

    if((pRequired && pRequired->GetSelectEntryPos() == 0) || pFieldDescr->IsPrimaryKey() || (pBoolDefault && pBoolDefault->GetEntryCount() == 2))  // yes
        pFieldDescr->SetIsNullable( ColumnValue::NO_NULLS );
    else
        pFieldDescr->SetIsNullable( ColumnValue::NULLABLE );

    if ( pAutoIncrement )
        pFieldDescr->SetAutoIncrement( pAutoIncrement->GetSelectEntryPos() == 0 );

    if( pTextLen )
        pFieldDescr->SetPrecision( static_cast<sal_Int32>(pTextLen->GetValue()) );
    else if( pLength )
        pFieldDescr->SetPrecision( static_cast<sal_Int32>(pLength->GetValue()) );
    if( pScale )
        pFieldDescr->SetScale( static_cast<sal_Int32>(pScale->GetValue()) );

    if(m_pColumnName)
        pFieldDescr->SetName(m_pColumnName->GetText());

    if ( m_pAutoIncrementValue && isAutoIncrementValueEnabled() )
        pFieldDescr->SetAutoIncrementValue(m_pAutoIncrementValue->GetText());
}

void OFieldDescControl::UpdateFormatSample(OFieldDescription* pFieldDescr)
{
    if ( pFieldDescr && pFormatSample )
        pFormatSample->SetText(getControlDefault(pFieldDescr,sal_False));
}

void OFieldDescControl::GetFocus()
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // Set the Focus to the Control that has been active last
    TabPage::GetFocus();
    if( pLastFocusWindow )
    {
        pLastFocusWindow->GrabFocus();
        pLastFocusWindow = NULL;
    }
}

void OFieldDescControl::implFocusLost(Window* _pWhich)
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    OSL_ENSURE(!_pWhich || IsChild(_pWhich), "OFieldDescControl::implFocusLost : invalid window !");

    // Remember the active Control
    if (!pLastFocusWindow)
        pLastFocusWindow = _pWhich;

    // Reset HelpText
    if (pHelp && !pHelp->HasChildPathFocus())
        pHelp->SetHelpText( String() );
}

void OFieldDescControl::LoseFocus()
{
    DBG_CHKTHIS(OFieldDescControl,NULL);

    implFocusLost(NULL);

    TabPage::LoseFocus();
}

sal_Bool OFieldDescControl::isCopyAllowed()
{
    sal_Bool bAllowed = (m_pActFocusWindow != NULL) &&
                        (m_pActFocusWindow == pDefault || m_pActFocusWindow == pFormatSample    ||
                        m_pActFocusWindow == pTextLen || m_pActFocusWindow == pLength           ||
                        m_pActFocusWindow == pScale  || m_pActFocusWindow == m_pColumnName      ||
                        m_pActFocusWindow == m_pAutoIncrementValue) &&
                        !static_cast<Edit*>(m_pActFocusWindow)->GetSelected().isEmpty();

    return bAllowed;
}

sal_Bool OFieldDescControl::isCutAllowed()
{
    sal_Bool bAllowed = (m_pActFocusWindow != NULL) &&
                        (m_pActFocusWindow == pDefault || m_pActFocusWindow == pFormatSample    ||
                        m_pActFocusWindow == pTextLen || m_pActFocusWindow == pLength           ||
                        m_pActFocusWindow == pScale  || m_pActFocusWindow == m_pColumnName      ||
                        m_pActFocusWindow == m_pAutoIncrementValue) &&
                        !static_cast<Edit*>(m_pActFocusWindow)->GetSelected().isEmpty();
    return bAllowed;
}

sal_Bool OFieldDescControl::isPasteAllowed()
{
    sal_Bool bAllowed = (m_pActFocusWindow != NULL) &&
                        (m_pActFocusWindow == pDefault || m_pActFocusWindow == pFormatSample    ||
                        m_pActFocusWindow == pTextLen || m_pActFocusWindow == pLength           ||
                        m_pActFocusWindow == pScale  || m_pActFocusWindow == m_pColumnName      ||
                        m_pActFocusWindow == m_pAutoIncrementValue);
    if ( bAllowed )
    {
        TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(GetParent()));
        bAllowed = aTransferData.HasFormat(SOT_FORMAT_STRING);
    }
    return bAllowed;
}

void OFieldDescControl::cut()
{
    if(isCutAllowed())
        static_cast<Edit*>(m_pActFocusWindow)->Cut();
}

void OFieldDescControl::copy()
{
    if(isCopyAllowed()) // this only checks if the focus window is valid
        static_cast<Edit*>(m_pActFocusWindow)->Copy();
}

void OFieldDescControl::paste()
{
    if(m_pActFocusWindow) // this only checks if the focus window is valid
        static_cast<Edit*>(m_pActFocusWindow)->Paste();
}

sal_Bool OFieldDescControl::isTextFormat(const OFieldDescription* _pFieldDescr,sal_uInt32& _nFormatKey) const
{
    _nFormatKey = _pFieldDescr->GetFormatKey();
    sal_Bool bTextFormat = sal_True;

    try
    {
        if (!_nFormatKey)
        {
            Reference< ::com::sun::star::util::XNumberFormatTypes> xNumberTypes(GetFormatter()->getNumberFormatsSupplier()->getNumberFormats(),UNO_QUERY);
            OSL_ENSURE(xNumberTypes.is(),"XNumberFormatTypes is null!");

            _nFormatKey = ::dbtools::getDefaultNumberFormat( _pFieldDescr->GetType(),
                _pFieldDescr->GetScale(),
                _pFieldDescr->IsCurrency(),
                xNumberTypes,
                GetLocale());
        }
        sal_Int32 nNumberFormat = ::comphelper::getNumberFormatType(GetFormatter(),_nFormatKey);
        bTextFormat = (nNumberFormat == ::com::sun::star::util::NumberFormat::TEXT);
    }
    catch(const Exception&)
    {

    }

    return bTextFormat;
}

String OFieldDescControl::getControlDefault( const OFieldDescription* _pFieldDescr ,sal_Bool _bCheck) const
{
    OUString sDefault;
    sal_Bool bCheck = !_bCheck || _pFieldDescr->GetControlDefault().hasValue();
    if ( bCheck )
    {
        sal_uInt32 nFormatKey;
        sal_Bool bTextFormat = sal_False;

        try
        {
            double nValue = 0.0;
            bTextFormat = isTextFormat(_pFieldDescr,nFormatKey);
            if ( _pFieldDescr->GetControlDefault() >>= sDefault )
            {
                if ( !bTextFormat )
                {
                    if ( !sDefault.isEmpty() )
                    {
                        try
                        {
                            nValue = GetFormatter()->convertStringToNumber(nFormatKey,sDefault);
                        }
                        catch(const Exception&)
                        {
                            return OUString(); // return empty string for format example
                        }
                    }
                }
            }
            else
                _pFieldDescr->GetControlDefault() >>= nValue;

            Reference< ::com::sun::star::util::XNumberFormatter> xNumberFormatter = GetFormatter();
            Reference<XPropertySet> xFormSet = xNumberFormatter->getNumberFormatsSupplier()->getNumberFormats()->getByKey(nFormatKey);
            OSL_ENSURE(xFormSet.is(),"XPropertySet is null!");
            OUString sFormat;
            xFormSet->getPropertyValue("FormatString") >>= sFormat;

            if ( !bTextFormat )
            {
                Locale aLocale;
                ::comphelper::getNumberFormatProperty(xNumberFormatter,nFormatKey,OUString("Locale")) >>= aLocale;

                sal_Int32 nNumberFormat = ::comphelper::getNumberFormatType(xNumberFormatter,nFormatKey);
                if(     (nNumberFormat & ::com::sun::star::util::NumberFormat::DATE)    == ::com::sun::star::util::NumberFormat::DATE
                    || (nNumberFormat & ::com::sun::star::util::NumberFormat::DATETIME) == ::com::sun::star::util::NumberFormat::DATETIME )
                {
                    nValue = DBTypeConversion::toNullDate(DBTypeConversion::getNULLDate(xNumberFormatter->getNumberFormatsSupplier()),nValue);
                }

                Reference< ::com::sun::star::util::XNumberFormatPreviewer> xPreViewer(xNumberFormatter,UNO_QUERY);
                OSL_ENSURE(xPreViewer.is(),"XNumberFormatPreviewer is null!");
                sDefault = xPreViewer->convertNumberToPreviewString(sFormat,nValue,aLocale,sal_True);
            }
            else if ( !(_bCheck && sDefault.isEmpty()) )
                sDefault = xNumberFormatter->formatString(nFormatKey, sDefault.isEmpty() ? sFormat : sDefault);
        }
        catch(const Exception&)
        {

        }
    }

    return sDefault;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
