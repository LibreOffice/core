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



#ifndef SC_XMLEXPORTSHAREDDATA_HXX
#define SC_XMLEXPORTSHAREDDATA_HXX

#include "global.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <vector>
#include <list>

struct ScMyDrawPage
{
    com::sun::star::uno::Reference<com::sun::star::drawing::XDrawPage> xDrawPage;
    sal_Bool bHasForms;

    ScMyDrawPage() : bHasForms(sal_False) {}
};

typedef std::list< com::sun::star::uno::Reference<com::sun::star::drawing::XShape> > ScMyTableXShapes;
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
    ScMyTableShapes*            pTableShapes;
    ScMyDrawPages*              pDrawPages;
    ScMyShapesContainer*        pShapesContainer;
    ScMyDetectiveObjContainer*  pDetectiveObjContainer;
    ScMyNoteShapesContainer*    pNoteShapes;
    sal_Int32                   nTableCount;
public:
    ScMySharedData(const sal_Int32 nTableCount);
    ~ScMySharedData();

    void SetLastColumn(const sal_Int32 nTable, const sal_Int32 nCol);
    void SetLastRow(const sal_Int32 nTable, const sal_Int32 nRow);
    sal_Int32 GetLastColumn(const sal_Int32 nTable);
    sal_Int32 GetLastRow(const sal_Int32 nTable);
    void AddDrawPage(const ScMyDrawPage& aDrawPage, const sal_Int32 nTable);
    void SetDrawPageHasForms(const sal_Int32 nTable, sal_Bool bHasForms);
    com::sun::star::uno::Reference<com::sun::star::drawing::XDrawPage> GetDrawPage(const sal_Int32 nTable);
    sal_Bool HasDrawPage() { return pDrawPages != NULL; }
    sal_Bool HasForm(const sal_Int32 nTable, com::sun::star::uno::Reference<com::sun::star::drawing::XDrawPage>& xDrawPage);
    void AddNewShape(const ScMyShape& aMyShape);
    void SortShapesContainer();
    ScMyShapesContainer* GetShapesContainer() { return pShapesContainer; }
    sal_Bool HasShapes();
    void AddTableShape(const sal_Int32 nTable, const com::sun::star::uno::Reference<com::sun::star::drawing::XShape>& xShape);
    ScMyTableShapes* GetTableShapes() { return pTableShapes; }
    ScMyDetectiveObjContainer* GetDetectiveObjContainer() { return pDetectiveObjContainer; }
    void AddNoteObj(const com::sun::star::uno::Reference<com::sun::star::drawing::XShape>& xShape, const ScAddress& rPos);
    void SortNoteShapes();
    ScMyNoteShapesContainer* GetNoteShapes() { return pNoteShapes; }
};

#endif

