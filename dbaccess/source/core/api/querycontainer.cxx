/*************************************************************************
 *
 *  $RCSfile: querycontainer.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:03:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBA_CORE_QUERYCONTAINER_HXX_
#include "querycontainer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_COREAPI_QUERY_HXX_
#include "query.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OQueryContainer
//==========================================================================
DBG_NAME(OQueryContainer)
//------------------------------------------------------------------------------
OQueryContainer::OQueryContainer(
                  const Reference< XNameContainer >& _rxCommandDefinitions
                , const Reference< XConnection >& _rxConn
                , const Reference< XMultiServiceFactory >& _rxORB,
                IWarningsContainer* _pWarnings)
    :ODefinitionContainer(_rxORB,NULL,TContentPtr(new ODefinitionContainer_Impl))
    ,m_pWarnings( _pWarnings )
    ,m_xCommandDefinitions(_rxCommandDefinitions)
    ,m_xConnection(_rxConn)
{
    DBG_CTOR(OQueryContainer, NULL);

    increment(m_refCount);

    m_pCommandsListener = new OCommandsListener(this);
    m_pCommandsListener->acquire();

    {
        Reference< XContainer > xContainer(m_xCommandDefinitions, UNO_QUERY);
        DBG_ASSERT(xContainer.is(), "OQueryContainer::OQueryContainer : the CommandDefinitions container is invalid !");
        xContainer->addContainerListener(m_pCommandsListener);

//      // fill my structures with dummies
//      Sequence< ::rtl::OUString > sDefinitionNames = m_xCommandDefinitions->getElementNames();
//      const ::rtl::OUString* pIter = sDefinitionNames.getConstArray();
//      const ::rtl::OUString* pEnd = pIter + sDefinitionNames.getLength();
//      for (;pIter != pEnd; ++pIter)
//      {
//          implAppend(*pIter,Reference< XContent >());
//      }
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------------------
OQueryContainer::~OQueryContainer()
{
    DBG_DTOR(OQueryContainer, NULL);
    //  dispose();
        //  maybe we're already disposed, but this should be uncritical
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( OQueryContainer,ODefinitionContainer,OQueryContainer_Base)
//------------------------------------------------------------------------------
void OQueryContainer::disposing()
{
    ODefinitionContainer::disposing();
    MutexGuard aGuard(m_aMutex);

    // say our listeners goobye
    Reference< XContainer > xContainer(m_xCommandDefinitions, UNO_QUERY);
    if (xContainer.is())
        xContainer->removeContainerListener(m_pCommandsListener);
    if(m_pCommandsListener)
        m_pCommandsListener->release();
    m_pCommandsListener     = NULL;
    m_xCommandDefinitions   = NULL;
    m_xConnection           = NULL;
}

// XServiceInfo
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2(OQueryContainer, "com.sun.star.sdb.dbaccess.OQueryContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDB_QUERIES)

// XDataDescriptorFactory
//--------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OQueryContainer::createDataDescriptor(  ) throw(RuntimeException)
{
    return new OQueryDescriptor();
}

// XAppend
//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::appendByDescriptor( const Reference< XPropertySet >& _rxDesc ) throw(SQLException, ElementExistException, RuntimeException)
{
    Reference< XContent> xNewObject;
    ::rtl::OUString sNewObjectName;
    {
        MutexGuard aGuard(m_aMutex);

        OQueryDescriptor* pImpl = NULL;
                comphelper::getImplementation(pImpl, Reference< XInterface >(_rxDesc.get()));
        DBG_ASSERT(pImpl != NULL, "OQueryContainer::appendByDescriptor : can't fully handle this descriptor !");

        // first clone this object's CommandDefinition part
        if (!m_xCommandDefinitions.is())
        {
            DBG_ERROR("OQueryContainer::appendByDescriptor : have no CommandDefinition container anymore !");
                // perhaps somebody modified the DataSource, so all connections were separated
            throwGenericSQLException(::rtl::OUString::createFromAscii("Unable to insert objects into containers of standalone connections (not belonging to a data source)."), *this);
                // TODO : resource
        }

        Reference< XPropertySet > xCommandDefinitionPart(m_xORB->createInstance(SERVICE_SDB_QUERYDEFINITION), UNO_QUERY);
        if (!xCommandDefinitionPart.is())
        {
            DBG_ERROR("OQueryContainer::appendByDescriptor : could not create a CommandDefinition object !");
            throwGenericSQLException(::rtl::OUString::createFromAscii("Unable to create an object supporting the com.sun.star.sdb.CommandDefinition service"), *this);
                // TODO : resource
        }

        ::comphelper::copyProperties(_rxDesc, xCommandDefinitionPart);
        // and insert it into the CommDef container
        _rxDesc->getPropertyValue(PROPERTY_NAME) >>= sNewObjectName;
        {
            m_eDoingCurrently = INSERTING;
            OAutoActionReset aAutoReset(this);
            m_xCommandDefinitions->insertByName(sNewObjectName, makeAny(xCommandDefinitionPart));
        }

#if DBG_UTIL
        // check if the object was really inserted
        try
        {
            Reference< XPropertySet > xNewEl(m_xCommandDefinitions->getByName(sNewObjectName),UNO_QUERY);
            DBG_ASSERT(xNewEl.get() == xCommandDefinitionPart.get(), "OQueryContainer::appendByDescriptor : the CommandDefinition container worked as it had a descriptor !");
                // normally should not have changed after inserting
        }
        catch(Exception&)
        {
            DBG_ERROR("OQueryContainer::appendByDescriptor : could not find the just inserted CommandDefinition !");
        }
#endif
        // TODO : the columns part of the descriptor has to be copied
        xNewObject = implCreateWrapper(Reference< XContent>(xCommandDefinitionPart,UNO_QUERY));
    }

    ClearableMutexGuard aGuard(m_aMutex);
    implAppend(sNewObjectName, xNewObject);

    notifyByName(aGuard,sNewObjectName,xNewObject,NULL,E_INSERTED);
}

// XDrop
//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::dropByName( const ::rtl::OUString& _rName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if ( !checkExistence(_rName) )
        throw NoSuchElementException(_rName,*this);

    if ( !m_xCommandDefinitions.is() )
    {
        DBG_ERROR("OQueryContainer::dropByIndex : have no CommandDefinition container anymore !");
            // perhaps somebody modified the DataSource, so all connections were separated
        throwGenericSQLException(::rtl::OUString::createFromAscii("Unable to remove objects from containers of standalone connections (not belonging to a data source)."), *this);
            // TODO : resource
    }

    // now simply forward the remove request to the CommandDefinition container, we're a listener for the removal
    // and thus we do everything neccessary in ::elementRemoved
    m_xCommandDefinitions->removeByName(_rName);
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::dropByIndex( sal_Int32 _nIndex ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if ((_nIndex<0) || (_nIndex>getCount()))
        throw IndexOutOfBoundsException();

    ::rtl::OUString sName;
    Reference<XPropertySet> xProp(Reference<XIndexAccess>(m_xCommandDefinitions,UNO_QUERY)->getByIndex(_nIndex),UNO_QUERY);
    if ( xProp.is() )
        xProp->getPropertyValue(PROPERTY_NAME) >>= sName;

    dropByName(sName);
}
//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::elementInserted( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XContent > xNewElement;
    ::rtl::OUString sElementName;
    _rEvent.Accessor >>= sElementName;
    {
        MutexGuard aGuard(m_aMutex);
        if (INSERTING == m_eDoingCurrently)
            // nothing to do, we're inserting via an "appendByDescriptor"
            return;

        DBG_ASSERT(sElementName.getLength(), "OQueryContainer::elementInserted : invalid name !");
        DBG_ASSERT(!hasByName(sElementName), "OQueryContainer::elementInserted : oops .... we're inconsistent with our master container !");
        if (!sElementName.getLength() || hasByName(sElementName))
            return;

        // insert an own new element
        xNewElement = implCreateWrapper(sElementName);
    }
    insertByName(sElementName,makeAny(xNewElement));
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::elementRemoved( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sAccessor;
    _rEvent.Accessor >>= sAccessor;
    {
        DBG_ASSERT(sAccessor.getLength(), "OQueryContainer::elementRemoved : invalid name !");
        DBG_ASSERT(hasByName(sAccessor), "OQueryContainer::elementRemoved : oops .... we're inconsistent with our master container !");
        if ( !sAccessor.getLength() || !hasByName(sAccessor) )
            return;
    }
    removeByName(sAccessor);
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::elementReplaced( const ::com::sun::star::container::ContainerEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XPropertySet > xReplacedElement;
    Reference< XContent > xNewElement;
    ::rtl::OUString sAccessor;
    _rEvent.Accessor >>= sAccessor;

    {
        MutexGuard aGuard(m_aMutex);
        DBG_ASSERT(sAccessor.getLength(), "OQueryContainer::elementReplaced : invalid name !");
        DBG_ASSERT(hasByName(sAccessor), "OQueryContainer::elementReplaced : oops .... we're inconsistent with our master container !");
        if (!sAccessor.getLength() || !hasByName(sAccessor))
            return;

        xNewElement = implCreateWrapper(sAccessor);
    }

    replaceByName(sAccessor,makeAny(xNewElement));
}

//------------------------------------------------------------------------------
void SAL_CALL OQueryContainer::disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw(::com::sun::star::uno::RuntimeException)
{
    if (_rSource.Source.get() == Reference< XInterface >(m_xCommandDefinitions, UNO_QUERY).get())
    {   // our "master container" (with the command definitions) is beeing disposed
        DBG_ERROR("OQueryContainer::disposing : nobody should dispose the CommandDefinition container before disposing my connection !");
        dispose();
    }
    else
    {
        Reference< XContent > xSource(_rSource.Source, UNO_QUERY);
        // it's one of our documents ....
        Documents::iterator aIter = m_aDocumentMap.begin();
        Documents::iterator aEnd = m_aDocumentMap.end();
        for (;aIter != aEnd;++aIter )
        {
            if ( xSource == aIter->second.get() )
            {
                m_xCommandDefinitions->removeByName(aIter->first);
                break;
            }
        }
        ODefinitionContainer::disposing(_rSource);
    }
}
// -----------------------------------------------------------------------------
Reference< XContent > OQueryContainer::implCreateWrapper(const ::rtl::OUString& _rName)
{
    Reference< XContent > xObject(m_xCommandDefinitions->getByName(_rName),UNO_QUERY);
    return implCreateWrapper(xObject);
}

//--------------------------------------------------------------------------
Reference< XContent > OQueryContainer::implCreateWrapper(const Reference< XContent >& _rxCommandDesc)
{
    Reference<XNameContainer> xContainer(_rxCommandDesc,UNO_QUERY);
    Reference< XContent > xReturn;
    if ( xContainer .is() )
    {
        xReturn = new OQueryContainer(xContainer,m_xConnection,m_xORB,m_pWarnings);
    }
    else
    {
        OQuery* pNewObject = new OQuery(Reference<XPropertySet>(_rxCommandDesc,UNO_QUERY), m_xConnection,m_xORB);
        xReturn = pNewObject;

        pNewObject->setWarningsContainer( m_pWarnings );
        pNewObject->getColumns();
    }

    return xReturn;
}
//--------------------------------------------------------------------------
Reference< XContent > OQueryContainer::createObject( const ::rtl::OUString& _rName)
{
    return implCreateWrapper(_rName);
}
// -----------------------------------------------------------------------------
sal_Bool OQueryContainer::checkExistence(const ::rtl::OUString& _rName)
{
    sal_Bool bRet = sal_False;
    if ( !m_bInPropertyChange )
    {
        bRet = m_xCommandDefinitions->hasByName(_rName);
        Documents::iterator aFind = m_aDocumentMap.find(_rName);
        if ( !bRet && aFind != m_aDocumentMap.end() )
        {
            m_aDocuments.erase( ::std::find(m_aDocuments.begin(),m_aDocuments.end(),aFind));
            m_aDocumentMap.erase(aFind);
        }
        else if ( bRet && aFind == m_aDocumentMap.end() )
        {
            implAppend(_rName,NULL);
        }
    }
    return bRet;
}
//--------------------------------------------------------------------------
sal_Bool SAL_CALL OQueryContainer::hasElements( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_xCommandDefinitions->hasElements();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OQueryContainer::getCount(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return Reference<XIndexAccess>(m_xCommandDefinitions,UNO_QUERY)->getCount();
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OQueryContainer::getElementNames(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    return m_xCommandDefinitions->getElementNames();
}
//........................................................................
}   // namespace dbaccess
//........................................................................

