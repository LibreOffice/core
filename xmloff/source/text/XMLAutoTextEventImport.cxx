/*************************************************************************
 *
 *  $RCSfile: XMLAutoTextEventImport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2001-02-06 11:51:27 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLAUTOTEXTEVENTIMPORT_HXX
#include "XMLAutoTextEventImport.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTCONTAINER_HPP_
#include <com/sun/star/text/XAutoTextContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTGROUP_HPP_
#include <com/sun/star/text/XAutoTextGroup.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTGROUP_HPP_
#include <com/sun/star/text/XAutoTextGroup.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _XMLOFF_XMLAUTOTEXTGROUPEVENTIMPORT_HXX
#include "XMLAutoTextGroupEventImport.hxx"
#endif

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::comphelper::getProcessServiceFactory;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::text::XAutoTextContainer;
using ::com::sun::star::lang::XMultiServiceFactory;

const sal_Char sAPI_AutoText[] = "com.sun.star.text.AutoTextContainer";


XMLAutoTextEventImport::XMLAutoTextEventImport() throw() :
    SvXMLImport()
{
    // instantiate AutoTextContainer via factory
    Reference<XMultiServiceFactory> xFactory = getProcessServiceFactory();
    if (xFactory.is())
    {
        OUString sService(RTL_CONSTASCII_USTRINGPARAM(sAPI_AutoText));
        Reference<XAutoTextContainer> xContainer(
            xFactory->createInstance(sService), uno::UNO_QUERY);

        xAutoTextContainer = xContainer;
    }

}

XMLAutoTextEventImport::~XMLAutoTextEventImport()
{
}


SvXMLImportContext* XMLAutoTextEventImport::CreateContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList > & xAttrList )
{
    if (xAutoTextContainer.is())
    {
        return new XMLAutoTextGroupEventImport(*this, nPrefix, rLocalName,
                                               xAutoTextContainer);
    }
    else
    {
        return SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
}



Sequence< OUString > SAL_CALL
    XMLAutoTextEventImport_getSupportedServiceNames()
        throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.office.sax.importer.AutoTextEventReader" ) );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL XMLAutoTextEventImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLAutoTextEventImport" ) );
}

Reference< XInterface > SAL_CALL XMLAutoTextEventImport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new XMLAutoTextEventImport;
}

