/*************************************************************************
 *
 *  $RCSfile: layerwriter.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-05-27 10:39:04 $
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

#include "layerwriter.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace sax       = ::com::sun::star::xml::sax;
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateLayerWriter
( CreationContext const& rServiceManager )
{
    return * new LayerWriter(rServiceManager);
}
// -----------------------------------------------------------------------------

LayerWriter::LayerWriter(ServiceFactory const & _xSvcFactory)
: LayerWriterService_Base(_xSvcFactory)
, m_bInProperty(false)
{
}
// -----------------------------------------------------------------------------

LayerWriter::~LayerWriter()
{
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::startLayer(  )
        throw (backenduno::MalformedDataException, uno::RuntimeException)
{
    m_aFormatter.reset();
    m_bInProperty = false;

    checkInElement(false);

    getWriteHandler()->startDocument();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::endLayer(  )
        throw (backenduno::MalformedDataException, lang::IllegalAccessException, uno::RuntimeException)
{
    checkInElement(false);

    getWriteHandler()->endDocument();
    m_aFormatter.reset();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::overrideNode( const OUString& aName, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    ElementInfo aInfo(aName, ElementType::node);
    aInfo.flags = aAttributes;
    aInfo.op = Operation::modify;

    m_aFormatter.prepareElement(aInfo);

    this->startNode();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true);

    ElementInfo aInfo(aName, ElementType::node);
    aInfo.flags = aAttributes;
    aInfo.op = Operation::replace;

    m_aFormatter.prepareElement(aInfo);

    this->startNode();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::addOrReplaceNodeFromTemplate( const OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
        throw (backenduno::MalformedDataException, container::NoSuchElementException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true);

    ElementInfo aInfo(aName, ElementType::node);
    aInfo.flags = aAttributes;
    aInfo.op = Operation::replace;

    m_aFormatter.prepareElement(aInfo);
    m_aFormatter.addInstanceType(aTemplate.Name,aTemplate.Component);

    this->startNode();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::endNode(  )
        throw (backenduno::MalformedDataException, uno::RuntimeException)
{
    checkInElement(true);
    this->endElement();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::dropNode( const OUString& aName )
        throw (backenduno::MalformedDataException, container::NoSuchElementException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true);

    ElementInfo aInfo(aName, ElementType::node);
    aInfo.flags = 0;
    aInfo.op = Operation::remove;

    m_aFormatter.prepareElement(aInfo);

    this->startNode();
    this->endElement();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (backenduno::MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true);

    ElementInfo aInfo(aName, ElementType::property);
    aInfo.flags = aAttributes;
    aInfo.op = Operation::replace;

    m_aFormatter.prepareElement(aInfo);

    this->startProp(aType, true);
    this->writeValue(uno::Any());
    this->endElement();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
        throw (backenduno::MalformedDataException, beans::PropertyExistException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true);

    ElementInfo aInfo(aName, ElementType::property);
    aInfo.flags = aAttributes;
    aInfo.op = Operation::replace;

    m_aFormatter.prepareElement(aInfo);

    this->startProp(aValue.getValueType(), true);
    this->writeValue(aValue);
    this->endElement();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
        throw (backenduno::MalformedDataException, beans::UnknownPropertyException, beans::IllegalTypeException, lang::IllegalAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true);

    ElementInfo aInfo(aName, ElementType::property);
    aInfo.flags = aAttributes;
    aInfo.op = Operation::modify;

    m_aFormatter.prepareElement(aInfo);

    this->startProp(aType, false);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::endProperty(  )
        throw (backenduno::MalformedDataException, uno::RuntimeException)
{
    checkInElement(true,true);
    this->endElement();
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::setPropertyValue( const uno::Any& aValue )
        throw (backenduno::MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true,true);
    this->writeValue(aValue);
}
// -----------------------------------------------------------------------------

void SAL_CALL LayerWriter::setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
        throw (backenduno::MalformedDataException, beans::IllegalTypeException, lang::IllegalArgumentException, uno::RuntimeException)
{
    checkInElement(true,true);
    this->writeValue(aValue,aLocale);
}
// -----------------------------------------------------------------------------

void LayerWriter::raiseMalformedDataException(sal_Char const * pMsg)
{
    OSL_ASSERT(pMsg);
    OUString sMsg = OUString::createFromAscii(pMsg);

    throw backenduno::MalformedDataException( sMsg, *this );
}
// -----------------------------------------------------------------------------

void LayerWriter::raiseIllegalTypeException(sal_Char const * pMsg)
{
    OSL_ASSERT(pMsg);
    OUString sMsg = OUString::createFromAscii(pMsg);

    throw beans::IllegalTypeException( sMsg, *this );
}
// -----------------------------------------------------------------------------

void LayerWriter::checkInElement(bool bInElement, bool bInProperty)
{
    if (bInElement != m_aTagStack.empty())
    {
        sal_Char const * pMsg = bInElement ?
            "LayerWriter: Illegal Data: Operation requires a started node" :
            "LayerWriter: Illegal Data: There is a started node already" ;
        raiseMalformedDataException(pMsg);
    }

    if (bInProperty != m_bInProperty)
    {
        sal_Char const * pMsg = bInProperty ?
            "LayerWriter: Illegal Data: Operation requires a started property" :
            "LayerWriter: Illegal Data: There is a started property already" ;
        raiseMalformedDataException(pMsg);
    }
}
// -----------------------------------------------------------------------------

void LayerWriter::startNode()
{
    OUString sTag = m_aFormatter.getElementTag();
    getWriteHandler()->startElement(sTag,m_aFormatter.getElementAttributes());
    m_aTagStack.push(sTag);
}
// -----------------------------------------------------------------------------

void LayerWriter::startProp(uno::Type const & _aType, bool bNeedType)
{
    if (bNeedType && _aType == uno::Type())
        raiseIllegalTypeException("LayerWriter: Illegal Data: Cannot add VOID property");

    m_aFormatter.addPropertyValueType(_aType);

    startNode();

    m_aPropertyType = _aType;
    m_bInProperty = true;
}
// -----------------------------------------------------------------------------

void LayerWriter::endElement()
{
    OSL_ASSERT(!m_aTagStack.empty()); // checks done elsewhere

    getWriteHandler()->endElement(m_aTagStack.top());
    m_aTagStack.pop();
    m_bInProperty = false;
}
// -----------------------------------------------------------------------------

void LayerWriter::writeValue(uno::Any const & _aValue)
{
}
// -----------------------------------------------------------------------------

void LayerWriter::writeValue(uno::Any const & _aValue, OUString const & _aLocale)
{
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

