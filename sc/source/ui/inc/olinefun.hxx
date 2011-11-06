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



#ifndef SC_OLINEFUN_HXX
#define SC_OLINEFUN_HXX

#include "global.hxx"

class ScDocShell;
class ScRange;


// ---------------------------------------------------------------------------

class ScOutlineDocFunc
{
private:
    ScDocShell& rDocShell;

public:
                ScOutlineDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                ~ScOutlineDocFunc() {}

    sal_Bool        MakeOutline( const ScRange& rRange, sal_Bool bColumns, sal_Bool bRecord, sal_Bool bApi );
    sal_Bool        RemoveOutline( const ScRange& rRange, sal_Bool bColumns, sal_Bool bRecord, sal_Bool bApi );
    sal_Bool        RemoveAllOutlines( SCTAB nTab, sal_Bool bRecord, sal_Bool bApi );
    sal_Bool        AutoOutline( const ScRange& rRange, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool        SelectLevel( SCTAB nTab, sal_Bool bColumns, sal_uInt16 nLevel,
                                    sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

    sal_Bool        ShowMarkedOutlines( const ScRange& rRange, sal_Bool bRecord, sal_Bool bApi );
    sal_Bool        HideMarkedOutlines( const ScRange& rRange, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool        ShowOutline( SCTAB nTab, sal_Bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );
    sal_Bool        HideOutline( SCTAB nTab, sal_Bool bColumns, sal_uInt16 nLevel, sal_uInt16 nEntry,
                                    sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );
};



#endif

