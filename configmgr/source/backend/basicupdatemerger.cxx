/*************************************************************************
 *
 *  $RCSfile: basicupdatemerger.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:13:57 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

BasicUpdateMerger::BasicUpdateMerger( LayerSource const & _xSourceLayer )
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

void SAL_CALL BasicUpdateMerger::readData( ResultHandler const & _xResultHandler )
    throw ( MalformedDataException, lang::NullPointerException,
            lang::WrappedTargetException, uno::RuntimeException)
{
    if (!_xResultHandler.is())
    {
        OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("UpdateMerger: Error - NULL output handler unexpected") );
        throw lang::NullPointerException(sMsg,*this);
    }
    if (!m_xSourceLayer.is())
    {
        OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("UpdateMerger: Error - No source layer set") );
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
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (m_nNesting)
        raiseMalformedDataException("UpdateMerger: Cannot start layer - layer already in progress");

    m_bSkipping = false;

    m_xResultHandler->startLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endLayer(  )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (m_nNesting > 0)
        raiseMalformedDataException("UpdateMerger: Cannot end layer - data handling still in progress");

    this->flushContext();

    m_xResultHandler->endLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->overrideNode(aName, aAttributes, bClear);

    pushLevel(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addOrReplaceNode(aName, aAttributes);

    pushLevel(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);

    pushLevel(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endNode(  )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->endNode();

    popLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::dropNode( const OUString& aName )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->dropNode(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->overrideProperty(aName, aAttributes, aType, bClear);

    pushLevel( OUString() ); // do not match context path to property names
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endProperty(  )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->endProperty();

    popLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::setPropertyValue( const uno::Any& aValue )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->setPropertyValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::setPropertyValueForLocale( const uno::Any& aValue, const OUString & aLocale )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->setPropertyValueForLocale(aValue,aLocale);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addProperty(aName, aAttributes, aType);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addPropertyWithValue(aName, aAttributes, aValue);
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::raiseMalformedDataException(sal_Char const * pMsg)
{
    OUString sMsg = OUString::createFromAscii(pMsg);

    throw MalformedDataException(sMsg, *this, uno::Any());
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


void BasicUpdateMerger::pushLevel(OUString const & _aContext)
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

void BasicUpdateMerger::findContext(ContextPath const & _aContext)
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
    ContextPath::size_type nNesting = m_aSearchPath.size();

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

