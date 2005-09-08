/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: enumhelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:45:10 $
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

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

//==================================================================
//= OEnumerationByName
//==================================================================
//------------------------------------------------------------------------------
OEnumerationByName::OEnumerationByName(const staruno::Reference<starcontainer::XNameAccess>& _rxAccess)
    :m_aNames(_rxAccess->getElementNames())
    ,m_nPos(0)
    ,m_xAccess(_rxAccess)
    ,m_bListening(sal_False)
{
    impl_startDisposeListening();
}

//------------------------------------------------------------------------------
OEnumerationByName::OEnumerationByName(const staruno::Reference<starcontainer::XNameAccess>& _rxAccess,
                                       const staruno::Sequence< ::rtl::OUString >&           _aNames  )
    :m_aNames(_aNames)
    ,m_nPos(0)
    ,m_xAccess(_rxAccess)
    ,m_bListening(sal_False)
{
    impl_startDisposeListening();
}

//------------------------------------------------------------------------------
OEnumerationByName::~OEnumerationByName()
{
    impl_stopDisposeListening();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OEnumerationByName::hasMoreElements(  ) throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_xAccess.is() && m_aNames.getLength() > m_nPos)
        return sal_True;

    if (m_xAccess.is())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    return sal_False;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OEnumerationByName::nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    staruno::Any aRes;
    if (m_xAccess.is() && m_nPos < m_aNames.getLength())
        aRes = m_xAccess->getByName(m_aNames.getConstArray()[m_nPos++]);

    if (m_xAccess.is() && m_nPos >= m_aNames.getLength())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    if (!aRes.hasValue())       // es gibt kein Element mehr
        throw starcontainer::NoSuchElementException();

    return aRes;
}

//------------------------------------------------------------------------------
void SAL_CALL OEnumerationByName::disposing(const starlang::EventObject& aEvent)
        throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (aEvent.Source == m_xAccess)
        m_xAccess.clear();
}

//------------------------------------------------------------------------------
void OEnumerationByName::impl_startDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->addEventListener(this);
        m_bListening = sal_True;
    }
    --m_refCount;
}

//------------------------------------------------------------------------------
void OEnumerationByName::impl_stopDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (!m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->removeEventListener(this);
        m_bListening = sal_False;
    }
    --m_refCount;
}

//==================================================================
//= OEnumerationByIndex
//==================================================================
//------------------------------------------------------------------------------
OEnumerationByIndex::OEnumerationByIndex(const staruno::Reference< starcontainer::XIndexAccess >& _rxAccess)
    :m_xAccess(_rxAccess)
    ,m_nPos(0)
    ,m_bListening(sal_False)
{
    impl_startDisposeListening();
}

//------------------------------------------------------------------------------
OEnumerationByIndex::~OEnumerationByIndex()
{
    impl_stopDisposeListening();
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OEnumerationByIndex::hasMoreElements(  ) throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_xAccess.is() && m_xAccess->getCount() > m_nPos)
        return sal_True;

    if (m_xAccess.is())
    {
        impl_stopDisposeListening();
        m_xAccess.clear();
    }

    return sal_False;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OEnumerationByIndex::nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    staruno::Any aRes;
    if (m_xAccess.is())
    {
        aRes = m_xAccess->getByIndex(m_nPos++);
        if (m_nPos >= m_xAccess->getCount())
        {
            impl_stopDisposeListening();
            m_xAccess.clear();
        }
    }

    if (!aRes.hasValue())       // es gibt kein Element mehr
        throw starcontainer::NoSuchElementException();
    return aRes;
}

//------------------------------------------------------------------------------
void SAL_CALL OEnumerationByIndex::disposing(const starlang::EventObject& aEvent)
        throw(staruno::RuntimeException)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (aEvent.Source == m_xAccess)
        m_xAccess.clear();
}

//------------------------------------------------------------------------------
void OEnumerationByIndex::impl_startDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->addEventListener(this);
        m_bListening = sal_True;
    }
    --m_refCount;
}

//------------------------------------------------------------------------------
void OEnumerationByIndex::impl_stopDisposeListening()
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    if (!m_bListening)
        return;

    ++m_refCount;
    staruno::Reference< starlang::XComponent > xDisposable(m_xAccess, staruno::UNO_QUERY);
    if (xDisposable.is())
    {
        xDisposable->removeEventListener(this);
        m_bListening = sal_False;
    }
    --m_refCount;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................


