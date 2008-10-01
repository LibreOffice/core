/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FieldDescControl.cxx,v $
 * $Revision: 1.49.68.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#define ITEMID_HORJUSTIFY       SID_ATTR_ALIGN_HOR_JUSTIFY
#define ITEMID_VERJUSTIFY       SID_ATTR_ALIGN_VER_JUSTIFY
//#define ITEMID_ORIENTATION     SID_ATTR_ALIGN_ORIENTATION
#define ITEMID_LINEBREAK        SID_ATTR_ALIGN_LINEBREAK
#define ITEMID_MARGIN           SID_ATTR_ALIGN_MARGIN

#ifndef DBAUI_FIELDDESCRIPTIONCONTROL_HXX
#include "FieldDescControl.hxx"
#endif
#ifndef DBAUI_FIELDCONTROLS_HXX
#include "FieldControls.hxx"
#endif
#ifndef _ZFORLIST_HXX //autogen
#define _ZFORLIST_DECLARE_TABLE     // ohne das bekomme ich einen Compiler-Fehler in <svtools/zforlist.hxx>
#include <svtools/zforlist.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#include "TableDesignHelpBar.hxx"
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SVX_NUMFMTSH_HXX
#include <svx/numfmtsh.hxx>
#endif
#ifndef _DBU_CONTROL_HRC_
#include "dbu_control.hrc"
#endif
#ifndef _DBU_TBL_HRC_
#include "dbu_tbl.hrc"
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef DBAUI_SBATTRDLG_HXX
#include "dlgattr.hxx"
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_ALGITEM_HXX
#include <svx/algitem.hxx>
#endif
#ifndef _SFXRNGITEM_HXX
#include <svtools/rngitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATPREVIEWER_HPP_
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _NUMUNO_HXX
#include <svtools/numuno.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#include <memory>
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

using namespace dbaui;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;

//==================================================================

// fuer die Controls auf der OFieldDescGenPage
#define CONTROL_SPACING_X   18  // 6
#define CONTROL_SPACING_Y   4
#define CONTROL_HEIGHT      10
#define CONTROL_WIDTH_1     160 // 100
#define CONTROL_WIDTH_2     100 // 60
#define CONTROL_WIDTH_3     250
#define CONTROL_WIDTH_4     (CONTROL_WIDTH_3 - 20 - 5)

#define SBA_DEF_RANGEFORMAT         (100 + 143) // RangeItem
#define SBA_DEF_FMTVALUE            (100 + 144) // SfxULONG, Format
#define SBA_ATTR_ALIGN_HOR_JUSTIFY  (100 + 145) //  SvxHorJustifyItem

#define HSCROLL_STEP        20


namespace
{
    // -----------------------------------------------------------------------------
    double checkDoubleForDateFormat(double _nValue,sal_Int32 _nFormatKey,const Reference< ::com::sun::star::util::XNumberFormatter>& _xNumberFormatter)
    {
        double nValue = _nValue;
        sal_Int32 nNumberFormat = ::comphelper::getNumberFormatType(_xNumberFormatter,_nFormatKey);
        if(     (nNumberFormat & ::com::sun::star::util::NumberFormat::DATE)    == ::com::sun::star::util::NumberFormat::DATE
            || (nNumberFormat & ::com::sun::star::util::NumberFormat::DATETIME) == ::com::sun::star::util::NumberFormat::DATETIME )
        {
            nValue = DBTypeConversion::toStandardDbDate(DBTypeConversion::getNULLDate(_xNumberFormatter->getNumberFormatsSupplier()),nValue);
        }

        return nValue;
    }
    // -----------------------------------------------------------------------------
}

//==================================================================
// class OFieldDescControl
//==================================================================

DBG_NAME(OFieldDescControl)

//==================================================================
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
    ,nCurChildId(1)
    ,m_nPos(-1)
    ,aYes(ModuleRes(STR_VALUE_YES))
    ,aNo(ModuleRes(STR_VALUE_NO))
    ,m_nOldVThumb( 0 )
    ,m_nOldHThumb( 0 )
    ,m_nWidth(50)
    ,nDelayedGrabFocusEvent(0)
    ,m_bAdded(sal_False)
    ,m_bRightAligned(false)
    ,pActFieldDescr(NULL)
{
    DBG_CTOR(OFieldDescControl,NULL);

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
//------------------------------------------------------------------------------
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
    ,nCurChildId(1)
    ,m_nPos(-1)
    ,aYes(ModuleRes(STR_VALUE_YES))
    ,aNo(ModuleRes(STR_VALUE_NO))
    ,m_nOldVThumb( 0 )
    ,m_nOldHThumb( 0 )
    ,m_nWidth(50)
    ,nDelayedGrabFocusEvent(0)
    ,m_bAdded(sal_False)
    ,m_bRightAligned(false)
    ,pActFieldDescr(NULL)
{
    DBG_CTOR(OFieldDescControl,NULL);

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

//------------------------------------------------------------------------------
OFieldDescControl::~OFieldDescControl()
{
    DBG_DTOR(OFieldDescControl,NULL);

    {
        ::std::auto_ptr<Window> aTemp(m_pVertScroll);
        m_pVertScroll    = NULL;
    }
    {
        ::std::auto_ptr<Window> aTemp(m_pHorzScroll);
        m_pHorzScroll    = NULL;
    }
    if ( m_bAdded )
        ::dbaui::notifySystemWindow(this,this,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    pLastFocusWindow = NULL;

    //////////////////////////////////////////////////////////////////////
    // Childs zerstoeren
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

    if(nDelayedGrabFocusEvent)
        Application::RemoveUserEvent(nDelayedGrabFocusEvent);
}

//------------------------------------------------------------------------------
String OFieldDescControl::BoolStringPersistent(const String& rUIString) const
{
    static String aZero('0');
    static String aOne('1');

    if (rUIString == aNo)
        return aZero;
    if (rUIString == aYes)
        return aOne;
    return String();
}

//------------------------------------------------------------------------------
String OFieldDescControl::BoolStringUI(const String& rPersistentString) const
{
    static String aZero('0');
    static String aOne('1');
    static String aNone(ModuleRes(STR_VALUE_NONE));

    // FS - 66161 - 14.05.1999 - aeltere Versionen haben eventuell einen sprachabhaengigen String als Default gespeichert
    if (rPersistentString.Equals(aYes) || rPersistentString.Equals(aNo))
        return rPersistentString;

    if (rPersistentString == aZero)
        return aNo;
    if (rPersistentString == aOne)
        return aYes;

    return aNone;
}

//------------------------------------------------------------------------------
void OFieldDescControl::Init()
{
    Reference< ::com::sun::star::util::XNumberFormatter > xFormatter = GetFormatter();
    ::dbaui::setEvalDateFormatForFormatter(xFormatter);
}

//------------------------------------------------------------------------------
IMPL_LINK(OFieldDescControl, OnScroll, ScrollBar*, /*pBar*/)
{
    ScrollAllAggregates();
    return 0;
}
// -----------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void OFieldDescControl::CheckScrollBars()
{
    // ein paar Berechnungen zur neuen Position der ScrollBars
    Size szOverallSize = GetSizePixel();
    long nHScrollHeight = m_pHorzScroll->GetSizePixel().Height();
    long nVScrollWidth = m_pVertScroll->GetSizePixel().Width();

    long nNewHWidth = szOverallSize.Width() - nVScrollWidth;
    long nNewVHeight = szOverallSize.Height() - nHScrollHeight;

    sal_Bool bNeedHScrollBar(sal_False), bNeedVScrollBar(sal_False);

    // die Bereiche anpassen
    // brauche ich ScrollBars eigentlich ?
    // horizontal :
    long lMaxXPosition = 0;
    Control* ppAggregates[] = { pRequired, pNumType, pAutoIncrement, pDefault, pTextLen, pLength, pScale, pFormat, m_pColumnName, m_pType,m_pAutoIncrementValue};
    for (sal_uInt16 i=0; i<sizeof(ppAggregates)/sizeof(ppAggregates[0]); ++i)
        getMaxXPosition(ppAggregates[i],lMaxXPosition);

    if (m_pHorzScroll)
        lMaxXPosition += m_pHorzScroll->GetThumbPos() * HSCROLL_STEP;

    long lMaxXAvailable = szOverallSize.Width();
    bNeedHScrollBar = lMaxXPosition > lMaxXAvailable;
        // aendert sich vielleicht noch

    // vertikal
    // wieviel Controls habe ich
    sal_uInt16 nActive = CountActiveAggregates();
    // welches ist das letzte, was ganz drauf passt ?
    sal_uInt16 nLastVisible;
    const sal_Int32 nControlHeight = LogicToPixel(Size(0, CONTROL_HEIGHT),MAP_APPFONT).Height();
    const sal_Int32 nControl_Spacing_y = LogicToPixel(Size(0, CONTROL_SPACING_Y),MAP_APPFONT).Height();
    if (bNeedHScrollBar)
        nLastVisible = static_cast<sal_uInt16>((szOverallSize.Height() - nControl_Spacing_y - nHScrollHeight) / (nControl_Spacing_y + nControlHeight));
    else
        nLastVisible = static_cast<sal_uInt16>((szOverallSize.Height() - nControl_Spacing_y) / (nControl_Spacing_y + nControlHeight));
    bNeedVScrollBar = nActive>nLastVisible;

    if (bNeedVScrollBar)
    {
        // in die urspruengliche Berechnung von lMaxXAvailable ist nicht mit eingegangen, dass ich eine VScrollBar habe, also muss ich
        // das nachholen
        lMaxXAvailable -= nVScrollWidth;
        if (!bNeedHScrollBar && (lMaxXPosition > lMaxXAvailable))
        {
            // durch die vertikale brauche ich jetzt ploetzlich doch eine horizontale
            bNeedHScrollBar = sal_True;
            // nLastVisible anpassen
            nLastVisible = static_cast<sal_uInt16>((szOverallSize.Height() - nControl_Spacing_y - nHScrollHeight) / (nControl_Spacing_y + nControlHeight));
                // bNeedVScrollBar aendert sich nicht : es ist schon auf sal_True und nLastVisible wird hoechstens kleiner
        }
    }

    // jetzt kann ich sie wirklich positionieren und ihre Parameter setzen
    if (bNeedVScrollBar)
    {
        m_pVertScroll->Show();
        m_pVertScroll->SetRangeMax(nActive - nLastVisible);
//      m_pVertScroll->SetThumbPos(0);

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
//      m_pHorzScroll->SetThumbPos(0);

        m_pHorzScroll->SetPosSizePixel( Point(0, nNewVHeight), Size(bNeedVScrollBar ? nNewHWidth : szOverallSize.Width(), nHScrollHeight) );
    }
    else
    {
        m_pHorzScroll->Hide();
        m_pHorzScroll->SetRangeMax(0);
        m_pHorzScroll->SetThumbPos(0);
    }
}

//------------------------------------------------------------------------------
void OFieldDescControl::Resize()
{
    CheckScrollBars();
    ScrollAllAggregates();
}

//------------------------------------------------------------------------------
inline void OFieldDescControl::ScrollAggregate(Control* pText, Control* pInput, Control* pButton, long nDeltaX, long nDeltaY)
{
    if  (!pText)
        return;
    pText->SetPosPixel(pText->GetPosPixel() + Point(nDeltaX, nDeltaY));
    pInput->SetPosPixel(pInput->GetPosPixel() + Point(nDeltaX, nDeltaY));
    if (pButton)
        pButton->SetPosPixel(pButton->GetPosPixel() + Point(nDeltaX, nDeltaY));
}

//------------------------------------------------------------------------------
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
        const sal_Int32 nControlHeight = LogicToPixel(Size(0, CONTROL_HEIGHT),MAP_APPFONT).Height();
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

//------------------------------------------------------------------------------
sal_uInt16 OFieldDescControl::CountActiveAggregates() const
{
    Control* ppAggregates[] = { pRequired, pNumType, pAutoIncrement, pDefault, pTextLen, pLength, pScale, pFormat, m_pColumnName, m_pType,m_pAutoIncrementValue};
    sal_uInt16 nVisibleAggregates = 0;
    for (sal_uInt16 i=0; i<sizeof(ppAggregates)/sizeof(ppAggregates[0]); ++i)
        if (ppAggregates[i])
            ++nVisibleAggregates;
    return nVisibleAggregates;
}

//------------------------------------------------------------------------------
void OFieldDescControl::SetReadOnly( sal_Bool bReadOnly )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Controls enablen/disablen
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

//------------------------------------------------------------------------------
String OFieldDescControl::GetControlText( sal_uInt16 nControlId )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Texte der Controls auslesen
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
                return String::CreateFromInt64(pTextLen->GetValue());
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

//------------------------------------------------------------------------------
void OFieldDescControl::SetControlText( sal_uInt16 nControlId, const String& rText )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Texte der Controls setzen
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

//------------------------------------------------------------------------
IMPL_LINK( OFieldDescControl, FormatClickHdl, Button *, /*pButton*/ )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Temporaere Column erzeugen, mit der Datenaustausch mit Dialog erfolgt
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

// -----------------------------------------------------------------------
void OFieldDescControl::SetModified(sal_Bool /*bModified*/)
{
}
//------------------------------------------------------------------------
IMPL_LINK( OFieldDescControl, ChangeHdl, ListBox *, pListBox )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    if ( !pActFieldDescr )
        return 0;

    if ( pListBox->GetSavedValue() != pListBox->GetSelectEntryPos() )
        SetModified(sal_True);

    // Sonderbehandlund f"ur Bool Felder
    if(pListBox == pRequired && pBoolDefault )
    {
        // wenn pRequired auf sal_True gesetzt ist, dann darf das sal_Bool Feld nicht den Eintrag <<keiner>> besitzen
        String sDef = BoolStringUI(::comphelper::getString(pActFieldDescr->GetControlDefault()));

        if(pRequired->GetSelectEntryPos() == 0) // JA
        {
            pBoolDefault->RemoveEntry(String(ModuleRes(STR_VALUE_NONE)));
            if (!sDef.Equals(aYes) && !sDef.Equals(aNo))
                pBoolDefault->SelectEntryPos(1);  // nein als Default
            else
                pBoolDefault->SelectEntry(sDef);
        }
        else if(pBoolDefault->GetEntryCount() < 3)
        {
            pBoolDefault->InsertEntry(String(ModuleRes(STR_VALUE_NONE)));
            pBoolDefault->SelectEntry(sDef);
        }
    }

    // nur fuer AutoIncrement eine Sonderbehandlung
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
        // und jetzt alle nach oben schieben
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
//------------------------------------------------------------------------------
// alle Control neu anordnen, so dass sie in fester Reihenfolge und wirklich
// OBEN auf der DescriptionPage stehen
void OFieldDescControl::ArrangeAggregates()
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    // die Beschreibung eines Controls
    struct AGGREGATE_DESCRIPTION
    {
        Control*    pctrlInputControl;  // das eigentliche Control zur Eingabe
        Control*    pctrlTextControl;   // das Label dazu
        USHORT      nPosSizeArgument;   // das zweite Argument fuer SetPosSize
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

    // und los ...
    int nCurrentControlPos = 0;
    Control* pZOrderPredecessor = NULL;
    for (size_t i=0; i<sizeof(adAggregates)/sizeof(adAggregates[0]); i++)
    {
        if (adAggregates[i].pctrlInputControl)
        {
            SetPosSize(&adAggregates[i].pctrlTextControl, nCurrentControlPos, 0);
            SetPosSize(&adAggregates[i].pctrlInputControl, nCurrentControlPos, adAggregates[i].nPosSizeArgument);

            // die Z-Order so, dass die Controls auch wirklich in derselben Reihenfolge durchwandert werden koennen, in der sie
            // hier angeordnet wurden
            adAggregates[i].pctrlInputControl->SetZOrder(pZOrderPredecessor, pZOrderPredecessor ? WINDOW_ZORDER_BEHIND : WINDOW_ZORDER_FIRST);
            pZOrderPredecessor = adAggregates[i].pctrlInputControl;

            if (adAggregates[i].pctrlInputControl == pFormatSample)
            {
                pFormat->SetZOrder(pZOrderPredecessor, WINDOW_ZORDER_BEHIND);
                pZOrderPredecessor = pFormat;
            }

            ++nCurrentControlPos;
        }
    }

    // eine Sonderbehandlung fuer die Format-Controls
    if (pFormat)
    {
        Point ptSamplePos(pFormatSample->GetPosPixel());
        Size szSampleSize(pFormatSample->GetSizePixel());
        pFormat->SetPosPixel(Point(ptSamplePos.X() + szSampleSize.Width() + 5, ptSamplePos.Y()));
    }

    // als letztes noch die ScrollBars in der ZOrder ganz nach oben
    m_pVertScroll->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
    m_pHorzScroll->SetZOrder(NULL, WINDOW_ZORDER_FIRST);
}

//------------------------------------------------------------------------------
void OFieldDescControl::ActivateAggregate( EControlType eType )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Controls erzeugen
    switch( eType )
    {
    case tpDefault:
        if( pDefault )
            return;
        m_nPos++;
        pDefaultText  =new FixedText( this );
        pDefaultText->SetText( ModuleRes(STR_DEFAULT_VALUE) );
        pDefault = new OPropEditCtrl( this, STR_HELP_DEFAULT_VALUE, FIELD_PROPERTY_DEFAULT, WB_BORDER );
        pDefault->SetHelpId(HID_TAB_ENT_DEFAULT);
        //  SetPosSize( (Control**)&pDefaultText, m_nPos, 0 );
        //  SetPosSize( (Control**)&pDefault, m_nPos, 3 );

        pDefault->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pDefault->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pDefaultText->EnableClipSiblings();
        pDefault->EnableClipSiblings();
        break;
    case tpAutoIncrementValue:
        if( m_pAutoIncrementValue || !isAutoIncrementValueEnabled() )
            return;
        m_nPos++;
        m_pAutoIncrementValueText  =new FixedText( this );
        m_pAutoIncrementValueText->SetText( ModuleRes(STR_AUTOINCREMENT_VALUE) );
        m_pAutoIncrementValue = new OPropEditCtrl( this, STR_HELP_AUTOINCREMENT_VALUE, FIELD_PRPOERTY_AUTOINCREMENT, WB_BORDER );
        m_pAutoIncrementValue->SetHelpId(HID_TAB_AUTOINCREMENTVALUE);
        //  SetPosSize( (Control**)&m_pAutoIncrementValueText, m_nPos, 0 );
        //  SetPosSize( (Control**)&m_pAutoIncrementValue, m_nPos, 3 );

        m_pAutoIncrementValue->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        m_pAutoIncrementValue->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        m_pAutoIncrementValueText->EnableClipSiblings();
        m_pAutoIncrementValue->EnableClipSiblings();
        m_pAutoIncrementValue->SetText( getAutoIncrementValue() );
        break;

    case tpRequired:
    {
        if( pRequired )
            return;
        Reference< XDatabaseMetaData> xMetaData = getMetaData();

        if(xMetaData.is() && xMetaData->supportsNonNullableColumns())
        {
            m_nPos++;
            pRequiredText  =new FixedText( this );
            pRequiredText->SetText( ModuleRes(STR_FIELD_REQUIRED) );
            pRequired = new OPropListBoxCtrl( this, STR_HELP_FIELD_REQUIRED, FIELD_PROPERTY_REQUIRED, WB_DROPDOWN);
            pRequired->SetHelpId(HID_TAB_ENT_REQUIRED);

            pRequired->InsertEntry( aYes );
            pRequired->InsertEntry( aNo );
            pRequired->SelectEntryPos(1);
            pRequired->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

            pRequired->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
            pRequired->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

            pRequiredText->EnableClipSiblings();
            pRequired->EnableClipSiblings();
        }
    }
    break;
    case tpAutoIncrement:
    {
        if( pAutoIncrement )
            return;
        m_nPos++;
        pAutoIncrementText  =new FixedText( this );
        pAutoIncrementText->SetText( ModuleRes(STR_FIELD_AUTOINCREMENT) );
        pAutoIncrement = new OPropListBoxCtrl( this, STR_HELP_AUTOINCREMENT, FIELD_PROPERTY_AUTOINC, WB_DROPDOWN );
        pAutoIncrement->SetHelpId(HID_TAB_ENT_AUTOINCREMENT);

        pAutoIncrement->InsertEntry( aYes );
        pAutoIncrement->InsertEntry( aNo );
        pAutoIncrement->SelectEntryPos(0);
        pAutoIncrement->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

        pAutoIncrement->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pAutoIncrement->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pAutoIncrementText->EnableClipSiblings();
        pAutoIncrement->EnableClipSiblings();
    }
    break;
    case tpTextLen:
        if( pTextLen )
            return;
        m_nPos++;
        pTextLenText  =new FixedText( this );
        pTextLenText->SetText( ModuleRes(STR_TEXT_LENGTH) );

        pTextLen = new OPropNumericEditCtrl( this, STR_HELP_TEXT_LENGTH, FIELD_PROPERTY_TEXTLEN, WB_BORDER );
        pTextLen->SetDecimalDigits(0);
        pTextLen->SetMin(0);
        pTextLen->SetMax(0x7FFFFFFF);   // soll draussen geaendert werden, wenn noetig
        pTextLen->SetStrictFormat(TRUE);

        pTextLen->SetHelpId(HID_TAB_ENT_TEXT_LEN);

        pTextLen->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pTextLen->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pTextLenText->EnableClipSiblings();
        pTextLen->EnableClipSiblings();
        break;

    case tpType:
        if( m_pType)
            return;
        m_nPos++;
        m_pTypeText  =new FixedText( this );
        m_pTypeText->SetText( ModuleRes(STR_TAB_FIELD_DATATYPE) );
        m_pType = new OPropListBoxCtrl( this, STR_HELP_AUTOINCREMENT, FIELD_PRPOERTY_TYPE, WB_DROPDOWN );
        m_pType->SetHelpId(HID_TAB_ENT_TYPE);
        m_pType->SetDropDownLineCount(20);
        {
            const OTypeInfoMap* pTypeInfo = getTypeInfo();
            OTypeInfoMap::const_iterator aIter = pTypeInfo->begin();
            for(;aIter != pTypeInfo->end();++aIter)
                m_pType->InsertEntry( aIter->second->aUIName );
        }
        m_pType->SelectEntryPos(0);
        m_pType->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

        m_pType->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        m_pType->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        m_pTypeText->EnableClipSiblings();
        m_pType->EnableClipSiblings();
        break;
    case tpColumnName:
        if( m_pColumnName )
            return;
        m_nPos++;
        {
            Reference< XDatabaseMetaData> xMetaData = getMetaData();
            sal_uInt32 nMax = xMetaData.is() ? xMetaData->getMaxColumnNameLength() : EDIT_NOLIMIT;
            m_pColumnNameText  =new FixedText( this );
            m_pColumnNameText->SetText( ModuleRes(STR_TAB_FIELD_NAME) );
            ::rtl::OUString aTmpString( xMetaData.is() ? xMetaData->getExtraNameCharacters() : ::rtl::OUString() );
            m_pColumnName = new OPropColumnEditCtrl( this,
                                                    aTmpString,
                                                    STR_HELP_DEFAULT_VALUE,
                                                    FIELD_PRPOERTY_COLUMNNAME,
                                                    WB_BORDER );
            m_pColumnName->SetHelpId(HID_TAB_ENT_COLUMNNAME);
            m_pColumnName->SetMaxTextLen(xub_StrLen( nMax ? nMax : EDIT_NOLIMIT));
            m_pColumnName->setCheck( isSQL92CheckEnabled(getConnection()) );
        }

        m_pColumnName->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        m_pColumnName->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        m_pColumnNameText->EnableClipSiblings();
        m_pColumnName->EnableClipSiblings();
        break;
    case tpNumType:
        if( pNumType )
            return;
        m_nPos++;
        pNumTypeText  =new FixedText( this );
        pNumTypeText->SetText( ModuleRes(STR_NUMERIC_TYPE) );

        pNumType = new OPropListBoxCtrl( this, STR_HELP_NUMERIC_TYPE, FIELD_PROPERTY_NUMTYPE, WB_DROPDOWN );
        pNumType->SetDropDownLineCount(5);
        pNumType->SetHelpId(HID_TAB_ENT_NUMTYP);

        pNumType->InsertEntry( String::CreateFromAscii("Byte") );
        pNumType->InsertEntry( String::CreateFromAscii("SmallInt") );
        pNumType->InsertEntry( String::CreateFromAscii("Integer") );
        pNumType->InsertEntry( String::CreateFromAscii("Single") );
        pNumType->InsertEntry( String::CreateFromAscii("Double") );
        pNumType->SelectEntryPos(2);
        pNumType->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

        pNumType->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pNumType->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pNumTypeText->EnableClipSiblings();
        pNumType->EnableClipSiblings();
        break;

    case tpLength:
        if( pLength )
            return;
        m_nPos++;
        pLengthText  =new FixedText( this );
        pLengthText->SetText( ModuleRes(STR_LENGTH) );

        pLength = new OPropNumericEditCtrl( this, STR_HELP_LENGTH, FIELD_PROPERTY_LENGTH, WB_BORDER );
        pLength->SetDecimalDigits(0);
        pLength->SetMin(0);
        pLength->SetMax(0x7FFFFFFF);    // soll draussen geaendert werden, wenn noetig
        pLength->SetStrictFormat(TRUE);

        pLength->SetHelpId(HID_TAB_ENT_LEN);

        pLength->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pLength->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pLengthText->EnableClipSiblings();
        pLength->EnableClipSiblings();
        break;

    case tpScale:
        if( pScale )
            return;
        m_nPos++;
        pScaleText  =new FixedText( this );
        pScaleText->SetText( ModuleRes(STR_SCALE) );
        pScale = new OPropNumericEditCtrl( this, STR_HELP_SCALE, FIELD_PROPERTY_SCALE, WB_BORDER );
        pScale->SetDecimalDigits(0);
        pScale->SetMin(0);
        pScale->SetMax(0x7FFFFFFF); // soll draussen geaendert werden, wenn noetig
        pScale->SetStrictFormat(TRUE);

        pScale->SetHelpId(HID_TAB_ENT_SCALE);

        pScale->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pScale->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pScaleText->EnableClipSiblings();
        pScale->EnableClipSiblings();
        break;

    case tpFormat:
        if (!pFormat)
        {
            m_nPos++;
            pFormatText  =new FixedText( this );
            pFormatText->SetText( ModuleRes(STR_FORMAT) );

            pFormatSample = new OPropEditCtrl( this, STR_HELP_FORMAT_CODE, -1, WB_BORDER );
            pFormatSample->SetHelpId(HID_TAB_ENT_FORMAT_SAMPLE);
            pFormatSample->SetReadOnly(sal_True);
            pFormatSample->Enable(sal_False);

            pFormat = new PushButton( this, ModuleRes(PB_FORMAT) );
            pFormat->SetHelpId(HID_TAB_ENT_FORMAT);
            const sal_Int32 nControlHeight = LogicToPixel(Size(0, CONTROL_HEIGHT),MAP_APPFONT).Height();
            pFormat->SetSizePixel(Size(nControlHeight, nControlHeight));
            pFormat->SetClickHdl( LINK( this, OFieldDescControl, FormatClickHdl ) );

            pFormatSample->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
            pFormatSample->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));
            pFormat->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
            pFormat->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

            pFormatText->EnableClipSiblings();
            pFormatSample->EnableClipSiblings();
            pFormat->EnableClipSiblings();
        }

        UpdateFormatSample(pActFieldDescr);
        break;
    case tpBoolDefault:
        if (pBoolDefault)
            return;

        m_nPos++;
        pBoolDefaultText  =new FixedText(this);
        pBoolDefaultText->SetText(ModuleRes(STR_DEFAULT_VALUE));
        pBoolDefault = new OPropListBoxCtrl( this, STR_HELP_BOOL_DEFAULT, FIELD_PROPERTY_BOOL_DEFAULT, WB_DROPDOWN );
        pBoolDefault->SetDropDownLineCount(3);
        pBoolDefault->InsertEntry(String(ModuleRes(STR_VALUE_NONE)));
        pBoolDefault->InsertEntry(aYes);
        pBoolDefault->InsertEntry(aNo);
        pBoolDefault->SetHelpId(HID_TAB_ENT_BOOL_DEFAULT);

        pBoolDefault->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pBoolDefault->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pBoolDefaultText->EnableClipSiblings();
        pBoolDefault->EnableClipSiblings();
        break;

    }
}

//------------------------------------------------------------------------------
void OFieldDescControl::DeactivateAggregate( EControlType eType )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    pLastFocusWindow = NULL;
    //////////////////////////////////////////////////////////////////////
    // Controls zerstoeren
    switch( eType )
    {
    case tpDefault:
        if( !pDefault )
            return;
        m_nPos--;
        pDefault->Hide();
        pDefaultText->Hide();
        delete pDefault;
        delete pDefaultText;
        pDefault = NULL;
        pDefaultText  =NULL;
        break;

    case tpAutoIncrementValue:
        if( !m_pAutoIncrementValue )
            return;
        m_nPos--;
        m_pAutoIncrementValue->Hide();
        m_pAutoIncrementValueText->Hide();
        delete m_pAutoIncrementValue;
        delete m_pAutoIncrementValueText;
        m_pAutoIncrementValue = NULL;
        m_pAutoIncrementValueText  =NULL;
        break;

    case tpColumnName:
        if( !m_pColumnName )
            return;
        m_nPos--;
        m_pColumnName->Hide();
        m_pColumnNameText->Hide();
        delete m_pColumnName;
        delete m_pColumnNameText;
        m_pColumnName = NULL;
        m_pColumnNameText  =NULL;
        break;

    case tpType:
        if( !m_pType )
            return;
        m_nPos--;
        m_pType->Hide();
        m_pTypeText->Hide();
        delete m_pType;
        delete m_pTypeText;
        m_pType = NULL;
        m_pTypeText  =NULL;
        break;

    case tpAutoIncrement:
        if( !pAutoIncrement )
            return;
        m_nPos--;
        pAutoIncrement->Hide();
        pAutoIncrementText->Hide();
        delete pAutoIncrement;
        delete pAutoIncrementText;
        pAutoIncrement = NULL;
        pAutoIncrementText  =NULL;
        break;

    case tpRequired:
        if( !pRequired )
            return;
        m_nPos--;
        pRequired->Hide();
        pRequiredText->Hide();
        delete pRequired;
        delete pRequiredText;
        pRequired = NULL;
        pRequiredText  =NULL;
        break;

    case tpTextLen:
        if( !pTextLen )
            return;
        m_nPos--;
        pTextLen->Hide();
        pTextLenText->Hide();
        delete pTextLen;
        delete pTextLenText;
        pTextLen = NULL;
        pTextLenText  =NULL;
        break;

    case tpNumType:
        if( !pNumType )
            return;
        m_nPos--;
        pNumType->Hide();
        pNumTypeText->Hide();
        delete pNumType;
        delete pNumTypeText;
        pNumType = NULL;
        pNumTypeText  =NULL;
        break;

    case tpLength:
        if( !pLength )
            return;
        m_nPos--;
        pLength->Hide();
        pLengthText->Hide();
        delete pLength;
        delete pLengthText;
        pLength = NULL;
        pLengthText  =NULL;
        break;

    case tpScale:
        if( !pScale )
            return;
        m_nPos--;
        pScale->Hide();
        pScaleText->Hide();
        delete pScale;
        delete pScaleText;
        pScale = NULL;
        pScaleText  =NULL;
        break;

    case tpFormat:
        if( !pFormat )
            return;
        pFormatText->Hide();
        pFormatSample->Hide();
        pFormat->Hide();
        delete pFormatText;
        delete pFormatSample;
        delete pFormat;
        pFormatText  =NULL;
        pFormatSample = NULL;
        pFormat = NULL;
        break;
    case tpBoolDefault:
        if (!pBoolDefault)
            return;
        m_nPos--;
        pBoolDefault->Hide();
        pBoolDefaultText->Hide();
        delete pBoolDefault;
        delete pBoolDefaultText;
        pBoolDefault = NULL;
        pBoolDefaultText  =NULL;
        break;

    }
}

//------------------------------------------------------------------------------
void OFieldDescControl::SetPosSize( Control** ppControl, long nRow, sal_uInt16 nCol )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);

    //////////////////////////////////////////////////////////////////////
    // Groesse ermitteln
    const sal_Int32 nControlHeight = LogicToPixel(Size(0, CONTROL_HEIGHT),MAP_APPFONT).Height();
    Size aSize;
    switch( nCol )
    {
    case 0:
        aSize.Width()  = CONTROL_WIDTH_1;
        aSize.Height() = nControlHeight;
        break;
    case 1:
        if ( isRightAligned() )
            aSize.Width() = LogicToPixel(Size(m_nWidth, 0),MAP_APPFONT).Width();
        else
            aSize.Width()  = CONTROL_WIDTH_2;
        aSize.Height() = nControlHeight;
        break;
    case 2:
        if ( isRightAligned() )
            aSize.Width() = LogicToPixel(Size(m_nWidth, 0),MAP_APPFONT).Width();
        else
            aSize.Width()  = CONTROL_WIDTH_2;
        aSize.Height() = long(1.5*nControlHeight);
        break;
    case 3:
        if ( isRightAligned() )
            aSize.Width() = LogicToPixel(Size(m_nWidth, 0),MAP_APPFONT).Width();
        else
            aSize.Width()  = CONTROL_WIDTH_3;
        aSize.Height() = nControlHeight;
        break;
    case 4:
        if ( isRightAligned() )
            aSize.Width() = LogicToPixel(Size(m_nWidth, 0),MAP_APPFONT).Width();
        else
            aSize.Width()  = CONTROL_WIDTH_4;
        aSize.Height() = nControlHeight;
        break;
    default:
        if ( isRightAligned() )
            aSize.Width() = LogicToPixel(Size(m_nWidth, 0),MAP_APPFONT).Width();
        else
            aSize.Width()  = CONTROL_WIDTH_1;
        aSize.Height() = nControlHeight;
    }


    //////////////////////////////////////////////////////////////////////
    // Position ermitteln
    Point aPosition;
    switch( nCol )
    {
    case 0:
        aPosition.X() = 0;
        aPosition.Y() = 1;
        break;
    case 1:
    case 2:
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

    //////////////////////////////////////////////////////////////////////
    // Control anzeigen
    (*ppControl)->SetPosSizePixel( aPosition, aSize );
    aSize = (*ppControl)->GetSizePixel();

    (*ppControl)->Show();
}
//------------------------------------------------------------------------------
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
        //////////////////////////////////////////////////////////////////////
        // Zeiger des gespeicherten Focus zuruecksetzen
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
    //////////////////////////////////////////////////////////////////////
    // Wenn sich der Typ geaendert hat, Controls austauschen
    if( m_pPreviousType != pFieldType )
    {
        //////////////////////////////////////////////////////////////////////
        // Zeiger des gespeicherten Focus zuruecksetzen
        pLastFocusWindow = NULL;

        //////////////////////////////////////////////////////////////////////
        // Controls, die nicht mehr angezeigt werden duerfen
        DeactivateAggregate( tpNumType );

        //////////////////////////////////////////////////////////////////////
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
            pLength->SetSpecialReadOnly(pFieldType->aCreateParams.getLength()==0);
        }
        else
            DeactivateAggregate( tpLength );

        if (pFieldType->nMaximumScale)
        {
            ActivateAggregate( tpScale );
            pScale->SetMax(::std::max<sal_Int32>(pFieldType->nMaximumScale,pFieldDescr->GetScale()));
            pScale->SetMin(pFieldType->nMinimumScale);
            static const ::rtl::OUString s_sPRECISION(RTL_CONSTASCII_USTRINGPARAM("PRECISION"));
            pScale->SetSpecialReadOnly(pFieldType->aCreateParams.getLength() == 0 || pFieldType->aCreateParams == s_sPRECISION);
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
                    pTextLen->SetSpecialReadOnly(pFieldType->aCreateParams.getLength()==0);
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
                OSL_ENSURE(0,"Unknown type");
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
    //////////////////////////////////////////////////////////////////////
    // Controls initialisieren
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
            // hat Auswirkungen auf pRequired
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
        // wenn pRequired auf sal_True gesetzt ist, dann darf das sal_Bool Feld nicht den Eintrag <<keiner>> besitzen
        String sDef = BoolStringUI(::comphelper::getString(pFieldDescr->GetControlDefault()));

        // sicher stellen das <<keiner>> nur vorhanden ist, wenn das Feld NULL sein darf
        if ( pFieldType.get() && !pFieldType->bNullable || !pFieldDescr->IsNullable() )
        {
            pFieldDescr->SetIsNullable(ColumnValue::NO_NULLS); // der Typ sagt das

            pBoolDefault->RemoveEntry(String(ModuleRes(STR_VALUE_NONE)));
            if ( !sDef.Equals(aYes) && !sDef.Equals(aNo) )
                pBoolDefault->SelectEntryPos(1);  // nein als Default
            else
                pBoolDefault->SelectEntry(sDef);

            pFieldDescr->SetControlDefault(makeAny(::rtl::OUString(BoolStringPersistent(pBoolDefault->GetSelectEntry()))));
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
        pTextLen->SetText( String::CreateFromInt32(pFieldDescr->GetPrecision()) );
        pTextLen->ClearModifyFlag();
    }

    if( pNumType )
    {
        OSL_ENSURE(sal_False, "OFieldDescControl::DisplayData: invalid num type!");
    }

    if( pLength )
        pLength->SetText( String::CreateFromInt32(pFieldDescr->GetPrecision()) );

    if( pScale )
        pScale->SetText( String::CreateFromInt32(pFieldDescr->GetScale()) );

    if( pFormat )
        UpdateFormatSample(pFieldDescr);

    if(m_pColumnName)
        m_pColumnName->SetText(pFieldDescr->GetName());

    if(m_pType)
    {
        USHORT nPos = pFieldType.get() ? m_pType->GetEntryPos(String(pFieldDescr->getTypeInfo()->aUIName)) : LISTBOX_ENTRY_NOTFOUND;
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


    //////////////////////////////////////////////////////////////////////
    // Controls Enablen/Disablen
    sal_Bool bRead(IsReadOnly());


    ArrangeAggregates();
    CheckScrollBars();
    ScrollAllAggregates();

    SetReadOnly( bRead );
}
//------------------------------------------------------------------------
IMPL_LINK(OFieldDescControl, DelayedGrabFocus, Control**, ppControl)
{
    nDelayedGrabFocusEvent = 0;
    if (*ppControl)
        (*ppControl)->GrabFocus();

    return 0L;
}

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
void OFieldDescControl::SaveData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    if( !pFieldDescr )
        return;

    //////////////////////////////////////////////////////////////////////
    // Controls auslesen
    ::rtl::OUString sDefault;
    if (pDefault)
    {
        if ( pDefault->GetSavedValue() != pDefault->GetText() )
            sDefault = pDefault->GetText();
    }
    else if (pBoolDefault)
    {
        sDefault = BoolStringPersistent(pBoolDefault->GetSelectEntry());
    }

    if ( sDefault.getLength() )
    {
        sal_uInt32 nFormatKey;
        try
        {
            if ( isTextFormat(pFieldDescr,nFormatKey) || pBoolDefault )
            {
                pFieldDescr->SetControlDefault(makeAny(sDefault));
            }
            else
            {
                try
                {
                    double nValue = GetFormatter()->convertStringToNumber(nFormatKey,sDefault);
                    nValue = checkDoubleForDateFormat(nValue,nFormatKey,GetFormatter());
                    pFieldDescr->SetControlDefault(makeAny(nValue));
                }
                catch(const Exception&)
                {
                    if ( sDefault.getLength() )
                        pFieldDescr->SetControlDefault(makeAny(sDefault));
                    else
                        pFieldDescr->SetControlDefault(Any());
                }
            }
        }
        catch(const Exception&)
        {
        }
    } // if ( sDefault.getLength() )
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

//------------------------------------------------------------------------------
void OFieldDescControl::UpdateFormatSample(OFieldDescription* pFieldDescr)
{
    if(!pFieldDescr)
        return;
    if(!pFormatSample)
        return;

    pFormatSample->SetText(getControlDefault(pFieldDescr,sal_False));
}

//------------------------------------------------------------------------------
void OFieldDescControl::GetFocus()
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Setzt den Focus auf das zuletzt aktive Control
    TabPage::GetFocus();
    if( pLastFocusWindow )
    {
        pLastFocusWindow->GrabFocus();
        pLastFocusWindow = NULL;
    }
}

//------------------------------------------------------------------------------
void OFieldDescControl::implFocusLost(Window* _pWhich)
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    DBG_ASSERT(!_pWhich || IsChild(_pWhich), "OFieldDescControl::implFocusLost : invalid window !");

    //////////////////////////////////////////////////////////////////////
    // Das aktive Control merken
    if (!pLastFocusWindow)
        pLastFocusWindow = _pWhich;

    //////////////////////////////////////////////////////////////////////
    // HelpText zuruecksetzen
    if (pHelp && !pHelp->HasChildPathFocus())
        pHelp->SetHelpText( String() );
}

//------------------------------------------------------------------------------
void OFieldDescControl::LoseFocus()
{
    DBG_CHKTHIS(OFieldDescControl,NULL);

    implFocusLost(NULL);

    TabPage::LoseFocus();
}
// -----------------------------------------------------------------------------
sal_Bool OFieldDescControl::isCopyAllowed()
{
    sal_Bool bAllowed = (m_pActFocusWindow != NULL) &&
                        (m_pActFocusWindow == pDefault || m_pActFocusWindow == pFormatSample    ||
                        m_pActFocusWindow == pTextLen || m_pActFocusWindow == pLength           ||
                        m_pActFocusWindow == pScale  || m_pActFocusWindow == m_pColumnName      ||
                        m_pActFocusWindow == m_pAutoIncrementValue) &&
                        static_cast<Edit*>(m_pActFocusWindow)->GetSelected().Len() != 0;

    return bAllowed;
}
// -----------------------------------------------------------------------------
sal_Bool OFieldDescControl::isCutAllowed()
{
    sal_Bool bAllowed = (m_pActFocusWindow != NULL) &&
                        (m_pActFocusWindow == pDefault || m_pActFocusWindow == pFormatSample    ||
                        m_pActFocusWindow == pTextLen || m_pActFocusWindow == pLength           ||
                        m_pActFocusWindow == pScale  || m_pActFocusWindow == m_pColumnName      ||
                        m_pActFocusWindow == m_pAutoIncrementValue) &&
                        static_cast<Edit*>(m_pActFocusWindow)->GetSelected().Len() != 0;
    return bAllowed;
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void OFieldDescControl::cut()
{
    if(isCutAllowed())
        static_cast<Edit*>(m_pActFocusWindow)->Cut();
}
// -----------------------------------------------------------------------------
void OFieldDescControl::copy()
{
    if(isCopyAllowed()) // this only checks if the focus window is valid
        static_cast<Edit*>(m_pActFocusWindow)->Copy();
}
// -----------------------------------------------------------------------------
void OFieldDescControl::paste()
{
    if(m_pActFocusWindow) // this only checks if the focus window is valid
        static_cast<Edit*>(m_pActFocusWindow)->Paste();
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
String OFieldDescControl::getControlDefault( const OFieldDescription* _pFieldDescr ,sal_Bool _bCheck) const
{
    ::rtl::OUString sDefault;
    sal_Bool bCheck = !_bCheck || _pFieldDescr->GetControlDefault().hasValue();
    if ( bCheck )
    {
        sal_uInt32 nFormatKey;
        sal_Bool bTextFormat = sal_False;
        double nValue = 0.0;

        try
        {
            bTextFormat = isTextFormat(_pFieldDescr,nFormatKey);
            if ( _pFieldDescr->GetControlDefault() >>= sDefault )
            {
                if ( !bTextFormat )
                {
                    if ( sDefault.getLength() )
                    {
                        try
                        {
                            nValue = GetFormatter()->convertStringToNumber(nFormatKey,sDefault);
                        }
                        catch(const Exception&)
                        {
                            return ::rtl::OUString(); // return empty string for format example
                        }
                    }
                }
            }
            else
                _pFieldDescr->GetControlDefault() >>= nValue;


            Reference< ::com::sun::star::util::XNumberFormatter> xNumberFormatter = GetFormatter();
            Reference<XPropertySet> xFormSet = xNumberFormatter->getNumberFormatsSupplier()->getNumberFormats()->getByKey(nFormatKey);
            OSL_ENSURE(xFormSet.is(),"XPropertySet is null!");
            ::rtl::OUString sFormat;
            xFormSet->getPropertyValue(::rtl::OUString::createFromAscii("FormatString")) >>= sFormat;

            if ( !bTextFormat )
            {
                Locale aLocale;
                ::comphelper::getNumberFormatProperty(xNumberFormatter,nFormatKey,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Locale"))) >>= aLocale;

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
            else if ( !_bCheck || (sDefault.getLength() != 0) )
                sDefault = xNumberFormatter->formatString(nFormatKey,(sDefault.getLength() != 0 )? sDefault : sFormat);
        }
        catch(const Exception&)
        {

        }
    }

    return sDefault;
}
// -----------------------------------------------------------------------------





