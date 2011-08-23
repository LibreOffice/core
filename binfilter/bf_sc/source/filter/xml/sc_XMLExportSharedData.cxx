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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------
#include "XMLExportSharedData.hxx"
#include "XMLExportIterator.hxx"

#include <tools/debug.hxx>
namespace binfilter {

using namespace ::com::sun::star;

ScMySharedData::ScMySharedData(const sal_Int32 nTempTableCount) :
    nLastColumns(nTempTableCount, 0),
    nLastRows(nTempTableCount, 0),
    pTableShapes(NULL),
    pDrawPages(NULL),
    pShapesContainer(NULL),
    pDetectiveObjContainer(NULL),
    nTableCount(nTempTableCount)
{
    pDetectiveObjContainer = new ScMyDetectiveObjContainer();
}

ScMySharedData::~ScMySharedData()
{
    if (pShapesContainer)
        delete pShapesContainer;
    if (pTableShapes)
        delete pTableShapes;
    if (pDrawPages)
        delete pDrawPages;
    if (pDetectiveObjContainer)
        delete pDetectiveObjContainer;
}

void ScMySharedData::SetLastColumn(const sal_Int32 nTable, const sal_Int32 nCol)
{
    if(nCol > nLastColumns[nTable]) nLastColumns[nTable] = nCol;
}

sal_Int32 ScMySharedData::GetLastColumn(const sal_Int32 nTable)
{
    return nLastColumns[nTable];
}

void ScMySharedData::SetLastRow(const sal_Int32 nTable, const sal_Int32 nRow)
{
    if(nRow > nLastRows[nTable]) nLastRows[nTable] = nRow;
}

sal_Int32 ScMySharedData::GetLastRow(const sal_Int32 nTable)
{
    return nLastRows[nTable];
}

void ScMySharedData::AddDrawPage(const ScMyDrawPage& aDrawPage, const sal_Int32 nTable)
{
    if (!pDrawPages)
        pDrawPages = new ScMyDrawPages(nTableCount, ScMyDrawPage());
    (*pDrawPages)[nTable] = aDrawPage;
}

void ScMySharedData::SetDrawPageHasForms(const sal_Int32 nTable, sal_Bool bHasForms)
{
    DBG_ASSERT(pDrawPages, "DrawPages not collected");
    if (pDrawPages)
        (*pDrawPages)[nTable].bHasForms = bHasForms;
}

uno::Reference<drawing::XDrawPage> ScMySharedData::GetDrawPage(const sal_Int32 nTable)
{
    DBG_ASSERT(pDrawPages, "DrawPages not collected");
    if (pDrawPages)
        return (*pDrawPages)[nTable].xDrawPage;
    else
        return uno::Reference<drawing::XDrawPage>();
}

sal_Bool ScMySharedData::HasForm(const sal_Int32 nTable, uno::Reference<drawing::XDrawPage>& xDrawPage)
{
    sal_Bool bResult(sal_False);
    if (pDrawPages)
    {
        if ((*pDrawPages)[nTable].bHasForms)
        {
            bResult = sal_True;
            xDrawPage = (*pDrawPages)[nTable].xDrawPage;
        }
    }
    return bResult;
}

void ScMySharedData::AddNewShape(const ScMyShape& aMyShape)
{
    if (!pShapesContainer)
        pShapesContainer = new ScMyShapesContainer();
    pShapesContainer->AddNewShape(aMyShape);
}

void ScMySharedData::SortShapesContainer()
{
    if (pShapesContainer)
        pShapesContainer->Sort();
}

sal_Bool ScMySharedData::HasShapes()
{
    return ((pShapesContainer && pShapesContainer->HasShapes()) ||
            (pTableShapes && !pTableShapes->empty()));
}

void ScMySharedData::AddTableShape(const sal_Int32 nTable, const uno::Reference<drawing::XShape>& xShape)
{
    if (!pTableShapes)
        pTableShapes = new ScMyTableShapes(nTableCount);
    (*pTableShapes)[nTable].push_back(xShape);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
