/*************************************************************************
 *
 *  $RCSfile: viewfun5.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-30 19:14:44 $
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

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
//#define _SHL_HXX
//#define _LINK_HXX
//#define _ERRCODE_HXX
//#define _GEN_HXX
//#define _FRACT_HXX
//#define _STRING_HXX
//#define _MTF_HXX
//#define _CONTNR_HXX
//#define _LIST_HXX
//#define _TABLE_HXX
#define _DYNARY_HXX
//#define _UNQIDX_HXX
#define _SVMEMPOOL_HXX
//#define _UNQID_HXX
//#define _DEBUG_HXX
//#define _DATE_HXX
//#define _TIME_HXX
//#define _DATETIME_HXX
//#define _INTN_HXX
//#define _WLDCRD_HXX
//#define _FSYS_HXX
//#define _STREAM_HXX
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX

//SV
//#define _CLIP_HXX ***
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
//#define _COLOR_HXX
//#define _PAL_HXX
//#define _BITMAP_HXX
//#define _GDIOBJ_HXX
//#define _POINTR_HXX
//#define _ICON_HXX
//#define _IMAGE_HXX
//#define _KEYCOD_HXX
//#define _EVENT_HXX
#define _HELP_HXX
//#define _APP_HXX
//#define _MDIAPP_HXX
//#define _TIMER_HXX
//#define _METRIC_HXX
//#define _REGION_HXX
//#define _OUTDEV_HXX
//#define _SYSTEM_HXX
//#define _VIRDEV_HXX
//#define _JOBSET_HXX
//#define _PRINT_HXX
//#define _WINDOW_HXX
//#define _SYSWIN_HXX
//#define _WRKWIN_HXX
#define _MDIWIN_HXX
//#define _FLOATWIN_HXX
//#define _DOCKWIN_HXX
//#define _CTRL_HXX
//#define _SCRBAR_HXX
//#define _BUTTON_HXX
//#define _IMAGEBTN_HXX
//#define _FIXED_HXX
//#define _GROUP_HXX
//#define _EDIT_HXX
//#define _COMBOBOX_HXX
//#define _LSTBOX_HXX
//#define _SELENG_HXX ***
//#define _SPLIT_HXX
#define _SPIN_HXX
//#define _FIELD_HXX
//#define _MOREBTN_HXX ***
//#define _TOOLBOX_HXX
//#define _STATUS_HXX ***
//#define _DIALOG_HXX
//#define _MSGBOX_HXX
//#define _SYSDLG_HXX
#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
#define _MENU_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX

#if defined  WIN
#define _MENUBTN_HXX
#endif

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
//#define _TABBAR_HXX
//#define _VALUESET_HXX
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX
#define _EXTATTR_HXX

//SVTOOLS
//#define _SVTREELIST_HXX ***
#define _FILTER_HXX
//#define _SVLBOXITM_HXX ***
//#define _SVTREEBOX_HXX ***
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
//#define _SFXINIMGR_HXX ***
//#define _SFXCFGITEM_HXX
//#define _SFX_PRINTER_HXX
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
//#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFXSH_HXX
///#define _PLUGAPPL_HXX
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
//#define _SFXDISPATCH_HXX
#define _SFXMSG_HXX
#define _SFXOBJFACE_HXX
#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX


// SFX
#define _SFX_SAVEOPT_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX
#define _SFX_CHILDALIGN_HXX
#define _SFXAPPWIN_HXX
#define _SFX_CHILDWIN_HXX
#define _SFXCTRLITEM_HXX


//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
//#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdoc.hxx
//#define _SFXDOC_HXX
// --- #define _RSCSFX_HXX
//#define SFX_DECL_OBJECTSHELL_DEFINED
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HACK_HXX
#define _SFX_FCONTNR_HXX
#define STRING_LIST
//#define _SFXDOCFILE_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXDLG_HXX
#define _DOCMGR_HXX
//#define _SFXITEMS_HXX
//#define _SFXTABDLG_HXX
#define _BASEDLGS_HXX
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define SFX_DECL_OBJECTSHELL_DEFINED
#define _SFXDOCTEMPL_HXX
#define _SFXDOCTDLG_HXX
#define _SFXDOCKWIN_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXNEW_HXX
#define _SFXDOCMAN_HXX
#define _SFXNAVI_HXX


//sfxitems.hxx
#define _SFX_WHMAP_HXX
//#define _ARGS_HXX
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
//#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
//#define _SFXITEMPOOL_HXX
//#define _SFXITEMSET_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
//#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
//#define _SFXSLSTITM_HXX
//#define _SFXSTYLE_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX
//#define _XOUTX_HXX
//#define _XPOOL_HXX
//#define _XTABLE_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
#define _SDR_NOOBJECTS //*
//#define _SDR_NOVIEWS

//#define SI_NOITEMS
//#define SI_NODRW
#define _SI_NOSBXCONTROLS
//#define _VCATTR_HXX
#define _VCONT_HXX
//#define _VCSBX_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
//#define _VCDRWOBJ_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
//#define _SIDLL_HXX ***

#ifdef WIN
#define _VCSBX_HXX
#define _VCDRWOBJ_HXX
#define _SBX_HXX
#endif

//view3d.hxx
//#define _E3D_VIEW3D_HXX

//obj3d.hxx
//#define _E3D_OBJ3D_HXX

//volume3d.hxx
//#define _VOLUME3D_HXX




#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX


//------------------------------------------------------------------

#include <svx/dbexch.hrc>
#include <svx/fmmodel.hxx>
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <so3/ipobj.hxx>
#include <so3/svstor.hxx>
#include <offmgr/app.hxx>
#include <so3/clsids.hxx>
#include <sot/formats.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/ptitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/transfer.hxx>
#include <vcl/drag.hxx>
#include <vcl/graph.hxx>

#include <sot/formats.hxx>
#define SOT_FORMATSTR_ID_STARCALC_CURRENT   SOT_FORMATSTR_ID_STARCALC_50

#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#ifndef SO2_DECL_SVSTORAGE_DEFINED
#define SO2_DECL_SVSTORAGE_DEFINED
SO2_DECL_REF(SvStorage)
#endif

#include "viewfunc.hxx"
#include "docsh.hxx"
#include "drawview.hxx"
#include "impex.hxx"
#include "dbfunc.hxx"
#include "dbcolect.hxx"
#include "sc.hrc"
#include "filter.hxx"
#include "scextopt.hxx"
#include "tabvwsh.hxx"      //  wegen GetViewFrame
#include "compiler.hxx"

#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

BOOL ScViewFunc::PasteDataFormat( ULONG nFormatId,
                    const uno::Reference<datatransfer::XTransferable>& rxTransferable,
                    USHORT nPosX, USHORT nPosY, Point* pLogicPos )
{
    Point aPos;                     //  inserting position (1/100 mm)
    if (pLogicPos)
        aPos = *pLogicPos;
    else
    {
        //  inserting position isn't needed for text formats
        BOOL bIsTextFormat = ( ScImportExport::IsFormatSupported( nFormatId ) ||
                                nFormatId == FORMAT_RTF );
        if ( !bIsTextFormat )
        {
            //  Window MapMode isn't drawing MapMode if DrawingLayer hasn't been created yet

            ScDocument* pDoc = GetViewData()->GetDocument();
            USHORT nTab = GetViewData()->GetTabNo();
            USHORT i;
            long nXT = 0;
            for (i=0; i<nPosX; i++)
                nXT += pDoc->GetColWidth(i,nTab);
            long nYT = 0;
            for (i=0; i<nPosY; i++)
                nYT += pDoc->FastGetRowHeight(i,nTab);
            aPos = Point( (long)(nXT * HMM_PER_TWIPS), (long)(nYT * HMM_PER_TWIPS) );
        }
    }

    TransferableDataHelper aDataHelper( rxTransferable );
    BOOL bRet = FALSE;

    //
    //  handle individual formats
    //

    if ( nFormatId == SOT_FORMATSTR_ID_EMBED_SOURCE ||
         nFormatId == SOT_FORMATSTR_ID_LINK_SOURCE ||
         nFormatId == SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ||
         nFormatId == SOT_FORMATSTR_ID_LINK_SOURCE_OLE )
    {
        SotStorageStreamRef             xStm;
        TransferableObjectDescriptor    aObjDesc;

        if( aDataHelper.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) &&
            aDataHelper.GetSotStorageStream( nFormatId, xStm ) )
        {
            SvStorageRef xStore( new SvStorage( *xStm ) );
            if ( aObjDesc.maClassName == SvGlobalName( SO3_SC_CLASSID_30 ) ||
                 aObjDesc.maClassName == SvGlobalName( SO3_SC_CLASSID_40 ) ||
                 aObjDesc.maClassName == SvGlobalName( SO3_SC_CLASSID_50 ) ||
                 aObjDesc.maClassName == SvGlobalName( SO3_SC_CLASSID_60 ) )
            {
                //  own format (including old formats)
                ScDocShellRef xDocShRef = new ScDocShell(SFX_CREATE_MODE_EMBEDDED);
                if (xDocShRef->DoLoad(xStore))
                {
                    ScDocument* pSrcDoc = xDocShRef->GetDocument();
                    USHORT nSrcTab = pSrcDoc->GetVisibleTab();
                    if (!pSrcDoc->HasTable(nSrcTab))
                        nSrcTab = 0;

                    ScMarkData aSrcMark;
                    aSrcMark.SelectOneTable( nSrcTab );         // for CopyToClip
                    ScDocument* pClipDoc = new ScDocument( SCDOCMODE_CLIP );

                    USHORT nFirstCol, nFirstRow, nLastCol, nLastRow;
                    if ( pSrcDoc->GetDataStart( nSrcTab, nFirstCol, nFirstRow ) )
                        pSrcDoc->GetCellArea( nSrcTab, nLastCol, nLastRow );
                    else
                        nFirstCol = nFirstRow = nLastCol = nLastRow = 0;
                    pSrcDoc->CopyToClip( nFirstCol, nFirstRow, nLastCol, nLastRow,
                                            FALSE, pClipDoc, FALSE, &aSrcMark );
                    ScGlobal::SetClipDocName( xDocShRef->GetTitle( SFX_TITLE_FULLNAME ) );

                    SetCursor( nPosX, nPosY );
                    Unmark();
                    PasteFromClip( IDF_ALL, pClipDoc );
                    delete pClipDoc;
                    bRet = TRUE;
                }
                ((SfxInPlaceObject*)xDocShRef)->DoClose();
                xDocShRef.Clear();
            }
            else
            {
                SvInPlaceObjectRef xIPObj = &( (SvFactory*) SvInPlaceObject::ClassFactory() )->CreateAndLoad( xStore );
                if ( xIPObj.Is() )
                {
                    PasteObject( aPos, xIPObj, &aObjDesc.maSize );
                    bRet = TRUE;
                }
                else
                    DBG_ERROR("Error in CreateAndLoad");
            }
        }
    }
    else if ( nFormatId == SOT_FORMATSTR_ID_LINK )      // LINK is also in ScImportExport
    {
        bRet = PasteDDE( rxTransferable );
    }
    else if ( ScImportExport::IsFormatSupported( nFormatId ) || nFormatId == SOT_FORMAT_RTF )
    {
        if ( nFormatId == SOT_FORMAT_RTF && aDataHelper.HasFormat( SOT_FORMATSTR_ID_EDITENGINE ) )
        {
            //  use EditView's PasteSpecial / Drop
            PasteRTF( nPosX, nPosY, rxTransferable );
            bRet = TRUE;
        }
        else
        {
            ScAddress aPos( nPosX, nPosY, GetViewData()->GetTabNo() );
            ScImportExport aObj( GetViewData()->GetDocument(), aPos );

            String aStr;
            SotStorageStreamRef xStream;
            if ( aDataHelper.GetSotStorageStream( nFormatId, xStream ) && xStream.Is() )
                bRet = aObj.ImportStream( *xStream, nFormatId );
            else if ( aDataHelper.GetString( nFormatId, aStr ) )
                bRet = aObj.ImportString( aStr, nFormatId );

            InvalidateAttribs();
            GetViewData()->UpdateInputHandler();
        }
    }
    else if (nFormatId == SOT_FORMATSTR_ID_SBA_DATAEXCHANGE)
    {
        //  import of database data into table

        String sDataDesc;
        if ( aDataHelper.GetString( nFormatId, sDataDesc ) )
        {
            SfxStringItem aDataDesc(SID_SBA_IMPORT, sDataDesc);

            ScDocShell* pDocSh = GetViewData()->GetDocShell();
            ScDocument* pDoc = pDocSh->GetDocument();
            USHORT nTab = GetViewData()->GetTabNo();

            ClickCursor(nPosX, nPosY, FALSE);               // set cursor position

            //  Creation of database area "Import1" isn't here, but in the DocShell
            //  slot execute, so it can be added to the undo action

            ScDBData* pDBData = pDocSh->GetDBData( ScRange(nPosX,nPosY,nTab), SC_DB_OLD, FALSE );
            String sTarget;
            if (pDBData)
                sTarget = pDBData->GetName();
            else
            {
                ScAddress aPos( nPosX,nPosY,nTab );
                aPos.Format( sTarget, SCA_ABS_3D, pDoc );
            }
            SfxStringItem aTarget(FN_PARAM_1, sTarget);

            BOOL bAreaIsNew = !pDBData;
            SfxBoolItem aAreaNew(FN_PARAM_2, bAreaIsNew);

            //  asynchronous, to avoid doing the whole import in drop handler
            SfxDispatcher& rDisp = GetViewData()->GetDispatcher();
            rDisp.Execute(SID_SBA_IMPORT, SFX_CALLMODE_ASYNCHRON,
                                        &aDataDesc, &aTarget, &aAreaNew, 0L );

            bRet = TRUE;
        }
    }
    else if (nFormatId == SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE)
    {
        //  insert database field control

        String aString;
        if ( aDataHelper.GetString( nFormatId, aString ) )
        {
            MakeDrawLayer();
            ScDrawView* pDrawView = GetScDrawView();
            SdrObject* pObj = pDrawView->CreateFieldControl(aString);
            if (pObj)
            {
                Point aInsPos = aPos;
                Rectangle aRect(pObj->GetLogicRect());
                aInsPos.X() -= aRect.GetSize().Width()  / 2;
                aInsPos.Y() -= aRect.GetSize().Height() / 2;
                if ( aInsPos.X() < 0 ) aInsPos.X() = 0;
                if ( aInsPos.Y() < 0 ) aInsPos.Y() = 0;
                aRect.SetPos(aInsPos);
                pObj->SetLogicRect(aRect);

                if ( pObj->ISA(SdrUnoObj) )
                    pObj->NbcSetLayer(SC_LAYER_CONTROLS);
                else
                    pObj->NbcSetLayer(SC_LAYER_FRONT);
                if (pObj->ISA(SdrObjGroup))
                {
                    SdrObjListIter aIter( *pObj, IM_DEEPWITHGROUPS );
                    SdrObject* pSubObj = aIter.Next();
                    while (pSubObj)
                    {
                        if ( pSubObj->ISA(SdrUnoObj) )
                            pSubObj->NbcSetLayer(SC_LAYER_CONTROLS);
                        else
                            pSubObj->NbcSetLayer(SC_LAYER_FRONT);
                        pSubObj = aIter.Next();
                    }
                }

                pDrawView->InsertObjectSafe(pObj, *pDrawView->GetPageViewPvNum(0));

                GetViewData()->GetViewShell()->SetDrawShell( TRUE );
                bRet = TRUE;
            }
        }
    }
    else if (nFormatId == SOT_FORMAT_BITMAP)
    {
        Bitmap aBmp;
        if( aDataHelper.GetBitmap( FORMAT_BITMAP, aBmp ) )
            bRet = PasteBitmap( aPos, aBmp );
    }
    else if (nFormatId == SOT_FORMAT_GDIMETAFILE)
    {
        GDIMetaFile aMtf;
        if( aDataHelper.GetGDIMetaFile( FORMAT_GDIMETAFILE, aMtf ) )
            bRet = PasteMetaFile( aPos, aMtf );
    }
    else if (nFormatId == SOT_FORMATSTR_ID_SVXB)
    {
        SotStorageStreamRef xStm;
        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_SVXB, xStm ) )
        {
            Graphic aGraphic;
            *xStm >> aGraphic;
            bRet = PasteGraphic( aPos, aGraphic, EMPTY_STRING, EMPTY_STRING );
        }
    }
    else if ( nFormatId == SOT_FORMATSTR_ID_DRAWING )
    {
        SotStorageStreamRef xStm;
        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStm ) )
        {
            MakeDrawLayer();    // before loading model, so 3D factory has been created

            SvtPathOptions aPathOpt;
            String aPath = aPathOpt.GetPalettePath();
            FmFormModel* pModel = new FmFormModel(
                                    aPath, NULL, GetViewData()->GetDocShell() );
            pModel->GetItemPool().FreezeIdRanges();
            xStm->Seek(0);

            //  SdrModel stream operator doesn't support XML
            //! call XML export here!
            xStm->SetVersion(SOFFICE_FILEFORMAT_50);

            pModel->SetStreamingSdrModel(TRUE);
            pModel->GetItemPool().Load(*xStm);
            *xStm >> *pModel;
            pModel->SetStreamingSdrModel(FALSE);

                                        // set everything to right layer:
            ULONG nObjCount = 0;
            USHORT nPages = pModel->GetPageCount();
            for (USHORT i=0; i<nPages; i++)
            {
                SdrPage* pPage = pModel->GetPage(i);
                SdrObjListIter aIter( *pPage, IM_DEEPWITHGROUPS );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    ++nObjCount;
                    if ( pObject->ISA(SdrUnoObj) )
                        pObject->NbcSetLayer(SC_LAYER_CONTROLS);
                    else
                        pObject->NbcSetLayer(SC_LAYER_FRONT);
                    pObject = aIter.Next();
                }
            }

            PasteDraw( aPos, pModel, (nObjCount > 1) );     // grouped if more than 1 object
            delete pModel;
            bRet = TRUE;
        }
    }
    else if ( nFormatId == SOT_FORMATSTR_ID_BIFF_5 )
    {
        //  do excel import into a clipboard document

        SotStorageStreamRef xStm;
        if( aDataHelper.GetSotStorageStream( SOT_FORMATSTR_ID_BIFF_5, xStm ) )
        {
            SvStorageRef pStor = new SvStorage( *xStm );
#if 0
            SvStorage aDest( "d:\\test.xls" );  // to see the file
            pStor->CopyTo( &aDest );
#endif
            ScDocument* pDoc = GetViewData()->GetDocument();
            ScDocument* pInsDoc = new ScDocument( SCDOCMODE_CLIP );
            USHORT nSrcTab = 0;     // Biff5 in clipboard: always sheet 0
            pInsDoc->ResetClip( pDoc, nSrcTab );

            SfxMedium aMed( pStor, TRUE );
            FltError eErr = ScImportExcel( aMed, pInsDoc, EIF_AUTO );
            if ( eErr == eERR_OK )
            {
                ScRange aSource;
                const ScExtDocOptions* pExtOpt = pInsDoc->GetExtDocOptions();
                const ScExtTabOptions* pTabOpt = pExtOpt ?
                                        pExtOpt->GetExtTabOptions(nSrcTab) : NULL;
                if ( pTabOpt && pTabOpt->bValidDim &&
                        pTabOpt->aDim.aEnd.Col() > pTabOpt->aDim.aStart.Col() &&
                        pTabOpt->aDim.aEnd.Row() > pTabOpt->aDim.aStart.Row() )
                {
                    //  "end" points behind the data area
                    aSource = ScRange(
                        pTabOpt->aDim.aStart.Col(), pTabOpt->aDim.aStart.Row(), nSrcTab,
                        pTabOpt->aDim.aEnd.Col()-1, pTabOpt->aDim.aEnd.Row()-1, nSrcTab );
                    //  selection can be bigger than the used area
                    if ( pTabOpt->bValidSel )
                    {
                        if ( pTabOpt->aLastSel.aStart.Col() < aSource.aStart.Col() )
                            aSource.aStart.SetCol( pTabOpt->aLastSel.aStart.Col() );
                        if ( pTabOpt->aLastSel.aStart.Row() < aSource.aStart.Row() )
                            aSource.aStart.SetRow( pTabOpt->aLastSel.aStart.Row() );
                        //  "end" in selection is really the end
                        if ( pTabOpt->aLastSel.aEnd.Col() > aSource.aEnd.Col() )
                            aSource.aEnd.SetCol( pTabOpt->aLastSel.aEnd.Col() );
                        if ( pTabOpt->aLastSel.aEnd.Row() > aSource.aEnd.Row() )
                            aSource.aEnd.SetRow( pTabOpt->aLastSel.aEnd.Row() );
                    }
                }
                else
                {
                    DBG_ERROR("no dimension");  //! possible?
                    USHORT nFirstCol, nFirstRow, nLastCol, nLastRow;
                    if ( pInsDoc->GetDataStart( nSrcTab, nFirstCol, nFirstRow ) )
                        pInsDoc->GetCellArea( nSrcTab, nLastCol, nLastRow );
                    else
                        nFirstCol = nFirstRow = nLastCol = nLastRow = 0;
                    aSource = ScRange( nFirstCol, nFirstRow, nSrcTab,
                                        nLastCol, nLastRow, nSrcTab );
                }

                if ( pLogicPos )
                {
                    // position specified (Drag&Drop) - change selection
                    MoveCursorAbs( nPosX, nPosY, SC_FOLLOW_NONE, FALSE, FALSE );
                    Unmark();
                }

                pInsDoc->SetClipArea( aSource );
                PasteFromClip(IDF_ALL, pInsDoc);
                delete pInsDoc;

                bRet = TRUE;
            }
        }
    }
    else if ( nFormatId == SOT_FORMAT_FILE )
    {
        USHORT nCount = DragServer::GetItemCount();
        if (nCount == 0)    // normal handling (not D&D)
        {
            String aFile;
            if ( aDataHelper.GetString( nFormatId, aFile ) )
                bRet = PasteFile( aPos, aFile );
        }
        else        // use multiple items from drag server
        {
            for( USHORT i = 0; i < nCount ; i++ )
            {
                String aFile = DragServer::PasteFile( i );
                SfxStringItem aNameItem( FID_INSERT_FILE, aFile );
                SfxPointItem aPosItem( FN_PARAM_1, aPos );
                SfxDispatcher* pDisp =
                    GetViewData()->GetViewShell()->GetViewFrame()->GetDispatcher();
                if (pDisp)
                    pDisp->Execute( FID_INSERT_FILE, SFX_CALLMODE_ASYNCHRON,
                                        &aNameItem, &aPosItem, (void*)0 );

                aPos.X() += 400;
                aPos.Y() += 400;
            }
            bRet = TRUE;
        }
    }
    else if ( nFormatId == SOT_FORMATSTR_ID_SOLK ||
              nFormatId == SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR ||
              nFormatId == SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK ||
              nFormatId == SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR )
    {
        bRet = PasteBookmark( nFormatId, rxTransferable, nPosX, nPosY );
    }

    return bRet;
}

String lcl_GetSubString( sal_Char* pData, long nStart, long nDataSize, rtl_TextEncoding eEncoding )
{
    if ( nDataSize <= nStart /* || pData[nDataSize] != 0 */ )
    {
        DBG_ERROR("DDE Data: invalid data");
        return String();
    }
    return String( pData + nStart, eEncoding );
}

BOOL ScViewFunc::PasteDDE( const uno::Reference<datatransfer::XTransferable>& rxTransferable )
{
    TransferableDataHelper aDataHelper( rxTransferable );

    //  check size (only if string is available in transferable)

    USHORT nCols = 1;
    USHORT nRows = 1;
    if ( aDataHelper.HasFormat( SOT_FORMAT_STRING ) )
    {
        String aDataStr;
        if ( aDataHelper.GetString( SOT_FORMAT_STRING, aDataStr ) )
        {
            //  get size from string the same way as in ScDdeLink::DataChanged

            aDataStr.ConvertLineEnd(LINEEND_LF);
            xub_StrLen nLen = aDataStr.Len();
            if (nLen && aDataStr.GetChar(nLen-1) == '\n')
                aDataStr.Erase(nLen-1);

            if (aDataStr.Len())
            {
                nRows = aDataStr.GetTokenCount( '\n' );
                String aLine = aDataStr.GetToken( 0, '\n' );
                if (aLine.Len())
                    nCols = aLine.GetTokenCount( '\t' );
            }
        }
    }

    //  create formula

    uno::Sequence<sal_Int8> aSequence;
    if ( !aDataHelper.GetSequence( SOT_FORMATSTR_ID_LINK, aSequence ) )
    {
        DBG_ERROR("DDE Data not found.");
        return FALSE;
    }
    long nSeqLen = aSequence.getLength();
    sal_Char* pData = (sal_Char*)aSequence.getConstArray();

    rtl_TextEncoding eSysEnc = gsl_getSystemTextEncoding();

    String aApp   = lcl_GetSubString( pData, 0, nSeqLen, eSysEnc );
    String aTopic = lcl_GetSubString( pData, aApp.Len() + 1, nSeqLen, eSysEnc );
    String aItem  = lcl_GetSubString( pData, aApp.Len() + aTopic.Len() + 2, nSeqLen, eSysEnc );

    if (!ScCompiler::pSymbolTableNative)
    {
        DBG_ERROR("ScCompiler::pSymbolTableNative missing");
        return FALSE;
    }

    //! use tokens
    String aFormula( '=' );
    aFormula += ScCompiler::pSymbolTableNative[SC_OPCODE_DDE];
    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("(\""));
    aFormula += aApp;
    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\";\""));
    aFormula += aTopic;
    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\";\""));
    aFormula += aItem;
    aFormula.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\")"));

    //  mark range

    USHORT nTab = GetViewData()->GetTabNo();
    USHORT nCurX = GetViewData()->GetCurX();
    USHORT nCurY = GetViewData()->GetCurY();
    HideAllCursors();
    DoneBlockMode();
    InitBlockMode( nCurX, nCurY, nTab );
    MarkCursor( nCurX+nCols-1, nCurY+nRows-1, nTab );
    ShowAllCursors();

    //  enter formula

    EnterMatrix( aFormula );
    CursorPosChanged();

    return TRUE;
}


