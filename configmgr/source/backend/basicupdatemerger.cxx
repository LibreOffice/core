/*************************************************************************
 *
 *  $RCSfile: basicupdatemerger.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-05-27 10:35:00 $
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

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

BasicUpdateMerger::BasicUpdateMerger( ResultHandler const & _xResultHandler )
: m_xResultHandler(_xResultHandler)
, m_nNesting(0)
, m_bSkipping(false)
{
    if (!m_xResultHandler.is())
    {
        OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("UpdateMerger: Error - NULL output handler unexpected") );
        throw uno::RuntimeException(sMsg,NULL);
    }
}
// -----------------------------------------------------------------------------

BasicUpdateMerger::~BasicUpdateMerger()
{
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::startLayer(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    if (m_nNesting)
        raiseMalformedDataException("UpdateMerger: Cannot start layer - layer already in progress");

    m_bSkipping = false;

    m_xResultHandler->startLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endLayer(  )
        throw (MalformedDataException, lang::IllegalAccessException, uno::RuntimeException)
{
    if (m_nNesting > 0)
        raiseMalformedDataException("UpdateMerger: Cannot end layer - layer still in progress");

    m_xResultHandler->endLayer();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::overrideNode( const OUString& aName, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->overrideNode(aName, aAttributes);

    pushLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addOrReplaceNode(aName, aAttributes);

    pushLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
        throw (MalformedDataException, container::NoSuchElementException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);

    pushLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endNode(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->endNode();

    popLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::dropNode( const OUString& aName )
        throw (MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->dropNode(aName);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (MalformedDataException, beans::UnknownPropertyException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->overrideProperty(aName, aAttributes, aType);

    pushLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::endProperty(  )
        throw (MalformedDataException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->endProperty();

    popLevel();
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::setPropertyValue( const uno::Any& aValue )
        throw (MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->setPropertyValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::setPropertyValueForLocale( const uno::Any& aValue, const OUString & aLocale )
        throw (MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->setPropertyValueForLocale(aValue,aLocale);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addProperty(aName, aAttributes, aType);
}
// -----------------------------------------------------------------------------

void SAL_CALL BasicUpdateMerger::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
        throw (MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (!isSkipping())
        m_xResultHandler->addPropertyWithValue(aName, aAttributes, aValue);
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::raiseMalformedDataException(sal_Char const * pMsg)
{
    OUString sMsg = OUString::createFromAscii(pMsg);

    throw MalformedDataException(sMsg, *this);
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::startSkipping()
{
    OSL_PRECOND( ! isHandling(), "BasicUpdateMerger: starting to skip, while already forwarding data");
    m_nNesting = 1;
    m_bSkipping = true;
    OSL_POSTCOND( isHandling(), "BasicUpdateMerger: isHandling() is broken");
    OSL_POSTCOND( isSkipping(), "BasicUpdateMerger: isSkipping() is broken");
}
// -----------------------------------------------------------------------------


void BasicUpdateMerger::pushLevel()
{
    ++m_nNesting;
    OSL_POSTCOND( isHandling(), "BasicUpdateMerger: level counting is broken");
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::popLevel()
{
    OSL_PRECOND( isHandling(), "BasicUpdateMerger: ending a node that wasn't handled here");
    if (m_nNesting != 0)
        if (--m_nNesting == 0)
            m_bSkipping = false;
}
// -----------------------------------------------------------------------------


void BasicUpdateMerger::findContext(OUString const & _aContext)
{
}
// -----------------------------------------------------------------------------

void BasicUpdateMerger::leaveContext()
{
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

