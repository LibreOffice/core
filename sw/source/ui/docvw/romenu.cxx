/*************************************************************************
 *
 *  $RCSfile: romenu.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:23:52 $
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

#include <hintids.hxx>


#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _INETIMG_HXX //autogen
#include <svtools/inetimg.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <svx/impgrf.hxx>
#endif
#ifndef _GALLERY_HXX_ //autogen
#include <svx/gallery.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif


#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _ROMENU_HXX
#include <romenu.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _DOCVW_HRC
#include <docvw.hrc>
#endif
#ifndef _DOCVW_HRC
#include <docvw.hrc>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
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



void SwReadOnlyPopup::Check( USHORT nMID, USHORT nSID, SfxDispatcher &rDis )
{
    SfxPoolItem *pItem = 0;
    SfxItemState eState = rDis.GetBindings()->QueryState( nSID, pItem );
    if (eState >= SFX_ITEM_AVAILABLE)
    {
        EnableItem( nMID, TRUE );
        if (pItem)
        {
            CheckItem ( nMID, !pItem->ISA(SfxVoidItem) &&
                            pItem->ISA(SfxBoolItem) &&
                            ((SfxBoolItem*)pItem)->GetValue());
            //remove full screen entry when not in full screen mode
            if(SID_WIN_FULLSCREEN == nSID && !IsItemChecked(SID_WIN_FULLSCREEN) )
                EnableItem(nMID, FALSE);
        }
    }
    else
        EnableItem( nMID, FALSE );

    delete pItem;
}


SwReadOnlyPopup::SwReadOnlyPopup( const Point &rDPos, SwView &rV ) :
    PopupMenu( SW_RES(MN_READONLY_POPUP) ),
    rDocPos( rDPos ),
    rView  ( rV ),
    pImageMap( 0 ),
    pTargetURL( 0 )
{
    bGrfToGalleryAsLnk = SW_MOD()->GetModuleConfig()->IsGrfToGalleryAsLnk();
    SwWrtShell &rSh = rView.GetWrtShell();
    rSh.IsURLGrfAtPos( rDocPos, &sURL, &sTargetFrameName, &sDescription );
    if ( !sURL.Len() )
    {
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
        if( rSh.GetContentAtPos( rDocPos, aCntntAtPos, FALSE))
        {
            SwFmtINetFmt &rIItem = *(SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr;
            sURL = rIItem.GetValue();
            sTargetFrameName = rIItem.GetTargetFrame();
            sDescription = aCntntAtPos.sStr;
        }
    }

    BOOL bLink = FALSE;
    const Graphic *pGrf;
    if ( 0 == (pGrf = rSh.GetGrfAtPos( rDocPos, sGrfName, bLink )) )
    {
        EnableItem( MN_READONLY_SAVEGRAPHIC, FALSE );
        EnableItem( MN_READONLY_COPYGRAPHIC, FALSE );
    }
    else
    {
        aGraphic = *pGrf;
        const SwFrmFmt* pGrfFmt = rSh.GetFmtFromObj( rDocPos );
        const SfxPoolItem* pItem;
        if( pGrfFmt && SFX_ITEM_SET == pGrfFmt->GetItemState(
            RES_URL, TRUE, &pItem ))
        {
            const SwFmtURL& rURL = *(SwFmtURL*)pItem;
            if( rURL.GetMap() )
                pImageMap = new ImageMap( *rURL.GetMap() );
            else if( rURL.GetURL().Len() )
                pTargetURL = new INetImage( bLink ? sGrfName : aEmptyStr,
                                            rURL.GetURL(),
                                            rURL.GetTargetFrameName(),
                                            aEmptyStr, Size() );
        }
    }

    BOOL bEnableGraphicToGallery;
    if ( TRUE == (bEnableGraphicToGallery = bLink) )
    {
        GalleryExplorer::FillThemeList( aThemeList );
        if ( aThemeList.Count() )
        {
            PopupMenu *pMenu = GetPopupMenu(MN_READONLY_GRAPHICTOGALLERY);
            pMenu->CheckItem( MN_READONLY_TOGALLERYLINK,  bGrfToGalleryAsLnk );
            pMenu->CheckItem( MN_READONLY_TOGALLERYCOPY, !bGrfToGalleryAsLnk );
            for ( USHORT i=0; i < aThemeList.Count(); ++i )
                pMenu->InsertItem( MN_READONLY_GRAPHICTOGALLERY+i + 3,
                                   *(String*)aThemeList.GetObject( i ) );
        }
        else
            bEnableGraphicToGallery = FALSE;
    }
    EnableItem( MN_READONLY_GRAPHICTOGALLERY, bEnableGraphicToGallery );

    SfxViewFrame * pVFrame = rV.GetViewFrame();
    SfxDispatcher &rDis = *pVFrame->GetDispatcher();
    const SwPageDesc &rDesc = rSh.GetPageDesc( rSh.GetCurPageDesc() );
    pItem = &rDesc.GetMaster().GetBackground();
    BOOL bEnableBackGallery = FALSE,
         bEnableBack = FALSE;

    if ( GPOS_NONE != pItem->GetGraphicPos() )
    {
        bEnableBack = TRUE;
        if ( pItem->GetGraphicLink() )
        {
            if ( !aThemeList.Count() )
                GalleryExplorer::FillThemeList( aThemeList );
            if ( aThemeList.Count() )
            {
                PopupMenu *pMenu = GetPopupMenu(MN_READONLY_BACKGROUNDTOGALLERY);
                pMenu->CheckItem( MN_READONLY_TOGALLERYLINK,  bGrfToGalleryAsLnk );
                pMenu->CheckItem( MN_READONLY_TOGALLERYCOPY, !bGrfToGalleryAsLnk );
                bEnableBackGallery = TRUE;
                for ( USHORT i=0; i < aThemeList.Count(); ++i )
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
        EnableItem( MN_READONLY_LOADGRAPHIC, FALSE );

    BOOL bReloadFrame = 0 != rSh.GetView().GetViewFrame()->GetFrame()->GetParentFrame();
    EnableItem( MN_READONLY_RELOAD_FRAME,
            bReloadFrame );
    EnableItem( MN_READONLY_RELOAD, !bReloadFrame);

    Check( MN_READONLY_EDITDOC,         SID_EDITDOC,        rDis );
    Check( MN_READONLY_SELECTION_MODE,  FN_READONLY_SELECTION_MODE,    rDis );
    Check( MN_READONLY_SOURCEVIEW,      SID_SOURCEVIEW,     rDis );
    Check( MN_READONLY_BROWSE_STOP,     SID_BROWSE_STOP,    rDis );
    Check( MN_READONLY_BROWSE_BACKWARD, SID_BROWSE_BACKWARD,rDis );
    Check( MN_READONLY_BROWSE_FORWARD,  SID_BROWSE_FORWARD, rDis );
#ifdef WNT
    Check( MN_READONLY_PLUGINOFF,       SID_PLUGINS_ACTIVE, rDis );
#endif
    Check( MN_READONLY_OPENURL,         SID_OPENDOC,        rDis );
    Check( MN_READONLY_OPENURLNEW,      SID_OPENDOC,        rDis );

    SfxPoolItem* pState;
    SfxItemState eState = pVFrame->GetBindings().QueryState( SID_EDITDOC, pState );
    if(eState < SFX_ITEM_DEFAULT ||
        rSh.IsGlobalDoc() && rView.GetDocShell()->IsReadOnlyUI())
        EnableItem( MN_READONLY_EDITDOC, FALSE );

    if ( !sURL.Len() )
    {
        EnableItem( MN_READONLY_OPENURL, FALSE );
        EnableItem( MN_READONLY_OPENURLNEW, FALSE );
        EnableItem( MN_READONLY_COPYLINK, FALSE );
    }
    Check( SID_WIN_FULLSCREEN,         SID_WIN_FULLSCREEN,        rDis );

    RemoveDisabledEntries( TRUE, TRUE );
}

void SwReadOnlyPopup::Execute( Window* pWin, const Point &rPixPos )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SfxDispatcher &rDis = *rView.GetViewFrame()->GetDispatcher();
    USHORT nId     = PopupMenu::Execute(
    pWin,
    rPixPos );

    if ( nId >= MN_READONLY_GRAPHICTOGALLERY )
    {
        String sTmp;
        USHORT nSaveId;
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

    USHORT nExecId = USHRT_MAX;
    USHORT nFilter = USHRT_MAX;
    switch( nId )
    {
        case SID_WIN_FULLSCREEN :           nExecId = SID_WIN_FULLSCREEN; break;
        case MN_READONLY_OPENURL:           nFilter = URLLOAD_NOFILTER;   break;
        case MN_READONLY_OPENURLNEW:        nFilter = URLLOAD_NEWVIEW;    break;
        case MN_READONLY_EDITDOC:           nExecId = SID_EDITDOC;        break;
        case MN_READONLY_SELECTION_MODE:    nExecId = FN_READONLY_SELECTION_MODE; break;
        case MN_READONLY_RELOAD:
        case MN_READONLY_RELOAD_FRAME:
            rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(SID_RELOAD);
        break;

        case MN_READONLY_BROWSE_STOP:       nExecId = SID_BROWSE_STOP;    break;
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
                BOOL bModified = rSh.IsModified();
                SwViewOption aOpt( *rSh.GetViewOptions() );
                aOpt.SetGraphic( TRUE );
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
            SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk( TRUE );
            break;
        case MN_READONLY_TOGALLERYCOPY:
            SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk( FALSE );
            break;

        default: /* do nothing */;
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
                                             //nicht const, weil GetLink() noch
                                             //nicht const. kann bei naechster
                                             //Aenderung dieses Files mit
                                             //erledigt werden.
void lcl_GetPreferedExtension( String &rExt, /*const*/ Graphic &rGrf )
{
    // dann ggfs. ueber die native-Info der Grafik den "besten"
    // Filter vorschlagen
    const sal_Char* pExt = "png";
    switch( rGrf.GetLink().GetType() )
    {
        case GFX_LINK_TYPE_NATIVE_GIF:      pExt = "gif"; break;
        case GFX_LINK_TYPE_NATIVE_TIF:      pExt = "tif"; break;
        case GFX_LINK_TYPE_NATIVE_WMF:      pExt = "wmf"; break;
        case GFX_LINK_TYPE_NATIVE_MET:      pExt = "met"; break;
        case GFX_LINK_TYPE_NATIVE_PCT:      pExt = "pct"; break;
        case GFX_LINK_TYPE_NATIVE_JPG:      pExt = "jpg"; break;
    }
    rExt.AssignAscii( pExt );
}


String SwReadOnlyPopup::SaveGraphic( USHORT nId )
{
    SvtPathOptions aPathOpt;
    String sGrfPath( aPathOpt.GetGraphicPath() );
    SwWrtShell &rSh = rView.GetWrtShell();

    FileDialogHelper aDlgHelper( TemplateDescription::FILESAVE_SIMPLE, 0 );
    Reference < XFilePicker > xFP = aDlgHelper.GetFilePicker();

//    aExpDlg.SetHelpId(HID_FILEDLG_ROMENU);
    INetURLObject aPath;
    aPath.SetSmartURL( sGrfPath);

    //Namen der Grafik herausfischen.
    String aName;
    if ( MN_READONLY_SAVEBACKGROUND == nId )
    {
        if ( pItem->GetGraphicLink() )
            aName = *pItem->GetGraphicLink();
        ((SvxBrushItem*)pItem)->SetDoneLink( Link() );
        const Graphic *pGrf = pItem->GetGraphic( rView.GetDocShell() );
        if ( pGrf )
        {
            aGraphic = *pGrf;
            if ( pItem->GetGraphicLink() )
                sGrfName = *pItem->GetGraphicLink();
        }
        else
            return aEmptyStr;
    }
    else
        aName = sGrfName;
    INetURLObject aURL;
    aURL.SetSmartURL( aName );
    aPath.Append( aURL.GetName() );
    xFP->setDisplayDirectory( aPath.GetMainURL(INetURLObject::DECODE_TO_IURI) );
    xFP->setTitle( SW_RESSTR(STR_EXPORT_GRAFIK_TITLE));

    GraphicFilter& rGF = *GetGrfFilter();
    const USHORT nCount = rGF.GetExportFormatCount();

    String aExt( aURL.GetExtension() );
    if( !aExt.Len() )
        lcl_GetPreferedExtension( aExt, aGraphic );

    aExt.ToLowerAscii();
    int nDfltFilter = INT_MAX;

    Reference<XFilterManager> xFltMgr(xFP, UNO_QUERY);

    for ( int i = 0; i < nCount; i++ )
    {
        xFltMgr->appendFilter( rGF.GetExportFormatName( i ), rGF.GetExportWildcard( i ) );
        if ( COMPARE_EQUAL == aExt.CompareIgnoreCaseToAscii(rGF.GetExportFormatShortName( i ).ToLowerAscii() ))
            nDfltFilter = i;
    }
    if ( INT_MAX == nDfltFilter )
    {
        //"falsche" Extension?
        lcl_GetPreferedExtension( aExt, aGraphic );
        for ( int i = 0; i < nCount; ++i )
            if ( aExt == rGF.GetExportFormatShortName( i ).ToLowerAscii() )
            {
                nDfltFilter =  i;
                break;
            }
    }

    if( INT_MAX != nDfltFilter )
    {
        xFltMgr->setCurrentFilter( rGF.GetExportFormatName( nDfltFilter ) ) ;

        if( aDlgHelper.Execute() == ERRCODE_NONE )
        {
            String sPath( xFP->getFiles().getConstArray()[0] );
            //verwendeten Pfad merken - bitte nicht wieder wegoptimieren!
            aPath.SetSmartURL( sPath);
            sGrfPath = aPath.GetPath();

            if( sGrfName.Len() &&
                 nDfltFilter == rGF.GetExportFormatNumber( xFltMgr->getCurrentFilter()))
            {
                //Versuchen die Originalgrafik zu speichern.
                SfxMedium aIn( sGrfName, STREAM_READ | STREAM_NOCREATE,
                                TRUE );
                if( aIn.GetInStream() && !aIn.GetInStream()->GetError() )
                {
                    SfxMedium aOut( sPath, STREAM_WRITE | STREAM_SHARE_DENYNONE,
                                            FALSE);
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

            int nFilter;
            if ( xFltMgr->getCurrentFilter().getLength() && rGF.GetExportFormatCount() )
                nFilter = rGF.GetExportFormatNumber( xFltMgr->getCurrentFilter() );
            else
                nFilter = GRFILTER_FORMAT_DONTKNOW;
            String aFilter( rGF.GetExportFormatShortName( nFilter ) );
            XOutBitmap::WriteGraphic( aGraphic, sPath, aFilter,
                                        XOUTBMP_DONT_EXPAND_FILENAME );
            return sPath;
        }
    }
    return aEmptyStr;
}



