/*************************************************************************
 *
 *  $RCSfile: convuno.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-09 09:30:31 $
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

#ifndef SC_CONVUNO_HXX
#define SC_CONVUNO_HXX

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

#ifndef _COM_SUN_STAR_TABLE_CELLADDRESS_HPP_
#include <com/sun/star/table/CellAddress.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_CELLRANGEADDRESS_HPP_
#include <com/sun/star/table/CellRangeAddress.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class ScUnoConversion
{
public:
    static LanguageType GetLanguage( const com::sun::star::lang::Locale& rLocale );
    static void FillLocale( com::sun::star::lang::Locale& rLocale, LanguageType eLang );

    // CellAddress -> ScAddress
    static inline void  FillScAddress(
                            ScAddress& rScAddress,
                            const ::com::sun::star::table::CellAddress& rApiAddress );
    // ScAddress -> CellAddress
    static inline void  FillApiAddress(
                            ::com::sun::star::table::CellAddress& rApiAddress,
                            const ScAddress& rScAddress );
    // CellRangeAddress -> ScRange
    static inline void  FillScRange(
                            ScRange& rScRange,
                            const ::com::sun::star::table::CellRangeAddress& rApiRange );
    // ScRange -> CellRangeAddress
    static inline void  FillApiRange(
                            ::com::sun::star::table::CellRangeAddress& rApiRange,
                            const ScRange& rScRange );
    // CellAddress -> CellRangeAddress
    static inline void  FillApiRange(
                            ::com::sun::star::table::CellRangeAddress& rApiRange,
                            const ::com::sun::star::table::CellAddress& rApiAddress );
    // CellRangeAddress-Start -> CellAddress
    static inline void  FillApiStartAddress(
                            ::com::sun::star::table::CellAddress& rApiAddress,
                            const ::com::sun::star::table::CellRangeAddress& rApiRange );
    // CellRangeAddress-End -> CellAddress
    static inline void  FillApiEndAddress(
                            ::com::sun::star::table::CellAddress& rApiAddress,
                            const ::com::sun::star::table::CellRangeAddress& rApiRange );
};


inline void ScUnoConversion::FillScAddress(
        ScAddress& rScAddress,
        const ::com::sun::star::table::CellAddress& rApiAddress )
{
    rScAddress.Set( rApiAddress.Column, rApiAddress.Row, rApiAddress.Sheet );
}

inline void ScUnoConversion::FillApiAddress(
        ::com::sun::star::table::CellAddress& rApiAddress,
        const ScAddress& rScAddress )
{
    rApiAddress.Column = rScAddress.Col();
    rApiAddress.Row = rScAddress.Row();
    rApiAddress.Sheet = rScAddress.Tab();
}

inline void ScUnoConversion::FillScRange(
        ScRange& rScRange,
        const ::com::sun::star::table::CellRangeAddress& rApiRange )
{
    rScRange.aStart.Set( rApiRange.StartColumn, rApiRange.StartRow, rApiRange.Sheet );
    rScRange.aEnd.Set( rApiRange.EndColumn, rApiRange.EndRow, rApiRange.Sheet );
}

inline void ScUnoConversion::FillApiRange(
        ::com::sun::star::table::CellRangeAddress& rApiRange,
        const ScRange& rScRange )
{
    rApiRange.StartColumn = rScRange.aStart.Col();
    rApiRange.StartRow = rScRange.aStart.Row();
    rApiRange.Sheet = rScRange.aStart.Tab();
    rApiRange.EndColumn = rScRange.aEnd.Col();
    rApiRange.EndRow = rScRange.aEnd.Row();
}

inline void ScUnoConversion::FillApiRange(
        ::com::sun::star::table::CellRangeAddress& rApiRange,
        const ::com::sun::star::table::CellAddress& rApiAddress )
{
    rApiRange.StartColumn = rApiRange.EndColumn = rApiAddress.Column;
    rApiRange.StartRow = rApiRange.EndRow = rApiAddress.Row;
    rApiRange.Sheet = rApiAddress.Sheet;
}

inline void ScUnoConversion::FillApiStartAddress(
        ::com::sun::star::table::CellAddress& rApiAddress,
        const ::com::sun::star::table::CellRangeAddress& rApiRange )
{
    rApiAddress.Column = rApiRange.StartColumn;
    rApiAddress.Row = rApiRange.StartRow;
    rApiAddress.Sheet = rApiRange.Sheet;
}

inline void ScUnoConversion::FillApiEndAddress(
        ::com::sun::star::table::CellAddress& rApiAddress,
        const ::com::sun::star::table::CellRangeAddress& rApiRange )
{
    rApiAddress.Column = rApiRange.EndColumn;
    rApiAddress.Row = rApiRange.EndRow;
    rApiAddress.Sheet = rApiRange.Sheet;
}

//___________________________________________________________________

inline sal_Bool operator==(
        const ::com::sun::star::table::CellAddress& rApiAddress1,
        const ::com::sun::star::table::CellAddress& rApiAddress2 )
{
    return
        (rApiAddress1.Column == rApiAddress2.Column) &&
        (rApiAddress1.Row == rApiAddress2.Row) &&
        (rApiAddress1.Sheet == rApiAddress2.Sheet);
}

inline sal_Bool operator!=(
        const ::com::sun::star::table::CellAddress& rApiAddress1,
        const ::com::sun::star::table::CellAddress& rApiAddress2 )
{
    return !(rApiAddress1 == rApiAddress2);
}

inline sal_Bool operator==(
        const ::com::sun::star::table::CellRangeAddress& rApiRange1,
        const ::com::sun::star::table::CellRangeAddress& rApiRange2 )
{
    return
        (rApiRange1.StartColumn == rApiRange2.StartColumn) &&
        (rApiRange1.StartRow == rApiRange2.StartRow) &&
        (rApiRange1.EndColumn == rApiRange2.EndColumn) &&
        (rApiRange1.EndRow == rApiRange2.EndRow) &&
        (rApiRange1.Sheet == rApiRange2.Sheet);
}

inline sal_Bool operator!=(
        const ::com::sun::star::table::CellRangeAddress& rApiRange1,
        const ::com::sun::star::table::CellRangeAddress& rApiRange2 )
{
    return !(rApiRange1 == rApiRange2);
}

#endif

