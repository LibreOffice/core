/*************************************************************************
 *
 *  $RCSfile: numpages.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-28 18:40:55 $
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

#pragma hdrstop

#include <numpages.hxx>
#include <numpages.hrc>
#ifndef _SVX_DIALMGR_HXX //autogen
#include <dialmgr.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <dialogs.hrc>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _SVX_HELPID_HRC
#include <helpid.hrc>
#endif
#ifndef _SVX_NUMITEM_HXX
#include <numitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _GALLERY_HXX_ //autogen
#include <gallery.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#define ITEMID_BRUSH SID_ATTR_BRUSH
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <brshitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <impgrf.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_CHARMAP_HXX //autogen
#include <charmap.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <dlgutil.hxx>
#endif
#ifndef _XTABLE_HXX //autogen
#define ITEMID_COLOR_TABLE SID_COLOR_TABLE
#include <xtable.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <drawitem.hxx>
#endif
#ifndef _SVX_NUMVSET_HXX
#include <numvset.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <htmlmode.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#include <string>
#include <algorithm>

/*-----------------07.02.97 15.37-------------------

--------------------------------------------------*/
#define NUM_PAGETYPE_BULLET         0
#define NUM_PAGETYPE_SINGLENUM      1
#define NUM_PAGETYPE_NUM            2
#define NUM_PAGETYPE_BMP            3
#define PAGETYPE_USER_START         10

#define SHOW_NUMBERING              0
#define SHOW_BULLET                 1
#define SHOW_BITMAP                 2

#define MAX_BMP_WIDTH               16
#define MAX_BMP_HEIGHT              16

static BOOL bLastRelative =         FALSE;

/* -----------------27.10.98 15:40-------------------
 *
 * --------------------------------------------------*/
static const sal_Char aSglPostPreFixes[] =
{
    ' ', ')',
    ' ', '.',
    '(', ')',
    ' ', '.',
    ' ', ')',
    ' ', ')',
    '(', ')',
    ' ', '.'
};
/* -----------------27.10.98 15:42-------------------
 *
 * --------------------------------------------------*/
static const SvxExtNumType aSglNumTypes[] =
{
    SVX_NUM_ARABIC,                 // 1),2),3)...
    SVX_NUM_ARABIC,                 // 1.,2.,3. ...
    SVX_NUM_ARABIC,                 // (1),(2),(3)...
    SVX_NUM_ROMAN_UPPER,            // I.,II. ...
    SVX_NUM_CHARS_UPPER_LETTER,     // A),B)...
    SVX_NUM_CHARS_LOWER_LETTER,     // a),b),c) ...
    SVX_NUM_CHARS_LOWER_LETTER,     // (a),(b),(c),...
    SVX_NUM_ROMAN_LOWER             // i.,ii.,...
};
/* -----------------28.10.98 08:32-------------------
 *
 * --------------------------------------------------*/
// Die Auswahl an Bullets aus den StarBats
static const sal_Unicode aBulletTypes[] =
{
    149 + 0xf000,
    34 + 0xf000,
    65 + 0xf000,
    61 + 0xf000,
    49 + 0xf000,
    47 + 0xf000,
    79 + 0xf000,
    58 + 0xf000
};
/* -----------------28.10.98 09:42-------------------
 *
 * --------------------------------------------------*/
static sal_Char __READONLY_DATA aNumChar[] =
{
    'A', //CHARS_UPPER_LETTER
    'a', //CHARS_LOWER_LETTER
    'I', //ROMAN_UPPER
    'i', //ROMAN_LOWER
    '1', //ARABIC
    ' '
};

/*-----------------18.03.98 08:35-------------------
    Ist eins der maskierten Formate gesetzt?
--------------------------------------------------*/
BOOL lcl_IsNumFmtSet(SvxNumRule* pNum, USHORT nLevelMask)
{
    BOOL bRet = FALSE;
    USHORT nMask = 1;
    for( USHORT i = 0; i < SVX_MAX_NUM && !bRet; i++ )
    {
        if(nLevelMask & nMask)
            bRet |= 0 != pNum->Get( i );
        nMask <<= 1 ;
    }
    return bRet;
}
/* -----------------28.10.98 08:50-------------------
 *
 * --------------------------------------------------*/
#if defined UNX
#define BULLET_FONT_NAME "starbats"
#else
#define BULLET_FONT_NAME  "StarBats"
#endif

Font& lcl_GetDefaultBulletFont()
{
    static BOOL bInit = 0;
    static Font aDefBulletFont( UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( BULLET_FONT_NAME ) ),
                                String(), Size( 0, 14 ) );
    if(!bInit)
    {
        aDefBulletFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );
        aDefBulletFont.SetFamily( FAMILY_DONTKNOW );
        aDefBulletFont.SetPitch( PITCH_DONTKNOW );
        aDefBulletFont.SetWeight( WEIGHT_DONTKNOW );
        aDefBulletFont.SetTransparent( TRUE );
        bInit = TRUE;
    }
    return aDefBulletFont;
}


/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/


SvxSingleNumPickTabPage::SvxSingleNumPickTabPage(Window* pParent,
                               const SfxItemSet& rSet)  :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_PICK_SINGLE_NUM ), rSet ),
    aValuesGB(      this, ResId(BG_VALUES) ),
    pExamplesVS(    new SvxNumValueSet(this, ResId(VS_VALUES), NUM_PAGETYPE_SINGLENUM )),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    bModified(FALSE),
    bPreset(FALSE),
    bHasChild(TRUE),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    FreeResource();
    SetExchangeSupport();
    pExamplesVS->SetSelectHdl(LINK(this, SvxSingleNumPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxSingleNumPickTabPage, DoubleClickHdl_Impl));
    pExamplesVS->SetHelpId(HID_VALUESET_SINGLENUM );

}
/*-----------------07.02.97 12.08-------------------

--------------------------------------------------*/

 SvxSingleNumPickTabPage::~SvxSingleNumPickTabPage()
{
    delete pActNum;
    delete pExamplesVS;
    delete pSaveNum;
}

/*-----------------07.02.97 12.13-------------------

--------------------------------------------------*/

SfxTabPage*  SvxSingleNumPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxSingleNumPickTabPage(pParent, rAttrSet);
}

/*-----------------07.02.97 12.09-------------------

--------------------------------------------------*/


BOOL  SvxSingleNumPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(bPreset || bModified && pSaveNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }

    return bModified;
}

/*-----------------08.02.97 16.27-------------------

--------------------------------------------------*/

void  SvxSingleNumPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = FALSE;
    BOOL bIsPreset = FALSE;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, FALSE, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, FALSE, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, FALSE, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }
    // ersten Eintrag vorselektieren
    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = TRUE;
    }
    bPreset |= bIsPreset;

    bModified = FALSE;
}

/*-----------------08.02.97 11.28-------------------

--------------------------------------------------*/

int  SvxSingleNumPickTabPage::DeactivatePage(SfxItemSet *pSet)
{
    if(pSet)
        FillItemSet(*pSet);
    return TRUE;
}

/*-----------------07.02.97 12.09-------------------

--------------------------------------------------*/


void  SvxSingleNumPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
//  nActNumLvl = ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();
    //im Draw gibt es das Item als WhichId, im Writer nur als SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, FALSE, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, FALSE, &pItem);
    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!")
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    if(SFX_ITEM_SET == rSet.GetItemState(SID_PARAM_CHILD_LEVELS, FALSE, &pItem))
        bHasChild = ((const SfxBoolItem*)pItem)->GetValue();
    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
}
/*-----------------08.02.97 11.40-------------------

--------------------------------------------------*/

IMPL_LINK(SvxSingleNumPickTabPage, NumSelectHdl_Impl, ValueSet*, EMPTYARG)
{
    if(pActNum)
    {
        bPreset = FALSE;
        bModified = TRUE;
        USHORT nIdx = pExamplesVS->GetSelectItemId() - 1;
        SvxExtNumType eNewType = aSglNumTypes[nIdx];
        sal_Char cPrefix    = aSglPostPreFixes[2 * nIdx];
        sal_Char cPostfix   = aSglPostPreFixes[2 * (nIdx) + 1];

        USHORT nMask = 1;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aFmt(pActNum->GetLevel(i));
                aFmt.SetNumType(eNewType);
                String aEmptyStr;
                if(cPrefix == ' ')
                    aFmt.SetPrefix( aEmptyStr );
                else
                    aFmt.SetPrefix( String( sal_Unicode( cPrefix ) ) );
                if(cPostfix == ' ')
                    aFmt.SetSuffix( aEmptyStr );
                else
                    aFmt.SetSuffix( String( sal_Unicode( cPostfix ) ) );
                aFmt.SetCharFmt(sNumCharFmtName);
                pActNum->SetLevel(i, aFmt);
            }
            nMask <<= 1 ;
        }
    }
    return 0;
}

/*-----------------06.06.97 11.15-------------------

--------------------------------------------------*/
IMPL_LINK(SvxSingleNumPickTabPage, DoubleClickHdl_Impl, ValueSet*, EMPTYARG)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/


SvxBulletPickTabPage::SvxBulletPickTabPage(Window* pParent,
                               const SfxItemSet& rSet)  :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_PICK_BULLET ), rSet ),
    aValuesGB(      this, ResId(BG_VALUES) ),
    pExamplesVS(    new SvxNumValueSet(this, ResId(VS_VALUES), NUM_PAGETYPE_BULLET )),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    bModified(FALSE),
    bPreset(FALSE),
    bHasChild(/*FALSE*/TRUE),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    FreeResource();
    SetExchangeSupport();
    pExamplesVS->SetSelectHdl(LINK(this, SvxBulletPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxBulletPickTabPage, DoubleClickHdl_Impl));
    pExamplesVS->SetHelpId(HID_VALUESET_BULLET    );

}
/*-----------------07.02.97 12.10-------------------

--------------------------------------------------*/


 SvxBulletPickTabPage::~SvxBulletPickTabPage()
{
    delete pActNum;
    delete pExamplesVS;
    delete pSaveNum;
}
/*-----------------07.02.97 12.10-------------------

--------------------------------------------------*/


SfxTabPage*  SvxBulletPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxBulletPickTabPage(pParent, rAttrSet);
}

/*-----------------07.02.97 12.10-------------------

--------------------------------------------------*/


BOOL  SvxBulletPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(bPreset || bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }
    return bModified;
}
/*-----------------08.02.97 16.28-------------------

--------------------------------------------------*/

void  SvxBulletPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = FALSE;
    BOOL bIsPreset = FALSE;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, FALSE, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, FALSE, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, FALSE, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }
    // ersten Eintrag vorselektieren
    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = TRUE;
    }
    bPreset |= bIsPreset;
    bModified = FALSE;
}
/*-----------------08.02.97 11.28-------------------

--------------------------------------------------*/

int  SvxBulletPickTabPage::DeactivatePage(SfxItemSet *pSet)
{
    if(pSet)
        FillItemSet(*pSet);
    return TRUE;
}

/*-----------------07.02.97 12.11-------------------

--------------------------------------------------*/


void  SvxBulletPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    //im Draw gibt es das Item als WhichId, im Writer nur als SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, FALSE, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, FALSE, &pItem);
    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!")
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

//  nActNumLvl = ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();

    if(SFX_ITEM_SET == rSet.GetItemState(SID_PARAM_CHILD_LEVELS, FALSE, &pItem))
        bHasChild = ((const SfxBoolItem*)pItem)->GetValue();
    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
}
/*-----------------08.02.97 11.58-------------------

--------------------------------------------------*/

IMPL_LINK(SvxBulletPickTabPage, NumSelectHdl_Impl, ValueSet*, EMPTYARG)
{
    if(pActNum)
    {
        bPreset = FALSE;
        bModified = TRUE;
        sal_Unicode cChar = aBulletTypes[pExamplesVS->GetSelectItemId() - 1];
        Font& rActBulletFont = lcl_GetDefaultBulletFont();

        USHORT nMask = 1;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aFmt(pActNum->GetLevel(i));
                aFmt.SetNumType( SVX_NUM_CHAR_SPECIAL );
                aFmt.SetBulletFont(&rActBulletFont);
                aFmt.SetBulletChar(cChar );
                aFmt.SetCharFmt(sBulletCharFmtName);
                // #62069#
                aFmt.SetBulletRelSize(75);
                pActNum->SetLevel(i, aFmt);
            }
            nMask <<= 1;
        }
    }

    return 0;
}

/*-----------------06.06.97 11.16-------------------

--------------------------------------------------*/
IMPL_LINK(SvxBulletPickTabPage, DoubleClickHdl_Impl, ValueSet*, EMPTYARG)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/


SvxNumPickTabPage::SvxNumPickTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_PICK_NUM ), rSet ),
    aValuesGB(      this, ResId(BG_VALUES) ),
    pExamplesVS(    new SvxNumValueSet(this, ResId(VS_VALUES), NUM_PAGETYPE_NUM )),
    aNumFmt1(ResId(STR_NUMFMT_1)),
    aNumFmt2(ResId(STR_NUMFMT_2)),
    aNumFmt3(ResId(STR_NUMFMT_3)),
    aNumFmt4(ResId(STR_NUMFMT_4)),
    aNumFmt5(ResId(STR_NUMFMT_5)),
    aNumFmt6(ResId(STR_NUMFMT_6)),
    aNumFmt7(ResId(STR_NUMFMT_7)),
    aNumFmt8(ResId(STR_NUMFMT_8)),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    bModified(FALSE),
    bPreset(FALSE),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    aNumArr[0] = &aNumFmt1;
    aNumArr[1] = &aNumFmt2;
    aNumArr[2] = &aNumFmt3;
    aNumArr[3] = &aNumFmt4;
    aNumArr[4] = &aNumFmt5;
    aNumArr[5] = &aNumFmt6;
    aNumArr[6] = &aNumFmt7;
    aNumArr[7] = &aNumFmt8;

#ifdef DBG_UTIL
    for(USHORT dbg = 0; dbg < NUM_VALUSET_COUNT; dbg++)
        DBG_ASSERT((NUMTYPE_MEMBER * 5) == aNumArr[dbg]->GetTokenCount('|'),
                        "FormatString inkonsistent")
#endif

    SfxObjectShell* pShell;
    const SfxPoolItem* pItem;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, FALSE, &pItem )
         || ( 0 != ( pShell = SfxObjectShell::Current()) &&
              0 != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        USHORT nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        BOOL bHTMLMode = 0 != (nHtmlMode&HTMLMODE_ON);
        if(bHTMLMode)
        {
            aNumFmt7 = String(ResId(STR_NUMFMT_7_HTML));
            pExamplesVS->SetHTMLMode(TRUE);
        }
    }
    FreeResource();

    SetExchangeSupport();

    pExamplesVS->SetSelectHdl(LINK(this, SvxNumPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxNumPickTabPage, DoubleClickHdl_Impl));
    pExamplesVS->SetHelpId(HID_VALUESET_NUM       );

}
/*-----------------07.02.97 12.12-------------------

--------------------------------------------------*/


 SvxNumPickTabPage::~SvxNumPickTabPage()
{
    delete pActNum;
    delete pExamplesVS;
    delete pSaveNum;
}

/*-----------------07.02.97 12.12-------------------

--------------------------------------------------*/


SfxTabPage*  SvxNumPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxNumPickTabPage(pParent, rAttrSet);
}

/*-----------------07.02.97 12.12-------------------

--------------------------------------------------*/


BOOL  SvxNumPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(bPreset || bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }
    return bModified;
}
/*-----------------08.02.97 16.28-------------------

--------------------------------------------------*/

void  SvxNumPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = FALSE;
    BOOL bIsPreset = FALSE;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, FALSE, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, FALSE, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, FALSE, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }
    // ersten Eintrag vorselektieren
    if(pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = TRUE;
    }
    bPreset |= bIsPreset;
    bModified = FALSE;
}

/* -----------------08.02.97 11.29-------------------

--------------------------------------------------*/

int  SvxNumPickTabPage::DeactivatePage(SfxItemSet *pSet)
{
    if(pSet)
        FillItemSet(*pSet);
    return TRUE;
}

/*-----------------07.02.97 12.12-------------------

--------------------------------------------------*/

void  SvxNumPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    //im Draw gibt es das Item als WhichId, im Writer nur als SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, FALSE, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, FALSE, &pItem);
    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!")
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

//  nActNumLvl = ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();
    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;

}

/*-----------------08.02.97 11.58-------------------
    Hier werden alle Ebenen veraendert,
--------------------------------------------------*/

IMPL_LINK(SvxNumPickTabPage, NumSelectHdl_Impl, ValueSet*, EMPTYARG)
{
    if(pActNum)
    {
        bPreset = FALSE;
        bModified = TRUE;

        String* pNum = aNumArr[pExamplesVS->GetSelectItemId() - 1];
        Font& rActBulletFont = lcl_GetDefaultBulletFont();
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            SvxNumberFormat aFmt(pActNum->GetLevel(i));
            // Definitionen reichen nur bis fuenf, danach wird die Art beibehalten
            USHORT nTokenPos = i > 4 ? 4 : i;
            aFmt.SetNumType( (SvxExtNumType)(USHORT) pNum->GetToken(nTokenPos * NUMTYPE_MEMBER, '|').ToInt32() );
            USHORT nUpperLevelOrChar = pNum->GetToken(nTokenPos * NUMTYPE_MEMBER + 3, '|').ToInt32();
            if(aFmt.GetNumType() == SVX_NUM_CHAR_SPECIAL)
            {
                aFmt.SetBulletFont(&rActBulletFont);
                aFmt.SetBulletChar(nUpperLevelOrChar);
                aFmt.SetCharFmt(sBulletCharFmtName);
            }
            else
            {
                aFmt.SetIncludeUpperLevels(1 == nUpperLevelOrChar ? pActNum->GetLevelCount() : 0);
                aFmt.SetCharFmt(sNumCharFmtName);
            }
            aFmt.SetPrefix(pNum->GetToken(nTokenPos * NUMTYPE_MEMBER + 1, '|'));
            aFmt.SetSuffix(pNum->GetToken(nTokenPos * NUMTYPE_MEMBER + 2, '|'));
            pActNum->SetLevel(i, aFmt);
        }
    }

    return 0;
}

/*-----------------06.06.97 11.16-------------------

--------------------------------------------------*/
IMPL_LINK(SvxNumPickTabPage, DoubleClickHdl_Impl, ValueSet*, EMPTYARG)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}

/*-----------------07.02.97 15.59-------------------

--------------------------------------------------*/

void  SvxNumValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    static USHORT __READONLY_DATA aLinesArr[] =
    {
        15, 10,
        20, 30,
        25, 50,
        30, 70,
        35, 90, // bis hierher nur Linien, danach die char-Positionen
        10, 10,
        15, 30,
        20, 50,
        25, 70,
        30, 90,
    };

    OutputDevice*  pDev = rUDEvt.GetDevice();
    Rectangle aRect = rUDEvt.GetRect();
    USHORT  nItemId = rUDEvt.GetItemId();
    long nRectWidth = aRect.GetWidth();
    long nRectHeight = aRect.GetHeight();
    Size aRectSize(nRectWidth, aRect.GetHeight());
    Point aBLPos = aRect.TopLeft();
    Font aOldFont = pDev->GetFont();
    Color aOldColor = pDev->GetLineColor();
    pDev->SetLineColor(aLineColor);
    Font aFont(System::GetStandardFont(STDFONT_ROMAN));
    Size aSize = aFont.GetSize();

    Font aRuleFont( lcl_GetDefaultBulletFont() );
    aSize.Height() = nRectHeight/6;
    aRuleFont.SetSize(aSize);
    if(nPageType == NUM_PAGETYPE_BULLET)
        aFont = aRuleFont;
    else if(nPageType == NUM_PAGETYPE_NUM)
        aSize.Height() = nRectHeight/8;
    aFont.SetSize( aSize );
    pDev->SetFont(aFont);

    if(!pVDev)
    {
        // Die Linien werden nur einmalig in das VirtualDevice gepainted
        // nur die Gliederungspage bekommt es aktuell
        pVDev = new VirtualDevice(*pDev);
        pVDev->SetMapMode(pDev->GetMapMode());
        pVDev->SetOutputSize( aRectSize );
        aOrgRect = aRect;

        pVDev->SetLineColor(aLineColor);
        // Linien nur einmalig Zeichnen
        if(nPageType != NUM_PAGETYPE_NUM)
        {
            Point aStart(aBLPos.X() + nRectWidth *25 / 100,0);
            Point aEnd(aBLPos.X() + nRectWidth * 9 / 10,0);
            for( USHORT i = 11; i < 100; i += 33)
            {
                aStart.Y() = aEnd.Y() = aBLPos.Y() + nRectHeight  * i / 100;
                pVDev->DrawLine(aStart, aEnd);
                aStart.Y() = aEnd.Y() = aBLPos.Y() + nRectHeight  * (i + 11) / 100;
                pVDev->DrawLine(aStart, aEnd);
            }
        }
    }
    pDev->DrawOutDev(   aRect.TopLeft(), aRectSize,
                        aOrgRect.TopLeft(), aRectSize,
                        *pVDev );
    // jetzt kommt der Text
    if( NUM_PAGETYPE_SINGLENUM == nPageType ||
            NUM_PAGETYPE_BULLET == nPageType )
    {
        Point aStart(aBLPos.X() + nRectWidth / 9,0);
        for( USHORT i = 0; i < 3; i++ )
        {
            USHORT nY = 11 + i * 33;
            aStart.Y() = aBLPos.Y() + nRectHeight  * nY / 100;
            String sText;
            if(nPageType == NUM_PAGETYPE_BULLET)
            {
                sText = aBulletTypes[nItemId - 1];
                aStart.Y() -= pDev->GetTextHeight()/2;
                aStart.X() = aBLPos.X() + 5;
            }
            else
            {
                SvxExtNumType eNumType = aSglNumTypes[nItemId - 1];
                sal_Unicode cChar = aNumChar[(USHORT)eNumType];

                sText = sal_Unicode( aSglPostPreFixes[2 * (nItemId -1)] );
                if(eNumType == SVX_NUM_ROMAN_UPPER||
                        eNumType == SVX_NUM_ROMAN_LOWER)
                {

                    for(USHORT j = 0; j <= i; j++)
                        sText += cChar;
                }
                else
                    sText += (sal_Unicode)(cChar + i);
                sText += aSglPostPreFixes[2 * (nItemId - 1) + 1];
                // knapp neben dem linken Rand beginnen
                aStart.X() = aBLPos.X() + 2;
                aStart.Y() -= pDev->GetTextHeight()/2;
            }
            pDev->DrawText(aStart, sText);
        }
    }
    else if(NUM_PAGETYPE_NUM == nPageType )
    {
        // Gliederungen werden komplett ins VDev gemalt,
        // damit die Linien angepasst werden
        pVDev->SetFillColor( Color( COL_WHITE ) );
        pVDev->DrawRect(aOrgRect);
        long nStartX = aOrgRect.TopLeft().X();
        long nStartY = aOrgRect.TopLeft().Y();
        USHORT nResId = RID_STR_FULLNUMS_START + nItemId - 1;
        String sFormat(SVX_RES(nResId));
        if(bHTMLMode && RID_STR_FULLNUMS_7 == nResId)
            sFormat = String(SVX_RES(RID_STR_FULLNUMS_7_HTML));
        for( xub_StrLen i = 0; i < 5; i++)
        {
            long nTop = nStartY + nRectHeight * (aLinesArr[2 * i + 11])/100 ;
            Point aLeft(nStartX + nRectWidth *  (aLinesArr[2 * i + 10])/ 100, nTop );
            String sText = sFormat.GetToken(i,'|');
            USHORT nText = (USHORT)sText.ToInt32();
            if(nText > 1)
            {
                // jetzt kommt ein Bullet
                sText = sal_Unicode(nText);
                pVDev->SetFont(aRuleFont);
            }
            else
                pVDev->SetFont(aFont);
            aLeft.Y() -= (pDev->GetTextHeight()/2);
            pVDev->DrawText(aLeft, sText);

            long nLineTop = nStartY + nRectHeight * aLinesArr[2 * i + 1]/100 ;
            Point aLineLeft(pDev->GetTextWidth(sText) + nStartX + nRectWidth * aLinesArr[2 * i] / 100, nLineTop );
            Point aLineRight(nStartX + nRectWidth * 90 /100, nLineTop );
            pVDev->DrawLine(aLineLeft,  aLineRight);
        }
        pDev->DrawOutDev(   aRect.TopLeft(), aRectSize,
                            aOrgRect.TopLeft(), aRectSize,
                            *pVDev );
    }

    pDev->SetFont(aOldFont);
    pDev->SetLineColor(aOldColor);
}

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/

SvxNumValueSet::SvxNumValueSet( Window* pParent, const ResId& rResId, USHORT nType ) :

    ValueSet( pParent, rResId ),

    pVDev       ( NULL ),
    nPageType   ( nType ),
    bHTMLMode   ( FALSE ),
    aLineColor  ( COL_LIGHTGRAY )

{
    SetColCount( 4 );
    SetStyle( GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    if ( nType != NUM_PAGETYPE_BMP )
    {
        for ( USHORT i = 0; i < 8; i++ )
            InsertItem( i + 1, i );
    }
}

/*-----------------08.02.97 12.38-------------------

--------------------------------------------------*/

 SvxNumValueSet::~SvxNumValueSet()
{
    delete pVDev;
}

/**************************************************************************/
/*                                                                        */
/*                                                                        */
/**************************************************************************/
//Bug #51425#  MSVC 4.1 Optimierungsproblem        10.20.6166
#pragma optimize("g", off)
SvxBitmapPickTabPage::SvxBitmapPickTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_PICK_BMP ), rSet ),
    aValuesGB(      this, ResId(BG_VALUES) ),
    pExamplesVS(    new SvxBmpNumValueSet(this, ResId(VS_VALUES), aGrfNames )),
    aErrorText(     this, ResId(FT_ERROR)),
    aLinkedCB(      this, ResId(CB_LINKED)),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl( USHRT_MAX ),
    bModified(FALSE),
    bPreset(FALSE),
    bHasChild(/*FALSE*/TRUE),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    FreeResource();
    SetExchangeSupport();
    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));
    pExamplesVS->SetSelectHdl(LINK(this, SvxBitmapPickTabPage, NumSelectHdl_Impl));
    pExamplesVS->SetDoubleClickHdl(LINK(this, SvxBitmapPickTabPage, DoubleClickHdl_Impl));
    aLinkedCB.SetClickHdl(LINK(this, SvxBitmapPickTabPage, LinkBmpHdl_Impl));

    // Grafiknamen ermitteln

    GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames);
    pExamplesVS->SetHelpId(HID_VALUESET_NUMBMP    );
    for(USHORT i = 0; i < aGrfNames.Count(); i++)
    {
        pExamplesVS->InsertItem( i + 1, i);
        String* pGrfNm = (String*) aGrfNames.GetObject(i);
        *pGrfNm = URIHelper::SmartRelToAbs(*pGrfNm);
        pExamplesVS->SetItemText( i + 1, *pGrfNm );
    }
    if(!aGrfNames.Count())
    {
        aErrorText.Show();
    }
    else
    {
        pExamplesVS->Show();
        pExamplesVS->Format();
    }

}

#pragma optimize("", on)

/*-----------------12.02.97 07.46-------------------

--------------------------------------------------*/

 SvxBitmapPickTabPage::~SvxBitmapPickTabPage()
{
    String* pStr = (String*)aGrfNames.First();
    while( pStr )
    {
        delete pStr;
        pStr = (String*)aGrfNames.Next();
    }
    delete pExamplesVS;
    delete pActNum;
    delete pSaveNum;
}

/*-----------------12.02.97 07.46-------------------

--------------------------------------------------*/

SfxTabPage*  SvxBitmapPickTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxBitmapPickTabPage(pParent, rAttrSet);
}

/*-----------------12.02.97 07.46-------------------

--------------------------------------------------*/

void  SvxBitmapPickTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    bPreset = FALSE;
    BOOL bIsPreset = FALSE;
//  nActNumLvl = ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, FALSE, &pItem))
            bIsPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, FALSE, &pItem))
            nActNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, FALSE, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    if(*pSaveNum != *pActNum)
    {
        *pActNum = *pSaveNum;
        pExamplesVS->SetNoSelection();
    }
    // ersten Eintrag vorselektieren
    if(aGrfNames.Count() &&
        (pActNum && (!lcl_IsNumFmtSet(pActNum, nActNumLvl) || bIsPreset)))
    {
        pExamplesVS->SelectItem(1);
        NumSelectHdl_Impl(pExamplesVS);
        bPreset = TRUE;
    }
    bPreset |= bIsPreset;
    bModified = FALSE;
}
/*-----------------12.02.97 07.46-------------------

--------------------------------------------------*/

int  SvxBitmapPickTabPage::DeactivatePage(SfxItemSet *pSet)
{
    if(pSet)
        FillItemSet(*pSet);
    return TRUE;
}
/*-----------------12.02.97 07.46-------------------

--------------------------------------------------*/

BOOL  SvxBitmapPickTabPage::FillItemSet( SfxItemSet& rSet )
{
    if ( !aGrfNames.Count() )
    {
// das ist im SfxItemSet leider nicht zulaessig #52134#
//      rSet.DisableItem(SID_ATTR_NUMBERING_RULE);
        return FALSE;
    }
    if(bPreset || bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, bPreset));
    }

    return bModified;
}
/*-----------------12.02.97 07.46-------------------

--------------------------------------------------*/

void  SvxBitmapPickTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    //im Draw gibt es das Item als WhichId, im Writer nur als SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, FALSE, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, FALSE, &pItem);
    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!")
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

//  nActNumLvl = ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();

    if(SFX_ITEM_SET == rSet.GetItemState(SID_PARAM_CHILD_LEVELS, FALSE, &pItem))
        bHasChild = ((const SfxBoolItem*)pItem)->GetValue();
    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    if(!pActNum->IsFeatureSupported(NUM_ENABLE_LINKED_BMP))
    {
        aLinkedCB.Check(FALSE);
        aLinkedCB.Enable(FALSE);
    }
    else if(!pActNum->IsFeatureSupported(NUM_ENABLE_EMBEDDED_BMP))
    {
        aLinkedCB.Check(TRUE);
        aLinkedCB.Enable(FALSE);
    }
}

/*-----------------12.02.97 07.53-------------------

--------------------------------------------------*/

IMPL_LINK(SvxBitmapPickTabPage, NumSelectHdl_Impl, ValueSet*, EMPTYARG)
{
    if(pActNum)
    {
        bPreset = FALSE;
        bModified = TRUE;
        USHORT nIdx = pExamplesVS->GetSelectItemId() - 1;

        String* pGrfName = 0;
        if(aGrfNames.Count() > nIdx)
        {
            pGrfName = (String*)aGrfNames.GetObject(nIdx);
            *pGrfName = URIHelper::SmartRelToAbs(*pGrfName);
        }
        USHORT nMask = 1;
        String aEmptyStr;
        USHORT nSetNumType = SVX_NUM_BITMAP;
        if(aLinkedCB.IsChecked())
            nSetNumType |= LINK_TOKEN;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aFmt(pActNum->GetLevel(i));
                aFmt.SetNumType((SvxExtNumType)nSetNumType);
                aFmt.SetPrefix( aEmptyStr );
                aFmt.SetSuffix( aEmptyStr );
                aFmt.SetCharFmt( sNumCharFmtName );

                SvxBmpItemInfo* pInfo = pExamplesVS->FindInfo(nIdx + 1);
                const Graphic* pGraphic = pInfo->pBrushItem->GetGraphic(SfxObjectShell::Current());
                if(pGraphic)
                {
                    Size aSize = SvxNumberFormat::GetGraphicSizeMM100(pGraphic);
                    SvxFrameVertOrient eOrient = SVX_VERT_LINE_CENTER;
                    aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)eCoreUnit);
                    aFmt.SetGraphicBrush( pInfo->pBrushItem, &aSize, &eOrient );
                }
                else if(pGrfName)
                    aFmt.SetGraphic( *pGrfName );
                pActNum->SetLevel(i, aFmt);
            }
            nMask <<= 1 ;
        }
    }

    return 0;
}

/*-----------------06.06.97 11.17-------------------

--------------------------------------------------*/
IMPL_LINK(SvxBitmapPickTabPage, DoubleClickHdl_Impl, ValueSet*, EMPTYARG)
{
    NumSelectHdl_Impl(pExamplesVS);
    OKButton& rOk = GetTabDialog()->GetOKButton();
    rOk.GetClickHdl().Call(&rOk);
    return 0;
}
/* -----------------03.11.99 13:46-------------------

 --------------------------------------------------*/
IMPL_LINK(SvxBitmapPickTabPage, LinkBmpHdl_Impl, CheckBox*, pBox )
{
    if(!pExamplesVS->IsNoSelection())
    {
        NumSelectHdl_Impl(pExamplesVS);
    }
    return 0;
}
/*-----------------13.02.97 09.40-------------------

--------------------------------------------------*/

SvxBmpNumValueSet::SvxBmpNumValueSet( Window* pParent, const ResId& rResId, const List& rStrNames ) :

    SvxNumValueSet( pParent, rResId, NUM_PAGETYPE_BMP ),

    rStrList    ( rStrNames ),
    bGrfNotFound( FALSE )

{
    SetStyle( GetStyle() | WB_VSCROLL );
    SetLineCount( 2 );
    aFormatTimer.SetTimeout(300);
    aFormatTimer.SetTimeoutHdl(LINK(this, SvxBmpNumValueSet, FormatHdl_Impl));
}

/*-----------------13.02.97 09.41-------------------

--------------------------------------------------*/

 SvxBmpNumValueSet::~SvxBmpNumValueSet()
{
    aFormatTimer.Stop();
    SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)aGrfBrushItems.First();
    while( pInfo )
    {
        delete pInfo->pBrushItem;
        delete pInfo;
        pInfo = (SvxBmpItemInfo*)aGrfBrushItems.Next();
    }
}
/*-----------------13.02.97 14.19-------------------

--------------------------------------------------*/

SvxBmpItemInfo*  SvxBmpNumValueSet::FindInfo(USHORT nInfo)
{
    SvxBmpItemInfo* pRet = 0;
    for ( USHORT i = 0; i < aGrfBrushItems.Count(); i++ )
    {
        SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)aGrfBrushItems.GetObject(i);
        if(pInfo->nItemId == nInfo)
        {
            pRet = pInfo; break;
        }
    }
    return pRet;
}

/*-----------------13.02.97 09.41-------------------

--------------------------------------------------*/

void    SvxBmpNumValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    SvxNumValueSet::UserDraw(rUDEvt);

    Rectangle aRect = rUDEvt.GetRect();
    OutputDevice*  pDev = rUDEvt.GetDevice();
    USHORT  nItemId = rUDEvt.GetItemId();
    Point aBLPos = aRect.TopLeft();

    int nRectHeight = aRect.GetHeight();
    Size aSize(nRectHeight/8, nRectHeight/8);


    if(rStrList.Count() > USHORT( nItemId - 1) )
    {
        String* pGrfName = (String*)rStrList.GetObject(nItemId - 1);
        SvxBmpItemInfo* pInfo;
        if(0 == (pInfo = FindInfo(nItemId)))
        {
            pInfo = new SvxBmpItemInfo();
            pInfo->nItemId = nItemId;
            String aEmptyStr;
            pInfo->pBrushItem = new SvxBrushItem(*pGrfName, aEmptyStr, GPOS_AREA);
            pInfo->pBrushItem->SetDoneLink(STATIC_LINK(
                                    this, SvxBmpNumValueSet, GraphicArrivedHdl_Impl));
            aGrfBrushItems.Insert(pInfo, aGrfBrushItems.Count());
        }

        const Graphic* pGraphic = pInfo->pBrushItem->GetGraphic(SfxObjectShell::Current());
        if(pGraphic)
        {
            Point aPos(aBLPos.X() + 5, 0);
            for( USHORT i = 0; i < 3; i++ )
            {
                USHORT nY = 11 + i * 33;
                aPos.Y() = aBLPos.Y() + nRectHeight  * nY / 100;
                pGraphic->Draw(pDev, aPos, aSize);
            }
        }
        else
        {
            bGrfNotFound = TRUE;
        }
    }
}

/*-----------------13.02.97 09.41-------------------

--------------------------------------------------*/

IMPL_STATIC_LINK(SvxBmpNumValueSet, GraphicArrivedHdl_Impl, SvxBrushItem*, pItem)
{
    // Ueber Timer wird ein Format angeworfen
    pThis->aFormatTimer.Start();
    return 0;
}

/*-----------------14.02.97 07.34-------------------

--------------------------------------------------*/

IMPL_LINK(SvxBmpNumValueSet, FormatHdl_Impl, Timer*, EMPTYARG)
{
    // nur, wenn eine Grafik nicht da war, muss formatiert werden
    if(bGrfNotFound)
    {
        bGrfNotFound = FALSE;
        Format();
    }
    Invalidate();
    return 0;
}
/*-----------------01.12.97 16:15-------------------
    Tabpage Numerierungsoptionen
--------------------------------------------------*/
#define NUM_NO_GRAPHIC 1000
SvxNumOptionsTabPage::SvxNumOptionsTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_NUM_OPTIONS ), rSet ),

    aLevelGB(       this, ResId(GB_LEVEL    )),
    aLevelLB(       this, ResId(LB_LEVEL    )),
    aFormatGB(      this, ResId(GB_FORMAT   )),
    aPrefixFT(      this, ResId(FT_PREFIX   )),
    aPrefixED(      this, ResId(ED_PREFIX   )),
    aFmtFT(         this, ResId(FT_FMT      )),
    aFmtLB(         this, ResId(LB_FMT      )),
    aSuffixFT(      this, ResId(FT_SUFFIX   )),
    aSuffixED(      this, ResId(ED_SUFFIX   )),
    aCharFmtFT(     this, ResId(FT_CHARFMT  )),
    aCharFmtLB(     this, ResId(LB_CHARFMT  )),
    aBulColorFT(    this, ResId(FT_BUL_COLOR)),
    aBulColLB(      this, ResId(LB_BUL_COLOR)),
    aBulRelSizeFT(  this, ResId(FT_BUL_REL_SIZE)),
    aBulRelSizeMF(  this, ResId(MF_BUL_REL_SIZE)),
    aAllLevelFT(    this, ResId(FT_ALL_LEVEL)),
    aAllLevelNF(    this, ResId(NF_ALL_LEVEL)),
    aStartFT(       this, ResId(FT_START    )),
    aStartED(       this, ResId(ED_START    )),
    aSameLevelGB(   this, ResId(GB_SAME_LEVEL)),
    aSameLevelCB(   this, ResId(CB_SAME_LEVEL)),
    aBulletPB(      this, ResId(PB_BULLET   )),
    aBitmapFT(      this, ResId(FT_BITMAP   )),
    aBitmapMB(      this, ResId(MB_BITMAP   )),
    aRatioCB(       this, ResId(CB_RATIO    )),
    aSizeFT(        this, ResId(FT_SIZE     )),
    aWidthMF(       this, ResId(MF_WIDTH    )),
    aMultFT(        this, ResId(FT_MULT     )),
    aHeightMF(      this, ResId(MF_HEIGHT   )),
    aOrientFT(      this, ResId(FT_ORIENT   )),
    aOrientLB(      this, ResId(LB_ORIENT   )),
    aPreviewGB(     this, ResId(GB_PREVIEW  )),
    aUseBulletCB(   this, ResId(CB_USE_BULLET)),
    aAlignFT(       this, ResId(FT_ALIGN    )),
    aAlignLB(       this, ResId(LB_ALIGN    )),
    pPreviewWIN(    new SvxNumberingPreview(this, ResId(WIN_PREVIEW ))),
    sBullet(ResId(STR_BULLET)),
    pActNum(0),
    pSaveNum(0),
    nActNumLvl(USHRT_MAX),
    nBullet(0xff),
    bHasChild(FALSE),
    bLastWidthModified(FALSE),
    bAutomaticCharStyles(TRUE),
    bHTMLMode(FALSE),
    bMenuButtonInitialized(FALSE),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    sStartWith = aStartFT.GetText();
    pPreviewWIN->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));
    SetExchangeSupport();
    aActBulletFont = lcl_GetDefaultBulletFont();

    aBulletPB.SetClickHdl(LINK(this, SvxNumOptionsTabPage, BulletHdl_Impl));
    aFmtLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, NumberTypeSelectHdl_Impl));
    aBitmapMB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, GraphicHdl_Impl));
    aLevelLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, LevelHdl_Impl));
    aCharFmtLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, CharFmtHdl_Impl));
    aWidthMF.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, SizeHdl_Impl));
    aHeightMF.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, SizeHdl_Impl));
    aRatioCB.SetClickHdl(LINK(this, SvxNumOptionsTabPage, RatioHdl_Impl));
    aStartED.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    aPrefixED.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    aSuffixED.SetModifyHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    aAllLevelNF.SetModifyHdl(LINK(this,SvxNumOptionsTabPage, AllLevelHdl_Impl));
    aOrientLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, OrientHdl_Impl));
    aSameLevelCB.SetClickHdl(LINK(this, SvxNumOptionsTabPage, SameLevelHdl_Impl));
    aBulRelSizeMF.SetModifyHdl(LINK(this,SvxNumOptionsTabPage, BulRelSizeHdl_Impl));
    aBulColLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, BulColorHdl_Impl));
    aUseBulletCB.SetClickHdl(LINK(this, SvxNumOptionsTabPage, UseBulletHdl_Impl));

    aBitmapMB.GetPopupMenu()->SetHighlightHdl(LINK(this, SvxNumOptionsTabPage, PopupActivateHdl_Impl));
    PopupMenu* pPopup = new PopupMenu;
    aBitmapMB.GetPopupMenu()->SetPopupMenu( MN_GALLERY, pPopup );

    pPopup->InsertItem( NUM_NO_GRAPHIC, String(ResId(ST_POPUP_EMPTY_ENTRY)) );
    pPopup->EnableItem( NUM_NO_GRAPHIC, FALSE );

    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));

    FreeResource();
}

/*-----------------01.12.97 16:30-------------------

--------------------------------------------------*/
SvxNumOptionsTabPage::~SvxNumOptionsTabPage()
{
    delete aBitmapMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );
    String* pStr = (String*)aGrfNames.First();
    while( pStr )
    {
        delete pStr;
        pStr = (String*)aGrfNames.Next();
    }
    SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)aGrfBrushItems.First();
    while( pInfo )
    {
        delete pInfo->pBrushItem;
        delete pInfo;
        pInfo = (SvxBmpItemInfo*)aGrfBrushItems.Next();
    }
    delete pActNum;
    delete pPreviewWIN;
    delete pSaveNum;
}

/*-----------------03.12.97 07:52-------------------

--------------------------------------------------*/
void SvxNumOptionsTabPage::SetMetric(FieldUnit eMetric)
{
    if(eMetric == FUNIT_MM)
    {
        aWidthMF     .SetDecimalDigits(1);
        aHeightMF     .SetDecimalDigits(1);
    }
    aWidthMF .SetUnit( eMetric );
    aHeightMF .SetUnit( eMetric );
}

/*-----------------01.12.97 16:30-------------------

--------------------------------------------------*/
SfxTabPage* SvxNumOptionsTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxNumOptionsTabPage(pParent, rAttrSet);
};
/*-----------------01.12.97 16:29-------------------

--------------------------------------------------*/
void    SvxNumOptionsTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    UINT16 nTmpNumLvl = USHRT_MAX;
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, FALSE, &pItem))
            bPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, FALSE, &pItem))
            nTmpNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, FALSE, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    //
    bModified = (!pActNum->Get( 0 ) || bPreset);
    if(*pActNum != *pSaveNum ||
        nActNumLvl != nTmpNumLvl)
    {
        nActNumLvl = nTmpNumLvl;
        USHORT nMask = 1;
        aLevelLB.SetUpdateMode(FALSE);
        aLevelLB.SetNoSelection();
        aLevelLB.SelectEntryPos( pActNum->GetLevelCount(), nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                    aLevelLB.SelectEntryPos( i, TRUE);
                nMask <<= 1 ;
            }
        aLevelLB.SetUpdateMode(TRUE);
        *pActNum = *pSaveNum;
        InitControls();
    }

}
/*-----------------01.12.97 16:29-------------------

--------------------------------------------------*/
int     SvxNumOptionsTabPage::DeactivatePage(SfxItemSet *pSet)
{
//  ((SwNumBulletTabDialog*)GetTabDialog())->SetActNumLevel(nActNumLvl);
    if(pSet)
        FillItemSet(*pSet);
    return TRUE;
}
/*-----------------01.12.97 16:29-------------------

--------------------------------------------------*/
BOOL    SvxNumOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    rSet.Put(SfxUInt16Item(SID_PARAM_CUR_NUM_LEVEL, nActNumLvl));
    if(bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, FALSE));
    }
    return bModified;
};
/*-----------------01.12.97 16:29-------------------

--------------------------------------------------*/
void    SvxNumOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    //im Draw gibt es das Item als WhichId, im Writer nur als SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, FALSE, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, FALSE, &pItem);
    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!")
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    // Ebenen einfuegen
    if(!aLevelLB.GetEntryCount())
    {
        for(USHORT i = 1; i <= pSaveNum->GetLevelCount(); i++)
            aLevelLB.InsertEntry( UniString::CreateFromInt32(i));
        if(pSaveNum->GetLevelCount() > 1)
        {
            String sEntry( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "1 - " ) ) );
            sEntry += UniString::CreateFromInt32( pSaveNum->GetLevelCount() );
            aLevelLB.InsertEntry(sEntry);
            aLevelLB.SelectEntry(sEntry);
        }
        else
            aLevelLB.SelectEntryPos(0);
    }
    else
        aLevelLB.SelectEntryPos(aLevelLB.GetEntryCount() - 1);

//  nActNumLvl = ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();
    USHORT nMask = 1;
    aLevelLB.SetUpdateMode(FALSE);
    aLevelLB.SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        aLevelLB.SelectEntryPos( pSaveNum->GetLevelCount(), TRUE);
    }
    else
        for(USHORT i = 0; i < pSaveNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
                aLevelLB.SelectEntryPos( i, TRUE);
            nMask <<= 1 ;
        }
    aLevelLB.SetUpdateMode(TRUE);

    if(SFX_ITEM_SET == rSet.GetItemState(SID_PARAM_CHILD_LEVELS, FALSE, &pItem))
        bHasChild = ((const SfxBoolItem*)pItem)->GetValue();
    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    pPreviewWIN->SetNumRule(pActNum);
    aSameLevelCB.Check(pActNum->IsContinuousNumbering());

    //ColorListBox bei Bedarf fuellen
    if ( pActNum->IsFeatureSupported( NUM_BULLET_COLOR ) )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        DBG_ASSERT( pDocSh, "DocShell not found!" );
        XColorTable* pColorTable = NULL;
        FASTBOOL bKillTable = FALSE;
        if ( pDocSh && ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
            pColorTable = ( (SvxColorTableItem*)pItem )->GetColorTable();

        if ( !pColorTable )
        {
            pColorTable = new XColorTable( SvtPathOptions().GetPalettePath() );
            bKillTable = TRUE;
        }

        for ( long i = 0; i < pColorTable->Count(); i++ )
        {
            XColorEntry* pEntry = pColorTable->Get(i);
            aBulColLB.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }

        if ( bKillTable )
            delete pColorTable;
    }

    SfxObjectShell* pShell;
    if ( SFX_ITEM_SET == rSet.GetItemState( SID_HTML_MODE, FALSE, &pItem )
         || ( 0 != ( pShell = SfxObjectShell::Current()) &&
              0 != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        USHORT nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        bHTMLMode = 0 != (nHtmlMode&HTMLMODE_ON);
    }

    BOOL bCharFmt = pActNum->IsFeatureSupported(NUM_CHAR_STYLE);
    aCharFmtFT.Show(bCharFmt);
    aCharFmtLB.Show(bCharFmt);

    BOOL bContinuous = pActNum->IsFeatureSupported(NUM_CONTINUOUS);

    BOOL bAllLevel = bContinuous && !bHTMLMode;
    aAllLevelFT.Show(bAllLevel);
    aAllLevelNF.Show(bAllLevel);

    aSameLevelGB.Show(bContinuous);
    aSameLevelCB.Show(bContinuous);
    //wieder Missbrauch: im Draw gibt es die Numerierung nur bis zum Bitmap
    // without SVX_NUM_NUMBER_NONE
    if(!bContinuous)
    {
        USHORT nFmtCount = aFmtLB.GetEntryCount();
        for(USHORT i = nFmtCount; i; i--)
        {
            USHORT nEntryData = (USHORT)(ULONG)aFmtLB.GetEntryData(i - 1);
            if(SVX_NUM_NUMBER_NONE == nEntryData ||
                SVX_NUM_BITMAP < nEntryData)
                aFmtLB.RemoveEntry(i - 1);
        }
    }
    //one must be enabled
    if(!pActNum->IsFeatureSupported(NUM_ENABLE_LINKED_BMP))
    {
        long nData = SVX_NUM_BITMAP|LINK_TOKEN;
        USHORT nPos = aFmtLB.GetEntryPos((void*)nData);
        if(LISTBOX_ENTRY_NOTFOUND != nPos)
            aFmtLB.RemoveEntry(nPos);
    }
    else if(!pActNum->IsFeatureSupported(NUM_ENABLE_EMBEDDED_BMP))
    {
        long nData = SVX_NUM_BITMAP;
        USHORT nPos = aFmtLB.GetEntryPos((void*)nData);
        if(LISTBOX_ENTRY_NOTFOUND != nPos)
            aFmtLB.RemoveEntry(nPos);
    }
    if(pActNum->IsFeatureSupported(NUM_HIDDEN_SYMBOLS))
    {
        Size aSz(aFormatGB.GetSizePixel());
        aSz.Height() = aLevelGB.GetSizePixel().Height();
        aFormatGB.SetSizePixel(aSz);
        aUseBulletCB.Show(TRUE);
    }
    if(pActNum->IsFeatureSupported(NUM_SYMBOL_ALIGNMENT))
    {
        aAlignFT.Show();
        aAlignLB.Show();
        Size aSz(aFormatGB.GetSizePixel());
        aSz.Height() = aLevelGB.GetSizePixel().Height();
        aFormatGB.SetSizePixel(aSz);
        aAlignLB.SetSelectHdl(LINK(this, SvxNumOptionsTabPage, EditModifyHdl_Impl));
    }

    //MegaHack: Aufgrund eines nicht fixbaren 'designfehlers' im Impress
    //Alle arten der numerischen Aufzaehlungen loeschen
    if(pActNum->IsFeatureSupported(NUM_NO_NUMBERS))
    {
        USHORT nFmtCount = aFmtLB.GetEntryCount();
        for(USHORT i = nFmtCount; i; i--)
        {
            USHORT nEntryData = (USHORT)(ULONG)aFmtLB.GetEntryData(i - 1);
            if(nEntryData >= SVX_NUM_CHARS_UPPER_LETTER &&  nEntryData <= SVX_NUM_NUMBER_NONE)
                aFmtLB.RemoveEntry(i - 1);
        }
    }

    InitControls();
    bModified = FALSE;

}
/*-----------------02.12.97 13:47-------------------

--------------------------------------------------*/
void SvxNumOptionsTabPage::InitControls()
{
    BOOL bShowBullet    = TRUE;
    BOOL bShowBitmap    = TRUE;
    BOOL bSameType      = TRUE;
    BOOL bSameStart     = TRUE;
    BOOL bSamePrefix    = TRUE;
    BOOL bSameSuffix    = TRUE;
    BOOL bAllLevel      = TRUE;
    BOOL bSameCharFmt   = TRUE;
    BOOL bSameVOrient   = TRUE;
    BOOL bSameSize      = TRUE;
    BOOL bSameBulColor  = TRUE;
    BOOL bSameBulRelSize= TRUE;
    BOOL bSameUseBullets = TRUE;
    BOOL bSameAdjust    = TRUE;

    const SvxNumberFormat* aNumFmtArr[SVX_MAX_NUM];
    String sFirstCharFmt;
    SvxFrameVertOrient eFirstOrient = SVX_VERT_NONE;
    Size aFirstSize(0,0);
    USHORT nMask = 1;
    USHORT nLvl = USHRT_MAX;
    USHORT nHighestLevel = 0;
    String aEmptyStr;

    BOOL bBullColor = pActNum->IsFeatureSupported(NUM_BULLET_COLOR);
    BOOL bBullRelSize = pActNum->IsFeatureSupported(NUM_BULLET_REL_SIZE);
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            aNumFmtArr[i] = &pActNum->GetLevel(i);
            bShowBullet &= aNumFmtArr[i]->GetNumType() == SVX_NUM_CHAR_SPECIAL;
            bShowBitmap &= (aNumFmtArr[i]->GetNumType()&(~LINK_TOKEN)) == SVX_NUM_BITMAP;
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
                sFirstCharFmt = aNumFmtArr[i]->GetCharFmt();
                eFirstOrient = aNumFmtArr[i]->GetVertOrient();
                if(bShowBitmap)
                    aFirstSize = aNumFmtArr[i]->GetGraphicSize();
            }
            if( i > nLvl)
            {
                bSameType &=   aNumFmtArr[i]->GetNumType() == aNumFmtArr[nLvl]->GetNumType();
                bSameStart = aNumFmtArr[i]->GetStart() == aNumFmtArr[nLvl]->GetStart();

                bSamePrefix = aNumFmtArr[i]->GetPrefix() == aNumFmtArr[nLvl]->GetPrefix();
                bSameSuffix = aNumFmtArr[i]->GetSuffix() == aNumFmtArr[nLvl]->GetSuffix();
                bAllLevel &= aNumFmtArr[i]->GetIncludeUpperLevels() == aNumFmtArr[nLvl]->GetIncludeUpperLevels();
                bSameCharFmt    &=  sFirstCharFmt == aNumFmtArr[i]->GetCharFmt();
                bSameVOrient    &= eFirstOrient == aNumFmtArr[i]->GetVertOrient();
                if(bShowBitmap && bSameSize)
                    bSameSize &= aNumFmtArr[i]->GetGraphicSize() == aFirstSize;
                bSameBulColor &= aNumFmtArr[i]->GetBulletColor() == aNumFmtArr[nLvl]->GetBulletColor();
                bSameBulRelSize &= aNumFmtArr[i]->GetBulletRelSize() == aNumFmtArr[nLvl]->GetBulletRelSize();
                bSameUseBullets &= aNumFmtArr[i]->IsShowSymbol() == aNumFmtArr[nLvl]->IsShowSymbol();
                bSameAdjust     &= aNumFmtArr[i]->GetNumAdjust() == aNumFmtArr[nLvl]->GetNumAdjust();
            }
            nHighestLevel = i;
        }
        else
            aNumFmtArr[i] = 0;

        nMask <<= 1 ;

    }
    SwitchNumberType(bShowBullet ? 1 : bShowBitmap ? 2 : 0);
    if(bShowBitmap)
    {
        if(!bSameVOrient || eFirstOrient == SVX_VERT_NONE)
            aOrientLB.SetNoSelection();
        else
            aOrientLB.SelectEntryPos(eFirstOrient - 1);// kein SVX_VERT_NONE

        if(bSameSize)
        {
            SetMetricValue(aHeightMF, aFirstSize.Height(), eCoreUnit);
            SetMetricValue(aWidthMF, aFirstSize.Width(), eCoreUnit);
        }
        else
        {
            aHeightMF.SetText(aEmptyStr);
            aWidthMF.SetText(aEmptyStr);
        }
    }

    if(bSameType)
    {
        USHORT nLBData = (USHORT) aNumFmtArr[nLvl]->GetNumType();
        aFmtLB.SelectEntryPos(aFmtLB.GetEntryPos( (void*) nLBData ));
    }
    else
        aFmtLB.SetNoSelection();

    aAllLevelNF.Enable(nHighestLevel > 0 && !aSameLevelCB.IsChecked());
    aAllLevelNF.SetMax(nHighestLevel + 1);
    if(bAllLevel)
    {
        aAllLevelNF.SetValue(aNumFmtArr[nLvl]->GetIncludeUpperLevels());
    }
    else
    {
        aAllLevelNF.SetText(aEmptyStr);
    }
    if(bSameUseBullets)
        aUseBulletCB.Check(
            aNumFmtArr[nLvl]->IsShowSymbol() ? STATE_CHECK : STATE_NOCHECK);
    else
    {
        aUseBulletCB.EnableTriState(TRUE);
        aUseBulletCB.SetState(STATE_DONTKNOW);
    }
    if(bSameAdjust)
    {
        USHORT nPos = 1; // zentriert
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        aAlignLB.SelectEntryPos(nPos);
    }
    else
    {
        aAlignLB.SetNoSelection();
    }

    if(bBullRelSize)
    {
        if(bSameBulRelSize)
            aBulRelSizeMF.SetValue(aNumFmtArr[nLvl]->GetBulletRelSize());
        else
            aBulRelSizeMF.SetText(aEmptyStr);
    }
    if(bBullColor)
    {
        if(bSameBulColor)
            aBulColLB.SelectEntry(aNumFmtArr[nLvl]->GetBulletColor());
        else
            aBulColLB.SetNoSelection();
    }
    switch(nBullet)
    {
        case SHOW_NUMBERING:
            if(bSameStart)
            {
                aStartED.SetValue(aNumFmtArr[nLvl]->GetStart());
            }
            else
                aStartED.SetText(aEmptyStr);
        break;
        case SHOW_BULLET:
        {
//              const Font* pFnt = aNumFmtArr[Lvl]->GetBulletFont();
//              if(pFnt)
//                  ChgTxtFont(aBulletFT, *pFnt);
//              aBulletFT.SetText(String((char)aNumFmtArr[nLvl]->GetBulletChar()));
        }
        break;
        case SHOW_BITMAP:
        break;
    }

    if(bSamePrefix)
        aPrefixED.SetText(aNumFmtArr[nLvl]->GetPrefix());
    else
        aPrefixED.SetText(aEmptyStr);
    if(bSameSuffix)
        aSuffixED.SetText(aNumFmtArr[nLvl]->GetSuffix());
    else
        aSuffixED.SetText(aEmptyStr);

    if(bSameCharFmt)
    {
        if(sFirstCharFmt.Len())
                aCharFmtLB.SelectEntry(sFirstCharFmt);
        else
            aCharFmtLB.SelectEntryPos( 0 );
    }
    else
        aCharFmtLB.SetNoSelection();
}

/*-----------------02.12.97 14:01-------------------
     0 - Nummer; 1 - Bullet; 2 - Bitmap
--------------------------------------------------*/

void SvxNumOptionsTabPage::SwitchNumberType( BYTE nType, BOOL bBmp )
{
    if(nBullet == nType)
        return;
    nBullet = nType;
    BOOL bBitmap = FALSE;
    BOOL bBullet = FALSE;
    BOOL bEnableBitmap = FALSE;
    if(nType == SHOW_NUMBERING)
    {
        // Label umschalten, alten Text merken
        aStartFT.SetText(sStartWith);

    }
    else if(nType == SHOW_BULLET)
    {
        // Label umschalten, alten Text merken
        aStartFT.SetText(sBullet);
        bBullet = TRUE;
    }
    else
    {
        bBitmap = TRUE;
        bEnableBitmap = TRUE;
    }
    BOOL bNumeric = !(bBitmap||bBullet);
    aPrefixFT.Show(bNumeric);
    aPrefixED.Show(bNumeric);
    aSuffixFT.Show(bNumeric);
    aSuffixED.Show(bNumeric);

    BOOL bCharFmt = pActNum->IsFeatureSupported(NUM_CHAR_STYLE);
    aCharFmtFT.Show(!bBitmap && bCharFmt);
    aCharFmtLB.Show(!bBitmap && bCharFmt);

    // das ist eigentlich Missbrauch, da fuer die vollst. Numerierung kein
    // eigenes Flag existiert
    BOOL bAllLevelFeature = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
    BOOL bAllLevel = bNumeric && bAllLevelFeature && !bHTMLMode;
    aAllLevelFT.Show(bAllLevel);
    aAllLevelNF.Show(bAllLevel);

    aStartFT.Show(!bBitmap);
    aStartED.Show(!(bBullet||bBitmap));

    aBulletPB.Show(bBullet);
    BOOL bBullColor = pActNum->IsFeatureSupported(NUM_BULLET_COLOR);
    aBulColorFT.Show(   !bBitmap && bBullColor );
    aBulColLB.Show(     !bBitmap && bBullColor );
    BOOL bBullResSize = pActNum->IsFeatureSupported(NUM_BULLET_REL_SIZE);
    aBulRelSizeFT.Show( !bBitmap && bBullResSize );
    aBulRelSizeMF.Show( !bBitmap && bBullResSize );

    aBitmapFT   .Show(bBitmap);
    aBitmapMB   .Show(bBitmap);

    aSizeFT     .Show(bBitmap);
    aWidthMF    .Show(bBitmap);
    aMultFT     .Show(bBitmap);
    aHeightMF   .Show(bBitmap);
    aRatioCB    .Show(bBitmap);

    aOrientFT   .Show(bBitmap &&  bAllLevelFeature);
    aOrientLB   .Show(bBitmap &&  bAllLevelFeature);

    aSizeFT     .Enable(bEnableBitmap);
    aWidthMF    .Enable(bEnableBitmap);
    aMultFT     .Enable(bEnableBitmap);
    aHeightMF   .Enable(bEnableBitmap);
    aRatioCB    .Enable(bEnableBitmap);
    aOrientFT   .Enable(bEnableBitmap);
    aOrientLB   .Enable(bEnableBitmap);

}
/*-----------------02.12.97 13:51-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, LevelHdl_Impl, ListBox *, pBox )
{
    USHORT nSaveNumLvl = nActNumLvl;
    nActNumLvl = 0;
    if(pBox->IsEntryPosSelected( pActNum->GetLevelCount() ) &&
        (pBox->GetSelectEntryCount() == 1 || nSaveNumLvl != 0xffff))
    {
        nActNumLvl = 0xFFFF;
        pBox->SetUpdateMode(FALSE);
        for( USHORT i = 0; i < pActNum->GetLevelCount(); i++ )
            pBox->SelectEntryPos( i, FALSE );
        pBox->SetUpdateMode(TRUE);
    }
    else if(pBox->GetSelectEntryCount())
    {
        USHORT nMask = 1;
        for( USHORT i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(pBox->IsEntryPosSelected( i ))
                nActNumLvl |= nMask;
            nMask <<= 1;
        }
        pBox->SelectEntryPos( pActNum->GetLevelCount(), FALSE );
    }
    else
    {
        nActNumLvl = nSaveNumLvl;
        USHORT nMask = 1;
        for( USHORT i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(nActNumLvl & nMask)
            {
                pBox->SelectEntryPos(i);
                break;
            }
            nMask <<=1;
        }
    }
    InitControls();
    return 0;
}
/* -----------------24.11.98 15:41-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, UseBulletHdl_Impl, TriStateBox*, pBox )
{
    pBox->EnableTriState(FALSE);
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        USHORT nMask = 1;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
                aNumFmt.SetShowSymbol(pBox->IsChecked());
                pActNum->SetLevel(i, aNumFmt);
            }
            nMask <<= 1;
        }
    }
    SetModified();
    return 0;
}
/*-----------------03.12.97 12:01-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, AllLevelHdl_Impl, NumericField*, pBox )
{
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        USHORT nMask = 1;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
                aNumFmt.SetIncludeUpperLevels((BYTE) std::min(pBox->GetValue(), (long) (i + 1)) );
                pActNum->SetLevel(i, aNumFmt);
            }
            nMask <<= 1;
        }
    }
    SetModified();
    return 0;
}

/*-----------------02.12.97 08:56-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, NumberTypeSelectHdl_Impl, ListBox *, pBox )
{
    String sSelectStyle;
    SvxExtNumType eOldType;
    BOOL bShowOrient = FALSE;
    BOOL bBmp = FALSE;
    String aEmptyStr;
    USHORT nMask = 1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            eOldType = aNumFmt.GetNumType();
            // PAGEDESC gibt es nicht
            USHORT nNumType = (USHORT)(ULONG)pBox->GetEntryData(pBox->GetSelectEntryPos());
            aNumFmt.SetNumType((SvxExtNumType)nNumType);
            if(SVX_NUM_BITMAP == (aNumFmt.GetNumType()&(~LINK_TOKEN)))
            {
                bBmp |= 0 != aNumFmt.GetBrush();
                aNumFmt.SetIncludeUpperLevels( FALSE );
                aNumFmt.SetSuffix( aEmptyStr );
                aNumFmt.SetPrefix( aEmptyStr );
                if(!bBmp)
                    aNumFmt.SetGraphic(aEmptyStr);
                pActNum->SetLevel(i, aNumFmt);
                SwitchNumberType(SHOW_BITMAP, bBmp );
                bShowOrient = TRUE;
            }
            else if( SVX_NUM_CHAR_SPECIAL == aNumFmt.GetNumType() )
            {
                aNumFmt.SetIncludeUpperLevels( FALSE );
                aNumFmt.SetSuffix( aEmptyStr );
                aNumFmt.SetPrefix( aEmptyStr );
                if( !aNumFmt.GetBulletFont() )
                    aNumFmt.SetBulletFont(&aActBulletFont);
                if( !aNumFmt.GetBulletChar() )
                    aNumFmt.SetBulletChar( SVX_DEF_BULLET );
                pActNum->SetLevel(i, aNumFmt);
                SwitchNumberType(SHOW_BULLET);
                //ChgTxtFont(aBulletFT, *aNumFmt.GetBulletFont());
                //aBulletFT.SetText( aNumFmt.GetBulletChar() );
                // Zuweisung der Zeichenvorlage automatisch
                if(bAutomaticCharStyles)
                {
                    sSelectStyle = sBulletCharFmtName;
                }
            }
            else
            {
                aNumFmt.SetPrefix( aPrefixED.GetText() );
                aNumFmt.SetSuffix( aSuffixED.GetText() );
//              aNumFmt.SetBulletFont(0);
                SwitchNumberType(SHOW_NUMBERING);
                pActNum->SetLevel(i, aNumFmt);
                // Zuweisung der Zeichenvorlage automatisch
                if(bAutomaticCharStyles)
                {
                    sSelectStyle = sNumCharFmtName;
                }
            }
        }
        nMask <<= 1;
    }
    BOOL bAllLevelFeature = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
    if(bShowOrient && bAllLevelFeature)
    {
        aOrientFT.Show();
        aOrientLB.Show();
    }
    else
    {
        aOrientFT.Hide();
        aOrientLB.Hide();
    }
    SetModified();
    if(sSelectStyle.Len())
    {
        aCharFmtLB.SelectEntry(sSelectStyle);
        CharFmtHdl_Impl(&aCharFmtLB);
        // bAutomaticCharStyles wird im CharFmtHdl_Impl zurueckgesetzt
        bAutomaticCharStyles = TRUE;
    }
    return 0;


}
/*-----------------03.12.97 16:43-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, OrientHdl_Impl, ListBox *, pBox )
{
    USHORT nPos = pBox->GetSelectEntryPos();
    nPos ++; // kein VERT_NONE

    USHORT nMask = 1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if(SVX_NUM_BITMAP == (aNumFmt.GetNumType()&(~LINK_TOKEN)))
            {
                const SvxBrushItem* pBrushItem =  aNumFmt.GetBrush();
                const Size& rSize = aNumFmt.GetGraphicSize();
                SvxFrameVertOrient eOrient = (SvxFrameVertOrient)nPos;
                aNumFmt.SetGraphicBrush( pBrushItem, &rSize, &eOrient );
                pActNum->SetLevel(i, aNumFmt);
            }
        }
        nMask <<= 1;
    }
    SetModified(FALSE);
    return 0;

}

/*-----------------06.12.97 12:00-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, SameLevelHdl_Impl, CheckBox *, pBox )
{
    BOOL bSet = pBox->IsChecked();
    pActNum->SetContinuousNumbering(bSet);
    BOOL bRepaint = FALSE;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
        if(aNumFmt.GetNumType() < SVX_NUM_NUMBER_NONE)
        {
            bRepaint = TRUE;
            break;
        }
    }
    SetModified(bRepaint);
    InitControls();
    return 0;
}
/* -----------------16.11.98 14:20-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, BulColorHdl_Impl, ColorListBox*, pBox )
{
    Color nSetColor = pBox->GetSelectEntryColor();

    USHORT nMask = 1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetBulletColor(nSetColor);
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}
/* -----------------16.11.98 14:20-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, BulRelSizeHdl_Impl, MetricField *, pField)
{
    USHORT nRelSize = (USHORT)pField->GetValue();

    USHORT nMask = 1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            aNumFmt.SetBulletRelSize(nRelSize);
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}

/*-----------------02.12.97 10:50-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, GraphicHdl_Impl, MenuButton *, pButton )
{
    USHORT nItemId = pButton->GetCurItemId();
    const Graphic* pGraphic = 0;
    String aGrfName;
    SvxImportGraphicDialog* pGrfDlg = 0;

    if(MN_GALLERY_ENTRY <= nItemId )
    {
        aGrfName = *((String*)aGrfNames.GetObject( nItemId - MN_GALLERY_ENTRY));
        SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)aGrfBrushItems.GetObject(nItemId - MN_GALLERY_ENTRY);
        pGraphic = pInfo->pBrushItem->GetGraphic();
    }
    else
    {
        pGrfDlg = new SvxImportGraphicDialog( this,
                                SVX_RESSTR(RID_STR_EDIT_GRAPHIC), ENABLE_STANDARD );
        if ( pGrfDlg->Execute() == RET_OK )
        {
            // ausgewaehlten Filter merken
            aGrfName = pGrfDlg->GetPath();
            pGraphic = pGrfDlg->GetGraphic();
        }
    }
    if(pGraphic)
    {
        Size aSize = SvxNumberFormat::GetGraphicSizeMM100(pGraphic);
        aSize = OutputDevice::LogicToLogic(aSize, MAP_100TH_MM, (MapUnit)eCoreUnit);

        USHORT nMask = 1;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
                aNumFmt.SetCharFmt(sNumCharFmtName);
                aNumFmt.SetGraphic(aGrfName);

                // Size schon mal fuer spaeteren Groessenabgleich setzen
                const SvxBrushItem* pBrushItem = aNumFmt.GetBrush();
                SvxFrameVertOrient eOrient = aNumFmt.GetVertOrient();
                aNumFmt.SetGraphicBrush( pBrushItem, &aSize, &eOrient );
                aInitSize[i] = aNumFmt.GetGraphicSize();

                pActNum->SetLevel(i, aNumFmt);
            }
            nMask <<= 1;
        }
        aRatioCB .Enable();
        aSizeFT .Enable();
        aMultFT.Enable();
        aWidthMF .Enable();
        aHeightMF.Enable();
        SetMetricValue(aWidthMF, aSize.Width(), eCoreUnit);
        SetMetricValue(aHeightMF, aSize.Height(), eCoreUnit);
        aOrientFT.Enable();
        aOrientLB.Enable();
        SetModified();
    }
    delete pGrfDlg;
    return 0;
}
/* -----------------27.07.99 12:20-------------------

 --------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, PopupActivateHdl_Impl, Menu *, pMenu )
{
    if(!bMenuButtonInitialized)
    {
        bMenuButtonInitialized = TRUE;
        EnterWait();
        PopupMenu* pPopup = aBitmapMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );
        GalleryExplorer::FillObjList(GALLERY_THEME_BULLETS, aGrfNames);
        if(aGrfNames.Count())
        {
            pPopup->RemoveItem( pPopup->GetItemPos( NUM_NO_GRAPHIC ));
            String aEmptyStr;
            SfxObjectShell *pDocSh = SfxObjectShell::Current();
            for(USHORT i = 0; i < aGrfNames.Count(); i++)
            {
                const String* pGrfName = (const String*)aGrfNames.GetObject(i);

                SvxBrushItem* pBrushItem = new SvxBrushItem(*pGrfName, aEmptyStr, GPOS_AREA);
                pBrushItem->SetDoneLink(STATIC_LINK(
                            this, SvxNumOptionsTabPage, GraphicArrivedHdl_Impl));

                SvxBmpItemInfo* pInfo = new SvxBmpItemInfo();
                pInfo->pBrushItem = pBrushItem;
                pInfo->nItemId = MN_GALLERY_ENTRY + i;
                aGrfBrushItems.Insert(pInfo, i);
                const Graphic* pGraphic = pBrushItem->GetGraphic(pDocSh);

                if(pGraphic)
                {
                    Bitmap aBitmap(pGraphic->GetBitmap());
                    Size aSize(aBitmap.GetSizePixel());
                    if(aSize.Width() > MAX_BMP_WIDTH ||
                        aSize.Height() > MAX_BMP_HEIGHT)
                    {
                        BOOL bWidth = aSize.Width() > aSize.Height();
                        double nScale = bWidth ?
                                            (double)MAX_BMP_WIDTH / (double)aSize.Width():
                                                (double)MAX_BMP_HEIGHT / (double)aSize.Height();
                        aBitmap.Scale(nScale, nScale);

                    }
                    Image aImage(aBitmap);
                    pPopup->InsertItem(
                        pInfo->nItemId, *pGrfName, aImage );
                }
                else
                {
                    Image aImage;
                    pPopup->InsertItem(
                        pInfo->nItemId, *pGrfName, aImage );
                }
            }
        }
        LeaveWait();
    }
    return 0;
}

/*-----------------02.12.97 10:58-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, BulletHdl_Impl, Button *, pBtn )
{
    SvxCharacterMap* pMap = new SvxCharacterMap(this, TRUE);

    USHORT nMask = 1;
    const Font* pFmtFont = 0;
    BOOL bSameBullet = TRUE;
    sal_Unicode cBullet;
    BOOL bFirst = TRUE;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            const SvxNumberFormat&  rCurFmt = pActNum->GetLevel(i);
            if(bFirst)
            {
                 cBullet = rCurFmt.GetBulletChar();
            }
            else if(rCurFmt.GetBulletChar() != cBullet )
            {
                bSameBullet = FALSE;
                break;
            }
            if(!pFmtFont)
                pFmtFont = rCurFmt.GetBulletFont();
            bFirst = FALSE;
        }
        nMask <<= 1;

    }

    if(pFmtFont)
        pMap->SetCharFont(*pFmtFont);
    else
        pMap->SetCharFont(aActBulletFont);
    if(bSameBullet)
        pMap->SetChar( cBullet );
    if(pMap->Execute() == RET_OK)
    {
        // Font Numrules umstellen
        aActBulletFont = pMap->GetCharFont();

        USHORT nMask = 1;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
                aNumFmt.SetBulletFont(&aActBulletFont); ;
                aNumFmt.SetBulletChar( pMap->GetChar() );
                pActNum->SetLevel(i, aNumFmt);
            }
            nMask <<= 1;
        }
//      ChgTxtFont(aBulletFT, aActBulletFont);
//      aBulletFT.SetText( pMap->GetChar() );

        SetModified();
    }
    delete pMap;
    return 0;
}

/*-----------------03.03.97 15:21-------------------

--------------------------------------------------*/

IMPL_LINK( SvxNumOptionsTabPage, SizeHdl_Impl, MetricField *, pField)
{
    BOOL bWidth = pField == &aWidthMF;
    bLastWidthModified = bWidth;
    BOOL bRatio = aRatioCB.IsChecked();
    long nWidthVal = aWidthMF.Denormalize(aWidthMF.GetValue(FUNIT_100TH_MM));
    long nHeightVal = aHeightMF.Denormalize(aHeightMF.GetValue(FUNIT_100TH_MM));
    nWidthVal = OutputDevice::LogicToLogic( nWidthVal ,
                                                MAP_100TH_MM, (MapUnit)eCoreUnit );
    nHeightVal = OutputDevice::LogicToLogic( nHeightVal,
                                                MAP_100TH_MM, (MapUnit)eCoreUnit);
    double  fSizeRatio;

    BOOL bRepaint = FALSE;
    USHORT nMask = 1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if(SVX_NUM_BITMAP == (aNumFmt.GetNumType()&(~LINK_TOKEN)))
            {
                Size aSize(aNumFmt.GetGraphicSize() );
                Size aSaveSize(aSize);

                if (aInitSize[i].Height())
                    fSizeRatio = (double)aInitSize[i].Width() / (double)aInitSize[i].Height();
                else
                    fSizeRatio = (double)1;

                if(bWidth)
                {
                    long nDelta = nWidthVal - aInitSize[i].Width();
                    aSize.Width() = nWidthVal;
                    if (bRatio)
                    {
                        aSize.Height() = aInitSize[i].Height() + (long)((double)nDelta / fSizeRatio);
                        aHeightMF.SetUserValue(aHeightMF.Normalize(
                            OutputDevice::LogicToLogic( aSize.Height(), (MapUnit)eCoreUnit, MAP_100TH_MM )),
                                FUNIT_100TH_MM);
                    }
                }
                else
                {
                    long nDelta = nHeightVal - aInitSize[i].Height();
                    aSize.Height() = nHeightVal;
                    if (bRatio)
                    {
                        aSize.Width() = aInitSize[i].Width() + (long)((double)nDelta * fSizeRatio);
                        aWidthMF.SetUserValue(aWidthMF.Normalize(
                            OutputDevice::LogicToLogic( aSize.Width(), (MapUnit)eCoreUnit, MAP_100TH_MM )),
                                FUNIT_100TH_MM);
                    }
                }
                const SvxBrushItem* pBrushItem =  aNumFmt.GetBrush();
                SvxFrameVertOrient eOrient = aNumFmt.GetVertOrient();
                if(aSize != aSaveSize)
                    bRepaint = TRUE;
                aNumFmt.SetGraphicBrush( pBrushItem, &aSize, &eOrient );
                pActNum->SetLevel(i, aNumFmt);
            }
        }
        nMask <<= 1;
    }
    SetModified(bRepaint);
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SvxNumOptionsTabPage, RatioHdl_Impl, CheckBox *, pBox )
{
    if (pBox->IsChecked())
    {
        if (bLastWidthModified)
            SizeHdl_Impl(&aWidthMF);
        else
            SizeHdl_Impl(&aHeightMF);
    }
    return 0;
}

/*-----------------02.12.97 16:07-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, CharFmtHdl_Impl, ListBox *, EMPTYARG )
{
    bAutomaticCharStyles = FALSE;
    USHORT nEntryPos = aCharFmtLB.GetSelectEntryPos();
    String sEntry = aCharFmtLB.GetSelectEntry();
    USHORT nMask = 1;
    String aEmptyStr;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if( 0 == nEntryPos )
                aNumFmt.SetCharFmt(aEmptyStr);
            else
            {
                if(SVX_NUM_BITMAP != (aNumFmt.GetNumType()&(~LINK_TOKEN)))
                    aNumFmt.SetCharFmt(sEntry);
            }
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified(FALSE);
    return 0;

};

/*-----------------03.12.97 11:01-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumOptionsTabPage, EditModifyHdl_Impl, Edit *, pEdit )
{
    BOOL bPrefix = pEdit == &aPrefixED;
    BOOL bSuffix = pEdit == &aSuffixED;
    BOOL bStart = pEdit == &aStartED;
    USHORT nMask = 1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));
            if(bPrefix)
                aNumFmt.SetPrefix( aPrefixED.GetText() );
            else if(bSuffix)
                aNumFmt.SetSuffix( aSuffixED.GetText() );
            else if(bStart)
                aNumFmt.SetStart( (USHORT)aStartED.GetValue() );
            else //align
            {
                USHORT nPos = aAlignLB.GetSelectEntryPos();
                SvxAdjust eAdjust = SVX_ADJUST_CENTER;
                if(nPos == 0)
                    eAdjust = SVX_ADJUST_LEFT;
                else if(nPos == 2)
                    eAdjust = SVX_ADJUST_RIGHT;
                aNumFmt.SetNumAdjust( eAdjust );
            }
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();

    return 0;
}

/*-----------------02.12.97 11:38-------------------

--------------------------------------------------*/
IMPL_STATIC_LINK( SvxNumOptionsTabPage, GraphicArrivedHdl_Impl,
                    SvxBrushItem*, pItem )
{
    PopupMenu* pPopup = pThis->aBitmapMB.GetPopupMenu()->GetPopupMenu( MN_GALLERY );

    SvxBmpItemInfo* pBmpInfo = 0;
    for ( USHORT i = 0; i < pThis->aGrfBrushItems.Count(); i++ )
    {
        SvxBmpItemInfo* pInfo = (SvxBmpItemInfo*)pThis->aGrfBrushItems.GetObject(i);
        if( pInfo->pBrushItem == pItem )
        {
            pBmpInfo = pInfo;
            break;
        }
    }
    if(pBmpInfo)
    {
        Image aImage( pItem->GetGraphic()->GetBitmap() );
        pPopup->SetItemImage( pBmpInfo->nItemId, aImage );
    }

    return 0;
}

/*-----------------09.12.97 11:49-------------------

--------------------------------------------------*/
USHORT lcl_DrawGraphic(VirtualDevice* pVDev, const SvxNumberFormat &rFmt, USHORT nXStart,
                        USHORT nYStart, USHORT nDivision)
{
    const SvxBrushItem* pBrushItem = rFmt.GetBrush();
    USHORT nRet = 0;
    if(pBrushItem)
    {
        const Graphic* pGrf = pBrushItem->GetGraphic();
        if(pGrf)
        {
            Size aGSize( rFmt.GetGraphicSize() );
            aGSize.Width() /= nDivision;
            nRet = (USHORT)aGSize.Width();
            aGSize.Height() /= nDivision;
            pGrf->Draw( pVDev, Point(nXStart,nYStart),
                    pVDev->PixelToLogic( aGSize ) );
        }
    }
    return nRet;

}

/*-----------------09.12.97 11:54-------------------

--------------------------------------------------*/
USHORT lcl_DrawBullet(VirtualDevice* pVDev,
            const SvxNumberFormat& rFmt, USHORT nXStart,
            USHORT nYStart, const Size& rSize)
{
    Font aTmpFont(pVDev->GetFont());

    //per Uno kann es sein, dass kein Font gesetzt ist!
    Font aFont(rFmt.GetBulletFont() ? *rFmt.GetBulletFont() : aTmpFont);
    Size aTmpSize(rSize);
    aTmpSize.Width() *= rFmt.GetBulletRelSize();
    aTmpSize.Width() /= 100 ;
    aTmpSize.Height() *= rFmt.GetBulletRelSize();
    aTmpSize.Height() /= 100 ;
    // bei einer Hoehe von Null wird in Ursprungshoehe gezeichnet
    if(!aTmpSize.Height())
        aTmpSize.Height() = 1;
    aFont.SetSize(aTmpSize);
    aFont.SetTransparent(TRUE);
    aFont.SetColor(rFmt.GetBulletColor());
    pVDev->SetFont( aFont );
    String aText(sal_Unicode(rFmt.GetBulletChar()));
    long nY = nYStart;
    nY -= ((aTmpSize.Height() - rSize.Height())/ 2);
    pVDev->DrawText( Point(nXStart, nY), aText );
    USHORT nRet = (USHORT)pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

/*-----------------02.12.97 10:34-------------------
    Vorschau der Numerierung painten
--------------------------------------------------*/
void    SvxNumberingPreview::Paint( const Rectangle& rRect )
{
    Size aSize(PixelToLogic(GetOutputSizePixel()));
    Rectangle aRect(Point(0,0), aSize);

    VirtualDevice* pVDev = new VirtualDevice(*this);
    pVDev->SetMapMode(GetMapMode());
    pVDev->SetOutputSize( aSize );


    pVDev->SetFillColor( Color( COL_WHITE ) );
    pVDev->DrawRect(aRect);

    if(pActNum)
    {
        USHORT nWidthRelation;
        if(nPageWidth)
        {
            nWidthRelation = USHORT (nPageWidth / aSize.Width());
            if(bPosition)
                nWidthRelation = nWidthRelation * 2 / 3;
            else
                nWidthRelation = nWidthRelation / 4;
        }
        else
            nWidthRelation = 30; // Kapiteldialog

        //Hoehe pro Ebene
        USHORT nXStep = aSize.Width() / (3 * pActNum->GetLevelCount());
        if(pActNum->GetLevelCount() < 10)
            nXStep /= 2;
        USHORT nYStart = 4;
        // fuer ein einziges Level darf nicht die gesamte Hoehe benutzt werden
        USHORT nYStep = (aSize.Height() - 6)/ (pActNum->GetLevelCount() > 1 ? pActNum->GetLevelCount() : 5);
        aStdFont = System::GetStandardFont(STDFONT_SWISS);

        //
        USHORT nFontHeight = nYStep * 6 / 10;
        if(bPosition)
            nFontHeight = nYStep * 15 / 10;
        aStdFont.SetSize(Size( 0, nFontHeight ));

        SvxNodeNum aNum( (BYTE)0 );
        USHORT nPreNum = pActNum->GetLevel(0).GetStart();

        if(bPosition)
        {
            USHORT nLineHeight = nFontHeight * 8 / 7;
            BYTE nStart = 0;
            while( !(nActLevel & (1<<nStart)) )
            {
                nStart++;
            }
            if(nStart)
                nStart--;
            BYTE nEnd = std::min((USHORT)(nStart + 3), pActNum->GetLevelCount());
            for( BYTE nLevel = nStart; nLevel < nEnd; ++nLevel )
            {
                const SvxNumberFormat &rFmt = pActNum->GetLevel(nLevel);
                aNum.GetLevelVal()[ nLevel ] = rFmt.GetStart();
                USHORT nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                short nTextOffset = rFmt.GetCharTextDistance() / nWidthRelation;
                USHORT nNumberXPos = nXStart;
                USHORT nFirstLineOffset = (-rFmt.GetFirstLineOffset()) / nWidthRelation;

                if(nFirstLineOffset <= nNumberXPos)
                    nNumberXPos -= nFirstLineOffset;
                else
                    nNumberXPos = 0;

                USHORT nBulletWidth = 0;
                //im draw ist das zulaeesig
                if(nTextOffset < 0)
                    nNumberXPos += nTextOffset;
                if( SVX_NUM_BITMAP == (rFmt.GetNumType() &(~LINK_TOKEN)))
                {
                    nBulletWidth = rFmt.IsShowSymbol() ? lcl_DrawGraphic(pVDev, rFmt,
                                        nNumberXPos,
                                            nYStart, nWidthRelation) : 0;
                }
                else if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumType() )
                {
                    nBulletWidth =  rFmt.IsShowSymbol() ?
                     lcl_DrawBullet(pVDev, rFmt, nNumberXPos, nYStart, aStdFont.GetSize()) : 0;
                }
                else
                {
                    pVDev->SetFont(aStdFont);
                    aNum.SetLevel( nLevel );
                    if(pActNum->IsContinuousNumbering())
                        aNum.GetLevelVal()[nLevel] = nPreNum;
                    String aText(pActNum->MakeNumString( aNum ));
                    Font aSaveFont = pVDev->GetFont();
                    Font aColorFont(aSaveFont);
                    aColorFont.SetColor(rFmt.GetBulletColor());
                    pVDev->SetFont(aColorFont);
                    pVDev->DrawText( Point(nNumberXPos, nYStart), aText );
                    pVDev->SetFont(aSaveFont);
                    nBulletWidth = (USHORT)pVDev->GetTextWidth(aText);
                    nPreNum++;
                }

                USHORT nTextXPos = nXStart;
                if(nTextOffset < 0)
                     nTextXPos += nTextOffset;
                if(nNumberXPos + nBulletWidth + nTextOffset > nTextXPos )
                    nTextXPos = nNumberXPos + nBulletWidth + nTextOffset;

                Rectangle aRect1(Point(nTextXPos, nYStart + nFontHeight / 2), Size(aSize.Width() / 2, 2));
                pVDev->SetFillColor( Color( COL_BLACK ) );
                pVDev->DrawRect( aRect1 );

                Rectangle aRect2(Point(nXStart, nYStart + nLineHeight + nFontHeight / 2 ), Size(aSize.Width() / 2, 2));
                pVDev->DrawRect( aRect2 );
                nYStart += 2 * nLineHeight;
            }
        }
        else
        {
            USHORT nLineHeight = nFontHeight * 3 / 2;
            for( BYTE nLevel = 0; nLevel < pActNum->GetLevelCount();
                            ++nLevel, nYStart += nYStep )
            {
                const SvxNumberFormat &rFmt = pActNum->GetLevel(nLevel);
                aNum.GetLevelVal()[ nLevel ] = rFmt.GetStart();
                USHORT nXStart = (rFmt.GetAbsLSpace() / nWidthRelation) / 2 + 2;
                USHORT nTextOffset = 2 * nXStep;
                if( SVX_NUM_BITMAP == (rFmt.GetNumType()&(~LINK_TOKEN)) )
                {
                    if(rFmt.IsShowSymbol())
                    {
                        nTextOffset = lcl_DrawGraphic(pVDev, rFmt, nXStart, nYStart, nWidthRelation);
                        nTextOffset += nXStep;
                    }
                }
                else if( SVX_NUM_CHAR_SPECIAL == rFmt.GetNumType() )
                {
//                  aNum.GetLevelVal()[ nLevel ] = 0;
                    if(rFmt.IsShowSymbol())
                    {
                        nTextOffset =  lcl_DrawBullet(pVDev, rFmt, nXStart, nYStart, aStdFont.GetSize());
                        nTextOffset += nXStep;
                    }
                }
                else
                {
                    Font aColorFont(aStdFont);
                    aColorFont.SetColor(rFmt.GetBulletColor());
                    pVDev->SetFont(aColorFont);
                    aNum.SetLevel( nLevel );
                    if(pActNum->IsContinuousNumbering())
                        aNum.GetLevelVal()[nLevel] = nPreNum;
                    String aText(pActNum->MakeNumString( aNum ));
                    pVDev->DrawText( Point(nXStart, nYStart), aText );
                    pVDev->SetFont(aStdFont);
                    nTextOffset = (USHORT)pVDev->GetTextWidth(aText);
                    nTextOffset += nXStep;
                    nPreNum++;
                }
                pVDev->SetFont(aStdFont);
                String sMsg( RTL_CONSTASCII_USTRINGPARAM( "StarOffice") );
                if(pOutlineNames)
                    sMsg = pOutlineNames[nLevel];
                pVDev->DrawText( Point(nXStart + nTextOffset, nYStart), sMsg );
            }
        }
    }
    DrawOutDev( Point(0,0), aSize,
                Point(0,0), aSize,
                        *pVDev );
    delete pVDev;

}

/*-----------------02.12.97 12:55-------------------

--------------------------------------------------*/
NumMenuButton::~NumMenuButton()
{
}

/*-----------------02.12.97 12:55-------------------

--------------------------------------------------*/
void NumMenuButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if(maClickHdl_Impl.IsSet())
        maClickHdl_Impl.Call(this);
    MenuButton::MouseButtonDown( rMEvt );
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SvxNumPositionTabPage::SvxNumPositionTabPage(Window* pParent,
                               const SfxItemSet& rSet) :
    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_NUM_POSITION ), rSet ),
    aLevelGB(       this, ResId(GB_LEVEL    )),
    aLevelLB(       this, ResId(LB_LEVEL    )),
    aPositionGB(    this, ResId(GB_POSITION )),
    aAlignFT(       this, ResId(FT_ALIGN    )),
    aAlignLB(       this, ResId(LB_ALIGN    )),
    aDistBorderFT(  this, ResId(FT_BORDERDIST   )),
    aDistBorderMF(  this, ResId(MF_BORDERDIST   )),
    aDistNumFT(     this, ResId(FT_NUMDIST      )),
    aDistNumMF(     this, ResId(MF_NUMDIST      )),
    aRelativeCB(    this, ResId(CB_RELATIVE     )),
    aIndentFT(      this, ResId(FT_INDENT       )),
    aIndentMF(      this, ResId(MF_INDENT       )),
    aStandardPB(    this, ResId(PB_STANDARD     )),
    aPreviewGB(     this, ResId(GB_PREVIEW      )),
    pPreviewWIN(    new SvxNumberingPreview(this, ResId(WIN_PREVIEW ))),
    bInInintControl(FALSE),
    nActNumLvl( USHRT_MAX ),
    pActNum(0),
    pSaveNum(0),
    nNumItemId(SID_ATTR_NUMBERING_RULE)
{
    FreeResource();
    SetExchangeSupport();
    pPreviewWIN->SetBackground(Wallpaper(Color(COL_TRANSPARENT)));

    aRelativeCB.Check();
    aAlignLB.SetSelectHdl(LINK(this, SvxNumPositionTabPage, EditModifyHdl_Impl));

    Link aLk = LINK(this, SvxNumPositionTabPage, DistanceHdl_Impl);

    aDistBorderMF.SetUpHdl(aLk);
    aDistBorderMF.SetDownHdl(aLk);
    aDistBorderMF.SetLoseFocusHdl(aLk);

    aDistNumMF.SetUpHdl(aLk);
    aDistNumMF.SetDownHdl(aLk);
    aDistNumMF.SetLoseFocusHdl(aLk);

    aIndentMF.SetUpHdl(aLk);
    aIndentMF.SetDownHdl(aLk);
    aIndentMF.SetLoseFocusHdl(aLk);

    aLevelLB.SetSelectHdl(LINK(this, SvxNumPositionTabPage, LevelHdl_Impl));
    aRelativeCB.SetClickHdl(LINK(this, SvxNumPositionTabPage, RelativeHdl_Impl));
    aStandardPB.SetClickHdl(LINK(this, SvxNumPositionTabPage, StandardHdl_Impl));


    aRelativeCB.Check(bLastRelative);
    pPreviewWIN->SetPositionMode();
    eCoreUnit = rSet.GetPool()->GetMetric(rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE));

#ifdef DEBUG
    pDebugFixedText = new FixedText(this, 0);
    pDebugFixedText->Show();
    Size aSize(200, 20);
    Point aPos(250,0);

    pDebugFixedText->SetPosSizePixel(aPos, aSize);
    pDebugFixedText->SetText( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "Das ist ein Debug-Text" ) ) );
#endif
}
/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SvxNumPositionTabPage::~SvxNumPositionTabPage()
{
    delete pActNum;
    delete pPreviewWIN;
    delete pSaveNum;
#ifdef DEBUG
    delete pDebugFixedText;
#endif
}
/*-------------------------------------------------------*/

#ifdef DEBUG
void lcl_PrintDebugOutput(FixedText& rFixed, const SvxNumberFormat& rNumFmt)
{
#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

    sal_Char const sHash[] = " # ";
    String sDebugText( UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetAbsLSpace() ) ) );
    sDebugText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sHash ) );
    sDebugText += UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetCharTextDistance() ) );
    sDebugText.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sHash ) );
    sDebugText += UniString::CreateFromInt32( TWIP_TO_MM100(rNumFmt.GetFirstLineOffset() ) );
    rFixed.SetText(sDebugText);
}
#endif

/*-----------------03.12.97 10:06-------------------

--------------------------------------------------*/
void SvxNumPositionTabPage::InitControls()
{
    bInInintControl = TRUE;
    BOOL bRelative = aRelativeCB.IsEnabled() && aRelativeCB.IsChecked();
//  SetMinDist();
    BOOL bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
    aDistBorderMF.Enable(bSingleSelection || bRelative );
    aDistBorderFT.Enable(bSingleSelection || bRelative );
    //HACK("der Wert sollte mal sinnvol gesetzt werden")
    long nWidth = 10000;
    nWidth = OutputDevice::LogicToLogic( nWidth,
                                            (MapUnit)eCoreUnit, MAP_100TH_MM );

    aDistBorderMF.SetMax(aDistBorderMF.Normalize( nWidth ), FUNIT_100TH_MM );
    aDistNumMF   .SetMax(aDistNumMF   .Normalize( nWidth ), FUNIT_100TH_MM );

    aIndentMF    .SetMax(aIndentMF    .Normalize( nWidth ), FUNIT_100TH_MM );
    long nLast2 = nWidth /2;
    aDistBorderMF.SetLast( aDistBorderMF.Normalize(   nLast2 ), FUNIT_100TH_MM );
    aDistNumMF   .SetLast( aDistNumMF     .Normalize( nLast2 ), FUNIT_100TH_MM );
    aIndentMF    .SetLast( aIndentMF      .Normalize( nLast2 ), FUNIT_100TH_MM );


    BOOL bSetDistEmpty = FALSE;
//  BOOL bSameDistBorder= TRUE;
    BOOL bSameDistBorderNum = TRUE;
    BOOL bSameDist      = TRUE;
    BOOL bSameIndent    = TRUE;
    BOOL bSameAdjust    = TRUE;

    const SvxNumberFormat* aNumFmtArr[SVX_MAX_NUM];
//  SvxFrameVertOrient eFirstOrient = SVX_VERT_NONE;
    USHORT nMask = 1;
    USHORT nLvl = USHRT_MAX;
//  long nFirstLSpace = 0;
    long nFirstBorderText = 0;
    long nFirstBorderTextRelative = -1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        aNumFmtArr[i] = &pActNum->GetLevel(i);
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nLvl)
            {
                nLvl = i;
                nFirstBorderText = nLvl > 0 ?
                    aNumFmtArr[nLvl]->GetAbsLSpace() + aNumFmtArr[nLvl]->GetFirstLineOffset() -
                    aNumFmtArr[nLvl - 1]->GetAbsLSpace() + aNumFmtArr[nLvl - 1]->GetFirstLineOffset():
                        aNumFmtArr[nLvl]->GetAbsLSpace() + aNumFmtArr[nLvl]->GetFirstLineOffset();

//              nFirstLSpace = nLvl > 0 ?
//                  aNumFmtArr[nLvl]->GetAbsLSpace() - aNumFmtArr[nLvl - 1]->GetAbsLSpace():
//                      aNumFmtArr[nLvl]->GetAbsLSpace();
            }

            if( i > nLvl)
            {
                if(bRelative)
                {
                    if(nFirstBorderTextRelative == -1)
                        nFirstBorderTextRelative =
                        (aNumFmtArr[i]->GetAbsLSpace() + aNumFmtArr[i]->GetFirstLineOffset() -
                        aNumFmtArr[i - 1]->GetAbsLSpace() + aNumFmtArr[i - 1]->GetFirstLineOffset());
                    else
                        bSameDistBorderNum &= nFirstBorderTextRelative ==
                        (aNumFmtArr[i]->GetAbsLSpace() + aNumFmtArr[i]->GetFirstLineOffset() -
                        aNumFmtArr[i - 1]->GetAbsLSpace() + aNumFmtArr[i - 1]->GetFirstLineOffset());

//                  bSameDistBorder &= nFirstLSpace == (aNumFmtArr[i]->GetAbsLSpace() - aNumFmtArr[i - 1]->GetAbsLSpace());
                }
                else
                    bSameDistBorderNum &=
                    aNumFmtArr[i]->GetAbsLSpace() - aNumFmtArr[i]->GetFirstLineOffset() ==
                    aNumFmtArr[i - 1]->GetAbsLSpace() - aNumFmtArr[i - 1]->GetFirstLineOffset();

//                  bSameDistBorder &= aNumFmtArr[i]->GetAbsLSpace() == aNumFmtArr[nLvl]->GetAbsLSpace();

                bSameDist       &= aNumFmtArr[i]->GetCharTextDistance() == aNumFmtArr[nLvl]->GetCharTextDistance();
                bSameIndent     &= aNumFmtArr[i]->GetFirstLineOffset() == aNumFmtArr[nLvl]->GetFirstLineOffset();
                bSameAdjust     &= aNumFmtArr[i]->GetNumAdjust() == aNumFmtArr[nLvl]->GetNumAdjust();

            }
        }
//          else
//              aNumFmtArr[i] = 0;
        nMask <<= 1;

    }
    if(bSameDistBorderNum)
//  if(bSameDistBorder)
    {
        long nDistBorderNum;
        if(bRelative)
        {
            nDistBorderNum = (long)aNumFmtArr[nLvl]->GetAbsLSpace()+ aNumFmtArr[nLvl]->GetFirstLineOffset();
            if(nLvl)
                nDistBorderNum -= (long)aNumFmtArr[nLvl - 1]->GetAbsLSpace()+ aNumFmtArr[nLvl - 1]->GetFirstLineOffset();
        }
        else
        {
            nDistBorderNum = (long)aNumFmtArr[nLvl]->GetAbsLSpace()+ aNumFmtArr[nLvl]->GetFirstLineOffset();
        }
        SetMetricValue(aDistBorderMF, nDistBorderNum, eCoreUnit);
//  so war es früher Abstand Text<->Rand
//      SetMetricValue(aDistBorderMF,
//          bRelative ? (long)nFirstLSpace : (long)aNumFmtArr[nLvl]->GetAbsLSpace(),
//                                                      eCoreUnit);
    }
    else
        bSetDistEmpty = TRUE;

#ifdef DEBUG
    lcl_PrintDebugOutput(*pDebugFixedText, *aNumFmtArr[nLvl]);
#endif

    String aEmptyStr;
    if(bSameDist)
        SetMetricValue(aDistNumMF, aNumFmtArr[nLvl]->GetCharTextDistance(), eCoreUnit);
    else
        aDistNumMF.SetText(aEmptyStr);
    if(bSameIndent)
        SetMetricValue(aIndentMF, - aNumFmtArr[nLvl]->GetFirstLineOffset(), eCoreUnit);
    else
        aIndentMF.SetText(aEmptyStr);

    if(bSameAdjust)
    {
        USHORT nPos = 1; // zentriert
        if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_LEFT)
            nPos = 0;
        else if(aNumFmtArr[nLvl]->GetNumAdjust() == SVX_ADJUST_RIGHT)
            nPos = 2;
        aAlignLB.SelectEntryPos(nPos);
    }
    else
    {
        aAlignLB.SetNoSelection();
    }

    if(TRUE == bSetDistEmpty)
        aDistBorderMF.SetText(aEmptyStr);

    bInInintControl = FALSE;
}
/*-----------------03.12.97 12:21-------------------

--------------------------------------------------*/
/*
void SvxNumPositionTabPage::SetMinDist()
{
    // JP 03.04.97: Bug 32903 - MinWert fuer DistBorderMF setzen

    // ggfs. den akt. NumLevel anpassen
    USHORT nStart = 0;
    USHORT nEnd = pActNum->GetLevelCount();
    USHORT nMask = 1;
    USHORT nTmpLvl = USHRT_MAX;
    long nMinVal = 0;
    BOOL bInit = FALSE;

    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            if(USHRT_MAX == nTmpLvl)
            {
                nTmpLvl = i;
                const SvxNumberFormat& rNumFmt = pActNum->GetLevel( nTmpLvl );

                nMinVal = - rNumFmt.GetFirstLineOffset();
                if( nTmpLvl )
                    nMinVal -= pActNum->GetLevel( nTmpLvl - 1 ).GetAbsLSpace();
            }
            const SvxNumberFormat& rAktNumFmt = pActNum->GetLevel( nStart );
            if( rAktNumFmt.GetAbsLSpace() < nMinVal )
            {
                bInit = TRUE;
                SvxNumberFormat aNumFmt( rAktNumFmt );
                aNumFmt.SetAbsLSpace( (USHORT)nMinVal );
                pActNum->SetLevel( nStart, aNumFmt );
            }
        }
        nMask <<=1;
    }

    if(!aRelativeCB.IsChecked() || !aRelativeCB.IsEnabled())
        nMinVal = 0;
    nMinVal = aDistBorderMF.Normalize( nMinVal );
    nMinVal = OutputDevice::LogicToLogic(nMinVal, (MapUnit)eCoreUnit, MAP_100TH_MM);
    aDistBorderMF.SetMin( nMinVal, FUNIT_100TH_MM );
    aDistBorderMF.SetFirst( nMinVal, FUNIT_100TH_MM );
    if(bInit)
        InitControls();
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
void SvxNumPositionTabPage::ActivatePage(const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    UINT16 nTmpNumLvl = USHRT_MAX;
    const SfxItemSet* pExampleSet = GetTabDialog()->GetExampleSet();
    if(pExampleSet)
    {
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_NUM_PRESET, FALSE, &pItem))
            bPreset = ((const SfxBoolItem*)pItem)->GetValue();
        if(SFX_ITEM_SET == pExampleSet->GetItemState(SID_PARAM_CUR_NUM_LEVEL, FALSE, &pItem))
            nTmpNumLvl = ((const SfxUInt16Item*)pItem)->GetValue();
    }
    //
    if(SFX_ITEM_SET == rSet.GetItemState(nNumItemId, FALSE, &pItem))
    {
        delete pSaveNum;
        pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());
    }
    bModified = (!pActNum->Get( 0 ) || bPreset);
    if(*pSaveNum != *pActNum ||
        nActNumLvl != nTmpNumLvl )
    {
        *pActNum = *pSaveNum;
        nActNumLvl = nTmpNumLvl;
        USHORT nMask = 1;
        aLevelLB.SetUpdateMode(FALSE);
        aLevelLB.SetNoSelection();
        aLevelLB.SelectEntryPos( pActNum->GetLevelCount(), nActNumLvl == USHRT_MAX);
        if(nActNumLvl != USHRT_MAX)
            for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
            {
                if(nActNumLvl & nMask)
                    aLevelLB.SelectEntryPos( i, TRUE);
                nMask <<= 1 ;
            }
        aRelativeCB.Enable(nActNumLvl != 1);
        aLevelLB.SetUpdateMode(TRUE);
        InitControls();
    }
    pPreviewWIN->SetLevel(nActNumLvl);
    pPreviewWIN->Invalidate();
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
int  SvxNumPositionTabPage::DeactivatePage(SfxItemSet *pSet)
{
    if(pSet)
        FillItemSet(*pSet);
    return TRUE;
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
BOOL SvxNumPositionTabPage::FillItemSet( SfxItemSet& rSet )
{
    rSet.Put(SfxUInt16Item(SID_PARAM_CUR_NUM_LEVEL, nActNumLvl));
    //HACK("pOutlineDlg")
//  if(pOutlineDlg)
//      *pOutlineDlg->GetNumRule() = *pActNum;
//  else
    if(bModified && pActNum)
    {
        *pSaveNum = *pActNum;
        rSet.Put(SvxNumBulletItem( *pSaveNum ), nNumItemId);
        rSet.Put(SfxBoolItem(SID_PARAM_NUM_PRESET, FALSE));
    }
    return bModified;
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
void SvxNumPositionTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    //im Draw gibt es das Item als WhichId, im Writer nur als SlotId
    SfxItemState eState = rSet.GetItemState(SID_ATTR_NUMBERING_RULE, FALSE, &pItem);
    if(eState != SFX_ITEM_SET)
    {
        nNumItemId = rSet.GetPool()->GetWhich(SID_ATTR_NUMBERING_RULE);
        eState = rSet.GetItemState(nNumItemId, FALSE, &pItem);
    }
    DBG_ASSERT(eState == SFX_ITEM_SET, "kein Item gefunden!")
    delete pSaveNum;
    pSaveNum = new SvxNumRule(*((SvxNumBulletItem*)pItem)->GetNumRule());

    // Ebenen einfuegen
    if(!aLevelLB.GetEntryCount())
    {
        for(USHORT i = 1; i <= pSaveNum->GetLevelCount(); i++)
            aLevelLB.InsertEntry(UniString::CreateFromInt32(i));
        if(pSaveNum->GetLevelCount() > 1)
        {
            String sEntry( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "1 - ") ) );
            sEntry.Append( UniString::CreateFromInt32( pSaveNum->GetLevelCount() ) );
            aLevelLB.InsertEntry(sEntry);
            aLevelLB.SelectEntry(sEntry);
        }
        else
            aLevelLB.SelectEntryPos(0);
    }
    else
        aLevelLB.SelectEntryPos(aLevelLB.GetEntryCount() - 1);
//  nActNumLvl =
//      pOutlineDlg ? pOutlineDlg->GetActNumLevel() : 0;
//      ((SwNumBulletTabDialog*)GetTabDialog())->GetActNumLevel();
    USHORT nMask = 1;
    aLevelLB.SetUpdateMode(FALSE);
    aLevelLB.SetNoSelection();
    if(nActNumLvl == USHRT_MAX)
    {
        aLevelLB.SelectEntryPos( pSaveNum->GetLevelCount(), TRUE);
    }
    else
        for(USHORT i = 0; i < pSaveNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
                aLevelLB.SelectEntryPos( i, TRUE);
            nMask <<= 1;
        }
    aLevelLB.SetUpdateMode(TRUE);

    if(SFX_ITEM_SET == rSet.GetItemState(SID_PARAM_CHILD_LEVELS, FALSE, &pItem))
        bHasChild = ((const SfxBoolItem*)pItem)->GetValue();
    if(!pActNum)
        pActNum = new  SvxNumRule(*pSaveNum);
    else if(*pSaveNum != *pActNum)
        *pActNum = *pSaveNum;
    pPreviewWIN->SetNumRule(pActNum);
    //BOOL bTextDist = pActNum->IsFeatureSupported(NUM_CHAR_TEXT_DISTANCE);
    BOOL bDraw = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
    aDistNumFT.Show(bDraw);
    aDistNumMF.Show(bDraw);
    // das ist eigentlich Missbrauch, da fuer die vollst. Numerierung kein
    // eigenes Flag existiert
//  BOOL bContinous = pActNum->IsFeatureSupported(NUM_CONTINUOUS);
//  aIndentFT.Show(bContinous);
//  aIndentMF.Show(bContinous);
    // das ist eigentlich Missbrauch, da fuer die neg. Einrueckung kein
    // eigenes Flag existiert
//  if(!pActNum->IsFeatureSupported(NUM_CONTINUOUS))
//      aDistNumMF   .SetMin( -10000 );

    InitControls();
    bModified = FALSE;
}

/*-----------------03.12.97 10:02-------------------

--------------------------------------------------*/
SfxTabPage* SvxNumPositionTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SvxNumPositionTabPage(pParent, rAttrSet);
}

/*-----------------04.12.97 12:51-------------------

--------------------------------------------------*/
void    SvxNumPositionTabPage::SetMetric(FieldUnit eMetric)
{
    if(eMetric == FUNIT_MM)
    {
        aDistBorderMF .SetDecimalDigits(1);
        aDistNumMF    .SetDecimalDigits(1);
        aIndentMF     .SetDecimalDigits(1);
    }
    aDistBorderMF .SetUnit( eMetric );
    aDistNumMF    .SetUnit( eMetric );
    aIndentMF     .SetUnit( eMetric );
}
/*void SvxNumPositionTabPage::SetWrtShell(SwWrtShell* pSh)
{
    pWrtSh = pSh;
    const SwRect& rPrtRect = pWrtSh->GetAnyCurRect(RECT_PAGE);
    aPreviewWIN.SetPageWidth(rPrtRect.Width());
}

/*-----------------03.12.97 11:06-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumPositionTabPage, EditModifyHdl_Impl, Edit *, pEdit )
{

    USHORT nStart = 0;
    USHORT nEnd = pActNum->GetLevelCount();
    USHORT nMask = 1;
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt(pActNum->GetLevel(i));

            USHORT nPos = aAlignLB.GetSelectEntryPos();
            SvxAdjust eAdjust = SVX_ADJUST_CENTER;
            if(nPos == 0)
                eAdjust = SVX_ADJUST_LEFT;
            else if(nPos == 2)
                eAdjust = SVX_ADJUST_RIGHT;
            aNumFmt.SetNumAdjust( eAdjust );
            pActNum->SetLevel(i, aNumFmt);
        }
        nMask <<= 1;
    }
    SetModified();
    return 0;
}
/*-----------------03.12.97 11:11-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumPositionTabPage, LevelHdl_Impl, ListBox *, pBox )
{
    USHORT nSaveNumLvl = nActNumLvl;
    nActNumLvl = 0;
    if(pBox->IsEntryPosSelected( pActNum->GetLevelCount() ) &&
            (pBox->GetSelectEntryCount() == 1 || nSaveNumLvl != 0xffff))
    {
        nActNumLvl = 0xFFFF;
        pBox->SetUpdateMode(FALSE);
        for( USHORT i = 0; i < pActNum->GetLevelCount(); i++ )
            pBox->SelectEntryPos( i, FALSE );
        pBox->SetUpdateMode(TRUE);
    }
    else if(pBox->GetSelectEntryCount())
    {
        USHORT nMask = 1;
        for( USHORT i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(pBox->IsEntryPosSelected( i ))
                nActNumLvl |= nMask;
            nMask <<= 1;
        }
        pBox->SelectEntryPos( pActNum->GetLevelCount(), FALSE );
    }
    else
    {
        nActNumLvl = nSaveNumLvl;
        USHORT nMask = 1;
        for( USHORT i = 0; i < pActNum->GetLevelCount(); i++ )
        {
            if(nActNumLvl & nMask)
            {
                pBox->SelectEntryPos(i);
                break;
            }
            nMask <<=1;
        }
    }
    aRelativeCB.Enable(nActNumLvl != 1);
    SetModified();
    InitControls();
    return 0;
}
/*-----------------03.12.97 12:24-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumPositionTabPage, DistanceHdl_Impl, MetricField *, pFld )
{
    if(bInInintControl)
        return 0;
    long nValue = GetCoreValue(*pFld, eCoreUnit);
    USHORT nMask = 1;
#ifdef DEBUG
    BOOL bFirst = TRUE;
#endif
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel( i ) );
            if(pFld == &aDistBorderMF)
            {

                if(aRelativeCB.IsChecked())
                {
                    if(0 == i)
                    {
                        long nTmp = aNumFmt.GetFirstLineOffset();
                        aNumFmt.SetAbsLSpace( USHORT(nValue - nTmp));
                    }
                    else
                    {
                        long nTmp = pActNum->GetLevel( i - 1 ).GetAbsLSpace() +
                                    pActNum->GetLevel( i - 1 ).GetFirstLineOffset() -
                                    pActNum->GetLevel( i ).GetFirstLineOffset();

                        aNumFmt.SetAbsLSpace( USHORT(nValue + nTmp));
                    }
                }
                else
                {
                    aNumFmt.SetAbsLSpace( (short)nValue - aNumFmt.GetFirstLineOffset());
                }
            }
            else if(pFld == &aDistNumMF)
            {
//              long nLValue = GetCoreValue(aDistBorderMF, eCoreUnit);

//              if(nValue < (-nLValue))
//                  SetMetricValue(aDistNumMF, -nLValue, eCoreUnit);

                aNumFmt.SetCharTextDistance( (short)nValue );
            }
            else if(pFld == &aIndentMF)
            {
                //jetzt muss mit dem FirstLineOffset auch der AbsLSpace veraendert werden
                long nDiff = nValue + aNumFmt.GetFirstLineOffset();
                long nAbsLSpace = aNumFmt.GetAbsLSpace();
                aNumFmt.SetAbsLSpace(USHORT(nAbsLSpace + nDiff));
                aNumFmt.SetFirstLineOffset( -(short)nValue );
            }

#ifdef DEBUG
            if(bFirst)
                lcl_PrintDebugOutput(*pDebugFixedText, aNumFmt);
            bFirst = FALSE;
#endif
            pActNum->SetLevel( i, aNumFmt );
        }
        nMask <<= 1;
    }

    SetModified();
//  SetMinDist();
    if(!aDistBorderMF.IsEnabled())
    {
        String aEmptyStr;
        aDistBorderMF.SetText(aEmptyStr);
    }

    return 0;
}

/*-----------------04.12.97 12:35-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumPositionTabPage, RelativeHdl_Impl, CheckBox *, pBox )
{
    BOOL bOn = pBox->IsChecked();
//  SetMinDist();
    BOOL bSingleSelection = aLevelLB.GetSelectEntryCount() == 1 && USHRT_MAX != nActNumLvl;
    BOOL bSetValue = FALSE;
    long nValue = 0;
    if(bOn || bSingleSelection)
    {
        USHORT nMask = 1;
        BOOL bFirst = TRUE;
        bSetValue = TRUE;
        for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
        {
            if(nActNumLvl & nMask)
            {
                const SvxNumberFormat &rNumFmt = pActNum->GetLevel(i);
                if(bFirst)
                {
                    nValue = rNumFmt.GetAbsLSpace() + rNumFmt.GetFirstLineOffset();
                    if(bOn && i)
                        nValue -= (pActNum->GetLevel(i - 1).GetAbsLSpace() + pActNum->GetLevel(i - 1).GetFirstLineOffset());
                }
                else
                    bSetValue = nValue ==
                        (rNumFmt.GetAbsLSpace() + rNumFmt.GetFirstLineOffset()) -
                            (pActNum->GetLevel(i - 1).GetAbsLSpace() + pActNum->GetLevel(i - 1).GetFirstLineOffset());
                bFirst = FALSE;
            }
            nMask <<= 1;
        }

    }
    String aEmptyStr;
    if(bSetValue)
        SetMetricValue(aDistBorderMF, nValue,   eCoreUnit);
    else
        aDistBorderMF.SetText(aEmptyStr);
    aDistBorderMF.Enable(bOn || bSingleSelection);
    aDistBorderFT.Enable(bOn || bSingleSelection);
    bLastRelative = bOn;
    return 0;
}
/*-----------------05.12.97 15:33-------------------

--------------------------------------------------*/
IMPL_LINK( SvxNumPositionTabPage, StandardHdl_Impl, PushButton *, EMPTYARG )
{
    USHORT nMask = 1;
    SvxNumRule aTmpNumRule(pActNum->GetFeatureFlags(), pActNum->GetLevelCount(), pActNum->IsContinuousNumbering());
    for(USHORT i = 0; i < pActNum->GetLevelCount(); i++)
    {
        if(nActNumLvl & nMask)
        {
            SvxNumberFormat aNumFmt( pActNum->GetLevel( i ) );
            SvxNumberFormat aTempFmt(aTmpNumRule.GetLevel( i ));
            aNumFmt.SetAbsLSpace( aTempFmt.GetAbsLSpace());
            aNumFmt.SetCharTextDistance( aTempFmt.GetCharTextDistance() );
            aNumFmt.SetFirstLineOffset( aTempFmt.GetFirstLineOffset() );

            pActNum->SetLevel( i, aNumFmt );
        }
        nMask <<= 1;
    }

    InitControls();
    SetModified();
    return 0;
}

void SvxNumPositionTabPage::SetModified(BOOL bRepaint)
{
    bModified = TRUE;
    if(bRepaint)
    {
        pPreviewWIN->SetLevel(nActNumLvl);
        pPreviewWIN->Invalidate();
    }
}

void SvxNumOptionsTabPage::SetModified(BOOL bRepaint)
{
    bModified = TRUE;
    if(bRepaint)
    {
        pPreviewWIN->SetLevel(nActNumLvl);
        pPreviewWIN->Invalidate();
    }
}

