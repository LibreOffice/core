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
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
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
        Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
        if (aReplace.equalsIgnoreAsciiCase("all")
         || aReplace.equalsIgnoreAsciiCase("document")) {
            Reference< XComponentLoader > xLoader;
            if (aFrame.is())
                xLoader = Reference< XComponentLoader >(aFrame, UNO_QUERY);

            if (!xLoader.is())
                xLoader = Reference< XComponentLoader >( Desktop::create(xContext), UNO_QUERY_THROW);

            // open the stream from the result...
            // build media descriptor
            Sequence< PropertyValue > descriptor(2);
            descriptor[0] = PropertyValue(::rtl::OUString("InputStream"),
                -1, makeAny(m_aResultStream), PropertyState_DIRECT_VALUE);
            descriptor[1] = PropertyValue(::rtl::OUString("ReadOnly"),
                -1, makeAny(sal_True), PropertyState_DIRECT_VALUE);

            ::rtl::OUString aURL = m_aURLObj.GetMainURL(INetURLObject::NO_DECODE);
            ::rtl::OUString aTarget = ::rtl::OUString("_default");
            xLoader->loadComponentFromURL(aURL, aTarget, FrameSearchFlag::ALL, descriptor);

            return CSubmission::SUCCESS;

        } else if (aReplace.equalsIgnoreAsciiCase("instance")) {
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
        } else if (aReplace.equalsIgnoreAsciiCase("none")) {
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
        pHelper->m_aInteractionHandler.set(
            InteractionHandler::createWithParent(m_xContext, 0), UNO_QUERY_THROW);

    CProgressHandlerHelper *pProgressHelper = new CProgressHandlerHelper;
    pHelper->m_aProgressHandler = Reference< XProgressHandler >(pProgressHelper);

    // UCB has ownership of environment...
    _rOutEnv = pHelper;
    return apSerialization;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
