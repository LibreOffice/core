/*************************************************************************
 *
 *  $RCSfile: hash.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

#ifndef _HASH_HXX
#define _HASH_HXX



#ifndef _REF_HXX //autogen
#include <tools/ref.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

/****************** H a s h - T a b l e **********************************/
class SvHashTable
{
    UINT32       nMax;                 // size of hash-tabel
    UINT32       nFill;                // elements in hash-tabel
    UINT32       lAsk;                 // Anzahl der Anfragen
    UINT32       lTry;                 // Anzahl der Versuche
protected:
    BOOL        Test_Insert( const void *, BOOL bInsert, UINT32 * pInsertPos );

                            // compare element with entry
    virtual StringCompare   Compare( const void * , UINT32 ) const = 0;
                            // get hash value from subclass
    virtual UINT32          HashFunc( const void * ) const = 0;
public:
                SvHashTable( UINT32 nMaxEntries );
                ~SvHashTable();

    UINT32      GetMax() const { return nMax; }

    virtual BOOL            IsEntry( UINT32 ) const = 0;
};

/************** S t r i n g H a s h T a b l e E n t r y ******************/
class SvStringHashTable;
class SvStringHashEntry : public SvRefBase
{
friend SvStringHashTable;
    ByteString  aName;
    UINT32  nHashId;
    ULONG   nValue;
    BOOL    bHasId;
public:
                    SvStringHashEntry() : bHasId( FALSE ) {;}
                    SvStringHashEntry( const ByteString & rName, UINT32 nIdx )
                        : aName( rName )
                        , nHashId( nIdx )
                        , bHasId( TRUE )
                        , nValue( 0 ) {}
                    ~SvStringHashEntry();

    const ByteString &  GetName() const { return aName; }
    BOOL            HasId() const { return bHasId; }
    UINT32          GetId() const { return nHashId; }

    void            SetValue( ULONG n ) { nValue = n; }
    ULONG           GetValue() const { return nValue; }

    BOOL            operator == ( const SvStringHashEntry & rRef )
                    { return nHashId == rRef.nHashId; }
    BOOL            operator != ( const SvStringHashEntry & rRef )
                    { return ! operator == ( rRef ); }
    SvStringHashEntry & operator = ( const SvStringHashEntry & rRef )
        { SvRefBase::operator=( rRef );
          aName   = rRef.aName;
          nHashId = rRef.nHashId;
          nValue  = rRef.nValue;
          bHasId  = rRef.bHasId;
          return *this;
        }
};

SV_DECL_IMPL_REF(SvStringHashEntry)

/****************** S t r i n g H a s h T a b l e ************************/
DECLARE_LIST(SvStringHashList,SvStringHashEntry *)

class SvStringHashTable : public SvHashTable
{
    SvStringHashEntry * pEntries;
protected:
    virtual UINT32          HashFunc( const void * pElement ) const;
    virtual StringCompare   Compare( const void * pElement, UINT32 nIndex ) const;
public:
            SvStringHashTable( UINT32 nMaxEntries );   // max size of hash-tabel
            ~SvStringHashTable();

    ByteString          GetNearString( const ByteString & rName ) const;
    virtual BOOL    IsEntry( UINT32 nIndex ) const;

    BOOL    Insert( const ByteString & rStr, UINT32 * pHash ); // insert string
    BOOL    Test( const ByteString & rStr, UINT32 * pHash ) const; // test of insert string
    SvStringHashEntry * Get ( UINT32 nIndex ) const; // return pointer to string
    SvStringHashEntry & operator []( UINT32 nPos ) const
            { return pEntries[ nPos ]; }

    void    FillHashList( SvStringHashList * rList ) const;
};

#endif // _RSCHASH_HXX
