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

#ifndef SC_SORTPARAM_HXX
#define SC_SORTPARAM_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
namespace binfilter {


#define MAXSORT		3


struct ScSubTotalParam;
struct ScQueryParam;

struct ScSortParam
{
    USHORT		nCol1;
    USHORT		nRow1;
    USHORT		nCol2;
    USHORT		nRow2;
    BOOL		bHasHeader;
    BOOL		bByRow;
    BOOL		bCaseSens;
    BOOL		bUserDef;
    USHORT		nUserIndex;
    BOOL		bIncludePattern;
    BOOL		bInplace;
    USHORT		nDestTab;
    USHORT		nDestCol;
    USHORT		nDestRow;
    BOOL		bDoSort[MAXSORT];
    USHORT		nField[MAXSORT];
    BOOL		bAscending[MAXSORT];
    ::com::sun::star::lang::Locale		aCollatorLocale;
    String		aCollatorAlgorithm;

    ScSortParam();
    ScSortParam( const ScSortParam& r );
    /// SubTotals sort
    /// TopTen sort

    void			Clear		();

};


} //namespace binfilter
#endif // SC_SORTPARAM_HXX
