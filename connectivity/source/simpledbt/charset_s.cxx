/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: charset_s.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 03:13:16 $
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

#ifndef CONNECTIVITY_DBTOOLS_CHARSET_S_HXX
#include "charset_s.hxx"
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    using namespace ::dbtools;

    //====================================================================
    //= ODataAccessCharSet
    //====================================================================
    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessCharSet::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessCharSet::release()
    {
        return ORefBase::release();
    }

    //--------------------------------------------------------------------
    sal_Int32 ODataAccessCharSet::getSupportedTextEncodings( ::std::vector< rtl_TextEncoding >& _rEncs ) const
    {
        _rEncs.clear();

        OCharsetMap::const_iterator aLoop = m_aCharsetInfo.begin();
        OCharsetMap::const_iterator aLoopEnd = m_aCharsetInfo.end();
        while (aLoop != aLoopEnd)
        {
            _rEncs.push_back( (*aLoop).getEncoding() );
            ++aLoop;
        }

        return _rEncs.size();
    }

//........................................................................
}   // namespace connectivity
//........................................................................

