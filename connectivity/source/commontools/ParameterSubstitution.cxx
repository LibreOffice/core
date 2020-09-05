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
#include <ParameterSubstitution.hxx>
#include <connectivity/sqlparse.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace connectivity
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    ParameterSubstitution::ParameterSubstitution(const css::uno::Reference< css::uno::XComponentContext >& _rxContext ) : m_xContext(_rxContext)
    {
    }
    void SAL_CALL ParameterSubstitution::initialize( const uno::Sequence< uno::Any >& _aArguments )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        comphelper::SequenceAsHashMap aArgs(_aArguments);
        uno::Reference< sdbc::XConnection > xConnection;
        xConnection = aArgs.getUnpackedValueOrDefault("ActiveConnection",xConnection);
        m_xConnection = xConnection;
    }

    OUString SAL_CALL ParameterSubstitution::getImplementationName(  )
    {
        return "org.openoffice.comp.helper.ParameterSubstitution";
    }

    sal_Bool SAL_CALL ParameterSubstitution::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL ParameterSubstitution::getSupportedServiceNames(  )
    {
        return { "com.sun.star.sdb.ParameterSubstitution" };
    }


    OUString SAL_CALL ParameterSubstitution::substituteVariables( const OUString& _sText, sal_Bool /*bSubstRequired*/ )
    {
        OUString sRet = _sText;
        uno::Reference< sdbc::XConnection > xConnection = m_xConnection;
        if ( xConnection.is() )
        {
            try
            {
                OSQLParser aParser( m_xContext );
                OUString sErrorMessage;
                std::unique_ptr<OSQLParseNode> pNode = aParser.parseTree(sErrorMessage,_sText);
                if(pNode)
                {   // special handling for parameters
                    OSQLParseNode::substituteParameterNames(pNode.get());
                    OUString sNewSql;
                    pNode->parseNodeToStr( sNewSql, xConnection );
                    sRet = sNewSql;
                }
            }
            catch(const Exception&)
            {
            }
        }
        return sRet;
    }

    OUString SAL_CALL ParameterSubstitution::reSubstituteVariables( const OUString& _sText )
    {
        return _sText;
    }

    OUString SAL_CALL ParameterSubstitution::getSubstituteVariableValue( const OUString& /*variable*/ )
    {
        throw container::NoSuchElementException();
    }


} // connectivity

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_dbtools_ParameterSubstitution_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new connectivity::ParameterSubstitution(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
