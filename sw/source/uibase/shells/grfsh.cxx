/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cmdid.h>
#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/protitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/sdgluitm.hxx>
#include <svx/sdgcoitm.hxx>
#include <svx/sdggaitm.hxx>
#include <svx/sdgtritm.hxx>
#include <svx/sdginitm.hxx>
#include <svx/sdgmoitm.hxx>
#include <editeng/brushitem.hxx>
#include <svx/grfflt.hxx>
#include <svx/compressgraphicdialog.hxx>
#include <vcl/GraphicNativeTransform.hxx>
#include <svx/tbxcolor.hxx>
#include <drawdoc.hxx>
#include <fmturl.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <frmatr.hxx>
#include <swundo.hxx>
#include <uitool.hxx>
#include <docsh.hxx>
#include <grfsh.hxx>
#include <frmmgr.hxx>
#include <frmdlg.hxx>
#include <frmfmt.hxx>
#include <grfatr.hxx>
#include <usrpref.hxx>
#include <edtwin.hxx>
#include <swwait.hxx>
#include <shells.hrc>
#include <popup.hrc>
#include <svx/extedit.hxx>
#include <svx/graphichelper.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>

#include <svx/drawitem.hxx>
#define SwGrfShell

#include <sfx2/msg.hxx>
#include "swslots.hxx"
#include "swabstdlg.hxx"
#include <unocrsr.hxx>
#include <memory>

#define TOOLBOX_NAME "colorbar"

class SwGrfShell::SwExternalToolEdit
    : public ExternalToolEdit
{
private:
    SwWrtShell *const m_pShell;
    ::std::shared_ptr<SwUnoCrsr> const m_pCursor;

public:
    explicit SwExternalToolEdit(SwWrtShell *const pShell)
        : m_pShell(pShell)
        , m_pCursor( // need only Point, must point to SwGrfNode
            pShell->GetDoc()->CreateUnoCrsr(
                *pShell->GetCurrentShellCursor().GetPoint()))
    {
    }

    virtual void Update(Graphic & rGraphic) override
    {
        DBG_TESTSOLARMUTEX();
        m_pShell->Push();
        m_pShell->GetCurrentShellCursor().DeleteMark();
        *m_pShell->GetCurrentShellCursor().GetPoint() = *m_pCursor->GetPoint();
        m_pShell->ReRead(OUString(), OUString(), &rGraphic);
        m_pShell->Pop();
    }
};

SFX_IMPL_INTERFACE(SwGrfShell, SwBaseShell)

void SwGrfShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu(SW_RES(MN_GRF_POPUPMENU));

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, RID_GRAFIK_TOOLBOX);
}

void SwGrfShell::Execute(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();

    sal_uInt16 nSlot = rReq.GetSlot();
    switch(nSlot)
    {
        case SID_TWAIN_TRANSFER:
        {
            GetView().ExecuteScan( rReq );
            break;
        }

        case SID_SAVE_GRAPHIC:
        {
            const Graphic *pGraphic;
            if(0 != (pGraphic = rSh.GetGraphic()))
            {
                OUString sGrfNm;
                OUString sFilterNm;
                rSh.GetGrfNms( &sGrfNm, &sFilterNm );
                GraphicHelper::ExportGraphic( *pGraphic, sGrfNm );
            }
        }
        break;

        case SID_COMPRESS_GRAPHIC:
        {
            const Graphic* pGraphic = rSh.GetGraphic();
            if( pGraphic )
            {
                Size aSize (
                    convertTwipToMm100(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Width()),
                    convertTwipToMm100(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Height()));

                SfxItemSet aSet( rSh.GetAttrPool(), RES_GRFATR_CROPGRF, RES_GRFATR_CROPGRF );
                rSh.GetCurAttr( aSet );
                SwCropGrf aCrop( static_cast<const SwCropGrf&>( aSet.Get(RES_GRFATR_CROPGRF)) );

                Rectangle aCropRectangle(
                    convertTwipToMm100(aCrop.GetLeft()),
                    convertTwipToMm100(aCrop.GetTop()),
                    convertTwipToMm100(aCrop.GetRight()),
                    convertTwipToMm100(aCrop.GetBottom()) );

                Graphic aGraphic = Graphic( *pGraphic );

                ScopedVclPtrInstance< CompressGraphicsDialog > aDialog( GetView().GetWindow(), aGraphic, aSize, aCropRectangle, GetView().GetViewFrame()->GetBindings() );
                if( aDialog->Execute() == RET_OK )
                {
                    rSh.StartAllAction();
                    rSh.StartUndo(UNDO_START);
                    Rectangle aScaledCropedRectangle = aDialog->GetScaledCropRectangle();

                    aCrop.SetLeft(   convertMm100ToTwip( aScaledCropedRectangle.Left() ));
                    aCrop.SetTop(    convertMm100ToTwip( aScaledCropedRectangle.Top() ));
                    aCrop.SetRight(  convertMm100ToTwip( aScaledCropedRectangle.Right() ));
                    aCrop.SetBottom( convertMm100ToTwip( aScaledCropedRectangle.Bottom() ));

                    Graphic aCompressedGraphic( aDialog->GetCompressedGraphic() );
                    rSh.ReRead(OUString(), OUString(), const_cast<const Graphic*>(&aCompressedGraphic));

                    rSh.SetAttrItem(aCrop);

                    rSh.EndUndo(UNDO_END);
                    rSh.EndAllAction();
                }
            }
        }
        break;
        case SID_EXTERNAL_EDIT:
        {
            // When the graphic is selected to be opened via some external tool
            // for advanced editing
            GraphicObject const*const pGraphicObject(rSh.GetGraphicObj());
            if(0 != pGraphicObject)
            {
                m_ExternalEdits.push_back(std::unique_ptr<SwExternalToolEdit>(
                            new SwExternalToolEdit(&rSh)));
                m_ExternalEdits.back()->Edit(pGraphicObject);
            }
        }
        break;
        case SID_CHANGE_PICTURE:
        case SID_INSERT_GRAPHIC:
        {
            // #i123922# implement slot independent from the two below to
            // bring up the insert graphic dialog and associated actions
            SwView& rLclView = GetView();
            rReq.SetReturnValue(SfxBoolItem(nSlot, rLclView.InsertGraphicDlg( rReq )));
            break;
        }
        case FN_FORMAT_GRAFIC_DLG:
        case FN_DRAW_WRAP_DLG:
        {
            SwFlyFrmAttrMgr aMgr( false, &rSh, rSh.IsFrmSelected() ?
                                               FRMMGR_TYPE_NONE : FRMMGR_TYPE_GRF);
            const SwViewOption* pVOpt = rSh.GetViewOptions();
            SwViewOption aUsrPref( *pVOpt );

            SfxItemSet aSet(GetPool(), //UUUU sorted by indices

                RES_FRMATR_BEGIN,RES_FRMATR_END - 1,                            // [   82
                RES_GRFATR_MIRRORGRF,RES_GRFATR_CROPGRF,                        // [  123

                //UUUU FillAttribute support
                XATTR_FILL_FIRST,       XATTR_FILL_LAST,                        // [ 1014

                SID_DOCFRAME,SID_DOCFRAME,                                      // [ 5598
                SID_ATTR_BORDER_INNER,SID_ATTR_BORDER_INNER,                    // [10023
                SID_ATTR_PAGE_SIZE,SID_ATTR_PAGE_SIZE,                          // [10051
                SID_ATTR_GRAF_KEEP_ZOOM,SID_ATTR_GRAF_KEEP_ZOOM,                // [10882
                SID_ATTR_GRAF_FRMSIZE,SID_ATTR_GRAF_GRAPHIC,                    // [10884, contains SID_ATTR_GRAF_FRMSIZE_PERCENT

                //UUUU items to hand over XPropertyList things like
                // XColorList, XHatchList, XGradientList and XBitmapList
                // to the Area TabPage
                SID_COLOR_TABLE,        SID_BITMAP_LIST,                        // [10179

                SID_HTML_MODE,SID_HTML_MODE,                                    // [10414
                FN_GET_PRINT_AREA,FN_GET_PRINT_AREA,                            // [21032
                FN_PARAM_GRF_CONNECT,FN_PARAM_GRF_CONNECT,                      // [21153
                FN_PARAM_GRF_DIALOG,FN_PARAM_GRF_DIALOG,                        // [21171
                FN_SET_FRM_NAME,FN_KEEP_ASPECT_RATIO,                           // [21306
                FN_SET_FRM_ALT_NAME,FN_SET_FRM_ALT_NAME,                        // [21318
                SID_REFERER,            SID_REFERER,
                0);

            //UUUU create needed items for XPropertyList entries from the DrawModel so that
            // the Area TabPage can access them
            const SwDrawModel* pDrawModel = rSh.GetView().GetDocShell()->GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();

            aSet.Put(SvxColorListItem(pDrawModel->GetColorList(), SID_COLOR_TABLE));
            aSet.Put(SvxGradientListItem(pDrawModel->GetGradientList(), SID_GRADIENT_LIST));
            aSet.Put(SvxHatchListItem(pDrawModel->GetHatchList(), SID_HATCH_LIST));
            aSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapList(), SID_BITMAP_LIST));

            sal_uInt16 nHtmlMode = ::GetHtmlMode(GetView().GetDocShell());
            aSet.Put(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));
            FieldUnit eMetric = ::GetDfltMetric((0 != (nHtmlMode&HTMLMODE_ON)));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );

            const SwRect* pRect = &rSh.GetAnyCurRect(RECT_PAGE);
            SwFormatFrmSize aFrmSize( ATT_VAR_SIZE, pRect->Width(), pRect->Height());
            aFrmSize.SetWhich( GetPool().GetWhich( SID_ATTR_PAGE_SIZE ) );
            aSet.Put( aFrmSize );

            aSet.Put(SfxStringItem(FN_SET_FRM_NAME, rSh.GetFlyName()));
            if ( nSlot == FN_FORMAT_GRAFIC_DLG )
            {
                // #i73249#
                aSet.Put( SfxStringItem( FN_SET_FRM_ALT_NAME, rSh.GetObjTitle() ) );
            }

            pRect = &rSh.GetAnyCurRect(RECT_PAGE_PRT);
            aFrmSize.SetWidth( pRect->Width() );
            aFrmSize.SetHeight( pRect->Height() );
            aFrmSize.SetWhich( GetPool().GetWhich(FN_GET_PRINT_AREA) );
            aSet.Put( aFrmSize );

            aSet.Put( aMgr.GetAttrSet() );
            aSet.SetParent( aMgr.GetAttrSet().GetParent() );

            // At percentage values initialize size
            SwFormatFrmSize aSizeCopy = static_cast<const SwFormatFrmSize&>(aSet.Get(RES_FRM_SIZE));
            if (aSizeCopy.GetWidthPercent() && aSizeCopy.GetWidthPercent() != SwFormatFrmSize::SYNCED)
                aSizeCopy.SetWidth(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Width());
            if (aSizeCopy.GetHeightPercent() && aSizeCopy.GetHeightPercent() != SwFormatFrmSize::SYNCED)
                aSizeCopy.SetHeight(rSh.GetAnyCurRect(RECT_FLY_EMBEDDED).Height());
            // and now set the size for "external" tabpages
            {
                SvxSizeItem aSzItm( SID_ATTR_GRAF_FRMSIZE, aSizeCopy.GetSize() );
                aSet.Put( aSzItm );

                Size aSz( aSizeCopy.GetWidthPercent(), aSizeCopy.GetHeightPercent() );
                if( SwFormatFrmSize::SYNCED == aSz.Width() )   aSz.Width() = 0;
                if( SwFormatFrmSize::SYNCED == aSz.Height() )  aSz.Height() = 0;

                aSzItm.SetSize( aSz );
                aSzItm.SetWhich( SID_ATTR_GRAF_FRMSIZE_PERCENT );
                aSet.Put( aSzItm );
            }

            OUString sGrfNm;
            OUString sFilterNm;
            rSh.GetGrfNms( &sGrfNm, &sFilterNm );
            if( !sGrfNm.isEmpty() )
            {
                aSet.Put( SvxBrushItem( INetURLObject::decode( sGrfNm,
                                           INetURLObject::DECODE_UNAMBIGUOUS ),
                                        sFilterNm, GPOS_LT,
                                        SID_ATTR_GRAF_GRAPHIC ));
            }
            else
            {
                // #119353# - robust
                const GraphicObject* pGrfObj = rSh.GetGraphicObj();
                if ( pGrfObj )
                {
                    aSet.Put( SvxBrushItem( *pGrfObj, GPOS_LT,
                                            SID_ATTR_GRAF_GRAPHIC ) );
                }
            }
            aSet.Put( SfxBoolItem( FN_PARAM_GRF_CONNECT, !sGrfNm.isEmpty() ) );

            // get Mirror and Crop
            {
                SfxItemSet aTmpSet( rSh.GetAttrPool(),
                                RES_GRFATR_MIRRORGRF, RES_GRFATR_CROPGRF );

                rSh.GetCurAttr( aTmpSet );
                aSet.Put( aTmpSet );
            }

            aSet.Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO, aUsrPref.IsKeepRatio()));
            aSet.Put(SfxBoolItem( SID_ATTR_GRAF_KEEP_ZOOM, aUsrPref.IsGrfKeepZoom()));

            aSet.Put(SfxFrameItem( SID_DOCFRAME, &GetView().GetViewFrame()->GetTopFrame()));

            SfxObjectShell * sh = rSh.GetDoc()->GetPersist();
            if (sh != 0 && sh->HasName())
            {
                aSet.Put(
                    SfxStringItem(SID_REFERER, sh->GetMedium()->GetName()));
            }

            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "no dialog factory!");
            std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateFrmTabDialog("PictureDialog",
                                                    GetView().GetViewFrame(),
                                                    GetView().GetWindow(),
                                                    aSet, false));
            OSL_ENSURE(pDlg, "no tab dialog!");

            if (nSlot == FN_DRAW_WRAP_DLG)
                pDlg->SetCurPageId("wrap");

            if( pDlg->Execute() )
            {
                rSh.StartAllAction();
                rSh.StartUndo(UNDO_START);
                const SfxPoolItem* pItem;
                SfxItemSet* pSet = const_cast<SfxItemSet*>(pDlg->GetOutputItemSet());
                rReq.Done(*pSet);
                // change the 2 frmsize SizeItems to the correct SwFrmSizeItem
                if( SfxItemState::SET == pSet->GetItemState(
                                SID_ATTR_GRAF_FRMSIZE, false, &pItem ))
                {
                    SwFormatFrmSize aSize;
                    const Size& rSz = static_cast<const SvxSizeItem*>(pItem)->GetSize();
                    aSize.SetWidth( rSz.Width() );
                    aSize.SetHeight( rSz.Height() );

                    if( SfxItemState::SET == pSet->GetItemState(
                            SID_ATTR_GRAF_FRMSIZE_PERCENT, false, &pItem ))
                    {
                        const Size& rRelativeSize = static_cast<const SvxSizeItem*>(pItem)->GetSize();
                        aSize.SetWidthPercent( static_cast< sal_uInt8 >( rRelativeSize.Width() ) );
                        aSize.SetHeightPercent( static_cast< sal_uInt8 >( rRelativeSize.Height() ) );
                    }
                    pSet->Put( aSize );
                }

                // Templates AutoUpdate
                SwFrameFormat* pFormat = rSh.GetSelectedFrameFormat();
                if(pFormat && pFormat->IsAutoUpdateFormat())
                {
                    pFormat->SetFormatAttr(*pSet);
                    SfxItemSet aShellSet(GetPool(), RES_FRM_SIZE,   RES_FRM_SIZE,
                                                    RES_SURROUND,   RES_SURROUND,
                                                    RES_ANCHOR,     RES_ANCHOR,
                                                    RES_VERT_ORIENT,RES_HORI_ORIENT,
                                                    0);
                    aShellSet.Put(*pSet);
                    aMgr.SetAttrSet(aShellSet);
                }
                else
                {
                    aMgr.SetAttrSet(*pSet);
                }
                aMgr.UpdateFlyFrm();

                bool bApplyUsrPref = false;
                if (SfxItemState::SET == pSet->GetItemState(
                    FN_KEEP_ASPECT_RATIO, true, &pItem ))
                {
                    aUsrPref.SetKeepRatio(
                                    static_cast<const SfxBoolItem*>(pItem)->GetValue() );
                    bApplyUsrPref = true;
                }
                if( SfxItemState::SET == pSet->GetItemState(
                    SID_ATTR_GRAF_KEEP_ZOOM, true, &pItem ))
                {
                    aUsrPref.SetGrfKeepZoom(
                                    static_cast<const SfxBoolItem*>(pItem)->GetValue() );
                    bApplyUsrPref = true;
                }

                if( bApplyUsrPref )
                    SW_MOD()->ApplyUsrPref(aUsrPref, &GetView());

                // and now set all the graphic attributes and other stuff
                if( SfxItemState::SET == pSet->GetItemState(
                                        SID_ATTR_GRAF_GRAPHIC, true, &pItem ))
                {
                    if( !static_cast<const SvxBrushItem*>(pItem)->GetGraphicLink().isEmpty() )
                        sGrfNm = static_cast<const SvxBrushItem*>(pItem)->GetGraphicLink();
                    else
                        sGrfNm.clear();

                    if( !static_cast<const SvxBrushItem*>(pItem)->GetGraphicFilter().isEmpty() )
                        sFilterNm = static_cast<const SvxBrushItem*>(pItem)->GetGraphicFilter();
                    else
                        sFilterNm.clear();

                    if( !sGrfNm.isEmpty() )
                    {
                        SwDocShell* pDocSh = GetView().GetDocShell();
                        SwWait aWait( *pDocSh, true );
                        SfxMedium* pMedium = pDocSh->GetMedium();
                        INetURLObject aAbs;
                        if( pMedium )
                            aAbs = pMedium->GetURLObject();
                        rSh.ReRead( URIHelper::SmartRel2Abs(
                                        aAbs, sGrfNm,
                                        URIHelper::GetMaybeFileHdl() ),
                                     sFilterNm, 0 );
                    }
                }
                if ( SfxItemState::SET == pSet->GetItemState(
                                        FN_SET_FRM_ALT_NAME, true, &pItem ))
                {
                    // #i73249#
                    rSh.SetObjTitle( static_cast<const SfxStringItem*>(pItem)->GetValue() );
                }

                SfxItemSet aGrfSet( rSh.GetAttrPool(), RES_GRFATR_BEGIN,
                                                       RES_GRFATR_END-1 );
                aGrfSet.Put( *pSet );
                if( aGrfSet.Count() )
                    rSh.SetAttrSet( aGrfSet );

                rSh.EndUndo(UNDO_END);
                rSh.EndAllAction();
            }
        }
        break;

        case FN_GRAPHIC_MIRROR_ON_EVEN_PAGES:
        {
            SfxItemSet aSet(rSh.GetAttrPool(), RES_GRFATR_MIRRORGRF, RES_GRFATR_MIRRORGRF);
            rSh.GetCurAttr( aSet );
            SwMirrorGrf aGrf(static_cast<const SwMirrorGrf &>(aSet.Get(RES_GRFATR_MIRRORGRF)));
            aGrf.SetGrfToggle(!aGrf.IsGrfToggle());
            rSh.SetAttrItem(aGrf);
        }
        break;

        case SID_OBJECT_CROP:
        {
            GraphicObject const *pGraphicObject = rSh.GetGraphicObj();
            if (0 != pGraphicObject  && SDRDRAG_CROP != rSh.GetDragMode()) {
                rSh.StartCropImage();
            }
        }
        break;

        default:
            OSL_ENSURE(false, "wrong dispatcher");
            return;
    }
}

void SwGrfShell::ExecAttr( SfxRequest &rReq )
{
    sal_uInt16 nGrfType;
    if( CNT_GRF == GetShell().GetCntType() &&
        ( GRAPHIC_BITMAP == ( nGrfType = GetShell().GetGraphicType()) ||
          GRAPHIC_GDIMETAFILE == nGrfType ))
    {
        SfxItemSet aGrfSet( GetShell().GetAttrPool(), RES_GRFATR_BEGIN,
                                                      RES_GRFATR_END -1 );
        const SfxItemSet *pArgs = rReq.GetArgs();
        const SfxPoolItem* pItem;
        sal_uInt16 nSlot = rReq.GetSlot();
        if( !pArgs || SfxItemState::SET != pArgs->GetItemState( nSlot, false, &pItem ))
            pItem = 0;

        switch( nSlot )
        {
            case SID_FLIP_VERTICAL:
            case SID_FLIP_HORIZONTAL:
            {
                GetShell().GetCurAttr( aGrfSet );
                SwMirrorGrf aMirror( static_cast<const SwMirrorGrf&>(aGrfSet.Get( RES_GRFATR_MIRRORGRF )) );
                sal_uInt16 nMirror = aMirror.GetValue();
                if ( nSlot==SID_FLIP_HORIZONTAL )
                    switch( nMirror )
                    {
                    case RES_MIRROR_GRAPH_DONT: nMirror = RES_MIRROR_GRAPH_VERT;
                                                break;
                    case RES_MIRROR_GRAPH_HOR:  nMirror = RES_MIRROR_GRAPH_BOTH;
                                                break;
                    case RES_MIRROR_GRAPH_VERT:   nMirror = RES_MIRROR_GRAPH_DONT;
                                                break;
                    case RES_MIRROR_GRAPH_BOTH: nMirror = RES_MIRROR_GRAPH_HOR;
                                                break;
                    }
                else
                    switch( nMirror )
                    {
                    case RES_MIRROR_GRAPH_DONT: nMirror = RES_MIRROR_GRAPH_HOR;
                                                break;
                    case RES_MIRROR_GRAPH_VERT: nMirror = RES_MIRROR_GRAPH_BOTH;
                                                break;
                    case RES_MIRROR_GRAPH_HOR:    nMirror = RES_MIRROR_GRAPH_DONT;
                                                break;
                    case RES_MIRROR_GRAPH_BOTH: nMirror = RES_MIRROR_GRAPH_VERT;
                                                break;
                    }
                aMirror.SetValue( nMirror );
                aGrfSet.ClearItem();
                aGrfSet.Put( aMirror );
            }
            break;

        case SID_ATTR_GRAF_LUMINANCE:
            if( pItem )
                aGrfSet.Put( SwLuminanceGrf(
                            static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
            break;

        case SID_ATTR_GRAF_CONTRAST:
            if( pItem )
                aGrfSet.Put( SwContrastGrf(
                            static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
            break;

        case SID_ATTR_GRAF_RED:
            if( pItem )
                aGrfSet.Put( SwChannelRGrf(
                            static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
            break;

        case SID_ATTR_GRAF_GREEN:
            if( pItem )
                aGrfSet.Put( SwChannelGGrf(
                            static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
            break;

        case SID_ATTR_GRAF_BLUE:
            if( pItem )
                aGrfSet.Put( SwChannelBGrf(
                            static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
            break;

        case SID_ATTR_GRAF_GAMMA:
            if( pItem )
            {
                double fVal = static_cast<const SfxUInt32Item*>(pItem)->GetValue();
                aGrfSet.Put( SwGammaGrf(fVal/100. ));
            }
            break;

        case SID_ATTR_GRAF_TRANSPARENCE:
            if( pItem )
                aGrfSet.Put( SwTransparencyGrf(
                    static_cast< sal_Int8 >( static_cast<const SfxUInt16Item*>(pItem )->GetValue() ) ) );
            break;

        case SID_ATTR_GRAF_INVERT:
            if( pItem )
                aGrfSet.Put( SwInvertGrf(
                            static_cast<const SfxBoolItem*>(pItem)->GetValue() ));
            break;

        case SID_ATTR_GRAF_MODE:
            if( pItem )
                aGrfSet.Put( SwDrawModeGrf(
                            static_cast<const SfxUInt16Item*>(pItem)->GetValue() ));
            break;

        case SID_COLOR_SETTINGS:
        {
            svx::ToolboxAccess aToolboxAccess( TOOLBOX_NAME );
            aToolboxAccess.toggleToolbox();
            break;
        }

        case SID_GRFFILTER:
        case SID_GRFFILTER_INVERT:
        case SID_GRFFILTER_SMOOTH:
        case SID_GRFFILTER_SHARPEN:
        case SID_GRFFILTER_REMOVENOISE:
        case SID_GRFFILTER_SOBEL:
        case SID_GRFFILTER_MOSAIC:
        case SID_GRFFILTER_EMBOSS:
        case SID_GRFFILTER_POSTER:
        case SID_GRFFILTER_POPART:
        case SID_GRFFILTER_SEPIA:
        case SID_GRFFILTER_SOLARIZE:
            if( GRAPHIC_BITMAP == nGrfType )
            {
                // #119353# - robust
                const GraphicObject* pFilterObj( GetShell().GetGraphicObj() );
                if ( pFilterObj )
                {
                    GraphicObject aFilterObj( *pFilterObj );
                    if( SVX_GRAPHICFILTER_ERRCODE_NONE ==
                        SvxGraphicFilter::ExecuteGrfFilterSlot( rReq, aFilterObj ))
                        GetShell().ReRead( OUString(), OUString(),
                                           &aFilterObj.GetGraphic() );
                }
            }
            break;

        default:
            OSL_ENSURE(false, "wrong dispatcher");
        }

        if( aGrfSet.Count() )
            GetShell().SetAttrSet( aGrfSet );
    }
    GetView().GetViewFrame()->GetBindings().Invalidate(rReq.GetSlot());
}

void SwGrfShell::GetAttrState(SfxItemSet &rSet)
{
    SwWrtShell &rSh = GetShell();
    SfxItemSet aCoreSet( GetPool(), aNoTextNodeSetRange );
    rSh.GetCurAttr( aCoreSet );
    bool bParentCntProt = FlyProtectFlags::NONE != rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent );
    bool bIsGrfContent = CNT_GRF == GetShell().GetCntType();

    SetGetStateSet( &rSet );

    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        bool bDisable = bParentCntProt;
        switch( nWhich )
        {
        case SID_INSERT_GRAPHIC:
        case FN_FORMAT_GRAFIC_DLG:
        case SID_TWAIN_TRANSFER:
            if( bParentCntProt || !bIsGrfContent )
                bDisable = true;
            else if ( nWhich == SID_INSERT_GRAPHIC
                      && rSh.CrsrInsideInputField() )
            {
                bDisable = true;
            }
            break;

        case SID_SAVE_GRAPHIC:
        case SID_EXTERNAL_EDIT:
            if( rSh.GetGraphicType() == GRAPHIC_NONE )
                bDisable = true;
            break;

        case SID_COLOR_SETTINGS:
        {
            if ( bParentCntProt || !bIsGrfContent )
                bDisable = true;
            else
            {
                svx::ToolboxAccess aToolboxAccess( TOOLBOX_NAME );
                rSet.Put( SfxBoolItem( nWhich, aToolboxAccess.isToolboxVisible() ) );
            }
            break;
        }

        case SID_FLIP_HORIZONTAL:
            if( !bParentCntProt )
            {
                MirrorGraph nState = static_cast< const MirrorGraph >(static_cast<const SwMirrorGrf &>( aCoreSet.Get(
                                        RES_GRFATR_MIRRORGRF )).GetValue());

                rSet.Put(SfxBoolItem( nWhich, nState == RES_MIRROR_GRAPH_VERT ||
                                              nState == RES_MIRROR_GRAPH_BOTH));
            }
            break;

        case SID_FLIP_VERTICAL:
            if( !bParentCntProt )
            {
                MirrorGraph nState = static_cast< MirrorGraph >(static_cast<const SwMirrorGrf &>( aCoreSet.Get(
                                        RES_GRFATR_MIRRORGRF )).GetValue());

                rSet.Put(SfxBoolItem( nWhich, nState == RES_MIRROR_GRAPH_HOR ||
                                              nState == RES_MIRROR_GRAPH_BOTH));
            }
            break;

        case SID_ATTR_GRAF_LUMINANCE:
            if( !bParentCntProt )
                rSet.Put( SfxInt16Item( nWhich, static_cast<const SwLuminanceGrf&>(
                        aCoreSet.Get(RES_GRFATR_LUMINANCE)).GetValue() ));
            break;

        case SID_ATTR_GRAF_CONTRAST:
            if( !bParentCntProt )
                rSet.Put( SfxInt16Item( nWhich, static_cast<const SwContrastGrf&>(
                        aCoreSet.Get(RES_GRFATR_CONTRAST)).GetValue() ));
            break;

        case SID_ATTR_GRAF_RED:
            if( !bParentCntProt )
                rSet.Put( SfxInt16Item( nWhich, static_cast<const SwChannelRGrf&>(
                        aCoreSet.Get(RES_GRFATR_CHANNELR)).GetValue() ));
            break;

        case SID_ATTR_GRAF_GREEN:
            if( !bParentCntProt )
                rSet.Put( SfxInt16Item( nWhich, static_cast<const SwChannelGGrf&>(
                        aCoreSet.Get(RES_GRFATR_CHANNELG)).GetValue() ));
            break;

        case SID_ATTR_GRAF_BLUE:
            if( !bParentCntProt )
                rSet.Put( SfxInt16Item( nWhich, static_cast<const SwChannelBGrf&>(
                        aCoreSet.Get(RES_GRFATR_CHANNELB)).GetValue() ));
            break;

        case SID_ATTR_GRAF_GAMMA:
            if( !bParentCntProt )
                rSet.Put( SfxUInt32Item( nWhich, static_cast< sal_uInt32 >(
                    static_cast<const SwGammaGrf&>(aCoreSet.Get( RES_GRFATR_GAMMA ) ).GetValue() * 100 ) ) );
            break;

        case SID_ATTR_GRAF_TRANSPARENCE:
            if( !bParentCntProt )
            {
                // #119353# - robust
                const GraphicObject* pGrafObj = rSh.GetGraphicObj();
                if ( pGrafObj )
                {
                    if( pGrafObj->IsAnimated() ||
                        GRAPHIC_GDIMETAFILE == pGrafObj->GetType() )
                        bDisable = true;
                    else
                        rSet.Put( SfxUInt16Item( nWhich, static_cast<const SwTransparencyGrf&>(
                            aCoreSet.Get(RES_GRFATR_TRANSPARENCY)).GetValue() ));
                }
            }
            break;

        case SID_ATTR_GRAF_INVERT:
            if( !bParentCntProt )
                rSet.Put( SfxBoolItem( nWhich, static_cast<const SwInvertGrf&>(
                        aCoreSet.Get(RES_GRFATR_INVERT)).GetValue() ));
            break;

        case SID_ATTR_GRAF_MODE:
            if( !bParentCntProt )
                rSet.Put( SfxUInt16Item( nWhich, static_cast<const SwDrawModeGrf&>(
                        aCoreSet.Get(RES_GRFATR_DRAWMODE)).GetValue() ));
            break;

        case SID_GRFFILTER:
        case SID_GRFFILTER_INVERT:
        case SID_GRFFILTER_SMOOTH:
        case SID_GRFFILTER_SHARPEN:
        case SID_GRFFILTER_REMOVENOISE:
        case SID_GRFFILTER_SOBEL:
        case SID_GRFFILTER_MOSAIC:
        case SID_GRFFILTER_EMBOSS:
        case SID_GRFFILTER_POSTER:
        case SID_GRFFILTER_POPART:
        case SID_GRFFILTER_SEPIA:
        case SID_GRFFILTER_SOLARIZE:
            {
                if( bParentCntProt || !bIsGrfContent )
                    bDisable = true;
                // #i59688# load graphic only if type is unknown
                else
                {
                    const sal_uInt16 eGraphicType( rSh.GetGraphicType() );
                    if ( ( eGraphicType == GRAPHIC_NONE ||
                           eGraphicType == GRAPHIC_DEFAULT ) &&
                         rSh.IsGrfSwapOut( true ) )
                    {
                        rSet.DisableItem( nWhich );
                        if( AddGrfUpdateSlot( nWhich ))
                            rSh.GetGraphic(false);  // start the loading
                    }
                    else
                    {
                        bDisable = eGraphicType != GRAPHIC_BITMAP;
                    }
                }
            }
            break;

        case SID_OBJECT_CROP:
            {
                bDisable = FlyProtectFlags::NONE != rSh.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent );
                if( rSh.GetGraphicType() == GRAPHIC_NONE )
                    bDisable = true;
            }
            break;

        default:
            bDisable = false;
        }

        if( bDisable )
            rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
    SetGetStateSet( 0 );
}

void SwGrfShell::ExecuteRotation(SfxRequest &rReq)
{
    sal_uInt16 aRotation;

    SwWrtShell& rShell = GetShell();

    if (rReq.GetSlot() == SID_ROTATE_GRAPHIC_LEFT)
    {
        aRotation = 900;
    }
    else if (rReq.GetSlot() == SID_ROTATE_GRAPHIC_RIGHT)
    {
        aRotation = 2700;
    }
    else
    {
        return;
    }

    rShell.StartAllAction();
    rShell.StartUndo(UNDO_START);

    Graphic aGraphic = *rShell.GetGraphic();
    GraphicNativeTransform aTransform(aGraphic);
    aTransform.rotate(aRotation);
    rShell.ReRead(OUString(), OUString(), const_cast<const Graphic*>(&aGraphic));

    SwFlyFrmAttrMgr aManager(false, &rShell, rShell.IsFrmSelected() ? FRMMGR_TYPE_NONE : FRMMGR_TYPE_GRF);
    const long nRotatedWidth = aManager.GetSize().Height();
    const long nRotatedHeight = aManager.GetSize().Width();
    Size aSize(nRotatedWidth, nRotatedHeight);
    aManager.SetSize(aSize);
    aManager.UpdateFlyFrm();

    SfxItemSet aSet( rShell.GetAttrPool(), RES_GRFATR_CROPGRF, RES_GRFATR_CROPGRF );
    rShell.GetCurAttr( aSet );
    SwCropGrf aCrop( static_cast<const SwCropGrf&>( aSet.Get(RES_GRFATR_CROPGRF) ) );
    Rectangle aCropRectangle(aCrop.GetLeft(),  aCrop.GetTop(), aCrop.GetRight(), aCrop.GetBottom());

    if (rReq.GetSlot() == SID_ROTATE_GRAPHIC_LEFT)
    {
        aCrop.SetLeft(   aCropRectangle.Top()    );
        aCrop.SetTop(    aCropRectangle.Right()  );
        aCrop.SetRight(  aCropRectangle.Bottom() );
        aCrop.SetBottom( aCropRectangle.Left()   );
    }
    else if (rReq.GetSlot() == SID_ROTATE_GRAPHIC_RIGHT)
    {
        aCrop.SetLeft(   aCropRectangle.Bottom() );
        aCrop.SetTop(    aCropRectangle.Left()   );
        aCrop.SetRight(  aCropRectangle.Top()    );
        aCrop.SetBottom( aCropRectangle.Right()  );
    }

    rShell.SetAttrItem(aCrop);

    rShell.EndUndo(UNDO_END);
    rShell.EndAllAction();
}

void SwGrfShell::GetAttrStateForRotation(SfxItemSet &rSet)
{
    SwWrtShell& rShell = GetShell();
    bool bIsParentContentProtected = FlyProtectFlags::NONE != rShell.IsSelObjProtected( FlyProtectFlags::Content|FlyProtectFlags::Parent );

    SetGetStateSet( &rSet );

    SfxWhichIter aIterator( rSet );
    sal_uInt16 nWhich = aIterator.FirstWhich();
    while( nWhich )
    {
        bool bDisable = bIsParentContentProtected;
        switch( nWhich )
        {
        case SID_ROTATE_GRAPHIC_LEFT:
        case SID_ROTATE_GRAPHIC_RIGHT:
            if( rShell.GetGraphicType() == GRAPHIC_NONE )
            {
                bDisable = true;
            }
            else
            {
                Graphic aGraphic = *rShell.GetGraphic();
                GraphicNativeTransform aTransform(aGraphic);
                if (!aTransform.canBeRotated())
                {
                    bDisable = true;
                }
            }
            break;
        default:
            bDisable = false;
        }

        if( bDisable )
            rSet.DisableItem( nWhich );
        nWhich = aIterator.NextWhich();
    }
    SetGetStateSet( 0 );
}

SwGrfShell::~SwGrfShell()
{
}

SwGrfShell::SwGrfShell(SwView &_rView) :
    SwBaseShell(_rView)
{
    SetName(OUString("Graphic"));
    SetHelpId(SW_GRFSHELL);
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Graphic));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
