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

#include "XMLExportSharedData.hxx"
#include "XMLExportIterator.hxx"
#include <osl/diagnose.h>

using namespace com::sun::star;

ScMySharedData::ScMySharedData(const sal_Int32 nTempTableCount) :
    nLastColumns(nTempTableCount, 0),
    nLastRows(nTempTableCount, 0),
    pTableShapes(nullptr),
    pDrawPages(nullptr),
    pShapesContainer(nullptr),
    pDetectiveObjContainer(new ScMyDetectiveObjContainer()),
    pNoteShapes(nullptr),
    nTableCount(nTempTableCount)
{
}

ScMySharedData::~ScMySharedData()
{
    delete pShapesContainer;
    delete pTableShapes;
    delete pDrawPages;
    delete pDetectiveObjContainer;
    delete pNoteShapes;
}

void ScMySharedData::SetLastColumn(const sal_Int32 nTable, const sal_Int32 nCol)
{
    if(nCol > nLastColumns[nTable]) nLastColumns[nTable] = nCol;
}

sal_Int32 ScMySharedData::GetLastColumn(const sal_Int32 nTable) const
{
    return nLastColumns[nTable];
}

void ScMySharedData::SetLastRow(const sal_Int32 nTable, const sal_Int32 nRow)
{
    if(nRow > nLastRows[nTable]) nLastRows[nTable] = nRow;
}

sal_Int32 ScMySharedData::GetLastRow(const sal_Int32 nTable) const
{
    return nLastRows[nTable];
}

void ScMySharedData::AddDrawPage(const ScMyDrawPage& aDrawPage, const sal_Int32 nTable)
{
    if (!pDrawPages)
        pDrawPages = new ScMyDrawPages(nTableCount, ScMyDrawPage());
    (*pDrawPages)[nTable] = aDrawPage;
}

void ScMySharedData::SetDrawPageHasForms(const sal_Int32 nTable, bool bHasForms)
{
    OSL_ENSURE(pDrawPages, "DrawPages not collected");
    if (pDrawPages)
        (*pDrawPages)[nTable].bHasForms = bHasForms;
}

uno::Reference<drawing::XDrawPage> ScMySharedData::GetDrawPage(const sal_Int32 nTable)
{
    OSL_ENSURE(pDrawPages, "DrawPages not collected");
    if (pDrawPages)
        return (*pDrawPages)[nTable].xDrawPage;
    else
        return uno::Reference<drawing::XDrawPage>();
}

bool ScMySharedData::HasForm(const sal_Int32 nTable, uno::Reference<drawing::XDrawPage>& xDrawPage)
{
    bool bResult(false);
    if (pDrawPages)
    {
        if ((*pDrawPages)[nTable].bHasForms)
        {
            bResult = true;
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

bool ScMySharedData::HasShapes()
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

void ScMySharedData::AddNoteObj(const uno::Reference<drawing::XShape>& xShape, const ScAddress& rPos)
{
    if (!pNoteShapes)
        pNoteShapes = new ScMyNoteShapesContainer();
    ScMyNoteShape aNote;
    aNote.xShape = xShape;
    aNote.aPos = rPos;
    pNoteShapes->AddNewNote(aNote);
}

void ScMySharedData::SortNoteShapes()
{
    if (pNoteShapes)
        pNoteShapes->Sort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
