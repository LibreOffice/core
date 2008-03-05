/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Groups.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:52:55 $
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
#ifndef REPORTDESIGN_API_GROUPS_HXX
#include "Groups.hxx"
#endif
#ifndef REPORTDESIGN_API_GROUP_HXX
#include "Group.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef REPORTDESIGN_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef REPORTDESIGN_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#include <boost/bind.hpp>
#include <algorithm>
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
DBG_NAME( rpt_OGroups )
// -----------------------------------------------------------------------------
OGroups::OGroups(const uno::Reference< report::XReportDefinition >& _xParent,const uno::Reference< uno::XComponentContext >& context)
:GroupsBase(m_aMutex)
,m_aContainerListeners(m_aMutex)
,m_xContext(context)
,m_xParent(_xParent)
{
    DBG_CTOR( rpt_OGroups,NULL);
}
//--------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
OGroups::~OGroups()
{
    DBG_DTOR( rpt_OGroups,NULL);
}
//--------------------------------------------------------------------------
void OGroups::copyGroups(const uno::Reference< report::XGroups >& _xSource)
{
    sal_Int32 nCount = _xSource->getCount();
    for (sal_Int32 i = 0; i != nCount; ++i)
    {
        OGroup* pGroup = new OGroup(this,m_xContext);
        m_aGroups.push_back(pGroup);
        uno::Reference<report::XGroup> xGroup(_xSource->getByIndex(i),uno::UNO_QUERY);
        pGroup->copyGroup(xGroup);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroups::dispose() throw(uno::RuntimeException)
{
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
void SAL_CALL OGroups::disposing()
{
    ::std::for_each(m_aGroups.begin(),m_aGroups.end(),::boost::mem_fn(&com::sun::star::report::XGroup::dispose));
    m_aGroups.clear();
    lang::EventObject aDisposeEvent( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aContainerListeners.disposeAndClear( aDisposeEvent );
    m_xContext.clear();
}
// -----------------------------------------------------------------------------
// XGroups
uno::Reference< report::XReportDefinition > SAL_CALL OGroups::getReportDefinition() throw (uno::RuntimeException)
{
    return m_xParent;
}
// -----------------------------------------------------------------------------
uno::Reference< report::XGroup > SAL_CALL OGroups::createGroup(  ) throw (uno::RuntimeException)
{
    return new OGroup(this,m_xContext);
}
// -----------------------------------------------------------------------------
// XIndexContainer
void SAL_CALL OGroups::insertByIndex( ::sal_Int32 Index, const uno::Any& aElement ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        sal_Bool bAdd = (Index == static_cast<sal_Int32>(m_aGroups.size()));
        if ( !bAdd )
            checkIndex(Index);
        uno::Reference< report::XGroup > xGroup(aElement,uno::UNO_QUERY);
        if ( !xGroup.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL,m_xContext->getServiceManager()),*this,2);

        if ( bAdd )
            m_aGroups.push_back(xGroup);
        else
        {
            TGroups::iterator aPos = m_aGroups.begin();
            ::std::advance(aPos,Index);
            m_aGroups.insert(aPos, xGroup);
        }
    }
    // notify our container listeners
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), aElement, uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementInserted,aEvent);
}

// -----------------------------------------------------------------------------
void SAL_CALL OGroups::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< report::XGroup > xGroup;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        TGroups::iterator aPos = m_aGroups.begin();
        ::std::advance(aPos,Index);
        xGroup = *aPos;
        m_aGroups.erase(aPos);
    }
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), uno::makeAny(xGroup), uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved,aEvent);
}
// -----------------------------------------------------------------------------
// XIndexReplace
void SAL_CALL OGroups::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aOldElement;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        uno::Reference< report::XGroup > xGroup(Element,uno::UNO_QUERY);
        if ( !xGroup.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL,m_xContext->getServiceManager()),*this,2);
        TGroups::iterator aPos = m_aGroups.begin();
        ::std::advance(aPos,Index);
        aOldElement <<= *aPos;
        *aPos = xGroup;
    }

    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), Element, aOldElement);
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementReplaced,aEvent);
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 SAL_CALL OGroups::getCount(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aGroups.size();
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OGroups::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkIndex(Index);
    TGroups::iterator aPos = m_aGroups.begin();
    ::std::advance(aPos,Index);
    return uno::makeAny(*aPos);
}
// -----------------------------------------------------------------------------
// XElementAccess
uno::Type SAL_CALL OGroups::getElementType(  ) throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast< uno::Reference<report::XGroup>*>(NULL));
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroups::hasElements(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return !m_aGroups.empty();
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OGroups::getParent(  ) throw (uno::RuntimeException)
{
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroups::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ ) throw (lang::NoSupportException, uno::RuntimeException)
{
    throw lang::NoSupportException();
}
// -----------------------------------------------------------------------------
// XContainer
void SAL_CALL OGroups::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aContainerListeners.addInterface(xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroups::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aContainerListeners.removeInterface(xListener);
}
// -----------------------------------------------------------------------------
void OGroups::checkIndex(sal_Int32 _nIndex)
{
    if ( _nIndex < 0 || static_cast<sal_Int32>(m_aGroups.size()) <= _nIndex )
        throw lang::IndexOutOfBoundsException();
}
// =============================================================================
}
// =============================================================================
