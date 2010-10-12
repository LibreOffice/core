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
#include "precompiled_connectivity.hxx"

#include <connectivity/virtualdbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include "dbtfactory.hxx"
#include "parser_s.hxx"
#include "staticdbtools_s.hxx"
#include "charset_s.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//================================================================
// the entry point for load-on-call usage of the DBTOOLS lib
extern "C" void* SAL_CALL createDataAccessToolsFactory()
{
    ::connectivity::ODataAccessToolsFactory* pFactory = new ::connectivity::ODataAccessToolsFactory;
    pFactory->acquire();
    return pFactory;
}

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= ODataAccessToolsFactory
    //================================================================
    //----------------------------------------------------------------
    ODataAccessToolsFactory::ODataAccessToolsFactory()
    {
        ODataAccessStaticTools* pStaticTools = new ODataAccessStaticTools;
        m_xTypeConversionHelper = pStaticTools;
        m_xToolsHelper = pStaticTools;
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessToolsFactory::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessToolsFactory::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::IDataAccessTypeConversion > ODataAccessToolsFactory::getTypeConversionHelper()
    {
        return m_xTypeConversionHelper;
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::IDataAccessCharSet > ODataAccessToolsFactory::createCharsetHelper( ) const
    {
        return new ODataAccessCharSet;
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::IDataAccessTools > ODataAccessToolsFactory::getDataAccessTools()
    {
        return m_xToolsHelper;
    }

    //----------------------------------------------------------------
    ::std::auto_ptr< ::dbtools::FormattedColumnValue > ODataAccessToolsFactory::createFormattedColumnValue( const ::comphelper::ComponentContext& _rContext,
        const Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet, const Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn )
    {
        ::std::auto_ptr< ::dbtools::FormattedColumnValue > pValue( new ::dbtools::FormattedColumnValue( _rContext, _rxRowSet, _rxColumn ) );
        return pValue;
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::ISQLParser > ODataAccessToolsFactory::createSQLParser(const Reference< XMultiServiceFactory >& _rxServiceFactory,const IParseContext* _pContext) const
    {
        return new OSimpleSQLParser(_rxServiceFactory,_pContext);
    }

//........................................................................
}   // namespace connectivity
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
