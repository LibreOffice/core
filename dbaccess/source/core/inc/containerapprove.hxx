/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: containerapprove.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:12:52 $
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

#ifndef DBACCESS_CONTAINERAPPROVE_HXX
#define DBACCESS_CONTAINERAPPROVE_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= IContainerApprove
    //====================================================================
    /** interface for approving elements to be inserted into a container

        On the long run, one could imagine that this interface completely encapsulates
        container/element approvals in all our various container classes herein (document
        containers, definition containers, table containers, query containers,
        command definition containers, bookmark containers). This would decrease coupling
        of the respective classes.
     */
    class SAL_NO_VTABLE IContainerApprove
    {
    public:
        /** approves a given element for insertion into the container
            @param  _rName
                specifies the name under which the element is going to be inserted
            @param  _rxElement
                specifies the element which is going to be inserted
            @throws Exception
                if the name or the object are invalid, or not eligible for insertion
                into the container
        */
        virtual void SAL_CALL   approveElement(
            const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement
        ) = 0;
    };

    typedef ::boost::shared_ptr< IContainerApprove >    PContainerApprove;

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // DBACCESS_CONTAINERAPPROVE_HXX

