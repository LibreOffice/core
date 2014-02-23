/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <connectivity/virtualdbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include "dbtfactory.hxx"
#include "parser_s.hxx"
#include "staticdbtools_s.hxx"
#include "charset_s.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


// the entry point for load-on-call usage of the DBTOOLS lib
extern "C" void* SAL_CALL createDataAccessToolsFactory()
{
    ::connectivity::ODataAccessToolsFactory* pFactory = new ::connectivity::ODataAccessToolsFactory;
    pFactory->acquire();
    return pFactory;
}


namespace connectivity
{



    //= ODataAccessToolsFactory


    ODataAccessToolsFactory::ODataAccessToolsFactory()
    {
        ODataAccessStaticTools* pStaticTools = new ODataAccessStaticTools;
        m_xTypeConversionHelper = pStaticTools;
        m_xToolsHelper = pStaticTools;
    }


    oslInterlockedCount SAL_CALL ODataAccessToolsFactory::acquire()
    {
        return ORefBase::acquire();
    }


    oslInterlockedCount SAL_CALL ODataAccessToolsFactory::release()
    {
        return ORefBase::release();
    }


    ::rtl::Reference< simple::IDataAccessTypeConversion > ODataAccessToolsFactory::getTypeConversionHelper()
    {
        return m_xTypeConversionHelper;
    }


    ::rtl::Reference< simple::IDataAccessCharSet > ODataAccessToolsFactory::createCharsetHelper( ) const
    {
        return new ODataAccessCharSet;
    }


    ::rtl::Reference< simple::IDataAccessTools > ODataAccessToolsFactory::getDataAccessTools()
    {
        return m_xToolsHelper;
    }


    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ::dbtools::FormattedColumnValue > ODataAccessToolsFactory::createFormattedColumnValue(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
        const Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet, const Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn )
    {
        ::std::auto_ptr< ::dbtools::FormattedColumnValue > pValue( new ::dbtools::FormattedColumnValue( _rxContext, _rxRowSet, _rxColumn ) );
        return pValue;
    }
    SAL_WNODEPRECATED_DECLARATIONS_POP


    ::rtl::Reference< simple::ISQLParser > ODataAccessToolsFactory::createSQLParser(const Reference< XComponentContext >& rxContext, const IParseContext* _pContext) const
    {
        return new OSimpleSQLParser(rxContext, _pContext);
    }


}   // namespace connectivity



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
