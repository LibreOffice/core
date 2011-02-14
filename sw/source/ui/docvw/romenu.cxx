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
#include "precompiled_sw.hxx"
#include <hintids.hxx>


#include <tools/urlobj.hxx>
#include <vcl/graph.hxx>
#include <vcl/msgbox.hxx>
#include <sot/formats.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/filter.hxx>
#include <svtools/imap.hxx>
#include <svtools/inetimg.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/gallery.hxx>
#include <editeng/brshitem.hxx>


#include <swunodef.hxx>
#include <frmatr.hxx>
#include <fmturl.hxx>
#include <fmtinfmt.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <romenu.hxx>
#include <pagedesc.hxx>
#include <modcfg.hxx>

#include <cmdid.h>
#include <helpid.h>
#include <docvw.hrc>
#include <docvw.hrc>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <sfx2/filedlghelper.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;

SwReadOnlyPopup::~SwReadOnlyPopup()
{
    String *pDel = (String*)aThemeList.First();
    while ( pDel )
    {
        delete pDel;
        pDel = (String*)aThemeList.Next();
    }
    delete pImageMap;
    delete pTargetURL;
}



void SwReadOnlyPopup::Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher &rDis )
{
    SfxPoolItem *_pItem = 0;
    SfxItemState eState = rDis.GetBindings()->QueryState( nSID, _pItem );
    if (eState >= SFX_ITEM_AVAILABLE)
    {
        EnableItem( nMID, sal_True );
        if (_pItem)
        {
            CheckItem ( nMID, !_pItem->ISA(SfxVoidItem) &&
                            _pItem->ISA(SfxBoolItem) &&
                            ((SfxBoolItem*)_pItem)->GetValue());
            //remove full screen entry when not in full screen mode
            if(SID_WIN_FULLSCREEN == nSID && !IsItemChecked(SID_WIN_FULLSCREEN) )
                EnableItem(nMID, sal_False);
        }
    }
    else
        EnableItem( nMID, sal_False );

    delete _pItem;
}


SwReadOnlyPopup::SwReadOnlyPopup( const Point &rDPos, SwView &rV ) :
    PopupMenu( SW_RES(MN_READONLY_POPUP) ),
    rView  ( rV ),
    rDocPos( rDPos ),
    pImageMap( 0 ),
    pTargetURL( 0 )
{
    bGrfToGalleryAsLnk = SW_MOD()->GetModuleConfig()->IsGrfToGalleryAsLnk();
    SwWrtShell &rSh = rView.GetWrtShell();
    rSh.IsURLGrfAtPos( rDocPos, &sURL, &sTargetFrameName, &sDescription );
    if ( !sURL.Len() )
    {
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
        if( rSh.GetContentAtPos( rDocPos, aCntntAtPos, sal_False))
        {
            SwFmtINetFmt &rIItem = *(SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr;
            sURL = rIItem.GetValue();
            sTargetFrameName = rIItem.GetTargetFrame();
            sDescription = aCntntAtPos.sStr;
        }
    }

    sal_Bool bLink = sal_False;
    const Graphic *pGrf;
    if ( 0 == (pGrf = rSh.GetGrfAtPos( rDocPos, sGrfName, bLink )) )
    {
        EnableItem( MN_READONLY_SAVEGRAPHIC, sal_False );
        EnableItem( MN_READONLY_COPYGRAPHIC, sal_False );
    }
    else
    {
        aGraphic = *pGrf;
        const SwFrmFmt* pGrfFmt = rSh.GetFmtFromObj( rDocPos );
        const SfxPoolItem* pURLItem;
        if( pGrfFmt && SFX_ITEM_SET == pGrfFmt->GetItemState(
            RES_URL, sal_True, &pURLItem ))
        {
            const SwFmtURL& rURL = *(SwFmtURL*)pURLItem;
            if( rURL.GetMap() )
                pImageMap = new ImageMap( *rURL.GetMap() );
            else if( rURL.GetURL().Len() )
                pTargetURL = new INetImage( bLink ? sGrfName : aEmptyStr,
                                            rURL.GetURL(),
                                            rURL.GetTargetFrameName(),
                                            aEmptyStr, Size() );
        }
    }

    sal_Bool bEnableGraphicToGallery;
    if ( sal_True == (bEnableGraphicToGallery = bLink) )
    {
        GalleryExplorer::FillThemeList( aThemeList );
        if ( aThemeList.Count() )
        {
            PopupMenu *pMenu = GetPopupMenu(MN_READONLY_GRAPHICTOGALLERY);
            pMenu->CheckItem( MN_READONLY_TOGALLERYLINK,  bGrfToGalleryAsLnk );
            pMenu->CheckItem( MN_READONLY_TOGALLERYCOPY, !bGrfToGalleryAsLnk );
            for ( sal_uInt16 i=0; i < aThemeList.Count(); ++i )
                pMenu->InsertItem( MN_READONLY_GRAPHICTOGALLERY+i + 3,
                                   *(String*)aThemeList.GetObject( i ) );
        }
        else
            bEnableGraphicToGallery = sal_False;
    }
    EnableItem( MN_READONLY_GRAPHICTOGALLERY, bEnableGraphicToGallery );

    SfxViewFrame * pVFrame = rV.GetViewFrame();
    SfxDispatcher &rDis = *pVFrame->GetDispatcher();
    const SwPageDesc &rDesc = rSh.GetPageDesc( rSh.GetCurPageDesc() );
    pItem = &rDesc.GetMaster().GetBackground();
    sal_Bool bEnableBackGallery = sal_False,
         bEnableBack = sal_False;

    if ( GPOS_NONE != pItem->GetGraphicPos() )
    {
        bEnableBack = sal_True;
        if ( pItem->GetGraphicLink() )
        {
            if ( !aThemeList.Count() )
                GalleryExplorer::FillThemeList( aThemeList );
            if ( aThemeList.Count() )
            {
                PopupMenu *pMenu = GetPopupMenu(MN_READONLY_BACKGROUNDTOGALLERY);
                pMenu->CheckItem( MN_READONLY_TOGALLERYLINK,  bGrfToGalleryAsLnk );
                pMenu->CheckItem( MN_READONLY_TOGALLERYCOPY, !bGrfToGalleryAsLnk );
                bEnableBackGallery = sal_True;
                for ( sal_uInt16 i=0; i < aThemeList.Count(); ++i )
                    pMenu->InsertItem( MN_READONLY_BACKGROUNDTOGALLERY+i + 3,
                                       *(String*)aThemeList.GetObject( i ) );
            }
        }
    }
    EnableItem( MN_READONLY_SAVEBACKGROUND, bEnableBack );
    EnableItem( MN_READONLY_BACKGROUNDTOGALLERY, bEnableBackGallery );

    if ( !rSh.GetViewOptions()->IsGraphic() )
        CheckItem( MN_READONLY_GRAPHICOFF );
    else
        EnableItem( MN_READONLY_LOADGRAPHIC, sal_False );

    sal_Bool bReloadFrame = 0 != rSh.GetView().GetViewFrame()->GetFrame().GetParentFrame();
    EnableItem( MN_READONLY_RELOAD_FRAME,
            bReloadFrame );
    EnableItem( MN_READONLY_RELOAD, !bReloadFrame);

    Check( MN_READONLY_EDITDOC,         SID_EDITDOC,        rDis );
    Check( MN_READONLY_SELECTION_MODE,  FN_READONLY_SELECTION_MODE,    rDis );
    Check( MN_READONLY_SOURCEVIEW,      SID_SOURCEVIEW,     rDis );
    Check( MN_READONLY_BROWSE_BACKWARD, SID_BROWSE_BACKWARD,rDis );
    Check( MN_READONLY_BROWSE_FORWARD,  SID_BROWSE_FORWARD, rDis );
#ifdef WNT
    Check( MN_READONLY_PLUGINOFF,       SID_PLUGINS_ACTIVE, rDis );
#endif
    Check( MN_READONLY_OPENURL,         SID_OPENDOC,        rDis );
    Check( MN_READONLY_OPENURLNEW,      SID_OPENDOC,        rDis );

    SfxPoolItem* pState;

    SfxItemState eState = pVFrame->GetBindings().QueryState( SID_COPY, pState );
    Check( MN_READONLY_COPY,            SID_COPY,           rDis );
    if(eState < SFX_ITEM_AVAILABLE)
        EnableItem( MN_READONLY_COPY, sal_False );

    eState = pVFrame->GetBindings().QueryState( SID_EDITDOC, pState );
    if (
        eState < SFX_ITEM_DEFAULT ||
        (rSh.IsGlobalDoc() && rView.GetDocShell()->IsReadOnlyUI())
       )
    {
        EnableItem( MN_READONLY_EDITDOC, sal_False );
    }

    if ( !sURL.Len() )
    {
        EnableItem( MN_READONLY_OPENURL, sal_False );
        EnableItem( MN_READONLY_OPENURLNEW, sal_False );
        EnableItem( MN_READONLY_COPYLINK, sal_False );
    }
    Check( SID_WIN_FULLSCREEN,         SID_WIN_FULLSCREEN,        rDis );

    RemoveDisabledEntries( sal_True, sal_True );
}

void SwReadOnlyPopup::Execute( Window* pWin, const Point &rPixPos )
{
    sal_uInt16 nId     = PopupMenu::Execute(
    pWin,
    rPixPos );
    Execute(pWin, nId);
}

/*-- 17.03.2004 13:06:18---------------------------------------------------
    execute the resulting ID only - necessary to support XContextMenuInterception
  -----------------------------------------------------------------------*/
void SwReadOnlyPopup::Execute( Window* pWin, sal_uInt16 nId )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SfxDispatcher &rDis = *rView.GetViewFrame()->GetDispatcher();
    if ( nId >= MN_READONLY_GRAPHICTOGALLERY )
    {
        String sTmp;
        sal_uInt16 nSaveId;
        if ( nId >= MN_READONLY_BACKGROUNDTOGALLERY )
        {
            nId -= MN_READONLY_BACKGROUNDTOGALLERY+3;
            nSaveId = MN_READONLY_SAVEBACKGROUND;
            sTmp = *pItem->GetGraphicLink();
        }
        else
        {
            nId -= MN_READONLY_GRAPHICTOGALLERY+3;
            nSaveId = MN_READONLY_SAVEGRAPHIC;
            sTmp = sGrfName;
        }
        if ( !bGrfToGalleryAsLnk )
            sTmp = SaveGraphic( nSaveId );

        if ( sTmp.Len() )
        {
            String sThemeName( *(String*)aThemeList.GetObject( nId ));
            GalleryExplorer::InsertURL( sThemeName, sTmp );
        }
        return;
    }

    TransferDataContainer* pClipCntnr = 0;

    sal_uInt16 nExecId = USHRT_MAX;
    sal_uInt16 nFilter = USHRT_MAX;
    switch( nId )
    {
        case SID_WIN_FULLSCREEN :           nExecId = SID_WIN_FULLSCREEN; break;
        case MN_READONLY_OPENURL:           nFilter = URLLOAD_NOFILTER;   break;
        case MN_READONLY_OPENURLNEW:        nFilter = URLLOAD_NEWVIEW;    break;
        case MN_READONLY_COPY:              nExecId = SID_COPY;           break;

        case MN_READONLY_EDITDOC:           nExecId = SID_EDITDOC;        break;
        case MN_READONLY_SELECTION_MODE:    nExecId = FN_READONLY_SELECTION_MODE; break;
        case MN_READONLY_RELOAD:
        case MN_READONLY_RELOAD_FRAME:
            rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(SID_RELOAD);
        break;

        case MN_READONLY_BROWSE_BACKWARD:   nExecId = SID_BROWSE_BACKWARD;break;
        case MN_READONLY_BROWSE_FORWARD:    nExecId = SID_BROWSE_FORWARD; break;
        case MN_READONLY_SOURCEVIEW:        nExecId = SID_SOURCEVIEW;     break;
        case MN_READONLY_SAVEGRAPHIC:
        case MN_READONLY_SAVEBACKGROUND:
            {
                SaveGraphic( nId );
                break;
            }
        case MN_READONLY_COPYLINK:
            pClipCntnr = new TransferDataContainer;
            pClipCntnr->CopyString( sURL );
            break;

        case MN_READONLY_COPYGRAPHIC:
            pClipCntnr = new TransferDataContainer;
            pClipCntnr->CopyGraphic( aGraphic );

            if( pImageMap )
                pClipCntnr->CopyImageMap( *pImageMap );
            if( pTargetURL )
                pClipCntnr->CopyINetImage( *pTargetURL );
            break;

        case MN_READONLY_LOADGRAPHIC:
            {
                sal_Bool bModified = rSh.IsModified();
                SwViewOption aOpt( *rSh.GetViewOptions() );
                aOpt.SetGraphic( sal_True );
                rSh.ApplyViewOptions( aOpt );
                if(!bModified)
                    rSh.ResetModified();
                break;
            }
        case MN_READONLY_GRAPHICOFF:        nExecId = FN_VIEW_GRAPHIC;    break;
#ifdef WNT
        case MN_READONLY_PLUGINOFF:         nExecId = SID_PLUGINS_ACTIVE; break;
#endif
        case MN_READONLY_TOGALLERYLINK:
            SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk( sal_True );
            break;
        case MN_READONLY_TOGALLERYCOPY:
            SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk( sal_False );
            break;

        default: //forward the id to the SfxBindings
            nExecId = nId;
    }
    if( USHRT_MAX != nExecId )
        rDis.GetBindings()->Execute( nExecId );
    if( USHRT_MAX != nFilter )
        ::LoadURL( sURL, &rSh, nFilter, &sTargetFrameName);

    if( pClipCntnr )
    {
        STAR_REFERENCE( datatransfer::XTransferable ) xRef( pClipCntnr );
        if( pClipCntnr->HasAnyData() )
            pClipCntnr->CopyToClipboard( pWin );
    }
}
static void lcl_GetPreferedExtension( String &rExt, const Graphic &rGrf )
{
    // dann ggfs. ueber die native-Info der Grafik den "besten"
    // Filter vorschlagen
    const sal_Char* pExt = "png";
    switch( const_cast<Graphic&>(rGrf).GetLink().GetType() )
    {
        case GFX_LINK_TYPE_NATIVE_GIF:      pExt = "gif"; break;
        case GFX_LINK_TYPE_NATIVE_TIF:      pExt = "tif"; break;
        case GFX_LINK_TYPE_NATIVE_WMF:      pExt = "wmf"; break;
        case GFX_LINK_TYPE_NATIVE_MET:      pExt = "met"; break;
        case GFX_LINK_TYPE_NATIVE_PCT:      pExt = "pct"; break;
        case GFX_LINK_TYPE_NATIVE_JPG:      pExt = "jpg"; break;
        default:; //prevent warning
    }
    rExt.AssignAscii( pExt );
}


String SwReadOnlyPopup::SaveGraphic( sal_uInt16 nId )
{

    //Namen der Grafik herausfischen.
    String aName;
    if ( MN_READONLY_SAVEBACKGROUND == nId )
    {
        if ( pItem->GetGraphicLink() )
            sGrfName = *pItem->GetGraphicLink();
        ((SvxBrushItem*)pItem)->SetDoneLink( Link() );
        const Graphic *pGrf = pItem->GetGraphic();
        if ( pGrf )
        {
            aGraphic = *pGrf;
            if ( pItem->GetGraphicLink() )
                sGrfName = *pItem->GetGraphicLink();
        }
        else
            return aEmptyStr;
    }
    return ExportGraphic( aGraphic, sGrfName );
}

String ExportGraphic( const Graphic &rGraphic, const String &rGrfName )
{
    SvtPathOptions aPathOpt;
    String sGrfPath( aPathOpt.GetGraphicPath() );

    FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_SIMPLE, 0 );
    Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

//    aExpDlg.SetHelpId(HID_FILEDLG_ROMENU);
    INetURLObject aPath;
    aPath.SetSmartURL( sGrfPath );

    //Namen der Grafik herausfischen.
    String aName = rGrfName;

    aDlgHelper.SetTitle( SW_RESSTR(STR_EXPORT_GRAFIK_TITLE));
    aDlgHelper.SetDisplayDirectory( aPath.GetMainURL(INetURLObject::DECODE_TO_IURI) );
    INetURLObject aURL;
    aURL.SetSmartURL( aName );
    aDlgHelper.SetFileName( aURL.GetName() );

    GraphicFilter& rGF = *GraphicFilter::GetGraphicFilter();
    const sal_uInt16 nCount = rGF.GetExportFormatCount();

    String aExt( aURL.GetExtension() );
    if( !aExt.Len() )
        lcl_GetPreferedExtension( aExt, rGraphic );

    aExt.ToLowerAscii();
    sal_uInt16 nDfltFilter = USHRT_MAX;

    Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        xFltMgr->appendFilter( rGF.GetExportFormatName( i ), rGF.GetExportWildcard( i ) );
        if ( COMPARE_EQUAL == aExt.CompareIgnoreCaseToAscii(rGF.GetExportFormatShortName( i ).ToLowerAscii() ))
            nDfltFilter = i;
    }
    if ( USHRT_MAX == nDfltFilter )
    {
        //"falsche" Extension?
        lcl_GetPreferedExtension( aExt, rGraphic );
        for ( sal_uInt16 i = 0; i < nCount; ++i )
            if ( aExt == rGF.GetExportFormatShortName( i ).ToLowerAscii() )
            {
                nDfltFilter =  i;
                break;
            }
    }

    if( USHRT_MAX != nDfltFilter )
    {
        xFltMgr->setCurrentFilter( rGF.GetExportFormatName( nDfltFilter ) ) ;

        if( aDlgHelper.Execute() == ERRCODE_NONE )
        {
            String sPath( xFP->getFiles().getConstArray()[0] );
            //verwendeten Pfad merken - bitte nicht wieder wegoptimieren!
            aPath.SetSmartURL( sPath);
            sGrfPath = aPath.GetPath();

            if( rGrfName.Len() &&
                 nDfltFilter == rGF.GetExportFormatNumber( xFltMgr->getCurrentFilter()))
            {
                //Versuchen die Originalgrafik zu speichern.
                SfxMedium aIn( rGrfName, STREAM_READ | STREAM_NOCREATE,
                                sal_True );
                if( aIn.GetInStream() && !aIn.GetInStream()->GetError() )
                {
                    SfxMedium aOut( sPath, STREAM_WRITE | STREAM_SHARE_DENYNONE,
                                            sal_False);
                    if( aOut.GetOutStream() && !aOut.GetOutStream()->GetError())
                    {
                        *aOut.GetOutStream() << *aIn.GetInStream();
                        if ( 0 == aIn.GetError() )
                        {
                            aOut.Close();
                            aOut.Commit();
                            if ( 0 == aOut.GetError() )
                                return sPath;
                        }
                    }
                }
            }

            sal_uInt16 nFilter;
            if ( xFltMgr->getCurrentFilter().getLength() && rGF.GetExportFormatCount() )
                nFilter = rGF.GetExportFormatNumber( xFltMgr->getCurrentFilter() );
            else
                nFilter = GRFILTER_FORMAT_DONTKNOW;
            String aFilter( rGF.GetExportFormatShortName( nFilter ) );
            XOutBitmap::WriteGraphic( rGraphic, sPath, aFilter,
                                        XOUTBMP_DONT_EXPAND_FILENAME );
            return sPath;
        }
    }
    return aEmptyStr;
}



