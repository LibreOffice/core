/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertiesfilterednotifier.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:16:32 $
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

#include "propertiesfilterednotifier.hxx"

#include <cppuhelper/queryinterface.hxx>

namespace configmgr
{
    namespace uno   = ::com::sun::star::uno;
    namespace lang  = ::com::sun::star::lang;
    namespace beans = ::com::sun::star::beans;

    using ::rtl::OUString;

    using uno::Any;
    using uno::Type;
    using uno::Reference;
    using uno::Sequence;

    using lang::EventObject;

    using beans::PropertyChangeEvent;
//-----------------------------------------------------------------------------
//  class PropertiesFilteredNotifier
//-----------------------------------------------------------------------------

PropertiesFilteredNotifier::PropertiesFilteredNotifier(
    Reference< beans::XPropertiesChangeListener >const& xTarget,
    Sequence< OUString > const& aFilterNames
)
: m_aRefCount()
, m_xTarget(xTarget)
, m_aFilterNames(aFilterNames)
{
    OSL_ENSURE(xTarget.is(),"PropertiesFilteredNotifier: FORWARDING TO NULL LISTENER");
    OSL_ENSURE(aFilterNames.getLength() > 0,"PropertiesFilteredNotifier: FILTER IS EMPTY (no target)");
}
//-----------------------------------------------------------------------------

PropertiesFilteredNotifier::~PropertiesFilteredNotifier()
{
}
//-----------------------------------------------------------------------------

void SAL_CALL PropertiesFilteredNotifier::acquire() throw()
{
    m_aRefCount.acquire();
}
//-----------------------------------------------------------------------------

void SAL_CALL PropertiesFilteredNotifier::release(  ) throw()
{
    if (m_aRefCount.release() == 0)
        delete this;
}
//-----------------------------------------------------------------------------

uno::Any SAL_CALL PropertiesFilteredNotifier::queryInterface( const Type& aType )
    throw(uno::RuntimeException)
{
    return cppu::queryInterface(aType
                , static_cast< beans::XPropertiesChangeListener *>(this)
                , static_cast< lang::XEventListener *>(this)
                , static_cast< uno::XInterface *>(this)
            );
}
//-----------------------------------------------------------------------------

void SAL_CALL PropertiesFilteredNotifier::disposing( const EventObject& Source )
    throw(uno::RuntimeException)
{
    if (m_xTarget.is())
        m_xTarget->disposing(Source);
}
//-----------------------------------------------------------------------------

inline // private and only used twice
bool PropertiesFilteredNotifier::implAccept(const ::com::sun::star::beans::PropertyChangeEvent& evt) const
{
    // todo: optimize by presorting and binary searching
    sal_Int32 const nCount = m_aFilterNames.getLength();

    for (sal_Int32 i = 0; i<nCount; ++i)
        if (evt.PropertyName == m_aFilterNames[i])
            return true;
    return false;
}
//-----------------------------------------------------------------------------

// private and only used once
Sequence< PropertyChangeEvent > PropertiesFilteredNotifier::implFilter(const Sequence< PropertyChangeEvent >& evt)  const
{
    sal_Int32 const nSize = evt.getLength();
    sal_Int32 nAccepted = 0;

    while ( nAccepted < nSize && implAccept(evt[nAccepted]) )
        ++nAccepted;

    if (nAccepted == nSize) // all accepted
        return evt;

    // create a modified copy
    Sequence< PropertyChangeEvent > aResult(evt);
    for (sal_Int32 nCur = nAccepted+1; nCur<nSize; ++nCur)
    {
        if (implAccept(evt[nCur]))
        {
            aResult[nAccepted++] = evt[nCur];
        }
    }
    aResult.realloc(nAccepted);
    OSL_ASSERT(aResult.getLength() == nAccepted);

    return aResult;
}
//-----------------------------------------------------------------------------

void SAL_CALL PropertiesFilteredNotifier::propertiesChange( const Sequence< PropertyChangeEvent >& evt )
    throw(uno::RuntimeException)
{
    Sequence< PropertyChangeEvent > aFilteredEvt( implFilter(evt) );

    if (aFilteredEvt.getLength() > 0)
    {
        if (m_xTarget.is())
            m_xTarget->propertiesChange(aFilteredEvt);
    }
}
//-----------------------------------------------------------------------------

}


