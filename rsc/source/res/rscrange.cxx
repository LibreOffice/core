/*************************************************************************
 *
 *  $RCSfile: rscrange.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
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
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rscrange.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

*************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Solar Definitionen
#include <tools/solar.h>

// Programmabh„ngige Includes.
#ifndef _RSCRANGE_HXX
#include <rscrange.hxx>
#endif

/****************** D E F I N E S ****************************************/
#define USHORTBITS  (sizeof( USHORT ) * 8)
/****************** C O D E **********************************************/
/****************** R s c R a n g e **************************************/
/*************************************************************************
|*
|*    RscRange::RscRange()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RscRange::RscRange( HASHID nId, USHORT nTypeId )
                        : RscTop( nId, nTypeId )
{
    nMin = nMax = 0;
    nSize = ALIGNED_SIZE( sizeof( RscRangeInst ) );
}

/*************************************************************************
|*
|*    RscRange::GetClassType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RSCCLASS_TYPE RscRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

/*************************************************************************
|*
|*    RscRange::SetRange()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
ERRTYPE RscRange::SetRange( long nMinimum, long nMaximum )
{
    if( nMinimum > nMaximum )
    {
        nMin = nMaximum;
        nMax = nMinimum;
    }
    else
    {
        nMax = nMaximum;
        nMin = nMinimum;
    };

    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscRange::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.02.92
|*    Letzte Aenderung  MM 15.02.92
|*
*************************************************************************/
BOOL RscRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pDef )
    {
        if( ((RscRangeInst*)rInst.pData)->nValue ==
          ((RscRangeInst*)pDef)->nValue )
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*************************************************************************
|*
|*    RscRange::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
ERRTYPE RscRange::SetNumber( const RSCINST & rInst, long nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );
    ((RscRangeInst *)rInst.pData)->nValue = (USHORT)( nValue - nMin );
    ((RscRangeInst *)rInst.pData)->bDflt = FALSE;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscRange::GetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.04.91
|*    Letzte Aenderung  MM 22.04.91
|*
*************************************************************************/
ERRTYPE RscRange::GetNumber( const RSCINST & rInst, long * pN )
{
    *pN = ((RscRangeInst *)rInst.pData)->nValue + nMin;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscRange::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RSCINST RscRange::Create( RSCINST * pInst, const RSCINST & rDflt,
                            BOOL bOwnClass )
{
    RSCINST aInst;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)
                      RscMem::Malloc( sizeof( RscRangeInst ) );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, sizeof( RscRangeInst ) );
    else
    {
        if( 0L >= nMin && 0L <= nMax )
            ((RscRangeInst *)aInst.pData)->nValue = (USHORT)(0L - nMin);
        else
            ((RscRangeInst *)aInst.pData)->nValue = 0;
        ((RscRangeInst *)aInst.pData)->bDflt = TRUE;
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscRange::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.04.91
|*    Letzte Aenderung  MM 08.04.91
|*
*************************************************************************/
void RscRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont *, USHORT, const char * )
{
    fprintf( fOutput, "%ld", ((RscRangeInst *)rInst.pData)->nValue + nMin );
}

/*************************************************************************
|*
|*    RscRange::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
ERRTYPE RscRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                           RscTypCont *, USHORT, BOOL )
{
    if( nMin >= 0 )
    {
        USHORT n;
        n = (USHORT)(((RscRangeInst *)rInst.pData)->nValue + nMin);
        aMem.Put( n );
    }
    else
    {
        short n;
        n = (short)(((RscRangeInst *)rInst.pData)->nValue + nMin);
        aMem.Put( n );
    }

    return( ERR_OK );
}

//=======================================================================
void RscRange::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    if( nMin >= 0 )
        fprintf( fOutput, "*(USHORT *)(pResData+nOffset) );\n", pName );
    else
        fprintf( fOutput, "*(short *)(pResData+nOffset) );\n", pName );
    fprintf( fOutput, "\t\tnOffset += sizeof( short );\n" );
}

/****************** R s c L o n g R a n g e ******************************/
/*************************************************************************
|*
|*    RscLongRange::RscLongRange()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.07.94
|*
*************************************************************************/
RscLongRange::RscLongRange( HASHID nId, USHORT nTypeId )
                        : RscTop( nId, nTypeId )
{
    nMin = nMax = 0;
    nSize = ALIGNED_SIZE( sizeof( RscLongRangeInst ) );
}

/*************************************************************************
|*
|*    RscLongRange::GetClassType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.07.94
|*
*************************************************************************/
RSCCLASS_TYPE RscLongRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

/*************************************************************************
|*
|*    RscLongRange::SetRange()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.07.94
|*
*************************************************************************/
ERRTYPE RscLongRange::SetRange( long nMinimum, long nMaximum )
{
    if( nMinimum > nMaximum )
    {
        nMin = nMaximum;
        nMax = nMinimum;
    }
    else
    {
        nMax = nMaximum;
        nMin = nMinimum;
    };

    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscLongRange::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.02.92
|*    Letzte Aenderung  MM 15.02.92
|*
*************************************************************************/
BOOL RscLongRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef )
{
    if( pDef )
        return 0 == memcmp( &((RscLongRangeInst*)rInst.pData)->nValue,
                            &((RscLongRangeInst*)pDef)->nValue,
                            sizeof( long ) );

    return FALSE;
}

/*************************************************************************
|*
|*    RscLongRange::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.07.94
|*
*************************************************************************/
ERRTYPE RscLongRange::SetNumber( const RSCINST & rInst, long nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );
    void * pData = &((RscLongRangeInst*)rInst.pData)->nValue;
    memmove( pData, &nValue, sizeof( long ) );
    ((RscLongRangeInst *)rInst.pData)->bDflt = FALSE;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscLongRange::GetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.04.91
|*    Letzte Aenderung  MM 22.04.91
|*
*************************************************************************/
ERRTYPE RscLongRange::GetNumber( const RSCINST & rInst, long * pN )
{
    memmove( pN, &((RscLongRangeInst*)rInst.pData)->nValue,
             sizeof( long ) );
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscLongRange::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.07.94
|*
*************************************************************************/
RSCINST RscLongRange::Create( RSCINST * pInst, const RSCINST & rDflt,
                              BOOL bOwnClass )
{
    RSCINST aInst;

    if( !pInst )
    {
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)
                      RscMem::Malloc( sizeof( RscLongRangeInst ) );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    if( bOwnClass )
        memmove( aInst.pData, rDflt.pData, sizeof( RscLongRangeInst ) );
    else
    {
        long    lDflt;
        if( 0L >= nMin && 0L <= nMax )
            lDflt = 0;
        else
            lDflt = nMin;
        void * pData = &((RscLongRangeInst*)aInst.pData)->nValue;
        memmove( pData, &lDflt, sizeof( long ) );
        ((RscLongRangeInst *)aInst.pData)->bDflt = TRUE;
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscLongRange::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.07.94
|*
*************************************************************************/
void RscLongRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                         RscTypCont *, USHORT, const char * )
{
    long lVal;
    GetNumber( rInst, &lVal );
    fprintf( fOutput, "%ld", lVal );
}

/*************************************************************************
|*
|*    RscLongRange::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.07.94
|*    Letzte Aenderung  MM 18.04.94
|*
*************************************************************************/
ERRTYPE RscLongRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                               RscTypCont *, USHORT, BOOL )
{
    long lVal;

    GetNumber( rInst, &lVal );
    aMem.Put( (INT32)lVal );

    return( ERR_OK );
}

//=======================================================================
void RscLongRange::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    fprintf( fOutput, "GetLong( pResData+nOffset ) );\n", pName );
    fprintf( fOutput, "\t\tnOffset += sizeof( INT32 );\n" );
}

/****************** R s c L o n g E n u m R a n g e *********************/
/*************************************************************************
|*    RscLongEnumRange::RscLongEnumRange()
|*
|*    Beschreibung
*************************************************************************/
RscLongEnumRange::RscLongEnumRange( HASHID nId, USHORT nTypeId )
                        : RscLongRange( nId, nTypeId )
{
}

/*************************************************************************
|*    RscLongEnumRange::SetConst()
|*
|*    Beschreibung
*************************************************************************/
ERRTYPE RscLongEnumRange::SetConst( const RSCINST & rInst, HASHID nConst,
                                    long nValue )
{
    return SetNumber( rInst, nValue );
}

/****************** R s c I d R a n g e **********************************/
/*************************************************************************
|*
|*    RscIdRange::RscIdRange()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RscIdRange::RscIdRange( HASHID nId, USHORT nTypeId, BOOL bRcL )
            : RscTop( nId, nTypeId )
{
    nSize = ALIGNED_SIZE( sizeof( RscId ) );
    nMin = nMax = 0;
    bRcLong = bRcL;
}

/*************************************************************************
|*
|*    RscIdRange::RscIdRange()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RSCCLASS_TYPE RscIdRange::GetClassType() const
{
    return RSCCLASS_NUMBER;
}

/*************************************************************************
|*
|*    RscIdRange::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 03.04.91
|*    Letzte Aenderung  MM 03.04.91
|*
*************************************************************************/
RSCINST RscIdRange::Create( RSCINST * pInst, const RSCINST & rDflt, BOOL bOwnClass ){
    RSCINST aInst;
    RscId * pClassData;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)RscMem::Malloc( sizeof( RscId ) );
    }
    else
        aInst = *pInst;


    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    pClassData = (RscId *)aInst.pData;

    pClassData->Create();
    if( bOwnClass )
        *pClassData = *(RscId *)rDflt.pData;
    else{
            *pClassData = RscId();
        if( 0L >= nMin && 0L <= nMax )
            *pClassData = RscId( 0L );
        else
            *pClassData = RscId( nMin );
        //cUnused wird fuer Defaultkennung verwendet
        ((RscId *)aInst.pData)->aExp.cUnused = TRUE;
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscIdRange::Destroy()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.11.91
|*    Letzte Aenderung  MM 22.11.91
|*
*************************************************************************/
void RscIdRange :: Destroy( const RSCINST & rInst ){
    ((RscId *)rInst.pData)->Destroy();
}

/*************************************************************************
|*
|*    RscIdRange::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.01.92
|*    Letzte Aenderung  MM 15.01.92
|*
*************************************************************************/
BOOL RscIdRange::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    if( pDef ){
        if( ((RscId*)rInst.pData)->aExp.IsNumber()
          && ((RscId*)pDef)->aExp.IsNumber() )
        {
            if( ((RscId*)rInst.pData)->GetNumber() ==
              ((RscId*)pDef)->GetNumber() )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*************************************************************************
|*
|*    RscIdRange::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.11.91
|*    Letzte Aenderung  MM 25.11.91
|*
*************************************************************************/
ERRTYPE RscIdRange::SetNumber( const RSCINST & rInst, long nValue )
{
    if( nMax < nValue || nMin > nValue )
        return( ERR_RSCRANGE_OUTDEFSET );

    *(RscId *)rInst.pData = RscId( nValue );
    ((RscId *)rInst.pData)->aExp.cUnused = FALSE;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscIdRange::GetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.11.91
|*    Letzte Aenderung  MM 25.11.91
|*
*************************************************************************/
ERRTYPE RscIdRange::GetNumber( const RSCINST & rInst, long * plValue ){
    *plValue = ((RscId *)rInst.pData)->GetNumber();
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscIdRange::SetRef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.11.91
|*    Letzte Aenderung  MM 22.11.91
|*
*************************************************************************/
ERRTYPE RscIdRange::SetRef( const RSCINST & rInst, const RscId & rRscId ){
    ERRTYPE aError;
    if( rRscId.IsId() ){
        aError = SetNumber( rInst, rRscId );
        if( aError.IsOk() ){
            *(RscId *)rInst.pData = rRscId;
            ((RscId *)rInst.pData)->aExp.cUnused = FALSE;
        }
    }
    else
        aError = ERR_RSCRANGE_OUTDEFSET;

    return( aError );
}

/*************************************************************************
|*
|*    RscIdRange::GetRef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.11.91
|*    Letzte Aenderung  MM 22.11.91
|*
*************************************************************************/
ERRTYPE RscIdRange::GetRef( const RSCINST & rInst, RscId * pRscId ){
    *pRscId = *(RscId *)rInst.pData;

    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscIdRange::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.11.91
|*    Letzte Aenderung  MM 25.11.91
|*
*************************************************************************/
void RscIdRange::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                           RscTypCont *, USHORT, const char * )
{
    fprintf( fOutput, "%s", ((RscId *)rInst.pData)->GetName().GetBuffer() );
}

/*************************************************************************
|*
|*    RscIdRange::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.11.91
|*    Letzte Aenderung  MM 25.11.91
|*
*************************************************************************/
ERRTYPE RscIdRange::WriteRc( const RSCINST & rInst, RscWriteRc & aMem,
                             RscTypCont *, USHORT, BOOL )
{
    long lVal = ((RscId*)rInst.pData)->GetNumber();

    if( bRcLong )
        //wenn long geschrieben werden soll
        aMem.Put( (INT32)lVal );
    else
        aMem.Put( (USHORT)lVal );

    return( ERR_OK );
}

//=======================================================================
void RscIdRange::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    if( bRcLong )
    {
        fprintf( fOutput, "GetLong( pResData+nOffset ) );\n", pName );
        fprintf( fOutput, "\t\tnOffset += sizeof( INT32 );\n" );
    }
    else
    {
        fprintf( fOutput, "*(USHORT *)(pResData+nOffset) );\n", pName );
        fprintf( fOutput, "\t\tnOffset += sizeof( short );\n" );
    }
}

/*************************************************************************
|*
|*    RscIdRange::IsConsistent()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.11.91
|*    Letzte Aenderung  MM 22.11.91
|*
*************************************************************************/
BOOL RscIdRange::IsConsistent( const RSCINST & rInst, RscInconsList * pList )
{
    long nValue = ((RscId *)rInst.pData)->GetNumber();
    if( (nMax >= nValue) && (nMin <= nValue) )
        return TRUE;
    else {
        if( pList )
            pList->Insert( new RscInconsistent(
                *(RscId *)rInst.pData, *(RscId *)rInst.pData ) );
        return FALSE;
    }
}

/****************** R s c B o o l ****************************************/
/*************************************************************************
|*
|*    RscBool::RscBool()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.04.91
|*    Letzte Aenderung  MM 29.04.91
|*
*************************************************************************/
RscBool::RscBool( HASHID nId, USHORT nTypeId )
        : RscRange( nId, nTypeId )
{
    RscRange::SetRange( 0, 1 );
}

/*************************************************************************
|*
|*    RscBool::GetClassType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.04.91
|*    Letzte Aenderung  MM 29.04.91
|*
*************************************************************************/
RSCCLASS_TYPE  RscBool::GetClassType() const
{
    return RSCCLASS_BOOL;
}

/*************************************************************************
|*
|*    RscBool::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 29.04.91
|*    Letzte Aenderung  MM 29.04.91
|*
*************************************************************************/
void RscBool::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                        RscTypCont *, USHORT, const char * )
{
    long l;

    GetNumber( rInst, &l );
    if( l )
        fprintf( fOutput, "TRUE" );
    else
        fprintf( fOutput, "FALSE" );
}

//=======================================================================
void RscBool::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    fprintf( fOutput, "\t\tSet%s( ", pName );
    fprintf( fOutput, "(BOOL)*(short *)(pResData+nOffset) );\n", pName );
    fprintf( fOutput, "\t\tnOffset += sizeof( short );\n" );
}

/****************** R s c B r e a k R a n g e ****************************/
/*************************************************************************
|*
|*    RscBreakRange::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.06.91
|*    Letzte Aenderung  MM 24.06.91
|*
*************************************************************************/
RscBreakRange :: RscBreakRange( HASHID nId, USHORT nTypeId )
                        : RscRange( nId, nTypeId )
{
    nOutRange = 0xFFFFFFFF;
}

/*************************************************************************
|*
|*    RscBreakRange::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.06.91
|*    Letzte Aenderung  MM 24.06.91
|*
*************************************************************************/
ERRTYPE RscBreakRange::SetNumber( const RSCINST & rInst, long nValue ){
    if( nValue == nOutRange )
        return( ERR_RSCRANGE_OUTDEFSET );
    else
        return( RscRange::SetNumber( rInst, nValue ) );
}

/*************************************************************************
|*
|*    RscBreakRange::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 24.06.91
|*    Letzte Aenderung  MM 24.06.91
|*
*************************************************************************/
RSCINST RscBreakRange::Create( RSCINST * pInst, const RSCINST & rDflt,
                               BOOL bOwnClass )
{
    RSCINST aInst;
    long    l;

    aInst = RscRange::Create( pInst, rDflt, bOwnClass );

    GetNumber( aInst, &l );
    if( l == nOutRange )
        ((RscRangeInst *)aInst.pData)->nValue++;

    return( aInst );
}

