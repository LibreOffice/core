/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docerrorhandling.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:02:47 $
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

#ifndef DBACCESS_DOCERRORHANDLING_HXX
#define DBACCESS_DOCERRORHANDLING_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
/** === end UNO includes === **/

namespace comphelper {
    class ComponentContext;
}

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= DocumentErrorHandling
    //====================================================================
    class DocumentErrorHandling
    {
    public:
        /** reports the given error (usually an exception caught on the caller's side)
            to the user, using the document's interaction handler, if present. If the document
            does not have an own interaction handler, the given component context is used
            to create a new one.
        */
        static void reportError(
                const ::comphelper::ComponentContext& _rContext,
                const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XOfficeDatabaseDocument >& _rxDocument,
                const ::com::sun::star::uno::Any& _rError
               );
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_DOCERRORHANDLING_HXX
