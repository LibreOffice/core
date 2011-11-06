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


#include "dbfunc.hxx"
#include "docsh.hxx"
#include "global.hxx"
#include "document.hxx"
#include "sc.hrc"
#include "globstr.hrc"


// STATIC DATA -----------------------------------------------------------


#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

//==================================================================

class ScDrawLayer;
class ScChartCollection;

void ScDBFunc::UpdateCharts( sal_Bool bAllCharts )
{
    sal_uInt16 nFound = 0;
    ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();

    if ( pDoc->GetDrawLayer() )
        nFound = DoUpdateCharts( ScAddress( pViewData->GetCurX(),
                                            pViewData->GetCurY(),
                                            pViewData->GetTabNo()),
                                 pDoc,
                                 bAllCharts );

    if ( !nFound && !bAllCharts )
        ErrorMessage(STR_NOCHARTATCURSOR);
}






