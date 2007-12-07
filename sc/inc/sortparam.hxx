/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sortparam.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:41:17 $
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

#ifndef SC_SORTPARAM_HXX
#define SC_SORTPARAM_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#define MAXSORT     3


struct ScSubTotalParam;
struct ScQueryParam;

struct SC_DLLPUBLIC ScSortParam
{
    SCCOL       nCol1;
    SCROW       nRow1;
    SCCOL       nCol2;
    SCROW       nRow2;
    BOOL        bHasHeader;
    BOOL        bByRow;
    BOOL        bCaseSens;
    BOOL        bUserDef;
    USHORT      nUserIndex;
    BOOL        bIncludePattern;
    BOOL        bInplace;
    SCTAB       nDestTab;
    SCCOL       nDestCol;
    SCROW       nDestRow;
    BOOL        bDoSort[MAXSORT];
    SCCOLROW    nField[MAXSORT];
    BOOL        bAscending[MAXSORT];
    ::com::sun::star::lang::Locale      aCollatorLocale;
    String      aCollatorAlgorithm;
    USHORT      nCompatHeader;

    ScSortParam();
    ScSortParam( const ScSortParam& r );
    /// SubTotals sort
    ScSortParam( const ScSubTotalParam& rSub, const ScSortParam& rOld );
    /// TopTen sort
    ScSortParam( const ScQueryParam&, SCCOL nCol );

    ScSortParam&    operator=   ( const ScSortParam& r );
    BOOL            operator==  ( const ScSortParam& rOther ) const;
    void            Clear       ();

    void            MoveToDest();
};


#endif // SC_SORTPARAM_HXX
