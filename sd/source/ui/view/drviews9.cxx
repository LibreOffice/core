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

#include <config_features.h>

#include <DrawViewShell.hxx>
#include <editeng/outlobj.hxx>
#include <vcl/wrkwin.hxx>
#include <svx/xgrad.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <sfx2/bindings.hxx>

#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <svx/svxids.hrc>
#include <svx/xtable.hxx>
#include <svx/gallery.hxx>
#include <vcl/graph.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <sot/storage.hxx>
#include <svl/whiter.hxx>
#include <basic/sbstar.hxx>
#include <basic/sberrors.hxx>

#include <sfx2/viewfrm.hxx>

#include <app.hrc>
#include <strings.hrc>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <drawview.hxx>
#include <DrawDocShell.hxx>
#include <sdresid.hxx>
#include <fupoor.hxx>

#include <svx/galleryitem.hxx>
#include <com/sun/star/gallery/GalleryItemType.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <memory>

using namespace com::sun::star;

namespace sd {

void DrawViewShell::ExecGallery(SfxRequest const & rReq)
{
    // nothing is executed during a slide show!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    const SfxItemSet* pArgs = rReq.GetArgs();

    const SvxGalleryItem* pGalleryItem = SfxItemSet::GetItem<SvxGalleryItem>(pArgs, SID_GALLERY_FORMATS, false);
    if ( !pGalleryItem )
        return;

    GetDocSh()->SetWaitCursor( true );

    sal_Int8 nType( pGalleryItem->GetType() );
    // insert graphic
    if (nType == css::gallery::GalleryItemType::GRAPHIC)
    {
        Graphic aGraphic( pGalleryItem->GetGraphic() );

        // reduce size if necessary
        ScopedVclPtrInstance< Window > aWindow(GetActiveWindow());
        aWindow->SetMapMode(aGraphic.GetPrefMapMode());
        Size aSizePix = aWindow->LogicToPixel(aGraphic.GetPrefSize());
        aWindow->SetMapMode( MapMode(MapUnit::Map100thMM) );
        Size aSize = aWindow->PixelToLogic(aSizePix);

        // constrain size to page size if necessary
        SdrPage* pPage = mpDrawView->GetSdrPageView()->GetPage();
        Size aPageSize = pPage->GetSize();
        aPageSize.AdjustWidth( -(pPage->GetLeftBorder() + pPage->GetRightBorder()) );
        aPageSize.AdjustHeight( -(pPage->GetUpperBorder() + pPage->GetLowerBorder()) );

        // If the image is too large we make it fit into the page
        if ( ( ( aSize.Height() > aPageSize.Height() ) || ( aSize.Width()   > aPageSize.Width() ) ) &&
            aSize.Height() && aPageSize.Height() )
        {
            float fGrfWH =  static_cast<float>(aSize.Width()) /
                            static_cast<float>(aSize.Height());
            float fWinWH =  static_cast<float>(aPageSize.Width()) /
                            static_cast<float>(aPageSize.Height());

            // constrain size to page size if necessary
            if ((fGrfWH != 0.F) && (fGrfWH < fWinWH))
            {
                aSize.setWidth( static_cast<long>(aPageSize.Height() * fGrfWH) );
                aSize.setHeight( aPageSize.Height() );
            }
            else
            {
                aSize.setWidth( aPageSize.Width() );
                aSize.setHeight( static_cast<long>(aPageSize.Width() / fGrfWH) );
            }
        }

        // set output rectangle for graphic
        Point aPnt ((aPageSize.Width()  - aSize.Width())  / 2,
                    (aPageSize.Height() - aSize.Height()) / 2);
        aPnt += Point(pPage->GetLeftBorder(), pPage->GetUpperBorder());
        ::tools::Rectangle aRect (aPnt, aSize);

        SdrGrafObj* pGrafObj = nullptr;

        bool bInsertNewObject = true;

        if ( mpDrawView->AreObjectsMarked() )
        {
            // is there a empty graphic object?
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();

            if (rMarkList.GetMarkCount() == 1)
            {
                SdrMark* pMark = rMarkList.GetMark(0);
                SdrObject* pObj = pMark->GetMarkedSdrObj();

                if (pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_GRAF)
                {
                    pGrafObj = static_cast<SdrGrafObj*>(pObj);

                    if( pGrafObj->IsEmptyPresObj() )
                    {
                        // the empty graphic object gets a new graphic
                        bInsertNewObject = false;

                        SdrGrafObj* pNewGrafObj(pGrafObj->CloneSdrObject(pGrafObj->getSdrModelFromSdrObject()));
                        pNewGrafObj->SetEmptyPresObj(false);
                        pNewGrafObj->SetOutlinerParaObject(nullptr);
                        pNewGrafObj->SetGraphic(aGraphic);

                        OUString aStr(mpDrawView->GetDescriptionOfMarkedObjects());
                        aStr += " " + SdResId(STR_UNDO_REPLACE);
                        mpDrawView->BegUndo(aStr);
                        SdrPageView* pPV = mpDrawView->GetSdrPageView();
                        mpDrawView->ReplaceObjectAtView(pGrafObj, *pPV, pNewGrafObj);
                        mpDrawView->EndUndo();
                    }
                }
            }
        }

        if( bInsertNewObject )
        {
            pGrafObj = new SdrGrafObj(
                GetView()->getSdrModelFromSdrView(),
                aGraphic,
                aRect);
            SdrPageView* pPV = mpDrawView->GetSdrPageView();
            mpDrawView->InsertObjectAtView(pGrafObj, *pPV, SdrInsertFlags::SETDEFLAYER);
        }
    }
    // insert sound
    else if( nType == css::gallery::GalleryItemType::MEDIA )
    {
        const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, pGalleryItem->GetURL() );
        GetViewFrame()->GetDispatcher()->ExecuteList(SID_INSERT_AVMEDIA,
                SfxCallMode::SYNCHRON, { &aMediaURLItem });
    }

    GetDocSh()->SetWaitCursor( false );
}

/**
 * Edit macros for attribute configuration
 */

/* the work flow to adjust the attributes is nearly everywhere the same
   1. read existing attributes
   2. read parameter from the basic-set
   3. delete selected item from the attribute-set
   4. create new attribute-item
   5. insert item into set      */
void DrawViewShell::AttrExec (SfxRequest &rReq)
{
    // nothing is executed during a slide show!
    if(HasCurrentFunction(SID_PRESENTATION))
        return;

    CheckLineTo (rReq);

    SfxBindings&    rBindings = GetViewFrame()->GetBindings();
    std::unique_ptr<SfxItemSet> pAttr(new SfxItemSet ( GetDoc()->GetPool() ));

    GetView()->GetAttributes( *pAttr );
    const SfxItemSet* pArgs = rReq.GetArgs();

    switch (rReq.GetSlot ())
    {
        // set new fill-style
        case SID_SETFILLSTYLE :
            if (pArgs && pArgs->Count () == 1)
            {
                const SfxUInt32Item* pFillStyle = rReq.GetArg<SfxUInt32Item>(ID_VAL_STYLE);
                if (CHECK_RANGE (drawing::FillStyle_NONE, static_cast<drawing::FillStyle>(pFillStyle->GetValue ()), drawing::FillStyle_BITMAP))
                {
                    pAttr->ClearItem (XATTR_FILLSTYLE);
                    XFillStyleItem aStyleItem(static_cast<drawing::FillStyle>(pFillStyle->GetValue ()));
                    aStyleItem.SetWhich(XATTR_FILLSTYLE);
                    pAttr->Put (aStyleItem);
                    rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    rBindings.Invalidate (SID_ATTR_PAGE_FILLSTYLE);
                }
#if HAVE_FEATURE_SCRIPTING
                else StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        // determine new line style
        case SID_SETLINESTYLE :
            if (pArgs && pArgs->Count () == 1)
            {
                const SfxUInt32Item* pLineStyle = rReq.GetArg<SfxUInt32Item>(ID_VAL_STYLE);
                if (CHECK_RANGE (sal_Int32(drawing::LineStyle_NONE), static_cast<sal_Int32>(pLineStyle->GetValue()), sal_Int32(drawing::LineStyle_DASH)))
                {
                    pAttr->ClearItem (XATTR_LINESTYLE);
                    XLineStyleItem aStyleItem(static_cast<drawing::LineStyle>(pLineStyle->GetValue()));
                    aStyleItem.SetWhich(XATTR_LINESTYLE);
                    pAttr->Put(aStyleItem);
                    rBindings.Invalidate (SID_ATTR_LINE_STYLE);
                }
#if HAVE_FEATURE_SCRIPTING
                else StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        // set line width
        case SID_SETLINEWIDTH :
            if (pArgs && pArgs->Count () == 1)
            {
                const SfxUInt32Item* pLineWidth = rReq.GetArg<SfxUInt32Item>(ID_VAL_WIDTH);
                pAttr->ClearItem (XATTR_LINEWIDTH);
                XLineWidthItem aWidthItem(pLineWidth->GetValue());
                aWidthItem.SetWhich(XATTR_LINEWIDTH);
                pAttr->Put(aWidthItem);
                rBindings.Invalidate (SID_ATTR_LINE_WIDTH);
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        case SID_SETFILLCOLOR :
            if (pArgs && pArgs->Count () == 3)
            {
                const SfxUInt32Item* pRed = rReq.GetArg<SfxUInt32Item>(ID_VAL_RED);
                const SfxUInt32Item* pGreen = rReq.GetArg<SfxUInt32Item>(ID_VAL_GREEN);
                const SfxUInt32Item* pBlue = rReq.GetArg<SfxUInt32Item>(ID_VAL_BLUE);

                pAttr->ClearItem (XATTR_FILLCOLOR);
                pAttr->ClearItem (XATTR_FILLSTYLE);
                XFillColorItem aColorItem(-1, Color (static_cast<sal_uInt8>(pRed->GetValue ()),
                                                       static_cast<sal_uInt8>(pGreen->GetValue ()),
                                                       static_cast<sal_uInt8>(pBlue->GetValue ())));
                aColorItem.SetWhich(XATTR_FILLCOLOR);
                pAttr->Put(aColorItem);
                rBindings.Invalidate (SID_ATTR_FILL_COLOR);
                rBindings.Invalidate (SID_ATTR_PAGE_COLOR);
                rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                rBindings.Invalidate (SID_ATTR_PAGE_FILLSTYLE);
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        case SID_SETLINECOLOR :
            if (pArgs && pArgs->Count () == 3)
            {
                const SfxUInt32Item* pRed = rReq.GetArg<SfxUInt32Item>(ID_VAL_RED);
                const SfxUInt32Item* pGreen = rReq.GetArg<SfxUInt32Item>(ID_VAL_GREEN);
                const SfxUInt32Item* pBlue = rReq.GetArg<SfxUInt32Item>(ID_VAL_BLUE);

                pAttr->ClearItem (XATTR_LINECOLOR);
                XLineColorItem aColorItem(-1, Color(static_cast<sal_uInt8>(pRed->GetValue()),
                                                    static_cast<sal_uInt8>(pGreen->GetValue()),
                                                    static_cast<sal_uInt8>(pBlue->GetValue())));
                aColorItem.SetWhich(XATTR_LINECOLOR);
                pAttr->Put(aColorItem);
                rBindings.Invalidate (SID_ATTR_LINE_COLOR);
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        case SID_SETGRADSTARTCOLOR :
        case SID_SETGRADENDCOLOR :
            if (pArgs && pArgs->Count () == 4)
            {
                const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);
                const SfxUInt32Item* pRed = rReq.GetArg<SfxUInt32Item>(ID_VAL_RED);
                const SfxUInt32Item* pGreen = rReq.GetArg<SfxUInt32Item>(ID_VAL_GREEN);
                const SfxUInt32Item* pBlue = rReq.GetArg<SfxUInt32Item>(ID_VAL_BLUE);

                XGradientListRef pGradientList = GetDoc()->GetGradientList ();
                long          nCounts        = pGradientList->Count ();
                Color         aColor (static_cast<sal_uInt8>(pRed->GetValue ()),
                                      static_cast<sal_uInt8>(pGreen->GetValue ()),
                                      static_cast<sal_uInt8>(pBlue->GetValue ()));
                long i;

                pAttr->ClearItem (XATTR_FILLGRADIENT);
                pAttr->ClearItem (XATTR_FILLSTYLE);

                for ( i = 0; i < nCounts; i ++)
                {
                    const XGradientEntry* pEntry = pGradientList->GetGradient(i);

                    if (pEntry->GetName () == pName->GetValue ())
                    {
                        XGradient aGradient(pEntry->GetGradient());

                        if (rReq.GetSlot () == SID_SETGRADSTARTCOLOR) aGradient.SetStartColor (aColor);
                        else aGradient.SetEndColor (aColor);

                        XFillStyleItem aStyleItem(drawing::FillStyle_GRADIENT);
                        aStyleItem.SetWhich(XATTR_FILLSTYLE);
                        pAttr->Put(aStyleItem);
                        XFillGradientItem aGradientItem(pName->GetValue (), aGradient);
                        aGradientItem.SetWhich(XATTR_FILLGRADIENT);
                        pAttr->Put(aGradientItem);
                        break;
                    }
                }

                if (i >= nCounts)
                {
                    Color aBlack (0, 0, 0);
                    XGradient aGradient ((rReq.GetSlot () == SID_SETGRADSTARTCOLOR)
                                             ? aColor
                                             : aBlack,
                                         (rReq.GetSlot () == SID_SETGRADENDCOLOR)
                                             ? aColor
                                             : aBlack);

                    GetDoc()->GetGradientList()->Insert(std::make_unique<XGradientEntry>(aGradient, pName->GetValue()));

                    XFillStyleItem aStyleItem(drawing::FillStyle_GRADIENT);
                    aStyleItem.SetWhich(XATTR_FILLSTYLE);
                    pAttr->Put(aStyleItem);
                    XFillGradientItem aGradientItem(pName->GetValue(), aGradient);
                    aGradientItem.SetWhich(XATTR_FILLGRADIENT);
                    pAttr->Put(aGradientItem);
                }

                rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                rBindings.Invalidate (SID_ATTR_PAGE_FILLSTYLE);
                rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
                rBindings.Invalidate (SID_ATTR_PAGE_GRADIENT);
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        case SID_SETHATCHCOLOR :
            if (pArgs && pArgs->Count () == 4)
            {
                const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);
                const SfxUInt32Item* pRed = rReq.GetArg<SfxUInt32Item>(ID_VAL_RED);
                const SfxUInt32Item* pGreen = rReq.GetArg<SfxUInt32Item>(ID_VAL_GREEN);
                const SfxUInt32Item* pBlue = rReq.GetArg<SfxUInt32Item>(ID_VAL_BLUE);

                XHatchListRef pHatchList = GetDoc()->GetHatchList ();
                long       nCounts     = pHatchList->Count ();
                Color      aColor (static_cast<sal_uInt8>(pRed->GetValue ()),
                                   static_cast<sal_uInt8>(pGreen->GetValue ()),
                                   static_cast<sal_uInt8>(pBlue->GetValue ()));
                long i;

                pAttr->ClearItem (XATTR_FILLHATCH);
                pAttr->ClearItem (XATTR_FILLSTYLE);

                for ( i = 0; i < nCounts; i ++)
                {
                    const XHatchEntry* pEntry = pHatchList->GetHatch(i);

                    if (pEntry->GetName () == pName->GetValue ())
                    {
                        XHatch aHatch(pEntry->GetHatch());

                        aHatch.SetColor (aColor);

                        XFillStyleItem aStyleItem(drawing::FillStyle_HATCH);
                        aStyleItem.SetWhich(XATTR_FILLSTYLE);
                        pAttr->Put(aStyleItem);
                        XFillHatchItem aHatchItem(pName->GetValue(), aHatch);
                        aHatchItem.SetWhich(XATTR_FILLHATCH);
                        pAttr->Put(aHatchItem);
                        break;
                    }
                }

                if (i >= nCounts)
                {
                    XHatch aHatch (aColor);

                    GetDoc()->GetHatchList()->Insert(std::make_unique<XHatchEntry>(aHatch, pName->GetValue()));

                    XFillStyleItem aStyleItem(drawing::FillStyle_HATCH);
                    aStyleItem.SetWhich(XATTR_FILLSTYLE);
                    pAttr->Put(aStyleItem);
                    XFillHatchItem aHatchItem(pName->GetValue (), aHatch);
                    aHatchItem.SetWhich(XATTR_FILLHATCH);
                    pAttr->Put(aHatchItem);
                }

                rBindings.Invalidate (SID_ATTR_FILL_HATCH);
                rBindings.Invalidate (SID_ATTR_PAGE_HATCH);
                rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                rBindings.Invalidate (SID_ATTR_PAGE_FILLSTYLE);
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        // configuration for line-dash
        case SID_DASH :
            if (pArgs && pArgs->Count () == 7)
            {
                const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);
                const SfxUInt32Item* pStyle = rReq.GetArg<SfxUInt32Item>(ID_VAL_STYLE);
                const SfxUInt32Item* pDots = rReq.GetArg<SfxUInt32Item>(ID_VAL_DOTS);
                const SfxUInt32Item* pDotLen = rReq.GetArg<SfxUInt32Item>(ID_VAL_DOTLEN);
                const SfxUInt32Item* pDashes = rReq.GetArg<SfxUInt32Item>(ID_VAL_DASHES);
                const SfxUInt32Item* pDashLen = rReq.GetArg<SfxUInt32Item>(ID_VAL_DASHLEN);
                const SfxUInt32Item* pDistance = rReq.GetArg<SfxUInt32Item>(ID_VAL_DISTANCE);

                if (CHECK_RANGE (sal_Int32(css::drawing::DashStyle_RECT), static_cast<sal_Int32>(pStyle->GetValue()), sal_Int32(css::drawing::DashStyle_ROUNDRELATIVE)))
                {
                    XDash aNewDash (static_cast<css::drawing::DashStyle>(pStyle->GetValue ()), static_cast<short>(pDots->GetValue ()), pDotLen->GetValue (),
                                    static_cast<short>(pDashes->GetValue ()), pDashLen->GetValue (), pDistance->GetValue ());

                    pAttr->ClearItem (XATTR_LINEDASH);
                    pAttr->ClearItem (XATTR_LINESTYLE);

                    XDashListRef pDashList = GetDoc()->GetDashList();
                    long       nCounts    = pDashList->Count ();
                    std::unique_ptr<XDashEntry> pEntry = std::make_unique<XDashEntry>(aNewDash, pName->GetValue());
                    long i;

                    for ( i = 0; i < nCounts; i++ )
                        if (pDashList->GetDash (i)->GetName () == pName->GetValue ())
                            break;

                    if (i < nCounts)
                        pDashList->Replace(std::move(pEntry), i);
                    else
                        pDashList->Insert(std::move(pEntry));

                    XLineDashItem aDashItem(pName->GetValue(), aNewDash);
                    aDashItem.SetWhich(XATTR_LINEDASH);
                    pAttr->Put(aDashItem);
                    XLineStyleItem aStyleItem(drawing::LineStyle_DASH);
                    aStyleItem.SetWhich(XATTR_LINESTYLE);
                    pAttr->Put(aStyleItem);
                    rBindings.Invalidate (SID_ATTR_LINE_DASH);
                    rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                }
#if HAVE_FEATURE_SCRIPTING
                else StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        // configuration for gradients
        case SID_GRADIENT :
            if (pArgs && pArgs->Count () == 8)
            {
                const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);
                const SfxUInt32Item* pStyle = rReq.GetArg<SfxUInt32Item>(ID_VAL_STYLE);
                const SfxUInt32Item* pAngle = rReq.GetArg<SfxUInt32Item>(ID_VAL_ANGLE);
                const SfxUInt32Item* pBorder = rReq.GetArg<SfxUInt32Item>(ID_VAL_BORDER);
                const SfxUInt32Item* pCenterX = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_X);
                const SfxUInt32Item* pCenterY = rReq.GetArg<SfxUInt32Item>(ID_VAL_CENTER_Y);
                const SfxUInt32Item* pStart = rReq.GetArg<SfxUInt32Item>(ID_VAL_STARTINTENS);
                const SfxUInt32Item* pEnd = rReq.GetArg<SfxUInt32Item>(ID_VAL_ENDINTENS);

                if (CHECK_RANGE (sal_Int32(css::awt::GradientStyle_LINEAR), static_cast<sal_Int32>(pStyle->GetValue()), sal_Int32(css::awt::GradientStyle_RECT)) &&
                    CHECK_RANGE (0, static_cast<sal_Int32>(pAngle->GetValue ()), 360) &&
                    CHECK_RANGE (0, static_cast<sal_Int32>(pBorder->GetValue ()), 100) &&
                    CHECK_RANGE (0, static_cast<sal_Int32>(pCenterX->GetValue ()), 100) &&
                    CHECK_RANGE (0, static_cast<sal_Int32>(pCenterY->GetValue ()), 100) &&
                    CHECK_RANGE (0, static_cast<sal_Int32>(pStart->GetValue ()), 100) &&
                    CHECK_RANGE (0, static_cast<sal_Int32>(pEnd->GetValue ()), 100))
                {
                    pAttr->ClearItem (XATTR_FILLGRADIENT);
                    pAttr->ClearItem (XATTR_FILLSTYLE);

                    XGradientListRef pGradientList = GetDoc()->GetGradientList ();
                    long           nCounts        = pGradientList->Count ();
                    long i;

                    for ( i = 0; i < nCounts; i++ )
                    {
                        const XGradientEntry* pEntry = pGradientList->GetGradient(i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            XGradient aGradient(pEntry->GetGradient());

                            aGradient.SetGradientStyle (static_cast<css::awt::GradientStyle>(pStyle->GetValue ()));
                            aGradient.SetAngle (pAngle->GetValue () * 10);
                            aGradient.SetBorder (static_cast<short>(pBorder->GetValue ()));
                            aGradient.SetXOffset (static_cast<short>(pCenterX->GetValue ()));
                            aGradient.SetYOffset (static_cast<short>(pCenterY->GetValue ()));
                            aGradient.SetStartIntens (static_cast<short>(pStart->GetValue ()));
                            aGradient.SetEndIntens (static_cast<short>(pEnd->GetValue ()));

                            XFillStyleItem aStyleItem(drawing::FillStyle_GRADIENT);
                            aStyleItem.SetWhich(XATTR_FILLSTYLE);
                            pAttr->Put(aStyleItem);
                            XFillGradientItem aGradientItem(pName->GetValue (), aGradient);
                            aGradientItem.SetWhich(XATTR_FILLGRADIENT);
                            pAttr->Put(aGradientItem);
                            break;
                        }
                    }

                    if (i >= nCounts)
                    {
                        Color aBlack (0, 0, 0);
                        XGradient aGradient (aBlack, aBlack, static_cast<css::awt::GradientStyle>(pStyle->GetValue ()),
                                             pAngle->GetValue () * 10, static_cast<short>(pCenterX->GetValue ()),
                                             static_cast<short>(pCenterY->GetValue ()), static_cast<short>(pBorder->GetValue ()),
                                             static_cast<short>(pStart->GetValue ()), static_cast<short>(pEnd->GetValue ()));

                        pGradientList->Insert(std::make_unique<XGradientEntry>(aGradient, pName->GetValue()));
                        XFillStyleItem aStyleItem(drawing::FillStyle_GRADIENT);
                        aStyleItem.SetWhich(XATTR_FILLSTYLE);
                        pAttr->Put(aStyleItem);
                        XFillGradientItem aGradientItem(pName->GetValue (), aGradient);
                        aGradientItem.SetWhich(XATTR_FILLGRADIENT);
                        pAttr->Put(aGradientItem);
                    }

                    rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
                    rBindings.Invalidate (SID_ATTR_PAGE_GRADIENT);
                    rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                    rBindings.Invalidate (SID_ATTR_PAGE_FILLSTYLE);
                }
#if HAVE_FEATURE_SCRIPTING
                else StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        // configuration for hatch
        case SID_HATCH :
            if (pArgs && pArgs->Count () == 4)
            {
                const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);
                const SfxUInt32Item* pStyle = rReq.GetArg<SfxUInt32Item>(ID_VAL_STYLE);
                const SfxUInt32Item* pDistance = rReq.GetArg<SfxUInt32Item>(ID_VAL_DISTANCE);
                const SfxUInt32Item* pAngle = rReq.GetArg<SfxUInt32Item>(ID_VAL_ANGLE);

                if (CHECK_RANGE (sal_Int32(css::drawing::HatchStyle_SINGLE), static_cast<sal_Int32>(pStyle->GetValue()), sal_Int32(css::drawing::HatchStyle_TRIPLE)) &&
                    CHECK_RANGE (0, static_cast<sal_Int32>(pAngle->GetValue ()), 360))
                {
                    pAttr->ClearItem (XATTR_FILLHATCH);
                    pAttr->ClearItem (XATTR_FILLSTYLE);

                    XHatchListRef pHatchList = GetDoc()->GetHatchList ();
                    long       nCounts     = pHatchList->Count ();
                    long i;

                    for ( i = 0; i < nCounts; i++ )
                    {
                        const XHatchEntry* pEntry = pHatchList->GetHatch(i);

                        if (pEntry->GetName () == pName->GetValue ())
                        {
                            XHatch aHatch(pEntry->GetHatch());

                            aHatch.SetHatchStyle (static_cast<css::drawing::HatchStyle>(pStyle->GetValue ()));
                            aHatch.SetDistance (pDistance->GetValue ());
                            aHatch.SetAngle (pAngle->GetValue () * 10);

                            XFillStyleItem aStyleItem(drawing::FillStyle_HATCH);
                            aStyleItem.SetWhich(XATTR_FILLSTYLE);
                            pAttr->Put(aStyleItem);
                            XFillHatchItem aHatchItem(pName->GetValue (), aHatch);
                            aHatchItem.SetWhich(XATTR_FILLHATCH);
                            pAttr->Put(aHatchItem);
                            break;
                        }
                    }

                    if (i >= nCounts)
                    {
                        XHatch aHatch (Color(0), static_cast<css::drawing::HatchStyle>(pStyle->GetValue ()), pDistance->GetValue (),
                                       pAngle->GetValue () * 10);

                        pHatchList->Insert(std::make_unique<XHatchEntry>(aHatch, pName->GetValue()));
                        XFillStyleItem aStyleItem(drawing::FillStyle_HATCH);
                        aStyleItem.SetWhich(XATTR_FILLSTYLE);
                        pAttr->Put(aStyleItem);
                        XFillHatchItem aHatchItem(pName->GetValue (), aHatch);
                        aHatchItem.SetWhich(XATTR_FILLHATCH);
                        pAttr->Put(aHatchItem);
                    }

                    rBindings.Invalidate (SID_ATTR_FILL_HATCH);
                    rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                }
#if HAVE_FEATURE_SCRIPTING
                else StarBASIC::FatalError (ERRCODE_BASIC_BAD_PROP_VALUE);
#endif
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        case SID_SELECTGRADIENT :
            if (pArgs && (pArgs->Count () == 1))
            {
                const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);

                XGradientListRef pGradientList = GetDoc()->GetGradientList ();
                long           nCounts        = pGradientList->Count ();

                for (long i = 0; i < nCounts; i ++)
                {
                    const XGradientEntry* pEntry = pGradientList->GetGradient(i);

                    if (pEntry->GetName () == pName->GetValue ())
                    {
                        pAttr->ClearItem (XATTR_FILLGRADIENT);
                        pAttr->ClearItem (XATTR_FILLSTYLE);
                        XFillStyleItem aStyleItem(drawing::FillStyle_GRADIENT);
                        aStyleItem.SetWhich(XATTR_FILLSTYLE);
                        pAttr->Put(aStyleItem);
                        XFillGradientItem aGradientItem(pName->GetValue (), pEntry->GetGradient ());
                        aGradientItem.SetWhich(XATTR_FILLGRADIENT);
                        pAttr->Put(aGradientItem);
                        rBindings.Invalidate (SID_ATTR_FILL_GRADIENT);
                        rBindings.Invalidate (SID_ATTR_PAGE_GRADIENT);
                        rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                        rBindings.Invalidate (SID_ATTR_PAGE_FILLSTYLE);
                        break;
                    }
                }

                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        case SID_SELECTHATCH :
            if (pArgs && pArgs->Count () == 1)
            {
                const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);

                XHatchListRef pHatchList = GetDoc()->GetHatchList ();
                long       nCounts     = pHatchList->Count ();

                for (long i = 0; i < nCounts; i ++)
                {
                    const XHatchEntry* pEntry = pHatchList->GetHatch(i);

                    if (pEntry->GetName () == pName->GetValue ())
                    {
                        pAttr->ClearItem (XATTR_FILLHATCH);
                        pAttr->ClearItem (XATTR_FILLSTYLE);
                        XFillStyleItem aStyleItem(drawing::FillStyle_HATCH);
                        aStyleItem.SetWhich(XATTR_FILLSTYLE);
                        pAttr->Put(aStyleItem);
                        XFillHatchItem aHatchItem(pName->GetValue (), pEntry->GetHatch ());
                        aHatchItem.SetWhich(XATTR_FILLHATCH);
                        pAttr->Put(aHatchItem);

                        rBindings.Invalidate (SID_ATTR_FILL_HATCH);
                        rBindings.Invalidate (SID_ATTR_PAGE_HATCH);
                        rBindings.Invalidate (SID_ATTR_FILL_STYLE);
                        rBindings.Invalidate (SID_ATTR_PAGE_FILLSTYLE);
                        break;
                    }
                }

                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

        case SID_UNSELECT :
            mpDrawView->UnmarkAll ();
            break;

        case SID_GETRED :
            if (pArgs && pArgs->Count () == 1)
            {
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            break;

/*        case SID_SETFONTFAMILYNAME :
        case SID_SETFONTSTYLENAME :
        case SID_SETFONTFAMILY :
        case SID_SETFONTPITCH :
        case SID_SETFONTCHARSET :
        case SID_SETFONTPOSTURE :
        case SID_SETFONTWEIGHT :
        case SID_SETFONTUNDERLINE :
        case SID_SETFONTCROSSEDOUT :
        case SID_SETFONTSHADOWED :
        case SID_SETFONTCONTOUR :
        case SID_SETFONTCOLOR :
        case SID_SETFONTLANGUAGE :
        case SID_SETFONTWORDLINE :
        case SID_SETFONTCASEMAP :
        case SID_SETFONTESCAPE :
        case SID_SETFONTKERNING :
            break;*/

        default :
            ;
    }

    mpDrawView->SetAttributes (*const_cast<const SfxItemSet *>(pAttr.get()));
    rReq.Ignore ();
}

/**
 * Edit macros for attribute configuration
 */
void DrawViewShell::AttrState (SfxItemSet& rSet)
{
    SfxWhichIter     aIter (rSet);
    sal_uInt16           nWhich = aIter.FirstWhich ();
    SfxItemSet aAttr( GetDoc()->GetPool() );
    mpDrawView->GetAttributes( aAttr );

    while (nWhich)
    {
        switch (nWhich)
        {
            case SID_GETFILLSTYLE :
            {
                const XFillStyleItem &rFillStyleItem = aAttr.Get (XATTR_FILLSTYLE);

                rSet.Put (SfxUInt32Item (nWhich, static_cast<long>(rFillStyleItem.GetValue ())));
                break;
            }

            case SID_GETLINESTYLE :
            {
                const XLineStyleItem &rLineStyleItem = aAttr.Get (XATTR_LINESTYLE);

                rSet.Put (SfxUInt32Item (nWhich, static_cast<long>(rLineStyleItem.GetValue ())));
                break;
            }

            case SID_GETLINEWIDTH :
            {
                const XLineWidthItem &rLineWidthItem = aAttr.Get (XATTR_LINEWIDTH);

                rSet.Put (SfxUInt32Item (nWhich, static_cast<long>(rLineWidthItem.GetValue ())));
                break;
            }

            case SID_GETGREEN :
            case SID_GETRED :
            case SID_GETBLUE :
            {
                const SfxUInt32Item &rWhatKind = static_cast<const SfxUInt32Item &>( rSet.Get (ID_VAL_WHATKIND) );
                Color               aColor;

                switch (rWhatKind.GetValue ())
                {
                    case 1 :
                    {
                        const XLineColorItem &rLineColorItem = aAttr.Get (XATTR_LINECOLOR);

                        aColor = rLineColorItem.GetColorValue ();
                        break;
                    }

                    case 2 :
                    {
                        const XFillColorItem &rFillColorItem = aAttr.Get (XATTR_FILLCOLOR);

                        aColor = rFillColorItem.GetColorValue ();
                        break;
                    }

                    case 3 :
                    case 4 :
                    {
                        const XFillGradientItem &rFillGradientItem = aAttr.Get (XATTR_FILLGRADIENT);
                        const XGradient         &rGradient         = rFillGradientItem.GetGradientValue ();

                        aColor = (rWhatKind.GetValue () == 3)
                                    ? rGradient.GetStartColor ()
                                    : rGradient.GetEndColor ();
                        break;
                    }

                    case 5:
                    {
                        const XFillHatchItem &rFillHatchItem = aAttr.Get (XATTR_FILLHATCH);
                        const XHatch         &rHatch         = rFillHatchItem.GetHatchValue ();

                        aColor = rHatch.GetColor ();
                        break;
                    }

                    default :
                        ;
                }

                rSet.Put (SfxUInt32Item (nWhich, static_cast<long>((nWhich == SID_GETRED)
                                                             ? aColor.GetRed ()
                                                             : (nWhich == SID_GETGREEN)
                                                                   ? aColor.GetGreen ()
                                                                   : aColor.GetBlue ())));
                break;
            }

            default :
                ;
        }

        nWhich = aIter.NextWhich ();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
