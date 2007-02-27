/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adiasync.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 12:10:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SC_ADIASYNC_HXX
#define _SC_ADIASYNC_HXX

#ifndef _SVT_BROADCAST_HXX
#include <svtools/broadcast.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

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
    ULONG           nHandle;            // wird von double auf ULONG gecasted
    ParamType       eType;              // PTR_DOUBLE oder PTR_STRING Ergebnis
    BOOL            bValid;             // ob Wert gueltig

public:
                    // cTor nur wenn ScAddInAsync::Get fehlschlaegt!
                    // nIndex: Index aus der FunctionCollection
                    ScAddInAsync( ULONG nHandle, USHORT nIndex,
                                    ScDocument* pDoc );
                    // default-cTor nur fuer das eine globale aSeekObj !!!
                    ScAddInAsync();
    virtual         ~ScAddInAsync();
    static ScAddInAsync*    Get( ULONG nHandle );
    static void     CallBack( ULONG nHandle, void* pData );
    static void     RemoveDocument( ScDocument* pDocument );
    BOOL            IsValid() const         { return bValid; }
    ParamType       GetType() const         { return eType; }
    double          GetValue() const        { return nVal; }
    const String&   GetString() const       { return *pStr; }
    BOOL            HasDocument( ScDocument* pDoc ) const
                        { return pDocs->Seek_Entry( pDoc ); }
    void            AddDocument( ScDocument* pDoc ) { pDocs->Insert( pDoc ); }

    // Vergleichsoperatoren fuer PtrArrSort
    BOOL operator < ( const ScAddInAsync& r ) { return nHandle <  r.nHandle; }
    BOOL operator ==( const ScAddInAsync& r ) { return nHandle == r.nHandle; }
};



#endif
