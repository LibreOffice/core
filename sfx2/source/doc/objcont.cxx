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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XStandaloneDocumentInfo.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <tools/cachestr.hxx>
#include <vcl/msgbox.hxx>
#include <svl/style.hxx>
#include <vcl/wrkwin.hxx>

#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/rectitem.hxx>
#include <svl/eitem.hxx>
#include <svl/urihelper.hxx>
#include <svl/ctloptions.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/securityoptions.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <tools/datetime.hxx>
#include <math.h>

#include <unotools/saveopt.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <vcl/virdev.hxx>
#include <vcl/oldprintadaptor.hxx>

#include <sfx2/app.hxx>
#include "sfx2/sfxresid.hxx"
#include "appdata.hxx"
#include <sfx2/dinfdlg.hxx>
#include "fltfnc.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include "objshimp.hxx"
#include <sfx2/evntconf.hxx>
#include "sfx2/sfxhelp.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include "sfx2/basmgr.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/doctempl.hxx>
#include "doc.hrc"
#include <sfx2/sfxbasemodel.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/request.hxx>
#include "openflag.hxx"
#include "querytemplate.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

//====================================================================

//====================================================================

static
bool operator> (const util::DateTime& i_rLeft, const util::DateTime& i_rRight)
{
    if ( i_rLeft.Year != i_rRight.Year )
        return i_rLeft.Year > i_rRight.Year;

    if ( i_rLeft.Month != i_rRight.Month )
        return i_rLeft.Month > i_rRight.Month;

    if ( i_rLeft.Day != i_rRight.Day )
        return i_rLeft.Day > i_rRight.Day;

    if ( i_rLeft.Hours != i_rRight.Hours )
        return i_rLeft.Hours > i_rRight.Hours;

    if ( i_rLeft.Minutes != i_rRight.Minutes )
        return i_rLeft.Minutes > i_rRight.Minutes;

    if ( i_rLeft.Seconds != i_rRight.Seconds )
        return i_rLeft.Seconds > i_rRight.Seconds;

    if ( i_rLeft.HundredthSeconds != i_rRight.HundredthSeconds )
        return i_rLeft.HundredthSeconds > i_rRight.HundredthSeconds;

    return sal_False;
}


::boost::shared_ptr<GDIMetaFile>
SfxObjectShell::GetPreviewMetaFile( sal_Bool bFullContent ) const
{
    return CreatePreviewMetaFile_Impl( bFullContent, sal_False );
}


::boost::shared_ptr<GDIMetaFile>
SfxObjectShell::CreatePreviewMetaFile_Impl( sal_Bool bFullContent, sal_Bool bHighContrast ) const
{
    // Nur wenn gerade nicht gedruckt wird, darf DoDraw aufgerufen
    // werden, sonst wird u.U. der Printer abgeschossen !
    SfxViewFrame *pFrame = SfxViewFrame::GetFirst( this );
    if ( pFrame && pFrame->GetViewShell() &&
         pFrame->GetViewShell()->GetPrinter() &&
         pFrame->GetViewShell()->GetPrinter()->IsPrinting() )
         return ::boost::shared_ptr<GDIMetaFile>();

    ::boost::shared_ptr<GDIMetaFile> pFile(new GDIMetaFile);

    VirtualDevice aDevice;
    aDevice.EnableOutput( sal_False );

    // adjust the output device if HC-metafile is requested
    if ( bHighContrast )
        aDevice.SetDrawMode( aDevice.GetDrawMode() | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );

    MapMode aMode( ((SfxObjectShell*)this)->GetMapUnit() );
    aDevice.SetMapMode( aMode );
    pFile->SetPrefMapMode( aMode );

    Size aTmpSize;
    sal_Int8 nAspect;
    if ( bFullContent )
    {
        nAspect = ASPECT_CONTENT;
        aTmpSize = GetVisArea( nAspect ).GetSize();
    }
    else
    {
        nAspect = ASPECT_THUMBNAIL;
        aTmpSize = ((SfxObjectShell*)this)->GetFirstPageSize();
    }

    pFile->SetPrefSize( aTmpSize );
    DBG_ASSERT( aTmpSize.Height()*aTmpSize.Width(),
                "size of first page is 0, overload GetFirstPageSize or set vis-area!" );

    pFile->Record( &aDevice );

    LanguageType eLang;
    SvtCTLOptions*  pCTLOptions = new SvtCTLOptions;
    if ( SvtCTLOptions::NUMERALS_HINDI == pCTLOptions->GetCTLTextNumerals() )
        eLang = LANGUAGE_ARABIC_SAUDI_ARABIA;
    else if ( SvtCTLOptions::NUMERALS_ARABIC == pCTLOptions->GetCTLTextNumerals() )
        eLang = LANGUAGE_ENGLISH;
    else
        eLang = (LanguageType) Application::GetSettings().GetLanguage();

    aDevice.SetDigitLanguage( eLang );

    ((SfxObjectShell*)this)->DoDraw( &aDevice, Point(0,0), aTmpSize, JobSetup(), nAspect );
    pFile->Stop();

    return pFile;
}

//====================================================================

void SfxObjectShell::UpdateDocInfoForSave()
{
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());

    // clear user data if recommend (see 'Tools - Options - Open/StarOffice - Security')
    if ( SvtSecurityOptions().IsOptionSet(
            SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO ) )
    {
        xDocProps->resetUserData( ::rtl::OUString() );
    }
    else if ( IsModified() )
    {
        String aUserName = SvtUserOptions().GetFullName();
        if ( !IsUseUserData() )
        {
            // remove all data pointing to the current user
            if (xDocProps->getAuthor().equals(aUserName)) {
                xDocProps->setAuthor( ::rtl::OUString() );
            }
            xDocProps->setModifiedBy( ::rtl::OUString() );
            if (xDocProps->getPrintedBy().equals(aUserName)) {
                xDocProps->setPrintedBy( ::rtl::OUString() );
            }
        }
        else
        {
            // update ModificationAuthor, revision and editing time
            ::DateTime now;
            xDocProps->setModificationDate( util::DateTime(
                now.Get100Sec(), now.GetSec(), now.GetMin(),
                now.GetHour(), now.GetDay(), now.GetMonth(),
                now.GetYear() ) );
            xDocProps->setModifiedBy( aUserName );
            if ( !HasName() || pImp->bIsSaving )
                // QUESTION: not in case of "real" SaveAs as this is meant to create a new document
                UpdateTime_Impl( xDocProps );
        }
    }
}

//--------------------------------------------------------------------

static void
lcl_add(util::Duration & rDur, Time const& rTime)
{
    // here we don't care about overflow: rDur is converted back to seconds
    // anyway, and Time cannot store more than ~4000 hours
    rDur.Hours   += rTime.GetHour();
    rDur.Minutes += rTime.GetMin();
    rDur.Seconds += rTime.GetSec();
}

// Bearbeitungszeit aktualisieren
void SfxObjectShell::UpdateTime_Impl(
    const uno::Reference<document::XDocumentProperties> & i_xDocProps)
{
    // Get old time from documentinfo
    const sal_Int32 secs = i_xDocProps->getEditingDuration();
    util::Duration editDuration(sal_False, 0, 0, 0,
            secs/3600, (secs%3600)/60, secs%60, 0);

    // Initialize some local member! Its neccessary for wollow operations!
    DateTime    aNow                    ;   // Date and time at current moment
    Time        n24Time     (24,0,0,0)  ;   // Time-value for 24 hours - see follow calculation
    sal_uIntPtr     nDays       = 0         ;   // Count of days between now and last editing
    Time        nAddTime    (0)         ;   // Value to add on aOldTime

    // Safe impossible cases!
    // User has changed time to the past between last editing and now ... its not possible!!!
    DBG_ASSERT( !(aNow.GetDate()<pImp->nTime.GetDate()), "Timestamp of last change is in the past ?!..." );

    // Do the follow only, if user has NOT changed time to the past.
    // Else add a time of 0 to aOldTime ... !!!
    if (aNow.GetDate()>=pImp->nTime.GetDate())
    {
        // Get count of days last editing.
        nDays = aNow.GetSecFromDateTime(pImp->nTime.GetDate())/86400 ;

        if (nDays==0)
        {
            // If no day between now and last editing - calculate time directly.
            nAddTime    =   (const Time&)aNow - (const Time&)pImp->nTime ;
        }
        else
        // If time of working without save greater then 1 month (!) ....
        // we add 0 to aOldTime!
        if (nDays<=31)
        {
            // If 1 or up to 31 days between now and last editing - calculate time indirectly.
            // nAddTime = (24h - nTime) + (nDays * 24h) + aNow
            --nDays;
             nAddTime    =  nDays*n24Time.GetTime() ;
            nAddTime    +=  n24Time-(const Time&)pImp->nTime        ;
            nAddTime    +=  aNow                    ;
        }

        lcl_add(editDuration, nAddTime);
    }

    pImp->nTime = aNow;
    try {
        const sal_Int32 newSecs( (editDuration.Hours*3600)
            + (editDuration.Minutes*60) + editDuration.Seconds);
        i_xDocProps->setEditingDuration(newSecs);
        i_xDocProps->setEditingCycles(i_xDocProps->getEditingCycles() + 1);
    }
    catch (lang::IllegalArgumentException &)
    {
        // ignore overflow
    }
}

//--------------------------------------------------------------------

SfxDocumentInfoDialog* SfxObjectShell::CreateDocumentInfoDialog
(
    Window*             pParent,
    const SfxItemSet&   rSet
)
{
    return new SfxDocumentInfoDialog(pParent, rSet);
}

//--------------------------------------------------------------------

SfxStyleSheetBasePool* SfxObjectShell::GetStyleSheetPool()
{
    return 0;
}

void SfxObjectShell::SetOrganizerSearchMask(
    SfxStyleSheetBasePool* pStylePool) const
{
    pStylePool->SetSearchMask(
        SFX_STYLE_FAMILY_ALL,
        SFXSTYLEBIT_USERDEF | SFXSTYLEBIT_USED);
}

//--------------------------------------------------------------------

sal_uInt16 SfxObjectShell::GetContentCount(
    sal_uInt16 nIdx1,
    sal_uInt16 /*nIdx2*/)
{
    switch(nIdx1)
    {
        case INDEX_IGNORE:
            return DEF_CONTENT_COUNT;
        case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
            if(!pStylePool)
                return 0;
            SetOrganizerSearchMask(pStylePool);
            return pStylePool->Count();
        }
        case CONTENT_MACRO:
            break;
/*
        case CONTENT_CONFIG:
            return ( GetConfigManager() ) ?
                        GetConfigManager()->GetItemCount() : 0;
            break;
 */
    }
    return 0;
}


//--------------------------------------------------------------------
//TODO/CLEANUP: remove this method (it's virtual)
void  SfxObjectShell::TriggerHelpPI(sal_uInt16 nIdx1, sal_uInt16 nIdx2, sal_uInt16)
{
    if(nIdx1==CONTENT_STYLE && nIdx2 != INDEX_IGNORE) //StyleSheets
    {
        SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
        SetOrganizerSearchMask(pStylePool);
    }
}

sal_Bool   SfxObjectShell::CanHaveChilds(sal_uInt16 nIdx1,
                                       sal_uInt16 nIdx2)
{
    switch(nIdx1) {
    case INDEX_IGNORE:
        return sal_True;
    case CONTENT_STYLE:
        return INDEX_IGNORE == nIdx2 || !GetStyleSheetPool()? sal_False: sal_True;
    case CONTENT_MACRO:
//!!    return INDEX_IGNORE == nIdx2? sal_False: sal_True;
        return sal_False;
/*
    case CONTENT_CONFIG:
        return INDEX_IGNORE == nIdx2 ? sal_False : sal_True;
 */
    }
    return sal_False;
}

//--------------------------------------------------------------------

void SfxObjectShell::GetContent(String &rText,
                                Bitmap &rClosedBitmap,
                                Bitmap &rOpenedBitmap,
                                sal_Bool &bCanDel,
                                sal_uInt16 i,
                                sal_uInt16 nIdx1,
                                sal_uInt16 nIdx2 )
{
    DBG_ERRORFILE( "Non high contrast method called. Please update calling code!" );
    SfxObjectShell::GetContent( rText, rClosedBitmap, rOpenedBitmap, BMP_COLOR_NORMAL, bCanDel, i, nIdx1, nIdx2 );
}

//--------------------------------------------------------------------

void   SfxObjectShell::GetContent(String &rText,
                                  Bitmap &rClosedBitmap,
                                  Bitmap &rOpenedBitmap,
                                  BmpColorMode eColorMode,
                                  sal_Bool &bCanDel,
                                  sal_uInt16 i,
                                  sal_uInt16 nIdx1,
                                  sal_uInt16 /*nIdx2*/ )
{
    bCanDel=sal_True;

    switch(nIdx1)
    {
        case INDEX_IGNORE:
        {
            sal_uInt16 nTextResId = 0;
            sal_uInt16 nClosedBitmapResId = 0; // evtl. sp"ater mal unterschiedliche
            sal_uInt16 nOpenedBitmapResId = 0; // "     "       "   "
            switch(i)
            {
                case CONTENT_STYLE:
                    nTextResId = STR_STYLES;
                    if ( eColorMode == BMP_COLOR_NORMAL )
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED;
                        nOpenedBitmapResId= BMP_STYLES_OPENED;
                    }
                    else
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED_HC;
                        nOpenedBitmapResId= BMP_STYLES_OPENED_HC;
                    }
                    break;
                case CONTENT_MACRO:
                    nTextResId = STR_MACROS;
                    if ( eColorMode == BMP_COLOR_NORMAL )
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED;
                        nOpenedBitmapResId= BMP_STYLES_OPENED;
                    }
                    else
                    {
                        nClosedBitmapResId= BMP_STYLES_CLOSED_HC;
                        nOpenedBitmapResId= BMP_STYLES_OPENED_HC;
                    }
                    break;
/*
                case CONTENT_CONFIG:
                    nTextResId = STR_CONFIG;
                    nClosedBitmapResId= BMP_STYLES_CLOSED;
                    nOpenedBitmapResId= BMP_STYLES_OPENED;
                    break;
 */
            }

            if ( nTextResId )
            {
                rText  = String(SfxResId(nTextResId));
                rClosedBitmap = Bitmap(SfxResId(nClosedBitmapResId));
                rOpenedBitmap = Bitmap(SfxResId(nOpenedBitmapResId));
            }
            break;
        }

        case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
            SetOrganizerSearchMask(pStylePool);
            SfxStyleSheetBase *pStyle = (*pStylePool)[i];
            rText = pStyle->GetName();
            bCanDel=((pStyle->GetMask() & SFXSTYLEBIT_USERDEF)
                     == SFXSTYLEBIT_USERDEF);
            rClosedBitmap = rOpenedBitmap =
                GetStyleFamilyBitmap(pStyle->GetFamily(), eColorMode );
        }
            break;
        case CONTENT_MACRO:
            break;
/*
        case CONTENT_CONFIG:
            if ( GetConfigManager() )
            {
                rText = GetConfigManager()->GetItem(i);
                bCanDel = GetConfigManager()->CanDelete(i);
            }
            else
                rText = String();
            rClosedBitmap = Bitmap(SfxResId(BMP_STYLES_CLOSED));
            rOpenedBitmap = Bitmap(SfxResId(BMP_STYLES_OPENED));
            break;
*/
    }
}

//--------------------------------------------------------------------
Bitmap SfxObjectShell::GetStyleFamilyBitmap( SfxStyleFamily eFamily )
{
    DBG_ERRORFILE( "Non high contrast method called. Please update calling code!" );
    return SfxObjectShell::GetStyleFamilyBitmap( eFamily, BMP_COLOR_NORMAL );
}

//--------------------------------------------------------------------

Bitmap SfxObjectShell::GetStyleFamilyBitmap(SfxStyleFamily eFamily, BmpColorMode eColorMode )
{
    sal_uInt16 nResId = 0;
    switch(eFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY1 : BMP_STYLES_FAMILY1_HC;
            break;
        case SFX_STYLE_FAMILY_PARA:
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY2 : BMP_STYLES_FAMILY2_HC;
            break;
        case SFX_STYLE_FAMILY_FRAME:
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY3 : BMP_STYLES_FAMILY3_HC;
            break;
        case SFX_STYLE_FAMILY_PAGE :
            nResId = ( eColorMode == BMP_COLOR_NORMAL ) ? BMP_STYLES_FAMILY4 : BMP_STYLES_FAMILY4_HC;
            break;
        case SFX_STYLE_FAMILY_PSEUDO:
        case SFX_STYLE_FAMILY_ALL:
            break;
    }

    if ( nResId )
        return Bitmap(SfxResId(nResId));
    else
        return Bitmap();
}


//--------------------------------------------------------------------

sal_Bool SfxObjectShell::Insert(SfxObjectShell &rSource,
                              sal_uInt16 nSourceIdx1,
                              sal_uInt16 nSourceIdx2,
                              sal_uInt16 /*nSourceIdx3*/,
                              sal_uInt16 &nIdx1,
                              sal_uInt16 &nIdx2,
                              sal_uInt16 &/*nIdx3*/,
                              sal_uInt16 &/*nDeleted*/)
{
    sal_Bool bRet = sal_False;

    if (INDEX_IGNORE == nIdx1 && CONTENT_STYLE == nSourceIdx1)
        nIdx1 = CONTENT_STYLE;

    if (CONTENT_STYLE == nSourceIdx1 && CONTENT_STYLE == nIdx1)
    {
        SfxStyleSheetBasePool* pHisPool  = rSource.GetStyleSheetPool();
        SfxStyleSheetBasePool* pMyPool   = GetStyleSheetPool();
        SetOrganizerSearchMask(pHisPool);
        SetOrganizerSearchMask(pMyPool);
        SfxStyleSheetBase* pHisSheet = NULL;

        if ( pHisPool && pHisPool->Count() > nSourceIdx2 )
            pHisSheet = (*pHisPool)[nSourceIdx2];

        // Einfuegen ist nur dann noetig, wenn ein StyleSheet
        // zwischen unterschiedlichen(!) Pools bewegt wird

        if ( pHisSheet && pMyPool != pHisPool )
        {
            if (INDEX_IGNORE == nIdx2)
            {
                nIdx2 = pMyPool->Count();
            }

            // wenn so eine Vorlage schon existiert: loeschen!
            String aOldName(pHisSheet->GetName());
            SfxStyleFamily eOldFamily = pHisSheet->GetFamily();

            SfxStyleSheetBase* pExist = pMyPool->Find(aOldName, eOldFamily);
            // sal_uInt16 nOldHelpId = pExist->GetHelpId(??? VB ueberlegt sich was);
            sal_Bool bUsedOrUserDefined;
            if( pExist )
            {
                bUsedOrUserDefined =
                    pExist->IsUsed() || pExist->IsUserDefined();
                if( ErrorHandler::HandleError(
                    *new MessageInfo( ERRCODE_SFXMSG_STYLEREPLACE, aOldName ) )
                    != ERRCODE_BUTTON_OK )
                    return sal_False;
                else
                {
                    pMyPool->Replace( *pHisSheet, *pExist );
                    SetModified( sal_True );
                    nIdx2 = nIdx1 = INDEX_IGNORE;
                    return sal_True;
                }
            }

            SfxStyleSheetBase& rNewSheet = pMyPool->Make(
                aOldName, eOldFamily,
                pHisSheet->GetMask(), nIdx2);

            // ItemSet der neuen Vorlage fuellen
            rNewSheet.GetItemSet().Set(pHisSheet->GetItemSet());

            // wer bekommt den Neuen als Parent? wer benutzt den Neuen als Follow?
            SfxStyleSheetBase* pTestSheet = pMyPool->First();
            while (pTestSheet)
            {
                if (pTestSheet->GetFamily() == eOldFamily &&
                    pTestSheet->HasParentSupport() &&
                    pTestSheet->GetParent() == aOldName)
                {
                    pTestSheet->SetParent(aOldName);
                    // Verknuepfung neu aufbauen
                }

                if (pTestSheet->GetFamily() == eOldFamily &&
                    pTestSheet->HasFollowSupport() &&
                    pTestSheet->GetFollow() == aOldName)
                {
                    pTestSheet->SetFollow(aOldName);
                    // Verknuepfung neu aufbauen
                }

                pTestSheet = pMyPool->Next();
            }
            bUsedOrUserDefined =
                rNewSheet.IsUsed() || rNewSheet.IsUserDefined();


            // hat der Neue einen Parent? wenn ja, mit gleichem Namen bei uns suchen
            if (pHisSheet->HasParentSupport())
            {
                const String& rParentName = pHisSheet->GetParent();
                if (0 != rParentName.Len())
                {
                    SfxStyleSheetBase* pParentOfNew =
                        pMyPool->Find(rParentName, eOldFamily);
                    if (pParentOfNew)
                        rNewSheet.SetParent(rParentName);
                }
            }

            // hat der Neue einen Follow? wenn ja, mit gleichem
            // Namen bei uns suchen
            if (pHisSheet->HasFollowSupport())
            {
                const String& rFollowName = pHisSheet->GetFollow();
                if (0 != rFollowName.Len())
                {
                    SfxStyleSheetBase* pFollowOfNew =
                        pMyPool->Find(rFollowName, eOldFamily);
                    if (pFollowOfNew)
                        rNewSheet.SetFollow(rFollowName);
                }
            }

            SetModified( sal_True );
            if( !bUsedOrUserDefined ) nIdx2 = nIdx1 = INDEX_IGNORE;

            bRet = sal_True;
        }
        else
            bRet = sal_False;
    }
/*
    else if (nSourceIdx1 == CONTENT_CONFIG)
    {
        nIdx1 = CONTENT_CONFIG;

        SfxConfigManager *pCfgMgr = SFX_CFGMANAGER();
        if ( !GetConfigManager() )
        {
            SetConfigManager(new SfxConfigManager(0, pCfgMgr));
            SetTemplateConfig(sal_False);
            if (this == Current())
                GetConfigManager()->Activate(pCfgMgr);
        }

        if (GetConfigManager()->CopyItem(
            nSourceIdx2, nIdx2, rSource.GetConfigManager()))
        {
            SetModified(sal_True);
            bRet = sal_True;
            SFX_APP()->GetDispatcher_Impl()->Update_Impl(sal_True);
        }
    }
*/
    return bRet;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::Remove
(
    sal_uInt16 nIdx1,
    sal_uInt16 nIdx2,
    sal_uInt16 /*nIdx3*/
)
{
    sal_Bool bRet = sal_False;

    if (CONTENT_STYLE == nIdx1)
    {
        SfxStyleSheetBasePool* pMyPool  = GetStyleSheetPool();

        SetOrganizerSearchMask(pMyPool);

        SfxStyleSheetBase* pMySheet =  (*pMyPool)[nIdx2];
        String aName(pMySheet->GetName());
        String aEmpty;
        SfxStyleFamily  eFamily = pMySheet->GetFamily();
        pMyPool->Remove(pMySheet);
        bRet = sal_True;

        SfxStyleSheetBase* pTestSheet = pMyPool->First();
        while (pTestSheet)
        {
            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasParentSupport() &&
                pTestSheet->GetParent() == aName)
            {
                pTestSheet->SetParent(aEmpty); // Verknuepfung aufloesen
            }

            if (pTestSheet->GetFamily() == eFamily &&
                pTestSheet->HasFollowSupport() &&
                pTestSheet->GetFollow() == aName)
            {
                pTestSheet->SetFollow(aEmpty); // Verknuepfung aufloesen
            }

            pTestSheet = pMyPool->Next();
        }

        SetModified( sal_True );
    }

    return bRet;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::Print
(
    Printer&        rPrt,
    sal_uInt16          nIdx1,
    sal_uInt16          /*nIdx2*/,
    sal_uInt16          /*nIdx3*/,
    const String*   pObjectName
)

/*  [Beschreibung]
*/

{
    switch(nIdx1)
    {
      case CONTENT_STYLE:
        {
            SfxStyleSheetBasePool *pStylePool = GetStyleSheetPool();
            SetOrganizerSearchMask(pStylePool);
            SfxStyleSheetIterator* pIter = pStylePool->CreateIterator(
                pStylePool->GetSearchFamily(), pStylePool->GetSearchMask() );
            sal_uInt16 nStyles = pIter->Count();
            SfxStyleSheetBase *pStyle = pIter->First();
            if ( !pStyle )
                return sal_True;

            // pepare adaptor for old style StartPage/EndPage printing
            boost::shared_ptr< Printer > pPrinter( new Printer( rPrt.GetJobSetup() ) );
            vcl::OldStylePrintAdaptor* pAdaptor = new vcl::OldStylePrintAdaptor( pPrinter );
            boost::shared_ptr< vcl::PrinterController > pController( pAdaptor );

            pAdaptor->StartPage();

            pPrinter->SetMapMode(MapMode(MAP_10TH_MM));
            Font aFont( DEFINE_CONST_UNICODE( "Arial" ), Size(0, 64));   // 18pt
            aFont.SetWeight(WEIGHT_BOLD);
            pPrinter->SetFont(aFont);
            const Size aPageSize(pPrinter->GetOutputSize());
            const sal_uInt16 nXIndent = 200;
            sal_uInt16 nYIndent = 200;
            Point aOutPos(nXIndent, nYIndent);
            String aHeader(SfxResId(STR_PRINT_STYLES_HEADER));
            if ( pObjectName )
                aHeader += *pObjectName;
            else
                aHeader += GetTitle();
            long nTextHeight( pPrinter->GetTextHeight() );
            pPrinter->DrawText(aOutPos, aHeader);
            aOutPos.Y() += nTextHeight;
            aOutPos.Y() += nTextHeight/2;
            aFont.SetSize(Size(0, 35)); // 10pt
            nStyles = 1;
            while(pStyle)
            {
                // print template name
                String aStr(pStyle->GetName());
                aFont.SetWeight(WEIGHT_BOLD);
                pPrinter->SetFont(aFont);
                nTextHeight = pPrinter->GetTextHeight();
                // check for new page
                if ( aOutPos.Y() + nTextHeight*2 >
                    aPageSize.Height() - (long) nYIndent )
                {
                    pAdaptor->EndPage();
                    pAdaptor->StartPage();
                    aOutPos.Y() = nYIndent;
                }
                pPrinter->DrawText(aOutPos, aStr);
                aOutPos.Y() += nTextHeight;

                // print template description
                aFont.SetWeight(WEIGHT_NORMAL);
                pPrinter->SetFont(aFont);
                aStr = pStyle->GetDescription();
                const char cDelim = ' ';
                sal_uInt16 nStart = 0, nIdx = 0;

                nTextHeight = pPrinter->GetTextHeight();
                // break text into lines
                while(nIdx < aStr.Len())
                {
                    sal_uInt16  nOld = nIdx;
                    long nTextWidth;
                    nIdx = aStr.Search(cDelim, nStart);
                    nTextWidth = pPrinter->GetTextWidth(aStr, nStart, nIdx-nStart);
                    while(nIdx != STRING_NOTFOUND &&
                          aOutPos.X() + nTextWidth <
                          aPageSize.Width() - (long) nXIndent)
                    {
                        nOld = nIdx;
                        nIdx = aStr.Search(cDelim, nIdx+1);
                        nTextWidth = pPrinter->GetTextWidth(aStr, nStart, nIdx-nStart);
                    }
                    String aTmp(aStr, nStart, nIdx == STRING_NOTFOUND?
                                STRING_LEN :
                                nOld-nStart);
                    if ( aTmp.Len() )
                    {
                        nStart = nOld+1;    // trailing space
                    }
                    else
                    {
                        sal_uInt16 nChar = 1;
                        while(
                            nStart + nChar < aStr.Len() &&
                            aOutPos.X() + pPrinter->GetTextWidth(
                                aStr, nStart, nChar) <
                            aPageSize.Width() - nXIndent)
                            ++nChar;
                        aTmp = String(aStr, nStart, nChar-1);
                        nIdx = nStart + nChar;
                        nStart = nIdx;
                    }
                    if ( aOutPos.Y() + nTextHeight*2 >
                        aPageSize.Height() - nYIndent )
                    {
                        pAdaptor->EndPage();
                        pAdaptor->StartPage();
                        aOutPos.Y() = nYIndent;
                    }
                    pPrinter->DrawText(aOutPos, aTmp);
                    aOutPos.Y() += pPrinter->GetTextHeight();
                }
                pStyle = pIter->Next();
            }
            pAdaptor->EndPage();

            Printer::PrintJob( pController, rPrt.GetJobSetup() );

            delete pIter;
            break;
        }
      default:
          return sal_False;
    }
    return sal_True;
}

//--------------------------------------------------------------------

void SfxObjectShell::LoadStyles
(
    SfxObjectShell &rSource         /*  die Dokument-Vorlage, aus der
                                            die Styles geladen werden sollen */
)

/*  [Beschreibung]

    Diese Methode wird vom SFx gerufen, wenn aus einer Dokument-Vorlage
    Styles nachgeladen werden sollen. Bestehende Styles soll dabei
    "uberschrieben werden. Das Dokument mu"s daher neu formatiert werden.
    Daher werden die Applikationen in der Regel diese Methode "uberladen
    und in ihrer Implementierung die Implementierung der Basisklasse
    rufen.
*/

{
    struct Styles_Impl
    {
        SfxStyleSheetBase *pSource;
        SfxStyleSheetBase *pDest;
//      Styles_Impl () : pSource(0), pDest(0) {}
    };

    SfxStyleSheetBasePool *pSourcePool = rSource.GetStyleSheetPool();
    DBG_ASSERT(pSourcePool, "Source-DocumentShell ohne StyleSheetPool");
    SfxStyleSheetBasePool *pMyPool = GetStyleSheetPool();
    DBG_ASSERT(pMyPool, "Dest-DocumentShell ohne StyleSheetPool");
    pSourcePool->SetSearchMask(SFX_STYLE_FAMILY_ALL, 0xffff);
    Styles_Impl *pFound = new Styles_Impl[pSourcePool->Count()];
    sal_uInt16 nFound = 0;

    SfxStyleSheetBase *pSource = pSourcePool->First();
    while ( pSource )
    {
        SfxStyleSheetBase *pDest =
            pMyPool->Find( pSource->GetName(), pSource->GetFamily() );
        if ( !pDest )
        {
            pDest = &pMyPool->Make( pSource->GetName(),
                    pSource->GetFamily(), pSource->GetMask());
            // Setzen des Parents, der Folgevorlage
        }
        pFound[nFound].pSource = pSource;
        pFound[nFound].pDest = pDest;
        ++nFound;
        pSource = pSourcePool->Next();
    }

    for ( sal_uInt16 i = 0; i < nFound; ++i )
    {
        pFound[i].pDest->GetItemSet().PutExtended(pFound[i].pSource->GetItemSet(), SFX_ITEM_DONTCARE, SFX_ITEM_DEFAULT);
//      pFound[i].pDest->SetHelpId(pFound[i].pSource->GetHelpId());
        if(pFound[i].pSource->HasParentSupport())
            pFound[i].pDest->SetParent(pFound[i].pSource->GetParent());
        if(pFound[i].pSource->HasFollowSupport())
            pFound[i].pDest->SetFollow(pFound[i].pSource->GetParent());
    }
    delete [] pFound;
}

//--------------------------------------------------------------------

void SfxObjectShell::UpdateFromTemplate_Impl(  )

/*  [Beschreibung]

    Diese interne Methode pr"uft, ob das Dokument aus einem Template
    erzeugt wurde, und ob dieses neuer ist als das Dokument. Ist dies
    der Fall, wird der Benutzer gefragt, ob die Vorlagen (StyleSheets)
    updated werden sollen. Wird dies positiv beantwortet, werden die
    StyleSheets updated.
*/

{
    // Storage-medium?
    SfxMedium *pFile = GetMedium();
    DBG_ASSERT( pFile, "cannot UpdateFromTemplate without medium" );
    if ( !pFile )
        return;

    if ( !::utl::LocalFileHelper::IsLocalFile( pFile->GetName() ) )
        // update only for documents loaded from the local file system
        return;

    // only for own storage formats
    uno::Reference< embed::XStorage > xDocStor = pFile->GetStorage();
    if ( !pFile->GetFilter() || !pFile->GetFilter()->IsOwnFormat() )
        return;

    SFX_ITEMSET_ARG( pFile->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
    sal_Int16 bCanUpdateFromTemplate = pUpdateDocItem ? pUpdateDocItem->GetValue() : document::UpdateDocMode::NO_UPDATE;

    // created from template?
    uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
    ::rtl::OUString aTemplName( xDocProps->getTemplateName() );
    ::rtl::OUString aTemplURL( xDocProps->getTemplateURL() );
    String aFoundName;

    if ( aTemplName.getLength() || (aTemplURL.getLength() && !IsReadOnly()) )
    {
        // try to locate template, first using filename
        // this must be done because writer global document uses this "great" idea to manage the templates of all parts
        // in the master document
        // but it is NOT an error if the template filename points not to a valid file
        SfxDocumentTemplates aTempl;
        aTempl.Construct();
        if ( aTemplURL.getLength() )
        {
            String aURL;
            if( ::utl::LocalFileHelper::ConvertSystemPathToURL( aTemplURL, GetMedium()->GetName(), aURL ) )
                aFoundName = aURL;
        }

        if( !aFoundName.Len() && aTemplName.getLength() )
            // if the template filename did not lead to success, try to get a file name for the logical template name
            aTempl.GetFull( String(), aTemplName, aFoundName );
    }

    if ( aFoundName.Len() )
    {
        // check existence of template storage
        aTemplURL = aFoundName;
        sal_Bool bLoad = sal_False;

        // should the document checked against changes in the template ?
        if ( IsQueryLoadTemplate() )
        {
            // load document info of template
            sal_Bool bOK = sal_False;
            util::DateTime aTemplDate;
            try
            {
                Reference < document::XStandaloneDocumentInfo > xDocInfo (
                    ::comphelper::getProcessServiceFactory()->createInstance(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.document.StandaloneDocumentInfo") ) ),
                    UNO_QUERY_THROW );
                Reference < beans::XFastPropertySet > xSet( xDocInfo,
                    UNO_QUERY_THROW );
                xDocInfo->loadFromURL( aTemplURL );
                Any aAny = xSet->getFastPropertyValue( WID_DATE_MODIFIED );
                ::com::sun::star::util::DateTime aTmp;
                if ( aAny >>= aTemplDate )
                {
                    // get modify date from document info
                    bOK = sal_True;
                }
            }
            catch ( Exception& )
            {
            }

            // if modify date was read successfully
            if ( bOK )
            {
                // compare modify data of template with the last check date of the document
                const util::DateTime aInfoDate( xDocProps->getTemplateDate() );
                if ( aTemplDate > aInfoDate )
                {
                    // ask user
                    if( bCanUpdateFromTemplate == document::UpdateDocMode::QUIET_UPDATE
                    || bCanUpdateFromTemplate == document::UpdateDocMode::FULL_UPDATE )
                        bLoad = sal_True;
                    else if ( bCanUpdateFromTemplate == document::UpdateDocMode::ACCORDING_TO_CONFIG )
                    {
                        String sMessage( SfxResId( STR_QRYTEMPL_MESSAGE ) );
                        sMessage.SearchAndReplace( String::CreateFromAscii("$(ARG1)"), aTemplName );
                        sfx2::QueryTemplateBox aBox( GetDialogParent(), sMessage );
                        if ( RET_YES == aBox.Execute() )
                            bLoad = sal_True;
                    }

                    if( !bLoad )
                    {
                        // user refuses, so don't ask again for this document
                        SetQueryLoadTemplate(sal_False);
                        SetModified( sal_True );
                    }
                }
            }

            if ( bLoad )
            {
                // styles should be updated, create document in organizer mode to read in the styles
                //TODO: testen!
                SfxObjectShellLock xTemplDoc = CreateObjectByFactoryName( GetFactory().GetFactoryName(), SFX_CREATE_MODE_ORGANIZER );
                xTemplDoc->DoInitNew(0);

                // TODO/MBA: do we need a BaseURL? Then LoadFrom must be extended!
                //xTemplDoc->SetBaseURL( aFoundName );

                // TODO/LATER: make sure that we don't use binary templates!
                SfxMedium aMedium( aFoundName, STREAM_STD_READ );
                if ( xTemplDoc->LoadFrom( aMedium ) )
                {
                    // transfer styles from xTemplDoc to this document
                    // TODO/MBA: make sure that no BaseURL is needed in *this* document
                    LoadStyles(*xTemplDoc);

                    // remember date/time of check
                    xDocProps->setTemplateDate(aTemplDate);
                    // TODO/LATER: new functionality to store document info is required ( didn't work for SO7 XML format )
//REPLACE                   pInfo->Save(xDocStor);
                }
            }
        }
    }
}

sal_Bool SfxObjectShell::IsHelpDocument() const
{
    const SfxFilter* pFilter = GetMedium()->GetFilter();
    return ( pFilter && pFilter->GetFilterName().CompareToAscii("writer_web_HTML_help") == COMPARE_EQUAL );
}

void SfxObjectShell::ResetFromTemplate( const String& rTemplateName, const String& rFileName )
{
    // only care about reseting this data for openoffice formats otherwise
    if ( IsOwnStorageFormat_Impl( *GetMedium())  )
    {
        uno::Reference<document::XDocumentProperties> xDocProps(getDocProperties());
        xDocProps->setTemplateURL( ::rtl::OUString() );
        xDocProps->setTemplateName( ::rtl::OUString() );
        xDocProps->setTemplateDate( util::DateTime() );
        xDocProps->resetUserData( ::rtl::OUString() );

        // TODO/REFACTOR:
        // Title?

        if( ::utl::LocalFileHelper::IsLocalFile( rFileName ) )
        {
            String aFoundName;
            if( SFX_APP()->Get_Impl()->GetDocumentTemplates()->GetFull( String(), rTemplateName, aFoundName ) )
            {
                INetURLObject aObj( rFileName );
                xDocProps->setTemplateURL( aObj.GetMainURL(INetURLObject::DECODE_TO_IURI) );
                xDocProps->setTemplateName( rTemplateName );

                ::DateTime now;
                xDocProps->setTemplateDate( util::DateTime(
                    now.Get100Sec(), now.GetSec(), now.GetMin(),
                    now.GetHour(), now.GetDay(), now.GetMonth(),
                    now.GetYear() ) );

                SetQueryLoadTemplate( sal_True );
            }
        }
    }
}

sal_Bool SfxObjectShell::IsQueryLoadTemplate() const
{
    return pImp->bQueryLoadTemplate;
}

sal_Bool SfxObjectShell::IsUseUserData() const
{
    return pImp->bUseUserData;
}

void SfxObjectShell::SetQueryLoadTemplate( sal_Bool bNew )
{
    if ( pImp->bQueryLoadTemplate != bNew )
        SetModified( sal_True );
    pImp->bQueryLoadTemplate = bNew;
}

void SfxObjectShell::SetUseUserData( sal_Bool bNew )
{
    if ( pImp->bUseUserData != bNew )
        SetModified( sal_True );
    pImp->bUseUserData = bNew;
}

sal_Bool SfxObjectShell::IsLoadReadonly() const
{
    return pImp->bLoadReadonly;
}

sal_Bool SfxObjectShell::IsSaveVersionOnClose() const
{
    return pImp->bSaveVersionOnClose;
}

void SfxObjectShell::SetLoadReadonly( sal_Bool bNew )
{
    if ( pImp->bLoadReadonly != bNew )
        SetModified( sal_True );
    pImp->bLoadReadonly = bNew;
}

void SfxObjectShell::SetSaveVersionOnClose( sal_Bool bNew )
{
    if ( pImp->bSaveVersionOnClose != bNew )
        SetModified( sal_True );
    pImp->bSaveVersionOnClose = bNew;
}

sal_uInt32 SfxObjectShell::GetModifyPasswordHash() const
{
    return pImp->m_nModifyPasswordHash;
}

sal_Bool SfxObjectShell::SetModifyPasswordHash( sal_uInt32 nHash )
{
    if ( ( !IsReadOnly() && !IsReadOnlyUI() )
      || !(pImp->nFlagsInProgress & SFX_LOADED_MAINDOCUMENT ) )
    {
        // the hash can be changed only in editable documents,
        // or during loading of document
        pImp->m_nModifyPasswordHash = nHash;
        return sal_True;
    }

    return sal_False;
}

uno::Sequence< beans::PropertyValue > SfxObjectShell::GetModifyPasswordInfo() const
{
    return pImp->m_aModifyPasswordInfo;
}

sal_Bool SfxObjectShell::SetModifyPasswordInfo( const uno::Sequence< beans::PropertyValue >& aInfo )
{
    if ( ( !IsReadOnly() && !IsReadOnlyUI() )
      || !(pImp->nFlagsInProgress & SFX_LOADED_MAINDOCUMENT ) )
    {
        // the hash can be changed only in editable documents,
        // or during loading of document
        pImp->m_aModifyPasswordInfo = aInfo;
        return sal_True;
    }

    return sal_False;
}

void SfxObjectShell::SetModifyPasswordEntered( sal_Bool bEntered )
{
    pImp->m_bModifyPasswordEntered = bEntered;
}

sal_Bool SfxObjectShell::IsModifyPasswordEntered()
{
    return pImp->m_bModifyPasswordEntered;
}

