/*************************************************************************
 *
 *  $RCSfile: adiasync.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:16 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SC_ADIASYNC_HXX
#define _SC_ADIASYNC_HXX

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#include "callform.hxx"

extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData );
}


class ScAddInAsync;
typedef ScAddInAsync* ScAddInAsyncPtr;
SV_DECL_PTRARR_SORT( ScAddInAsyncs, ScAddInAsyncPtr, 4, 4 );
extern ScAddInAsyncs theAddInAsyncTbl;  // in adiasync.cxx

class ScDocument;
typedef ScDocument* ScAddInDocPtr;
SV_DECL_PTRARR_SORT( ScAddInDocs, ScAddInDocPtr, 1, 1 );

class String;

class ScAddInAsync : public SfxBroadcaster
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
