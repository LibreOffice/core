/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbptools.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:59:29 $
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

#ifndef _EXTENSIONS_DBP_DBPTOOLS_HXX_
#include "dbptools.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;

    //---------------------------------------------------------------------
    void disambiguateName(const Reference< XNameAccess >& _rxContainer, ::rtl::OUString& _rElementsName)
    {
        DBG_ASSERT(_rxContainer.is(), "::dbp::disambiguateName: invalid container!");
        if (!_rxContainer.is())
            return;

        try
        {
            ::rtl::OUString sBase(_rElementsName);
            for (sal_Int32 i=1; i<0x7FFFFFFF; ++i)
            {
                _rElementsName = sBase;
                _rElementsName += ::rtl::OUString::valueOf((sal_Int32)i);
                if (!_rxContainer->hasByName(_rElementsName))
                    return;
            }
            // can't do anything ... no free names
            _rElementsName = sBase;
        }
        catch(Exception&)
        {
            DBG_ERROR("::dbp::disambiguateName: something went (strangely) wrong!");
        }
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

