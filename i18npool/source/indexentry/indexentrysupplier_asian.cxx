/*************************************************************************
 *
 *  $RCSfile: indexentrysupplier_asian.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: khong $ $Date: 2002-06-18 22:29:26 $
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

#include <indexentrysupplier_asian.hxx>
#include <data/indexdata_alphanumeric.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
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

// for CJK we only need to compare index entry, which contains key information in first characters implicitly.
sal_Int16 SAL_CALL IndexEntrySupplier_CJK::compareIndexKey(
    const OUString& rIndexEntry1, const OUString& rPhoneticEntry1, const Locale& rLocale1,
    const OUString& rIndexEntry2, const OUString& rPhoneticEntry2, const Locale& rLocale2 )
    throw (RuntimeException)
{
    return 0;
}

} } } }
