/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pcrcommon.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_extensions.hxx"
#include "pcrcommon.hxx"
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef EXTENSIONS_PROPRESID_HRC
#include "propresid.hrc"
#endif

/** === begin UNO includes === **/
#include <com/sun/star/util/MeasureUnit.hpp>
/** === end UNO includes === **/
#include <rtl/ustrbuf.hxx>

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::util;

    //========================================================================
    //= HelpIdUrl
    //========================================================================
    //------------------------------------------------------------------------
    SmartId HelpIdUrl::getHelpId( const ::rtl::OUString& _rHelpURL )
    {
        SmartId aSmartHelpId( _rHelpURL );
        if ( 0 == _rHelpURL.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "HID:" ) ) )
            aSmartHelpId = SmartId( _rHelpURL.copy( sizeof( "HID:" ) - 1 ).toInt32() );
        return aSmartHelpId;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString HelpIdUrl::getHelpURL( sal_uInt32 _nHelpId )
    {
        ::rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii( "HID:" );
        aBuffer.append( (sal_Int32)_nHelpId );
        return aBuffer.makeStringAndClear();
    }
//............................................................................
} // namespace pcr
//............................................................................

