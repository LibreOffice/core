/*************************************************************************
 *
 *  $RCSfile: scuitphfedit.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 14:26:45 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#define _TPHFEDIT_CXX
#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

//CHINA001 #include <svx/chardlg.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/editview.hxx>
#include <svx/flditem.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <svtools/useroptions.hxx>
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

//CHINA001 #include "tphfedit.hxx"
#include "editutil.hxx"
#include "global.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "prevwsh.hxx"
#include "hfedtdlg.hrc"
#include "textdlgs.hxx"

#ifndef _SC_ACCESSIBLEEDITOBJECT_HXX
#include "AccessibleEditObject.hxx"
#endif

#include "scuitphfedit.hxx" //CHINA001
#include <memory> // header file for auto_ptr

// STATIC DATA -----------------------------------------------------------

static USHORT pPageRightHeaderRanges[] = { SID_SCATTR_PAGE_HEADERRIGHT,
                                           SID_SCATTR_PAGE_HEADERRIGHT,
                                           0 };

static USHORT pPageRightFooterRanges[] = { SID_SCATTR_PAGE_FOOTERRIGHT,
                                           SID_SCATTR_PAGE_FOOTERRIGHT,
                                           0 };

static USHORT pPageLeftHeaderRanges[] =  { SID_SCATTR_PAGE_HEADERLEFT,
                                           SID_SCATTR_PAGE_HEADERLEFT,
                                           0 };

static USHORT pPageLeftFooterRanges[] =  { SID_SCATTR_PAGE_FOOTERLEFT,
                                           SID_SCATTR_PAGE_FOOTERLEFT,
                                           0 };


static ScEditWindow* pActiveEdWnd = NULL;


//========================================================================
// class ScHFEditPage
//

ScHFEditPage::ScHFEditPage( Window*             pParent,
                            USHORT              nResId,
                            const SfxItemSet&   rCoreAttrs,
                            USHORT              nWhichId,
                            bool bHeader  )

    :   SfxTabPage      ( pParent, ScResId( nResId ), rCoreAttrs ),

        aWndLeft        ( this, ScResId( WND_LEFT ), Left ),
        aWndCenter      ( this, ScResId( WND_CENTER ), Center ),
        aWndRight       ( this, ScResId( WND_RIGHT ), Right ),
        aFtLeft         ( this, ScResId( FT_LEFT ) ),
        aFtCenter       ( this, ScResId( FT_CENTER ) ),
        aFtRight        ( this, ScResId( FT_RIGHT ) ),
        maFtDefinedHF       ( this, ScResId( FT_HF_DEFINED ) ),
        maLbDefined     ( this, ScResId( LB_DEFINED ) ),
        maFtCustomHF        ( this, ScResId( FT_HF_CUSTOM ) ),
        aFlInfo         ( this, ScResId( FL_INFO ) ),
        aFtInfo         ( this, ScResId( FT_INFO ) ),
        aBtnText        ( this, ScResId( BTN_TEXT ) ),
        aBtnPage        ( this, ScResId( BTN_PAGE ) ),
        aBtnLastPage    ( this, ScResId( BTN_PAGES ) ),
        aBtnDate        ( this, ScResId( BTN_DATE ) ),
        aBtnTime        ( this, ScResId( BTN_TIME ) ),
        aBtnFile        ( this, ScResId( BTN_FILE ) ),
        aBtnTable       ( this, ScResId( BTN_TABLE ) ),
        aPopUpFile      ( ScResId( RID_POPUP_FCOMMAND) ),
        nWhich          ( nWhichId )
{
    //! use default style from current document?
    //! if font color is used, header/footer background color must be set

    ScPatternAttr   aPatAttr( rCoreAttrs.GetPool() );


    aBtnFile.SetPopupMenu(&aPopUpFile);

    maLbDefined.SetSelectHdl( LINK( this, ScHFEditPage, ListHdl_Impl ) );
    aBtnFile.SetMenuHdl( LINK( this, ScHFEditPage, MenuHdl ) );
    aBtnText    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnPage    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnLastPage.SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnDate    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnTime    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnFile    .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );
    aBtnTable   .SetClickHdl( LINK( this, ScHFEditPage, ClickHdl ) );

    aBtnText    .SetModeImage( Image( ScResId( IMG_TEXT_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnFile    .SetModeImage( Image( ScResId( IMG_FILE_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnTable   .SetModeImage( Image( ScResId( IMG_TABLE_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnPage    .SetModeImage( Image( ScResId( IMG_PAGE_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnLastPage.SetModeImage( Image( ScResId( IMG_PAGES_H ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnDate    .SetModeImage( Image( ScResId( IMG_DATE_H  ) ), BMP_COLOR_HIGHCONTRAST );
    aBtnTime    .SetModeImage( Image( ScResId( IMG_TIME_H  ) ), BMP_COLOR_HIGHCONTRAST );

    if(!bHeader)
    {
        maFtDefinedHF.SetText(ScGlobal::GetRscString( STR_FOOTER ));
        maFtCustomHF.SetText(ScGlobal::GetRscString( STR_HF_CUSTOM_FOOTER ));
    }
    aWndLeft.   SetFont( aPatAttr );
    aWndCenter. SetFont( aPatAttr );
    aWndRight.  SetFont( aPatAttr );

    FillCmdArr();

    aWndLeft.GrabFocus();

    InitPreDefinedList();

    FreeResource();
}

// -----------------------------------------------------------------------

__EXPORT ScHFEditPage::~ScHFEditPage()
{
}

void ScHFEditPage::SetNumType(SvxNumType eNumType)
{
    aWndLeft.SetNumType(eNumType);
    aWndCenter.SetNumType(eNumType);
    aWndRight.SetNumType(eNumType);
}

// -----------------------------------------------------------------------

#define IS_AVAILABLE(w)(rCoreSet.GetItemState( (w) ) >= SFX_ITEM_AVAILABLE)

void __EXPORT ScHFEditPage::Reset( const SfxItemSet& rCoreSet )
{
    if ( IS_AVAILABLE( nWhich ) )
    {
        const ScPageHFItem& rItem = (const ScPageHFItem&)(rCoreSet.Get( nWhich ));

        const EditTextObject* pLeft   = rItem.GetLeftArea();
        const EditTextObject* pCenter = rItem.GetCenterArea();
        const EditTextObject* pRight  = rItem.GetRightArea();

        if ( pLeft && pCenter && pRight )
        {
            aWndLeft    .SetText( *pLeft );
            aWndCenter  .SetText( *pCenter );
            aWndRight   .SetText( *pRight );
        }
        SetSelectDefinedList();
    }
}

#undef IS_AVAILABLE

// -----------------------------------------------------------------------

BOOL __EXPORT ScHFEditPage::FillItemSet( SfxItemSet& rCoreSet )
{
    ScPageHFItem    aItem( nWhich );
    EditTextObject* pLeft   = aWndLeft  .CreateTextObject();
    EditTextObject* pCenter = aWndCenter.CreateTextObject();
    EditTextObject* pRight  = aWndRight .CreateTextObject();

    aItem.SetLeftArea  ( *pLeft );
    aItem.SetCenterArea( *pCenter );
    aItem.SetRightArea ( *pRight );
    delete pLeft;
    delete pCenter;
    delete pRight;

    rCoreSet.Put( aItem );

    return TRUE;
}

// -----------------------------------------------------------------------

#define SET_CMD(i,id) \
    aCmd  = aDel;                           \
    aCmd += ScGlobal::GetRscString( id );   \
    aCmd += aDel;                           \
    aCmdArr[i] = aCmd;

// -----------------------------------------------------------------------

void ScHFEditPage::FillCmdArr()
{
    String aDel( ScGlobal::GetRscString( STR_HFCMD_DELIMITER ) );
    String aCmd;

    SET_CMD( 0, STR_HFCMD_PAGE )
    SET_CMD( 1, STR_HFCMD_PAGES )
    SET_CMD( 2, STR_HFCMD_DATE )
    SET_CMD( 3, STR_HFCMD_TIME )
    SET_CMD( 4, STR_HFCMD_FILE )
    SET_CMD( 5, STR_HFCMD_TABLE )
}

#undef SET_CMD

void ScHFEditPage::InitPreDefinedList()
{
    SvtUserOptions aUserOpt;

    Color* pTxtColour = NULL;
    Color* pFldColour = NULL;

    // Get the all field values at the outset.
    String aPageFieldValue(aWndLeft.GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxPageField()), 0,0, pTxtColour, pFldColour));
    String aSheetFieldValue(aWndLeft.GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxTableField()), 0,0, pTxtColour, pFldColour));
    String aFileFieldValue(aWndLeft.GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxFileField()), 0,0, pTxtColour, pFldColour));
    String aExtFileFieldValue(aWndLeft.GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxExtFileField()), 0,0, pTxtColour, pFldColour));
    String aDateFieldValue(aWndLeft.GetEditEngine()->CalcFieldValue(SvxFieldItem(SvxDateField()), 0,0, pTxtColour, pFldColour));

    maLbDefined.Clear();

    maLbDefined.InsertEntry( ScGlobal::GetRscString( STR_HF_NONE_IN_BRACKETS ));

    String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
    aPageEntry += ' ';
    aPageEntry += aPageFieldValue;
    maLbDefined.InsertEntry(aPageEntry);

    String aPageOfEntry(aPageEntry);
    aPageOfEntry += ' ';
    aPageOfEntry += ScGlobal::GetRscString( STR_HF_OF_QUESTION );
    maLbDefined.InsertEntry( aPageOfEntry);

    maLbDefined.InsertEntry(aSheetFieldValue);

    String aConfidentialEntry(aUserOpt.GetCompany());
    aConfidentialEntry += ' ';
    aConfidentialEntry += ScGlobal::GetRscString( STR_HF_CONFIDENTIAL );
    aConfidentialEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aConfidentialEntry += aDateFieldValue;
    aConfidentialEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aConfidentialEntry += aPageEntry;
    maLbDefined.InsertEntry( aConfidentialEntry);

    String aFileNamePageEntry(aFileFieldValue);
    aFileNamePageEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aFileNamePageEntry += aPageEntry;
    maLbDefined.InsertEntry( aFileNamePageEntry);

    maLbDefined.InsertEntry( aExtFileFieldValue);

    String aPageSheetNameEntry(aPageEntry);
    aPageSheetNameEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aPageSheetNameEntry += aSheetFieldValue;
    maLbDefined.InsertEntry( aPageSheetNameEntry);

    String aPageFileNameEntry(aPageEntry);
    aPageFileNameEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aPageFileNameEntry += aFileFieldValue;
    maLbDefined.InsertEntry( aPageFileNameEntry);

    String aPagePathNameEntry(aPageEntry);
    aPagePathNameEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aPagePathNameEntry += aExtFileFieldValue;
    maLbDefined.InsertEntry( aPagePathNameEntry);

    String aUserNameEntry(aUserOpt.GetFirstName());
    aUserNameEntry += ' ';
    aUserNameEntry += aUserOpt.GetLastName();
    aUserNameEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aUserNameEntry += aPageEntry;
    aUserNameEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aUserNameEntry += aDateFieldValue;
    maLbDefined.InsertEntry( aUserNameEntry);

    String aCreatedByEntry(ScGlobal::GetRscString( STR_HF_CREATED_BY ) );
    aCreatedByEntry += ' ';
    aCreatedByEntry += aUserOpt.GetFirstName();
    aCreatedByEntry += ' ';
    aCreatedByEntry += aUserOpt.GetLastName();
    aCreatedByEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aCreatedByEntry += aDateFieldValue;
    aCreatedByEntry.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
    aCreatedByEntry += aPageEntry;
    maLbDefined.InsertEntry( aCreatedByEntry);
}

void ScHFEditPage::InsertToDefinedList()
{
    USHORT nCount =  maLbDefined.GetEntryCount();
    if(nCount == eEntryCount)
    {
        String aCustomizedEntry(ScGlobal::GetRscString( STR_HF_CUSTOMIZED ) );
        maLbDefined.InsertEntry( aCustomizedEntry);
        maLbDefined.SelectEntryPos(eEntryCount);
    }
}

void ScHFEditPage::RemoveFromDefinedList()
{
    USHORT nCount =  maLbDefined.GetEntryCount();
    if(nCount > eEntryCount )
        maLbDefined.RemoveEntry( nCount-1);
}

// determine if the header/footer exists in our predefined list and set select to it.
void ScHFEditPage::SetSelectDefinedList()
{
    SvtUserOptions aUserOpt;

    // default to customized
    ScHFEntryId eSelectEntry = eEntryCount;

    ::std::auto_ptr< EditTextObject > pLeftObj;
    ::std::auto_ptr< EditTextObject > pCenterObj;
    ::std::auto_ptr< EditTextObject > pRightObj;

    XubString aLeftEntry;
    XubString aCenterEntry;
    XubString aRightEntry;

    pLeftObj.reset(aWndLeft.GetEditEngine()->CreateTextObject());
    pCenterObj.reset(aWndCenter.GetEditEngine()->CreateTextObject());
    pRightObj.reset(aWndRight.GetEditEngine()->CreateTextObject());

    bool bFound = false;

    USHORT i;
    USHORT nCount =  maLbDefined.GetEntryCount();
    for(i = 0; i < nCount && !bFound; i++)
    {
        switch(static_cast<ScHFEntryId>(i))
        {
            case eNoneEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aCenterEntry = pCenterObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(aLeftEntry == EMPTY_STRING && aCenterEntry == EMPTY_STRING
                    && aRightEntry == EMPTY_STRING)
                {
                    eSelectEntry = eNoneEntry;
                    bFound = true;
                }
            }
            break;

            case ePageEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(aLeftEntry == EMPTY_STRING && aRightEntry == EMPTY_STRING)
                {
                    if(IsPageEntry(aWndCenter.GetEditEngine(), pCenterObj.get()))
                    {
                        eSelectEntry = ePageEntry;
                        bFound = true;
                    }
                }
            }
            break;


            //TODO
            case ePagesEntry:
            {
            }
            break;

            case eSheetEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(aLeftEntry == EMPTY_STRING && aRightEntry == EMPTY_STRING)
                {
                    if(pCenterObj->IsFieldObject())
                    {
                        const SvxFieldItem* pFieldItem = pCenterObj->GetField();
                        if(pFieldItem)
                        {
                            const SvxFieldData* pField = pFieldItem->GetField();
                            if(pField && pField->ISA(SvxTableField))
                            {
                                eSelectEntry = eSheetEntry;
                                bFound = true;
                            }
                        }
                    }
                }
            }
            break;

            case eConfidentialEntry:
            {
                if(IsDateEntry(pCenterObj.get()) && IsPageEntry(aWndRight.GetEditEngine(), pRightObj.get()))
                {
                    String aConfidentialEntry(aUserOpt.GetCompany());
                    aConfidentialEntry += ' ';
                    aConfidentialEntry += ScGlobal::GetRscString( STR_HF_CONFIDENTIAL );
                    if(aConfidentialEntry == aWndLeft.GetEditEngine()->GetText(0))
                    {
                        eSelectEntry = eConfidentialEntry;
                        bFound = true;
                    }
                }
            }
            break;

            //TODO
            case eFileNamePageEntry:
            {
            }
            break;

            case eExtFileNameEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                aRightEntry = pRightObj->GetText(0);
                if(IsExtFileNameEntry(pCenterObj.get()) && aLeftEntry == EMPTY_STRING
                    && aRightEntry == EMPTY_STRING)
                {
                    eSelectEntry = eExtFileNameEntry;
                    bFound = true;
                }
            }
            break;

            //TODO
            case ePageSheetEntry:
            {
            }
            break;

            //TODO
            case ePageFileNameEntry:
            {
            }
            break;

            case ePageExtFileNameEntry:
            {
                aLeftEntry = pLeftObj->GetText(0);
                if(IsPageEntry(aWndCenter.GetEditEngine(), pCenterObj.get()) &&
                    IsExtFileNameEntry(pRightObj.get()) && aLeftEntry == EMPTY_STRING)
                {
                    eSelectEntry = ePageExtFileNameEntry;
                    bFound = true;
                }
            }
            break;

            case eUserNameEntry:
            {
                if(IsDateEntry(pRightObj.get()) && IsPageEntry(aWndCenter.GetEditEngine(), pCenterObj.get()))
                {
                    String aUserNameEntry(aUserOpt.GetFirstName());
                    aUserNameEntry += ' ';
                    aUserNameEntry += aUserOpt.GetLastName();
                    if(aUserNameEntry == aWndLeft.GetEditEngine()->GetText(0))
                    {
                        eSelectEntry = eUserNameEntry;
                        bFound = true;
                    }
                }
            }
            break;

            case eCreatedByEntry:
            {
                if(IsDateEntry(pCenterObj.get()) && IsPageEntry(aWndRight.GetEditEngine(), pRightObj.get()))
                {
                    String aCreatedByEntry(ScGlobal::GetRscString( STR_HF_CREATED_BY ) );
                    aCreatedByEntry += ' ';
                    aCreatedByEntry += aUserOpt.GetFirstName();
                    aCreatedByEntry += ' ';
                    aCreatedByEntry += aUserOpt.GetLastName();
                    if(aCreatedByEntry == aWndLeft.GetEditEngine()->GetText(0))
                    {
                        eSelectEntry = eCreatedByEntry;
                        bFound = true;
                    }
                }
            }
            break;
        }
    }

    if(eSelectEntry == eEntryCount)
        InsertToDefinedList();

    maLbDefined.SelectEntryPos(eSelectEntry);
}

bool ScHFEditPage::IsPageEntry(EditEngine*pEngine, EditTextObject* pTextObj)
{
    if(!pEngine && !pTextObj)
        return false;

    bool bReturn = false;

    if(!pTextObj->IsFieldObject())
    {
        SvUShorts aPosList;
        pEngine->GetPortions(0,aPosList);
        if(aPosList.Count() == 2)
        {
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            ESelection aSel(0,0,0,0);
            aSel.nEndPos = aPageEntry.Len();
            if(aPageEntry == pEngine->GetText(aSel))
            {
                aSel.nStartPos = aSel.nEndPos;
                aSel.nEndPos++;
                ::std::auto_ptr< EditTextObject > pPageObj;
                pPageObj.reset(pEngine->CreateTextObject(aSel));
                if(pPageObj.get() && pPageObj->IsFieldObject() )
                {
                    const SvxFieldItem* pFieldItem = pPageObj->GetField();
                    if(pFieldItem)
                    {
                        const SvxFieldData* pField = pFieldItem->GetField();
                        if(pField && pField->ISA(SvxPageField))
                            bReturn = true;
                    }
                }
            }
        }
    }
    return bReturn;
}

bool ScHFEditPage::IsDateEntry(EditTextObject* pTextObj)
{
    if(!pTextObj)
        return false;

    bool bReturn = false;
    if(pTextObj->IsFieldObject())
    {
        const SvxFieldItem* pFieldItem = pTextObj->GetField();
        if(pFieldItem)
        {
            const SvxFieldData* pField = pFieldItem->GetField();
            if(pField && pField->ISA(SvxDateField))
                bReturn = true;
        }
    }
    return bReturn;
}

bool ScHFEditPage::IsExtFileNameEntry(EditTextObject* pTextObj)
{
    if(!pTextObj)
        return false;
    bool bReturn = false;
    if(pTextObj->IsFieldObject())
    {
        const SvxFieldItem* pFieldItem = pTextObj->GetField();
        if(pFieldItem)
    {
            const SvxFieldData* pField = pFieldItem->GetField();
            if(pField && pField->ISA(SvxExtFileField))
                bReturn = true;
        }
    }
    return bReturn;
}

void ScHFEditPage::ProcessDefinedListSel(ScHFEntryId eSel, bool bTravelling)
{
    SvtUserOptions aUserOpt;
    ::std::auto_ptr< EditTextObject > pTextObj;

    switch(eSel)
    {
        case eNoneEntry:
            ClearTextAreas();
            if(!bTravelling)
                aWndLeft.GrabFocus();
        break;

        case ePageEntry:
        {
            ClearTextAreas();
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndCenter.GetEditEngine()->SetText(aPageEntry);
            aWndCenter.InsertField( SvxFieldItem(SvxPageField()) );
            if(!bTravelling)
                aWndCenter.GrabFocus();
        }
        break;

        case ePagesEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndCenter.GetEditEngine()->SetText(aPageEntry);
            aSel.nEndPos = aPageEntry.Len();
            aWndCenter.GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField()), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;
            String aPageOfEntry = ' ';
            aPageOfEntry += ScGlobal::GetRscString( STR_HF_OF );
            aPageOfEntry += ' ';
            aWndCenter.GetEditEngine()->QuickInsertText(aPageOfEntry,ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos += aPageOfEntry.Len();
            aWndCenter.GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPagesField()), ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj.reset(aWndCenter.GetEditEngine()->CreateTextObject());
            aWndCenter.SetText(*pTextObj);
            XubString aEntry(pTextObj.get()->GetText(0));
            if(!bTravelling)
                aWndCenter.GrabFocus();
        }
        break;

        case eSheetEntry:
            ClearTextAreas();
            aWndCenter.InsertField( SvxFieldItem(SvxTableField()) );
            if(!bTravelling)
                aWndCenter.GrabFocus();
        break;

        case eConfidentialEntry:
        {
            ClearTextAreas();
            String aConfidentialEntry(aUserOpt.GetCompany());
            aConfidentialEntry += ' ';
            aConfidentialEntry += ScGlobal::GetRscString( STR_HF_CONFIDENTIAL );
            aWndLeft.GetEditEngine()->SetText(aConfidentialEntry);
            aWndCenter.InsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)) );
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndRight.GetEditEngine()->SetText(aPageEntry);
            aWndRight.InsertField( SvxFieldItem(SvxPageField()) );
            if(!bTravelling)
                aWndRight.GrabFocus();
        }
        break;

        case eFileNamePageEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            aWndCenter.GetEditEngine()->QuickInsertField(SvxFieldItem( SvxFileField() ), aSel );
            ++aSel.nEndPos;
            String aPageEntry(RTL_CONSTASCII_STRINGPARAM(", "));
            aPageEntry += ScGlobal::GetRscString( STR_PAGE ) ;
            aPageEntry += ' ';
            aWndCenter.GetEditEngine()->QuickInsertText(aPageEntry, ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nStartPos = aSel.nEndPos;
            aSel.nEndPos += aPageEntry.Len();
            aWndCenter.GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField()), ESelection(aSel.nEndPara,aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj.reset(aWndCenter.GetEditEngine()->CreateTextObject());
            aWndCenter.SetText(*pTextObj);
            XubString aEntry2(pTextObj.get()->GetText(0));
            if(!bTravelling)
                aWndCenter.GrabFocus();
        }
        break;

        case eExtFileNameEntry:
            ClearTextAreas();
            aWndCenter.InsertField( SvxFieldItem( SvxExtFileField(
                EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_FULLPATH ) ) );
            if(!bTravelling)
                aWndCenter.GrabFocus();
        break;

        case ePageSheetEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndCenter.GetEditEngine()->SetText(aPageEntry);
            aSel.nEndPos = aPageEntry.Len();
            aWndCenter.GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField()), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;
            String aCommaSpace(RTL_CONSTASCII_STRINGPARAM(", "));
            aWndCenter.GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos += aCommaSpace.Len();
            aWndCenter.GetEditEngine()->QuickInsertField( SvxFieldItem(SvxTableField()), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj.reset(aWndCenter.GetEditEngine()->CreateTextObject());
            aWndCenter.SetText(*pTextObj);
            if(!bTravelling)
                aWndCenter.GrabFocus();
        }
        break;

        case ePageFileNameEntry:
        {
            ClearTextAreas();
            ESelection aSel(0,0,0,0);
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndCenter.GetEditEngine()->SetText(aPageEntry);
            aSel.nEndPos = aPageEntry.Len();
            aWndCenter.GetEditEngine()->QuickInsertField(SvxFieldItem(SvxPageField()), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            ++aSel.nEndPos;
            String aCommaSpace(RTL_CONSTASCII_STRINGPARAM(", "));
            aWndCenter.GetEditEngine()->QuickInsertText(aCommaSpace,ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            aSel.nEndPos += aCommaSpace.Len();
            aWndCenter.GetEditEngine()->QuickInsertField( SvxFieldItem(SvxFileField()), ESelection(aSel.nEndPara, aSel.nEndPos, aSel.nEndPara, aSel.nEndPos));
            pTextObj.reset(aWndCenter.GetEditEngine()->CreateTextObject());
            aWndCenter.SetText(*pTextObj);
            if(!bTravelling)
                aWndCenter.GrabFocus();
        }
        break;

        case ePageExtFileNameEntry:
        {
            ClearTextAreas();
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndCenter.GetEditEngine()->SetText(aPageEntry);
            aWndCenter.InsertField( SvxFieldItem(SvxPageField()) );
            aWndRight.InsertField( SvxFieldItem( SvxExtFileField(
                EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_FULLPATH ) ) );
            if(!bTravelling)
                aWndRight.GrabFocus();
        }
        break;

        case eUserNameEntry:
        {
            ClearTextAreas();
            String aUserNameEntry(aUserOpt.GetFirstName());
            aUserNameEntry += ' ';
            aUserNameEntry += aUserOpt.GetLastName();
            aWndLeft.GetEditEngine()->SetText(aUserNameEntry);
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndCenter.GetEditEngine()->SetText(aPageEntry);
            aWndCenter.InsertField( SvxFieldItem(SvxPageField()) );
            aWndRight.InsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)) );
            if(!bTravelling)
                aWndRight.GrabFocus();
        }
        break;

        case eCreatedByEntry:
        {
            ClearTextAreas();
            String aCreatedByEntry(ScGlobal::GetRscString( STR_HF_CREATED_BY ) );
            aCreatedByEntry += ' ';
            aCreatedByEntry += aUserOpt.GetFirstName();
            aCreatedByEntry += ' ';
            aCreatedByEntry += aUserOpt.GetLastName();
            aWndLeft.GetEditEngine()->SetText(aCreatedByEntry);
            aWndCenter.InsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)) );
            String aPageEntry(ScGlobal::GetRscString( STR_PAGE ) );
            aPageEntry += ' ';
            aWndRight.GetEditEngine()->SetText(aPageEntry);
            aWndRight.InsertField( SvxFieldItem(SvxPageField()) );
            if(!bTravelling)
                aWndRight.GrabFocus();
        }
        break;

        default :
            break;
    }
}

void ScHFEditPage::ClearTextAreas()
{
    aWndLeft.GetEditEngine()->SetText(EMPTY_STRING);
    aWndLeft.Invalidate();
    aWndCenter.GetEditEngine()->SetText(EMPTY_STRING);
    aWndCenter.Invalidate();
    aWndRight.GetEditEngine()->SetText(EMPTY_STRING);
    aWndRight.Invalidate();
}

//-----------------------------------------------------------------------
// Handler:
//-----------------------------------------------------------------------

IMPL_LINK( ScHFEditPage, ListHdl_Impl, ListBox*, pList )
{
    if ( pList && pList == &maLbDefined )
    {
        ScHFEntryId eSel = static_cast<ScHFEntryId>(maLbDefined.GetSelectEntryPos());
        if(!maLbDefined.IsTravelSelect())
        {
            ProcessDefinedListSel(eSel);

            // check if we need to remove the customized entry.
            if(eSel < eEntryCount)
                RemoveFromDefinedList();
        }
        else
        {
            ProcessDefinedListSel(eSel, true);
        }
    }
    return 0;
}

IMPL_LINK( ScHFEditPage, ClickHdl, ImageButton*, pBtn )
{
    pActiveEdWnd = ::GetScEditWindow(); //CHINA001
    if ( !pActiveEdWnd )
        return 0;

    if ( pBtn == &aBtnText )
    {
        pActiveEdWnd->SetCharAttriutes();
    }
    else
    {
        if ( pBtn == &aBtnPage )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxPageField()) );
        else if ( pBtn == &aBtnLastPage )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxPagesField()) );
        else if ( pBtn == &aBtnDate )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxDateField(Date(),SVXDATETYPE_VAR)) );
        else if ( pBtn == &aBtnTime )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxTimeField()) );
        else if ( pBtn == &aBtnFile )
        {
            pActiveEdWnd->InsertField( SvxFieldItem( SvxFileField() ) );
        }
        else if ( pBtn == &aBtnTable )
            pActiveEdWnd->InsertField( SvxFieldItem(SvxTableField()) );
    }
    InsertToDefinedList();
    pActiveEdWnd->GrabFocus();

    return 0;
}

IMPL_LINK( ScHFEditPage, MenuHdl, ScExtIButton*, pBtn )
{
    pActiveEdWnd = ::GetScEditWindow(); //CHINA001
    if ( !pActiveEdWnd )
        return 0;

    if(pBtn!=NULL)
    {
        switch(pBtn->GetSelected())
        {
            case FILE_COMMAND_TITEL:
                pActiveEdWnd->InsertField( SvxFieldItem( SvxFileField() ) );
                break;
            case FILE_COMMAND_FILENAME:
                pActiveEdWnd->InsertField( SvxFieldItem( SvxExtFileField(
                        EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_NAME_EXT ) ) );
                break;
            case FILE_COMMAND_PATH:
                pActiveEdWnd->InsertField( SvxFieldItem( SvxExtFileField(
                        EMPTY_STRING, SVXFILETYPE_VAR, SVXFILEFORMAT_FULLPATH ) ) );
                break;
        }
    }
    return 0;
}

//========================================================================
// class ScRightHeaderEditPage
//========================================================================

ScRightHeaderEditPage::ScRightHeaderEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_HR, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERRIGHT ), true )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScRightHeaderEditPage::GetRanges()
    { return pPageRightHeaderRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScRightHeaderEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScRightHeaderEditPage( pParent, rCoreSet ) ); };


//========================================================================
// class ScLeftHeaderEditPage
//========================================================================

ScLeftHeaderEditPage::ScLeftHeaderEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_HL, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_HEADERLEFT ), true )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScLeftHeaderEditPage::GetRanges()
    { return pPageLeftHeaderRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScLeftHeaderEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScLeftHeaderEditPage( pParent, rCoreSet ) ); };

//========================================================================
// class ScRightFooterEditPage
//========================================================================

ScRightFooterEditPage::ScRightFooterEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_FR, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERRIGHT ), false )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScRightFooterEditPage::GetRanges()
    { return pPageRightFooterRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScRightFooterEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScRightFooterEditPage( pParent, rCoreSet ) ); };

//========================================================================
// class ScLeftFooterEditPage
//========================================================================

ScLeftFooterEditPage::ScLeftFooterEditPage( Window* pParent, const SfxItemSet& rCoreSet )
    : ScHFEditPage( pParent, RID_SCPAGE_HFED_FL, rCoreSet,
                    rCoreSet.GetPool()->GetWhich(SID_SCATTR_PAGE_FOOTERLEFT ), false )
    {}

// -----------------------------------------------------------------------

USHORT* __EXPORT ScLeftFooterEditPage::GetRanges()
    { return pPageLeftFooterRanges; }

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT ScLeftFooterEditPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
    { return ( new ScLeftFooterEditPage( pParent, rCoreSet ) ); };
