/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Functions.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:52:19 $
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
#ifndef REPORTDESIGN_API_FUNCTIONS_HXX
#include "Functions.hxx"
#endif
#ifndef RPT_FUNCTION_HXX
#include "Function.hxx"
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
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#include <boost/bind.hpp>
#include <algorithm>
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
DBG_NAME( rpt_OFunctions )
// -----------------------------------------------------------------------------
OFunctions::OFunctions(const uno::Reference< report::XFunctionsSupplier >& _xParent,const uno::Reference< uno::XComponentContext >& context)
:FunctionsBase(m_aMutex)
,m_aContainerListeners(m_aMutex)
,m_xContext(context)
,m_xParent(_xParent)
{
    DBG_CTOR( rpt_OFunctions,NULL);
}
//--------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
OFunctions::~OFunctions()
{
    DBG_DTOR( rpt_OFunctions,NULL);
}
//--------------------------------------------------------------------------
void OFunctions::copyFunctions(const uno::Reference< report::XFunctions >& _xSource)
{
    sal_Int32 nCount = _xSource->getCount();
    for (sal_Int32 i = 0; i != nCount; ++i)
    {
        uno::Reference<report::XFunction> xDestFunction = new OFunction(m_xContext);
        m_aFunctions.push_back(xDestFunction);
        uno::Reference<report::XFunction> xSourceFunction(_xSource->getByIndex(i),uno::UNO_QUERY);
        ::comphelper::copyProperties(xSourceFunction.get(),xDestFunction.get());
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunctions::dispose() throw(uno::RuntimeException)
{
    cppu::WeakComponentImplHelperBase::dispose();
}
// -----------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function!
//
void SAL_CALL OFunctions::disposing()
{
    ::std::for_each(m_aFunctions.begin(),m_aFunctions.end(),::boost::mem_fn(&com::sun::star::report::XFunction::dispose));
    m_aFunctions.clear();
    lang::EventObject aDisposeEvent( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aContainerListeners.disposeAndClear( aDisposeEvent );
    m_xContext.clear();
}
// -----------------------------------------------------------------------------
// XFunctionsSupplier
// -----------------------------------------------------------------------------
uno::Reference< report::XFunction > SAL_CALL OFunctions::createFunction(  ) throw (uno::RuntimeException)
{
    return new OFunction(m_xContext);
}
// -----------------------------------------------------------------------------
// XIndexContainer
void SAL_CALL OFunctions::insertByIndex( ::sal_Int32 Index, const uno::Any& aElement ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        sal_Bool bAdd = (Index == static_cast<sal_Int32>(m_aFunctions.size()));
        if ( !bAdd )
            checkIndex(Index);
        uno::Reference< report::XFunction > xFunction(aElement,uno::UNO_QUERY);
        if ( !xFunction.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL,m_xContext->getServiceManager()),*this,2);

        if ( bAdd )
            m_aFunctions.push_back(xFunction);
        else
        {
            TFunctions::iterator aPos = m_aFunctions.begin();
            ::std::advance(aPos,Index);
            m_aFunctions.insert(aPos, xFunction);
        }
        xFunction->setParent(*this);
    }
    // notify our container listeners
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), aElement, uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementInserted,aEvent);
}

// -----------------------------------------------------------------------------
void SAL_CALL OFunctions::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< report::XFunction > xFunction;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        TFunctions::iterator aPos = m_aFunctions.begin();
        ::std::advance(aPos,Index);
        xFunction = *aPos;
        m_aFunctions.erase(aPos);
        xFunction->setParent(NULL);
    }
    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), uno::makeAny(xFunction), uno::Any());
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved,aEvent);
}
// -----------------------------------------------------------------------------
// XIndexReplace
void SAL_CALL OFunctions::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aOldElement;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        checkIndex(Index);
        uno::Reference< report::XFunction > xFunction(Element,uno::UNO_QUERY);
        if ( !xFunction.is() )
            throw lang::IllegalArgumentException(RPT_RESSTRING(RID_STR_ARGUMENT_IS_NULL,m_xContext->getServiceManager()),*this,2);
        TFunctions::iterator aPos = m_aFunctions.begin();
        ::std::advance(aPos,Index);
        aOldElement <<= *aPos;
        *aPos = xFunction;
    }

    container::ContainerEvent aEvent(static_cast<container::XContainer*>(this), uno::makeAny(Index), Element, aOldElement);
    m_aContainerListeners.notifyEach(&container::XContainerListener::elementReplaced,aEvent);
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 SAL_CALL OFunctions::getCount(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aFunctions.size();
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OFunctions::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkIndex(Index);
    TFunctions::iterator aPos = m_aFunctions.begin();
    ::std::advance(aPos,Index);
    return uno::makeAny(*aPos);
}
// -----------------------------------------------------------------------------
// XElementAccess
uno::Type SAL_CALL OFunctions::getElementType(  ) throw (uno::RuntimeException)
{
    return ::getCppuType(static_cast< uno::Reference<report::XFunction>*>(NULL));
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OFunctions::hasElements(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return !m_aFunctions.empty();
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OFunctions::getParent(  ) throw (uno::RuntimeException)
{
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunctions::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ ) throw (lang::NoSupportException, uno::RuntimeException)
{
    throw lang::NoSupportException();
}
// -----------------------------------------------------------------------------
// XContainer
void SAL_CALL OFunctions::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aContainerListeners.addInterface(xListener);
}
// -----------------------------------------------------------------------------
void SAL_CALL OFunctions::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    m_aContainerListeners.removeInterface(xListener);
}
// -----------------------------------------------------------------------------
void OFunctions::checkIndex(sal_Int32 _nIndex)
{
    if ( _nIndex < 0 || static_cast<sal_Int32>(m_aFunctions.size()) <= _nIndex )
        throw lang::IndexOutOfBoundsException();
}
// =============================================================================
}
// =============================================================================
