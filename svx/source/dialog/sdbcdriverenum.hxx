/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdbcdriverenum.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:00:40 $
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

#ifndef _OFFMGR_SDBCDRIVERENUMERATION_HXX_
#define _OFFMGR_SDBCDRIVERENUMERATION_HXX_


#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>

namespace rtl
{
    class OUString;
}

//........................................................................
namespace offapp
{
//........................................................................

    //====================================================================
    //= ODriverEnumeration
    //====================================================================
    class ODriverEnumerationImpl;
    /** simple class for accessing SDBC drivers registered within the office
        <p>Rather small, introduced to not contaminate other instances with the
        exception handling (code-size-bloating) implementations here.
        </p>
    */
    class ODriverEnumeration
    {
    private:
        ODriverEnumerationImpl* m_pImpl;

    public:
        ODriverEnumeration() throw();
        ~ODriverEnumeration() throw();
        typedef ::std::vector< ::rtl::OUString >::const_iterator const_iterator;

        const_iterator  begin() const throw();
        const_iterator  end() const throw();
        sal_Int32 size() const throw();
    };

//........................................................................
}   // namespace offapp
//........................................................................

#endif // _OFFMGR_SDBCDRIVERENUMERATION_HXX_


