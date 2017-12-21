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
#include <o3tl/make_unique.hxx>

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
        aPageSize.Width() -= pPage->GetLeftBorder() + pPage->GetRightBorder();
        aPageSize.Height() -= pPage->GetUpperBorder() + pPage->GetLowerBorder();

        // If the image is too large we make it fit into the page
        if ( ( ( aSize.Height() > aPageSize.Height() ) || ( aSize.Width()   > aPageSize.Width() ) ) &&
            aSize.Height() && aPageSize.Height() )
        {
            float fGrfWH =  (float)aSize.Width() /
                            (float)aSize.Height();
            float fWinWH =  (float)aPageSize.Width() /
                            (float)aPageSize.Height();

            // constrain size to page size if necessary
            if ((fGrfWH != 0.F) && (fGrfWH < fWinWH))
            {
                aSize.Width() = (long)(aPageSize.Height() * fGrfWH);
                aSize.Height()= aPageSize.Height();
            }
            else
            {
                aSize.Width() = aPageSize.Width();
                aSize.Height()= (long)(aPageSize.Width() / fGrfWH);
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

                        SdrGrafObj* pNewGrafObj = pGrafObj->Clone();
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
            pGrafObj = new SdrGrafObj(aGraphic, aRect);
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
        // configuration for line-dash
        case SID_DASH :
            if (pArgs)
                if (pArgs->Count () == 7)
                {
                    const SfxStringItem* pName = rReq.GetArg<SfxStringItem>(ID_VAL_INDEX);
                    const SfxUInt32Item* pStyle = rReq.GetArg<SfxUInt32Item>(ID_VAL_STYLE);
                    const SfxUInt32Item* pDots = rReq.GetArg<SfxUInt32Item>(ID_VAL_DOTS);
                    const SfxUInt32Item* pDotLen = rReq.GetArg<SfxUInt32Item>(ID_VAL_DOTLEN);
                    const SfxUInt32Item* pDashes = rReq.GetArg<SfxUInt32Item>(ID_VAL_DASHES);
                    const SfxUInt32Item* pDashLen = rReq.GetArg<SfxUInt32Item>(ID_VAL_DASHLEN);
                    const SfxUInt32Item* pDistance = rReq.GetArg<SfxUInt32Item>(ID_VAL_DISTANCE);

                    if (CHECK_RANGE ((sal_Int32)css::drawing::DashStyle_RECT, (sal_Int32)pStyle->GetValue(), (sal_Int32)css::drawing::DashStyle_ROUNDRELATIVE))
                    {
                        XDash aNewDash ((css::drawing::DashStyle) pStyle->GetValue (), (short) pDots->GetValue (), pDotLen->GetValue (),
                                        (short) pDashes->GetValue (), pDashLen->GetValue (), pDistance->GetValue ());

                        pAttr->ClearItem (XATTR_LINEDASH);
                        pAttr->ClearItem (XATTR_LINESTYLE);

                        XDashListRef pDashList = GetDoc()->GetDashList();
                        long       nCounts    = pDashList->Count ();
                        std::unique_ptr<XDashEntry> pEntry = o3tl::make_unique<XDashEntry>(aNewDash, pName->GetValue());
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
void DrawViewShell::AttrState (SfxItemSet& /*rSet*/)
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
