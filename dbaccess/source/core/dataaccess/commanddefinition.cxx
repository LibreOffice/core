/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commanddefinition.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:37:10 $
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

#ifndef _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_
#include "commanddefinition.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OCommandDefinition
//==========================================================================
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition()
{
    static OMultiInstanceAutoRegistration< OCommandDefinition > aAutoRegistration;
}

//--------------------------------------------------------------------------
DBG_NAME(OCommandDefinition)
//--------------------------------------------------------------------------
void OCommandDefinition::registerProperties()
{
    OCommandDefinition_Impl& rCommandDefinition( getCommandDefinition() );
    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sCommand, ::getCppuType(&rCommandDefinition.m_sCommand));

    registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_bEscapeProcessing, ::getBooleanCppuType());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateTableName, ::getCppuType(&rCommandDefinition.m_sUpdateTableName));

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateSchemaName, ::getCppuType(&rCommandDefinition.m_sUpdateSchemaName));

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateCatalogName, ::getCppuType(&rCommandDefinition.m_sUpdateCatalogName));
    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_aLayoutInformation, ::getCppuType(&rCommandDefinition.m_aLayoutInformation));
}

//--------------------------------------------------------------------------
OCommandDefinition::OCommandDefinition(const Reference< XMultiServiceFactory >& _xORB
                                       ,const Reference< XInterface >& _rxContainer
                                       ,const TContentPtr& _pImpl)
    :OComponentDefinition(_xORB,_rxContainer,_pImpl,sal_False)
{
    DBG_CTOR(OCommandDefinition, NULL);
    registerProperties();
}
//--------------------------------------------------------------------------
OCommandDefinition::~OCommandDefinition()
{
    DBG_DTOR(OCommandDefinition, NULL);
}

//--------------------------------------------------------------------------
OCommandDefinition::OCommandDefinition( const Reference< XInterface >& _rxContainer
                                       ,const ::rtl::OUString& _rElementName
                                       ,const Reference< XMultiServiceFactory >& _xORB
                                       ,const TContentPtr& _pImpl)
    :OComponentDefinition(_rxContainer,_rElementName,_xORB,_pImpl,sal_False)
{
    DBG_CTOR(OCommandDefinition, NULL);
    registerProperties();
}

//--------------------------------------------------------------------------
IMPLEMENT_IMPLEMENTATION_ID(OCommandDefinition);
IMPLEMENT_GETTYPES2(OCommandDefinition,OCommandDefinition_Base,OComponentDefinition);
IMPLEMENT_FORWARD_XINTERFACE2( OCommandDefinition,OComponentDefinition,OCommandDefinition_Base)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2(OCommandDefinition,OCommandDefinition_PROP)
//--------------------------------------------------------------------------
::rtl::OUString OCommandDefinition::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.dba.OCommandDefinition");
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCommandDefinition::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > OCommandDefinition::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aServices(3);
    aServices.getArray()[0] = SERVICE_SDB_QUERYDEFINITION;
    aServices.getArray()[1] = SERVICE_SDB_COMMAND_DEFINITION;
    aServices.getArray()[2] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.Content"));
    return aServices;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OCommandDefinition::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//------------------------------------------------------------------------------
Reference< XInterface > OCommandDefinition::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OCommandDefinition(_rxFactory,NULL,TContentPtr(new OCommandDefinition_Impl)));
}

// -----------------------------------------------------------------------------
void SAL_CALL OCommandDefinition::rename( const ::rtl::OUString& newName ) throw (SQLException, ElementExistException, RuntimeException)
{
    try
    {
        sal_Int32 nHandle = PROPERTY_ID_NAME;
        osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
        Any aOld = makeAny(m_pImpl->m_aProps.aTitle);
        aGuard.clear();
        Any aNew = makeAny(newName);
        fire(&nHandle, &aNew, &aOld, 1, sal_True );

        m_pImpl->m_aProps.aTitle = newName;
        fire(&nHandle, &aNew, &aOld, 1, sal_False );
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(newName,*this);
    }
}
// -----------------------------------------------------------------------------
//........................................................................
}   // namespace dbaccess
//........................................................................

