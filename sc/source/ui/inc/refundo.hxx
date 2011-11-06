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



#ifndef SC_REFUNDO_HXX
#define SC_REFUNDO_HXX

#include <tools/solar.h>

class ScDocument;
class ScDBCollection;
class ScRangeName;
class ScPrintRangeSaver;
class ScDPCollection;
class ScChartCollection;
class ScConditionalFormatList;
class ScDetOpList;
class ScChartListenerCollection;
class ScAreaLinkSaveCollection;
class ScUnoRefList;

class ScRefUndoData
{
private:
    ScDBCollection*             pDBCollection;
    ScRangeName*                pRangeName;
    ScPrintRangeSaver*          pPrintRanges;
    ScDPCollection*             pDPCollection;
    ScConditionalFormatList*    pCondFormList;
    ScDetOpList*                pDetOpList;
    ScChartListenerCollection*  pChartListenerCollection;
    ScAreaLinkSaveCollection*   pAreaLinks;
    ScUnoRefList*               pUnoRefs;

public:
                ScRefUndoData( const ScDocument* pDoc );
                ~ScRefUndoData();

    void        DeleteUnchanged( const ScDocument* pDoc );
    void        DoUndo( ScDocument* pDoc, sal_Bool bUndoRefFirst );
};



#endif

