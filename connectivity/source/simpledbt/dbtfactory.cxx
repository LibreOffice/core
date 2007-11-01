/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtfactory.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:52:52 $
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
#include "precompiled_connectivity.hxx"

#include <connectivity/virtualdbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>

#ifndef CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX
#include "dbtfactory.hxx"
#endif
#ifndef CONNECTIVITY_DBTOOLS_PARSER_SIMPLE_HXX
#include "parser_s.hxx"
#endif
#ifndef CONNECTIVITY_STATIC_DBTOOLS_SIMPLE_HXX
#include "staticdbtools_s.hxx"
#endif
#ifndef CONNECTIVITY_DBTOOLS_CHARSET_S_HXX
#include "charset_s.hxx"
#endif

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


