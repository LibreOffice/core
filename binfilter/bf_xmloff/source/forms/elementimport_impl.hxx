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

// no include protection. This file is included from elementimport.hxx only.

#ifndef _INCLUDING_FROM_ELEMENTIMPORT_HXX_
#error "do not include this file directly!"
#endif

// no namespace. Same as above: this file is included from elementimport.hxx only,
// and this is done inside the namespace

//=========================================================================
//= OContainerImport
//=========================================================================
//-------------------------------------------------------------------------
template <class BASE>
SvXMLImportContext* OContainerImport< BASE >::CreateChildContext(
    sal_uInt16 _nPrefix, const ::rtl::OUString& _rLocalName,
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList)
{
    // maybe it's a sub control
    if (_rLocalName == m_sWrapperElementName)
        if (m_xMeAsContainer.is())
            return implCreateControlWrapper(_nPrefix, _rLocalName);
        else
        {
            OSL_ENSURE(sal_False, "OContainerImport::CreateChildContext: don't have an element!");
            return NULL;
        }

    return BASE::CreateChildContext(_nPrefix, _rLocalName, _rxAttrList);
}

//-------------------------------------------------------------------------
template <class BASE>
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
    OContainerImport< BASE >::createElement()
{
    // let the base class create the object
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xReturn = BASE::createElement();
    if (!xReturn.is())
        return xReturn;

    // ensure that the object is a XNameContainer (we strongly need this for inserting child elements)
    m_xMeAsContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >(xReturn, ::com::sun::star::uno::UNO_QUERY);
    if (!m_xMeAsContainer.is())
    {
        OSL_ENSURE(sal_False, "OContainerImport::createElement: invalid element (no XNameContainer) created!");
        xReturn.clear();
    }

    return xReturn;
}

//-------------------------------------------------------------------------
template <class BASE>
void OContainerImport< BASE >::EndElement()
{
    BASE::EndElement();

    // now that we have all children, attach the events
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > xIndexContainer(m_xMeAsContainer, ::com::sun::star::uno::UNO_QUERY);
    if (xIndexContainer.is())
        ODefaultEventAttacherManager::setEvents(xIndexContainer);
}

//=========================================================================
//= OColumnImport
//=========================================================================
//-------------------------------------------------------------------------
template <class BASE>
OColumnImport< BASE >::OColumnImport(IFormsImportContext& _rImport, IEventAttacherManager& _rEventManager, sal_uInt16 _nPrefix, const ::rtl::OUString& _rName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& _rxParentContainer,
        OControlElement::ElementType _eType,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxOuterAttribs)
    :BASE(_rImport, _rEventManager, _nPrefix, _rName, _rxParentContainer, _eType)
    ,m_xColumnFactory(_rxParentContainer, ::com::sun::star::uno::UNO_QUERY)
{
    OSL_ENSURE(m_xColumnFactory.is(), "OColumnImport::OColumnImport: invalid parent container (no factory)!");
}

//-------------------------------------------------------------------------
// OElementImport overridables
template <class BASE>
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > OColumnImport< BASE >::createElement()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xReturn;
    // no call to the base class' method. We have to use the grid column factory
    if (m_xColumnFactory.is())
    {
        // create the column
        xReturn = m_xColumnFactory->createColumn(this->m_sServiceName);
        OSL_ENSURE(xReturn.is(), "OColumnImport::createElement: the factory returned an invalid object!");
    }
    return xReturn;
}

