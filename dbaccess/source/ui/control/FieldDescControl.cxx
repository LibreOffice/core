/*************************************************************************
 *
 *  $RCSfile: FieldDescControl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-14 07:37:39 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#define ITEMID_HORJUSTIFY       SID_ATTR_ALIGN_HOR_JUSTIFY
#define ITEMID_VERJUSTIFY       SID_ATTR_ALIGN_VER_JUSTIFY
#define ITEMID_ORIENTATION      SID_ATTR_ALIGN_ORIENTATION
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
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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

using namespace dbaui;
//  using namespace comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
//  using namespace ::com::sun::star::sdb;

//==================================================================

// fuer die Controls auf der OFieldDescGenPage
#define CONTROL_SPACING_X   18  // 6
#define CONTROL_SPACING_Y   5
#define CONTROL_HEIGHT      20
#define CONTROL_WIDTH_1     140 // 100
#define CONTROL_WIDTH_2     100 // 60
#define CONTROL_WIDTH_3     250
#define CONTROL_WIDTH_4     (CONTROL_WIDTH_3 - CONTROL_HEIGHT - 5)

#define SBA_DEF_RANGEFORMAT         (100 + 143) // RangeItem
#define SBA_DEF_FMTVALUE            (100 + 144) // SfxULONG, Format
#define SBA_ATTR_ALIGN_HOR_JUSTIFY  (100 + 145) //  SvxHorJustifyItem

#define HSCROLL_STEP        20

//==================================================================
// class OFieldDescControl
//==================================================================

DBG_NAME(OFieldDescControl);

//==================================================================
OFieldDescControl::OFieldDescControl( Window* pParent, const ResId& rResId, OTableDesignHelpBar* pHelpBar)
    :TabPage( pParent, rResId )
    ,m_pPreviousType( NULL )
    ,pHelp( pHelpBar )
    ,nCurChildId(1)
    ,pDefault(NULL)
    ,pDefaultText(NULL)
    ,pRequired(NULL)
    ,pRequiredText(NULL)
    ,pAutoIncrement(NULL)
    ,pAutoIncrementText(NULL)
    ,pTextLen(NULL)
    ,pTextLenText(NULL)
    ,pNumType(NULL)
    ,pNumTypeText(NULL)
    ,pLastFocusWindow(NULL)
    ,pFormatSample(NULL)
    ,pFormatText(NULL)
    ,pFormat(NULL)
    ,pLength(NULL)
    ,pLengthText(NULL)
    ,pScale(NULL)
    ,pScaleText(NULL)
    ,pBoolDefault(NULL)
    ,pBoolDefaultText(NULL)
    ,m_pColumnName(NULL)
    ,m_pColumnNameText(NULL)
    ,m_pType(NULL)
    ,m_pTypeText(NULL)
    ,nPos(-1)
    ,aYes(ModuleRes(STR_VALUE_YES))
    ,aNo(ModuleRes(STR_VALUE_NO))
    ,nDelayedGrabFocusEvent(0)
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
    ,m_pPreviousType( NULL )
    ,pHelp( pHelpBar )
    ,nCurChildId(1)
    ,pDefault(NULL)
    ,pDefaultText(NULL)
    ,pRequired(NULL)
    ,pRequiredText(NULL)
    ,pAutoIncrement(NULL)
    ,pAutoIncrementText(NULL)
    ,pTextLen(NULL)
    ,pTextLenText(NULL)
    ,pNumType(NULL)
    ,pNumTypeText(NULL)
    ,pLastFocusWindow(NULL)
    ,pFormatSample(NULL)
    ,pFormatText(NULL)
    ,pFormat(NULL)
    ,pLength(NULL)
    ,pLengthText(NULL)
    ,pScale(NULL)
    ,pScaleText(NULL)
    ,pBoolDefault(NULL)
    ,pBoolDefaultText(NULL)
    ,m_pColumnName(NULL)
    ,m_pColumnNameText(NULL)
    ,m_pType(NULL)
    ,m_pTypeText(NULL)
    ,nPos(-1)
    ,aYes(ModuleRes(STR_VALUE_YES))
    ,aNo(ModuleRes(STR_VALUE_NO))
    ,nDelayedGrabFocusEvent(0)
    ,pActFieldDescr(0)
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

    delete m_pVertScroll;
    delete m_pHorzScroll;

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

    if(nDelayedGrabFocusEvent)
        Application::RemoveUserEvent(nDelayedGrabFocusEvent);
}

//------------------------------------------------------------------------------
String OFieldDescControl::BoolStringPersistent(const String& rUIString) const
{
    static String aZero('0');
    static String aOne('1');
    static String aNo(ModuleRes(STR_VALUE_NO));
    static String aYes(ModuleRes(STR_VALUE_YES));

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
    static String aYes(ModuleRes(STR_VALUE_YES));
    static String aNo(ModuleRes(STR_VALUE_NO));
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
}

//------------------------------------------------------------------------------
IMPL_LINK(OFieldDescControl, OnScroll, ScrollBar*, pBar)
{
    ScrollAllAggregates();
    return 0;
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
    if (pRequired)
        lMaxXPosition = ::std::max(lMaxXPosition, pRequired->GetSizePixel().Width() + pRequired->GetPosPixel().X());
    if (pNumType)
        lMaxXPosition = ::std::max(lMaxXPosition, pNumType->GetSizePixel().Width() + pNumType->GetPosPixel().X());
    if (pAutoIncrement)
        lMaxXPosition = ::std::max(lMaxXPosition, pAutoIncrement->GetSizePixel().Width() + pAutoIncrement->GetPosPixel().X());
    if (pDefault)
        lMaxXPosition = ::std::max(lMaxXPosition, pDefault->GetSizePixel().Width() + pDefault->GetPosPixel().X());
    if (pTextLen)
        lMaxXPosition = ::std::max(lMaxXPosition, pTextLen->GetSizePixel().Width() + pTextLen->GetPosPixel().X());
    if (pLength)
        lMaxXPosition = ::std::max(lMaxXPosition, pLength->GetSizePixel().Width() + pLength->GetPosPixel().X());
    if (pScale)
        lMaxXPosition = ::std::max(lMaxXPosition, pScale->GetSizePixel().Width() + pScale->GetPosPixel().X());
    if (pFormat)
        lMaxXPosition = ::std::max(lMaxXPosition, pFormat->GetSizePixel().Width() + pFormat->GetPosPixel().X());
    if (m_pColumnName)
        lMaxXPosition = ::std::max(lMaxXPosition, m_pColumnName->GetSizePixel().Width() + m_pColumnName->GetPosPixel().X());
    if (m_pType)
        lMaxXPosition = ::std::max(lMaxXPosition, m_pType->GetSizePixel().Width() + m_pType->GetPosPixel().X());

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
    if (bNeedHScrollBar)
        nLastVisible = (szOverallSize.Height() - CONTROL_SPACING_Y - nHScrollHeight) / (CONTROL_SPACING_Y + CONTROL_HEIGHT);
    else
        nLastVisible = (szOverallSize.Height() - CONTROL_SPACING_Y) / (CONTROL_SPACING_Y + CONTROL_HEIGHT);
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
            nLastVisible = (szOverallSize.Height() - CONTROL_SPACING_Y - nHScrollHeight) / (CONTROL_SPACING_Y + CONTROL_HEIGHT);
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
        nDeltaY = (m_nOldVThumb - m_pVertScroll->GetThumbPos()) * (CONTROL_SPACING_Y + CONTROL_HEIGHT);
        m_nOldVThumb = m_pVertScroll->GetThumbPos();
    }

    if (nDeltaX || nDeltaY)
    {
        ScrollAggregate(pRequiredText, pRequired, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(pNumTypeText, pNumType, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(pAutoIncrementText, pAutoIncrement, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(pDefaultText, pDefault, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(pTextLenText, pTextLen, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(pLengthText, pLength, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(pScaleText, pScale, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(pFormatText, pFormatSample, pFormat, nDeltaX, nDeltaY);
        ScrollAggregate(m_pColumnNameText, m_pColumnName, NULL, nDeltaX, nDeltaY);
        ScrollAggregate(m_pTypeText, m_pType, NULL, nDeltaX, nDeltaY);
    }
}

//------------------------------------------------------------------------------
sal_uInt16 OFieldDescControl::CountActiveAggregates() const
{
    Control* ppAggregates[] = { pRequired, pNumType, pAutoIncrement, pDefault, pTextLen, pLength, pScale, pFormat, m_pColumnName, m_pType};
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
    if (pDefault)
    {
        pDefaultText->Enable( !bReadOnly );
        pDefault->Enable( !bReadOnly );
    }
    if (pBoolDefault)
    {
        pBoolDefaultText->Enable( !bReadOnly );
        pBoolDefault->Enable( !bReadOnly );
    }
    if( pRequired )
    {
        pRequiredText->Enable( !bReadOnly );
        pRequired->Enable( !bReadOnly );
    }
    if( pTextLen )
    {
        pTextLenText->Enable( !bReadOnly );
        pTextLen->Enable( !bReadOnly );
    }
    if( pNumType )
    {
        pNumTypeText->Enable( !bReadOnly );
        pNumType->Enable( !bReadOnly );
    }
    if( pFormat )
    {
        pFormatText->Enable( !bReadOnly );
        pFormat->Enable( !bReadOnly );
    }
    if( pLength )
    {
        pLengthText->Enable( !bReadOnly );
        pLength->Enable( !bReadOnly );
    }
    if( pScale )
    {
        pScaleText->Enable( !bReadOnly );
        pScale->Enable( !bReadOnly );
    }
    if( pAutoIncrement )
    {
        pAutoIncrementText->Enable( !bReadOnly );
        pAutoIncrement->Enable( !bReadOnly );
    }

    if( m_pColumnName )
    {
        m_pColumnNameText->Enable( !bReadOnly );
        m_pColumnName->Enable( !bReadOnly );
    }
    if( m_pType )
    {
        m_pTypeText->Enable( !bReadOnly );
        m_pType->Enable( !bReadOnly );
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
                return String::CreateFromInt32(pTextLen->GetValue());
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

    }
}

//------------------------------------------------------------------------
IMPL_LINK( OFieldDescControl, FormatClickHdl, Button *, pButton )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Temporaere Column erzeugen, mit der Datenaustausch mit Dialog erfolgt
    if( !pActFieldDescr )
        return 0;

    sal_uInt32 nOldFormatKey(pActFieldDescr->GetFormatKey());
    SvxCellHorJustify rOldJustify = pActFieldDescr->GetHorJustify();

    // ------------
    // UNO->ItemSet
    static SfxItemInfo aItemInfos[] =
    {
        { 0, 0 },
        { SID_ATTR_NUMBERFORMAT_VALUE,      SFX_ITEM_POOLABLE },
        { SID_ATTR_ALIGN_HOR_JUSTIFY,       SFX_ITEM_POOLABLE },
        { SID_ATTR_NUMBERFORMAT_ONE_AREA,   SFX_ITEM_POOLABLE }
    };
    static sal_uInt16 aAttrMap[] =
    {
        SBA_DEF_RANGEFORMAT, SBA_ATTR_ALIGN_HOR_JUSTIFY,
        SID_ATTR_NUMBERFORMAT_ONE_AREA, SID_ATTR_NUMBERFORMAT_ONE_AREA,
        0
    };

    SfxPoolItem* pDefaults[] =
    {
        new SfxRangeItem(SBA_DEF_RANGEFORMAT, SBA_DEF_FMTVALUE, SBA_ATTR_ALIGN_HOR_JUSTIFY),
        new SfxUInt32Item(SBA_DEF_FMTVALUE),
        new SvxHorJustifyItem(SVX_HOR_JUSTIFY_STANDARD, SBA_ATTR_ALIGN_HOR_JUSTIFY),
        new SfxBoolItem(SID_ATTR_NUMBERFORMAT_ONE_AREA, sal_False)
    };

    SfxItemPool* pPool = new SfxItemPool(String::CreateFromAscii("GridBrowserProperties"), SBA_DEF_RANGEFORMAT, SBA_ATTR_ALIGN_HOR_JUSTIFY, aItemInfos, pDefaults);
    pPool->SetDefaultMetric( SFX_MAPUNIT_TWIP );    // ripped, don't understand why
    pPool->FreezeIdRanges();                        // the same

    SfxItemSet* pFormatDescriptor = new SfxItemSet(*pPool, aAttrMap);
    // fill it
    pFormatDescriptor->Put(SvxHorJustifyItem(rOldJustify, SBA_ATTR_ALIGN_HOR_JUSTIFY));
    pFormatDescriptor->Put(SfxUInt32Item(SBA_DEF_FMTVALUE, nOldFormatKey));

    {   // want the dialog to be destroyed before our set
        Reference< XNumberFormatsSupplier >  xSupplier = GetFormatter()->getNumberFormatsSupplier();

        Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
        SvNumberFormatsSupplierObj* pSupplierImpl = (SvNumberFormatsSupplierObj*)xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId());

        SvNumberFormatter* pFormatter = pSupplierImpl->GetNumberFormatter();

        SbaSbAttrDlg aDlg(this, pFormatDescriptor, pFormatter, TP_ATTR_ALIGN | TP_ATTR_NUMBER);
        if( aDlg.Execute() == RET_OK )
        {
            const SfxItemSet* pSet = aDlg.GetExampleSet();
                // GetExampleSet statt GetOutputItemSet, denn letzteres enthaelt nur die modifizierten Items

            sal_Bool bModified = sal_False;
            // den Formatkey
            SfxUInt32Item& rFormatItem = (SfxUInt32Item&)pSet->Get(SBA_DEF_FMTVALUE);
            sal_uInt32 nNewFormatKey(rFormatItem.GetValue());
            if (nNewFormatKey != nOldFormatKey)
            {
                SetModified(sal_True);
                bModified = sal_True;
            }

//          if (m_pFormatterShell->GetListPos4Entry(nNewFormatKey) == -1)
//              // Das neue Format ist meiner Formatter-Shell nicht bekannt, wurde also gerade neu angelegt
//              // (oder ist in einer anderen Sprache als die aktuell eingestellte gehalten)
//              // Das bedeutet, das ich die Shell updaten muss, sonst laeuft UpdateFormatSample in's Leere.
//              InitFormatterShell(nNewFormatKey);

            // die horizontale Ausrichtung
            SvxHorJustifyItem& rHorJustifyItem = (SvxHorJustifyItem&)pSet->Get(SBA_ATTR_ALIGN_HOR_JUSTIFY);
            SvxCellHorJustify rNewJustify = (SvxCellHorJustify)rHorJustifyItem.GetValue();
            if (rNewJustify != rOldJustify)
            {
                SetModified(sal_True);
                bModified = sal_True;
            }

            if (bModified)
                CellModified(-1, FIELD_PROPERTY_FORMAT);

            // das Setzen an der FieldDecription NACH den CellModified, da das ein Undo-::com::sun::star::chaos::Action erzeugt und die auf die Description
            // zurueckgreift
            pActFieldDescr->SetFormatKey( nNewFormatKey );
            pActFieldDescr->SetHorJustify(rNewJustify);

            UpdateFormatSample(pActFieldDescr);

            //  SFX_BINDINGS().Invalidate(SID_SAVEDOC);
        }
    }

    return 0;
}

//------------------------------------------------------------------------
IMPL_LINK( OFieldDescControl, ChangeHdl, ListBox *, pAutoInc )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    if(pAutoInc->GetSavedValue() == pAutoInc->GetSelectEntryPos() || !pActFieldDescr)
        return 0;

    SetModified(sal_True);

    // Sonderbehandlund f"ur Bool Felder
    if(pAutoInc == pRequired && pBoolDefault )
    {
        // wenn pRequired auf sal_True gesetzt ist, dann darf das sal_Bool Feld nicht den Eintrag <<keiner>> besitzen
        String sDef = BoolStringUI((pActFieldDescr->GetDefaultValue()));

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
    if (pAutoInc == pAutoIncrement)
    {
        if(pAutoInc->GetSelectEntryPos() == 1)
        { // no
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
        }
        // und jetzt alle nach oben schieben
        ArrangeAggregates();
    }

    if(pAutoInc == m_pType)
    {
        pAutoInc->SaveValue();
        const OTypeInfo* pTypeInfo = getTypeInfo(m_pType->GetSelectEntryPos());
        pActFieldDescr->SetType(pTypeInfo);

//      OFieldDescription *pFieldDescr = new OFieldDescription();
//      pFieldDescr->SetType(pTypeInfo);
//
//      pFieldDescr->SetName(pActFieldDescr->GetName());
//      pFieldDescr->SetFormatKey(pActFieldDescr->GetFormatKey());
//      pFieldDescr->SetPrimaryKey(pActFieldDescr->IsPrimaryKey());
//      pFieldDescr->SetDescription(pActFieldDescr->GetDescription());
//      pFieldDescr->SetAutoIncrement(pActFieldDescr->IsAutoIncrement());
//      pFieldDescr->SetHorJustify(pActFieldDescr->GetHorJustify());
//
//      //////////////////////////////////////////////////////////////////////
//      // Spezielle Daten
//      if(pTypeInfo->bNullable  && pActFieldDescr->getTypeInfo()->bNullable)
//          pFieldDescr->SetIsNullable(pActFieldDescr->IsNullable());
//
//      pFieldDescr->SetDefaultValue(pActFieldDescr->GetDefaultValue());
//      delete pActFieldDescr;
//      pActFieldDescr = pFieldDescr;
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
        int         nPosSizeArgument;   // das zweite Argument fuer SetPosSize
    };
    AGGREGATE_DESCRIPTION adAggregates[] = {
        { m_pColumnName, m_pColumnNameText, 1},
        { m_pType, m_pTypeText, 1},
        { pAutoIncrement, pAutoIncrementText, 1 },
        { pNumType, pNumTypeText, 1 },
        { pRequired, pRequiredText, 1 },
        { pTextLen, pTextLenText, 1 },
        { pLength, pLengthText, 1 },
        { pScale, pScaleText, 1 },
        { pDefault, pDefaultText, 3 },
        { pFormatSample, pFormatText, 4 },
        { pBoolDefault, pBoolDefaultText, 1 }
    };

    // und los ...
    int nCurrentControlPos = 0;
    Control* pZOrderPredecessor = NULL;
    for (int i=0; i<sizeof(adAggregates)/sizeof(adAggregates[0]); i++)
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
        nPos++;
        pDefaultText = new FixedText( this );
        pDefaultText->SetText( ModuleRes(STR_DEFAULT_VALUE) );
        pDefault = new OPropEditCtrl( this, STR_HELP_DEFAULT_VALUE, FIELD_PROPERTY_DEFAULT, WB_BORDER );
        pDefault->SetHelpId(HID_TAB_ENT_DEFAULT);
        SetPosSize( (Control**)&pDefaultText, nPos, 0 );
        SetPosSize( (Control**)&pDefault, nPos, 3 );

        pDefault->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pDefault->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pDefaultText->EnableClipSiblings();
        pDefault->EnableClipSiblings();
        break;

    case tpRequired:
    {
        if( pRequired )
            return;
        Reference< XDatabaseMetaData> xMetaData = getMetaData();

        if(xMetaData.is() && xMetaData->supportsNonNullableColumns())
        {
            nPos++;
            pRequiredText = new FixedText( this );
            pRequiredText->SetText( ModuleRes(STR_FIELD_REQUIRED) );
            pRequired = new OPropListBoxCtrl( this, STR_HELP_FIELD_REQUIRED, FIELD_PROPERTY_REQUIRED, WB_DROPDOWN);
            pRequired->SetHelpId(HID_TAB_ENT_REQUIRED);

            pRequired->InsertEntry( aYes );
            pRequired->InsertEntry( aNo );
            pRequired->SelectEntryPos(1);
            pRequired->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

            SetPosSize( (Control**)&pRequiredText, nPos, 0 );
            SetPosSize( (Control**)&pRequired, nPos, 2 );

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
        nPos++;
        pAutoIncrementText = new FixedText( this );
        pAutoIncrementText->SetText( ModuleRes(STR_FIELD_AUTOINCREMENT) );
        pAutoIncrement = new OPropListBoxCtrl( this, STR_HELP_AUTOINCREMENT, FIELD_PROPERTY_AUTOINC, WB_DROPDOWN );
        pAutoIncrement->SetHelpId(HID_TAB_ENT_AUTOINCREMENT);

        pAutoIncrement->InsertEntry( aYes );
        pAutoIncrement->InsertEntry( aNo );
        pAutoIncrement->SelectEntryPos(0);
        pAutoIncrement->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

        SetPosSize( (Control**)&pAutoIncrementText, nPos, 0 );
        SetPosSize( (Control**)&pAutoIncrement, nPos, 2 );

        pAutoIncrement->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pAutoIncrement->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pAutoIncrementText->EnableClipSiblings();
        pAutoIncrement->EnableClipSiblings();
    }
    break;
    case tpTextLen:
        if( pTextLen )
            return;
        nPos++;
        pTextLenText = new FixedText( this );
        pTextLenText->SetText( ModuleRes(STR_TEXT_LENGTH) );

        pTextLen = new OPropNumericEditCtrl( this, STR_HELP_TEXT_LENGTH, FIELD_PROPERTY_TEXTLEN, WB_BORDER );
        pTextLen->SetDecimalDigits(0);
        pTextLen->SetMin(0);
        pTextLen->SetMax(0x7FFFFFFF);   // soll draussen geaendert werden, wenn noetig

        pTextLen->SetHelpId(HID_TAB_ENT_TEXT_LEN);
        SetPosSize( (Control**)&pTextLenText, nPos, 0 );
        SetPosSize( (Control**)&pTextLen, nPos, 1 );

        pTextLen->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pTextLen->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pTextLenText->EnableClipSiblings();
        pTextLen->EnableClipSiblings();
        break;

    case tpType:
        if( m_pType)
            return;
        nPos++;
        m_pTypeText = new FixedText( this );
        m_pTypeText->SetText( ModuleRes(STR_TAB_FIELD_DATATYPE) );
        m_pType = new OPropListBoxCtrl( this, STR_HELP_AUTOINCREMENT, FIELD_PRPOERTY_TYPE, WB_DROPDOWN );
        m_pType->SetHelpId(HID_TAB_ENT_TYPE);
        m_pType->SetDropDownLineCount(5);
        {
            const OTypeInfoMap* pTypeInfo = getTypeInfo();
            OTypeInfoMap::const_iterator aIter = pTypeInfo->begin();
            for(;aIter != pTypeInfo->end();++aIter)
                m_pType->InsertEntry( aIter->second->aUIName );
        }
        m_pType->SelectEntryPos(0);
        m_pType->SetSelectHdl(LINK(this,OFieldDescControl,ChangeHdl));

        SetPosSize( (Control**)&m_pTypeText, nPos, 0 );
        SetPosSize( (Control**)&m_pType, nPos, 2 );

        m_pType->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        m_pType->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        m_pTypeText->EnableClipSiblings();
        m_pType->EnableClipSiblings();
        break;
    case tpColumnName:
        if( m_pColumnName )
            return;
        nPos++;
        {
            Reference< XDatabaseMetaData> xMetaData = getMetaData();
            sal_uInt32 nMax = xMetaData.is() ? xMetaData->getMaxColumnNameLength() : EDIT_NOLIMIT;
            m_pColumnNameText = new FixedText( this );
            m_pColumnNameText->SetText( ModuleRes(STR_TAB_FIELD_NAME) );
            m_pColumnName = new OPropColumnEditCtrl( this,
                            xMetaData.is() ? xMetaData->getExtraNameCharacters() : ::rtl::OUString(),
                                                    STR_HELP_DEFAULT_VALUE,
                                                    FIELD_PRPOERTY_COLUMNNAME,
                                                    WB_BORDER );
            m_pColumnName->SetHelpId(HID_TAB_ENT_COLUMNNAME);
            m_pColumnName->SetMaxTextLen(xub_StrLen( nMax ? nMax : EDIT_NOLIMIT));
        }

        SetPosSize( (Control**)&m_pColumnNameText, nPos, 0 );
        SetPosSize( (Control**)&m_pColumnName, nPos, 1 );

        m_pColumnName->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        m_pColumnName->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        m_pColumnNameText->EnableClipSiblings();
        m_pColumnName->EnableClipSiblings();
        break;
    case tpNumType:
        if( pNumType )
            return;
        nPos++;
        pNumTypeText = new FixedText( this );
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

        SetPosSize( (Control**)&pNumTypeText, nPos, 0 );
        SetPosSize( (Control**)&pNumType, nPos, 1 );

        pNumType->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pNumType->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pNumTypeText->EnableClipSiblings();
        pNumType->EnableClipSiblings();
        break;

    case tpLength:
        if( pLength )
            return;
        nPos++;
        pLengthText = new FixedText( this );
        pLengthText->SetText( ModuleRes(STR_LENGTH) );

        pLength = new OPropNumericEditCtrl( this, STR_HELP_LENGTH, FIELD_PROPERTY_LENGTH, WB_BORDER );
        pLength->SetDecimalDigits(0);
        pLength->SetMin(0);
        pLength->SetMax(0x7FFFFFFF);    // soll draussen geaendert werden, wenn noetig

        pLength->SetHelpId(HID_TAB_ENT_LEN);
        SetPosSize( (Control**)&pLengthText, nPos, 0 );
        SetPosSize( (Control**)&pLength, nPos, 1 );

        pLength->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pLength->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pLengthText->EnableClipSiblings();
        pLength->EnableClipSiblings();
        break;

    case tpScale:
        if( pScale )
            return;
        nPos++;
        pScaleText = new FixedText( this );
        pScaleText->SetText( ModuleRes(STR_SCALE) );
        pScale = new OPropNumericEditCtrl( this, STR_HELP_SCALE, FIELD_PROPERTY_SCALE, WB_BORDER );
        pScale->SetDecimalDigits(0);
        pScale->SetMin(0);
        pScale->SetMax(0x7FFFFFFF); // soll draussen geaendert werden, wenn noetig

        pScale->SetHelpId(HID_TAB_ENT_SCALE);

        SetPosSize( (Control**)&pScaleText, nPos, 0 );
        SetPosSize( (Control**)&pScale, nPos, 1 );

        pScale->SetGetFocusHdl(LINK(this, OFieldDescControl, OnControlFocusGot));
        pScale->SetLoseFocusHdl(LINK(this, OFieldDescControl, OnControlFocusLost));

        pScaleText->EnableClipSiblings();
        pScale->EnableClipSiblings();
        break;

    case tpFormat:
        if (!pFormat)
        {
            nPos++;
            pFormatText = new FixedText( this );
            pFormatText->SetText( ModuleRes(STR_FORMAT) );

            pFormatSample = new OPropEditCtrl( this, STR_HELP_FORMAT_CODE, -1, WB_BORDER );
            pFormatSample->SetHelpId(HID_TAB_ENT_FORMAT_SAMPLE);
            pFormatSample->SetReadOnly(sal_True);
            pFormatSample->Enable(sal_False);

            pFormat = new PushButton( this, ModuleRes(PB_FORMAT) );
            pFormat->SetHelpId(HID_TAB_ENT_FORMAT);
            pFormat->SetSizePixel(Size(CONTROL_HEIGHT, CONTROL_HEIGHT));
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

        nPos++;
        pBoolDefaultText = new FixedText(this);
        pBoolDefaultText->SetText(ModuleRes(STR_DEFAULT_VALUE));
        pBoolDefault = new OPropListBoxCtrl( this, STR_HELP_BOOL_DEFAULT, FIELD_PROPERTY_BOOL_DEFAULT, WB_DROPDOWN );
        pBoolDefault->SetDropDownLineCount(3);
        pBoolDefault->InsertEntry(String(ModuleRes(STR_VALUE_NONE)));
        pBoolDefault->InsertEntry(aYes);
        pBoolDefault->InsertEntry(aNo);
        pBoolDefault->SetHelpId(HID_TAB_ENT_BOOL_DEFAULT);
        SetPosSize( (Control**)&pBoolDefaultText, nPos, 0 );
        SetPosSize( (Control**)&pBoolDefault, nPos, 3 );

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
    //////////////////////////////////////////////////////////////////////
    // Controls zerstoeren
    switch( eType )
    {
    case tpDefault:
        if( !pDefault )
            return;
        nPos--;
        pDefault->Hide();
        pDefaultText->Hide();
        delete pDefault;
        delete pDefaultText;
        pDefault = NULL;
        pDefaultText = NULL;
        break;

    case tpColumnName:
        if( !m_pColumnName )
            return;
        nPos--;
        m_pColumnName->Hide();
        m_pColumnNameText->Hide();
        delete m_pColumnName;
        delete m_pColumnNameText;
        m_pColumnName = NULL;
        m_pColumnNameText = NULL;
        break;

    case tpType:
        if( !m_pType )
            return;
        nPos--;
        m_pType->Hide();
        m_pTypeText->Hide();
        delete m_pType;
        delete m_pTypeText;
        m_pType = NULL;
        m_pTypeText = NULL;
        break;

    case tpAutoIncrement:
        if( !pAutoIncrement )
            return;
        nPos--;
        pAutoIncrement->Hide();
        pAutoIncrementText->Hide();
        delete pAutoIncrement;
        delete pAutoIncrementText;
        pAutoIncrement = NULL;
        pAutoIncrementText = NULL;
        break;

    case tpRequired:
        if( !pRequired )
            return;
        nPos--;
        pRequired->Hide();
        pRequiredText->Hide();
        delete pRequired;
        delete pRequiredText;
        pRequired = NULL;
        pRequiredText = NULL;
        break;

    case tpTextLen:
        if( !pTextLen )
            return;
        nPos--;
        pTextLen->Hide();
        pTextLenText->Hide();
        delete pTextLen;
        delete pTextLenText;
        pTextLen = NULL;
        pTextLenText = NULL;
        break;

    case tpNumType:
        if( !pNumType )
            return;
        nPos--;
        pNumType->Hide();
        pNumTypeText->Hide();
        delete pNumType;
        delete pNumTypeText;
        pNumType = NULL;
        pNumTypeText = NULL;
        break;

    case tpLength:
        if( !pLength )
            return;
        nPos--;
        pLength->Hide();
        pLengthText->Hide();
        delete pLength;
        delete pLengthText;
        pLength = NULL;
        pLengthText = NULL;
        break;

    case tpScale:
        if( !pScale )
            return;
        nPos--;
        pScale->Hide();
        pScaleText->Hide();
        delete pScale;
        delete pScaleText;
        pScale = NULL;
        pScaleText = NULL;
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
        pFormatText = NULL;
        pFormatSample = NULL;
        pFormat = NULL;
        break;
    case tpBoolDefault:
        if (!pBoolDefault)
            return;
        nPos--;
        pBoolDefault->Hide();
        pBoolDefaultText->Hide();
        delete pBoolDefault;
        delete pBoolDefaultText;
        pBoolDefault = NULL;
        pBoolDefaultText = NULL;
        break;

    }
}

//------------------------------------------------------------------------------
void OFieldDescControl::SetPosSize( Control** ppControl, long nRow, sal_uInt16 nCol )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Position ermitteln
    Point aPosition;
    switch( nCol )
    {
    case 0:
        aPosition.X() = 0;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        aPosition.X() = CONTROL_WIDTH_1 + CONTROL_SPACING_X;
        break;
    default:
        aPosition.X() = 0;
    }

    aPosition.Y() = ((nRow+1)*CONTROL_SPACING_Y) +
                    (nRow*CONTROL_HEIGHT);

    //////////////////////////////////////////////////////////////////////
    // Groesse ermitteln
    Size aSize;
    switch( nCol )
    {
    case 0:
        aSize.Width()  = CONTROL_WIDTH_1;
        aSize.Height() = CONTROL_HEIGHT;
        break;
    case 1:
        aSize.Width()  = CONTROL_WIDTH_2;
        aSize.Height() = CONTROL_HEIGHT;
        break;
    case 2:
        aSize.Width()  = CONTROL_WIDTH_2;
        aSize.Height() = long(1.5*CONTROL_HEIGHT);
        break;
    case 3:
        aSize.Width()  = CONTROL_WIDTH_3;
        aSize.Height() = CONTROL_HEIGHT;
        break;
    case 4:
        aSize.Width()  = CONTROL_WIDTH_4;
        aSize.Height() = CONTROL_HEIGHT;
        break;
    default:
        aSize.Width()  = CONTROL_WIDTH_1;
        aSize.Height() = CONTROL_HEIGHT;
    }


    //////////////////////////////////////////////////////////////////////
    // Control anzeigen
    (*ppControl)->SetPosSizePixel( aPosition, aSize );
    (*ppControl)->Show();
}
//------------------------------------------------------------------------------
//void OFieldDescControl::DisplayData(const OColumn* pColumn)
//{
//  if(pActFieldDescr)
//      delete pActFieldDescr;
//
//  SFX_ITEMSET_GET(*pColumn, pName, ONameItem, SBA_DEF_FLTNAME, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pType, ODataFieldTypeItem, SBA_DEF_FLTTYPE, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pLength, SfxUInt32Item, SBA_DEF_FLTLENGTH, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pRequired, SfxBoolItem, SBA_DEF_FLTREQUIRED, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pPrimary, SfxBoolItem, SBA_DEF_FLTPRIMARY, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pComment, SfxStringItem, SBA_DEF_FLTCOMMENT, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pFormat, SfxUInt32Item, SBA_DEF_FMTVALUE, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pScale, SfxUInt16Item, SBA_DEF_FLTSCALE, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pDefault, SfxStringItem, SBA_DEF_FLTDEFAULT, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pAutoIncrement, SfxBoolItem, SBA_DEF_FLTAUTOINCREMENT, sal_True);
//  SFX_ITEMSET_GET(*pColumn, pHorJustify, SvxHorJustifyItem, SBA_ATTR_ALIGN_HOR_JUSTIFY, sal_True);
//
//  sal_Int32 aFieldType = pType->GetType();
//  OFieldDescription* pFieldDescr;
//  switch(aFieldType)
//  {
//      case dbNone:            // undefiniert: unbekannt
//          pFieldDescr = new OUnknownFieldDescr(); break;
//      case dbText:                // var. Text:
//          pFieldDescr = new OTextFieldDescr(); break;
//      case dbNumeric:         // Zahlen von Byte bis double:
//          pFieldDescr = new ONumericFieldDescr(); break;
//      case dbDateTime:            // Datumzeit:
//          pFieldDescr = new ODateTimeFieldDescr(); break;
//      case dbDate:                // Datum:
//          pFieldDescr = new ODateFieldDescr(); break;
//      case dbTime:                // Zeit:
//          pFieldDescr = new OTimeFieldDescr(); break;
//      case dbBool:                // Logical:
//          pFieldDescr = new OBoolFieldDescr(); break;
//      case dbCurrency:            // Währung:
//          pFieldDescr = new OCurrencyFieldDescr(); break;
//      case dbMemo:                // Langer Text:
//          pFieldDescr = new OMemoFieldDescr(); break;
//      case dbCounter:         // Zaehler:
//          pFieldDescr = new OCounterFieldDescr(); break;
//      case dbImage:           // Bilder:
//          pFieldDescr = new OImageFieldDescr(); break;
//      case dbChar:                // fixt. Text:
//          pFieldDescr = new OCharFieldDescr(); break;
//      case dbDecimal:         // decimal Werte: mit Vor und Nachkommastellen:
//          pFieldDescr = new ODecimalFieldDescr(); break;
//      case dbBinary:          // Binary Feld bis 255:
//          pFieldDescr = new OBinaryFieldDescr(); break;
//      case dbVarBinary:       // VarBinary Feld bis 255:
//          pFieldDescr = new OVarBinaryFieldDescr(); break;
//      case dbBigInt:          // Big integer:
//          pFieldDescr = new OBigIntFieldDescr(); break;
//
//  }
//
//
//  pFieldDescr->SetName(pName->GetValue());
//  pFieldDescr->SetFormatKey(pFormat->GetValue());
//  pFieldDescr->SetPrimaryKey(pPrimary->GetValue());
//  pFieldDescr->SetDescription(pComment->GetValue());
//  pFieldDescr->SetAutoIncrement(pAutoIncrement->GetValue());
//  pFieldDescr->SetHorJustify((SvxCellHorJustify)pHorJustify->GetValue());
//
//  //////////////////////////////////////////////////////////////////////
//  // Spezielle Daten
//  if( pFieldDescr->ISA(OExtFieldDescription) )
//  {
//      ((OExtFieldDescription*)pFieldDescr)->SetRequired(pRequired->GetValue());
//      ((OExtFieldDescription*)pFieldDescr)->SetDefaultValue(pDefault->GetValue());
//  }
//
//  switch( pFieldDescr->GetFieldType() )
//  {
//  case dbText:
//      ((OTextFieldDescr*)pFieldDescr )->SetTextLen( (xub_StrLen)(pLength->GetValue()) );
//      break;
//
//  case dbNumeric:
//      ( (ONumericFieldDescr*)pFieldDescr )->SetNumType(GetDataDef()->SizeString((ONumericSize)pLength->GetValue()) );
//      break;
//
//  case dbChar:
//      ((OCharFieldDescr*)pFieldDescr )->SetTextLen( (xub_StrLen)(pLength->GetValue()) );
//      break;
//
//  case dbDecimal:
//      {
//          ODecimalFieldDescr* pDecFieldDescr = (ODecimalFieldDescr*)pFieldDescr;
//          pDecFieldDescr->SetLength( (xub_StrLen)(pLength->GetValue()) );
//          pDecFieldDescr->SetScale( String::CreateFromInt32(pScale->GetValue()) );
//      }
//      break;
//  case dbBinary:
//      ((OBinaryFieldDescr*)pFieldDescr )->SetLength( (xub_StrLen)(pLength->GetValue()) );
//      break;
//
//  case dbVarBinary:
//      ((OVarBinaryFieldDescr*)pFieldDescr )->SetLength( (xub_StrLen)(pLength->GetValue()) );
//      break;
//  }
//
//  DisplayData(pFieldDescr);
//}
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
        m_pPreviousType = NULL;
        return;
    }

    const OTypeInfo* pFieldType = NULL;
    if( pFieldDescr )
        pFieldType = pFieldDescr->getTypeInfo();

    ActivateAggregate( tpColumnName );
    ActivateAggregate( tpType );

    OSL_ENSURE(pFieldType,"We need a type information here!");
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
        if(pFieldType->bNullable)
            ActivateAggregate( tpRequired );
        else
            DeactivateAggregate( tpRequired );

        // 2. the autoincrement
        if(pFieldType->bAutoIncrement)
        {
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
            ActivateAggregate( tpAutoIncrement );
        }
        else
        {
            DeactivateAggregate( tpAutoIncrement );
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
            pLength->SetMax(pFieldType->nPrecision);
            pLength->SetSpecialReadOnly(pFieldType->aCreateParams.getLength()==0);
        }
        else
            DeactivateAggregate( tpLength );

        if (pFieldType->nMaximumScale)
        {
            ActivateAggregate( tpScale );
            pScale->SetMax(pFieldType->nMaximumScale);
            pScale->SetMin(pFieldType->nMinimumScale);
            pScale->SetSpecialReadOnly(pFieldType->aCreateParams.getLength()==0);
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
                    pTextLen->SetMax(pFieldType->nPrecision);
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
        else if(!pAutoIncrement && pFieldType)
        {
            if(pFieldType->bNullable)
                ActivateAggregate( tpRequired );
            else
                DeactivateAggregate( tpRequired );
        }
    }
    //////////////////////////////////////////////////////////////////////
    // Controls initialisieren
    if( pAutoIncrement )
    {
        if(pFieldDescr->IsAutoIncrement())
        {
            pAutoIncrement->SelectEntryPos( 0 ); // yes
            DeactivateAggregate( tpRequired );
            DeactivateAggregate( tpDefault );
        }
        else
        {
            // hat Auswirkungen auf pRequired
            pAutoIncrement->SelectEntryPos( 1 );        // no
            ActivateAggregate( tpDefault );
            if(!pFieldDescr->IsPrimaryKey())
                ActivateAggregate( tpRequired );
        }
    }

    if( pDefault )
    {
        pDefault->SetText( pFieldDescr->GetDefaultValue() );
        pDefault->ClearModifyFlag();
    }

    if( pBoolDefault )
    {
        // wenn pRequired auf sal_True gesetzt ist, dann darf das sal_Bool Feld nicht den Eintrag <<keiner>> besitzen
        String sDef = BoolStringUI(pFieldDescr->GetDefaultValue());

        // sicher stellen das <<keiner>> nur vorhanden ist, wenn das Feld NULL sein darf
        if(pFieldType && !pFieldType->bNullable || !pFieldDescr->IsNullable() )
        {
            pFieldDescr->SetIsNullable(ColumnValue::NO_NULLS); // der Typ sagt das

            pBoolDefault->RemoveEntry(String(ModuleRes(STR_VALUE_NONE)));
            if (!sDef.Equals(aYes) && !sDef.Equals(aNo))
                pBoolDefault->SelectEntryPos(1);  // nein als Default
            else
                pBoolDefault->SelectEntry(sDef);

            pFieldDescr->SetDefaultValue(BoolStringPersistent(pBoolDefault->GetSelectEntry()));
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
        OSL_ASSERT(0);
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
        USHORT nPos = m_pType->GetEntryPos(String(pFieldDescr->getTypeInfo()->aUIName));
        if(nPos == LISTBOX_ENTRY_NOTFOUND)
        {
            const OTypeInfoMap* pMap = getTypeInfo();
            OTypeInfoMap::const_iterator aIter = pMap->find(pFieldDescr->getTypeInfo()->nType);
            if(aIter == pMap->end())
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
            pFieldDescr->SetType(aIter->second);
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
    if ((pControl == pLength) || (pControl == pScale) || (pControl == pTextLen))
    {
        ((OPropNumericEditCtrl*)pControl)->SaveValue();
        strHelpText = ((OPropNumericEditCtrl*)pControl)->GetHelp();
    }
    if ((pControl == pDefault) || (pControl == pFormatSample) || (pControl == m_pColumnName))
    {
        ((OPropEditCtrl*)pControl)->SaveValue();
        strHelpText = ((OPropEditCtrl*)pControl)->GetHelp();
    }
    else if ((pControl == pRequired) || (pControl == pNumType) || (pControl == pAutoIncrement) || (pControl == pBoolDefault) || (pControl == m_pType))
    {
        ((OPropListBoxCtrl*)pControl)->SaveValue();
        strHelpText = ((OPropListBoxCtrl*)pControl)->GetHelp();
    }
    else if (pControl == pFormat)
        strHelpText = String(ModuleRes(STR_HELP_FORMAT_BUTTON));

    if (strHelpText.Len() && (pHelp != NULL))
        pHelp->SetHelpText(strHelpText);

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
    if ((pControl == pDefault) || (pControl == pFormatSample) || (pControl == m_pColumnName))
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
void OFieldDescControl::ActivatePropertyField(sal_uInt16 nVirtualField)
{
    Control** ppToActivate = NULL;
    switch (nVirtualField)
    {
        case FIELD_PROPERTY_REQUIRED    : ppToActivate = (Control**)&pRequired; break;
        case FIELD_PROPERTY_NUMTYPE     : ppToActivate = (Control**)&pNumType; break;
        case FIELD_PROPERTY_AUTOINC     : ppToActivate = (Control**)&pAutoIncrement; break;
        case FIELD_PROPERTY_DEFAULT     : ppToActivate = (Control**)&pDefault; if (!*ppToActivate) ppToActivate = (Control**)&pBoolDefault; break;
                                                // da es immer nur eines der beiden Controls gibt, ist das hier eindeutig
        case FIELD_PROPERTY_TEXTLEN     : ppToActivate = (Control**)&pTextLen; break;
        case FIELD_PROPERTY_LENGTH      : ppToActivate = (Control**)&pLength; break;
        case FIELD_PROPERTY_SCALE       : ppToActivate = (Control**)&pScale; break;
        case FIELD_PROPERTY_FORMAT      : ppToActivate = (Control**)&pFormatSample; break;
        case FIELD_PRPOERTY_COLUMNNAME  : ppToActivate = (Control**)&m_pColumnName; break;
        case FIELD_PRPOERTY_TYPE        : ppToActivate = (Control**)&m_pType; break;

        default:
            DBG_ERROR("OFieldDescControl::ActivatePropertyField : ungueltiger Parameter !");
    }

    if (*ppToActivate)
        nDelayedGrabFocusEvent = Application::PostUserEvent(LINK(this, OFieldDescControl, DelayedGrabFocus), ppToActivate);
}
//------------------------------------------------------------------------------
void OFieldDescControl::SaveData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    if( !pFieldDescr )
        return;

    //////////////////////////////////////////////////////////////////////
    // Controls auslesen
    String sDefault;
    if (pDefault)
        sDefault = pDefault->GetText();
    else if (pBoolDefault)
        sDefault = BoolStringPersistent(pBoolDefault->GetSelectEntry());

    pFieldDescr->SetDefaultValue(sDefault);
    if((pRequired && pRequired->GetSelectEntryPos() == 0) || pFieldDescr->IsPrimaryKey() || (pBoolDefault && pBoolDefault->GetEntryCount() == 2))  // yes
        pFieldDescr->SetIsNullable( ColumnValue::NO_NULLS );
    else
        pFieldDescr->SetIsNullable( ColumnValue::NULLABLE );

    if(pAutoIncrement && pAutoIncrement->GetSelectEntryPos() == 0 )
        pFieldDescr->SetAutoIncrement( sal_True );
    else
        pFieldDescr->SetAutoIncrement( sal_False );

    if( pTextLen )
        pFieldDescr->SetPrecision( pTextLen->GetValue() );
    else if( pLength )
        pFieldDescr->SetPrecision( pLength->GetValue() );
    if( pScale )
        pFieldDescr->SetScale( pScale->GetValue() );

    if(m_pColumnName)
        pFieldDescr->SetName(m_pColumnName->GetText());
}

//------------------------------------------------------------------------------
void OFieldDescControl::UpdateFormatSample(OFieldDescription* pFieldDescr)
{
    if(!pFieldDescr)
        return;
    if(!pFormatSample)
        return;
    sal_uInt32 nFormatKey = pFieldDescr->GetFormatKey();

    try
    {
        Reference< XNumberFormatTypes> xNumberTypes(GetFormatter()->getNumberFormatsSupplier()->getNumberFormats(),UNO_QUERY);
        OSL_ENSURE(xNumberTypes.is(),"XNumberFormatTypes is null!");


        if (!nFormatKey)
        {
            nFormatKey = ::dbtools::getDefaultNumberFormat( pFieldDescr->GetType(),
                pFieldDescr->GetScale(),
                pFieldDescr->IsCurrency(),
                xNumberTypes,
                GetLocale());
        }

        String sDefault = pFieldDescr->GetDefaultValue();
        sal_Int32 nNumberFormat = ::comphelper::getNumberFormatType(GetFormatter(),nFormatKey);
        if(nNumberFormat == NumberFormat::TEXT)
            pFormatSample->SetText(sDefault);
        else if(sDefault.Len())
        {
            Reference<XNumberFormatPreviewer> xPreViewer(GetFormatter(),UNO_QUERY);
            OSL_ENSURE(xPreViewer.is(),"XNumberFormatPreviewer is null!");

            double nValue = GetFormatter()->convertStringToNumber(nFormatKey,sDefault);
            Reference<XPropertySet> xFormSet = GetFormatter()->getNumberFormatsSupplier()->getNumberFormats()->getByKey(nFormatKey);
            OSL_ENSURE(xFormSet.is(),"XPropertySet is null!");

            ::rtl::OUString sFormat;
            xFormSet->getPropertyValue(::rtl::OUString::createFromAscii("FormatString")) >>= sFormat;

            sDefault = xPreViewer->convertNumberToPreviewString(sFormat,nValue,GetLocale(),sal_True);
        }
        pFormatSample->SetText(sDefault);
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Format Exception!");
    }
}

//------------------------------------------------------------------------------
sal_Bool OFieldDescControl::ChildHasFocus()
{
    DBG_CHKTHIS(OFieldDescControl,NULL);
    //////////////////////////////////////////////////////////////////////
    // Hat eines der Childs den Focus?
    Window* pChild;
    for( sal_uInt16 nChildId=0; nChildId<GetChildCount(); nChildId++ )
    {
        pChild = GetChild( nChildId );
        if( pChild->HasFocus() )
            return sal_True;
    }

    return sal_False;
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






