/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docerrorhandling.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:02:34 $
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
#include "precompiled_dbaccess.hxx"

#include "docerrorhandling.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/interaction.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace dbmm
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::frame::XModel;
    /** === end UNO using === **/

    //====================================================================
    //= DocumentErrorHandling
    //====================================================================
    //--------------------------------------------------------------------
    void DocumentErrorHandling::reportError( const ::comphelper::ComponentContext& _rContext, const Reference< XOfficeDatabaseDocument >& _rxDocument,  const Any& _rError )
    {
        try
        {
            Reference< XInteractionHandler > xHandler( _rContext.createComponent( "com.sun.star.task.InteractionHandler" ), UNO_QUERY_THROW );
            // check whether the DB doc has an own interaction handler set
            Reference< XModel > xDocModel( _rxDocument, UNO_QUERY_THROW );
            ::comphelper::NamedValueCollection aDocArgs( xDocModel->getArgs() );
            xHandler = aDocArgs.getOrDefault( "InteractionHandler", xHandler );
            xHandler->handle(
                new ::comphelper::OInteractionRequest( _rError )
            );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
} // namespace dbmm
//........................................................................
