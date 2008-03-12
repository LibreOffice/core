/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: view2.cxx,v $
 *
 *  $Revision: 1.83 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:57:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _COM_SUN_STAR_UTIL_SEARCHOPTIONS_HPP_
#include <com/sun/star/util/SearchOptions.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_SEARCHFLAGS_HPP_
#include <com/sun/star/util/SearchFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif

// #ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
// #include <com/sun/star/lang/Locale.hpp>
// #endif
// #ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
// #include <com/sun/star/ui/dialogs/XFilePicker.hpp>
// #endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_LISTBOXCONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#endif


#define _SVSTDARR_STRINGSSORTDTOR
#include <svtools/svstdarr.hxx>

#ifndef _AEITEM_HXX
#include <svtools/aeitem.hxx>
#endif

// #ifndef _FILTER_HXX
// #include <svtools/filter.hxx>
// #endif

#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SW_REWRITER_HXX
#include <SwRewriter.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _CAPTION_HXX
#include <caption.hxx>
#endif

#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#include <svtools/PasswordHelper.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFX_PASSWD_HXX
#include <sfx2/passwd.hxx>
#endif
#ifndef _SFX2_DIALOG_HXX
#include <sfx2/sfxdlg.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _SFX_HELP_HXX
#include <sfx2/sfxhelp.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_VIEWLAYOUTITEM_HXX
#include <svx/viewlayoutitem.hxx>
#endif
#ifndef _SVX_ZOOMSLIDERITEM_HXX
#include <svx/zoomslideritem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include  <svx/impgrf.hxx>
#endif
#ifndef _TXTCMP_HXX //autogen
#include <svtools/txtcmp.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include "svx/unolingu.hxx"
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SFXEVENT_HXX //autogen
#include <sfx2/event.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <svx/sizeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_TOPFRM_HXX
#include <sfx2/topfrm.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _REDLNDLG_HXX
#include <redlndlg.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _BASESH_HXX
#include <basesh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _DRAWBASE_HXX
#include <drawbase.hxx>
#endif
#ifndef _FRMMGR_HXX
#include <frmmgr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _WORKCTRL_HXX
#include <workctrl.hxx>
#endif
#ifndef _SCROLL_HXX
#include <scroll.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#ifndef _TABSH_HXX
#include <tabsh.hxx>
#endif
#ifndef _LISTSH_HXX
#include <listsh.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _POOLFMT_HRC
#include <poolfmt.hrc>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _WEB_HRC
#include <web.hrc>
#endif
#ifndef _VIEW_HRC
#include <view.hrc>
#endif
#ifndef _APP_HRC
#include <app.hrc>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#include <helpid.h>
#ifndef _SVTOOLS_TEMPLDLG_HXX
#include <svtools/templdlg.hxx>
#endif
#ifndef _DBCONFIG_HXX
#include <dbconfig.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#include <PostItMgr.hxx>

// #ifndef _FRMMGR_HXX
// #include <frmmgr.hxx>
// #endif


#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "swabstdlg.hxx"
#include "globals.hrc"
#include <envelp.hrc>
#include <fmthdft.hxx>
#include <svx/ofaitem.hxx>
#include <unomid.h>


//Damit die Seitenanzeige in der Statusleiste nicht unnoetig erfolgt.
static String sLstPg;
static USHORT nPageCnt = 0;
const char __FAR_DATA sStatusDelim[] = " : ";

using ::rtl::OUString;
using namespace sfx2;
using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui::dialogs;


static void lcl_SetAllTextToDefaultLanguage( SwWrtShell &rWrtSh, USHORT nWhichId )
{
    if (nWhichId == RES_CHRATR_LANGUAGE ||
        nWhichId == RES_CHRATR_CJK_LANGUAGE ||
        nWhichId == RES_CHRATR_CTL_LANGUAGE)
    {
        rWrtSh.StartAction();
        rWrtSh.LockView( TRUE );
        rWrtSh.Push();

        // prepare to apply new language to all text in document
        rWrtSh.SelAll();

        // set language attribute to default for all text
        SvUShortsSort aAttribs;
        aAttribs.Insert( nWhichId );
        rWrtSh.ResetAttr( &aAttribs );

        rWrtSh.Pop( FALSE );
        rWrtSh.LockView( FALSE );
        rWrtSh.EndAction();
    }
}

/*---------------------------------------------------------------------------
    Beschreibung:   String fuer die Seitenanzeige in der Statusbar basteln.
 ----------------------------------------------------------------------------*/

String SwView::GetPageStr( USHORT nPg, USHORT nLogPg,
                            const String& rDisplay )
{
    String aStr( aPageStr );
    if( rDisplay.Len() )
        aStr += rDisplay;
    else
        aStr += String::CreateFromInt32(nLogPg);

    if( nLogPg && nLogPg != nPg )
    {
        aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM("   "));
        aStr += String::CreateFromInt32(nPg);
    }
    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM(" / "));
    aStr += String::CreateFromInt32( GetWrtShell().GetPageCnt() );

    return aStr;
}


int SwView::InsertGraphic( const String &rPath, const String &rFilter,
                                BOOL bLink, GraphicFilter *pFlt,
                                Graphic* pPreviewGrf, BOOL bRule )
{
    SwWait aWait( *GetDocShell(), TRUE );

    Graphic aGrf;
    int nRes = GRFILTER_OK;
    if ( pPreviewGrf )
        aGrf = *pPreviewGrf;
    else
    {
        if( !pFlt )
            pFlt = ::GetGrfFilter();
        nRes = ::LoadGraphic( rPath, rFilter, aGrf, pFlt /*, nFilter*/ );
    }

    if( GRFILTER_OK == nRes )
    {
        SwFlyFrmAttrMgr aFrmMgr( TRUE, GetWrtShellPtr(), FRMMGR_TYPE_GRF );

        SwWrtShell &rSh = GetWrtShell();
        rSh.StartAction();
        if( bLink )
        {
            SwDocShell* pDocSh = GetDocShell();
            INetURLObject aTemp(
                pDocSh->HasName() ?
                    pDocSh->GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) :
                    rtl::OUString());

            String sURL = URIHelper::SmartRel2Abs(
                aTemp, rPath, URIHelper::GetMaybeFileHdl() );

            rSh.Insert( sURL,
                        rFilter, aGrf, &aFrmMgr, bRule );
        }
        else
            rSh.Insert( aEmptyStr, aEmptyStr, aGrf, &aFrmMgr );
        // nach dem EndAction ist es zu spaet, weil die Shell dann schon zerstoert sein kann
        rSh.EndAction();
    }
    return nRes;
}


BOOL SwView::InsertGraphicDlg( SfxRequest& rReq )
{
#ifndef ENABLE_PROP_WITHOUTLINK
#define ENABLE_PROP_WITHOUTLINK 0x08
#endif

    BOOL bReturn = FALSE;
    SwDocShell* pDocShell = GetDocShell();
    USHORT nHtmlMode = ::GetHtmlMode(pDocShell);
    // im HTML-Mode nur verknuepft einfuegen
    FileDialogHelper* pFileDlg = new FileDialogHelper( SFXWB_GRAPHIC | SFXWB_SHOWSTYLES );
    pFileDlg->SetTitle(SW_RESSTR(STR_INSERT_GRAPHIC ));
    pFileDlg->SetContext( FileDialogHelper::SW_INSERT_GRAPHIC );
    uno::Reference < XFilePicker > xFP = pFileDlg->GetFilePicker();
    uno::Reference < XFilePickerControlAccess > xCtrlAcc(xFP, UNO_QUERY);
    if(nHtmlMode & HTMLMODE_ON)
    {
        sal_Bool bTrue = sal_True;
        Any aVal(&bTrue, ::getBooleanCppuType());
        xCtrlAcc->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aVal);
        xCtrlAcc->enableControl( ExtendedFilePickerElementIds::CHECKBOX_LINK, sal_False);
    }

    SvStringsSortDtor aFormats;
    SwDoc* pDoc = pDocShell->GetDoc();
    const USHORT nArrLen = pDoc->GetFrmFmts()->Count();
    USHORT i;
    for( i = 0; i < nArrLen; i++ )
    {
        SwFrmFmt* pFmt = (*pDoc->GetFrmFmts())[ i ];
        if(pFmt->IsDefault() || pFmt->IsAuto())
            continue;
        String *pFormat = new String(pFmt->GetName());
        aFormats.Insert(pFormat);
    }

    // pool formats
    //
    const SvStringsDtor& rFrmPoolArr = SwStyleNameMapper::GetFrmFmtUINameArray();
    for( i = 0; i < rFrmPoolArr.Count(); i++ )
    {
        String *pFormat = new String(*rFrmPoolArr[i]);
        if (!aFormats.Insert(pFormat))
            delete pFormat;
    }

    Sequence<OUString> aListBoxEntries(aFormats.Count());
    OUString* pEntries = aListBoxEntries.getArray();
    sal_Int16 nSelect = 0;
    String sGraphicFormat = SW_RESSTR(STR_POOLFRM_GRAPHIC);
    for(i = 0; i < aFormats.Count(); ++i)
    {
        pEntries[i] = *aFormats[i];
        if(pEntries[i].equals(sGraphicFormat))
            nSelect = i;
    }
    try
    {
        Any aTemplates(&aListBoxEntries, ::getCppuType(&aListBoxEntries));

        xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
            ListboxControlActions::ADD_ITEMS , aTemplates );

        Any aSelectPos(&nSelect, ::getCppuType(&nSelect));
        xCtrlAcc->setValue( ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
            ListboxControlActions::SET_SELECT_ITEM, aSelectPos );
    }
    catch(Exception& )
    {
        DBG_ERROR("control acces failed")
    }

    SFX_REQUEST_ARG( rReq, pName, SfxStringItem, SID_INSERT_GRAPHIC , sal_False );
    BOOL bShowError = !pName;
    if( pName || ERRCODE_NONE == pFileDlg->Execute() )
    {

        String aFileName, aFilterName;
        if ( pName )
        {
            aFileName = pName->GetValue();
            SFX_REQUEST_ARG( rReq, pFilter, SfxStringItem, FN_PARAM_FILTER , sal_False );
            if ( pFilter )
                aFilterName = pFilter->GetValue();
        }
        else
        {
            aFileName = pFileDlg->GetPath();
            aFilterName = pFileDlg->GetCurrentFilter();
            rReq.AppendItem( SfxStringItem( SID_INSERT_GRAPHIC, aFileName ) );
            rReq.AppendItem( SfxStringItem( FN_PARAM_FILTER, aFilterName ) );

            sal_Bool bAsLink = sal_False;
            if(nHtmlMode & HTMLMODE_ON)
                bAsLink = sal_True;
            else
            {
                try
                {
                    Any aVal = xCtrlAcc->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0);
                    DBG_ASSERT(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found")
                    bAsLink = aVal.hasValue() ? *(sal_Bool*) aVal.getValue() : sal_True;
                    Any aTemplateValue = xCtrlAcc->getValue(
                        ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
                        ListboxControlActions::GET_SELECTED_ITEM );
                    OUString sTmpl;
                    aTemplateValue >>= sTmpl;
                    rReq.AppendItem( SfxStringItem( FN_PARAM_2, sTmpl) );
                }
                catch(Exception& )
                {
                    DBG_ERROR("control acces failed")
                }
            }
            rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bAsLink ) );
        }

        SFX_REQUEST_ARG( rReq, pAsLink, SfxBoolItem, FN_PARAM_1 , sal_False );
        SFX_REQUEST_ARG( rReq, pStyle, SfxStringItem, FN_PARAM_2 , sal_False );

        sal_Bool bAsLink = sal_False;
        if( nHtmlMode & HTMLMODE_ON )
            bAsLink = sal_True;
        else
        {
            if ( rReq.GetArgs() )
            {
                if ( pAsLink )
                    bAsLink = pAsLink->GetValue();
                if ( pStyle )
                    sGraphicFormat = pStyle->GetValue();
            }
            else
            {
                Any aVal = xCtrlAcc->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0);
                DBG_ASSERT(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found")
                bAsLink = aVal.hasValue() ? *(sal_Bool*) aVal.getValue() : sal_True;
                Any aTemplateValue = xCtrlAcc->getValue(
                    ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE,
                    ListboxControlActions::GET_SELECTED_ITEM );
                OUString sTmpl;
                aTemplateValue >>= sTmpl;
                sGraphicFormat = sTmpl;
                if ( sGraphicFormat.Len() )
                    rReq.AppendItem( SfxStringItem( FN_PARAM_2, sGraphicFormat ) );
                rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bAsLink ) );
            }
        }

        SwWrtShell& rSh = GetWrtShell();
        rSh.StartAction();

        /// #111827#
        SwRewriter aRewriter;
        aRewriter.AddRule(UNDO_ARG1, String(SW_RES(STR_GRAPHIC_DEFNAME)));

        rSh.StartUndo(UNDO_INSERT, &aRewriter);

        int nError = InsertGraphic( aFileName, aFilterName, bAsLink, ::GetGrfFilter() );

        // Format ist ungleich Current Filter, jetzt mit auto. detection
        if( nError == GRFILTER_FORMATERROR )
            nError = InsertGraphic( aFileName, aEmptyStr, bAsLink, ::GetGrfFilter() );
        if ( rSh.IsFrmSelected() )
        {
            SwFrmFmt* pFmt = pDoc->FindFrmFmtByName( sGraphicFormat );
            if(!pFmt)
                pFmt = pDoc->MakeFrmFmt(sGraphicFormat,
                                        pDocShell->GetDoc()->GetDfltFrmFmt(),
                                        TRUE, FALSE);
            rSh.SetFrmFmt( pFmt );
        }

        RESOURCE_TYPE nResId = 0;
        switch( nError )
        {
            case GRFILTER_OPENERROR:
                nResId = STR_GRFILTER_OPENERROR;
                break;
            case GRFILTER_IOERROR:
                nResId = STR_GRFILTER_IOERROR;
                break;
            case GRFILTER_FORMATERROR:
                nResId = STR_GRFILTER_FORMATERROR;
                break;
            case GRFILTER_VERSIONERROR:
                nResId = STR_GRFILTER_VERSIONERROR;
                break;
            case GRFILTER_FILTERERROR:
                nResId = STR_GRFILTER_FILTERERROR;
                break;
            case GRFILTER_TOOBIG:
                nResId = STR_GRFILTER_TOOBIG;
                break;
        }

        rSh.EndAction();
        if( nResId )
        {
            if( bShowError )
            {
                InfoBox aInfoBox( GetWindow(), SW_RESSTR( nResId ));
                aInfoBox.Execute();
            }
            rReq.Ignore();
        }
        else
        {
            // set the specific graphic attrbutes to the graphic
            bReturn = TRUE;
            AutoCaption( GRAPHIC_CAP );
            rReq.Done();
        }

        rSh.EndUndo(UNDO_INSERT); // wegen moegl. Shellwechsel
    }

    delete pFileDlg;

    return bReturn;
}


void __EXPORT SwView::Execute(SfxRequest &rReq)
{
    USHORT nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    BOOL bIgnore = FALSE;
    switch( nSlot )
    {
        case SID_CREATE_SW_DRAWVIEW:
            // --> OD 2005-08-08 #i52858# - method name changed
            pWrtShell->getIDocumentDrawModelAccess()->GetOrCreateDrawModel();
            // <--
            break;

        case FN_LINE_NUMBERING_DLG:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            VclAbstractDialog* pDlg = pFact->CreateVclSwViewDialog( DLG_LINE_NUMBERING,    *this);
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
            break;
        }
        case FN_EDIT_LINK_DLG:
            EditLinkDlg();
            break;
        case FN_REFRESH_VIEW:
            GetEditWin().Invalidate();
            break;
        case FN_PAGEUP:
        case FN_PAGEUP_SEL:
        case FN_PAGEDOWN:
        case FN_PAGEDOWN_SEL:
        {
            Rectangle aVis( GetVisArea() );
            SwEditWin& rTmpWin = GetEditWin();
            if ( FN_PAGEUP == nSlot || FN_PAGEUP_SEL == nSlot )
                PageUpCrsr(FN_PAGEUP_SEL == nSlot);
            else
                PageDownCrsr(FN_PAGEDOWN_SEL == nSlot);

            rReq.SetReturnValue(SfxBoolItem(nSlot,
                                                aVis != GetVisArea()));
            //#i42732# - notify the edit window that from now on we do not use the input language
            rTmpWin.SetUseInputLanguage( sal_False );
        }
        break;
        case FN_REDLINE_ON:
        {
            if( pArgs &&
                SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem ))
            {
                IDocumentRedlineAccess* pIDRA = pWrtShell->getIDocumentRedlineAccess();
                Sequence <sal_Int8> aPasswd = pIDRA->GetRedlinePassword();
                if( aPasswd.getLength() )
                {
                    DBG_ASSERT( !((const SfxBoolItem*)pItem)->GetValue(), "SwView::Execute(): password set an redlining off doesn't match!" );
                    // xmlsec05:    new password dialog
                    Window* pParent;
                    const SfxPoolItem* pParentItem;
                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_ATTR_PARENTWINDOW, FALSE, &pParentItem ) )
                        pParent = ( Window* ) ( ( const OfaPtrItem* ) pParentItem )->GetValue();
                    else
                        pParent = &GetViewFrame()->GetWindow();
                    SfxPasswordDialog aPasswdDlg( pParent );
                    //#i69751# the result of Execute() can be ignored
                    aPasswdDlg.Execute();
                    String sNewPasswd( aPasswdDlg.GetPassword() );
                    Sequence <sal_Int8> aNewPasswd = pIDRA->GetRedlinePassword();
                    SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                    if(SvPasswordHelper::CompareHashPassword(aPasswd, sNewPasswd))
                        pIDRA->SetRedlinePassword(Sequence <sal_Int8> ());
                    else
                    {   // xmlsec05: message box for wrong password
                        break;
                    }
                }

                USHORT nOn = ((const SfxBoolItem*)pItem)->GetValue() ? nsRedlineMode_t::REDLINE_ON : 0;
                USHORT nMode = pWrtShell->GetRedlineMode();
                pWrtShell->SetRedlineMode( (nMode & ~nsRedlineMode_t::REDLINE_ON) | nOn);
            }
        }
        break;
        case FN_REDLINE_PROTECT :
        {
            IDocumentRedlineAccess* pIDRA = pWrtShell->getIDocumentRedlineAccess();
            Sequence <sal_Int8> aPasswd = pIDRA->GetRedlinePassword();
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem )
                && ((SfxBoolItem*)pItem)->GetValue() == ( aPasswd.getLength() != 0 ) )
                break;

            // xmlsec05:    new password dialog
            //              message box for wrong password
            Window* pParent;
            const SfxPoolItem* pParentItem;
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_ATTR_PARENTWINDOW, FALSE, &pParentItem ) )
                pParent = ( Window* ) ( ( const OfaPtrItem* ) pParentItem )->GetValue();
            else
                pParent = &GetViewFrame()->GetWindow();
            SfxPasswordDialog aPasswdDlg( pParent );
            if(!aPasswd.getLength())
                aPasswdDlg.ShowExtras(SHOWEXTRAS_CONFIRM);
            if (aPasswdDlg.Execute())
            {
                USHORT nOn = nsRedlineMode_t::REDLINE_ON;
                String sNewPasswd( aPasswdDlg.GetPassword() );
                Sequence <sal_Int8> aNewPasswd =
                        pIDRA->GetRedlinePassword();
                SvPasswordHelper::GetHashPassword( aNewPasswd, sNewPasswd );
                if(!aPasswd.getLength())
                {
                    pIDRA->SetRedlinePassword(aNewPasswd);
                }
                else if(SvPasswordHelper::CompareHashPassword(aPasswd, sNewPasswd))
                {
                    pIDRA->SetRedlinePassword(Sequence <sal_Int8> ());
                    nOn = 0;
                }
                USHORT nMode = pIDRA->GetRedlineMode();
                pWrtShell->SetRedlineMode( (nMode & ~nsRedlineMode_t::REDLINE_ON) | nOn);
                rReq.AppendItem( SfxBoolItem( FN_REDLINE_PROTECT, ((nMode&nsRedlineMode_t::REDLINE_ON)==0) ) );
            }
            else
                bIgnore = TRUE;
        }
        break;
        case FN_REDLINE_SHOW:

            if( pArgs &&
                SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem))
            {
                USHORT nMode = ( ~(nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE)
                        & pWrtShell->GetRedlineMode() ) | nsRedlineMode_t::REDLINE_SHOW_INSERT;
                if( ((const SfxBoolItem*)pItem)->GetValue() )
                    nMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;

                pWrtShell->SetRedlineMode( nMode );
            }
            break;
        case FN_MAILMERGE_SENDMAIL_CHILDWINDOW:
        case FN_REDLINE_ACCEPT:
            GetViewFrame()->ToggleChildWindow(nSlot);
        break;

        case SID_DOCUMENT_COMPARE:
        case SID_DOCUMENT_MERGE:
            {
                String sFileName, sFilterName;
                INT16 nVersion = 0;
                bool bHasFileName = false;
                pViewImpl->SetParam( 0 );

                if( pArgs )
                {
                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_FILE_NAME, FALSE, &pItem ))
                        sFileName = ((const SfxStringItem*)pItem)->GetValue();
                    bHasFileName = ( sFileName.Len() > 0 );

                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_FILTER_NAME, FALSE, &pItem ))
                        sFilterName = ((const SfxStringItem*)pItem)->GetValue();

                    if( SFX_ITEM_SET == pArgs->GetItemState( SID_VERSION, FALSE, &pItem ))
                    {
                        nVersion = ((const SfxInt16Item *)pItem)->GetValue();
                        pViewImpl->SetParam( nVersion );
                    }
                }

                pViewImpl->InitRequest( rReq );
                long nFound = InsertDoc( nSlot, sFileName, sFilterName, nVersion );

                if ( bHasFileName )
                {
                    rReq.SetReturnValue( SfxInt32Item( nSlot, nFound ));

                    if (nFound > 0) // Redline-Browser anzeigen
                    {
                        SfxViewFrame* pVFrame = GetViewFrame();
                        pVFrame->ShowChildWindow(FN_REDLINE_ACCEPT);

                        // RedlineDlg neu initialisieren
                        USHORT nId = SwRedlineAcceptChild::GetChildWindowId();
                        SwRedlineAcceptChild *pRed = (SwRedlineAcceptChild*)
                                                pVFrame->GetChildWindow(nId);
                        if (pRed)
                            pRed->ReInitDlg(GetDocShell());
                    }
                }
                else
                    bIgnore = TRUE;
            }
        break;
        case FN_SYNC_LABELS:
        case FN_MAILMERGE_CHILDWINDOW:
            GetViewFrame()->ShowChildWindow(nSlot, TRUE);
        break;
        case FN_ESCAPE:
        {
            if ( pWrtShell->HasDrawView() && pWrtShell->GetDrawView()->IsDragObj() )
            {
                pWrtShell->BreakDrag();
                pWrtShell->EnterSelFrmMode();
            }
            else if ( pWrtShell->IsDrawCreate() )
            {
                GetDrawFuncPtr()->BreakCreate();
                AttrChangedNotify(pWrtShell); // ggf Shellwechsel...
            }
            else if ( pWrtShell->HasSelection() || IsDrawMode() )
            {
                SdrView *pSdrView = pWrtShell->HasDrawView() ? pWrtShell->GetDrawView() : 0;
                if(pSdrView && pSdrView->AreObjectsMarked() &&
                    pSdrView->GetHdlList().GetFocusHdl())
                {
                    ((SdrHdlList&)pSdrView->GetHdlList()).ResetFocusHdl();
                }
                else
                {
                    if(pSdrView)
                    {
                        LeaveDrawCreate();
                        Point aPt(LONG_MIN, LONG_MIN);
                        //go out of the frame
                        pWrtShell->SelectObj(aPt, SW_LEAVE_FRAME);
                        SfxBindings& rBind = GetViewFrame()->GetBindings();
                        rBind.Invalidate( SID_ATTR_SIZE );
                    }
                    pWrtShell->EnterStdMode();
                    AttrChangedNotify(pWrtShell); // ggf Shellwechsel...
                }
            }
            else if ( GetEditWin().GetApplyTemplate() )
            {
                GetEditWin().SetApplyTemplate(SwApplyTemplate());
            }
            else if( ((SfxObjectShell*)GetDocShell())->IsInPlaceActive() )
            {
                Escape();
            }
            else if ( GetEditWin().IsChainMode() )
            {
                GetEditWin().SetChainMode( FALSE );
            }
            else if( pWrtShell->GetFlyFrmFmt() )
            {
                const SwFrmFmt* pFmt = pWrtShell->GetFlyFrmFmt();
                if(pWrtShell->GotoFly( pFmt->GetName(), FLYCNTTYPE_FRM ))
                {
                    pWrtShell->HideCrsr();
                    pWrtShell->EnterSelFrmMode();
                }
            }
            else
            {
                SfxBoolItem aItem( SID_WIN_FULLSCREEN, FALSE );
                GetViewFrame()->GetDispatcher()->Execute( SID_WIN_FULLSCREEN, SFX_CALLMODE_RECORD, &aItem, 0L );
                bIgnore = TRUE;
            }
        }
        break;
        case SID_ATTR_BORDER_INNER:
        case SID_ATTR_BORDER_OUTER:
        case SID_ATTR_BORDER_SHADOW:
            if(pArgs)
                pWrtShell->SetAttr(*pArgs);
            break;

        case SID_ATTR_PAGE:
        case SID_ATTR_PAGE_SIZE:
        case SID_ATTR_PAGE_MAXSIZE:
        case SID_ATTR_PAGE_PAPERBIN:
        case SID_ATTR_PAGE_EXT1:
        case FN_PARAM_FTN_INFO:
        {
            if(pArgs)
            {
                const USHORT nCurIdx = pWrtShell->GetCurPageDesc();
                SwPageDesc aPageDesc( pWrtShell->GetPageDesc( nCurIdx ) );
                ::ItemSetToPageDesc( *pArgs, aPageDesc );
                // Den Descriptor der Core veraendern.
                pWrtShell->ChgPageDesc( nCurIdx, aPageDesc );
            }
        }
        break;
        case FN_NAVIGATION_PI_GOTO_PAGE:
        {
            SfxViewFrame* pVFrame = GetViewFrame();
            SfxChildWindow* pCh = pVFrame->GetChildWindow( SID_NAVIGATOR );
            if(!pCh)
            {
                pVFrame->ToggleChildWindow( SID_NAVIGATOR );
                pCh = pVFrame->GetChildWindow( SID_NAVIGATOR );

            }
                ((SwNavigationPI*) pCh->GetContextWindow(SW_MOD()))->GotoPage();
        }
        break;
        case  FN_EDIT_CURRENT_TOX:
        {
            GetViewFrame()->GetDispatcher()->Execute(
                                FN_INSERT_MULTI_TOX, SFX_CALLMODE_ASYNCHRON);
        }
        break;
        case FN_UPDATE_CUR_TOX:
        {
            const SwTOXBase* pBase = pWrtShell->GetCurTOX();
            if(pBase)
            {
                pWrtShell->StartAction();
                if(TOX_INDEX == pBase->GetType())
                    pWrtShell->ApplyAutoMark();
                pWrtShell->UpdateTableOf( *pBase );
                pWrtShell->EndAction();
            }
        }
        break;
        case FN_UPDATE_TOX:
        {
            pWrtShell->StartAction();
            pWrtShell->EnterStdMode();
            BOOL bOldCrsrInReadOnly = pWrtShell->IsReadOnlyAvailable();
            pWrtShell->SetReadOnlyAvailable( TRUE );

            for( USHORT i = 0; i < 2; ++i )
            {
                USHORT nCount = pWrtShell->GetTOXCount();
                if( 1 == nCount )
                    ++i;

                while( pWrtShell->GotoPrevTOXBase() )
                    ;   // aufs erste Verzeichnis springen

                // falls wir nicht mehr in einem stehen, dann zum naechsten
                // springen.
                const SwTOXBase* pBase = pWrtShell->GetCurTOX();
                if( !pBase )
                {
                    pWrtShell->GotoNextTOXBase();
                    pBase = pWrtShell->GetCurTOX();
                }

                BOOL bAutoMarkApplied = FALSE;
                while( pBase )
                {
                    if(TOX_INDEX == pBase->GetType() && !bAutoMarkApplied)
                    {
                        pWrtShell->ApplyAutoMark();
                        bAutoMarkApplied = TRUE;
                    }
                    // JP 15.07.96: das pBase wird nur fuer die Schnittstelle
                    //              benoetigt. Muss mal umgetstellt werden!!!
                    pWrtShell->UpdateTableOf( *pBase );

                    if( pWrtShell->GotoNextTOXBase() )
                        pBase = pWrtShell->GetCurTOX();
                    else
                        pBase = 0;
                }
            }
            pWrtShell->SetReadOnlyAvailable( bOldCrsrInReadOnly );
            pWrtShell->EndAction();
        }
        break;
        case SID_ATTR_BRUSH:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(RES_BACKGROUND, FALSE, &pItem))
            {
                const USHORT nCurIdx = pWrtShell->GetCurPageDesc();
                SwPageDesc aDesc( pWrtShell->GetPageDesc( nCurIdx ));
                SwFrmFmt& rMaster = aDesc.GetMaster();
                rMaster.SetAttr(*pItem);
                pWrtShell->ChgPageDesc( nCurIdx, aDesc);
            }
        }
        break;
        case SID_CLEARHISTORY:
        {
            pWrtShell->DelAllUndoObj();
        }
        break;
        case SID_UNDO:
        {
            pShell->ExecuteSlot(rReq);
        }
        break;
        case FN_INSERT_CTRL:
        case FN_INSERT_OBJ_CTRL:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem))
            {
                USHORT nValue = ((SfxUInt16Item*)pItem)->GetValue();
                switch ( nSlot )
                {
                    case FN_INSERT_CTRL:
                    {
                        BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
                        if(bWeb)
                            SwView::nWebInsertCtrlState = nValue;
                        else
                            SwView::nInsertCtrlState = nValue;
                    }
                    break;
                    case FN_INSERT_OBJ_CTRL:    SwView::nInsertObjectCtrlState = nValue  ;break;
                }

            }
            GetViewFrame()->GetBindings().Invalidate( nSlot );
        }
        break;
#if defined WIN || defined WNT || defined UNX
        case SID_TWAIN_SELECT:
        case SID_TWAIN_TRANSFER:
            GetViewImpl()->ExecuteScan( rReq );
        break;
#endif

        case SID_ATTR_DEFTABSTOP:
        {
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_DEFTABSTOP, FALSE, &pItem))
            {
                SvxTabStopItem aDefTabs( 0, 0, SVX_TAB_ADJUST_DEFAULT, RES_PARATR_TABSTOP );
                USHORT nTab = ((const SfxUInt16Item*)pItem)->GetValue();
                MakeDefTabs( nTab, aDefTabs );
                pWrtShell->SetDefault( aDefTabs );
            }
        }
        break;
        case SID_ATTR_LANGUAGE:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_LANGUAGE, FALSE, &pItem))
        {
            SvxLanguageItem aLang(((SvxLanguageItem*)pItem)->GetLanguage(), RES_CHRATR_LANGUAGE);
            pWrtShell->SetDefault( aLang );
            lcl_SetAllTextToDefaultLanguage( *pWrtShell, RES_CHRATR_LANGUAGE );
        }
        break;
        case  SID_ATTR_CHAR_CTL_LANGUAGE:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(RES_CHRATR_CTL_LANGUAGE, FALSE, &pItem))
        {
            pWrtShell->SetDefault( *pItem );
            lcl_SetAllTextToDefaultLanguage( *pWrtShell, RES_CHRATR_CTL_LANGUAGE );
        }
        break;
        case  SID_ATTR_CHAR_CJK_LANGUAGE:
        if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(RES_CHRATR_CJK_LANGUAGE, FALSE, &pItem))
        {
            pWrtShell->SetDefault( *pItem );
            lcl_SetAllTextToDefaultLanguage( *pWrtShell, RES_CHRATR_CJK_LANGUAGE );
        }
        break;
        case FN_SCROLL_NEXT_PREV:
            if(pArgs && pArgs->GetItemState(FN_SCROLL_NEXT_PREV, FALSE, &pItem))
            {
                // call the handlers of PageUp/DownButtons, only
                bool* pbNext = new bool ( ((const SfxBoolItem*)pItem)->GetValue() );
                // #i75416# move the execution of the search to an asynchronously called static link
                Application::PostUserEvent( STATIC_LINK(this, SwView, MoveNavigationHdl), pbNext );
            }
            break;
        case SID_JUMPTOMARK:
            if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_JUMPTOMARK, FALSE, &pItem))
                JumpToSwMark( (( const SfxStringItem*)pItem)->GetValue() );
            break;
        case SID_GALLERY :
            GetViewFrame()->ChildWindowExecute(rReq);//ToggleChildWindow(SID_GALLERY);
        break;
        case SID_AVMEDIA_PLAYER :
            GetViewFrame()->ChildWindowExecute(rReq);//ToggleChildWindow(SID_AVMEDIA_PLAYER);
        break;
        case SID_VIEW_DATA_SOURCE_BROWSER:
        {
            SfxViewFrame* pVFrame = GetViewFrame();
            pVFrame->ChildWindowExecute(rReq);
            if(pVFrame->HasChildWindow(SID_BROWSER))
            {
                const SwDBData& rData = GetWrtShell().GetDBDesc();
                SW_MOD()->ShowDBObj(*this, rData, FALSE);
            }
        }
        break;
        case FN_INSERT_FIELD_DATA_ONLY :
        {
            BOOL bShow = FALSE;
            if( pArgs &&
                SFX_ITEM_SET == pArgs->GetItemState(nSlot, FALSE, &pItem ))
                bShow = ((const SfxBoolItem*)pItem)->GetValue();
            //GetViewFrame()->ShowChildWindow(nSlot, bShow && bInMailMerge);
            if((bShow && bInMailMerge) != GetViewFrame()->HasChildWindow(nSlot))
                GetViewFrame()->ToggleChildWindow(nSlot);
            //if fields have been succesfully inserted call the "real"
            //mail merge dialog
            SwWrtShell &rSh = GetWrtShell();
            if(bInMailMerge && rSh.IsAnyDatabaseFieldInDoc())
            {
                SwNewDBMgr* pNewDBMgr = rSh.GetNewDBMgr();
                if (pNewDBMgr)
                {
                    SwDBData aData;
                    aData = rSh.GetDBData();
                    rSh.EnterStdMode(); // Wechsel in Textshell erzwingen; ist fuer
                                        // das Mischen von DB-Feldern notwendig.
                    AttrChangedNotify( &rSh );
                    pNewDBMgr->SetMergeType( DBMGR_MERGE );

                    Sequence<PropertyValue> aProperties(3);
                    PropertyValue* pValues = aProperties.getArray();
                    pValues[0].Name = C2U("DataSourceName");
                    pValues[1].Name = C2U("Command");
                    pValues[2].Name = C2U("CommandType");
                    pValues[0].Value <<= aData.sDataSource;
                    pValues[1].Value <<= aData.sCommand;
                    pValues[2].Value <<= aData.nCommandType;
                    pNewDBMgr->ExecuteFormLetter(rSh, aProperties, TRUE);
                }
            }
            bInMailMerge &= bShow;
            GetViewFrame()->GetBindings().Invalidate(FN_INSERT_FIELD);
        }
        break;
        case FN_QRY_MERGE:
        {
            BOOL bUseCurrentDocument = TRUE;
            BOOL bQuery = !pArgs||SFX_ITEM_SET != pArgs->GetItemState(nSlot);
            if(bQuery)
            {
                SfxViewFrame* pTmpFrame = GetViewFrame();
                SfxHelp::OpenHelpAgent( pTmpFrame->GetFrame(), HID_MAIL_MERGE_SELECT );
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "Dialogdiet fail!");
                AbstractMailMergeCreateFromDlg* pDlg = pFact->CreateMailMergeCreateFromDlg( DLG_MERGE_CREATE,
                                                        &pTmpFrame->GetWindow());
                DBG_ASSERT(pDlg, "Dialogdiet fail!");
                if(RET_OK == pDlg->Execute())
                    bUseCurrentDocument = pDlg->IsThisDocument();
                else
                    break;
            }
            GenerateFormLetter(bUseCurrentDocument);
        }
        break;
        case SID_SPELL_DIALOG:
        {
            SfxViewFrame* pViewFrame = GetViewFrame();
            if (rReq.GetArgs() != NULL)
                pViewFrame->SetChildWindow (SID_SPELL_DIALOG,
                    ((const SfxBoolItem&) (rReq.GetArgs()->
                        Get(SID_SPELL_DIALOG))).GetValue());
            else
                pViewFrame->ToggleChildWindow(SID_SPELL_DIALOG);

            pViewFrame->GetBindings().Invalidate(SID_SPELL_DIALOG);
            rReq.Ignore ();
        }
        break;

        case SID_ALIGN_ANY_LEFT :
        case SID_ALIGN_ANY_HCENTER  :
        case SID_ALIGN_ANY_RIGHT    :
        case SID_ALIGN_ANY_JUSTIFIED:
        case SID_ALIGN_ANY_TOP      :
        case SID_ALIGN_ANY_VCENTER  :
        case SID_ALIGN_ANY_BOTTOM   :
        case SID_ALIGN_ANY_HDEFAULT :
        case SID_ALIGN_ANY_VDEFAULT :
        {
            USHORT nAlias = 0;
            if( nSelectionType & (nsSelectionType::SEL_DRW_TXT|nsSelectionType::SEL_TXT) )
            {
                switch( nSlot )
                {
                    case SID_ALIGN_ANY_LEFT :       nAlias = SID_ATTR_PARA_ADJUST_LEFT; break;
                    case SID_ALIGN_ANY_HCENTER  :   nAlias = SID_ATTR_PARA_ADJUST_CENTER; break;
                    case SID_ALIGN_ANY_RIGHT    :   nAlias = SID_ATTR_PARA_ADJUST_RIGHT; break;
                    case SID_ALIGN_ANY_JUSTIFIED:   nAlias = SID_ATTR_PARA_ADJUST_BLOCK; break;
                    case SID_ALIGN_ANY_TOP      :   nAlias = FN_TABLE_VERT_NONE; break;
                    case SID_ALIGN_ANY_VCENTER  :   nAlias = FN_TABLE_VERT_CENTER; break;
                    case SID_ALIGN_ANY_BOTTOM   :   nAlias = FN_TABLE_VERT_BOTTOM; break;
                }
            }
            else
            {
                switch( nSlot )
                {
                    case SID_ALIGN_ANY_LEFT :       nAlias = SID_OBJECT_ALIGN_LEFT    ; break;
                    case SID_ALIGN_ANY_HCENTER  :   nAlias = SID_OBJECT_ALIGN_CENTER ; break;
                    case SID_ALIGN_ANY_RIGHT    :   nAlias = SID_OBJECT_ALIGN_RIGHT  ; break;
                    case SID_ALIGN_ANY_TOP      :   nAlias = SID_OBJECT_ALIGN_UP     ;  break;
                    case SID_ALIGN_ANY_VCENTER  :   nAlias = SID_OBJECT_ALIGN_MIDDLE ;  break;
                    case SID_ALIGN_ANY_BOTTOM   :   nAlias = SID_OBJECT_ALIGN_DOWN    ; break;
                }
            }
            //special handling for the draw shell
            if(nAlias && (nSelectionType & (nsSelectionType::SEL_DRW)))
            {
                SfxAllEnumItem aEnumItem(SID_OBJECT_ALIGN, nAlias - SID_OBJECT_ALIGN_LEFT);
                GetViewFrame()->GetDispatcher()->Execute(
                                SID_OBJECT_ALIGN, SFX_CALLMODE_ASYNCHRON, &aEnumItem, 0L);
            }
            else if(nAlias)
            //these slots are either re-mapped to text or object alignment
                GetViewFrame()->GetDispatcher()->Execute(
                                nAlias, SFX_CALLMODE_ASYNCHRON);
        }
        break;
        case SID_RESTORE_EDITING_VIEW:
        {
            //#i33307# restore editing position
            Point aCrsrPos;
            bool bSelectObj;
            if(pViewImpl->GetRestorePosition(aCrsrPos, bSelectObj))
            {
                pWrtShell->SwCrsrShell::SetCrsr( aCrsrPos, !bSelectObj );
                if( bSelectObj )
                {
                    pWrtShell->SelectObj( aCrsrPos );
                    pWrtShell->EnterSelFrmMode( &aCrsrPos );
                }
            }
        }
        break;
        case SID_INSERT_GRAPHIC:
        {
            rReq.SetReturnValue(SfxBoolItem(nSlot, InsertGraphicDlg( rReq )));
        }
        break;


        default:
            ASSERT(!this, falscher Dispatcher);
            return;
    }
    if(!bIgnore)
        rReq.Done();
}

/*--------------------------------------------------------------------
    Beschreibung:   SeitenNr-Feld invalidieren
 --------------------------------------------------------------------*/

void SwView::UpdatePageNums(USHORT nPhyNum, USHORT nVirtNum, const String& rPgStr)
{
    String sTemp(GetPageStr( nPhyNum, nVirtNum, rPgStr ));
    if ( sLstPg != sTemp )
    {
        sLstPg = sTemp;
        const SfxStringItem aTmp( FN_STAT_PAGE,
                                 sLstPg);
        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.SetState( aTmp );
        rBnd.Update( FN_STAT_PAGE );
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Status der Stauszeile
 --------------------------------------------------------------------*/


void SwView::StateStatusLine(SfxItemSet &rSet)
{
    SwWrtShell& rShell = GetWrtShell();

    SfxWhichIter aIter( rSet );
    USHORT nWhich = aIter.FirstWhich();
    ASSERT( nWhich, "leeres Set");

    while( nWhich )
    {
        switch( nWhich )
        {
            case FN_STAT_PAGE:
/*
//JP 07.01.00: is a nice feature - show the selektion of DrawObjects
            if( rShell.IsObjSelected()
//???               || rShell.IsFrmSelected()
                )
            {
                String sDisplay( rShell.GetDrawView()->GetMarkedObjectList().
                                    GetDescriptionOfMarkedObjects() );
                rSet.Put( SfxStringItem( FN_STAT_PAGE, sDisplay ));
            }
            else
*/          {
                // Anzahl der Seiten, log. SeitenNr. SeitenNr ermitteln
                USHORT nPage, nLogPage;
                String sDisplay;
                rShell.GetPageNumber( -1, rShell.IsCrsrVisible(), nPage, nLogPage, sDisplay );
                rSet.Put( SfxStringItem( FN_STAT_PAGE,
                            GetPageStr( nPage, nLogPage, sDisplay) ));

                USHORT nCnt = GetWrtShell().GetPageCnt();
                if (nPageCnt != nCnt)   // Basic benachrichtigen
                {
                    nPageCnt = nCnt;
                    SFX_APP()->NotifyEvent(SfxEventHint(SW_EVENT_PAGE_COUNT, GetViewFrame()->GetObjectShell()), FALSE);
                }
            }
            break;
            case FN_STAT_TEMPLATE:
            {
                rSet.Put(SfxStringItem( FN_STAT_TEMPLATE,
                                        rShell.GetCurPageStyle(FALSE)));

            }
            break;
            case SID_ATTR_ZOOM:
            {
                if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    const SwViewOption* pVOpt = rShell.GetViewOptions();
                    SvxZoomType eZoom = (SvxZoomType) pVOpt->GetZoomType();
                    SvxZoomItem aZoom(eZoom, pVOpt->GetZoom());
                    if(pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE))
                    {
                        aZoom.SetValueSet(
                                SVX_ZOOM_ENABLE_50|
                                SVX_ZOOM_ENABLE_75|
                                SVX_ZOOM_ENABLE_100|
                                SVX_ZOOM_ENABLE_150|
                                SVX_ZOOM_ENABLE_200);
                    }
                    rSet.Put( aZoom );
                }
                else
                    rSet.DisableItem( SID_ATTR_ZOOM );
            }
            break;
            case SID_ATTR_VIEWLAYOUT:
            {
                if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    const SwViewOption* pVOpt = rShell.GetViewOptions();
                    const USHORT nColumns  = pVOpt->GetViewLayoutColumns();
                    const bool  bBookMode = pVOpt->IsViewLayoutBookMode();
                    SvxViewLayoutItem aViewLayout(nColumns, bBookMode);
                    rSet.Put( aViewLayout );
                }
                else
                    rSet.DisableItem( SID_ATTR_VIEWLAYOUT );
            }
            break;
            case SID_ATTR_ZOOMSLIDER:
            {
                if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    const SwViewOption* pVOpt = rShell.GetViewOptions();
                    const USHORT nCurrentZoom = pVOpt->GetZoom();
                    SvxZoomSliderItem aZoomSliderItem( nCurrentZoom, MINZOOM, MAXZOOM );
                    aZoomSliderItem.AddSnappingPoint( 100 );

                    if ( !pWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) )
                    {
                        const USHORT nColumns = pVOpt->GetViewLayoutColumns();
                        const bool bAutomaticViewLayout = 0 == nColumns;
                        const SwPostItMgr* pMgr = GetPostItMgr();

                        // snapping points:
                        // automatic mode: 1 Page, 2 Pages, 100%
                        // n Columns mode: n Pages, 100%
                        // n Columns book mode: nPages without gaps, 100%
                        const SwRect aPageRect( pWrtShell->GetAnyCurRect( RECT_PAGE_CALC ) );
                        const SwRect aRootRect( pWrtShell->GetAnyCurRect( RECT_PAGES_AREA ) ); // width of columns
                        Size aPageSize( aPageRect.SSize() );
                        aPageSize.Width() += pMgr->HasNotes() && pMgr->ShowNotes() ?
                                             pMgr->GetSidebarWidth() + pMgr->GetSidebarBorderWidth() :
                                             0;

                        Size aRootSize( aRootRect.SSize() );

                        const MapMode aTmpMap( MAP_TWIP );
                        const Size& rEditSize = GetEditWin().GetOutputSizePixel();
                        const Size aWindowSize( GetEditWin().PixelToLogic( rEditSize, aTmpMap ) );

                        const long nOf = DOCUMENTBORDER * 2L;
                        long nTmpWidth = bAutomaticViewLayout ? aPageSize.Width() : aRootSize.Width();
                        nTmpWidth += nOf;
                        aPageSize.Height() += nOf;
                        long nFac = aWindowSize.Width() * 100 / nTmpWidth;

                        long nVisPercent = aWindowSize.Height() * 100 / aPageSize.Height();
                        nFac = Min( nFac, nVisPercent );

                        aZoomSliderItem.AddSnappingPoint( nFac );

                        if ( bAutomaticViewLayout )
                        {
                            nTmpWidth += aPageSize.Width() + GAPBETWEENPAGES;
                            nFac = aWindowSize.Width() * 100 / nTmpWidth;
                            nFac = Min( nFac, nVisPercent );
                            aZoomSliderItem.AddSnappingPoint( nFac );
                        }
                    }

                    rSet.Put( aZoomSliderItem );
                }
                else
                    rSet.DisableItem( SID_ATTR_ZOOMSLIDER );
            }
            break;
            case SID_ATTR_POSITION:
            case SID_ATTR_SIZE:
            {
                if( !rShell.IsFrmSelected() && !rShell.IsObjSelected() )
                    SwBaseShell::_SetFrmMode( FLY_DRAG_END );
                else
                {
                    FlyMode eFrameMode = SwBaseShell::GetFrmMode();
                    if ( eFrameMode == FLY_DRAG_START || eFrameMode == FLY_DRAG )
                    {
                        if ( nWhich == SID_ATTR_POSITION )
                            rSet.Put( SfxPointItem( SID_ATTR_POSITION,
                                                    rShell.GetAnchorObjDiff()));
                        else
                            rSet.Put( SvxSizeItem( SID_ATTR_SIZE,
                                                   rShell.GetObjSize()));
                    }
                }
            }
            break;
            case SID_TABLE_CELL:

            if( rShell.IsFrmSelected() || rShell.IsObjSelected() )
            {
                // #i39171# Don't put a SvxSizeItem into a slot which is defined as SfxStringItem.
                // SvxPosSizeStatusBarControl no longer resets to empty display if only one slot
                // has no item, so SID_TABLE_CELL can remain empty (the SvxSizeItem is supplied
                // in SID_ATTR_SIZE).
            }
            else
            {
                String sStr;
                if( rShell.IsCrsrInTbl() )
                {
                    // table name + cell coordinate
                    sStr = rShell.GetTableFmt()->GetName();
                    sStr += ':';
                    sStr += rShell.GetBoxNms();
                }
                else
                {
                    const SwSection* pCurrSect = rShell.GetCurrSection();
                    if( pCurrSect )
                    {
                        switch( pCurrSect->GetType() )
                        {
                        case TOX_HEADER_SECTION:
                        case TOX_CONTENT_SECTION:
                            {
                                const SwTOXBase* pTOX = pWrtShell->GetCurTOX();
                                if( pTOX )
                                    sStr = pTOX->GetTOXName();
                                else
                                {
                                    ASSERT( !this,
                                        "was ist das fuer ein Verzeichnis?" );
                                    sStr = pCurrSect->GetName();
                                }
                            }
                            break;
                        default:
                            sStr = pCurrSect->GetName();
                            break;
                        }
                    }
                }
                const SwNumRule* pNumRule = rShell.GetCurNumRule();
                if (pNumRule)   // Cursor in Numerierung
                {
                    BYTE nNumLevel = rShell.GetNumLevel();
                    if( IsShowNum(nNumLevel) && MAXLEVEL >
                        ( nNumLevel = GetRealLevel( nNumLevel )) )
                    {
                        if( sStr.Len() )
                            sStr.AppendAscii(sStatusDelim);
                        sStr += SW_RESSTR(STR_NUM_LEVEL);
                        sStr += String::CreateFromInt32( nNumLevel + 1 );
                        if(!pNumRule->IsAutoRule())
                        {
                            SfxItemSet aSet(GetPool(),
                                    RES_PARATR_NUMRULE, RES_PARATR_NUMRULE);
                            rShell.GetCurAttr(aSet);
                            /* const SfxPoolItem* pItem; */
                            if(SFX_ITEM_AVAILABLE <=
                               aSet.GetItemState(RES_PARATR_NUMRULE, TRUE
                                                 /*, &pItem */ ))
                            {
                                const String& rNumStyle =
                                    ((const SfxStringItem &)
                                     aSet.Get(RES_PARATR_NUMRULE)).GetValue();
                                /* #i5116# GetItemState does not necessarily
                                   change pItem */
                                // ((const SfxStringItem*)pItem)->GetValue();
                                if(rNumStyle.Len())
                                {
                                    sStr.AppendAscii(sStatusDelim);
                                    sStr += rNumStyle;
                                }
                            }
                        }
                    }
                }

                if( rShell.HasReadonlySel() )
                {
                    if( sStr.Len() )
                        sStr.InsertAscii( sStatusDelim, 0 );
                    sStr.Insert( SW_RESSTR( STR_READONLY_SEL ), 0 );
                }
                if( sStr.Len() )
                    rSet.Put( SfxStringItem( SID_TABLE_CELL, sStr ));
            }
            break;
            case FN_STAT_SELMODE:
            {
                if(rShell.IsStdMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 0));
                else if(rShell.IsAddMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 2));
                else if(rShell.IsBlockMode())
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 3));
                else
                    rSet.Put(SfxUInt16Item(FN_STAT_SELMODE, 1));
                break;
            }
            case SID_ATTR_INSERT:
                if( rShell.IsRedlineOn() )
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put(SfxBoolItem(SID_ATTR_INSERT,rShell.IsInsMode()));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:   Execute fuer die Stauszeile
 --------------------------------------------------------------------*/


void SwView::ExecuteStatusLine(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetWrtShell();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem=NULL;
    BOOL bUp = FALSE;
    USHORT nWhich = rReq.GetSlot();
    switch( nWhich )
    {
        case FN_STAT_PAGE:
        {
            GetViewFrame()->GetDispatcher()->Execute( SID_NAVIGATOR,
                                      SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
        }
        break;

        case FN_STAT_BOOKMARK:
        if ( pArgs )
        {
            if (SFX_ITEM_SET == pArgs->GetItemState( nWhich, TRUE, &pItem))
            {
                USHORT nDest;
                nDest = ((const SfxUInt16Item *)pItem)->GetValue();
                rSh.EnterStdMode();
                rSh.GotoBookmark( nDest );
            }
        }
        break;

        case FN_STAT_TEMPLATE:
        {
            GetViewFrame()->GetDispatcher()->Execute(FN_FORMAT_PAGE_DLG,
                                        SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
        }
        break;
        case SID_ATTR_ZOOM:
        {
            if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                const SfxItemSet *pSet = 0;
                AbstractSvxZoomDialog *pDlg = 0;
                if ( pArgs )
                    pSet = pArgs;
                else if ( GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    const SwViewOption& rViewOptions = *rSh.GetViewOptions();
                    SfxItemSet aCoreSet(pShell->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM, SID_ATTR_VIEWLAYOUT, SID_ATTR_VIEWLAYOUT, 0 );
                    SvxZoomItem aZoom( (SvxZoomType)rViewOptions.GetZoomType(), rViewOptions.GetZoom() );

                    const bool bBrowseMode = rSh.getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE);
                    if( bBrowseMode )
                    {
                        aZoom.SetValueSet(
                                SVX_ZOOM_ENABLE_50|
                                SVX_ZOOM_ENABLE_75|
                                SVX_ZOOM_ENABLE_100|
                                SVX_ZOOM_ENABLE_150|
                                SVX_ZOOM_ENABLE_200);
                    }
                    aCoreSet.Put( aZoom );

                    // PAGES01
                    if ( !bBrowseMode )
                    {
                        const SvxViewLayoutItem aViewLayout( rViewOptions.GetViewLayoutColumns(), rViewOptions.IsViewLayoutBookMode() );
                        aCoreSet.Put( aViewLayout );
                    }

                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        pDlg = pFact->CreateSvxZoomDialog(&GetViewFrame()->GetWindow(), aCoreSet, RID_SVXDLG_ZOOM);
                        DBG_ASSERT(pDlg, "Dialogdiet fail!");
                    }

                    pDlg->SetLimits( MINZOOM, MAXZOOM );

                    if( pDlg->Execute() != RET_CANCEL )
                        pSet = pDlg->GetOutputItemSet();
                }

                // PAGES01
                const SfxPoolItem* pViewLayoutItem = 0;
                if ( pSet && SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_VIEWLAYOUT, TRUE, &pViewLayoutItem))
                {
                    const USHORT nColumns = ((const SvxViewLayoutItem *)pViewLayoutItem)->GetValue();
                    const bool bBookMode  = ((const SvxViewLayoutItem *)pViewLayoutItem)->IsBookMode();
                    SetViewLayout( nColumns, bBookMode );
                }

                if ( pSet && SFX_ITEM_SET == pSet->GetItemState(SID_ATTR_ZOOM, TRUE, &pItem))
                {
                    enum SvxZoomType eType = ((const SvxZoomItem *)pItem)->GetType();
                    SetZoom( eType, ((const SvxZoomItem *)pItem)->GetValue() );
                }
                bUp = TRUE;
                if ( pItem )
                    rReq.AppendItem( *pItem );
                rReq.Done();

                delete pDlg;
            }
        }
        break;

        case SID_ATTR_VIEWLAYOUT:
        {
            if ( pArgs && !rSh.getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE) &&
                 GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                // PAGES01
                if ( SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_VIEWLAYOUT, TRUE, &pItem ))
                {
                    const USHORT nColumns = ((const SvxViewLayoutItem *)pItem)->GetValue();
                    const bool bBookMode  = (0 == nColumns || 0 != (nColumns % 2)) ?
                                            false :
                                            ((const SvxViewLayoutItem *)pItem)->IsBookMode();

                    SetViewLayout( nColumns, bBookMode );
                }

                bUp = TRUE;
                rReq.Done();

                InvalidateRulerPos();
            }
        }
        break;

        case SID_ATTR_ZOOMSLIDER:
        {
            if ( pArgs && GetDocShell()->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
            {
                // PAGES01
                if ( SFX_ITEM_SET == pArgs->GetItemState(SID_ATTR_ZOOMSLIDER, TRUE, &pItem ))
                {
                    const USHORT nCurrentZoom = ((const SvxZoomSliderItem *)pItem)->GetValue();
                    SetZoom( SVX_ZOOM_PERCENT, nCurrentZoom );
                }

                bUp = TRUE;
                rReq.Done();
            }
        }
        break;

        case SID_ATTR_SIZE:
        {
            ULONG nId = FN_INSERT_FIELD;
            if( rSh.IsCrsrInTbl() )
                nId = FN_FORMAT_TABLE_DLG;
            else if( rSh.GetCurTOX() )
                nId = FN_INSERT_MULTI_TOX;
            else if( rSh.GetCurrSection() )
                nId = FN_EDIT_REGION;
            else
            {
                const SwNumRule* pNumRule = rSh.GetCurNumRule();
                if( pNumRule )  // Cursor in Numerierung
                {
                    if( pNumRule->IsAutoRule() )
                        nId = FN_NUMBER_BULLETS;
                    else
                    {
                        // Dialog vom Gestalter starten ;-)
                        nId = 0;
                    }
                }
                else if( rSh.IsFrmSelected() )
                    nId = FN_FORMAT_FRAME_DLG;
                else if( rSh.IsObjSelected() )
                    nId = SID_ATTR_TRANSFORM;
            }
            if( nId )
                GetViewFrame()->GetDispatcher()->Execute(
                    static_cast< USHORT >( nId ), SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD );
        }
        break;

        case FN_STAT_SELMODE:
        {
            if ( pArgs )
            {
                if (SFX_ITEM_SET == pArgs->GetItemState( nWhich, TRUE, &pItem))
                {
                    switch ( ((const SfxUInt16Item *)pItem)->GetValue() )
                    {
                        case 0: rSh.EnterStdMode(); break;
                        case 1: rSh.EnterExtMode(); break;
                        case 2: rSh.EnterAddMode(); break;
                        case 3: rSh.EnterBlockMode(); break;
                    }
                }
            }
            else
            {

                if( !rSh.IsAddMode() && !rSh.IsExtMode() && !rSh.IsBlockMode() )
                    rSh.ToggleExtMode();
                else if ( rSh.IsExtMode() )
                {
                    rSh.ToggleExtMode();
                    rSh.ToggleAddMode();
                }
                else if ( rSh.IsAddMode() )
                {
                    rSh.ToggleAddMode();
                    rSh.ToggleBlockMode();
                }
                else
                    rSh.ToggleBlockMode();
            }
            bUp = TRUE;
            break;
        }
        case FN_SET_ADD_MODE:
            rSh.ToggleAddMode();
            nWhich = FN_STAT_SELMODE;
            bUp = TRUE;
        break;
        case FN_SET_BLOCK_MODE:
            rSh.ToggleBlockMode();
            nWhich = FN_STAT_SELMODE;
            bUp = TRUE;
        break;
        case FN_SET_EXT_MODE:
            rSh.ToggleExtMode();
            nWhich = FN_STAT_SELMODE;
            bUp = TRUE;
        break;
        case SID_ATTR_INSERT:
            rSh.ToggleInsMode();
            bUp = TRUE;
        break;

    }
    if ( bUp )
    {
        SfxBindings &rBnd = GetViewFrame()->GetBindings();
        rBnd.Invalidate(nWhich);
        rBnd.Update(nWhich);
    }
}




void SwView::InsFrmMode(USHORT nCols)
{
    if ( pWrtShell->HasWholeTabSelection() )
    {
        SwFlyFrmAttrMgr aMgr( TRUE, pWrtShell, FRMMGR_TYPE_TEXT );

        const SwFrmFmt &rPageFmt =
                pWrtShell->GetPageDesc(pWrtShell->GetCurPageDesc()).GetMaster();
        SwTwips lWidth = rPageFmt.GetFrmSize().GetWidth();
        const SvxLRSpaceItem &rLR = rPageFmt.GetLRSpace();
        lWidth -= rLR.GetLeft() + rLR.GetRight();
        aMgr.SetSize(Size(lWidth, aMgr.GetSize().Height()));
        if(nCols > 1)
        {
            SwFmtCol aCol;
            aCol.Init( nCols, aCol.GetGutterWidth(), aCol.GetWishWidth() );
            aMgr.SetCol( aCol );
        }
        aMgr.InsertFlyFrm();
    }
    else
        GetEditWin().InsFrm(nCols);
}

/*--------------------------------------------------------------------
    Beschreibung:   Links bearbeiten
 --------------------------------------------------------------------*/

void SwView::EditLinkDlg()
{
    BOOL bWeb = 0 != PTR_CAST(SwWebView, this);
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractLinksDialog* pDlg = pFact->CreateLinksDialog( &GetViewFrame()->GetWindow(), &GetWrtShell().GetLinkManager(), bWeb );
    if ( pDlg )
    {
        pDlg->Execute();
        delete pDlg;
    }
/*
    SwLinkDlg* pDlg = new SwLinkDlg(GetFrameWindow());
    pDlg->SetShell(&GetWrtShell());
    pDlg->Execute();

    DELETEZ(pDlg);
*/
}

BOOL SwView::JumpToSwMark( const String& rMark )
{
    BOOL bRet = FALSE;
    if( rMark.Len() )
    {
        // wir wollen den Bookmark aber am oberen Rand haben
        BOOL bSaveCC = IsCrsrAtCenter();
        BOOL bSaveCT = IsCrsrAtTop();
        SetCrsrAtTop( TRUE );

        //JP 27.04.98: Bug 49786
        // Damit in FrameSet auch gescrollt werden kann, muss die
        // entsprechende Shell auch das Focus-Flag gesetzt haben!
        BOOL bHasShFocus = pWrtShell->HasShFcs();
        if( !bHasShFocus )
            pWrtShell->ShGetFcs( FALSE );

        const SwFmtINetFmt* pINet;
        String sCmp, sMark( INetURLObject::decode( rMark, INET_HEX_ESCAPE,
                                           INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 ));

        xub_StrLen nLastPos, nPos = sMark.Search( cMarkSeperator );
        if( STRING_NOTFOUND != nPos )
            while( STRING_NOTFOUND != ( nLastPos =
                sMark.Search( cMarkSeperator, nPos + 1 )) )
                nPos = nLastPos;

        if( STRING_NOTFOUND != nPos &&
            ( sCmp = sMark.Copy( nPos + 1 ) ).EraseAllChars().Len() )
        {
            String sName( sMark.Copy( 0, nPos ) );
            sCmp.ToLowerAscii();
            FlyCntType eFlyType = FLYCNTTYPE_ALL;

            if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToRegion ) )
            {
                pWrtShell->EnterStdMode();
                bRet = pWrtShell->GotoRegion( sName );
            }
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToOutline ) )
            {
                pWrtShell->EnterStdMode();
                bRet = pWrtShell->GotoOutline( sName );
            }
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToFrame ) )
                eFlyType = FLYCNTTYPE_FRM;
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToGraphic ) )
                eFlyType = FLYCNTTYPE_GRF;
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToOLE ) )
                eFlyType = FLYCNTTYPE_OLE;
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToTable ) )
            {
                pWrtShell->EnterStdMode();
                bRet = pWrtShell->GotoTable( sName );
            }
            else if( COMPARE_EQUAL == sCmp.CompareToAscii( pMarkToText ) )
            {
                // Normale Textsuche
                pWrtShell->EnterStdMode();

                SearchOptions aSearchOpt(
                                    SearchAlgorithms_ABSOLUTE, 0,
                                    sName, rtl::OUString(),
                                    SvxCreateLocale( LANGUAGE_SYSTEM ),
                                    0,0,0,
                                    TransliterationModules_IGNORE_CASE );

                if( pWrtShell->SearchPattern( aSearchOpt, DOCPOS_START, DOCPOS_END ))
                {
                    pWrtShell->EnterStdMode();      // Selektion wieder aufheben
                    bRet = TRUE;
                }
            }
            else if( USHRT_MAX != ( nPos = pWrtShell->FindBookmark( sMark ) ))
                pWrtShell->GotoBookmark( nPos, FALSE, TRUE ), bRet = TRUE;
            else if( 0 != ( pINet = pWrtShell->FindINetAttr( sMark ) ))
                bRet = pWrtShell->GotoINetAttr( *pINet->GetTxtINetFmt() );

            // fuer alle Arten von Flys
            if( FLYCNTTYPE_ALL != eFlyType && pWrtShell->GotoFly( sName, eFlyType ))
            {
                bRet = TRUE;
                if( FLYCNTTYPE_FRM == eFlyType )
                {
                    // TextFrames: Cursor in den Frame setzen
                    pWrtShell->UnSelectFrm();
                    pWrtShell->LeaveSelFrmMode();
                }
                else
                {
                    pWrtShell->HideCrsr();
                    pWrtShell->EnterSelFrmMode();
                }
            }
        }
        else if( USHRT_MAX != ( nPos = pWrtShell->FindBookmark( sMark ) ))
            pWrtShell->GotoBookmark( nPos, FALSE, TRUE ), bRet = TRUE;
        else if( 0 != ( pINet = pWrtShell->FindINetAttr( sMark ) ))
            bRet = pWrtShell->GotoINetAttr( *pINet->GetTxtINetFmt() );

        // #b6330459# make selection visible later
        if ( aVisArea.IsEmpty() )
            bMakeSelectionVisible = sal_True;

        // ViewStatus wieder zurueck setzen
        SetCrsrAtTop( bSaveCT, bSaveCC );

        if( !bHasShFocus )
            pWrtShell->ShLooseFcs();
    }
    return bRet;
}

// #i67305, #1367991: Undo after insert from file:
// Undo "Insert form file" crashes with documents imported from binary filter (.sdw) => disabled
// Undo "Insert form file" crashes with (.odt) documents crashes if these documents contains
// page styles with active header/footer => disabled for those documents

sal_uInt16 lcl_PageDescWithHeader( const SwDoc& rDoc )
{
    sal_uInt16 nRet = 0;
    sal_uInt16 nCnt = rDoc.GetPageDescCnt();
    for( sal_uInt16 i = 0; i < nCnt; ++i )
    {
        const SwPageDesc& rPageDesc = rDoc.GetPageDesc( i );
        const SwFrmFmt& rMaster = rPageDesc.GetMaster();
        const SfxPoolItem* pItem;
        if( ( SFX_ITEM_SET == rMaster.GetAttrSet().GetItemState( RES_HEADER, FALSE, &pItem ) &&
              ((SwFmtHeader*)pItem)->IsActive() ) ||
            ( SFX_ITEM_SET == rMaster.GetAttrSet().GetItemState( RES_FOOTER, FALSE, &pItem )  &&
              ((SwFmtFooter*)pItem)->IsActive()) )
            ++nRet;
    }
    return nRet; // number of page styles with active header/footer
}

/*--------------------------------------------------------------------
    Beschreibung:   Links bearbeiten
 --------------------------------------------------------------------*/

void SwView::ExecuteInsertDoc( SfxRequest& rRequest, const SfxPoolItem* pItem )
{
    pViewImpl->InitRequest( rRequest );
    pViewImpl->SetParam( pItem ? 1 : 0 );
    USHORT nSlot = rRequest.GetSlot();

    if ( !pItem )
    {
        String sEmpty;
        InsertDoc( nSlot, sEmpty, sEmpty );
    }
    else
    {
        String sFile, sFilter;
        sFile = ( (const SfxStringItem *)pItem )->GetValue();
        if ( SFX_ITEM_SET == rRequest.GetArgs()->GetItemState( FN_PARAM_1, TRUE, &pItem ) )
            sFilter = ( (const SfxStringItem *)pItem )->GetValue();

        bool bHasFileName = ( sFile.Len() > 0 );
        long nFound = InsertDoc( nSlot, sFile, sFilter );

        if ( bHasFileName )
        {
            rRequest.SetReturnValue( SfxBoolItem( nSlot, nFound != -1 ) );
            rRequest.Done();
        }
    }
}

long SwView::InsertDoc( USHORT nSlotId, const String& rFileName, const String& rFilterName, INT16 nVersion )
{
    SfxMedium* pMed = 0;
    SwDocShell* pDocSh = GetDocShell();

    if( rFileName.Len() )
    {
        SfxObjectFactory& rFact = pDocSh->GetFactory();
        const SfxFilter* pFilter = rFact.GetFilterContainer()->GetFilter4FilterName( rFilterName );
        if ( !pFilter )
        {
            pMed = new SfxMedium(rFileName, STREAM_READ, TRUE, 0, 0 );
            SfxFilterMatcher aMatcher( rFact.GetFilterContainer()->GetName() );
            pMed->UseInteractionHandler( TRUE );
            ErrCode nErr = aMatcher.GuessFilter( *pMed, &pFilter, FALSE );
            if ( nErr )
                DELETEZ(pMed);
            else
                pMed->SetFilter( pFilter );
        }
        else
            pMed = new SfxMedium(rFileName, STREAM_READ, TRUE, pFilter, 0);
    }
    else
    {
        String sFactory = String::CreateFromAscii( pDocSh->GetFactory().GetShortName() );
        pViewImpl->StartDocumentInserter( sFactory, LINK( this, SwView, DialogClosedHdl ) );
        return -1;
    }

    if( !pMed )
        return -1;

    return InsertMedium( nSlotId, pMed, nVersion );
}

long SwView::InsertMedium( USHORT nSlotId, SfxMedium* pMedium, INT16 nVersion )
{
    BOOL bInsert = FALSE, bCompare = FALSE, bMerge = FALSE;
    long nFound = 0;
    SwDocShell* pDocSh = GetDocShell();

    switch( nSlotId )
    {
        case SID_DOCUMENT_MERGE:        bMerge = TRUE;      break;
        case SID_DOCUMENT_COMPARE:      bCompare = TRUE;    break;
        case SID_INSERTDOC:             bInsert = TRUE;     break;

        default:
            ASSERT( !this, "Unbekannte SlotId!" );
            bInsert = TRUE;
            nSlotId = SID_INSERTDOC;
            break;
    }

    if( bInsert )
    {
        uno::Reference< frame::XDispatchRecorder > xRecorder =
                GetViewFrame()->GetBindings().GetRecorder();
        if ( xRecorder.is() )
        {
            SfxRequest aRequest(GetViewFrame(), SID_INSERTDOC);
            aRequest.AppendItem(SfxStringItem(SID_INSERTDOC, pMedium->GetOrigURL()));
            if(pMedium->GetFilter())
            aRequest.AppendItem(SfxStringItem(FN_PARAM_1, pMedium->GetFilter()->GetName()));
            aRequest.Done();
        }

        SfxObjectShellRef aRef( pDocSh );

        sal_uInt32 nError = SfxObjectShell::HandleFilter( pMedium, pDocSh );
        // #i16722# aborted?
        if(nError != ERRCODE_NONE)
        {
            delete pMedium;
            return -1;
        }
        pDocSh->RegisterTransfer( *pMedium );
        pMedium->DownLoad();    // ggfs. den DownLoad anstossen
        if( aRef.Is() && 1 < aRef->GetRefCount() )  // noch gueltige Ref?
        {
            SwReader* pRdr;
            Reader *pRead = pDocSh->StartConvertFrom( *pMedium, &pRdr, pWrtShell );
            if( pRead ||
                (pMedium->GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER) != 0 )
            {
                sal_uInt16 nUndoCheck = 0;
                SwDoc *pDoc = pDocSh->GetDoc();
                if( pRead && pDocSh->GetDoc() )
                    nUndoCheck = lcl_PageDescWithHeader( *pDoc );
                ULONG nErrno;
                {   //Scope for SwWait-Object, to be able to execute slots
                    //outside this scope.
                    SwWait aWait( *GetDocShell(), TRUE );
                    pWrtShell->StartAllAction();
                    if ( pWrtShell->HasSelection() )
                        pWrtShell->DelRight();      // Selektionen loeschen
                    if( pRead )
                    {
                        nErrno = pRdr->Read( *pRead );  // und Dokument einfuegen
                        delete pRdr;
                    }
                    else
                    {
                        sal_Bool bUndo = pDoc->DoesUndo();
                        pDoc->DoUndo( sal_False );
                        nErrno = pDocSh->ImportFrom( *pMedium ) ? 0 : ERR_SWG_READ_ERROR;
                        pDoc->DoUndo( bUndo );
                    }

                }

                // ggfs. alle Verzeichnisse updaten:
                if( pWrtShell->IsUpdateTOX() )
                {
                    SfxRequest aReq( FN_UPDATE_TOX, SFX_CALLMODE_SLOT, GetPool() );
                    Execute( aReq );
                    pWrtShell->SetUpdateTOX( FALSE );       // wieder zurueck setzen
                }

                if( pDoc )
                { // Disable Undo for .sdw (136991) or
                  // if the number of page styles with header/footer has changed (#i67305)
                    if( !pRead || nUndoCheck != lcl_PageDescWithHeader( *pDoc ) )
                        pDoc->DelAllUndoObj();
                }

                pWrtShell->EndAllAction();
                if( nErrno )
                {
                    ErrorHandler::HandleError( nErrno );
                    nFound = IsError( nErrno ) ? -1 : 0;
                }
                else
                    nFound = 0;
            }
        }
    }
    else
    {
        SfxObjectShellRef xDocSh;

extern int lcl_FindDocShell( SfxObjectShellRef& xDocSh,
                            const String& rFileName, const String& rPasswd,
                            String& rFilter, INT16 nVersion,
                            SwDocShell* pDestSh );

        String sFltNm;
        int nRet = lcl_FindDocShell( xDocSh, pMedium->GetName(), aEmptyStr,
                                    sFltNm, nVersion, pDocSh );
        if( nRet )
        {
            SwWait aWait( *GetDocShell(), TRUE );
            pWrtShell->StartAllAction();

            pWrtShell->EnterStdMode();          // Selektionen loeschen

            if( bCompare )
                nFound = pWrtShell->CompareDoc( *((SwDocShell*)&xDocSh)->GetDoc() );
            else
                nFound = pWrtShell->MergeDoc( *((SwDocShell*)&xDocSh)->GetDoc() );

            pWrtShell->EndAllAction();

            if (!bCompare && !nFound)
            {
                Window* pWin = &GetEditWin();
                InfoBox(pWin, SW_RES(MSG_NO_MERGE_ENTRY)).Execute();
            }
        }
        if( 2 == nRet && xDocSh.Is() )
            xDocSh->DoClose();
    }

    delete pMedium;
    return nFound;
}
/* -----------------05.02.2003 12:06-----------------
 *
 * --------------------------------------------------*/
void SwView::EnableMailMerge(BOOL bEnable )
{
    bInMailMerge = bEnable;
    SfxBindings& rBind = GetViewFrame()->GetBindings();
    rBind.Invalidate(FN_INSERT_FIELD_DATA_ONLY);
    rBind.Update(FN_INSERT_FIELD_DATA_ONLY);
}
/*
*/
namespace
{
    sal_Bool lcl_NeedAdditionalDataSource( const uno::Reference< XNameAccess >& _rDatasourceContext )
    {
        Sequence < OUString > aNames = _rDatasourceContext->getElementNames();

        return  (   !aNames.getLength()
                ||  (   ( 1 == aNames.getLength() )
                    &&  aNames.getConstArray()[0] == SW_MOD()->GetDBConfig()->GetBibliographySource().sDataSource
                    )
                );
    }
}

/* -----------------27.11.2002 12:12-----------------
 *
 * --------------------------------------------------*/

class SwMergeSourceWarningBox_Impl : public ModalDialog
{
        FixedInfo       aMessageFI;
        OKButton        aOK;
        CancelButton    aCancel;

        FixedImage      aWarnImage;
    public:
        SwMergeSourceWarningBox_Impl( Window* pParent ) :
            ModalDialog( pParent, SW_RES( DLG_MERGE_SOURCE_UNAVAILABLE   ) ),
                    aMessageFI( this, SW_RES( ST_MERGE_SOURCE_UNAVAILABLE ) ),
                    aOK(        this, SW_RES( PB_MERGE_OK                 ) ),
                    aCancel(    this, SW_RES( PB_MERGE_CANCEL             ) ),
                    aWarnImage( this, SW_RES( IMG_MERGE                   ) )
                    {
                        FreeResource();
                        SetText( Application::GetDisplayName() );
                        const Image& rImg = WarningBox::GetStandardImage();
                        aWarnImage.SetImage( rImg );
                        Size aImageSize( rImg.GetSizePixel() );
                        aImageSize.Width()  += 4;
                        aImageSize.Height() += 4;
                        aWarnImage.SetSizePixel( aImageSize );

                        aImageSize.Width() += aWarnImage.GetPosPixel().X();
                        Size aSz(GetSizePixel());
                        aSz.Width() += aImageSize.Width();
                        SetSizePixel(aSz);

                        Point aPos(aMessageFI.GetPosPixel());
                        aPos.X() += aImageSize.Width();
                        aMessageFI.SetPosPixel( aPos );

                        aPos = aOK.GetPosPixel();
                        aPos.X() += aImageSize.Width();
                        aOK.SetPosPixel( aPos );
                        aPos = aCancel.GetPosPixel();
                        aPos.X() += aImageSize.Width();
                        aCancel.SetPosPixel( aPos );

                    }

        String          GetMessText() const { return aMessageFI.GetText(); }
        void            SetMessText( const String& rText ) { aMessageFI.SetText( rText ); }
};




void SwView::GenerateFormLetter(BOOL bUseCurrentDocument)
{
    if(bUseCurrentDocument)
    {
        if(!GetWrtShell().IsAnyDatabaseFieldInDoc())
        {
            //check availability of data sources (except biblio source)
            uno::Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
            uno::Reference<XNameAccess>  xDBContext;
            if( xMgr.is() )
            {
                uno::Reference<XInterface> xInstance = xMgr->createInstance(
                    OUString::createFromAscii( "com.sun.star.sdb.DatabaseContext" ));
                xDBContext = uno::Reference<XNameAccess>(xInstance, UNO_QUERY) ;
            }
            if(!xDBContext.is())
                return ;
            BOOL bCallAddressPilot = FALSE;
            if ( lcl_NeedAdditionalDataSource( xDBContext ) )
            {
                // no data sources are available - create a new one
                WarningBox aWarning(
                            &GetViewFrame()->GetWindow(),
                            SW_RES(MSG_DATA_SOURCES_UNAVAILABLE));
                // no cancel allowed
                if ( RET_OK != aWarning.Execute() )
                    return;
                bCallAddressPilot = TRUE;
            }
            else
            {
                //take an existing data source or create a new one?
                    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "Dialogdiet fail!");
                    AbstractMailMergeFieldConnectionsDlg* pConnectionsDlg = pFact->CreateMailMergeFieldConnectionsDlg(
                                                        DLG_MERGE_FIELD_CONNECTIONS,
                                                        &GetViewFrame()->GetWindow());
                    DBG_ASSERT(pConnectionsDlg, "Dialogdiet fail!");
                    if(RET_OK == pConnectionsDlg->Execute())
                        bCallAddressPilot = !pConnectionsDlg->IsUseExistingConnections();
                    else
                        return;

            }
            if(bCallAddressPilot)
            {
                GetViewFrame()->GetDispatcher()->Execute(
                                SID_ADDRESS_DATA_SOURCE, SFX_CALLMODE_SYNCHRON);
                if ( lcl_NeedAdditionalDataSource( xDBContext ) )
                    // no additional data source has been created
                    // -> assume that the user has cancelled the pilot
                    return;
            }

            //call insert fields with database field page available, only
            SfxViewFrame* pVFrame = GetViewFrame();
            //at first hide the default field dialog if currently visible
            pVFrame->SetChildWindow(FN_INSERT_FIELD, FALSE);
            //enable the status of the db field dialog - it is disabled in the status method
            //to prevent creation of the dialog without mail merge active
            EnableMailMerge();
            //then show the "Data base only" field dialog
            SfxBoolItem aOn(FN_INSERT_FIELD_DATA_ONLY, TRUE);
            pVFrame->GetDispatcher()->Execute(FN_INSERT_FIELD_DATA_ONLY,
                                                SFX_CALLMODE_SYNCHRON, &aOn, 0L);
            return;
        }
        else
        {
            // check whether the
            String sSource;
            if(!GetWrtShell().IsFieldDataSourceAvailable(sSource))
            {
                SwMergeSourceWarningBox_Impl aWarning( &GetViewFrame()->GetWindow());
                String sTmp(aWarning.GetMessText());
                sTmp.SearchAndReplaceAscii("%1", sSource);
                aWarning.SetMessText(sTmp);
                if(RET_OK == aWarning.Execute())
                {
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    if ( pFact )
                    {
                        VclAbstractDialog* pDlg = pFact->CreateVclDialog( NULL, SID_OPTIONS_DATABASES );
                        pDlg->Execute();
                        delete pDlg;
                    }
                }
                return ;
            }
        }
        SwNewDBMgr* pNewDBMgr = GetWrtShell().GetNewDBMgr();

        SwDBData aData;
        SwWrtShell &rSh = GetWrtShell();
        aData = rSh.GetDBData();
        rSh.EnterStdMode(); // Wechsel in Textshell erzwingen; ist fuer
                            // das Mischen von DB-Feldern notwendig.
        AttrChangedNotify( &rSh );
        pNewDBMgr->SetMergeType( DBMGR_MERGE );

        if (pNewDBMgr)
        {
            Sequence<PropertyValue> aProperties(3);
            PropertyValue* pValues = aProperties.getArray();
            pValues[0].Name = C2U("DataSourceName");
            pValues[1].Name = C2U("Command");
            pValues[2].Name = C2U("CommandType");
            pValues[0].Value <<= aData.sDataSource;
            pValues[1].Value <<= aData.sCommand;
            pValues[2].Value <<= aData.nCommandType;
            pNewDBMgr->ExecuteFormLetter(GetWrtShell(), aProperties, TRUE);
        }
    }
    else
    {
        //call documents and template dialog
        SfxApplication* pSfxApp = SFX_APP();
        Window* pTopWin = pSfxApp->GetTopWindow();
        SvtDocumentTemplateDialog* pDocTemplDlg = new SvtDocumentTemplateDialog( pTopWin );
        pDocTemplDlg->SelectTemplateFolder();

        int nRet = pDocTemplDlg->Execute();
        sal_Bool bNewWin = sal_False;
        if ( nRet == RET_OK )
        {
            if ( pTopWin != pSfxApp->GetTopWindow() )
            {
                // the dialogue opens a document -> a new TopWindow appears
                pTopWin = pSfxApp->GetTopWindow();
                bNewWin = sal_True;
            }
        }

        delete pDocTemplDlg;
        if ( bNewWin )
            // after the destruction of the dialogue its parent comes to top,
            // but we want that the new document is on top
            pTopWin->ToTop();

//        return;
    }
}

IMPL_LINK( SwView, DialogClosedHdl, sfx2::FileDialogHelper*, _pFileDlg )
{
    if ( ERRCODE_NONE == _pFileDlg->GetError() )
    {
        SfxMedium* pMed = pViewImpl->CreateMedium();
        if ( pMed )
        {
            USHORT nSlot = pViewImpl->GetRequest()->GetSlot();
            long nFound = InsertMedium( nSlot, pMed, pViewImpl->GetParam() );

            if ( SID_INSERTDOC == nSlot )
            {
                if ( pViewImpl->GetParam() == 0 )
                {
                    pViewImpl->GetRequest()->SetReturnValue( SfxBoolItem( nSlot, nFound != -1 ) );
                    pViewImpl->GetRequest()->Ignore();
                }
                else
                {
                    pViewImpl->GetRequest()->SetReturnValue( SfxBoolItem( nSlot, nFound != -1 ) );
                    pViewImpl->GetRequest()->Done();
                }
            }
            else if ( SID_DOCUMENT_COMPARE == nSlot || SID_DOCUMENT_MERGE == nSlot )
            {
                pViewImpl->GetRequest()->SetReturnValue( SfxInt32Item( nSlot, nFound ) );

                if ( nFound > 0 ) // Redline-Browser anzeigen
                {
                    SfxViewFrame* pVFrame = GetViewFrame();
                    pVFrame->ShowChildWindow(FN_REDLINE_ACCEPT);

                    // RedlineDlg neu initialisieren
                    USHORT nId = SwRedlineAcceptChild::GetChildWindowId();
                    SwRedlineAcceptChild* pRed = (SwRedlineAcceptChild*)pVFrame->GetChildWindow( nId );
                    if ( pRed )
                        pRed->ReInitDlg( GetDocShell() );
                }
            }
        }
    }
    return 0;
}

