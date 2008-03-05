/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pcrcommon.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:13:05 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef EXTENSIONS_PROPRESID_HRC
#include "propresid.hrc"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_MEASUREUNIT_HPP_
#include <com/sun/star/util/MeasureUnit.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

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

