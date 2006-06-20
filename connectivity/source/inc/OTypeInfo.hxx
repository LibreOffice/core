/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OTypeInfo.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:57:41 $
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

#ifndef _CONNECTIVITY_OTYPEINFO_HXX_
#define _CONNECTIVITY_OTYPEINFO_HXX_

#ifndef _COM_SUN_STAR_SDBC_COLUMNSEARCH_HPP_
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif

namespace connectivity
{
    struct OTypeInfo
    {
        ::rtl::OUString aTypeName;      // Name des Types in der Datenbank
        ::rtl::OUString aLiteralPrefix; // Prefix zum Quoten
        ::rtl::OUString aLiteralSuffix; // Suffix zum Quoten
        ::rtl::OUString aCreateParams;  // Parameter zum Erstellen
        ::rtl::OUString aLocalTypeName;

        sal_Int32       nPrecision;     // Laenge des Types

        sal_Int16       nMaximumScale;  // Nachkommastellen
        sal_Int16       nMinimumScale;  // Min Nachkommastellen

        sal_Int16       nType;          // Datenbanktyp
        sal_Int16       nSearchType;    // kann nach dem Typen gesucht werden
        sal_Int16       nNumPrecRadix;  // indicating the radix, which is usually 2 or 10

        sal_Bool        bCurrency       : 1,    // Waehrung
                        bAutoIncrement  : 1,    // Ist es ein automatisch incrementierendes Feld
                        bNullable       : 1,    // Kann das Feld NULL annehmen
                        bCaseSensitive  : 1,    // Ist der Type Casesensitive
                        bUnsigned       : 1,    // Ist der Type Unsigned
                        bEmpty_1        : 1,    // for later use
                        bEmpty_2        : 1;

        OTypeInfo()
                :nPrecision(0)
                ,nMaximumScale(0)
                ,nMinimumScale(0)
                ,nType( ::com::sun::star::sdbc::DataType::OTHER)
                ,nSearchType( ::com::sun::star::sdbc::ColumnSearch::FULL)
                ,bCurrency(sal_False)
                ,bAutoIncrement(sal_False)
                ,bNullable(sal_True)
                ,bCaseSensitive(sal_False)
                ,bUnsigned(sal_False)
        {}

        inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint ) SAL_THROW( () )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ ) SAL_THROW( () )
            {  }

        sal_Bool operator == (const OTypeInfo& lh) const { return lh.nType == nType; }
        sal_Bool operator != (const OTypeInfo& lh) const { return lh.nType != nType; }

        inline ::rtl::OUString getDBName() const { return aTypeName; }
    };
}
#endif // _CONNECTIVITY_OTYPEINFO_HXX_


