/*************************************************************************
 *
 *  $RCSfile: sdbcoretools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:08 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif

//.........................................................................
namespace dbaccess
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //=====================================================================
    Reference< XNumberFormatsSupplier > getDataSourceNumberFormats( Reference< XConnection >& _rxDataSourceConnection )
    {
        DBG_ASSERT( _rxDataSourceConnection.is(), "::dbaccess::getDataSourceNumberFormats: invalid connection!" );

        Reference< XNumberFormatsSupplier > xReturn;
        if ( _rxDataSourceConnection.is() )
        {
            try
            {
                Reference< XChild > xConnAsChild( _rxDataSourceConnection, UNO_QUERY );

                Reference< XPropertySet > xDSProps;
                if ( xConnAsChild.is() )
                    xDSProps = xDSProps.query( xConnAsChild->getParent() );

                DBG_ASSERT( xDSProps.is(), "::dbaccess::getDataSourceNumberFormats: invalid connection (no XChild or an invalid parent)!" );
                if ( xDSProps.is() )
                {
                    xDSProps->getPropertyValue( PROPERTY_NUMBERFORMATSSUPPLIER ) >>= xReturn;
                    DBG_ASSERT( xReturn.is(), "::dbaccess::getDataSourceNumberFormats: could not retrieve the number formatter!" );
                }
            }
            catch ( const Exception& e )
            {
                e; // make compiler happy
            }
        }

        return xReturn;
    }

//.........................................................................
}   // namespace dbaccess
//.........................................................................

