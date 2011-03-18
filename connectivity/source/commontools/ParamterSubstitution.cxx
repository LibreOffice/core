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
#include "precompiled_connectivity.hxx"
#include "ParameterSubstitution.hxx"
#include "connectivity/sqlparse.hxx"
#include <comphelper/sequenceashashmap.hxx>

namespace connectivity
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    ParameterSubstitution::ParameterSubstitution(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext ) : m_xContext(_rxContext)
    {
    }
    void SAL_CALL ParameterSubstitution::initialize( const uno::Sequence< uno::Any >& _aArguments ) throw (uno::Exception, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        comphelper::SequenceAsHashMap aArgs(_aArguments);
        uno::Reference< sdbc::XConnection > xConnection;
        xConnection = aArgs.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ActiveConnection")),xConnection);
        m_xConnection = xConnection;
    }
    //------------------------------------------------------------------------------
    rtl::OUString ParameterSubstitution::getImplementationName_Static(  ) throw(RuntimeException)
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.helper.ParameterSubstitution"));
    }
    //------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ParameterSubstitution::getImplementationName(  ) throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    //------------------------------------------------------------------
    sal_Bool SAL_CALL ParameterSubstitution::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
        const ::rtl::OUString* pSupported = aSupported.getConstArray();
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
        for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
            ;

        return pSupported != pEnd;
    }
    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ParameterSubstitution::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > ParameterSubstitution::getSupportedServiceNames_Static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSNS( 1 );
        aSNS[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.ParameterSubstitution"));
        return aSNS;
    }

    //------------------------------------------------------------------
    Reference< XInterface >  ParameterSubstitution::create(const Reference< XComponentContext >& _xContext)
    {
        return *(new ParameterSubstitution(_xContext));
    }
    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ParameterSubstitution::substituteVariables( const ::rtl::OUString& _sText, ::sal_Bool /*bSubstRequired*/ ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
    {
        ::rtl::OUString sRet = _sText;
        uno::Reference< sdbc::XConnection > xConnection = m_xConnection;
        if ( xConnection.is() )
        {
            try
            {
                uno::Reference< XMultiServiceFactory> xFac(m_xContext->getServiceManager(),uno::UNO_QUERY_THROW);
                OSQLParser aParser( xFac );
                ::rtl::OUString sErrorMessage;
                ::rtl::OUString sNewSql;
                OSQLParseNode* pNode = aParser.parseTree(sErrorMessage,_sText);
                if(pNode)
                {   // special handling for parameters
                    OSQLParseNode::substituteParameterNames(pNode);
                    pNode->parseNodeToStr( sNewSql, xConnection );
                    delete pNode;
                    sRet = sNewSql;
                }
            }
            catch(const Exception&)
            {
            }
        }
        return sRet;
    }
    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ParameterSubstitution::reSubstituteVariables( const ::rtl::OUString& _sText ) throw (::com::sun::star::uno::RuntimeException)
    {
        return _sText;
    }
    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ParameterSubstitution::getSubstituteVariableValue( const ::rtl::OUString& /*variable*/ ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException)
    {
        throw container::NoSuchElementException();
    }
    //------------------------------------------------------------------


// ==================================
} // connectivity
// ==================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
