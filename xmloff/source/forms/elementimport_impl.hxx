/*************************************************************************
 *
 *  $RCSfile: elementimport_impl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:04:34 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4.334.1  2004/07/13 20:28:08  fa
 *  #i31445# explicitly scope data member to satisfy gcc 3.4
 *
 *  Revision 1.4  2001/01/03 16:25:34  fs
 *  file format change (extra wrapper element for controls, similar to columns)
 *
 *  Revision 1.3  2001/01/02 15:58:21  fs
 *  event ex- & import
 *
 *  Revision 1.2  2000/12/13 10:40:15  fs
 *  new import related implementations - at this version, we should be able to import everything we export (which is all except events and styles)
 *
 *  Revision 1.1  2000/12/12 12:02:53  fs
 *  initial checkin - implementations for the template classes in elementimport
 *
 *
 *  Revision 1.0 12.12.00 08:33:20  fs
 ************************************************************************/

