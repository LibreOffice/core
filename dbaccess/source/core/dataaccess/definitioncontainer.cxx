/*************************************************************************
 *
 *  $RCSfile: definitioncontainer.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:09:49 $
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

#ifndef _DBA_CORE_DEFINITIONCONTAINER_HXX_
#include "definitioncontainer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFO_HPP_
#include <com/sun/star/ucb/CommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::com::sun::star::ucb;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= ODefinitionContainer
//==========================================================================
DBG_NAME(ODefinitionContainer)
//--------------------------------------------------------------------------
ODefinitionContainer::ODefinitionContainer(   const Reference< XMultiServiceFactory >& _xORB
                                            , const Reference< XInterface >&    _xParentContainer
                                            , const TContentPtr& _pImpl
                                            )
    :OContentHelper(_xORB,_xParentContainer,_pImpl)
    ,m_aContainerListeners(m_aMutex)
    ,m_bInPropertyChange(sal_False)
{
    m_pImpl->m_aProps.bIsDocument = sal_False;
    m_pImpl->m_aProps.bIsFolder = sal_True;

    ODefinitionContainer_Impl* pItem = static_cast<ODefinitionContainer_Impl*>(m_pImpl.get());
    ODefinitionContainer_Impl::Documents::iterator aEnd = pItem->m_aDocumentMap.end();
    for (   ODefinitionContainer_Impl::Documents::iterator aNameIter = pItem->m_aDocumentMap.begin();
            aNameIter != aEnd;
            ++aNameIter
        )
        m_aDocuments.push_back(m_aDocumentMap.insert(Documents::value_type(aNameIter->first,Documents::mapped_type())).first);

    DBG_CTOR(ODefinitionContainer, NULL);
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::disposing()
{
    OContentHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    // say our listeners goobye
    EventObject aEvt(*this);
    m_aContainerListeners.disposeAndClear(aEvt);

    // dispose our elements
    Documents::iterator aIter = m_aDocumentMap.begin();
    Documents::iterator aEnd = m_aDocumentMap.end();

    for (; aIter != aEnd; ++aIter)
    {
        Reference<XContent> xProp = aIter->second;
        if ( xProp.is() )
        {
            removeObjectListener(xProp);
            ::comphelper::disposeComponent(xProp);
        }
    }

    // remove our elements
    m_aDocuments.clear();
    //  !!! do this before clearing the map which the vector elements refer to !!!
    m_aDocumentMap.clear();
}

//--------------------------------------------------------------------------
ODefinitionContainer::~ODefinitionContainer()
{
    DBG_DTOR(ODefinitionContainer, NULL);
}

IMPLEMENT_FORWARD_XINTERFACE2( ODefinitionContainer,OContentHelper,ODefinitionContainer_Base)
IMPLEMENT_TYPEPROVIDER2(ODefinitionContainer,OContentHelper,ODefinitionContainer_Base);
// XServiceInfo
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODefinitionContainer::getImplementationName(  ) throw(RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.ODefinitionContainer"));
}
//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODefinitionContainer::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aReturn(2);
    aReturn.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DefinitionContainer"));
    aReturn.getArray()[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.Content"));
    return aReturn;
}

// XNameContainer
//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::insertByName( const ::rtl::OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    ClearableMutexGuard aGuard(m_aMutex);

    implInsert(_rName,aElement);

    Reference< XContent > xNewElement(aElement,UNO_QUERY);
    notifyByName(aGuard,_rName,xNewElement,NULL,E_INSERTED);
}
// -----------------------------------------------------------------------------
void ODefinitionContainer::implInsert(const ::rtl::OUString& _rName, const Any& aElement)
{
    if (checkExistence(_rName))
        throw ElementExistException(_rName,*this);

    // approve the new object
    Reference< XContent > xNewElement(aElement,UNO_QUERY);
    if (!approveNewObject(_rName,xNewElement))
        throw IllegalArgumentException();

    implAppend(_rName, xNewElement);
}
//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::removeByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Reference< XContent > xOldElement;
    ClearableMutexGuard aGuard(m_aMutex);
    {
        // check the arguments
        if (!_rName.getLength())
            throw IllegalArgumentException();

        if (!checkExistence(_rName))
            throw NoSuchElementException(_rName,*this);

        // the old element (for the notifications)
        xOldElement = implGetByName(_rName, (m_aContainerListeners.getLength() != 0));
            // as this is potentially expensive (if the object is not already created and initialized from the registry)
            // we load the element only if we have listeners which may be interested in

        // do the removal
        implRemove(_rName);

        //  disposeComponent(xOldElement); // no dispose here, the object amy be inserted again unde a different name
    }
    notifyByName(aGuard,_rName,NULL,NULL,E_REMOVED);
}

// XNameReplace
//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::replaceByName( const ::rtl::OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ClearableMutexGuard aGuard(m_aMutex);

    // check the arguments
    if (!_rName.getLength())
        throw IllegalArgumentException();

    // let derived classes approve the new object
    Reference< XContent > xNewElement(aElement,UNO_QUERY);
    if (!approveNewObject(_rName,xNewElement))
        throw IllegalArgumentException();


    // the old element (for the notifications)
    Reference< XContent > xOldElement = implGetByName(_rName, m_aContainerListeners.getLength() != 0);
        // as this is potentially expensive (if the object is not already created and initialized from the registry)
        // we get the element only if we have listeners which may be interested in

    // do the replace
    implReplace(_rName, xNewElement);

    // and dispose it
    disposeComponent(xOldElement);
    notifyByName(aGuard,_rName,xNewElement,xOldElement,E_REPLACED);
}
// -----------------------------------------------------------------------------
void ODefinitionContainer::notifyByName( ClearableMutexGuard& _rGuard
                                        ,const ::rtl::OUString& _rName
                                        ,const Reference< XContent >& _xNewElement
                                        ,const Reference< XContent >& _xOldElement
                                        ,ENotifyKind _eHowToNotify)
{
    _rGuard.clear();
    // notify the listeners
    if (m_aContainerListeners.getLength())
    {
        ContainerEvent aEvent(*this, makeAny(_rName), makeAny(_xNewElement), makeAny(_xOldElement));
        OInterfaceIteratorHelper aListenerIterator(m_aContainerListeners);
        while (aListenerIterator.hasMoreElements())
        {
            switch( _eHowToNotify )
            {
                case E_REPLACED:
                    static_cast< XContainerListener* >(aListenerIterator.next())->elementReplaced(aEvent);
                    break;
                case E_REMOVED:
                    static_cast< XContainerListener* >(aListenerIterator.next())->elementRemoved(aEvent);
                    break;
                case E_INSERTED:
                    static_cast< XContainerListener* >(aListenerIterator.next())->elementInserted(aEvent);
                    break;
            }
        }
    }
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

// XElementAccess
//--------------------------------------------------------------------------
Type SAL_CALL ODefinitionContainer::getElementType( ) throw (RuntimeException)
{
    return ::getCppuType( static_cast< Reference< XContent >* >(NULL) );
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODefinitionContainer::hasElements( ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return !m_aDocuments.empty();
}

// XEnumerationAccess
//--------------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL ODefinitionContainer::createEnumeration(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return new ::comphelper::OEnumerationByIndex(static_cast<XIndexAccess*>(this));
}

//--------------------------------------------------------------------------
// XIndexAccess
sal_Int32 SAL_CALL ODefinitionContainer::getCount(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_aDocuments.size();
}

//--------------------------------------------------------------------------
Any SAL_CALL ODefinitionContainer::getByIndex( sal_Int32 _nIndex ) throw(IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    if ((_nIndex < 0) || (_nIndex >= (sal_Int32)m_aDocuments.size()))
        throw IndexOutOfBoundsException();

    Documents::iterator aPos = m_aDocuments[_nIndex];
    Reference<XContent> xProp = aPos->second;
    if (!xProp.is())
    {   // that's the first access to the object
        // -> create it
        xProp = createObject(aPos->first);
        aPos->second = Documents::mapped_type();
        // and update the name-access map
    }

    return makeAny(xProp);
}

//--------------------------------------------------------------------------
Any SAL_CALL ODefinitionContainer::getByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    return makeAny(implGetByName(_rName));
}

//--------------------------------------------------------------------------
Reference< XContent > ODefinitionContainer::implGetByName(const ::rtl::OUString& _rName, sal_Bool _bReadIfNeccessary) throw (NoSuchElementException)
{
    Documents::iterator aMapPos = m_aDocumentMap.find(_rName);
    if (aMapPos == m_aDocumentMap.end())
        throw NoSuchElementException(_rName,*this);

    Reference< XContent > xProp = aMapPos->second;

    if (_bReadIfNeccessary && !xProp.is())
    {   // the object has never been accessed before, so we have to read it now
        // (that's the expensive part)

        // create the object and insert it into the map
        xProp = createObject(_rName);
        aMapPos->second = xProp;
        addObjectListener(xProp);
    }

    return xProp;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL ODefinitionContainer::getElementNames(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Sequence< ::rtl::OUString > aNames(m_aDocumentMap.size());
    ::rtl::OUString* pNames = aNames.getArray();
    Documents::iterator aEnd = m_aDocumentMap.end();
    for (   Documents::iterator aNameIter = m_aDocumentMap.begin();
            aNameIter != aEnd;
            ++pNames, ++aNameIter
        )
    {
        *pNames = aNameIter->first;
    }

    return aNames;
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL ODefinitionContainer::hasByName( const ::rtl::OUString& _rName ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    return checkExistence(_rName);
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::disposing( const EventObject& _rSource ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Reference< XContent > xSource(_rSource.Source, UNO_QUERY);
    // it's one of our documents ....
    Documents::iterator aIter = m_aDocumentMap.begin();
    Documents::iterator aEnd = m_aDocumentMap.end();
    for (;aIter != aEnd;++aIter )
    {
        if ( xSource == aIter->second.get() )
        {
            removeObjectListener(xSource);
            // and clear our document map/vector, so the object will be recreated on next access
            aIter->second = Documents::mapped_type();
        }
    }
}

//--------------------------------------------------------------------------
sal_Bool ODefinitionContainer::isReadOnly() const
{
    return sal_False;
}

//--------------------------------------------------------------------------
void ODefinitionContainer::implRemove(const ::rtl::OUString& _rName)
{
    // from the object maps
    Documents::iterator aFind = m_aDocumentMap.find(_rName);
    if ( aFind != m_aDocumentMap.end() )
    {
        m_aDocuments.erase( ::std::find(m_aDocuments.begin(),m_aDocuments.end(),aFind));
        m_aDocumentMap.erase(aFind);

        ODefinitionContainer_Impl* pItem = static_cast<ODefinitionContainer_Impl*>(m_pImpl.get());
        pItem->m_aDocumentMap.erase(_rName);

        notifyDataSourceModified();
    }
}

//--------------------------------------------------------------------------
void ODefinitionContainer::implAppend(const ::rtl::OUString& _rName, const Reference< XContent >& _rxNewObject)
{
    MutexGuard aGuard(m_aMutex);
    try
    {
        Reference<XChild> xChild(_rxNewObject,UNO_QUERY);
        if ( xChild.is() )
            xChild->setParent(static_cast<OWeakObject*>(this));

        ODefinitionContainer_Impl* pItem = static_cast<ODefinitionContainer_Impl*>(m_pImpl.get());
        ODefinitionContainer_Impl::Documents::iterator aFind = pItem->m_aDocumentMap.find(_rName);
        if ( aFind == pItem->m_aDocumentMap.end() )
        {
            Reference<XUnoTunnel> xUnoTunnel(_rxNewObject,UNO_QUERY);
            ::rtl::Reference<OContentHelper> pContent = NULL;
            if ( xUnoTunnel.is() )
            {
                pContent = reinterpret_cast<OContentHelper*>(xUnoTunnel->getSomething(OContentHelper::getUnoTunnelImplementationId()));
                TContentPtr pImpl = pContent->getImpl();
                ODefinitionContainer_Impl::Documents::iterator aIter = ::std::find_if(pItem->m_aDocumentMap.begin(),pItem->m_aDocumentMap.end(),
                                                                    ::std::compose1(
                                                                        ::std::bind2nd(::std::equal_to<TContentPtr>(), pImpl),
                                                                        ::std::select2nd<ODefinitionContainer_Impl::Documents::value_type>())
                                                                    );

                if ( aIter != pItem->m_aDocumentMap.end() )
                    pItem->m_aDocumentMap.erase(aIter);

                pImpl->m_aProps.aTitle = _rName;
                pItem->m_aDocumentMap.insert(ODefinitionContainer_Impl::Documents::value_type(_rName,pImpl));
            }
        }


        m_aDocuments.push_back(m_aDocumentMap.insert(Documents::value_type(_rName,_rxNewObject)).first);
        notifyDataSourceModified();
        // now update our structures
        if ( _rxNewObject.is() )
            addObjectListener(_rxNewObject);
    }
    catch(Exception&)
    {
        DBG_ERROR("ODefinitionContainer::implAppend : catched something !");
    }
}

//--------------------------------------------------------------------------
void ODefinitionContainer::implReplace(const ::rtl::OUString& _rName, const Reference< XContent >& _rxNewObject)
{
    DBG_ASSERT(checkExistence(_rName), "ODefinitionContainer::implReplace : invalid name !");

    Documents::iterator aFind = m_aDocumentMap.find(_rName);
    removeObjectListener(aFind->second);
    aFind->second = _rxNewObject;
    addObjectListener(aFind->second);
}
// -----------------------------------------------------------------------------
sal_Bool ODefinitionContainer::approveNewObject(const ::rtl::OUString& _sName,const Reference< XContent >& _rxObject) const
{
    return _rxObject.is();
}
// -----------------------------------------------------------------------------
// XPropertyChangeListener
void SAL_CALL ODefinitionContainer::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException)
{
    ClearableMutexGuard aGuard(m_aMutex);
    if(evt.PropertyName == (rtl::OUString) PROPERTY_NAME)
    {
        m_bInPropertyChange = sal_True;
        try
        {
            ::rtl::OUString sNewName,sOldName;
            evt.OldValue >>= sOldName;
            evt.NewValue >>= sNewName;
            Reference<XContent> xProp(evt.Source,UNO_QUERY);
            removeObjectListener(xProp);
            implRemove(sOldName);
            implInsert(sNewName,makeAny(evt.Source));
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Exception catched!");
            throw RuntimeException();
        }
        m_bInPropertyChange = sal_False;
    }
}
// -----------------------------------------------------------------------------
// XVetoableChangeListener
void SAL_CALL ODefinitionContainer::vetoableChange( const PropertyChangeEvent& aEvent ) throw (PropertyVetoException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    if(aEvent.PropertyName == (rtl::OUString) PROPERTY_NAME)
    {
        ::rtl::OUString sNewName;
        aEvent.NewValue >>= sNewName;
        if(hasByName(sNewName))
            throw PropertyVetoException();
    }
}
// -----------------------------------------------------------------------------
void ODefinitionContainer::addObjectListener(const Reference< XContent >& _xNewObject)
{
    OSL_ENSURE(_xNewObject.is(),"ODefinitionContainer::addObjectListener: Object is null!");
    Reference<XPropertySet> xProp(_xNewObject,UNO_QUERY);
    if ( xProp.is() )
    {
        xProp->addPropertyChangeListener(PROPERTY_NAME, this);
        xProp->addVetoableChangeListener(PROPERTY_NAME, this);
    }
}
// -----------------------------------------------------------------------------
void ODefinitionContainer::removeObjectListener(const Reference< XContent >& _xNewObject)
{
    OSL_ENSURE(_xNewObject.is(),"ODefinitionContainer::addObjectListener: Object is null!");
    Reference<XPropertySet> xProp(_xNewObject,UNO_QUERY);
    if ( xProp.is() )
    {
        xProp->removePropertyChangeListener(PROPERTY_NAME, this);
        xProp->removeVetoableChangeListener(PROPERTY_NAME, this);
    }
}
// -----------------------------------------------------------------------------
sal_Bool ODefinitionContainer::checkExistence(const ::rtl::OUString& _rName)
{
    return m_aDocumentMap.find(_rName) != m_aDocumentMap.end();
}

//........................................................................
}
// namespace dbaccess
//........................................................................
