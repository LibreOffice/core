/*************************************************************************
 *
 *  $RCSfile: rscstr.cxx,v $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/res/rscstr.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.22  2000/09/17 12:51:11  willem.vandorp
    OpenOffice header added.

    Revision 1.21  2000/07/26 17:13:23  willem.vandorp
    Headers/footers replaced

    Revision 1.20  2000/07/12 11:38:42  th
    Unicode

    Revision 1.19  1999/10/12 08:45:03  mm
    gpf corrected

    Revision 1.18  1999/10/11 17:21:53  mm
    no LString

**************************************************************************/
/****************** I N C L U D E S **************************************/

// C and C++ Includes.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Programmabh„ngige Includes.
#ifndef _RSCDB_HXX
#include <rscdb.hxx>
#endif
#ifndef _RSCSTR_HXX
#include <rscstr.hxx>
#endif

#include <rtl/textcvt.h>
#include <rtl/textenc.h>

/****************** C O D E **********************************************/
/****************** R s c S t r i n g ************************************/
/*************************************************************************
|*
|*    RscString::RscString()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
RscString::RscString( HASHID nId, USHORT nTypeId )
                : RscTop( nId, nTypeId )
{
    nSize = ALIGNED_SIZE( sizeof( RscStringInst ) );
    pRefClass = NULL;
}

/*************************************************************************
|*
|*    RscString::GetClassType()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
RSCCLASS_TYPE RscString::GetClassType() const
{
    return RSCCLASS_STRING;
}

/*************************************************************************
|*
|*    RscString::SetNumber()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
ERRTYPE RscString::SetString( const RSCINST & rInst, char * pStr ){
    char    * pTmp;
    ERRTYPE aError;

    if( aError.IsOk() ){
        ((RscStringInst *)rInst.pData)->bDflt = FALSE;

        pTmp = ((RscStringInst *)rInst.pData)->pStr;
        if( pTmp ){
            RscMem::Free( pTmp );
            pTmp = NULL;
        }

        if( pStr ){
            USHORT  nLen = strlen( pStr ) +1;
            pTmp = (char *)RscMem::Malloc( nLen );
            memcpy( pTmp, pStr, nLen );
        };

        ((RscStringInst *)rInst.pData)->pStr = pTmp;
    }

    return( aError );
}

/*************************************************************************
|*
|*    RscString::GetString()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
ERRTYPE RscString::GetString( const RSCINST & rInst, char ** ppStr ){
    *ppStr = ((RscStringInst *)rInst.pData)->pStr;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscString::GetRef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.07.91
|*    Letzte Aenderung  MM 22.07.91
|*
*************************************************************************/
ERRTYPE RscString::GetRef( const RSCINST & rInst, RscId * pRscId ){
    *pRscId = ((RscStringInst *)rInst.pData)->aRefId;
    return( ERR_OK );
}

/*************************************************************************
|*
|*    RscString::SetRef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
|*
*************************************************************************/
ERRTYPE RscString::SetRef( const RSCINST & rInst, const RscId & rRefId ){
    if( pRefClass ){
        ((RscStringInst *)rInst.pData)->aRefId = rRefId;
        ((RscStringInst *)rInst.pData)->bDflt  = FALSE;
    }
    else
        return( ERR_REFNOTALLOWED );

    return ERR_OK;
}

/*************************************************************************
|*
|*    RscString::Create()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
RSCINST RscString::Create( RSCINST * pInst, const RSCINST & rDflt,
                           BOOL bOwnClass )
{
    RSCINST aInst;

    if( !pInst ){
        aInst.pClass = this;
        aInst.pData = (CLASS_DATA)
                      RscMem::Malloc( sizeof( RscStringInst ) );
    }
    else
        aInst = *pInst;
    if( !bOwnClass && rDflt.IsInst() )
        bOwnClass = rDflt.pClass->InHierarchy( this );

    ((RscStringInst *)aInst.pData)->aRefId.Create();
    ((RscStringInst *)aInst.pData)->pStr = NULL;
    ((RscStringInst *)aInst.pData)->bDflt = TRUE;

    if( bOwnClass ){
        ((RscStringInst *)aInst.pData)->aRefId =
                          ((RscStringInst *)rDflt.pData)->aRefId;
        SetString( aInst, ((RscStringInst *)rDflt.pData)->pStr );
        ((RscStringInst *)aInst.pData)->bDflt =
                            ((RscStringInst *)rDflt.pData)->bDflt ;
    }

    return( aInst );
}

/*************************************************************************
|*
|*    RscString::Destroy()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.05.91
|*    Letzte Aenderung  MM 15.05.91
|*
*************************************************************************/
void RscString::Destroy( const RSCINST & rInst ){
    if( ((RscStringInst *)rInst.pData)->pStr )
        RscMem::Free( ((RscStringInst *)rInst.pData)->pStr );
    ((RscStringInst *)rInst.pData)->aRefId.Destroy();
}

/*************************************************************************
|*
|*    RscString::IsValueDefault()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.01.92
|*    Letzte Aenderung  MM 15.01.92
|*
*************************************************************************/
BOOL RscString::IsValueDefault( const RSCINST & rInst, CLASS_DATA pDef ){
    RscStringInst * pData    = (RscStringInst*)rInst.pData;
    RscStringInst * pDefData = (RscStringInst*)pDef;

    if( pDef ){
        if( pData->aRefId.IsId() || pDefData->aRefId.IsId() )
        {
            if( pData->aRefId.aExp.IsNumber()
              && pDefData->aRefId.aExp.IsNumber() )
            {
                // Sind die Referenzidentifier gleich
                if( pData->aRefId.GetNumber() == pDefData->aRefId.GetNumber() )
                {
                    return TRUE;
                }
            }
        }
        else {
            BOOL bStrEmpty = FALSE;
            BOOL bDefStrEmpty = FALSE;

            if( pData->pStr ){
                bStrEmpty = ('\0' == *pData->pStr);
            }
            else
                bStrEmpty = TRUE;

            if( pDefData->pStr ){
                bDefStrEmpty = ('\0' == *pDefData->pStr);
            }
            else
                bDefStrEmpty = TRUE;

            if( !bStrEmpty || !bDefStrEmpty ){
                return FALSE;
            }
            else if( !(bStrEmpty || bDefStrEmpty) ){
                if( strcmp( pData->pStr, pDefData->pStr ) )
                    return FALSE;
            }
            return TRUE;
        }
    }

    return FALSE;
}

/*************************************************************************
|*
|*    RscString::WriteSrc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 25.04.91
|*    Letzte Aenderung  MM 25.04.91
|*
*************************************************************************/
void RscString::WriteSrc( const RSCINST & rInst, FILE * fOutput,
                          RscTypCont *, USHORT, const char * )
{
    if ( ((RscStringInst *)rInst.pData)->aRefId.IsId() )
    {
        fprintf( fOutput, "%s",
                 ((RscStringInst *)rInst.pData)->aRefId.GetName().GetBuffer() );
    }
    else
    {
        RscStringInst * pStrI = ((RscStringInst *)rInst.pData);
        if(  pStrI->pStr ){
            //char *  pChangeTab = RscChar::GetChangeTab();
            USHORT  n = 0;
            USHORT  nPos, nSlashPos;

            do {
                fputc( '\"', fOutput );
                nSlashPos = nPos = 0;
                while( pStrI->pStr[ n ]
                  && (nPos < 72 || nPos - nSlashPos <= 3) )
                { // nach \ mindesten 3 Zeichen wegeb \xa7
                    fputc( pStrI->pStr[ n ], fOutput );
                    if( pStrI->pStr[ n ] == '\\' )
                        nSlashPos = nPos;
                    n++;
                    nPos++;
                }

                fputc( '\"', fOutput );
                if( pStrI->pStr[ n ] ) //nocht nicht zu ende
                    fputc( '\n', fOutput );
            } while( pStrI->pStr[ n ] );
        }
        else
            fprintf( fOutput, "\"\"" );
        /*
        if( pStrI->pStr )
        {
            UINT16 nStrLen = strlen( pStrI->pStr );
            sal_Unicode *   pUniCode = new sal_Unicode[ nStrLen +1 ];
            rtl_TextToUnicodeConverter hConv = rtl_createTextToUnicodeConverter( RTL_TEXTENCODING_UTF8 );

            sal_uInt32 nInfo;
            sal_Size   nSrcCvtBytes;
            sal_Size nSize = rtl_convertTextToUnicode( hConv, 0,
                                                        pStrI->pStr, nStrLen,
                                                        pUniCode, nStrLen,
                                                        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT
                                                        | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT
                                                        | RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT
                                                        | RTL_TEXTTOUNICODE_FLAGS_FLUSH,
                                                        &nInfo,
                                                        &nSrcCvtBytes );
            fputc( 'L', fOutput );
            fputc( '\"', fOutput );
            for( sal_Size i = 0; i < nSize; i++ )
            {
                if( pUniCode[i] >= 32 && pUniCode[i] < 128 )
                {
                    if( pUniCode[i] == '\\' )
                    {
                        fputc( (char)pUniCode[i], fOutput );
                        fputc( (char)pUniCode[i], fOutput );
                    }
                    else if( pUniCode[i] == '\"' )
                    {
                        fputc( '\\', fOutput );
                        fputc( '"', fOutput );
                    }
                    else
                        fputc( (char)pUniCode[i], fOutput );
                }
                else
                    fprintf( fOutput, "\\x%4.4lX", (long)pUniCode[i] );
                if( !((i +1) % 40) && i +1 != nSize )
                    fprintf( fOutput, "\"\n\t\t\tL\"" );
            }
            fputc( '\"', fOutput );

            rtl_destroyTextToUnicodeConverter( hConv );
            delete pUniCode;
        }
        else
            fprintf( fOutput, "\"\"" );
        */
    };
}

/*************************************************************************
|*
|*    RscString::WriteRc()
|*
|*    Beschreibung
|*    Ersterstellung    MM 15.04.91
|*    Letzte Aenderung  MM 15.04.91
|*
*************************************************************************/
ERRTYPE RscString::WriteRc( const RSCINST & rInst, RscWriteRc & rMem,
                            RscTypCont * pTC, USHORT nDeep, BOOL bExtra )
{
    ERRTYPE aError;
    ObjNode *       pObjNode = NULL;


    if( ((RscStringInst *)rInst.pData)->aRefId.IsId() ){
        RscId   aId( ((RscStringInst *)rInst.pData)->aRefId );
        RSCINST aTmpI;

        aTmpI.pClass = pRefClass;

        while( aError.IsOk() && aId.IsId() ){
            //Erhoehen und abfragen um Endlosrekusion zu vermeiden
            nDeep++;
            if( nDeep > nRefDeep )
                aError = ERR_REFTODEEP;
            else
            {
                pObjNode = pRefClass->GetObjNode( aId );
                if( pObjNode )
                {
                    aTmpI.pData = pObjNode->GetRscObj();
                    aError = pRefClass->GetRef( aTmpI, &aId );
                }
                else
                {
                    if( pTC )
                    {
                        ByteString  aMsg( pHS->Get( pRefClass->GetId() ) );
                        aMsg += ' ';
                        aMsg += aId.GetName();
                        aError = WRN_STR_REFNOTFOUND;
                        pTC->pEH->Error( aError, rInst.pClass,
                                         RscId(), aMsg.GetBuffer() );
                    }
                    break;
                }
            }
        }
    }

    if( aError.IsOk() )
    {
        if( pObjNode )
        {
            RSCINST     aRefI;

            aRefI = RSCINST( pRefClass, pObjNode->GetRscObj() );
            aError = aRefI.pClass->WriteRc( aRefI, rMem, pTC, nDeep, bExtra );
        }
        else
        {
            if( ((RscStringInst *)rInst.pData)->pStr )
            {
                char * pStr = RscChar::MakeUTF8( ((RscStringInst *)rInst.pData)->pStr,
                                                pTC->GetSourceCharSet() );
                rMem.PutUTF8( pStr );
                RscMem::Free( pStr );
            }
            else
                rMem.PutUTF8( ((RscStringInst *)rInst.pData)->pStr );
            //rMem.PutUTF8( ((RscStringInst *)rInst.pData)->pStr );
        };
    };
    return( aError );
}

//==================================================================
void RscString::WriteRcAccess
(
    FILE * fOutput,
    RscTypCont * pTC,
    const char * pName
)
{
    fprintf( fOutput, "\t\tString aStr( (const char*)(pResData+nOffset) );\n" );
    fprintf( fOutput, "\t\tSet%s( aStr );\n", pName );
    fprintf( fOutput, "\t\tnOffset += GetStringSizeRes( aStr );\n" );
}

