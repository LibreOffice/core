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



#ifndef SC_ADIASYNC_HXX
#define SC_ADIASYNC_HXX

#include <svl/broadcast.hxx>
#include <svl/svarray.hxx>

#include "callform.hxx"

extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData );
}


class ScAddInAsync;
typedef ScAddInAsync* ScAddInAsyncPtr;
SV_DECL_PTRARR_SORT( ScAddInAsyncs, ScAddInAsyncPtr, 4, 4 )
extern ScAddInAsyncs theAddInAsyncTbl;  // in adiasync.cxx

class ScDocument;
typedef ScDocument* ScAddInDocPtr;
SV_DECL_PTRARR_SORT( ScAddInDocs, ScAddInDocPtr, 1, 1 )

class String;

class ScAddInAsync : public SvtBroadcaster
{
private:
    union
    {
        double      nVal;               // aktueller Wert
        String*     pStr;
    };
    ScAddInDocs*    pDocs;              // Liste der benutzenden Dokumente
    FuncData*       pFuncData;          // Zeiger auf die Daten in der Collection
    sal_uLong           nHandle;            // wird von double auf sal_uLong gecasted
    ParamType       eType;              // PTR_DOUBLE oder PTR_STRING Ergebnis
    sal_Bool            bValid;             // ob Wert gueltig

public:
                    // cTor nur wenn ScAddInAsync::Get fehlschlaegt!
                    // nIndex: Index aus der FunctionCollection
                    ScAddInAsync( sal_uLong nHandle, sal_uInt16 nIndex,
                                    ScDocument* pDoc );
                    // default-cTor nur fuer das eine globale aSeekObj !!!
                    ScAddInAsync();
    virtual         ~ScAddInAsync();
    static ScAddInAsync*    Get( sal_uLong nHandle );
    static void     CallBack( sal_uLong nHandle, void* pData );
    static void     RemoveDocument( ScDocument* pDocument );
    sal_Bool            IsValid() const         { return bValid; }
    ParamType       GetType() const         { return eType; }
    double          GetValue() const        { return nVal; }
    const String&   GetString() const       { return *pStr; }
    sal_Bool            HasDocument( ScDocument* pDoc ) const
                        { return pDocs->Seek_Entry( pDoc ); }
    void            AddDocument( ScDocument* pDoc ) { pDocs->Insert( pDoc ); }

    // Vergleichsoperatoren fuer PtrArrSort
    sal_Bool operator < ( const ScAddInAsync& r ) { return nHandle <  r.nHandle; }
    sal_Bool operator ==( const ScAddInAsync& r ) { return nHandle == r.nHandle; }
};



#endif
