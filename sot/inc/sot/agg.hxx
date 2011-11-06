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



#ifndef _SOT_AGG_HXX
#define _SOT_AGG_HXX

#include <tools/ownlist.hxx>

/************** class SvAggregate ***************************************/
/************************************************************************/
class SotFactory;
class SotObject;
struct SvAggregate
{
    union
    {
        SotFactory * pFact;
        SotObject * pObj;
    };
    sal_Bool    bFactory;
    sal_Bool    bMainObj; // sal_True, das Objekt, welches das casting steuert

    SvAggregate()
        : pFact( NULL )
        , bFactory( sal_False )
        , bMainObj( sal_False ) {}
    SvAggregate( SotObject * pObjP, sal_Bool bMainP )
        : pObj( pObjP )
        , bFactory( sal_False )
        , bMainObj( bMainP ) {}
    SvAggregate( SotFactory * pFactP )
        : pFact( pFactP )
        , bFactory( sal_True )
        , bMainObj( sal_False ) {}
};

/************** class SvAggregateMemberList *****************************/
/************************************************************************/
class SvAggregateMemberList
{
     PRV_SV_DECL_OWNER_LIST(SvAggregateMemberList,SvAggregate)
};

#endif // _AGG_HXX
