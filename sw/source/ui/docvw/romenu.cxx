/*************************************************************************
 *
 *  $RCSfile: romenu.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-06 13:33:32 $
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
#ifndef _CLIP_HXX //autogen
#include <vcl/clip.hxx>
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
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _IODLG_HXX
#include <sfx2/iodlg.hxx>
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
    const SfxPoolItem *pTmp;
    EnableItem( MN_READONLY_CREATEGRAPHIKLINK, bLink &&
                SFX_ITEM_AVAILABLE == rDis.QueryState( SID_CREATELINK, pTmp ));

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
    Check( MN_READONLY_SOURCEVIEW,      SID_SOURCEVIEW,     rDis );
    Check( MN_READONLY_BROWSE_STOP,     SID_BROWSE_STOP,    rDis );
    Check( MN_READONLY_BROWSE_BACKWARD, SID_BROWSE_BACKWARD,rDis );
    Check( MN_READONLY_BROWSE_FORWARD,  SID_BROWSE_FORWARD, rDis );
    Check( MN_READONLY_CREATELINK,      SID_CREATELINK,     rDis );
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
        EnableItem( MN_READONLY_CREATELINK, FALSE );
        EnableItem( MN_READONLY_DOWNLOAD, FALSE );
        EnableItem( MN_READONLY_COPYLINK, FALSE );
    }

    RemoveDisabledEntries( TRUE, TRUE );

#if 0
    //Jetzt noch das unnuetze Geraffel entfernen.
    MenuItemType eLast = MENUITEM_DONTKNOW;
    for ( int i = 0; i < GetItemCount(); ++i )
    {
        if ( MENUITEM_SEPARATOR == GetItemType( i ) )
        {
            if ( i == 0 || eLast == MENUITEM_SEPARATOR )
                RemoveItem( i-- );
        }
        else if ( !IsItemEnabled( GetItemId( i ) ) )
            RemoveItem( i-- );
        eLast = i >= 0 ? GetItemType(i) : MENUITEM_DONTKNOW;
    }
#endif
}

struct _CastINetImage : INetImage
{
    void Copy()
        {
            String sData( CopyExchange() );
            if( sData.Len() )
                Clipboard::CopyData( sData.GetBuffer(), sData.Len() / 2,
                                        SOT_FORMATSTR_ID_INET_IMAGE );
        }
};

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

    USHORT nExecId = USHRT_MAX;
    USHORT nFilter = USHRT_MAX;
    switch( nId )
    {
        case MN_READONLY_OPENURL:           nFilter = URLLOAD_NOFILTER;   break;
        case MN_READONLY_OPENURLNEW:        nFilter = URLLOAD_NEWVIEW;    break;
        case MN_READONLY_EDITDOC:           nExecId = SID_EDITDOC;        break;
        case MN_READONLY_RELOAD:
        case MN_READONLY_RELOAD_FRAME:
            rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(SID_RELOAD);
        break;

        case MN_READONLY_BROWSE_STOP:       nExecId = SID_BROWSE_STOP;    break;
        case MN_READONLY_BROWSE_BACKWARD:   nExecId = SID_BROWSE_BACKWARD;break;
        case MN_READONLY_BROWSE_FORWARD:    nExecId = SID_BROWSE_FORWARD; break;
        case MN_READONLY_SOURCEVIEW:        nExecId = SID_SOURCEVIEW;     break;
        case MN_READONLY_CREATELINK:
            {
                SfxStringItem aName( SID_BOOKMARK_TITLE, sDescription );
                SfxStringItem aURL( SID_BOOKMARK_URL, sURL );
                rDis.Execute( SID_CREATELINK, SFX_CALLMODE_ASYNCHRON,
                              &aName, &aURL, 0L);
            }
            break;
        case MN_READONLY_CREATEGRAPHIKLINK:
            {
                SfxStringItem aURL ( SID_BOOKMARK_URL, sGrfName );
                INetURLObject aTmp;
                aTmp.SetSmartURL( sGrfName );
                SfxStringItem aName( SID_BOOKMARK_TITLE, aTmp.GetName() );
                rDis.Execute( SID_CREATELINK, SFX_CALLMODE_ASYNCHRON,
                              &aName, &aURL, 0L);
            }
        case MN_READONLY_DOWNLOAD:          nFilter =
                                                URLLOAD_NEWVIEW |
                                                URLLOAD_DOWNLOADFILTER;   break;
        case MN_READONLY_SAVEGRAPHIC:
        case MN_READONLY_SAVEBACKGROUND:
            {
                SaveGraphic( nId );
                break;
            }
        case MN_READONLY_COPYLINK:
            Clipboard::Clear();
            Clipboard::CopyString( sURL );
            break;

        case MN_READONLY_COPYGRAPHIC:
            Clipboard::Clear();
            aGraphic.Copy();
            if( pImageMap )     pImageMap->Copy();
            if( pTargetURL )    ((_CastINetImage*)pTargetURL)->Copy();
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
    if ( nExecId != USHRT_MAX )
        rDis.GetBindings()->Execute( nExecId );
    if ( nFilter != USHRT_MAX )
        ::LoadURL( sURL, &rSh, nFilter, &sTargetFrameName);
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
    SfxFileDialog aExpDlg( NULL, WinBits(WB_SAVEAS|WB_3DLOOK) );
    aExpDlg.DisableSaveLastDirectory();
    aExpDlg.SetHelpId(HID_FILEDLG_ROMENU);
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
    aExpDlg.SetPath( aPath.GetMainURL() );
    aExpDlg.SetText( SW_RESSTR(STR_EXPORT_GRAFIK_TITLE));

    GraphicFilter& rGF = *GetGrfFilter();
    const USHORT nCount = rGF.GetExportFormatCount();

    String aExt( aURL.GetExtension() );
    if( !aExt.Len() )
        lcl_GetPreferedExtension( aExt, aGraphic );

    aExpDlg.SetDefaultExt( aExt );
    aExt.ToLowerAscii();
    int nDfltFilter = INT_MAX;

    for ( int i = 0; i < nCount; i++ )
    {
        aExpDlg.AddFilter( rGF.GetExportFormatName( i ),
                            rGF.GetExportWildcard( i ),
                            rGF.GetExportOSFileType( i ) );
        if ( COMPARE_EQUAL == aExt.CompareIgnoreCaseToAscii(rGF.GetExportFormatType( i ).ToLowerAscii() ))
            nDfltFilter = i;
    }
    if ( INT_MAX == nDfltFilter )
    {
        //"falsche" Extension?
        lcl_GetPreferedExtension( aExt, aGraphic );
        for ( int i = 0; i < nCount; ++i )
            if ( aExt == rGF.GetExportFormatType( i ).ToLowerAscii() )
            {
                nDfltFilter =  i;
                break;
            }
    }

    if( INT_MAX != nDfltFilter )
    {
        aExpDlg.SetCurFilter( rGF.GetExportFormatName( nDfltFilter ) );

        if( RET_OK == aExpDlg.Execute() )
        {
            String sPath( aExpDlg.GetPath() );
            //verwendeten Pfad merken - bitte nicht wieder wegoptimieren!
            aPath.SetSmartURL( sPath);
            sGrfPath = aPath.GetPath();

            if( sGrfName.Len() &&
                 nDfltFilter == rGF.GetExportFormatNumber( aExpDlg.GetCurFilter()))
            {
                //Versuchen die Originalgrafik zu speichern.
                SfxMedium aIn( sGrfName, STREAM_READ | STREAM_NOCREATE,
                                TRUE, TRUE );
                if( aIn.GetInStream() && !aIn.GetInStream()->GetError() )
                {
                    SfxMedium aOut( sPath, STREAM_WRITE | STREAM_SHARE_DENYNONE,
                                            FALSE, FALSE );
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
            if ( aExpDlg.GetCurFilter().Len() && rGF.GetExportFormatCount() )
                nFilter = rGF.GetExportFormatNumber( aExpDlg.GetCurFilter() );
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


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:35  hr
      initial import

      Revision 1.56  2000/09/18 16:05:24  willem.vandorp
      OpenOffice header added.

      Revision 1.55  2000/09/07 15:59:22  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.54  2000/04/19 12:56:34  os
      include sfx2/filedlg.hxx removed

      Revision 1.53  2000/04/18 15:18:17  os
      UNICODE

      Revision 1.52  2000/03/03 15:17:00  os
      StarView remainders removed

      Revision 1.51  2000/02/11 14:45:03  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.50  2000/01/24 12:49:10  os
      #72153# call SfxFileDialog::DisableSaveLastDirectory

      Revision 1.49  1999/03/08 10:51:04  JP
      Bug #62925#: ImageMap/INetImage mit ins Clipboard stellen


      Rev 1.48   08 Mar 1999 11:51:04   JP
   Bug #62925#: ImageMap/INetImage mit ins Clipboard stellen

      Rev 1.47   25 Nov 1998 13:56:30   OS
   #59809# Grafik-Pfad wieder merken

      Rev 1.46   26 Oct 1998 16:16:06   OS
   #58159# Reload und ReloadFrame im ReadOnlyPopup

      Rev 1.45   21 Oct 1998 13:23:26   OM
   #58267# Slotstatus ueber Bindings abfragen

      Rev 1.44   02 Sep 1998 14:11:48   OM
   #45378# HelpIDs fuer Dateidialoge

      Rev 1.43   12 Aug 1998 09:41:54   MA
   #54165# Phantasie-Extensions verarbeiten

      Rev 1.42   11 Aug 1998 14:15:24   JP
   Bug #54446#: bei embeddeten Grafiken, das default Format aus der Grafik besorgen

      Rev 1.41   05 Aug 1998 12:42:10   JP
   Bug #54446#: bei embeddeten Grafiken keine leeren Dateien erzeugen

      Rev 1.40   30 Jul 1998 22:15:16   JP
   Bug #54446# auch nicht gelinkte Grafiken speichern

      Rev 1.39   30 Jun 1998 17:47:44   MA
   RemoveDisabledEntries genutzt

      Rev 1.38   07 May 1998 17:58:42   MA
   PlugIn fuer WNT wieder aktiv

      Rev 1.37   29 Apr 1998 18:30:26   MA
   #49873# so macht man das also

      Rev 1.36   28 Apr 1998 15:07:54   MA
   chg: PathToFileName statt GetFull am URL-Objekt

      Rev 1.35   16 Apr 1998 08:15:10   OS
   nach Grafiken einschalten evtl. Modify-Flag zuruecksetzen#49488#

      Rev 1.34   25 Mar 1998 14:24:12   OS
   im ReadonlyUI-Zustand der Globaldokumente Bearbeiten nicht moeglich #48871#

      Rev 1.33   25 Nov 1997 10:32:58   MA
   includes

      Rev 1.32   16 Oct 1997 12:03:08   OS
   PlugIn nicht im VCL

      Rev 1.31   12 Sep 1997 10:38:44   OS
   ITEMID_* definiert

      Rev 1.30   02 Sep 1997 13:20:48   OS
   includes

      Rev 1.29   29 Aug 1997 16:03:32   OS
   PopupMenu::Execute mit Window* fuer VCL

      Rev 1.28   29 Aug 1997 14:35:36   OS
   DLL-Umbau

      Rev 1.27   15 Aug 1997 12:09:56   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.26   12 Aug 1997 15:59:04   OS
   frmitems/textitem/paraitem aufgeteilt

      Rev 1.25   12 Aug 1997 12:40:30   MA
   #41880# Original der Grafiken speichern

      Rev 1.24   08 Aug 1997 17:30:12   OM
   Headerfile-Umstellung

      Rev 1.23   07 Aug 1997 15:00:18   OM
   Headerfile-Umstellung

      Rev 1.22   06 Aug 1997 10:18:12   OS
   FN_SOURCEVIEW->SID_SOURCEVIEW

      Rev 1.21   27 May 1997 13:22:10   OS
   FN_SOURCEVIEW im Popup

      Rev 1.20   15 Apr 1997 14:14:24   OS
   Status fuer EDITDOC an den Bindings erfragen

      Rev 1.19   20 Mar 1997 16:59:42   MA
   fix: richtige Extension und damit Filtererkennung reanimiert

      Rev 1.18   18 Mar 1997 14:40:18   OS
   letzten Save-Pfad merken und am Dialog richtig einstellen

      Rev 1.17   23 Feb 1997 17:56:50   MA
   #36840# mit den Separatoren aufraeumen.

      Rev 1.16   14 Feb 1997 19:42:26   MA
   new: Flag fuer Readonly-Gallery

      Rev 1.15   12 Feb 1997 17:05:36   MA
   #36004# Background disablen

      Rev 1.14   10 Feb 1997 16:48:10   MA
   fix: BackgroundToGallery ggf. disablen

      Rev 1.13   06 Feb 1997 08:24:02   MA
   BackgroundToGallery, removen disableter Items

      Rev 1.12   04 Feb 1997 16:04:18   MA
   chg: Aufnehmen in Gallery

      Rev 1.11   02 Dec 1996 08:32:00   TRI
   SfxPoolItem const gemacht

      Rev 1.10   29 Nov 1996 10:20:36   MA
   fix: dontexpandfilename

      Rev 1.9   28 Nov 1996 18:15:00   HJS
   cast fuer os2

      Rev 1.8   28 Nov 1996 14:20:32   MA
   fix: Grafiken speichern, fehlt noch der TempName

      Rev 1.7   21 Nov 1996 20:07:08   MA
   chg: neue Eintraege

      Rev 1.6   18 Nov 1996 18:22:08   MA
   chg: SfxFileDialog statt FileDialog

      Rev 1.5   11 Nov 1996 09:39:32   MA
   ResMgr

      Rev 1.4   24 Oct 1996 15:37:18   MA
   chg: Filter per Extension detecten

      Rev 1.3   22 Oct 1996 12:27:50   MA
   new: Namen und Filter defaulten

      Rev 1.2   24 Sep 1996 16:45:16   OS
   OS/2-Absturz behoben; richtiges enablen

      Rev 1.1   18 Sep 1996 18:20:12   MA
   #31458# Dispatcher statt Bindings

      Rev 1.0   16 Sep 1996 14:56:38   MA
   new: Readonly-Popup

*************************************************************************/


