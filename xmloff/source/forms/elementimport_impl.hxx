/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

// no include protection. This file is included from elementimport.hxx only.

#ifndef _INCLUDING_FROM_ELEMENTIMPORT_HXX_
#error "do not include this file directly!"
#endif

#include <osl/diagnose.h>

// no namespace. Same as above: this file is included from elementimport.hxx only,
// and this is done inside the namespace

//= OContainerImport
template <class BASE>
inline SvXMLImportContext* OContainerImport< BASE >::CreateChildContext(
    sal_uInt16 _nPrefix, const OUString& _rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList)
{
    // maybe it's a sub control
    if (_rLocalName == m_sWrapperElementName)
    {
        if (m_xMeAsContainer.is())
            return implCreateControlWrapper(_nPrefix, _rLocalName);
        else
        {
            OSL_FAIL("OContainerImport::CreateChildContext: don't have an element!");
            return nullptr;
        }
    }

    return BASE::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
}

template <class BASE>
inline css::uno::Reference< css::beans::XPropertySet >
    OContainerImport< BASE >::createElement()
{
    // let the base class create the object
    css::uno::Reference< css::beans::XPropertySet > xReturn = BASE::createElement();
    if (!xReturn.is())
        return xReturn;

    // ensure that the object is a XNameContainer (we strongly need this for inserting child elements)
    m_xMeAsContainer.set(xReturn, css::uno::UNO_QUERY);
    if (!m_xMeAsContainer.is())
    {
        OSL_FAIL("OContainerImport::createElement: invalid element (no XNameContainer) created!");
        xReturn.clear();
    }

    return xReturn;
}

template <class BASE>
inline void OContainerImport< BASE >::EndElement()
{
    BASE::EndElement();

    // now that we have all children, attach the events
    css::uno::Reference< css::container::XIndexAccess > xIndexContainer(m_xMeAsContainer, css::uno::UNO_QUERY);
    if (xIndexContainer.is())
        ODefaultEventAttacherManager::setEvents(xIndexContainer);
}

//= OColumnImport
template <class BASE>
OColumnImport< BASE >::OColumnImport(OFormLayerXMLImport_Impl& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const OUString& _rName,
        const css::uno::Reference< css::container::XNameContainer >& _rxParentContainer,
        OControlElement::ElementType _eType)
    :BASE(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    ,m_xColumnFactory(_rxParentContainer, css::uno::UNO_QUERY)
{
    OSL_ENSURE(m_xColumnFactory.is(), "OColumnImport::OColumnImport: invalid parent container (no factory)!");
}

// OElementImport overridables
template <class BASE>
css::uno::Reference< css::beans::XPropertySet > OColumnImport< BASE >::createElement()
{
    css::uno::Reference< css::beans::XPropertySet > xReturn;
    // no call to the base class' method. We have to use the grid column factory
    if (m_xColumnFactory.is())
    {
        // create the column
        xReturn = m_xColumnFactory->createColumn(this->m_sServiceName);
        OSL_ENSURE(xReturn.is(), "OColumnImport::createElement: the factory returned an invalid object!");
    }
    return xReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
