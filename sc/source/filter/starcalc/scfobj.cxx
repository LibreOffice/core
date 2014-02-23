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

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>


using namespace com::sun::star;

#include <unotools/moduleoptions.hxx>
#include <svx/charthelper.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/objsh.hxx>
#include <sot/storage.hxx>
#include <sfx2/app.hxx>
#include <comphelper/classids.hxx>
#include "address.hxx"

#include "scfobj.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"


// STATIC DATA -----------------------------------------------------------




void Sc10InsertObject::InsertChart( ScDocument* pDoc, SCTAB nDestTab, const Rectangle& rRect,
                                SCTAB nSrcTab, sal_uInt16 nX1, sal_uInt16 nY1, sal_uInt16 nX2, sal_uInt16 nY2 )
{
    //  wenn Chart nicht installiert ist, darf nicht auf SCH_MOD zugegriffen werden!
    if ( !SvtModuleOptions().IsChart() )
        return;

    OUString aName;
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
            OSL_ENSURE(pModel,"Draw Layer ?");
        }

        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nDestTab));
        OSL_ENSURE(pPage,"Page ?");
        pPage->InsertObject(pSdrOle2Obj);

        pSdrOle2Obj->SetLogicRect(rRect);               // erst nach InsertObject !!!
        awt::Size aSz;
        aSz.Width = rRect.GetSize().Width();
        aSz.Height = rRect.GetSize().Height();
        xObj->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, aSz );

        // #i121334# This call will change the chart's default background fill from white to transparent.
        // Add here again if this is wanted (see task description for details)
        // ChartHelper::AdaptDefaultsForChart( xObj );

            // hier kann das Chart noch nicht mit Daten gefuettert werden,
            // weil die Formeln noch nicht berechnet sind.
            // Deshalb in die ChartCollection, die Daten werden dann im
            // Sc10Import dtor geholt.

        ScChartCollection* pColl = pDoc->GetChartCollection();
        pColl->push_back( new ScChartArray( pDoc, nSrcTab, static_cast<SCCOL>(nX1), static_cast<SCROW>(nY1), static_cast<SCCOL>(nX2), static_cast<SCROW>(nY2), aName ) );
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
