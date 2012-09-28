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

#include <memory>
#include "submission.hxx"
#include "serialization_app_xml.hxx"

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <ucbhelper/content.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::task;
using namespace com::sun::star::xml::dom;

CSubmission::SubmissionResult CSubmission::replace(const ::rtl::OUString& aReplace, const Reference<XDocument>& aDocument, const Reference<XFrame>& aFrame)
{
    if (!m_aResultStream.is())
        return CSubmission::UNKNOWN_ERROR;

    try {
        Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
        Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
        if (aReplace.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("all"))
         || aReplace.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("document"))) {
            Reference< XComponentLoader > xLoader;
            if (aFrame.is())
                xLoader = Reference< XComponentLoader >(aFrame, UNO_QUERY);

            if (!xLoader.is())
                xLoader = Reference< XComponentLoader >(xFactory->createInstance(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop") ) ), UNO_QUERY_THROW);

            // open the stream from the result...
            // build media descriptor
            Sequence< PropertyValue > descriptor(2);
            descriptor[0] = PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream") ),
                -1, makeAny(m_aResultStream), PropertyState_DIRECT_VALUE);
            descriptor[1] = PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                -1, makeAny(sal_True), PropertyState_DIRECT_VALUE);

            ::rtl::OUString aURL = m_aURLObj.GetMainURL(INetURLObject::NO_DECODE);
            ::rtl::OUString aTarget = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_default") );
            xLoader->loadComponentFromURL(aURL, aTarget, FrameSearchFlag::ALL, descriptor);

            return CSubmission::SUCCESS;

        } else if (aReplace.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("instance"))) {
            if (aDocument.is()) {
                // parse the result stream into a new document
                Reference< XDocumentBuilder > xBuilder(DocumentBuilder::create(xContext));
                Reference< XDocument > aNewDocument = xBuilder->parse(m_aResultStream);

                if (aNewDocument.is()) {
                    // and replace the content of the current instance
                    Reference< XElement > oldRoot = aDocument->getDocumentElement();
                    Reference< XElement > newRoot = aNewDocument->getDocumentElement();

                    Reference< XNode > aImportedNode = aDocument->importNode(Reference< XNode >(newRoot, UNO_QUERY_THROW), sal_True);
                    Reference< XNode >(aDocument, UNO_QUERY_THROW)->replaceChild(aImportedNode, Reference< XNode >(oldRoot, UNO_QUERY_THROW));
                    return CSubmission::SUCCESS;
                } else {
                    return CSubmission::UNKNOWN_ERROR;
                }
            } else {
                // nothing to replace
                return CSubmission::UNKNOWN_ERROR;
            }
        } else if (aReplace.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("none"))) {
            // do nothing \o/
            return CSubmission::SUCCESS;
        }
    } catch (const Exception& e) {
        ::rtl::OString aMsg("Exception during replace:\n");
        aMsg += OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8);
        OSL_FAIL(aMsg.getStr());
    }
    return CSubmission::UNKNOWN_ERROR;
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr< CSerialization > CSubmission::createSerialization(const Reference< XInteractionHandler >& _xHandler,Reference<XCommandEnvironment>& _rOutEnv)
{
    // PUT always uses application/xml
    ::std::auto_ptr< CSerialization > apSerialization(new CSerializationAppXML());
    apSerialization->setSource(m_aFragment);
    apSerialization->serialize();

    // create a commandEnvironment and use the default interaction handler
    CCommandEnvironmentHelper *pHelper = new CCommandEnvironmentHelper;
    if( _xHandler.is() )
        pHelper->m_aInteractionHandler = _xHandler;
    else
        pHelper->m_aInteractionHandler = CSS::uno::Reference< XInteractionHandler >(m_aFactory->createInstance(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler") ) ), UNO_QUERY);
    OSL_ENSURE(pHelper->m_aInteractionHandler.is(), "failed to create IntreractionHandler");

    CProgressHandlerHelper *pProgressHelper = new CProgressHandlerHelper;
    pHelper->m_aProgressHandler = Reference< XProgressHandler >(pProgressHelper);

    // UCB has ownership of environment...
    _rOutEnv = pHelper;
    return apSerialization;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
