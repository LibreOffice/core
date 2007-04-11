/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: globname.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:12:02 $
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
#ifndef _GLOBNAME_HXX
#define _GLOBNAME_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

/*************************************************************************
*************************************************************************/
struct ImpSvGlobalName
{
    BYTE        szData[ 16 ];
    USHORT      nRefCount;

                ImpSvGlobalName()
                {
                    nRefCount = 0;
                }
                ImpSvGlobalName( const ImpSvGlobalName & rObj );
                ImpSvGlobalName( int );

    BOOL    operator == ( const ImpSvGlobalName & rObj ) const;
};

#ifdef WNT
struct _GUID;
typedef struct _GUID GUID;
#else
struct GUID;
#endif
typedef GUID CLSID;
class SvStream;
class SvGlobalNameList;
class TOOLS_DLLPUBLIC SvGlobalName
{
friend class SvGlobalNameList;
    ImpSvGlobalName * pImp;
    void    NewImp();
public:
            SvGlobalName();
            SvGlobalName( const SvGlobalName & rObj )
            {
                pImp = rObj.pImp;
                pImp->nRefCount++;
            }
            SvGlobalName( ImpSvGlobalName * pImpP )
            {
                pImp = pImpP;
                pImp->nRefCount++;
            }
            SvGlobalName( UINT32 n1, USHORT n2, USHORT n3,
                          BYTE b8, BYTE b9, BYTE b10, BYTE b11,
                          BYTE b12, BYTE b13, BYTE b14, BYTE b15 );

            // create SvGlobalName from a platform independent representation
            SvGlobalName( const ::com::sun::star::uno::Sequence< sal_Int8 >& aSeq );

    SvGlobalName & operator = ( const SvGlobalName & rObj );
            ~SvGlobalName();

    TOOLS_DLLPUBLIC friend SvStream & operator >> ( SvStream &, SvGlobalName & );
    TOOLS_DLLPUBLIC friend SvStream & operator << ( SvStream &, const SvGlobalName & );

    BOOL            operator < ( const SvGlobalName & rObj ) const;
    SvGlobalName &  operator += ( UINT32 );
    SvGlobalName &  operator ++ () { return operator += ( 1 ); }

    BOOL    operator == ( const SvGlobalName & rObj ) const;
    BOOL    operator != ( const SvGlobalName & rObj ) const
            { return !(*this == rObj); }

    void    MakeFromMemory( void * pData );
    BOOL    MakeId( const String & rId );
    String  GetctorName() const;
    String  GetHexName() const;
    String  GetRegDbName() const
            {
                String a = '{';
                a += GetHexName();
                a += '}';
                return a;
            }

                  SvGlobalName( const CLSID & rId );
    const CLSID & GetCLSID() const { return *(CLSID *)pImp->szData; }
    const BYTE* GetBytes() const { return pImp->szData; }

    // platform independent representation of a "GlobalName"
    // maybe transported remotely
    com::sun::star::uno::Sequence < sal_Int8 > GetByteSequence() const;
};

class SvGlobalNameList
{
    List aList;
public:
                    SvGlobalNameList();
                    ~SvGlobalNameList();

    void            Append( const SvGlobalName & );
    SvGlobalName    GetObject( ULONG );
    BOOL            IsEntry( const SvGlobalName & rName );
    ULONG           Count() const { return aList.Count(); }
private:
                // nicht erlaubt
                SvGlobalNameList( const SvGlobalNameList & );
    SvGlobalNameList & operator = ( const SvGlobalNameList & );
};

#endif // _GLOBNAME_HXX

