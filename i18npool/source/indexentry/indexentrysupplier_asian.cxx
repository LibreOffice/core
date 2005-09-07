/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: indexentrysupplier_asian.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:09:15 $
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

#include <indexentrysupplier_asian.hxx>
#include <data/indexdata_alphanumeric.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL IndexEntrySupplier_CJK::getIndexString( const sal_Unicode ch,
    const sal_Unicode CJK_idxStr[], const sal_uInt16 idx1[], const sal_uInt16 idx2[])
    throw (RuntimeException)
{
    sal_uInt16 first = idx1[ ch >> 8 ];
    return first == 0xFFFF ?
        // using alphanumeric index for non-define stirng
        OUString(&idxStr[(ch & 0xFF00) ? 0 : ch], 1) :
        OUString(&CJK_idxStr[idx2[ first + (ch & 0xff) ]]);
}

OUString SAL_CALL IndexEntrySupplier_CJK::getIndexString( const sal_Unicode ch,
    const sal_uInt16 idx1[], const sal_Unicode idx2[]) throw (RuntimeException)
{
    sal_uInt16 first = idx1[ ch >> 8 ];
    return first == 0xFFFF ?
        // using alphanumeric index for non-define stirng
        OUString(&idxStr[(ch & 0xFF00) ? 0 : ch], 1) :
        OUString(&idx2[ first + (ch & 0xff) ], 1);
}

} } } }
