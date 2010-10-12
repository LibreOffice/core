/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "sdbcoretools.hxx"
#include "dbastrings.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/task/XInteractionRequestStringResolver.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/interaction.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

//.........................................................................
namespace dbaccess
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::task;
    using namespace ::com::sun::star::embed;
    using namespace ::com::sun::star::container;

    // =========================================================================
    // -------------------------------------------------------------------------
    void notifyDataSourceModified(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject,sal_Bool _bModified)
    {
        Reference< XInterface > xDs = getDataSource( _rxObject );
        Reference<XDocumentDataSource> xDocumentDataSource(xDs,UNO_QUERY);
        if ( xDocumentDataSource.is() )
            xDs = xDocumentDataSource->getDatabaseDocument();
        Reference< XModifiable > xModi( xDs, UNO_QUERY );
        if ( xModi.is() )
            xModi->setModified(_bModified);
    }

    // -------------------------------------------------------------------------
    Reference< XInterface > getDataSource( const Reference< XInterface >& _rxDependentObject )
    {
        Reference< XInterface > xParent = _rxDependentObject;
        Reference< XInterface > xReturn;
        while( xParent.is() )
        {
            xReturn = xParent;
            Reference<XChild> xChild(xParent,UNO_QUERY);
            xParent.set(xChild.is() ? xChild->getParent() : Reference< XInterface >(),UNO_QUERY);
        }
        return xReturn;
    }

// -----------------------------------------------------------------------------
    ::rtl::OUString extractExceptionMessage( const ::comphelper::ComponentContext& _rContext, const Any& _rError )
    {
        ::rtl::OUString sDisplayMessage;

        try
        {
            Reference< XInteractionRequestStringResolver > xStringResolver;
            if ( _rContext.createComponent( "com.sun.star.task.InteractionRequestStringResolver", xStringResolver ) )
            {
                ::rtl::Reference< ::comphelper::OInteractionRequest > pRequest( new ::comphelper::OInteractionRequest( _rError ) );
                ::rtl::Reference< ::comphelper::OInteractionApprove > pApprove( new ::comphelper::OInteractionApprove );
                pRequest->addContinuation( pApprove.get() );
                Optional< ::rtl::OUString > aMessage = xStringResolver->getStringFromInformationalRequest( pRequest.get() );
                if ( aMessage.IsPresent )
                    sDisplayMessage = aMessage.Value;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !sDisplayMessage.getLength() )
        {
            Exception aExcept;
            _rError >>= aExcept;

            ::rtl::OUStringBuffer aBuffer;
            aBuffer.append( _rError.getValueTypeName() );
            aBuffer.appendAscii( ":\n" );
            aBuffer.append( aExcept.Message );

            sDisplayMessage = aBuffer.makeStringAndClear();
        }

        return sDisplayMessage;
    }

    namespace tools { namespace stor {

    // -----------------------------------------------------------------------------
    bool storageIsWritable_nothrow( const Reference< XStorage >& _rxStorage )
    {
        if ( !_rxStorage.is() )
            return false;

        sal_Int32 nMode = ElementModes::READ;
        try
        {
            Reference< XPropertySet > xStorageProps( _rxStorage, UNO_QUERY_THROW );
            xStorageProps->getPropertyValue(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OpenMode" ) ) ) >>= nMode;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return ( nMode & ElementModes::WRITE ) != 0;
    }

    // -----------------------------------------------------------------------------
    bool commitStorageIfWriteable( const Reference< XStorage >& _rxStorage ) SAL_THROW(( IOException, WrappedTargetException, RuntimeException ))
    {
        bool bSuccess = false;
        Reference< XTransactedObject > xTrans( _rxStorage, UNO_QUERY );
        if ( xTrans.is() )
        {
            if ( storageIsWritable_nothrow( _rxStorage ) )
                xTrans->commit();
            bSuccess = true;
        }
        return bSuccess;
    }

    } } // tools::stor

//.........................................................................
}   // namespace dbaccess
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
