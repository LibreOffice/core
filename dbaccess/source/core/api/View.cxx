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

#include "View.hxx"
#include "stringconstants.hxx"
#include "strings.hxx"

#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>

namespace dbaccess
{

    using namespace ::com::sun::star::uno;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::XMultiServiceFactory;

    OUString lcl_getServiceNameForSetting(const Reference< css::sdbc::XConnection >& _xConnection,const OUString& i_sSetting)
    {
        OUString sSupportService;
        Any aValue;
        if ( dbtools::getDataSourceSetting(_xConnection,i_sSetting,aValue) )
        {
            aValue >>= sSupportService;
        }
        return sSupportService;
    }
    // View
    View::View( const Reference< XConnection >& _rxConnection, bool _bCaseSensitive,
        const OUString& _rCatalogName,const OUString& _rSchemaName, const OUString& _rName )
        :View_Base( _bCaseSensitive, _rName, _rxConnection->getMetaData(), OUString(), _rSchemaName, _rCatalogName )
    {
        m_nCommandHandle = getProperty(PROPERTY_COMMAND).Handle;
        try
        {
            Reference<XMultiServiceFactory> xFac(_rxConnection,UNO_QUERY_THROW);
            m_xViewAccess.set(xFac->createInstance(lcl_getServiceNameForSetting(_rxConnection,"ViewAccessServiceName")),UNO_QUERY);
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    View::~View()
    {
    }

    IMPLEMENT_FORWARD_REFCOUNT( View, View_Base )
    IMPLEMENT_GET_IMPLEMENTATION_ID( View )

    Any SAL_CALL View::queryInterface( const Type & _rType )
    {
        if(_rType == cppu::UnoType<XAlterView>::get()&& !m_xViewAccess.is() )
            return Any();
        Any aReturn = View_Base::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = View_IBASE::queryInterface( _rType );
        return aReturn;
    }

    Sequence< Type > SAL_CALL View::getTypes(  )
    {
        Type aAlterType = cppu::UnoType<XAlterView>::get();

        Sequence< Type > aTypes( ::comphelper::concatSequences(View_Base::getTypes(),View_IBASE::getTypes()) );
        std::vector<Type> aOwnTypes;
        aOwnTypes.reserve(aTypes.getLength());

        const Type* pIter = aTypes.getConstArray();
        const Type* pEnd = pIter + aTypes.getLength();
        for(;pIter != pEnd ;++pIter)
        {
            if( (*pIter != aAlterType || m_xViewAccess.is()) )
                aOwnTypes.push_back(*pIter);
        }

        return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
    }

    void SAL_CALL View::alterCommand( const OUString& _rNewCommand )
    {
        OSL_ENSURE(m_xViewAccess.is(),"Illegal call to AlterView!");
        m_xViewAccess->alterCommand(this,_rNewCommand);
    }

    void SAL_CALL View::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        if ( _nHandle == m_nCommandHandle && m_xViewAccess.is() )
        {
            // retrieve the very current command, don't rely on the base classes cached value
            // (which we initialized empty, anyway)
            _rValue <<= m_xViewAccess->getCommand(const_cast<View*>(this));
            return;
        }

        View_Base::getFastPropertyValue( _rValue, _nHandle );
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
