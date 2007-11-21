/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: definitioncontainer.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:38:24 $
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
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
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
#ifndef _COM_SUN_STAR_SDB_ERRORCONDITION_HPP_
#include <com/sun/star/sdb/ErrorCondition.hpp>
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
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::com::sun::star::ucb;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= ODefinitionContainer_Impl
//==========================================================================
//--------------------------------------------------------------------------
void ODefinitionContainer_Impl::erase( TContentPtr _pDefinition )
{
    NamedDefinitions::iterator aPos = find( _pDefinition );
    if ( aPos != end() )
        m_aDefinitions.erase( aPos );
}

//--------------------------------------------------------------------------
ODefinitionContainer_Impl::const_iterator ODefinitionContainer_Impl::find( TContentPtr _pDefinition ) const
{
    return ::std::find_if(
        m_aDefinitions.begin(),
        m_aDefinitions.end(),
        ::std::compose1(
            ::std::bind2nd( ::std::equal_to< TContentPtr >(), _pDefinition ),
            ::std::select2nd< NamedDefinitions::value_type >()
        )
    );
}

//--------------------------------------------------------------------------
ODefinitionContainer_Impl::iterator ODefinitionContainer_Impl::find( TContentPtr _pDefinition )
{
    return ::std::find_if(
        m_aDefinitions.begin(),
        m_aDefinitions.end(),
        ::std::compose1(
            ::std::bind2nd( ::std::equal_to< TContentPtr >(), _pDefinition ),
            ::std::select2nd< NamedDefinitions::value_type >()
        )
    );
}

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
    ,m_aApproveListeners(m_aMutex)
    ,m_aContainerListeners(m_aMutex)
    ,m_bInPropertyChange(sal_False)
{
    m_pImpl->m_aProps.bIsDocument = sal_False;
    m_pImpl->m_aProps.bIsFolder = sal_True;

    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    ODefinitionContainer_Impl::const_iterator aEnd = rDefinitions.end();
    for (   ODefinitionContainer_Impl::const_iterator aDefinition = rDefinitions.begin();
            aDefinition != aEnd;
            ++aDefinition
        )
        m_aDocuments.push_back(
            m_aDocumentMap.insert(
                Documents::value_type( aDefinition->first, Documents::mapped_type() ) ).first );

    DBG_CTOR(ODefinitionContainer, NULL);
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::disposing()
{
    OContentHelper::disposing();

    MutexGuard aGuard(m_aMutex);

    // say our listeners goobye
    EventObject aEvt(*this);
    m_aApproveListeners.disposeAndClear(aEvt);
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
    ResettableMutexGuard aGuard(m_aMutex);

    // approve the new object
    Reference< XContent > xNewElement(aElement,UNO_QUERY);
    approveNewObject( _rName, xNewElement );  // will throw if necessary

    notifyByName( aGuard, _rName, xNewElement, NULL, E_INSERTED, ApproveListeners );
    implAppend( _rName, xNewElement );
    notifyByName( aGuard, _rName, xNewElement, NULL, E_INSERTED, ContainerListemers );
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::removeByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ResettableMutexGuard aGuard(m_aMutex);

    // check the arguments
    if (!_rName.getLength())
        throw IllegalArgumentException();

    if (!checkExistence(_rName))
        throw NoSuchElementException(_rName,*this);

    // the old element (for the notifications)
    Reference< XContent > xOldElement = implGetByName( _rName, impl_haveAnyListeners_nothrow() );

    // do the removal
    notifyByName( aGuard, _rName, NULL, xOldElement, E_REMOVED, ApproveListeners );
    implRemove( _rName );
    notifyByName( aGuard, _rName, NULL, xOldElement, E_REMOVED, ContainerListemers );

    removeObjectListener( xOldElement );
    disposeComponent(xOldElement);
}

// XNameReplace
//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::replaceByName( const ::rtl::OUString& _rName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ResettableMutexGuard aGuard(m_aMutex);

    // let derived classes approve the new object
    Reference< XContent > xNewElement(aElement,UNO_QUERY);
    approveNewObject( _rName, xNewElement );    // will throw if necessary

    // the old element (for the notifications)
    Reference< XContent > xOldElement = implGetByName( _rName, impl_haveAnyListeners_nothrow() );

    notifyByName( aGuard, _rName, xNewElement, xOldElement, E_REPLACED, ApproveListeners );
    implReplace( _rName, xNewElement );
    notifyByName( aGuard, _rName, xNewElement, xOldElement, E_REPLACED, ContainerListemers );

    // and dispose it
    disposeComponent(xOldElement);
}

// -----------------------------------------------------------------------------
namespace
{
    typedef Reference< XVeto > ( SAL_CALL XContainerApproveListener::*ContainerApprovalMethod )( const ContainerEvent& );

    struct RaiseExceptionFromVeto
    {
    private:
        ContainerApprovalMethod m_pMethod;
        const ContainerEvent&   m_rEvent;

    public:
        RaiseExceptionFromVeto( ContainerApprovalMethod _pMethod, const ContainerEvent& _rEvent )
            :m_pMethod( _pMethod )
            ,m_rEvent( _rEvent )
        {
        }

        void operator()( const Reference< XContainerApproveListener >& _Listener ) const
        {
            Reference< XVeto > xVeto = (_Listener.get()->*m_pMethod)( m_rEvent );
            if ( !xVeto.is() )
                return;

            Any eVetoDetails = xVeto->getDetails();

            IllegalArgumentException aIllegalArgumentError;
            if ( eVetoDetails >>= aIllegalArgumentError )
                throw aIllegalArgumentError;

            WrappedTargetException aWrappedError;
            if ( eVetoDetails >>= aWrappedError )
                throw aWrappedError;

            throw WrappedTargetException( xVeto->getReason(), _Listener.get(), eVetoDetails );
        }
    };
}

// -----------------------------------------------------------------------------
void ODefinitionContainer::notifyByName( ResettableMutexGuard& _rGuard, const ::rtl::OUString& _rName,
        const Reference< XContent >& _xNewElement, const Reference< XContent >& _xOldElement,
        ContainerOperation _eOperation, ListenerType _eType )
{
    bool bApprove = ( _eType == ApproveListeners );

    ::cppu::OInterfaceContainerHelper& rContainer( bApprove ? m_aApproveListeners : m_aContainerListeners );
    if ( !rContainer.getLength() )
        return;

    ContainerEvent aEvent( *this, makeAny( _rName ), makeAny( _xNewElement ), makeAny( _xOldElement ) );

    _rGuard.clear();
    switch ( _eOperation )
    {
        case E_INSERTED:
            if ( bApprove )
                rContainer.forEach< XContainerApproveListener, RaiseExceptionFromVeto >(
                    RaiseExceptionFromVeto( &XContainerApproveListener::approveInsertElement, aEvent ) );
            else
                rContainer.notifyEach( &XContainerListener::elementInserted, aEvent );
            break;
        case E_REPLACED:
            if ( bApprove )
                rContainer.forEach< XContainerApproveListener, RaiseExceptionFromVeto >(
                    RaiseExceptionFromVeto( &XContainerApproveListener::approveReplaceElement, aEvent ) );
            else
                rContainer.notifyEach( &XContainerListener::elementReplaced, aEvent );
            break;
        case E_REMOVED:
            if ( bApprove )
                rContainer.forEach< XContainerApproveListener, RaiseExceptionFromVeto >(
                    RaiseExceptionFromVeto( &XContainerApproveListener::approveRemoveElement, aEvent ) );
            else
                rContainer.notifyEach( &XContainerListener::elementRemoved, aEvent );
            break;
    }

    if ( bApprove )
        _rGuard.reset();
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::addContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    if (_rxListener.is())
        m_aContainerListeners.addInterface(_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::removeContainerListener( const Reference< XContainerListener >& _rxListener ) throw(RuntimeException)
{
    if (_rxListener.is())
        m_aContainerListeners.removeInterface(_rxListener);
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::addContainerApproveListener( const Reference< XContainerApproveListener >& _Listener ) throw (RuntimeException)
{
    if ( _Listener.is() )
        m_aApproveListeners.addInterface( _Listener );
}

//--------------------------------------------------------------------------
void SAL_CALL ODefinitionContainer::removeContainerApproveListener( const Reference< XContainerApproveListener >& _Listener ) throw (RuntimeException)
{
    if ( _Listener.is() )
        m_aApproveListeners.removeInterface( _Listener );
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

    return makeAny( implGetByName( _rName, sal_True ) );
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
void ODefinitionContainer::implRemove(const ::rtl::OUString& _rName)
{
    // from the object maps
    Documents::iterator aFind = m_aDocumentMap.find(_rName);
    if ( aFind != m_aDocumentMap.end() )
    {
        m_aDocuments.erase( ::std::find(m_aDocuments.begin(),m_aDocuments.end(),aFind));
        m_aDocumentMap.erase(aFind);

        getDefinitions().erase( _rName );

        notifyDataSourceModified();
    }
}

//--------------------------------------------------------------------------
namespace
{
    bool    lcl_ensureName( const Reference< XContent >& _rxContent, const ::rtl::OUString& _rName )
    {
        if ( !_rxContent.is() )
            return true;

        // ..........................................................
        // obtain the current name. If it's the same as the new one,
        // don't do anything
        try
        {
            Reference< XPropertySet > xProps( _rxContent, UNO_QUERY );
            if ( xProps.is() )
            {
                ::rtl::OUString sCurrentName;
                OSL_VERIFY( xProps->getPropertyValue( PROPERTY_NAME ) >>= sCurrentName );
                if ( sCurrentName.equals( _rName ) )
                    return true;
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "lcl_ensureName: caught an exception while obtaining the current name!" );
        }

        // ..........................................................
        // set the new name
        Reference< XRename > xRename( _rxContent, UNO_QUERY );
        OSL_ENSURE( xRename.is(), "lcl_ensureName: invalid content (not renameable)!" );
        if ( !xRename.is() )
            return false;
        try
        {
            xRename->rename( _rName );
            return true;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "lcl_ensureName: caught an exception!" );
        }
        return false;
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

        ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
        ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( _rName );
        if ( aFind == rDefinitions.end() )
        {
            // ensure that the new object thas the proper name.
            // Somebody could create an object with name "foo", and insert it as "bar"
            // into a container. In this case, we need to ensure that the object name
            // is also "bar"
            // #i44786# / 2005-03-11 / frank.schoenheit@sun.com
            lcl_ensureName( _rxNewObject, _rName );

            ::rtl::Reference< OContentHelper > pContent = OContentHelper::getImplementation( _rxNewObject );
            if ( pContent.is() )
            {
                TContentPtr pImpl = pContent->getImpl();
                rDefinitions.erase( pImpl );
                pImpl->m_aProps.aTitle = _rName;
                rDefinitions.insert( _rName, pImpl );
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
        DBG_ERROR("ODefinitionContainer::implAppend: caught something !");
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
void ODefinitionContainer::approveNewObject(const ::rtl::OUString& _sName,const Reference< XContent >& _rxObject) const
{
    // check the arguments
    if ( !_sName.getLength() )
        throw IllegalArgumentException(
            DBA_RES( RID_STR_NAME_MUST_NOT_BE_EMPTY ),
            *this,
            0 );

    if ( _sName.indexOf( '/' ) != -1 )
        throw IllegalArgumentException(
            m_aErrorHelper.getErrorMessage( ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES ),
            *this,
            0 );

    if ( !_rxObject.is() )
        throw IllegalArgumentException(
            DBA_RES( RID_STR_NO_NULL_OBJECTS_IN_CONTAINER ),
            *this,
            0 );

    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    if ( rDefinitions.find( _sName ) != rDefinitions.end() )
        throw ElementExistException(
            DBA_RES( RID_STR_NAME_ALREADY_USED ),
            *this );

    ::rtl::Reference< OContentHelper > pContent( OContentHelper::getImplementation( _rxObject ) );
    if ( !pContent.is() )
        throw IllegalArgumentException(
            DBA_RES( RID_STR_OBJECT_CONTAINER_MISMATCH ),
            *this,
            1 );

    if ( rDefinitions.find( pContent->getImpl() ) != rDefinitions.end() )
        throw ElementExistException(
            DBA_RES( RID_STR_OBJECT_ALREADY_CONTAINED ),
            *this );
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
            Reference<XContent> xContent( evt.Source, UNO_QUERY );
            removeObjectListener( xContent );
            implRemove( sOldName );
            implAppend( sNewName, xContent );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
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
