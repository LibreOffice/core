/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
