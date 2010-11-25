/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_desktop.hxx"
#include "cfgfilter.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <unotools/textsearch.hxx>

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration::backend;

namespace desktop {

CConfigFilter::CConfigFilter(const strings_v* include,  const strings_v* exclude)
    : m_pvInclude(include)
    , m_pvExclude(exclude)
{
}

void SAL_CALL CConfigFilter::initialize(const Sequence< Any >& seqArgs)
        throw (Exception)
{
    NamedValue nv;
    for (sal_Int32 i=0; i < seqArgs.getLength(); ++i)
    {
        if (seqArgs[i] >>= nv)
        {
            if (nv.Name.equalsAscii("Source"))
                nv.Value >>= m_xSourceLayer;
            if (nv.Name.equalsAscii("ComponentName"))
                nv.Value >>= m_aCurrentComponent;
        }
    }
    if (m_aCurrentComponent.getLength() == 0)
        m_aCurrentComponent = OUString(RTL_CONSTASCII_USTRINGPARAM("unknown.component"));

    if (!m_xSourceLayer.is()) {
        throw Exception();
    }

}


void CConfigFilter::pushElement(rtl::OUString aName, sal_Bool bUse)
{
    OUString aPath;
    if (!m_elementStack.empty()) {
        aPath = m_elementStack.top().path; // or use base path
        aPath += OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    }
    aPath += aName;

    // create element
    element elem;
    elem.name = aName;
    elem.path = aPath;
    elem.use = bUse;
    m_elementStack.push(elem);
}

sal_Bool CConfigFilter::checkCurrentElement()
{
    return m_elementStack.top().use;
}

sal_Bool CConfigFilter::checkElement(rtl::OUString aName)
{

    sal_Bool bResult = sal_False;

    // get full pathname for element
    OUString aFullPath;
    if (!m_elementStack.empty())
        aFullPath = m_elementStack.top().path + OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

    aFullPath += aName;

    // check whether any include patterns patch this path
    for (strings_v::const_iterator i_in = m_pvInclude->begin();
        i_in != m_pvInclude->end(); ++i_in)
    {
        // pattern is beginning of path
        // or path is a begiing for pattern
        if (i_in->match(aFullPath.copy(0, i_in->getLength()>aFullPath.getLength()
            ? aFullPath.getLength() : i_in->getLength()), 0))
        {
            bResult = sal_True;
            break; // one match is enough
        }
    }
    // if match is found, check for exclusion
    if (bResult)
    {
        for (strings_v::const_iterator i_ex = m_pvExclude->begin();
            i_ex != m_pvExclude->end(); ++i_ex)
        {
            if (aFullPath.match(*i_ex, 0)) // pattern is beginning of path
            {
                bResult = sal_False;
                break; // one is enough...
            }
        }
    }
    return bResult;
}

void CConfigFilter::popElement()
{
    m_elementStack.pop();
}


void SAL_CALL CConfigFilter::readData(
        const Reference< configuration::backend::XLayerHandler >& layerHandler)
    throw (
        com::sun::star::lang::NullPointerException, lang::WrappedTargetException,
        com::sun::star::configuration::backend::MalformedDataException)
{
    // when readData is called, the submitted handler will be stored
    // in m_xLayerHandler. we will then submit ourself as a handler to
    // the SourceLayer in m_xSourceLayer.
    // when the source calls our handler functions we will use the patterns that
    // where given in the ctor to decide whther they should be relaied to the caller

    if (m_xSourceLayer.is() && layerHandler.is())
    {
        m_xLayerHandler = layerHandler;
        m_xSourceLayer->readData(Reference<XLayerHandler>(static_cast< XLayerHandler* >(this)));
    } else
    {
        throw NullPointerException();
    }
}

// XLayerHandler
void SAL_CALL CConfigFilter::startLayer()
    throw(::com::sun::star::lang::WrappedTargetException)
{
    m_xLayerHandler->startLayer();
}

void SAL_CALL CConfigFilter::endLayer()
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    m_xLayerHandler->endLayer();
}

void SAL_CALL CConfigFilter::overrideNode(
        const OUString& aName,
        sal_Int16 aAttributes,
        sal_Bool bClear)
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
    {
        m_xLayerHandler->overrideNode(aName, aAttributes, bClear);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL CConfigFilter::addOrReplaceNode(
        const OUString& aName,
        sal_Int16 aAttributes)
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
    {
        m_xLayerHandler->addOrReplaceNode(aName, aAttributes);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL  CConfigFilter::addOrReplaceNodeFromTemplate(
        const OUString& aName,
        const com::sun::star::configuration::backend::TemplateIdentifier& aTemplate,
        sal_Int16 aAttributes )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
    {
        m_xLayerHandler->addOrReplaceNodeFromTemplate(aName, aTemplate, aAttributes);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL  CConfigFilter::endNode()
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
    {
        m_xLayerHandler->endNode();
    }
    popElement();
}

void SAL_CALL  CConfigFilter::dropNode(
        const OUString& aName )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    // does not get pushed
    if (checkElement(aName))
    {
        m_xLayerHandler->dropNode(aName);
    }
}

void SAL_CALL  CConfigFilter::overrideProperty(
        const OUString& aName,
        sal_Int16 aAttributes,
        const Type& aType,
        sal_Bool bClear )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName)){
        m_xLayerHandler->overrideProperty(aName, aAttributes, aType, bClear);
        pushElement(aName);
    }
    else
        pushElement(aName, sal_False);
}

void SAL_CALL  CConfigFilter::setPropertyValue(
        const Any& aValue )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
        m_xLayerHandler->setPropertyValue(aValue);
}

void SAL_CALL CConfigFilter::setPropertyValueForLocale(
        const Any& aValue,
        const OUString& aLocale )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
        m_xLayerHandler->setPropertyValueForLocale(aValue, aLocale);
}

void SAL_CALL  CConfigFilter::endProperty()
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkCurrentElement())
    {
        m_xLayerHandler->endProperty();
    }
    popElement();

}

void SAL_CALL  CConfigFilter::addProperty(
        const rtl::OUString& aName,
        sal_Int16 aAttributes,
        const Type& aType )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    if (checkElement(aName))
        m_xLayerHandler->addProperty(aName, aAttributes, aType);
}

void SAL_CALL  CConfigFilter::addPropertyWithValue(
        const rtl::OUString& aName,
        sal_Int16 aAttributes,
        const Any& aValue )
    throw(
        ::com::sun::star::configuration::backend::MalformedDataException,
        ::com::sun::star::lang::WrappedTargetException )
{
    // add property with value doesn't push the property
    if (checkElement(aName))
        m_xLayerHandler->addPropertyWithValue(aName, aAttributes, aValue);

}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
