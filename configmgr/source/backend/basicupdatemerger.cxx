/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basicupdatemerger.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "basicupdatemerger.hxx"
#include "layerdefaultremover.hxx"

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#ifndef INCLUDED_ITERATOR
#include <iterator>
#define INCLUDED_ITERATOR
#endif

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

BasicUpdateMerger::BasicUpdateMerger( uno::Reference< backenduno::XLayer > const & _xSourceLayer )
: m_xSourceLayer(_xSourceLayer)
, m_xResultHandler()
, m_nNesting(0)
, m_bSkipping(false)
{
}
// -----------------------------------------------------------------------------

BasicUpdateMerger::~BasicUpdateMerger()
{
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::readData( uno::Reference< backenduno::XLayerHandler > const & _xResultHandler )
    throw ( backenduno::MalformedDataException, lang::NullPointerException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    if (!_xResultHandler.is())
    {
        rtl::OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("UpdateMerger: Error - NULL output handler unexpected") );
        throw lang::NullPointerException(sMsg,*this);
    }
    if (!m_xSourceLayer.is())
    {
        rtl::OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("UpdateMerger: Error - No source layer set") );
        throw lang::NullPointerException(sMsg,*this);
    }

    try
    {
        m_xResultHandler = new LayerDefaultRemover(_xResultHandler);
        m_xSourceLayer->readData( this );
    }
    catch (uno::Exception & )
    {
        m_xResultHandler.clear();
        throw;
    }

    m_xResultHandler.clear();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::startLayer(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (m_nNesting)
        raiseMalformedDataException("UpdateMerger: Cannot start layer - layer already in progress");

    m_bSkipping = false;

    m_xResultHandler->startLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endLayer(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (m_nNesting > 0)
        raiseMalformedDataException("UpdateMerger: Cannot end layer - data handling still in progress");

    this->flushContext();

    m_xResultHandler->endLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->overrideNode(aName, aAttributes, bClear);

    pushLevel(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addOrReplaceNode(aName, aAttributes);

    pushLevel(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);

    pushLevel(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endNode(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->endNode();

    popLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::dropNode( const rtl::OUString& aName )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->dropNode(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->overrideProperty(aName, aAttributes, aType, bClear);

    pushLevel( rtl::OUString() ); // do not match context path to property names
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endProperty(  )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->endProperty();

    popLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::setPropertyValue( const uno::Any& aValue )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->setPropertyValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString & aLocale )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->setPropertyValueForLocale(aValue,aLocale);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addProperty(aName, aAttributes, aType);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addPropertyWithValue(aName, aAttributes, aValue);
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::raiseMalformedDataException(sal_Char const * pMsg)
{
    rtl::OUString sMsg = rtl::OUString::createFromAscii(pMsg);

    throw backenduno::MalformedDataException(sMsg, *this, uno::Any());
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::startSkipping()
{
    OSL_PRECOND( m_nNesting == 0, "BasicUpdateMerger: starting to skip, while already forwarding data");
    m_nNesting = 1;
    m_bSkipping = true;
    OSL_POSTCOND( isHandling(), "BasicUpdateMerger: isHandling() is broken");
    OSL_POSTCOND( isSkipping(), "BasicUpdateMerger: isSkipping() is broken");
}
// -----------------------------------------------------------------------------


void BasicUpdateMerger::pushLevel(rtl::OUString const & _aContext)
{
    if (m_nNesting > 0)
    {
        ++m_nNesting;
        OSL_POSTCOND( isHandling(), "BasicUpdateMerger: level counting is broken" );
    }
    else if (m_nNesting < 0)
    {
        OSL_POSTCOND( isHandling(), "BasicUpdateMerger: level counting is broken" );
    }
    else if (m_aSearchPath.empty())
    {
        ++m_nNesting;
        OSL_POSTCOND( isHandling(), "BasicUpdateMerger: level counting is broken" );
    }
    else if ( m_aSearchPath.back().equals(_aContext) ) // search path is reverse - see findContext()
    {
        OSL_ENSURE( m_nNesting == 0, "BasicUpdateMerger: level count while searching must be zero");

        m_aSearchPath.pop_back();
    }
    else // start forwarding
    {
        m_nNesting = 1;
        OSL_POSTCOND( isHandling(), "BasicUpdateMerger: level counting is broken" );
        OSL_POSTCOND(!isSkipping(), "BasicUpdateMerger: skip flag set while searching " );
    }
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::popLevel()
{
    OSL_PRECOND( isHandling(), "BasicUpdateMerger: ending a node that wasn't handled here");
    if (m_nNesting > 0)
    {
        if (--m_nNesting == 0)
            m_bSkipping = false;
    }
    else if (m_nNesting == 0) // ending a context level, but the context is not yet gone
    {
        OSL_ENSURE( !m_aSearchPath.empty(), "BasicUpdateMerger: flushing a context that was already found");
        flushContext();
        leaveContext();
    }
    else
    {
        OSL_ENSURE( m_aSearchPath.empty(), "BasicUpdateMerger: Left an unfinished context" );
    }
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::findContext(std::vector<rtl::OUString> const & _aContext)
{
    // make the context a *reverse* copy of the context path
    OSL_PRECOND( ! isHandling(), "BasicUpdateMerger: starting context search while still handling data");
    m_aSearchPath.clear();
    m_aSearchPath.reserve(_aContext.size());
    std::copy( _aContext.rbegin(), _aContext.rend(), std::back_inserter(m_aSearchPath) );
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::leaveContext()
{
    OSL_PRECOND( !isHandling(), "BasicUpdateMerger: ending the context while still handling data or seaching the context");

    // force
    m_nNesting = -1;

    OSL_POSTCOND( ! isSkipping(), "BasicUpdateMerger: ending the context node while still skipping data");
    OSL_POSTCOND( isHandling(), "BasicUpdateMerger: cannot mark context as being handled to the end.");
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::flushContext()
{
    std::vector<rtl::OUString>::size_type nNesting = m_aSearchPath.size();

    while (!m_aSearchPath.empty())
    {
        m_xResultHandler->overrideNode(m_aSearchPath.back(), 0, false);
        m_aSearchPath.pop_back();
    }
    this->flushUpdate();

    while (nNesting > 0)
    {
        m_xResultHandler->endNode();
        --nNesting;
    }
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

