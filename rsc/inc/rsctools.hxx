/*************************************************************************
 *
 *  $RCSfile: rsctools.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:55 $
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

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/inc/rsctools.hxx,v 1.1.1.1 2000-09-18 16:42:55 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.16  2000/09/17 12:51:09  willem.vandorp
    OpenOffice header added.

    Revision 1.15  2000/07/26 17:13:17  willem.vandorp
    Headers/footers replaced

    Revision 1.14  2000/07/11 16:59:41  th
    Unicode

    Revision 1.13  1999/09/20 17:25:29  pl
    PutAt mit short

    Revision 1.12  1999/09/08 11:47:39  mm
    BigEndian/LittleEndian corrected

**************************************************************************/
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
#define HASH_NONAME         0xFFFF  //not an entry in hashtabel
typedef USHORT              HASHID; // Definition von HASHID
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
                     USHORT nArgc );
void RscExit( USHORT nExit );

/********* A n s i - F u n c t i o n   F o r w a r d s *******************/
int rsc_strnicmp( const char *string1, const char *string2, size_t count );
int rsc_stricmp( const char *string1, const char *string2 );

/****************** C L A S S E S ****************************************/
DECLARE_LIST( RscStrList, ByteString * )
/*********** R s c C h a r ***********************************************/
class RscChar
{
public:
    static char * MakeUTF8( char * pStr, UINT16 nTextEncoding );
    static char * MakeUTF8FromL( char * pStr );
};

/*********** R s c M e m *************************************************/
class RscMem
{
public:
    static void *   Malloc( USHORT nSize );
    static void *   Realloc( void * pMem, USHORT nSize );
    static char *   Realloc( char * pMem, USHORT nSize ){
                        return (char *)Realloc( (void *)pMem, nSize );
                    }
    static void     Free( void * pMem );
    static void     Free( char * pMem ){
                        Free( (void *)pMem );
                    }
    static char *   Assignsw( const char *psw, short nExtraSpace );
};

/****************** R s c P t r P t r ************************************/
class RscPtrPtr
{
    USHORT  nCount;
    void **         pMem;
public:
                    RscPtrPtr();
                    ~RscPtrPtr();
    void            Reset();
    USHORT  Append( void * );
    USHORT  Append( char * pStr ){
                        return( Append( (void *)pStr ) );
                    };
    USHORT  GetCount(){ return( nCount ); };
    void *          GetEntry( USHORT nEle );
    void **         GetBlock(){ return( pMem ); };
};

/****************** R s c W r i t e R c **********************************/
class RscWriteRc
{
    USHORT              nLen;
    BOOL                bSwap;
    RSCBYTEORDER_TYPE   nByteOrder;
    char *              pMem;
    char *              GetPointer( USHORT nSize );
public:
                RscWriteRc( RSCBYTEORDER_TYPE nOrder = RSC_SYSTEMENDIAN );
                ~RscWriteRc();
    USHORT      IncSize( USHORT nSize );// gibt die vorherige Groesse
    void *      GetBuffer()
                {
                    return GetPointer( 0 );
                }
    USHORT      GetShort( USHORT nPos )
                {
                    return bSwap ? SWAPSHORT( *(USHORT*)(GetPointer(nPos)) ) : *(USHORT*)(GetPointer(nPos));
                }
    char *      GetUTF8( USHORT nPos )
                {
                    return GetPointer( nPos );
                }


    RSCBYTEORDER_TYPE GetByteOrder() const { return nByteOrder; }
    USHORT      Size(){ return( nLen ); };
    //void        Put( void * pData, USHORT nSize );
    void        Put( INT32 lVal )
                {
                    if( bSwap )
                    {
                        Put( *(((USHORT*)&lVal) +1) );
                        Put( *(USHORT*)&lVal );
                    }
                    else
                    {
                        Put( *(USHORT*)&lVal );
                        Put( *(((USHORT*)&lVal) +1) );
                    }
                }
    void        Put( UINT32 nValue )
                { Put( (INT32)nValue ); }
    void        Put( USHORT nValue );
    void        Put( short nValue )
                { Put( (USHORT)nValue ); }
    void        PutUTF8( char * pData );

    void        PutAt( USHORT nPos, INT32 lVal )
                {
                    if( bSwap )
                    {
                        PutAt( nPos, *(((USHORT*)&lVal) +1) );
                        PutAt( nPos + 2, *(USHORT*)&lVal );
                    }
                    else
                    {
                        PutAt( nPos, *(USHORT*)&lVal );
                        PutAt( nPos + 2, *(((USHORT*)&lVal) +1) );
                    }
                }
    void        PutAt( USHORT nPos, short nVal )
                {
                    PutAt( nPos, (USHORT)nVal );
                }
    void        PutAt( USHORT nPos, USHORT nVal )
                {
                    *(USHORT *)(GetPointer( nPos )) = bSwap ? SWAPSHORT( nVal ) : nVal;
                }
};

#endif // _RSCTOOLS_HXX
