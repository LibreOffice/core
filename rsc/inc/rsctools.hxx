/*************************************************************************
 *
 *  $RCSfile: rsctools.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-03 17:23:37 $
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
struct RSHEADER_TYPE;
class  RscPtrPtr;

#ifndef _RSCTOOLS_HXX
#define _RSCTOOLS_HXX

#ifdef UNX
#include <stdlib.h>
#endif
#include <stdio.h>

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

/******************* T y p e s *******************************************/
// Zeichensatz
enum COMPARE { LESS = -1, EQUAL = 0, GREATER = 1 };

enum RSCBYTEORDER_TYPE { RSC_BIGENDIAN, RSC_LITTLEENDIAN, RSC_SYSTEMENDIAN };

/******************* M A K R O S *****************************************/
#define ALIGNED_SIZE( nSize )                               \
            (nSize + sizeof( void * ) -1) / sizeof( void * ) * sizeof( void * )
/******************* F u n c t i o n   F o r w a r d s *******************/
ByteString GetTmpFileName();
BOOL Append( ByteString aDestFile, ByteString aSourceFile );
BOOL Append( FILE * fDest, ByteString aSourceFile );
ByteString InputFile ( char * pInput, char * pExt );
ByteString OutputFile( ByteString aInput, char * ext );
char * ResponseFile( RscPtrPtr * ppCmd, char ** ppArgv,
                     sal_uInt32 nArgc );
void RscExit( sal_uInt32 nExit );

/********* A n s i - F u n c t i o n   F o r w a r d s *******************/
int rsc_strnicmp( const char *string1, const char *string2, size_t count );
int rsc_stricmp( const char *string1, const char *string2 );
char* rsc_strdup( const char* );

/****************** C L A S S E S ****************************************/
DECLARE_LIST( RscStrList, ByteString * )
/*********** R s c C h a r ***********************************************/
class RscChar
{
public:
    static char * MakeUTF8( char * pStr, UINT16 nTextEncoding );
    static char * MakeUTF8FromL( char * pStr );
};

/****************** R s c P t r P t r ************************************/
class RscPtrPtr
{
    sal_uInt32  nCount;
    void **         pMem;
public:
                    RscPtrPtr();
                    ~RscPtrPtr();
    void            Reset();
    sal_uInt32  Append( void * );
    sal_uInt32  Append( char * pStr ){
                        return( Append( (void *)pStr ) );
                    };
    sal_uInt32  GetCount(){ return( nCount ); };
    void *          GetEntry( sal_uInt32 nEle );
    void **         GetBlock(){ return( pMem ); };
};

/****************** R s c W r i t e R c **********************************/
class RscWriteRc
{
    sal_uInt32              nLen;
    BOOL                bSwap;
    RSCBYTEORDER_TYPE   nByteOrder;
    char *              pMem;
    char *              GetPointer( sal_uInt32 nSize );
public:
                RscWriteRc( RSCBYTEORDER_TYPE nOrder = RSC_SYSTEMENDIAN );
                ~RscWriteRc();
    sal_uInt32      IncSize( sal_uInt32 nSize );// gibt die vorherige Groesse
    void *      GetBuffer()
                {
                    return GetPointer( 0 );
                }
    sal_uInt16  GetShort( sal_uInt32 nPos )
                {
                    sal_uInt16 nVal = 0;
                    char* pFrom = GetPointer(nPos);
                    char* pTo = (char*)&nVal;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
                    return bSwap ? SWAPSHORT( nVal ) : nVal;
                }
    sal_uInt32  GetLong( sal_uInt32 nPos )
                {
                    sal_uInt32 nVal = 0;
                    char* pFrom = GetPointer(nPos);
                    char* pTo = (char*)&nVal;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
                    return bSwap ? SWAPLONG( nVal ) : nVal;
                }
    char *      GetUTF8( sal_uInt32 nPos )
                {
                    return GetPointer( nPos );
                }


    RSCBYTEORDER_TYPE GetByteOrder() const { return nByteOrder; }
    sal_uInt32      Size(){ return( nLen ); };
    void        Put( sal_uInt64 lVal )
                {
                    if( bSwap )
                    {
                        Put( *(((sal_uInt32*)&lVal)+1) );
                        Put( *(sal_uInt32*)&lVal );
                    }
                    else
                    {
                        Put( *(sal_uInt32*)&lVal );
                        Put( *(((sal_uInt32*)&lVal)+1) );
                    }
                }
    void        Put( sal_Int32 lVal )
                {
                    if( bSwap )
                    {
                        Put( *(((sal_uInt16*)&lVal) +1) );
                        Put( *(sal_uInt16*)&lVal );
                    }
                    else
                    {
                        Put( *(sal_uInt16*)&lVal );
                        Put( *(((sal_uInt16*)&lVal) +1) );
                    }
                }
    void        Put( sal_uInt32 nValue )
                { Put( (sal_Int32)nValue ); }
    void        Put( sal_uInt16 nValue );
    void        Put( sal_Int16 nValue )
                { Put( (sal_uInt16)nValue ); }
    void        PutUTF8( char * pData );

    void        PutAt( sal_uInt32 nPos, INT32 lVal )
                {
                    if( bSwap )
                    {
                        PutAt( nPos, *(((sal_uInt16*)&lVal) +1) );
                        PutAt( nPos + 2, *(sal_uInt16*)&lVal );
                    }
                    else
                    {
                        PutAt( nPos, *(sal_uInt16*)&lVal );
                        PutAt( nPos + 2, *(((sal_uInt16*)&lVal) +1) );
                    }
                }
    void        PutAt( sal_uInt32 nPos, sal_uInt32 lVal )
                {
                    PutAt( nPos, (INT32)lVal);
                }
    void        PutAt( sal_uInt32 nPos, short nVal )
                {
                    PutAt( nPos, (sal_uInt16)nVal );
                }
    void        PutAt( sal_uInt32 nPos, sal_uInt16 nVal )
                {
                    if( bSwap )
                        nVal = SWAPSHORT( nVal );
                    char* pTo = GetPointer( nPos );
                    char* pFrom = (char*)&nVal;
                    *pTo++ = *pFrom++;
                    *pTo++ = *pFrom++;
                }
};

#endif // _RSCTOOLS_HXX
