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
#include "precompiled_sc.hxx"


#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>


using namespace com::sun::star;

// INCLUDE ---------------------------------------------------------------

#include <unotools/moduleoptions.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/objsh.hxx>
#include <sot/storage.hxx>
#include <sfx2/app.hxx>
#include <sot/clsids.hxx>
#include "address.hxx"

#include "scfobj.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"


// STATIC DATA -----------------------------------------------------------


//==================================================================

void Sc10InsertObject::InsertChart( ScDocument* pDoc, SCTAB nDestTab, const Rectangle& rRect,
                                SCTAB nSrcTab, sal_uInt16 nX1, sal_uInt16 nY1, sal_uInt16 nX2, sal_uInt16 nY2 )
{
    //  wenn Chart nicht installiert ist, darf nicht auf SCH_MOD zugegriffen werden!
    if ( !SvtModuleOptions().IsChart() )
        return;

    ::rtl::OUString aName;
    uno::Reference < embed::XEmbeddedObject > xObj = pDoc->GetDocumentShell()->
            GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aName );
    if ( xObj.is() )
    {
        SdrOle2Obj* pSdrOle2Obj = new SdrOle2Obj( ::svt::EmbeddedObjectRef( xObj, embed::Aspects::MSOLE_CONTENT ), aName, rRect );

        ScDrawLayer* pModel = pDoc->GetDrawLayer();
        if (!pModel)
        {
            pDoc->InitDrawLayer();
            pModel = pDoc->GetDrawLayer();
            DBG_ASSERT(pModel,"Draw Layer ?");
        }

        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nDestTab));
        DBG_ASSERT(pPage,"Page ?");
        pPage->InsertObject(pSdrOle2Obj);

        pSdrOle2Obj->SetLogicRect(rRect);               // erst nach InsertObject !!!
        awt::Size aSz;
        aSz.Width = rRect.GetSize().Width();
        aSz.Height = rRect.GetSize().Height();
        xObj->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, aSz );

            // hier kann das Chart noch nicht mit Daten gefuettert werden,
            // weil die Formeln noch nicht berechnet sind.
            // Deshalb in die ChartCollection, die Daten werden dann im
            // Sc10Import dtor geholt.

        ScChartCollection* pColl = pDoc->GetChartCollection();
        pColl->Insert( new ScChartArray( pDoc, nSrcTab, static_cast<SCCOL>(nX1), static_cast<SCROW>(nY1), static_cast<SCCOL>(nX2), static_cast<SCROW>(nY2), aName ) );
    }
}



