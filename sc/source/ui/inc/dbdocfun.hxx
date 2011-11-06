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



#ifndef SC_DBDOCFUN_HXX
#define SC_DBDOCFUN_HXX

#include "address.hxx"
#include <tools/solar.h>
#include <com/sun/star/uno/Sequence.hxx>

class String;

struct ScImportParam;
struct ScQueryParam;
struct ScSortParam;
struct ScSubTotalParam;

class SfxViewFrame;
class ScDBData;
class ScDocShell;
class ScAddress;
class ScRange;
class ScDPObject;

namespace com { namespace sun { namespace star {
    namespace beans {
        struct PropertyValue;
    }
} } }

namespace svx {
    class ODataAccessDescriptor;
}

// ---------------------------------------------------------------------------
// -----------------------------------------------------------------


class ScDBDocFunc
{
friend class ScDBFunc;

private:
    ScDocShell&     rDocShell;

public:
                    ScDBDocFunc( ScDocShell& rDocSh ): rDocShell(rDocSh) {}
                    ~ScDBDocFunc() {}

    void            UpdateImport( const String& rTarget, const svx::ODataAccessDescriptor& rDescriptor );

    sal_Bool        DoImport( SCTAB nTab, const ScImportParam& rParam,
                        const svx::ODataAccessDescriptor* pDescriptor,      // used for selection and existing ResultSet
                        sal_Bool bRecord,
                        sal_Bool bAddrInsert = sal_False );

    sal_Bool        DoImportUno( const ScAddress& rPos,
                                const com::sun::star::uno::Sequence<
                                    com::sun::star::beans::PropertyValue>& aArgs );

    static void     ShowInBeamer( const ScImportParam& rParam, SfxViewFrame* pFrame );

    sal_Bool        Sort( SCTAB nTab, const ScSortParam& rSortParam,
                            sal_Bool bRecord, sal_Bool bPaint, sal_Bool bApi );

    SC_DLLPUBLIC sal_Bool Query( SCTAB nTab, const ScQueryParam& rQueryParam,
                            const ScRange* pAdvSource, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            DoSubTotals( SCTAB nTab, const ScSubTotalParam& rParam,
                                    const ScSortParam* pForceNewSort,
                                    sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            AddDBRange( const String& rName, const ScRange& rRange, sal_Bool bApi );
    sal_Bool            DeleteDBRange( const String& rName, sal_Bool bApi );
    sal_Bool            RenameDBRange( const String& rOld, const String& rNew, sal_Bool bApi );
    sal_Bool            ModifyDBData( const ScDBData& rNewData, sal_Bool bApi );    // Name unveraendert

    sal_Bool            RepeatDB( const String& rDBName, sal_Bool bRecord, sal_Bool bApi );

    sal_Bool            DataPilotUpdate( ScDPObject* pOldObj, const ScDPObject* pNewObj,
                                        sal_Bool bRecord, sal_Bool bApi, sal_Bool bAllowMove = sal_False );
};



#endif
