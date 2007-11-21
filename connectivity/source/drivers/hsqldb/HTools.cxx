/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HTools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:01:52 $
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
#include "precompiled_connectivity.hxx"

#include "hsqldb/HTools.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    //====================================================================
    //= HTools
    //====================================================================
    //--------------------------------------------------------------------
    void HTools::appendTableFilterCrit( ::rtl::OUStringBuffer& _inout_rBuffer, const ::rtl::OUString& _rCatalog,
        const ::rtl::OUString _rSchema, const ::rtl::OUString _rName, bool _bShortForm )
    {
        _inout_rBuffer.appendAscii( " WHERE " );
        if ( _rCatalog.getLength() )
        {
            _inout_rBuffer.appendAscii( _bShortForm ? "TABLE_CAT" : "TABLE_CATALOG" );
            _inout_rBuffer.appendAscii( " = '" );
            _inout_rBuffer.append     ( _rCatalog );
            _inout_rBuffer.appendAscii( "' AND " );
        }
        if ( _rSchema.getLength() )
        {
            _inout_rBuffer.appendAscii( _bShortForm ? "TABLE_SCHEM" : "TABLE_SCHEMA" );
            _inout_rBuffer.appendAscii( " = '" );
            _inout_rBuffer.append     ( _rSchema );
            _inout_rBuffer.appendAscii( "' AND " );
        }
        _inout_rBuffer.appendAscii( "TABLE_NAME = '" );
        _inout_rBuffer.append     ( _rName );
        _inout_rBuffer.appendAscii( "'" );
    }

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................
