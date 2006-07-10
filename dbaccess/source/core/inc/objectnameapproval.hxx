/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: objectnameapproval.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:14:13 $
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

#ifndef DBACCESS_OBJECTNAMEAPPROVAL_HXX
#define DBACCESS_OBJECTNAMEAPPROVAL_HXX

#ifndef DBACCESS_CONTAINERAPPROVE_HXX
#include "containerapprove.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= ObjectNameApproval
    //====================================================================
    struct ObjectNameApproval_Impl;
    /** implementation of the IContainerApprove interface which approves
        elements for insertion into a query or tables container.

        The only check done by this instance is whether the query name is
        not already used, taking into account that in some databases, queries
        and tables share the same namespace.

        The class is not thread-safe.
    */
    class ObjectNameApproval : public IContainerApprove
    {
        ::std::auto_ptr< ObjectNameApproval_Impl >   m_pImpl;

    public:
        enum ObjectType
        {
            TypeQuery,
            TypeTable
        };

    public:
        /** constructs the instance

            @param _rxConnection
                the connection relative to which the names should be checked. This connection
                will be held weak. In case it is closed, subsequent calls to this instance's
                methods throw a DisposedException.
            @param _eType
                specifies which type of objects is to be approved with this instance
        */
        ObjectNameApproval(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            ObjectType _eType
        );
        virtual ~ObjectNameApproval();

        // IContainerApprove
        virtual void SAL_CALL approveElement( const ::rtl::OUString& _rName, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_OBJECTNAMEAPPROVAL_HXX
