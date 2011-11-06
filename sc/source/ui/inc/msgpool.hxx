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



#ifndef SC_MSGPOOL_HXX
#define SC_MSGPOOL_HXX

#include "scitems.hxx"
#include <svl/srchitem.hxx>


#include <svl/itempool.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>

//#include <dbitems.hxx>
#include "uiitems.hxx"


//------------------------------------------------------------------------

class ScDocumentPool;

//------------------------------------------------------------------------

class ScMessagePool: public SfxItemPool
{
    SfxStringItem       aGlobalStringItem;
    SvxSearchItem       aGlobalSearchItem;
    ScSortItem          aGlobalSortItem;
    ScQueryItem         aGlobalQueryItem;
    ScSubTotalItem      aGlobalSubTotalItem;
    ScConsolidateItem   aGlobalConsolidateItem;
    ScPivotItem         aGlobalPivotItem;
    ScSolveItem         aGlobalSolveItem;
    ScUserListItem      aGlobalUserListItem;

    SfxBoolItem         aPrintWarnItem;

    SfxPoolItem**   ppPoolDefaults;
    ScDocumentPool* pDocPool;

public:
    ScMessagePool();
protected:
    virtual ~ScMessagePool();
public:

    virtual SfxMapUnit              GetMetric( sal_uInt16 nWhich ) const;
};



#endif

