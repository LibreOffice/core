/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impastp2.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <rtl/ustrbuf.hxx>
#include "impastpl.hxx"

using namespace std;
using namespace rtl;

//#############################################################################
//
// Class SvXMLAutoStylePoolProperties_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// ctor class SvXMLAutoStylePoolProperties_Impl
//

SvXMLAutoStylePoolPropertiesP_Impl::SvXMLAutoStylePoolPropertiesP_Impl( XMLFamilyData_Impl* pFamilyData, const vector< XMLPropertyState >& rProperties )
: maProperties( rProperties ),
  mnPos       ( pFamilyData->mnCount )
{
    // create a name that hasn't been used before. The created name has not
    // to be added to the array, because it will never tried again
    OUStringBuffer sBuffer( 7 );
    do
    {
        pFamilyData->mnName++;
        sBuffer.append( pFamilyData->maStrPrefix );
        sBuffer.append( OUString::valueOf( (sal_Int32)pFamilyData->mnName ) );
        msName = sBuffer.makeStringAndClear();
    }
    while( pFamilyData->mpNameList->Seek_Entry( &msName, 0 ) );
}
