/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: View.cxx,v $
 * $Revision: 1.3 $
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

#include "View.hxx"
#include "dbastrings.hrc"

#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
/** === end UNO includes === **/

#include <cppuhelper/exc_hlp.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using namespace ::com::sun::star::uno;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::sdbc::SQLException;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::sdbc::XStatement;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::sdbc::XRow;
    /** === end UNO using === **/

    //====================================================================
    //= View
    //====================================================================
    //--------------------------------------------------------------------
    View::View( const Reference< XConnection >& _rxConnection, sal_Bool _bCaseSensitive,
        const ::rtl::OUString& _rCatalogName,const ::rtl::OUString& _rSchemaName, const ::rtl::OUString& _rName )
        :View_Base( _bCaseSensitive, _rName, _rxConnection->getMetaData(), 0, ::rtl::OUString(), _rSchemaName, _rCatalogName )
    {
        m_nCommandHandle = getProperty(PROPERTY_COMMAND).Handle;
        try
        {
            Reference<XMultiServiceFactory> xFac(_rxConnection,UNO_QUERY_THROW);
            static const ::rtl::OUString s_sViewSupport(RTL_CONSTASCII_USTRINGPARAM("ViewSupportServiceName"));
            m_xViewSupport.set(xFac->createInstance(s_sViewSupport),UNO_QUERY);
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    View::~View()
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_REFCOUNT( View, View_Base )
    IMPLEMENT_GET_IMPLEMENTATION_ID( View )
    // -------------------------------------------------------------------------
    Any SAL_CALL View::queryInterface( const Type & _rType ) throw(RuntimeException)
    {
        if(_rType == getCppuType( (Reference<XAlterView>*)0) && !m_xViewSupport.is() )
            return Any();
        Any aReturn = View_Base::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = View_IBASE::queryInterface( _rType );
        return aReturn;
    }
    // -------------------------------------------------------------------------
    Sequence< Type > SAL_CALL View::getTypes(  ) throw(RuntimeException)
    {
        Type aAlterType = getCppuType( (Reference<XAlterView>*)0);

        Sequence< Type > aTypes( ::comphelper::concatSequences(View_Base::getTypes(),View_IBASE::getTypes()) );
        ::std::vector<Type> aOwnTypes;
        aOwnTypes.reserve(aTypes.getLength());

        const Type* pIter = aTypes.getConstArray();
        const Type* pEnd = pIter + aTypes.getLength();
        for(;pIter != pEnd ;++pIter)
        {
            if( (*pIter != aAlterType || m_xViewSupport.is()) )
                aOwnTypes.push_back(*pIter);
        }

        Type* pTypes = aOwnTypes.empty() ? 0 : &aOwnTypes[0];
        return Sequence< Type >(pTypes, aOwnTypes.size());
    }

    //--------------------------------------------------------------------
    void SAL_CALL View::alterCommand( const ::rtl::OUString& _rNewCommand ) throw (SQLException, RuntimeException)
    {
        OSL_ENSURE(m_xViewSupport.is(),"Illegal call to AlterView!");
        m_xViewSupport->alterCommand(this,_rNewCommand);
    }

    //--------------------------------------------------------------------
    void SAL_CALL View::getFastPropertyValue( Any& _rValue, sal_Int32 _nHandle ) const
    {
        if ( _nHandle == m_nCommandHandle && m_xViewSupport.is() )
        {
            // retrieve the very current command, don't rely on the base classes cached value
            // (which we initialized empty, anyway)
            _rValue <<= m_xViewSupport->getCommand(const_cast<View*>(this));
            return;
        }

        View_Base::getFastPropertyValue( _rValue, _nHandle );
    }
    // -----------------------------------------------------------------------------
//........................................................................
} // namespace dbaccess
//........................................................................
