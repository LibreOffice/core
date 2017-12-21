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
