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

#include "XMLTableShapeResizer.hxx"
#include "unonames.hxx"
#include "document.hxx"
#include "xmlimprt.hxx"
#include "chartlis.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"
#include "compiler.hxx"
#include "reftokenhelper.hxx"

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <memory>
#include <vector>

using namespace ::com::sun::star;
using ::std::auto_ptr;
using ::std::vector;

ScMyOLEFixer::ScMyOLEFixer(ScXMLImport& rTempImport)
    : rImport(rTempImport),
    aShapes(),
    pCollection(NULL)
{
}

ScMyOLEFixer::~ScMyOLEFixer()
{
}

sal_Bool ScMyOLEFixer::IsOLE(uno::Reference< drawing::XShape >& rShape)
{
    return rShape->getShapeType() == "com.sun.star.drawing.OLE2Shape";
}

void ScMyOLEFixer::CreateChartListener(ScDocument* pDoc,
    const OUString& rName,
    const OUString& rRangeList)
{
    // This is the minimum required.
    if (!pDoc)
        return;

    if (rRangeList.isEmpty())
    {
        pDoc->AddOLEObjectToCollection(rName);
        return;
    }

    OUString aRangeStr;
    ScRangeStringConverter::GetStringFromXMLRangeString(aRangeStr, rRangeList, pDoc);
    if (aRangeStr.isEmpty())
    {
        pDoc->AddOLEObjectToCollection(rName);
        return;
    }

    if (!pCollection)
        pCollection = pDoc->GetChartListenerCollection();

    if (!pCollection)
        return;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    auto_ptr< vector<ScTokenRef> > pRefTokens(new vector<ScTokenRef>);
    SAL_WNODEPRECATED_DECLARATIONS_POP
        const sal_Unicode cSep = ScCompiler::GetNativeSymbolChar(ocSep);
    ScRefTokenHelper::compileRangeRepresentation(
        *pRefTokens, aRangeStr, pDoc, cSep, pDoc->GetGrammar());
    if (!pRefTokens->empty())
    {
        ScChartListener* pCL(new ScChartListener(rName, pDoc, pRefTokens.release()));

        //for loading binary files e.g.
        //if we have the flat filter we need to set the dirty flag thus the visible charts get repainted
        //otherwise the charts keep their first visual representation which was created at a moment where the calc itself was not loaded completely and is incorect therefor
        if( (rImport.getImportFlags() & IMPORT_ALL) == IMPORT_ALL )
            pCL->SetDirty( sal_True );
        else
        {
            // #i104899# If a formula cell is already dirty, further changes aren't propagated.
            // This can happen easily now that row heights aren't updated for all sheets.
            pDoc->InterpretDirtyCells( *pCL->GetRangeList() );
        }

        pCollection->insert( pCL );
        pCL->StartListeningTo();
    }
}

void ScMyOLEFixer::AddOLE(uno::Reference <drawing::XShape>& rShape,
       const OUString &rRangeList)
{
    ScMyToFixupOLE aShape;
    aShape.xShape.set(rShape);
    aShape.sRangeList = rRangeList;
    aShapes.push_back(aShape);
}

void ScMyOLEFixer::FixupOLEs()
{
    if (!aShapes.empty() && rImport.GetModel().is())
    {
        OUString sPersistName ("PersistName");
        ScMyToFixupOLEs::iterator aItr(aShapes.begin());
        ScMyToFixupOLEs::iterator aEndItr(aShapes.end());
        ScDocument* pDoc(rImport.GetDocument());

        ScXMLImport::MutexGuard aGuard(rImport);

        while (aItr != aEndItr)
        {
            // #i78086# also call CreateChartListener for invalid position (anchored to sheet)
            if (!IsOLE(aItr->xShape))
                OSL_FAIL("Only OLEs should be in here now");

            if (IsOLE(aItr->xShape))
            {
                uno::Reference < beans::XPropertySet > xShapeProps ( aItr->xShape, uno::UNO_QUERY );
                uno::Reference < beans::XPropertySetInfo > xShapeInfo(xShapeProps->getPropertySetInfo());

                OUString sName;
                if (pDoc && xShapeProps.is() && xShapeInfo.is() && xShapeInfo->hasPropertyByName(sPersistName) &&
                    (xShapeProps->getPropertyValue(sPersistName) >>= sName))
                    CreateChartListener(pDoc, sName, aItr->sRangeList);
            }
            aItr = aShapes.erase(aItr);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
