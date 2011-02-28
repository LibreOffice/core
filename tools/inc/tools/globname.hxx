/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _GLOBNAME_HXX
#define _GLOBNAME_HXX

#include <vector>

#include "tools/toolsdllapi.h"
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/string.hxx>

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
    std::vector<ImpSvGlobalName*> aList;
public:
                    SvGlobalNameList();
                    ~SvGlobalNameList();

    void            Append( const SvGlobalName & );
    SvGlobalName    GetObject( ULONG );
    BOOL            IsEntry( const SvGlobalName & rName );
    ULONG           Count() const { return aList.size(); }
private:
                // nicht erlaubt
                SvGlobalNameList( const SvGlobalNameList & );
    SvGlobalNameList & operator = ( const SvGlobalNameList & );
};

#endif // _GLOBNAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
