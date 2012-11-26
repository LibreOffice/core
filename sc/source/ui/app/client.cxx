/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------


#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/sotref.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdview.hxx>
#include <svx/svdograf.hxx>
#include <svtools/embedhlp.hxx>
#include <svx/svdlegacy.hxx>

#include "client.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

ScClient::ScClient( ScTabViewShell* pViewShell, Window* pDraw, SdrModel* pSdrModel, SdrOle2Obj* pObj ) :
    SfxInPlaceClient( pViewShell, pDraw, pObj->GetAspect() ),
    pModel( pSdrModel ),
    pGrafEdit( 0 )
{
    SetObject( pObj->GetObjRef() );
}

__EXPORT ScClient::~ScClient()
{
}

SdrOle2Obj* ScClient::GetDrawObj()
{
    uno::Reference < embed::XEmbeddedObject > xObj = GetObject();
    SdrOle2Obj* pOle2Obj = NULL;
    String aName = GetViewShell()->GetObjectShell()->GetEmbeddedObjectContainer().GetEmbeddedObjectName( xObj );

    const sal_uInt32 nPages(pModel->GetPageCount());
    for (sal_uInt32 nPNr=0; nPNr<nPages && !pOle2Obj; nPNr++)
    {
        SdrPage* pPage = pModel->GetPage(nPNr);
        SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
        SdrObject* pObject = aIter.Next();
        while (pObject && !pOle2Obj)
        {
            if ( pObject->GetObjIdentifier() == OBJ_OLE2 )
            {
                // name from InfoObject is PersistName
                if ( ((SdrOle2Obj*)pObject)->GetPersistName() == aName )
                    pOle2Obj = (SdrOle2Obj*)pObject;
            }
            pObject = aIter.Next();
        }
    }
    return pOle2Obj;
}

void __EXPORT ScClient::RequestNewObjectArea( Rectangle& aLogicRect )
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell* >( pSfxViewSh );
    if (!pViewSh)
    {
        DBG_ERROR("Wrong ViewShell");
        return;
    }

    Rectangle aOldRect = GetObjArea();
    SdrOle2Obj*  pDrawObj = GetDrawObj();
    if ( pDrawObj )
    {
        if ( pDrawObj->IsResizeProtect() )
            aLogicRect.SetSize( aOldRect.GetSize() );

        if ( pDrawObj->IsMoveProtect() )
            aLogicRect.SetPos( aOldRect.TopLeft() );
    }

    sal_uInt16 nTab = pViewSh->GetViewData()->GetTabNo();
    SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(static_cast<sal_Int16>(nTab)));
    if ( pPage && aLogicRect != aOldRect )
    {
        basegfx::B2DPoint aPos(0.0, 0.0);
        basegfx::B2DVector aSize(pPage->GetPageScale());

        if ( aSize.getX() < 0.0 )
        {
            aPos.setX( aSize.getX() + 1.0);     // negative
            aSize.setX(-aSize.getX());      // positive
        }

        basegfx::B2DRange aPageRange( aPos, aPos + aSize );
        basegfx::B2DPoint aTranslate(0.0, 0.0);
        basegfx::B2DRange aLogicRange(aLogicRect.Left(), aLogicRect.Top(), aLogicRect.Right(), aLogicRect.Bottom());

        if(aPageRange.getMaxX() < aLogicRange.getMaxX())
        {
            aTranslate.setX(aPageRange.getMaxX() - aLogicRange.getMaxX());
        }

        if(aPageRange.getMinX() > aLogicRange.getMinX())
        {
            aTranslate.setX(aPageRange.getMinX() - aLogicRange.getMinX());
        }

        if(aPageRange.getMaxY() < aLogicRange.getMaxY())
        {
            aTranslate.setY(aPageRange.getMaxY() - aLogicRange.getMaxY());
        }

        if(aPageRange.getMinY() > aLogicRange.getMinY())
        {
            aTranslate.setY(aPageRange.getMinY() - aLogicRange.getMinY());
        }

        if(!aTranslate.equalZero())
        {
            aLogicRange.transform(basegfx::tools::createTranslateB2DHomMatrix(aTranslate));
            aLogicRect = Rectangle(
                basegfx::fround(aLogicRange.getMinX()), basegfx::fround(aLogicRange.getMinY()),
                basegfx::fround(aLogicRange.getMaxX()), basegfx::fround(aLogicRange.getMaxY()));
        }
    }
}

void __EXPORT ScClient::ObjectAreaChanged()
{
    SfxViewShell* pSfxViewSh = GetViewShell();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell* >( pSfxViewSh );
    if (!pViewSh)
    {
        DBG_ERROR("Wrong ViewShell");
        return;
    }

    //  Position und Groesse ins Dokument uebernehmen
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        const Rectangle aScaledObjArea(GetScaledObjArea());
        basegfx::B2DRange aNewRange(aScaledObjArea.Left(), aScaledObjArea.Top(), aScaledObjArea.Right(), aScaledObjArea.Bottom());

        // #i118524# if sheared/rotated, center to non-rotated LogicRect
        pDrawObj->setSuppressSetVisAreaSize(true);

        if(pDrawObj->isRotatedOrSheared())
        {
            sdr::legacy::SetLogicRange(*pDrawObj, aNewRange);

            const SdrView* pView(pViewSh ? pViewSh->GetSdrView() : 0);
            const basegfx::B2DRange& rBoundRange(pDrawObj->getObjectRange(pView));
            const basegfx::B2DPoint aDelta(aNewRange.getCenter() - rBoundRange.getCenter());

            aNewRange.transform(
                basegfx::tools::createTranslateB2DHomMatrix(
                    aDelta.getX(),
                    aDelta.getY()));
        }

        sdr::legacy::SetLogicRange(*pDrawObj, aNewRange);
        pDrawObj->setSuppressSetVisAreaSize(false);

        //  set document modified (SdrModel::SetChanged is not used)
        // TODO/LATER: is there a reason that this code is not executed in Draw?
//        SfxViewShell* pSfxViewSh = GetViewShell();
//        ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell* >( pSfxViewSh );
        if (pViewSh)
            pViewSh->GetViewData()->GetDocShell()->SetDrawModified();
    }

    if (pDrawObj)
        pViewSh->ScrollToObject( pDrawObj );
}

void __EXPORT ScClient::ViewChanged()
{
    if ( GetAspect() == embed::Aspects::MSOLE_ICON )
    {
        // the iconified object seems not to need such a scaling handling
        // since the replacement image and the size a completely controlled by the container
        // TODO/LATER: when the icon exchange is implemented the scaling handling might be required again here

        return;
    }

    uno::Reference < embed::XEmbeddedObject > xObj = GetObject();

    // TODO/LEAN: working with Visual Area can switch object to running state
    awt::Size aSz;
    try {
        aSz = xObj->getVisualAreaSize( GetAspect() );
    } catch ( embed::NoVisualAreaSizeException& )
    {
        DBG_ERROR("The visual area size must be available!\n");
    }

    MapUnit aMapUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( GetAspect() ) );
    Size aVisSize = OutputDevice::LogicToLogic( Size( aSz.Width, aSz.Height ), aMapUnit, MAP_100TH_MM );

    //  Groesse ins Dokument uebernehmen
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        Rectangle aLogicRect(sdr::legacy::GetLogicRect(*pDrawObj));
        Fraction aFractX = GetScaleWidth();
        Fraction aFractY = GetScaleHeight();
        aFractX *= aVisSize.Width();
        aFractY *= aVisSize.Height();
        aVisSize = Size( (long) aFractX, (long) aFractY );      // skaliert fuer Draw-Model

        //  pClientData->SetObjArea vor pDrawObj->SetLogicRect, damit keine
        //  falschen Skalierungen ausgerechnet werden:
        //Rectangle aObjArea = aLogicRect;
        //aObjArea.SetSize( aVisSize );          // Dokument-Groesse vom Server
        //SetObjArea( aObjArea );

        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell* >( pSfxViewSh );
        if ( pViewSh )
        {
            Window* pWin = pViewSh->GetActiveWin();
            if ( pWin->LogicToPixel( aVisSize ) != pWin->LogicToPixel( aLogicRect.GetSize() ) )
            {
                aLogicRect.SetSize( aVisSize );
                sdr::legacy::SetLogicRect(*pDrawObj, aLogicRect );

                //  set document modified (SdrModel::SetChanged is not used)
                pViewSh->GetViewData()->GetDocShell()->SetDrawModified();
            }
        }
    }
}

void __EXPORT ScClient::MakeVisible()
{
    SdrOle2Obj* pDrawObj = GetDrawObj();
    if (pDrawObj)
    {
        SfxViewShell* pSfxViewSh = GetViewShell();
        ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell* >( pSfxViewSh );
        if (pViewSh)
            pViewSh->ScrollToObject( pDrawObj );
    }
}

