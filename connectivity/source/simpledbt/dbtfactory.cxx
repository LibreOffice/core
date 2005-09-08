/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtfactory.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:46:24 $
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

#ifndef CONNECTIVITY_DBTOOLS_DBTFACTORY_HXX
#include "dbtfactory.hxx"
#endif
#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
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
    ::rtl::Reference< simple::ISQLParser > ODataAccessToolsFactory::createSQLParser(const Reference< XMultiServiceFactory >& _rxServiceFactory,const IParseContext* _pContext) const
    {
        return new OSimpleSQLParser(_rxServiceFactory,_pContext);
    }

//........................................................................
}   // namespace connectivity
//........................................................................


