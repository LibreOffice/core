/*************************************************************************
 *
 *  $RCSfile: methods1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#if defined(WIN)
#include <string.h>
#else
#include <stdlib.h> // getenv
#endif

#ifndef NOOLDSV //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MAPMOD_HXX
#include <vcl/mapmod.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SBXVAR_HXX
#include <svtools/sbxvar.hxx>
#endif
#ifndef _SBX_HXX
#include <svtools/sbx.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#ifdef OS2
#define INCL_DOS
#define INCL_DOSPROCESS
#include <tools/svpm.h>
#include <vcl/sysdep.hxx>
#endif

#if defined(WIN)
#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif
#endif

#ifndef OS2
#include <time.h>
#endif

#ifndef CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC
#endif

#ifdef VCL
#include <vcl/jobset.hxx>
#else
#include <vcl/jobset.hxx>
#endif

#pragma hdrstop
#include "sbintern.hxx"
#include "runtime.hxx"
#include "stdobj.hxx"
#include "rtlproto.hxx"
#include "dllmgr.hxx"
#include <iosys.hxx>
#ifndef SB_UNO_OBJ
#include "sbunoobj.hxx"
#endif
#include "propacc.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )


#if defined (OS2) && defined (__BORLANDC__)
#pragma option -w-par
#endif

static BOOL Convert (SbxDataType eType,
                     SbxValue &rSbxValue,
                     SbxVariable *pSbxVariable)
{
    return TRUE;
}

RTLFUNC(CBool) // JSM
{
    BOOL bVal = FALSE;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        bVal = pSbxVariable->GetBool();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutBool(bVal);
}

RTLFUNC(CByte) // JSM
{
    BYTE nByte = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nByte = pSbxVariable->GetByte();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutByte(nByte);
}

RTLFUNC(CCur)  // JSM
{
    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
}

RTLFUNC(CDate) // JSM
{
    double nVal = 0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetDate();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutDate(nVal);
}

RTLFUNC(CDbl)  // JSM
{
    double nVal = 0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            SbxError eOld = SbxBase::GetError();
            if( eOld != SbxERR_OK )
                SbxBase::ResetError();

            // AB #42529 , zunaechst Wandlung in Date versuchen
            // Wenn erfolgreich, ist das das Ergebnis
            nVal = pSbxVariable->GetDate();
            if( SbxBase::GetError() != SbxERR_OK )
            {
                SbxBase::ResetError();
                if( eOld != SbxERR_OK )
                    SbxBase::SetError( eOld );

                // AB #41690 , String holen
                String aScanStr = pSbxVariable->GetString();
                SbError Error = SbxValue::ScanNumIntnl( aScanStr, nVal );
                if( Error != SbxERR_OK )
                    StarBASIC::Error( Error );
            }
        }
        else
        {
            nVal = pSbxVariable->GetDouble();
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutDouble(nVal);
}

RTLFUNC(CInt)  // JSM
{
    INT16 nVal = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetInteger();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutInteger(nVal);
}

RTLFUNC(CLng)  // JSM
{
    INT32 nVal = 0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        nVal = pSbxVariable->GetLong();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutLong(nVal);
}

RTLFUNC(CSng)  // JSM
{
    float nVal = (float)0.0;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        if( pSbxVariable->GetType() == SbxSTRING )
        {
            SbxError eOld = SbxBase::GetError();
            if( eOld != SbxERR_OK )
                SbxBase::ResetError();

            // AB #42529 , zunaechst Wandlung in Date versuchen
            // Wenn erfolgreich, ist das das Ergebnis
            double dVal = pSbxVariable->GetDate();
            if( SbxBase::GetError() != SbxERR_OK )
            {
                SbxBase::ResetError();
                if( eOld != SbxERR_OK )
                    SbxBase::SetError( eOld );

                // AB #41690 , String holen
                String aScanStr = pSbxVariable->GetString();
                SbError Error = SbxValue::ScanNumIntnl( aScanStr, dVal, /*bSingle=*/TRUE );
                if( SbxBase::GetError() == SbxERR_OK && Error != SbxERR_OK )
                    StarBASIC::Error( Error );
            }
            nVal = (float)dVal;
        }
        else
        {
            nVal = pSbxVariable->GetSingle();
        }
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutSingle(nVal);
}

RTLFUNC(CStr)  // JSM
{
    String aString;
    if ( rPar.Count() == 2 )
    {
        SbxVariable *pSbxVariable = rPar.Get(1);
        aString = pSbxVariable->GetString();
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );

    rPar.Get(0)->PutString(aString);
}

RTLFUNC(CVar)  // JSM
{
    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
}

RTLFUNC(CVErr) // JSM
{
    rPar.Get(0)->PutEmpty();
    StarBASIC::Error(SbERR_NOT_IMPLEMENTED);
}

RTLFUNC(Iif) // JSM
{
    if ( rPar.Count() == 4 )
    {
        if (rPar.Get(1)->GetBool())
            *rPar.Get(0) = *rPar.Get(2);
        else
            *rPar.Get(0) = *rPar.Get(3);
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(GetSystemType)
{
    if ( rPar.Count() != 1 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
        // Removed for SRC595
        rPar.Get(0)->PutInteger( -1 );
}

RTLFUNC(GetGUIType)
{
    if ( rPar.Count() != 1 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // 17.7.2000 Make simple solution for testtool / fat office
#if defined (WNT) || (defined (OS2) && !defined (WTC))
        rPar.Get(0)->PutInteger( 1 );
#elif defined OS2
        rPar.Get(0)->PutInteger( 2 );
#elif defined UNX
        rPar.Get(0)->PutInteger( 4 );
#elif
        rPar.Get(0)->PutInteger( -1 );
#endif
    }
}

RTLFUNC(Red)
{
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        ULONG nRGB = (ULONG)rPar.Get(1)->GetLong();
        nRGB &= 0x00FF0000;
        nRGB >>= 16;
        rPar.Get(0)->PutInteger( (INT16)nRGB );
    }
}

RTLFUNC(Green)
{
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        ULONG nRGB = (ULONG)rPar.Get(1)->GetLong();
        nRGB &= 0x0000FF00;
        nRGB >>= 8;
        rPar.Get(0)->PutInteger( (INT16)nRGB );
    }
}

RTLFUNC(Blue)
{
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        ULONG nRGB = (ULONG)rPar.Get(1)->GetLong();
        nRGB &= 0x000000FF;
        rPar.Get(0)->PutInteger( (INT16)nRGB );
    }
}


RTLFUNC(Switch)
{
    USHORT nCount = rPar.Count();
    if( !(nCount & 0x0001 ))
        // Anzahl der Argumente muss ungerade sein
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    USHORT nCurExpr = 1;
    while( nCurExpr < (nCount-1) )
    {
        if( rPar.Get( nCurExpr )->GetBool())
        {
            (*rPar.Get(0)) = *(rPar.Get(nCurExpr+1));
            return;
        }
        nCurExpr += 2;
    }
    rPar.Get(0)->PutNull();
}


RTLFUNC(Wait)
{
    if( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    long nWait = rPar.Get(1)->GetLong();
    if( nWait < 0 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
#if defined(OS2)
    ULONG nStart, nCur;
    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT,&nStart,sizeof(ULONG) );
    // drucken wir gerade?
    int bPrinting = Sysdepen::IsMultiThread() ? TRUE : FALSE;
    do
    {
        Application::Reschedule();
        if( bPrinting )
            DosSleep( 50 ); // damit der Druck-Thread mehr CPU-Zeit bekommt
        DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT,&nCur,sizeof(ULONG) );
    } while( (nCur-nStart) < (ULONG)nWait );
#else
    long nSeconds = nWait / 1000;
    if( !nSeconds ) nSeconds = 1;
#if defined(UNX) || defined(WIN)
    // Unix hat kein clock()
    time_t nStart = time( 0 );
    time_t nEnd;
    do
    {
        Application::Reschedule();
        nEnd = time( 0 );
    } while( (nEnd-nStart) < nSeconds );
#else
    clock_t nStart = clock() / CLK_TCK;
    clock_t nEnd;
    do
    {
        Application::Reschedule();
        nEnd = clock() / CLK_TCK;
    } while( (nEnd-nStart) < nSeconds );
#endif

#endif
}

RTLFUNC(GetGUIVersion)
{
    if ( rPar.Count() != 1 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        // Removed for SRC595
        rPar.Get(0)->PutLong( -1 );
    }
}

RTLFUNC(Choose)
{
    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    INT16 nIndex = rPar.Get(1)->GetInteger();
    USHORT nCount = rPar.Count();
    nCount--;
    if( nCount == 1 || nIndex > (nCount-1) || nIndex < 1 )
    {
        rPar.Get(0)->PutNull();
        return;
    }
    (*rPar.Get(0)) = *(rPar.Get(nIndex+1));
}


RTLFUNC(Trim)
{
    if ( rPar.Count() < 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        String aStr( rPar.Get(1)->GetString() );
        aStr.EraseLeadingChars();
        aStr.EraseTrailingChars();
        rPar.Get(0)->PutString( aStr );
    }
}

RTLFUNC(DateAdd)
{
}

RTLFUNC(DateDiff)
{
}

RTLFUNC(DatePart)
{
}


RTLFUNC(GetSolarVersion)
{
    rPar.Get(0)->PutLong( (INT32)SUPD );
}

RTLFUNC(TwipsPerPixelX)
{
    Size aSize( 100,0 );
    MapMode aMap( MAP_TWIP );
    aSize = GetpApp()->GetAppWindow()->PixelToLogic( aSize, aMap );
    aSize.Width() /= 100;
    rPar.Get(0)->PutLong( aSize.Width() );
}

RTLFUNC(TwipsPerPixelY)
{
    Size aSize( 0,100 );
    MapMode aMap( MAP_TWIP );
    aSize = GetpApp()->GetAppWindow()->PixelToLogic( aSize, aMap );
    aSize.Height() /= 100;
    rPar.Get(0)->PutLong( aSize.Height() );
}


RTLFUNC(FreeLibrary)
{
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    ByteString aByteDLLName( rPar.Get(1)->GetString(), gsl_getSystemTextEncoding() );
    pINST->GetDllMgr()->FreeDll( aByteDLLName );
}

RTLFUNC(Array)
{
    SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
    USHORT nArraySize = rPar.Count() - 1;

    // Option Base zunaechst ignorieren (kennt leider nur der Compiler)
    if( nArraySize )
        pArray->AddDim( 0, nArraySize-1 );

    // Parameter ins Array uebernehmen
    for( short i = 0 ; i < nArraySize ; i++ )
        pArray->Put( rPar.Get(i+1), &i );

    // Array zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    USHORT nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}


// Featurewunsch #57868
// Die Funktion liefert ein Variant-Array, wenn keine Parameter angegeben
// werden, wird ein leeres Array erzeugt (entsprechend dim a(), entspricht
// einer Sequence der Laenge 0 in Uno).
// Wenn Parameter angegeben sind, wird fuer jeden eine Dimension erzeugt
// DimArray( 2, 2, 4 ) entspricht DIM a( 2, 2, 4 )
// Das Array ist immer vom Typ Variant
RTLFUNC(DimArray)
{
    SbxDimArray* pArray = new SbxDimArray( SbxVARIANT );
    USHORT nArrayDims = rPar.Count() - 1;
    if( nArrayDims > 0 )
    {
        for( USHORT i = 0; i < nArrayDims ; i++ )
        {
            INT16 ub = rPar.Get(i+1)->GetInteger();
            if( ub < 0 )
            {
                StarBASIC::Error( SbERR_OUT_OF_RANGE );
                ub = 0;
            }
            pArray->AddDim( 0, ub );
        }
    }
    // Array zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    USHORT nFlags = refVar->GetFlags();
    refVar->ResetFlag( SBX_FIXED );
    refVar->PutObject( pArray );
    refVar->SetFlags( nFlags );
    refVar->SetParameters( NULL );
}

/*
 * FindObject und FindPropertyObject ermoeglichen es,
 * Objekte und Properties vom Typ Objekt zur Laufzeit
 * ueber ihren Namen als String-Parameter anzusprechen.
 *
 * Bsp.:
 * MyObj.Prop1.Bla = 5
 *
 * entspricht:
 * dim ObjVar as Object
 * dim ObjProp as Object
 * ObjName$ = "MyObj"
 * ObjVar = FindObject( ObjName$ )
 * PropName$ = "Prop1"
 * ObjProp = FindPropertyObject( ObjVar, PropName$ )
 * ObjProp.Bla = 5
 *
 * Dabei koennen die Namen zur Laufzeit dynamisch
 * erzeugt werden und, so dass z.B. ueber Controls
 * "TextEdit1" bis "TextEdit5" in einem Dialog in
 * einer Schleife iteriert werden kann.
 */

// Objekt ueber den Namen ansprechen
// 1. Parameter = Name des Objekts als String
RTLFUNC(FindObject)
{
    // Wir brauchen einen Parameter
    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // 1. Parameter ist der Name
    String aNameStr = rPar.Get(1)->GetString();

    // Basic-Suchfunktion benutzen
    SbxBase* pFind =  StarBASIC::FindSBXInCurrentScope( aNameStr );
    SbxObject* pFindObj = NULL;
    if( pFind )
        pFindObj = PTR_CAST(SbxObject,pFind);
    /*
    if( !pFindObj )
    {
        StarBASIC::Error( SbERR_VAR_UNDEFINED );
        return;
    }
    */

    // Objekt zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}

// Objekt-Property in einem Objekt ansprechen
// 1. Parameter = Objekt
// 2. Parameter = Name der Property als String
RTLFUNC(FindPropertyObject)
{
    // Wir brauchen 2 Parameter
    if ( rPar.Count() < 3 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    // 1. Parameter holen, muss Objekt sein
    SbxBase* pObjVar = (SbxObject*)rPar.Get(1)->GetObject();
    SbxObject* pObj = NULL;
    if( pObjVar )
        pObj = PTR_CAST(SbxObject,pObjVar);
    if( !pObj && pObjVar && pObjVar->ISA(SbxVariable) )
    {
        SbxBase* pObjVarObj = ((SbxVariable*)pObjVar)->GetObject();
        pObj = PTR_CAST(SbxObject,pObjVarObj);
    }
    /*
    if( !pObj )
    {
        StarBASIC::Error( SbERR_VAR_UNDEFINED );
        return;
    }
    */

    // 2. Parameter ist der Name
    String aNameStr = rPar.Get(2)->GetString();

    // Jetzt muss ein Objekt da sein, sonst Error
    SbxObject* pFindObj = NULL;
    if( pObj )
    {
        // Im Objekt nach Objekt suchen
        SbxVariable* pFindVar = pObj->Find( aNameStr, SbxCLASS_OBJECT );
        pFindObj = PTR_CAST(SbxObject,pFindVar);
    }
    else
        StarBASIC::Error( SbERR_BAD_PARAMETER );

    // Objekt zurueckliefern
    SbxVariableRef refVar = rPar.Get(0);
    refVar->PutObject( pFindObj );
}



BOOL lcl_WriteSbxVariable( const SbxVariable& rVar, SvStream* pStrm,
    BOOL bBinary, short nBlockLen, BOOL bIsArray )
{
    ULONG nFPos = pStrm->Tell();

    BOOL bIsVariant = !rVar.IsFixed();
    SbxDataType eType = rVar.GetType();

    switch( eType )
    {
        case SbxBOOL:
        case SbxCHAR:
        case SbxBYTE:
                if( bIsVariant )
                    *pStrm << (USHORT)SbxBYTE; // VarType Id
                *pStrm << rVar.GetByte();
                break;

        case SbxEMPTY:
        case SbxNULL:
        case SbxVOID:
        case SbxINTEGER:
        case SbxUSHORT:
        case SbxINT:
        case SbxUINT:
                if( bIsVariant )
                    *pStrm << (USHORT)SbxINTEGER; // VarType Id
                *pStrm << rVar.GetInteger();
                break;

        case SbxLONG:
        case SbxULONG:
        case SbxLONG64:
        case SbxULONG64:
                if( bIsVariant )
                    *pStrm << (USHORT)SbxLONG; // VarType Id
                *pStrm << rVar.GetLong();
                break;

        case SbxSINGLE:
                if( bIsVariant )
                    *pStrm << (USHORT)eType; // VarType Id
                *pStrm << rVar.GetSingle();
                break;

        case SbxDOUBLE:
        case SbxCURRENCY:
        case SbxDATE:
                if( bIsVariant )
                    *pStrm << (USHORT)eType; // VarType Id
                *pStrm << rVar.GetDouble();
                break;

        case SbxSTRING:
        case SbxLPSTR:
                {
                const String& rStr = rVar.GetString();
                if( !bBinary || bIsArray )
                {
                    if( bIsVariant )
                        *pStrm << (USHORT)SbxSTRING;
                    pStrm->WriteByteString( rStr, gsl_getSystemTextEncoding() );
                    //*pStrm << rStr;
                }
                else
                {
                    // ohne Laengenangabe! ohne Endekennung!
                    // What does that mean for Unicode?! Choosing conversion to ByteString...
                    ByteString aByteStr( rStr, gsl_getSystemTextEncoding() );
                    *pStrm << (const char*)aByteStr.GetBuffer();
                    //*pStrm << (const char*)rStr.GetStr();
                }
                }
                break;

        default:
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return FALSE;
    }

    if( nBlockLen )
        pStrm->Seek( nFPos + nBlockLen );
    return pStrm->GetErrorCode() ? FALSE : TRUE;
}

BOOL lcl_ReadSbxVariable( SbxVariable& rVar, SvStream* pStrm,
    BOOL bBinary, short nBlockLen, BOOL bIsArray )
{
    double aDouble;

    ULONG nFPos = pStrm->Tell();

    BOOL bIsVariant = !rVar.IsFixed();
    SbxDataType eVarType = rVar.GetType();

    SbxDataType eSrcType = eVarType;
    if( bIsVariant )
    {
        USHORT nTemp;
        *pStrm >> nTemp;
        eSrcType = (SbxDataType)nTemp;
    }

    switch( eSrcType )
    {
        case SbxBOOL:
        case SbxCHAR:
        case SbxBYTE:
                {
                BYTE aByte;
                *pStrm >> aByte;
                rVar.PutByte( aByte );
                }
                break;

        case SbxEMPTY:
        case SbxNULL:
        case SbxVOID:
        case SbxINTEGER:
        case SbxUSHORT:
        case SbxINT:
        case SbxUINT:
                {
                INT16 aInt;
                *pStrm >> aInt;
                rVar.PutInteger( aInt );
                }
                break;

        case SbxLONG:
        case SbxULONG:
        case SbxLONG64:
        case SbxULONG64:
                {
                INT32 aInt;
                *pStrm >> aInt;
                rVar.PutLong( aInt );
                }
                break;

        case SbxSINGLE:
                {
                float nS;
                *pStrm >> nS;
                rVar.PutSingle( nS );
                }
                break;

        case SbxDOUBLE:
        case SbxCURRENCY:
                {
                *pStrm >> aDouble;
                rVar.PutDouble( aDouble );
                }
                break;

        case SbxDATE:
                {
                *pStrm >> aDouble;
                rVar.PutDate( aDouble );
                }
                break;

        case SbxSTRING:
        case SbxLPSTR:
                {
                String aStr;
                pStrm->ReadByteString( aStr, gsl_getSystemTextEncoding() );
                rVar.PutString( aStr );
                }
                break;

        default:
                StarBASIC::Error( SbERR_BAD_ARGUMENT );
                return FALSE;
    }

    if( nBlockLen )
        pStrm->Seek( nFPos + nBlockLen );
    return pStrm->GetErrorCode() ? FALSE : TRUE;
}


// nCurDim = 1...n
BOOL lcl_WriteReadSbxArray( SbxDimArray& rArr, SvStream* pStrm,
    BOOL bBinary, short nCurDim, short* pOtherDims, BOOL bWrite )
{
    DBG_ASSERT( nCurDim > 0,"Bad Dim");
    short nLower, nUpper;
    if( !rArr.GetDim( nCurDim, nLower, nUpper ) )
        return FALSE;
    for( short nCur = nLower; nCur <= nUpper; nCur++ )
    {
        pOtherDims[ nCurDim-1 ] = nCur;
        if( nCurDim != 1 )
            lcl_WriteReadSbxArray(rArr, pStrm, bBinary, nCurDim-1, pOtherDims, bWrite);
        else
        {
            SbxVariable* pVar = rArr.Get( (const short*)pOtherDims );
            BOOL bRet;
            if( bWrite )
                bRet = lcl_WriteSbxVariable(*pVar, pStrm, bBinary, 0, TRUE );
            else
                bRet = lcl_ReadSbxVariable(*pVar, pStrm, bBinary, 0, TRUE );
            if( !bRet )
                return FALSE;
        }
    }
    return TRUE;
}

void PutGet( SbxArray& rPar, BOOL bPut )
{
    // Wir brauchen 3 Parameter
    if ( rPar.Count() != 4 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    INT16 nFileNo = rPar.Get(1)->GetInteger();
    SbxVariable* pVar2 = rPar.Get(2);
    BOOL bHasRecordNo = (BOOL)(pVar2->GetType() != SbxEMPTY);
    long nRecordNo = pVar2->GetLong();
    if ( nFileNo < 1 || ( bHasRecordNo && nRecordNo < 1 ) )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    nRecordNo--; // wir moegen's ab 0!
    SbiIoSystem* pIO = pINST->GetIoSystem();
    SbiStream* pSbStrm = pIO->GetStream( nFileNo );
    // das File muss Random (feste Record-Laenge) oder Binary sein
    if ( !pSbStrm || !(pSbStrm->GetMode() & (SBSTRM_BINARY | SBSTRM_RANDOM)) )
    {
        StarBASIC::Error( SbERR_BAD_CHANNEL );
        return;
    }

    SvStream* pStrm = pSbStrm->GetStrm();
    BOOL bRandom = pSbStrm->IsRandom();
    short nBlockLen = bRandom ? pSbStrm->GetBlockLen() : 0;

    if( bPut )
    {
        // Datei aufplustern, falls jemand uebers Dateiende hinaus geseekt hat
        pSbStrm->ExpandFile();
    }

    // auf die Startposition seeken
    if( bHasRecordNo )
    {
        ULONG nFilePos = bRandom ? (ULONG)(nBlockLen*nRecordNo) : (ULONG)nRecordNo;
        pStrm->Seek( nFilePos );
    }

    SbxDimArray* pArr = 0;
    SbxVariable* pVar = rPar.Get(3);
    if( pVar->GetType() & SbxARRAY )
    {
        SbxBase* pParObj = pVar->GetObject();
        pArr = PTR_CAST(SbxDimArray,pParObj);
    }

    BOOL bRet;

    if( pArr )
    {
        ULONG nFPos = pStrm->Tell();
        short nDims = pArr->GetDims();
        short* pDims = new short[ nDims ];
        bRet = lcl_WriteReadSbxArray(*pArr,pStrm,!bRandom,nDims,pDims,bPut);
        delete pDims;
        if( nBlockLen )
            pStrm->Seek( nFPos + nBlockLen );
    }
    else
    {
        if( bPut )
            bRet = lcl_WriteSbxVariable(*pVar, pStrm, !bRandom, nBlockLen, FALSE);
        else
            bRet = lcl_ReadSbxVariable(*pVar, pStrm, !bRandom, nBlockLen, FALSE);
    }
    if( !bRet || pStrm->GetErrorCode() )
        StarBASIC::Error( SbERR_IO_ERROR );
}

RTLFUNC(Put)
{
    PutGet( rPar, TRUE );
}

RTLFUNC(Get)
{
    PutGet( rPar, FALSE );
}

RTLFUNC(Environ)
{
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    String aResult;
    // sollte ANSI sein, aber unter Win16 in DLL nicht moeglich
#if defined(WIN)
    LPSTR lpszEnv = GetDOSEnvironment();
    String aCompareStr( rPar.Get(1)->GetString() );
    aCompareStr += '=';
    const char* pCompare = aCompareStr.GetStr();
    int nCompareLen = aCompareStr.Len();
    while ( *lpszEnv )
    {
        // Es werden alle EnvString in der Form ENV=VAL 0-terminiert
        // aneinander gehaengt.

        if ( strnicmp( pCompare, lpszEnv, nCompareLen ) == 0 )
        {
            aResult = (const char*)(lpszEnv+nCompareLen);
            rPar.Get(0)->PutString( aResult );
            return;
        }
        lpszEnv += lstrlen( lpszEnv ) + 1;  // Next Enviroment-String
    }
#else
    ByteString aByteStr( rPar.Get(1)->GetString(), gsl_getSystemTextEncoding() );
    const char* pEnvStr = getenv( aByteStr.GetBuffer() );
    if ( pEnvStr )
        aResult = String::CreateFromAscii( pEnvStr );
#endif
    rPar.Get(0)->PutString( aResult );
}

static double GetDialogZoomFactor( BOOL bX, long nValue )
{
    Size aRefSize( nValue, nValue );
#ifndef WIN
    Fraction aFracX( 1, 26 );
#else
    Fraction aFracX( 1, 23 );
#endif
    Fraction aFracY( 1, 24 );
    MapMode aMap( MAP_APPFONT, Point(), aFracX, aFracY );
    Window* pWin = GetpApp()->GetAppWindow();
    Size aScaledSize = pWin->LogicToPixel( aRefSize, aMap );
    aRefSize = pWin->LogicToPixel( aRefSize, MapMode(MAP_TWIP) );
    double nRef, nScaled, nResult;
    if( bX )
    {
        nRef = aRefSize.Width();
        nScaled = aScaledSize.Width();
    }
    else
    {
        nRef = aRefSize.Height();
        nScaled = aScaledSize.Height();
    }
    nResult = nScaled / nRef;
    return nResult;
}


RTLFUNC(GetDialogZoomFactorX)
{
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutDouble( GetDialogZoomFactor( TRUE, rPar.Get(1)->GetLong() ));
}

RTLFUNC(GetDialogZoomFactorY)
{
    if ( rPar.Count() != 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutDouble( GetDialogZoomFactor( FALSE, rPar.Get(1)->GetLong()));
}


RTLFUNC(EnableReschedule)
{
    rPar.Get(0)->PutEmpty();
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    if( pINST )
        pINST->EnableReschedule( rPar.Get(1)->GetBool() );
}

RTLFUNC(GetSystemTicks)
{
    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutLong( Time::GetSystemTicks() );
}

RTLFUNC(GetPathSeparator)
{
    if ( rPar.Count() != 1 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }
    rPar.Get(0)->PutString( DirEntry::GetAccessDelimiter() );
}

RTLFUNC(ResolvePath)
{
    if ( rPar.Count() == 2 )
    {
        String aStr = rPar.Get(1)->GetString();
        DirEntry aEntry( aStr );
        //if( aEntry.IsVirtual() )
            //aStr = aEntry.GetRealPathFromVirtualURL();
        rPar.Get(0)->PutString( aStr );
    }
    else
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
}

RTLFUNC(TypeLen)
{
    if ( rPar.Count() != 2 )
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
    else
    {
        SbxDataType eType = rPar.Get(1)->GetType();
        INT16 nLen = 0;
        switch( eType )
        {
            case SbxEMPTY:
            case SbxNULL:
            case SbxVECTOR:
            case SbxARRAY:
            case SbxBYREF:
            case SbxVOID:
            case SbxHRESULT:
            case SbxPOINTER:
            case SbxDIMARRAY:
            case SbxCARRAY:
            case SbxUSERDEF:
                nLen = 0;
                break;

            case SbxINTEGER:
            case SbxERROR:
            case SbxUSHORT:
            case SbxINT:
            case SbxUINT:
                nLen = 2;
                break;

            case SbxLONG:
            case SbxSINGLE:
            case SbxULONG:
                nLen = 4;
                break;

            case SbxDOUBLE:
            case SbxCURRENCY:
            case SbxDATE:
            case SbxLONG64:
            case SbxULONG64:
                nLen = 8;
                break;

            case SbxOBJECT:
            case SbxVARIANT:
            case SbxDATAOBJECT:
                nLen = 0;
                break;

            case SbxCHAR:
            case SbxBYTE:
            case SbxBOOL:
                nLen = 1;
                break;

            case SbxLPSTR:
            case SbxLPWSTR:
            case SbxCoreSTRING:
            case SbxSTRING:
                nLen = (INT16)rPar.Get(1)->GetString().Len();
                break;

            default:
                nLen = 0;
        }
        rPar.Get(0)->PutInteger( nLen );
    }
}


// Uno-Struct eines beliebigen Typs erzeugen
// 1. Parameter == Klassename, weitere Parameter zur Initialisierung
RTLFUNC(CreateUnoStruct)
{
    RTL_Impl_CreateUnoStruct( pBasic, rPar, bWrite );
}

// Uno-Service erzeugen
// 1. Parameter == Service-Name
RTLFUNC(CreateUnoService)
{
    RTL_Impl_CreateUnoService( pBasic, rPar, bWrite );
}

// ServiceManager liefern (keine Parameter)
RTLFUNC(GetProcessServiceManager)
{
    RTL_Impl_GetProcessServiceManager( pBasic, rPar, bWrite );
}

// PropertySet erzeugen
// 1. Parameter == Sequence<PropertyValue>
RTLFUNC(CreatePropertySet)
{
    RTL_Impl_CreatePropertySet( pBasic, rPar, bWrite );
}

// Abfragen, ob ein Interface unterstuetzt wird
// Mehrere Interface-Namen als Parameter
RTLFUNC(HasUnoInterfaces)
{
    RTL_Impl_HasInterfaces( pBasic, rPar, bWrite );
}

// Abfragen, ob ein Basic-Objekt ein Uno-Struct repraesentiert
RTLFUNC(IsUnoStruct)
{
    RTL_Impl_IsUnoStruct( pBasic, rPar, bWrite );
}

// Abfragen, ob zwei Uno-Objekte identisch sind
RTLFUNC(EqualUnoObjects)
{
    RTL_Impl_EqualUnoObjects( pBasic, rPar, bWrite );
}

