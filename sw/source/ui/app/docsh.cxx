/*************************************************************************
 *
 *  $RCSfile: docsh.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:21:16 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_JOBSET_HXX //autogen
#include <vcl/jobset.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _PASSWD_HXX
#include <sfx2/passwd.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX //autogen
#include <sfx2/evntconf.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVXMSBAS_HXX
#include <svx/svxmsbas.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _OFA_HTMLCFG_HXX //autogen
#include <offmgr/htmlcfg.hxx>
#endif
#ifndef _OFA_FLTRCFG_HXX
#include <offmgr/fltrcfg.hxx>
#endif
#ifndef _SOERR_HXX
#include <so3/soerr.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
//#ifndef _SB_SBJSMOD_HXX //autogen
//#include <basic/sbjsmod.hxx>
//#endif

#ifndef _SWEVENT_HXX //autogen
#include <swevent.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif

#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>         // fuer die aktuelle Sicht
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>        // Verbindung zur Core
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>        // Dokumenterzeugung
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _GLOBDOC_HXX
#include <globdoc.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>      // I/O
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>        // I/O, Hausformat
#endif
#ifndef _DOCSTYLE_HXX
#include <docstyle.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DATAEX_HXX
#include <dataex.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _PVIEW_HXX
#include <pview.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _SWBASLNK_HXX
#include <swbaslnk.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _ASCFLDLG_HXX
#include <ascfldlg.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>        // Fehlermeldungen
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>          //
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif

using namespace rtl;
using namespace ::com::sun::star::uno;

extern BOOL bNotLoadLayout;


#define SwDocShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif


class SwTmpPersist : public SvPersist
{
    SwDocShell* pDShell;
    virtual void FillClass( SvGlobalName * pClassName,
                            ULONG * pClipFormat,
                            String * pAppName,
                            String * pLongUserName,
                            String * pUserName,
                            long nFileFormat=SOFFICE_FILEFORMAT_CURRENT ) const;
    virtual BOOL Save();
    virtual BOOL SaveCompleted( SvStorage * );
public:
    SwTmpPersist( SwDocShell& rDSh ) : pDShell( &rDSh ) {}
};


SFX_IMPL_INTERFACE( SwDocShell, SfxObjectShell, SW_RES(0) )
{
    SwGlobalDocShell::Factory().RegisterHelpFile(String::CreateFromAscii("swriter.svh"));
    SwGlobalDocShell::Factory().RegisterHelpPIFile(String::CreateFromAscii("swriter.svh"));
}

/*--------------------------------------------------------------------
    Beschreibung:   Aller Filter registrieren
 --------------------------------------------------------------------*/

SFX_IMPL_OBJECTFACTORY_DLL(SwDocShell, SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU,  \
//swriter3  0xDC5C7E40L, 0xB35C, 0x101B, 0x99, 0x61, 0x04, 0x02, 0x1C, 0x00, 0x70,0x02)
//swriter4  0x8b04e9b0,  0x420e, 0x11d0, 0xa4, 0x5e, 0x0,  0xa0, 0x24, 0x9d, 0x57,0xb1, Sw)
//swriter5  0xc20cf9d1, 0x85ae, 0x11d1, 0xaa, 0xb4, 0x0, 0x60, 0x97, 0xda, 0x56, 0x1a
  SvGlobalName(SO3_SW_CLASSID), Sw)      /*swriter5,*/

/*{
    ::RegisterFilterInSfxFactory( (SfxObjectFactory&)Factory(), RC_DOC_ICON );
}
  */


TYPEINIT2(SwDocShell, SfxObjectShell, SfxListener);

/*--------------------------------------------------------------------
    Beschreibung: Laden vorbereiten
 --------------------------------------------------------------------*/


Reader* SwDocShell::StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
                                    SwCrsrShell *pCrsrShell,
                                    SwPaM* pPaM )
{
    BOOL bAPICall = FALSE;
    const SfxPoolItem* pApiItem;
    const SfxItemSet* pMedSet;
    if( 0 != ( pMedSet = rMedium.GetItemSet() ) && SFX_ITEM_SET ==
            pMedSet->GetItemState( FN_API_CALL, TRUE, &pApiItem ) )
            bAPICall = ((const SfxBoolItem*)pApiItem)->GetValue();

    const SfxFilter* pFlt = rMedium.GetFilter();
    if( !pFlt )
    {
        if(!bAPICall)
        {
            InfoBox( 0, SW_RESSTR(STR_CANTOPEN)).Execute();
        }
        return 0;
    }
    String aFileName( rMedium.GetName() );
    SwRead pRead = SwIoSystem::GetReader( pFlt->GetUserData() );
    if( !pRead )
        return 0;

    if( rMedium.IsStorage()
        ? SW_STORAGE_READER & pRead->GetReaderType()
        : SW_STREAM_READER & pRead->GetReaderType() )
    {
        *ppRdr = pPaM ? new SwReader( rMedium, aFileName, *pPaM ) :
            pCrsrShell ?
                new SwReader( rMedium, aFileName, *pCrsrShell->GetCrsr() )
                    : new SwReader( rMedium, aFileName, pDoc );
    }
    else
        return 0;

    // PassWord Checken
    String aPasswd;
    if ((*ppRdr)->NeedsPasswd( *pRead ))
    {
        if(!bAPICall)
        {
            SfxPasswordDialog* pPasswdDlg =
                    new SfxPasswordDialog( 0 );
                if(RET_OK == pPasswdDlg->Execute())
                    aPasswd = pPasswdDlg->GetPassword();
        }
        else
        {
            const SfxItemSet* pSet = rMedium.GetItemSet();
            const SfxPoolItem *pPassItem;
            if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, TRUE, &pPassItem))
                aPasswd = ((const SfxStringItem *)pPassItem)->GetValue();
        }

        if (!(*ppRdr)->CheckPasswd( aPasswd, *pRead ))
        {
            InfoBox( 0, SW_RES(MSG_ERROR_PASSWD)).Execute();
                delete *ppRdr;
//JP: SFX-Aenderung - kein close rufen
//            if( !rMedium.IsStorage() )
//                rMedium.CloseInStream();
            return 0;
        }
    }
    if(rMedium.IsStorage())
    {
        SvStorageRef aStor( rMedium.GetStorage() );
        const SfxItemSet* pSet = rMedium.GetItemSet();
        const SfxPoolItem *pItem;
        if(pSet && SFX_ITEM_SET == pSet->GetItemState(SID_PASSWORD, TRUE, &pItem))
        {
            DBG_ASSERT(pItem->IsA( TYPE(SfxStringItem) ), "Fehler Parametertype");
            ByteString aPasswd( ((const SfxStringItem *)pItem)->GetValue(),
                                gsl_getSystemTextEncoding() );
            aStor->SetKey( aPasswd );
        }
        // Fuer's Dokument-Einfuegen noch die FF-Version, wenn's der
        // eigene Filter ist.
        ASSERT( pRead != ReadSw3 || pRead != ReadXML || pFlt->GetVersion(),
                "Am Filter ist keine FF-Version gesetzt" );
        if( (pRead == ReadSw3 || pRead == ReadXML) && pFlt->GetVersion() )
            aStor->SetVersion( (long)pFlt->GetVersion() );
    }
    // beim Sw3-Reader noch den pIo-Pointer setzen
    if( pRead == ReadSw3 )
        ((Sw3Reader*)pRead)->SetSw3Io( pIo );

    if( pFlt->GetDefaultTemplate().Len() )
        pRead->SetTemplateName( pFlt->GetDefaultTemplate() );

    if( pRead == ReadAscii && 0 != rMedium.GetInStream() &&
        pFlt->GetUserData().EqualsAscii( FILTER_TEXT_DLG ) )
    {
        SwAsciiOptions aOpt;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) && SFX_ITEM_SET ==
            pSet->GetItemState( SID_FILE_FILTEROPTIONS, TRUE, &pItem ) )
            aOpt.ReadUserData( ((const SfxStringItem*)pItem)->GetValue() );
        else if(!bAPICall)
        {
            SwAsciiFilterDlg* pDlg = new SwAsciiFilterDlg( 0, *this,
                                                    rMedium.GetInStream() );

            if( RET_OK == pDlg->Execute() )
                // get the options and put it to the reader
                pDlg->FillOptions( aOpt );
            else
            {
                pRead = 0;
                SetError( ERRCODE_IO_ABORT );   // silent abort
            }
            delete pDlg;
        }

        if( pRead )
            pRead->GetReaderOpt().SetASCIIOpts( aOpt );
    }

    return pRead;
}

/*--------------------------------------------------------------------
    Beschreibung: Laden
 --------------------------------------------------------------------*/

BOOL SwDocShell::ConvertFrom( SfxMedium& rMedium )
{
    SwReader* pRdr;
    SwRead pRead = StartConvertFrom(rMedium, &pRdr);
    if (!pRead)
        return FALSE;

    SwWait aWait( *this, TRUE );

        // SfxProgress unterdruecken, wenn man Embedded ist
    SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

    // fuer MD
    BOOL bSave = bNotLoadLayout;
    bNotLoadLayout = BOOL( !LoadLayout() );
#ifndef PRODUCT
    BOOL bWeb = 0 != PTR_CAST(SwWebDocShell, this);
    bNotLoadLayout |= SW_MOD()->GetUsrPref(bWeb)->IsTest1();
#endif

    pRdr->GetDoc()->SetHTMLMode( ISA(SwWebDocShell) );

    ULONG nErr = pRdr->Read( *pRead );
    bNotLoadLayout = bSave;
    // Evtl. ein altes Doc weg
    if( pDoc )
        RemoveLink();
    pDoc = pRdr->GetDoc();

    // die DocInfo vom Doc am DocShell-Medium setzen
    if( GetMedium()->GetFilter() &&
        GetMedium()->GetFilter()->UsesStorage() )
    {
        SvStorageRef aRef = GetMedium()->GetStorage();
        if( aRef.Is() )
            pDoc->GetInfo()->Save(aRef);
    }

    AddLink();

    ASSERT( !pBasePool, "wer hat seinen Pool nicht zerstoert?" );
    pBasePool = new SwDocStyleSheetPool( *pDoc,
                        SFX_CREATE_MODE_ORGANIZER == GetCreateMode() );
    UpdateFontList();
    InitDraw();

    delete pRdr;

    SW_MOD()->SetEmbeddedLoadSave( FALSE );

    SetError( nErr );
    BOOL bOk = !IsError( nErr );

    // StartFinishedLoading rufen. Nicht bei asynchronen Filtern!
    // Diese muessen das selbst rufen!
    if( bOk && !pDoc->IsInLoadAsynchron() )
        StartLoadFinishedTimer();

    return bOk;
}

/*--------------------------------------------------------------------
    Beschreibung: Sichern des Default-Formats, Stg vorhanden
 --------------------------------------------------------------------*/


BOOL SwDocShell::Save()
{
    sal_Bool bXML = pIo->GetStorage()->GetVersion() >= SOFFICE_FILEFORMAT_60;

    SwWait aWait( *this, TRUE );
    ULONG nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
    if( SfxInPlaceObject::Save() )
    {
        switch( GetCreateMode() )
        {
        case SFX_CREATE_MODE_INTERNAL:
            nErr = 0;
            break;

        case SFX_CREATE_MODE_ORGANIZER:
            if( bXML )
            {
                WriterRef xWrt;
                ::GetXMLWriter( aEmptyStr, xWrt );
                xWrt->SetOrganizerMode( TRUE );
                SwWriter aWrt( *pIo->GetStorage(), *pDoc );
                nErr = aWrt.Write( xWrt );
                xWrt->SetOrganizerMode( FALSE );
            }
            else
                nErr = pIo->SaveStyles();
            break;

        case SFX_CREATE_MODE_EMBEDDED:
            // SfxProgress unterdruecken, wenn man Embedded ist
            SW_MOD()->SetEmbeddedLoadSave( TRUE );
            // kein break;

        case SFX_CREATE_MODE_STANDARD:
        case SFX_CREATE_MODE_PREVIEW:
        default:
            {
                if( pDoc->ContainsMSVBasic() )
                {
                    SvxImportMSVBasic aTmp( *this, *pIo->GetStorage() );
                    aTmp.SaveOrDelMSVBAStorage( FALSE, aEmptyStr );
                    if( OFF_APP()->GetFilterOptions()->IsLoadWordBasicStorage() )
                        nVBWarning = SvxImportMSVBasic::
                                        GetSaveWarningOfMSVBAStorage( *this );
                    pDoc->SetContainsMSVBasic( FALSE );
                }

                if( !bXML &&
                    !ISA( SwGlobalDocShell ) && !ISA( SwWebDocShell ) &&
                    SFX_CREATE_MODE_EMBEDDED != GetCreateMode() )
                    AddXMLAsZipToTheStorage( *pIo->GetStorage() );

                // TabellenBox Edit beenden!
                if( pWrtShell )
                    pWrtShell->EndAllTblBoxEdit();

                WriterRef xWrt;
                if( bXML )
                {
                    ::GetXMLWriter( aEmptyStr, xWrt );
                }
                else
                {
                    ::GetSw3Writer( aEmptyStr, xWrt );
                    ((Sw3Writer*)&xWrt)->SetSw3Io( pIo, FALSE );
                }

                SwWriter aWrt( *pIo->GetStorage(), *pDoc );
                nErr = aWrt.Write( xWrt );
            }
            break;
        }
        SW_MOD()->SetEmbeddedLoadSave( FALSE );
    }
    SetError( nErr ? nErr : nVBWarning );

    SfxViewFrame* pFrm = pWrtShell ? pWrtShell->GetView().GetViewFrame() : 0;
    if( pFrm )
    {
        pFrm->GetBindings().SetState( SfxStringItem( SID_DOC_MODIFIED, ' ' ));
    }
    return !IsError( nErr );
}

/*--------------------------------------------------------------------
    Beschreibung: Sichern im Defaultformat
 --------------------------------------------------------------------*/


BOOL SwDocShell::SaveAs( SvStorage * pStor )
{
    sal_Bool bXML = pStor->GetVersion() >= SOFFICE_FILEFORMAT_60;

    SwWait aWait( *this, TRUE );

    if( pDoc->IsGlobalDoc() && !pDoc->IsGlblDocSaveLinks() )
        RemoveOLEObjects();

    {
        // Task 75666 - is the Document imported by our Microsoft-Filters?
        const SfxFilter* pOldFilter = GetMedium()->GetFilter();
        if( pOldFilter &&
            ( pOldFilter->GetUserData().EqualsAscii( FILTER_WW8 ) ||
              pOldFilter->GetUserData().EqualsAscii( "CWW6" ) ||
              pOldFilter->GetUserData().EqualsAscii( "WW6" ) ||
              pOldFilter->GetUserData().EqualsAscii( "WW1" ) ))
        {
            // when saving it in our own fileformat, then remove the template
            // name from the docinfo.
            SfxDocumentInfo aInfo = GetDocInfo();
            if( aInfo.GetTemplateName().Len() ||
                aInfo.GetTemplateFileName().Len() )
            {
                aInfo.SetTemplateName( aEmptyStr );
                aInfo.SetTemplateFileName( aEmptyStr );
                SetDocumentInfo( aInfo );
            }
        }
    }

    ULONG nErr = ERR_SWG_WRITE_ERROR, nVBWarning = ERRCODE_NONE;
    if( SfxInPlaceObject::SaveAs( pStor ) )
    {
        if( GetDoc()->IsGlobalDoc() && !ISA( SwGlobalDocShell ) )
        {
            // This is to set the correct class id if SaveAs is
            // called from SwDoc::SplitDoc to save a normal doc as
            // global doc. In this case, SaveAs is called at a
            // normal doc shell, therefore, SfxInplaceObject::SaveAs
            // will set the wrong class id.
            SvGlobalName aClassName;
            ULONG nClipFormat;
            String aAppName, aLongUserName, aUserName;
            SfxObjectShellRef xDocSh =
                new SwGlobalDocShell( SFX_CREATE_MODE_INTERNAL );
            xDocSh->FillClass( &aClassName, &nClipFormat, &aAppName,
                                &aLongUserName, &aUserName,
                                pStor->GetVersion() );
            pStor->SetClass( aClassName, nClipFormat, aUserName );
        }

        if( pDoc->ContainsMSVBasic() )
        {
            SvxImportMSVBasic aTmp( *this, *pIo->GetStorage() );
            aTmp.SaveOrDelMSVBAStorage( FALSE, aEmptyStr );
                    if( OFF_APP()->GetFilterOptions()->IsLoadWordBasicStorage() )
                        nVBWarning = SvxImportMSVBasic::
                                        GetSaveWarningOfMSVBAStorage( *this );
            pDoc->SetContainsMSVBasic( FALSE );
        }

        if( !bXML && !ISA( SwGlobalDocShell ) && !ISA( SwWebDocShell ) &&
            SFX_CREATE_MODE_EMBEDDED != GetCreateMode() )
            AddXMLAsZipToTheStorage( *pStor );

        // TabellenBox Edit beenden!
        if( pWrtShell )
            pWrtShell->EndAllTblBoxEdit();

        // Modified-Flag merken und erhalten ohne den Link zu Callen
        // (fuer OLE; nach Anweisung von MM)
        BOOL bIsModified = pDoc->IsModified();
        Link aOldOLELnk( pDoc->GetOle2Link() );
        pDoc->SetOle2Link( Link() );

            // SfxProgress unterdruecken, wenn man Embedded ist
        SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode() );

        WriterRef xWrt;
        if( bXML )
        {
            ::GetXMLWriter( aEmptyStr, xWrt );
        }
        else
        {
            ::GetSw3Writer( aEmptyStr, xWrt );
            ((Sw3Writer*)&xWrt)->SetSw3Io( pIo, TRUE );
        }

        SwWriter aWrt( *pStor, *pDoc );
        nErr = aWrt.Write( xWrt );

        if( bIsModified )
            pDoc->SetModified();
        pDoc->SetOle2Link( aOldOLELnk );

        SW_MOD()->SetEmbeddedLoadSave( FALSE );
    }
    SetError( nErr ? nErr : nVBWarning );

    return !IsError( nErr );
}

/*--------------------------------------------------------------------
    Beschreibung: Sichern aller Formate
 --------------------------------------------------------------------*/
SwSrcView* lcl_GetSourceView( SwDocShell* pSh )
{
    // sind wir in der SourceView?
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst( pSh );
    SfxViewShell* pViewShell = pVFrame ? pVFrame->GetViewShell() : 0;
    return PTR_CAST( SwSrcView, pViewShell);
}

BOOL SwDocShell::ConvertTo( SfxMedium& rMedium )
{
    const SfxFilter* pFlt = rMedium.GetFilter();
    if( !pFlt )
        return FALSE;

    WriterRef xWriter;
    SwIoSystem::GetWriter( pFlt->GetUserData(), xWriter );
    if( !xWriter.Is() )
    {   // Der Filter ist nicht vorhanden
        InfoBox( 0,
                 SW_RESSTR(STR_DLLNOTFOUND) ).Execute();
        return FALSE;
    }

    ULONG nVBWarning = 0;
    if( pDoc->ContainsMSVBasic() )
    {
        BOOL bSave = pFlt->GetUserData().EqualsAscii( "CWW8" )
             && OFF_APP()->GetFilterOptions()->IsLoadWordBasicStorage();

        SvStorage* pStg;
        if( xWriter->IsStgWriter() )
            pStg = rMedium.GetStorage();
        else
            pStg = pIo->GetStorage();
        SvxImportMSVBasic aTmp( *this, *pStg );
        nVBWarning = aTmp.SaveOrDelMSVBAStorage( bSave,
                                String::CreateFromAscii("Macros") );
        pDoc->SetContainsMSVBasic( bSave );
    }

    // TabellenBox Edit beenden!
    if( pWrtShell )
        pWrtShell->EndAllTblBoxEdit();

    if( pFlt->GetUserData().EqualsAscii( "HTML") )
    {
        OfficeApplication* pOffApp = OFF_APP();
        OfaHtmlOptions* pHtmlOpt = pOffApp->GetHtmlOptions();
        if( !pHtmlOpt->IsStarBasic() && HasBasic())
        {
            BOOL bModules = FALSE;
            BasicManager *pBasicMan = GetBasicManager();

            for( USHORT i=0; i < pBasicMan->GetLibCount(); i++ )
            {
                StarBASIC *pBasic = pBasicMan->GetLib( i  );
                SbxArray *pModules = pBasic->GetModules();
                for( USHORT j = 0; j < pModules->Count(); j++ )
                {
                    const SbModule *pModule = PTR_CAST( SbModule, pModules->Get(j) );
                    bModules = TRUE;
                    break;
                }
            }

            if(bModules && pHtmlOpt->IsStarBasicWarning())
            {
                SetError(WARN_SWG_HTML_NO_MACROS);
            }
        }
        UpdateDocInfoForSave();
    }

    if( xWriter->IsStgWriter() &&
        ( xWriter->IsSw3Writer() ||
          pFlt->GetUserData().EqualsAscii( FILTER_XML ) ||
           pFlt->GetUserData().EqualsAscii( FILTER_XMLV ) ||
           pFlt->GetUserData().EqualsAscii( FILTER_XMLVW ) ) )
    {
        // eigenen Typ ermitteln
        BYTE nMyType = 0;
        if( ISA( SwWebDocShell) )
            nMyType = 1;
        else if( ISA( SwGlobalDocShell) )
            nMyType = 2;

        // gewuenschten Typ ermitteln
        BYTE nSaveType = 0;
        ULONG nSaveClipId = pFlt->GetFormat();
        if( SOT_FORMATSTR_ID_STARWRITERWEB_60 == nSaveClipId ||
            SOT_FORMATSTR_ID_STARWRITERWEB_50 == nSaveClipId ||
            SOT_FORMATSTR_ID_STARWRITERWEB_40 == nSaveClipId )
            nSaveType = 1;
        else if( SOT_FORMATSTR_ID_STARWRITERGLOB_60 == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_50 == nSaveClipId ||
                 SOT_FORMATSTR_ID_STARWRITERGLOB_40 == nSaveClipId )
            nSaveType = 2;

        // Flags am Dokument entsprechend umsetzen
        BOOL bIsHTMLModeSave = GetDoc()->IsHTMLMode();
        BOOL bIsGlobalDocSave = GetDoc()->IsGlobalDoc();
        BOOL bIsGlblDocSaveLinksSave = GetDoc()->IsGlblDocSaveLinks();
        if( nMyType != nSaveType )
        {
            GetDoc()->SetHTMLMode( 1 == nSaveType );
            GetDoc()->SetGlobalDoc( 2 == nSaveType );
            if( 2 != nSaveType )
                GetDoc()->SetGlblDocSaveLinks( FALSE );
        }

        // Jetzt das Dokument normal speichern
        BOOL bRet = SaveAs( rMedium.GetStorage() );

        if( nMyType != nSaveType )
        {
            GetDoc()->SetHTMLMode( bIsHTMLModeSave );
            GetDoc()->SetGlobalDoc( bIsGlobalDocSave );
            GetDoc()->SetGlblDocSaveLinks( bIsGlblDocSaveLinksSave );
        }

        if( bRet && nMyType != nSaveType )
        {
            SvGlobalName aClassName;
            ULONG nClipFormat;
            String aAppName, aLongUserName, aUserName;
            SfxObjectShellRef xDocSh;
            switch( nSaveType )
            {
            case 0:
                xDocSh = new SwDocShell( SFX_CREATE_MODE_INTERNAL );
                break;
            case 1:
                xDocSh = new SwWebDocShell( SFX_CREATE_MODE_INTERNAL );
                break;
            case 2:
                xDocSh = new SwGlobalDocShell( SFX_CREATE_MODE_INTERNAL );
                break;
            }
            xDocSh->FillClass( &aClassName, &nClipFormat, &aAppName,
                                &aLongUserName, &aUserName,
                                pFlt->GetVersion() );
            ASSERT( nClipFormat == nSaveClipId,
                    "FillClass hat falsche Clipboard-Id gesetzt" );
            rMedium.GetStorage()->SetClass( aClassName, nClipFormat,
                                            aUserName );
        }

        return bRet;
    }

    if( pFlt->GetUserData().EqualsAscii( FILTER_TEXT_DLG ) &&
        ( pWrtShell || !::lcl_GetSourceView( this ) ))
    {
        SwAsciiOptions aOpt;
        String sItemOpt;
        BOOL bShowDlg = FALSE;
        const SfxItemSet* pSet;
        const SfxPoolItem* pItem;
        if( 0 != ( pSet = rMedium.GetItemSet() ) )
        {
            if( SFX_ITEM_SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS,
                                                    TRUE, &pItem ) )
                sItemOpt = ((const SfxStringItem*)pItem)->GetValue();
            if( SFX_ITEM_SET == pSet->GetItemState( SID_USE_FILTEROPTIONS,
                                                    TRUE, &pItem ) )
                bShowDlg = ((const SfxBoolItem*)pItem)->GetValue();
        }

        if( bShowDlg || !sItemOpt.Len() )
        {
            SwAsciiFilterDlg* pDlg = new SwAsciiFilterDlg( 0, *this, 0 );
            int nDlg = pDlg->Execute();

            if( RET_OK == nDlg )
                // get the options for the writer
                pDlg->FillOptions( aOpt );

            delete pDlg;

            if( RET_OK != nDlg )
            {
                SetError( ERRCODE_IO_ABORT );   // silent abort
                return FALSE;
            }
        }
        else
            aOpt.ReadUserData( sItemOpt );

        xWriter->SetAsciiOptions( aOpt );
    }

        // SfxProgress unterdruecken, wenn man Embedded ist
    SW_MOD()->SetEmbeddedLoadSave(
                            SFX_CREATE_MODE_EMBEDDED == GetCreateMode());

    // Kontext aufspannen, um die Anzeige der Selektion zu unterbinden
    ULONG nErrno;
    String aFileName( rMedium.GetName() );

    //Keine View also das ganze Dokument!
    if ( pWrtShell )
    {
        SwWait aWait( *this, TRUE );
        pWrtShell->StartAllAction();
        pWrtShell->Push();
        SwWriter aWrt( rMedium, *pWrtShell, TRUE );
        nErrno = aWrt.Write( xWriter, &aFileName );
        //JP 16.05.97: falls der SFX uns die View waehrend des speicherns
        //              entzieht
        if( pWrtShell )
        {
            pWrtShell->Pop(FALSE);
            pWrtShell->EndAllAction();
        }
    }
    else
    {
        // sind wir in der SourceView?
        SwSrcView* pSrcView = ::lcl_GetSourceView( this );
        if( pSrcView )
        {
            pSrcView->SaveContentTo(rMedium);
            nErrno = 0;
        }
        else
        {
            SwWriter aWrt( rMedium, *pDoc );
            nErrno = aWrt.Write( xWriter, &aFileName );
        }
    }

    SW_MOD()->SetEmbeddedLoadSave( FALSE );
    SetError( nErrno ? nErrno : nVBWarning );
    if( !rMedium.IsStorage() )
        rMedium.CloseOutStream();

    return !IsError( nErrno );
}

/*--------------------------------------------------------------------
    Beschreibung:   Haende weg
 --------------------------------------------------------------------*/


void SwDocShell::HandsOff()
{
    pIo->HandsOff();
    SfxInPlaceObject::HandsOff();
}

/*--------------------------------------------------------------------
    Beschreibung: ??? noch nicht zu aktivieren, muss TRUE liefern
 --------------------------------------------------------------------*/


BOOL SwDocShell::SaveCompleted( SvStorage * pStor )
{
    BOOL bRet = SfxInPlaceObject::SaveCompleted( pStor );
    if( bRet )
    {
        // erst hier entscheiden, ob das Speichern geklappt hat oder nicht
        if( IsModified() )
            pDoc->SetModified();
        else
            pDoc->ResetModified();

        bRet = pIo->SaveCompleted( pStor );
    }

    if( xOLEChildList.Is() )
    {
        BOOL bResetModified = IsEnableSetModified();
        if( bResetModified )
            EnableSetModified( FALSE );

        SvPersist* pPersist = this;
        const SvInfoObjectMemberList* pInfList = xOLEChildList->GetObjectList();

        for( ULONG n = pInfList->Count(); n; )
        {
            SvInfoObjectRef aRef( pInfList->GetObject( --n ));
            pPersist->Move( &aRef, aRef->GetStorageName() );
        }

        xOLEChildList.Clear();
        if( bResetModified )
            EnableSetModified( TRUE );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: Draw()-Overload fuer OLE2 (Sfx)
 --------------------------------------------------------------------*/

void SwDocShell::Draw( OutputDevice* pDev, const JobSetup& rSetup,
                               USHORT nAspect )
{
    //fix #25341# Draw sollte das Modified nicht beeinflussen
    BOOL bResetModified;
    if ( TRUE == (bResetModified = IsEnableSetModified()) )
        EnableSetModified( FALSE );

    //sollte am Document ein JobSetup haengen, dann kopieren wir uns diesen,
    //um nach dem PrtOle2 diesen wieder am Doc zu verankern.
    //Einen leeren JobSetup setzen wir nicht ein, denn der wuerde nur zu
    //fragwuerdigem Ergebnis nach teurer Neuformatierung fuehren (Preview!)
    JobSetup *pOrig = 0;
    if ( rSetup.GetPrinterName().Len() || ASPECT_THUMBNAIL != nAspect )
    {
        JobSetup* pOrig = (JobSetup*)pDoc->GetJobsetup();
        if( pOrig )         // dann kopieren wir uns den
            pOrig = new JobSetup( *pOrig );
        pDoc->SetJobsetup( rSetup );
    }

    Rectangle aRect( nAspect == ASPECT_THUMBNAIL ?
            GetVisArea( nAspect ) : ((SvEmbeddedObject*)this)->GetVisArea() );

    pDev->Push();
    pDev->SetFillColor();
    pDev->SetLineColor();
    pDev->SetBackground();
    BOOL bWeb = 0 != PTR_CAST(SwWebDocShell, this);
    ViewShell::PrtOle2( pDoc, SW_MOD()->GetUsrPref(bWeb), pDev, aRect );
    pDev->Pop();

    if( pOrig )
    {
        pDoc->SetJobsetup( *pOrig );
        delete pOrig;
    }
    if ( bResetModified )
        EnableSetModified( TRUE );
}


void SwDocShell::SetVisArea( const Rectangle &rRect )
{
    Rectangle aRect( rRect );
    if ( pView )
    {
        Size aSz( pView->GetDocSz() );
        aSz.Width() += DOCUMENTBORDER; aSz.Height() += DOCUMENTBORDER;
        long nMoveX = 0, nMoveY = 0;
        if ( aRect.Right() > aSz.Width() )
            nMoveX = aSz.Width() - aRect.Right();
        if ( aRect.Bottom() > aSz.Height() )
            nMoveY = aSz.Height() - aRect.Bottom();
        aRect.Move( nMoveX, nMoveY );
        nMoveX = aRect.Left() < 0 ? -aRect.Left() : 0;
        nMoveY = aRect.Top()  < 0 ? -aRect.Top()  : 0;
        aRect.Move( nMoveX, nMoveY );

        //Ruft das SfxInPlaceObject::SetVisArea()!
        pView->SetVisArea( aRect, TRUE );
    }
    else
        SfxInPlaceObject::SetVisArea( aRect );
}


Rectangle SwDocShell::GetVisArea( USHORT nAspect ) const
{
    if ( nAspect == ASPECT_THUMBNAIL )
    {
        //PreView: VisArea auf die erste Seite einstellen.

        //PageDesc besorgen, vom ersten Absatz oder den default.
        SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), 1 );
        SwCntntNode* pNd = pDoc->GetNodes().GoNext( &aIdx );
        const SwFmtPageDesc &rDesc = pNd->GetSwAttrSet().GetPageDesc();
        const SwPageDesc* pDesc = rDesc.GetPageDesc();
        if( !pDesc )
            pDesc = &pDoc->GetPageDesc( 0 );

        //Das Format wird evtl. von der virtuellen Seitennummer bestimmt.
        const USHORT nPgNum = rDesc.GetNumOffset();
        const BOOL bOdd = nPgNum % 2 ? TRUE : FALSE;
        const SwFrmFmt *pFmt = bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt();
        if ( !pFmt ) //#40568#
            pFmt = bOdd ? pDesc->GetLeftFmt() : pDesc->GetRightFmt();

        if ( pFmt->GetFrmSize().GetWidth() == LONG_MAX )
            //Jetzt wird es aber Zeit fuer die Initialisierung
            pDoc->GetPrt( TRUE );

        const SwFmtFrmSize& rFrmSz = pFmt->GetFrmSize();
        const Size aSz( rFrmSz.GetWidth(), rFrmSz.GetHeight() );
        const Point aPt( DOCUMENTBORDER, DOCUMENTBORDER );
        const Rectangle aRect( aPt, aSz );
        return aRect;
    }
    return SvEmbeddedObject::GetVisArea( nAspect );
}

Printer *SwDocShell::GetDocumentPrinter()
{
    return pDoc->GetPrt();
}

void SwDocShell::OnDocumentPrinterChanged( Printer * pNewPrinter )
{
    if ( pNewPrinter )
        GetDoc()->SetJobsetup( pNewPrinter->GetJobSetup() );
    else
        GetDoc()->SetPrt( 0 );
}

ULONG SwDocShell::GetMiscStatus() const
{
    return SfxInPlaceObject::GetMiscStatus() |
           SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE;
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwDocShell::GetState(SfxItemSet& rSet)
{
    SfxWhichIter aIter(rSet);
    USHORT  nWhich  = aIter.FirstWhich();

    while (nWhich)
    {
        switch (nWhich)
        {
        // MT: MakroChosser immer enablen, weil Neu moeglich
        // case SID_BASICCHOOSER:
        // {
        //  StarBASIC* pBasic = GetBasic();
        //  StarBASIC* pAppBasic = SFX_APP()->GetBasic();
        //  if ( !(pBasic->GetModules()->Count() ||
        //      pAppBasic->GetModules()->Count()) )
        //          rSet.DisableItem(nWhich);
        // }
        // break;
        case SID_PRINTPREVIEW:
        {
            FASTBOOL bDisable = GetProtocol().IsInPlaceActive();
            if ( !bDisable )
            {
                SfxViewFrame *pTmpFrm = SfxViewFrame::GetFirst(this);
                while (pTmpFrm)     // Preview suchen
                {
                    if ( PTR_CAST(SwView, pTmpFrm->GetViewShell()) &&
                         ((SwView*)pTmpFrm->GetViewShell())->GetWrtShell().
                                                    GetDoc()->IsBrowseMode())
                    {
                        bDisable = TRUE;
                        break;
                    }
                    pTmpFrm = pTmpFrm->GetNext(*pTmpFrm, this);
                }
            }
            if ( bDisable )
                rSet.DisableItem( SID_PRINTPREVIEW );
            else
            {
                SfxBoolItem aBool( SID_PRINTPREVIEW, FALSE );
                if( PTR_CAST( SwPagePreView, SfxViewShell::Current()) )
                    aBool.SetValue( TRUE );
                rSet.Put( aBool );
            }
        }
        break;
        case SID_SOURCEVIEW:
        {
            if(IsLoading())
                rSet.DisableItem(nWhich);
            else
            {
                SfxViewShell* pView = GetView() ? (SfxViewShell*)GetView()
                                            : SfxViewShell::Current();
                BOOL bSourceView = 0 != PTR_CAST(SwSrcView, pView);
                rSet.Put(SfxBoolItem(SID_SOURCEVIEW, bSourceView));
            }
        }
        break;
        case SID_HTML_MODE:
            rSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));
        break;

        case FN_ABSTRACT_STARIMPRESS:
        case FN_OUTLINE_TO_IMPRESS:
            {
                SvtModuleOptions aMOpt;
                if ( !aMOpt.IsImpress() )
                    rSet.DisableItem( nWhich );
            }
            /* no break here */
        case FN_ABSTRACT_NEWDOC:
        case FN_OUTLINE_TO_CLIPBOARD:
            {
                if ( !GetDoc()->GetNodes().GetOutLineNds().Count() )
                    rSet.DisableItem( nWhich );
            }
            break;

        case SID_BROWSER_MODE:
            {
                SfxViewShell* pViewShell = SfxViewShell::Current();
                BOOL bDisable = PTR_CAST(SwPagePreView, pViewShell) != 0;

                if (bDisable)
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem( SID_BROWSER_MODE, GetDoc()->IsBrowseMode()));
                break;
            }
        case FN_PRINT_LAYOUT:
            {
                rSet.Put( SfxBoolItem( FN_PRINT_LAYOUT, !GetDoc()->IsBrowseMode()));
                break;
            }
        case FN_NEW_GLOBAL_DOC:
            if ( ISA(SwGlobalDocShell) )
                rSet.DisableItem( nWhich );
            break;

        case FN_NEW_HTML_DOC:
            // ??? oder sollte das immer gehen??
            if( ISA( SwWebDocShell ) )
                rSet.DisableItem( nWhich );
            break;

        case SID_ATTR_YEAR2000:
            rSet.Put(SfxUInt16Item(nWhich, pDoc->GetNumberFormatter(TRUE)->GetYear2000()));
        break;

        default: DBG_ASSERT(!this,"Hier darfst Du nicht hinein!");

        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   OLE-Hdls
 --------------------------------------------------------------------*/


IMPL_LINK( SwDocShell, Ole2ModifiedHdl, void *, p )
{
    // vom Doc wird der Status mitgegeben (siehe doc.cxx)
    //  Bit 0:  -> alter Zustand
    //  Bit 1:  -> neuer Zustand
    long nStatus = (long)p;
    if( IsEnableSetModified() )
        SetModified( (nStatus & 2) ? TRUE : FALSE );
    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:   Pool returnen Hier weil virtuelll
 --------------------------------------------------------------------*/


SfxStyleSheetBasePool*  SwDocShell::GetStyleSheetPool()
{
    return pBasePool;
}


void SwDocShell::SetView(SwView* pVw)
{
    if ( 0 != (pView = pVw) )
        pWrtShell = &pView->GetWrtShell();
    else
        pWrtShell = 0;
}


void SwDocShell::PrepareReload()
{
    ::DelAllGrfCacheEntries( pDoc );
}


void SwDocShell::StartLoadFinishedTimer()
{
    BOOL bSttTimer = FALSE;
    // ohne WrtShell haben wir eine WebDocShell und muessen uns die
    // Optionen vom Modul holen
    if( pWrtShell ? pWrtShell->GetViewOptions()->IsGraphic()
                  : SW_MOD()->GetUsrPref(TRUE)->IsGraphic() )
    {
        const SvxLinkManager& rLnkMgr = pDoc->GetLinkManager();
        const ::so3::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
        for( USHORT n = 0; n < rLnks.Count(); ++n )
        {
            ::so3::SvBaseLink* pLnk = &(*rLnks[ n ]);
            if( pLnk && OBJECT_CLIENT_GRF == pLnk->GetObjType() &&
                pLnk->ISA( SwBaseLink ) )
            {
                ::so3::SvLinkSource* pLnkObj = pLnk->GetObj();
                if( !pLnkObj )
                {
                    String sFileNm;
                    if( rLnkMgr.GetDisplayNames( pLnk, 0, &sFileNm, 0, 0 ))
                    {
                        INetURLObject aURL( sFileNm );
                        switch( aURL.GetProtocol() )
                        {
                        case INET_PROT_NOT_VALID:
                        case INET_PROT_FILE:
                        case INET_PROT_MAILTO:
                        case INET_PROT_NEWS:
                        case INET_PROT_CID:
                            break;

                        default:
                            ((SwBaseLink*)pLnk)->SwapIn();
                            ((SwBaseLink*)pLnk)->GetCntntNode()->SetAutoFmtLvl(1);
                            bSttTimer = TRUE;
                            break;
                        }
                    }
                }
                else
                {
                    BOOL bSendState = FALSE;
                    if( pLnkObj->IsPending() )
                        bSttTimer = TRUE;       // Pending?
                    else if( !pLnkObj->IsDataComplete() )
                    {
                        // falls aber nicht angetickert werden muss (liegt
                        // im QuickdrawCache)
                        if( !((SwBaseLink*)pLnk)->IsShowQuickDrawBmp() )
                        {
                            ((SwBaseLink*)pLnk)->SwapIn();
                            ((SwBaseLink*)pLnk)->GetCntntNode()->SetAutoFmtLvl(1);
                            bSttTimer = TRUE;
                        }
                        else
                            // dann muss aber auf jedenfall der Status
                            // an die Handler verschickt werden!
                            bSendState = TRUE;
                    }
                    else if( ((SwBaseLink*)pLnk)->IsShowQuickDrawBmp() )
                        // Wenn die Grafik aus dem QuickDrawCache kommt,
                        // wird sie nie angefordert!
                        // Dann muss aber auf jedenfall der Status
                        // an die Handler verschickt werden!
                        bSendState = TRUE;

                    if( bSendState )
                    {
                        ::com::sun::star::uno::Any aValue;
                        aValue <<= ::rtl::OUString::valueOf(
                                            (sal_Int32)STATE_LOAD_OK );
                        String sMimeType( SotExchange::GetFormatMimeType(
                            SvxLinkManager::RegisterStatusInfoId() ));
                        pLnkObj->DataChanged( sMimeType, aValue );
                    }
                }
            }
        }
    }

    if( bSttTimer )
    {
        aFinishedTimer.SetTimeoutHdl( STATIC_LINK( this, SwDocShell, IsLoadFinished ));
        aFinishedTimer.SetTimeout( 1000 );
        aFinishedTimer.Start();
        GetDoc()->StopIdleTimer();
    }
    FinishedLoading( SFX_LOADED_MAINDOCUMENT |
                    ( bSttTimer ? 0 : SFX_LOADED_IMAGES ));
    // jetzt noch testen, ob die SourceView noch geladen werden muá
    SfxViewFrame* pVFrame = SfxViewFrame::GetFirst(this);
    if(pVFrame)
    {
        SfxViewShell* pShell = pVFrame->GetViewShell();
        if(PTR_CAST(SwSrcView, pShell))
            ((SwSrcView*)pShell)->Load(this);
    }
}


IMPL_STATIC_LINK( SwDocShell, IsLoadFinished, void*, EMPTYARG )
{
    BOOL bSttTimer = FALSE;

    if( !pThis->IsAbortingImport() )
    {
        const SvxLinkManager& rLnkMgr = pThis->pDoc->GetLinkManager();
        const ::so3::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
        for( USHORT n = rLnks.Count(); n; )
        {
            ::so3::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if( pLnk && OBJECT_CLIENT_GRF == pLnk->GetObjType() &&
                pLnk->ISA( SwBaseLink ) )
            {
                ::so3::SvLinkSource* pLnkObj = pLnk->GetObj();
                if( pLnkObj && pLnkObj->IsPending() &&
                    !((SwBaseLink*)pLnk)->IsShowQuickDrawBmp() )
                {
                    bSttTimer = TRUE;
                    break;
                }
            }
        }
    }

    if( bSttTimer )
        pThis->aFinishedTimer.Start();
    else
    {
        BOOL bIsModifiedEnabled = pThis->IsEnableSetModified();
        pThis->EnableSetModified( FALSE );
        pThis->FinishedLoading( SFX_LOADED_MAINDOCUMENT | SFX_LOADED_IMAGES );
        pThis->EnableSetModified( bIsModifiedEnabled );

        pThis->GetDoc()->StartIdleTimer();
    }
    return 0;
}


// eine Uebertragung wird abgebrochen (wird aus dem SFX gerufen)
void SwDocShell::CancelTransfers()
{
    // alle Links vom LinkManager Canceln
    aFinishedTimer.Stop();
    pDoc->GetLinkManager().CancelTransfers();
    SfxObjectShell::CancelTransfers();
}

SwFEShell* SwDocShell::GetFEShell()
{
    return pWrtShell;
}

    // embedded alle lokalen Links (Bereiche/Grafiken)
BOOL SwDocShell::EmbedAllLinks()
{
    if( pWrtShell )
        return pWrtShell->EmbedAllLinks();
    return pDoc->EmbedAllLinks();
}


void SwDocShell::RemoveOLEObjects()
{
    SvPersist* pPersist = this;
    SwClientIter aIter( *(SwModify*)pDoc->GetDfltGrfFmtColl() );
    for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
            pNd; pNd = (SwCntntNode*)aIter.Next() )
    {
        SwOLENode* pOLENd = pNd->GetOLENode();
        if( pOLENd && ( pOLENd->IsOLEObjectDeleted() ||
                        pOLENd->IsInGlobalDocSection() ) )
        {
            SvInfoObjectRef aRef( pPersist->Find(
                                    pOLENd->GetOLEObj().GetName() ) );
            if( aRef.Is() )
            {
                if( !xOLEChildList.Is() )
                {
                    xOLEChildList = new SwTmpPersist( *this );
                    xOLEChildList->DoInitNew( 0 );
                }

                xOLEChildList->Move( &aRef, aRef->GetStorageName(), TRUE );
                pPersist->Remove( &aRef );
            }
        }
    }
}
/* -----------------------------12.02.01 12:08--------------------------------

 ---------------------------------------------------------------------------*/
#if SUPD>620
Sequence< OUString >    SwDocShell::GetEventNames()
{
    Sequence< OUString > aRet = SfxObjectShell::GetEventNames();
    sal_Int32 nLen = aRet.getLength();
    aRet.realloc(nLen + 2);
    OUString* pNames = aRet.getArray();
    pNames[nLen++] = OUString::createFromAscii("OnMailMerge");
    pNames[nLen] = OUString::createFromAscii("OnPageCountChange");
    return aRet;
}
#endif
void SwTmpPersist::FillClass( SvGlobalName * pClassName,
                            ULONG * pClipFormat,
                            String * pAppName,
                            String * pLongUserName,
                            String * pUserName,
                            long nFileFormat ) const
{
    pDShell->SwDocShell::FillClass( pClassName, pClipFormat, pAppName,
                                    pLongUserName, pUserName, nFileFormat );
}

BOOL SwTmpPersist::Save()
{
    if( SaveChilds() )
        return SvPersist::Save();
    return FALSE;
}

BOOL SwTmpPersist::SaveCompleted( SvStorage * pStor )
{
    if( SaveCompletedChilds( pStor ) )
        return SvPersist::SaveCompleted( pStor );
    return FALSE;
}



