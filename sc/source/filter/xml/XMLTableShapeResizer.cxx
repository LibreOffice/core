/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "XMLTableShapeResizer.hxx"
#include "unonames.hxx"
#include "document.hxx"
#include "xmlimprt.hxx"
#include "chartlis.hxx"
#include "XMLConverter.hxx"
#include "rangeutl.hxx"
#include "reftokenhelper.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <memory>
#include <vector>

using namespace ::com::sun::star;
using ::std::auto_ptr;
using ::std::vector;
using ::rtl::OUString;

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
    return rShape->getShapeType().equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("com.sun.star.drawing.OLE2Shape"));
}

void ScMyOLEFixer::CreateChartListener(ScDocument* pDoc,
    const rtl::OUString& rName,
    const rtl::OUString& rRangeList)
{
    // This is the minimum required.
    if (!pDoc)
        return;

    if (!rRangeList.getLength())
    {
        pDoc->AddOLEObjectToCollection(rName);
        return;
    }

    OUString aRangeStr;
    ScRangeStringConverter::GetStringFromXMLRangeString(aRangeStr, rRangeList, pDoc);
    if (!aRangeStr.getLength())
    {
        pDoc->AddOLEObjectToCollection(rName);
        return;
    }

    if (!pCollection)
        pCollection = pDoc->GetChartListenerCollection();

    if (!pCollection)
        return;

    auto_ptr< vector<ScTokenRef> > pRefTokens(new vector<ScTokenRef>);
    ScRefTokenHelper::compileRangeRepresentation(*pRefTokens, aRangeStr, pDoc);
    if (!pRefTokens->empty())
    {
        ScChartListener* pCL(new ScChartListener(rName, pDoc, pRefTokens.release()));

        //for loading binary files e.g.
        //if we have the flat filter we need to set the dirty flag thus the visible charts get repainted
        //otherwise the charts keep their first visual representation which was created at a moment where the calc itself was not loaded completly and is incorect therefor
        if( (rImport.getImportFlags() & IMPORT_ALL) == IMPORT_ALL )
            pCL->SetDirty( TRUE );
        else
        {
            // #i104899# If a formula cell is already dirty, further changes aren't propagated.
            // This can happen easily now that row heights aren't updated for all sheets.
            pDoc->InterpretDirtyCells( *pCL->GetRangeList() );
        }

        pCollection->Insert( pCL );
        pCL->StartListeningTo();
    }
}

void ScMyOLEFixer::AddOLE(uno::Reference <drawing::XShape>& rShape,
       const rtl::OUString &rRangeList)
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
        rtl::OUString sPersistName (RTL_CONSTASCII_USTRINGPARAM("PersistName"));
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

                rtl::OUString sName;
                if (pDoc && xShapeProps.is() && xShapeInfo.is() && xShapeInfo->hasPropertyByName(sPersistName) &&
                    (xShapeProps->getPropertyValue(sPersistName) >>= sName))
                    CreateChartListener(pDoc, sName, aItr->sRangeList);
            }
            aItr = aShapes.erase(aItr);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
