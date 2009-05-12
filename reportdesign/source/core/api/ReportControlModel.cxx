/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportControlModel.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "ReportControlModel.hxx"
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
namespace reportdesign
{
using namespace com::sun::star;
using namespace comphelper;

bool operator==( const ::com::sun::star::awt::FontDescriptor& _lhs, const ::com::sun::star::awt::FontDescriptor& _rhs )
{
    return  ( _lhs.Name           == _rhs.Name )
        &&  ( _lhs.Height         == _rhs.Height )
        &&  ( _lhs.Width          == _rhs.Width )
        &&  ( _lhs.StyleName      == _rhs.StyleName )
        &&  ( _lhs.Family         == _rhs.Family )
        &&  ( _lhs.CharSet        == _rhs.CharSet )
        &&  ( _lhs.Pitch          == _rhs.Pitch )
        &&  ( _lhs.CharacterWidth == _rhs.CharacterWidth )
        &&  ( _lhs.Weight         == _rhs.Weight )
        &&  ( _lhs.Slant          == _rhs.Slant )
        &&  ( _lhs.Underline      == _rhs.Underline )
        &&  ( _lhs.Strikeout      == _rhs.Strikeout )
        &&  ( _lhs.Orientation    == _rhs.Orientation )
        &&  ( _lhs.Kerning        == _rhs.Kerning )
        &&  ( _lhs.WordLineMode   == _rhs.WordLineMode )
        &&  ( _lhs.Type           == _rhs.Type );
}

// -----------------------------------------------------------------------------
// XContainer
void OReportControlModel::addContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    aContainerListeners.addInterface(xListener);
}
// -----------------------------------------------------------------------------
void OReportControlModel::removeContainerListener( const uno::Reference< container::XContainerListener >& xListener ) throw (uno::RuntimeException)
{
    aContainerListeners.removeInterface(xListener);
}
// -----------------------------------------------------------------------------
::sal_Bool OReportControlModel::hasElements(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return !m_aFormatConditions.empty();
}
// -----------------------------------------------------------------------------
// XIndexContainer
void OReportControlModel::insertByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference<report::XFormatCondition> xElement(Element,uno::UNO_QUERY);
    if ( !xElement.is() )
        throw lang::IllegalArgumentException();

    uno::Reference< container::XContainer > xBroadcaster;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        xBroadcaster = m_pOwner;
        if ( Index > static_cast<sal_Int32>(m_aFormatConditions.size()) )
            throw lang::IndexOutOfBoundsException();

        //m_aFormatConditions.resize(m_aFormatConditions.size() + 1);
        m_aFormatConditions.insert(m_aFormatConditions.begin() + Index,xElement);
    }

    // notify our container listeners
    container::ContainerEvent aEvent(xBroadcaster, uno::makeAny(Index), Element, uno::Any());
    aContainerListeners.notifyEach(&container::XContainerListener::elementInserted,aEvent);
}
// -----------------------------------------------------------------------------
void OReportControlModel::removeByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any Element;
    uno::Reference< container::XContainer > xBroadcaster;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        xBroadcaster = m_pOwner;
        checkIndex(Index);
        Element <<= m_aFormatConditions[Index];
        m_aFormatConditions.erase(m_aFormatConditions.begin() + Index);
    }
    container::ContainerEvent aEvent(xBroadcaster, uno::makeAny(Index), Element, uno::Any());
    aContainerListeners.notifyEach(&container::XContainerListener::elementRemoved,aEvent);
}
// -----------------------------------------------------------------------------
// XIndexReplace
void OReportControlModel::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element ) throw (lang::IllegalArgumentException, lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference<report::XFormatCondition> xElement(Element,uno::UNO_QUERY);
    if ( !xElement.is() )
        throw lang::IllegalArgumentException();
    uno::Reference< container::XContainer > xBroadcaster;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        xBroadcaster = m_pOwner;
        checkIndex(Index);
        m_aFormatConditions[Index] = xElement;
    }
    container::ContainerEvent aEvent(xBroadcaster, uno::makeAny(Index), Element, uno::Any());
    aContainerListeners.notifyEach(&container::XContainerListener::elementReplaced,aEvent);
}
// -----------------------------------------------------------------------------
// XIndexAccess
::sal_Int32 OReportControlModel::getCount(  ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    return m_aFormatConditions.size();
}
// -----------------------------------------------------------------------------
uno::Any OReportControlModel::getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aElement;
    {
        ::osl::MutexGuard aGuard(m_rMutex);
        checkIndex(Index);
        aElement <<= m_aFormatConditions[Index];
    }
    return aElement;
}
// -----------------------------------------------------------------------------
void OReportControlModel::checkIndex(sal_Int32 _nIndex)
{
    if ( _nIndex < 0 || static_cast<sal_Int32>(m_aFormatConditions.size()) <= _nIndex )
        throw lang::IndexOutOfBoundsException();
}
// -----------------------------------------------------------------------------
void OReportControlModel::dispose()
{
    m_aFormatConditions.clear();
    lang::EventObject aDisposeEvent( m_pOwner );
    aContainerListeners.disposeAndClear( aDisposeEvent );
    m_aFormatConditions.clear();
}
// -----------------------------------------------------------------------------
bool OReportControlModel::isInterfaceForbidden(const uno::Type& _rType)
{
    return (_rType == ::getCppuType((const uno::Reference< beans::XPropertyState>* )0) || _rType == ::getCppuType((const uno::Reference< beans::XMultiPropertySet>* )0));
}
// -----------------------------------------------------------------------------
} // reportdesign

