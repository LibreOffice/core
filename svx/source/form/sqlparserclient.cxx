/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlparserclient.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:59:17 $
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
#include "precompiled_svx.hxx"

#ifndef SVX_SQLPARSERCLIENT_HXX
#include "sqlparserclient.hxx"
#endif
#include "ParseContext.hxx"

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //====================================================================
    //= OSQLParserClient
    //====================================================================
    //--------------------------------------------------------------------
    OSQLParserClient::OSQLParserClient(const Reference< XMultiServiceFactory >& _rxORB)
    {
        m_xORB = _rxORB;
    }
    //--------------------------------------------------------------------
    bool OSQLParserClient::ensureLoaded() const
    {
        if ( !ODbtoolsClient::ensureLoaded() )
            return false;
         m_xParser = getFactory()->createSQLParser(m_xORB,getParseContext());
        return m_xParser.is();
    }

//........................................................................
}   // namespace svxform
//........................................................................


