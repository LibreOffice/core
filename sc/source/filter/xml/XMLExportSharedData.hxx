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

#pragma once

#include <address.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <vector>
#include <list>
#include <memory>

namespace com::sun::star::drawing { class XDrawPage; }
namespace com::sun::star::drawing { class XShape; }

struct ScMyDrawPage
{
    css::uno::Reference<css::drawing::XDrawPage> xDrawPage;
    bool bHasForms;

    ScMyDrawPage() : bHasForms(false) {}
};

typedef std::list< css::uno::Reference<css::drawing::XShape> > ScMyTableXShapes;
typedef std::vector<ScMyTableXShapes> ScMyTableShapes;
typedef std::vector<ScMyDrawPage> ScMyDrawPages;

class ScMyShapesContainer;
class ScMyDetectiveObjContainer;
struct ScMyShape;
class ScMyNoteShapesContainer;

class ScMySharedData
{
    std::vector<sal_Int32>      nLastColumns;
    std::vector<sal_Int32>      nLastRows;
    std::unique_ptr<ScMyTableShapes>            pTableShapes;
    std::unique_ptr<ScMyDrawPages>              pDrawPages;
    std::unique_ptr<ScMyShapesContainer>        pShapesContainer;
    std::unique_ptr<ScMyDetectiveObjContainer>  pDetectiveObjContainer;
    std::unique_ptr<ScMyNoteShapesContainer>    pNoteShapes;
    sal_Int32                   nTableCount;
public:
    explicit ScMySharedData(const sal_Int32 nTableCount);
    ~ScMySharedData();

    void SetLastColumn(const sal_Int32 nTable, const sal_Int32 nCol);
    void SetLastRow(const sal_Int32 nTable, const sal_Int32 nRow);
    sal_Int32 GetLastColumn(const sal_Int32 nTable) const;
    sal_Int32 GetLastRow(const sal_Int32 nTable) const;
    void AddDrawPage(const ScMyDrawPage& aDrawPage, const sal_Int32 nTable);
    void SetDrawPageHasForms(const sal_Int32 nTable, bool bHasForms);
    css::uno::Reference<css::drawing::XDrawPage> GetDrawPage(const sal_Int32 nTable);
    bool HasDrawPage() const { return pDrawPages != nullptr; }
    bool HasForm(const sal_Int32 nTable, css::uno::Reference<css::drawing::XDrawPage>& xDrawPage);
    void AddNewShape(const ScMyShape& aMyShape);
    void SortShapesContainer();
    ScMyShapesContainer* GetShapesContainer() { return pShapesContainer.get(); }
    bool HasShapes() const;
    void AddTableShape(const sal_Int32 nTable, const css::uno::Reference<css::drawing::XShape>& xShape);
    ScMyTableShapes* GetTableShapes() { return pTableShapes.get(); }
    ScMyDetectiveObjContainer* GetDetectiveObjContainer() { return pDetectiveObjContainer.get(); }
    void AddNoteObj(const css::uno::Reference<css::drawing::XShape>& xShape, const ScAddress& rPos);
    void SortNoteShapes();
    ScMyNoteShapesContainer* GetNoteShapes() { return pNoteShapes.get(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
