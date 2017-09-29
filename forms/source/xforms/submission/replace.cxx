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

#include <osl/diagnose.h>
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
#include <cppuhelper/logging.hxx>
#include <ucbhelper/content.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::task;
using namespace com::sun::star::xml::dom;

CSubmission::SubmissionResult CSubmission::replace(const OUString& aReplace, const Reference<XDocument>& aDocument, const Reference<XFrame>& aFrame)
{
    if (!m_aResultStream.is())
        return CSubmission::UNKNOWN_ERROR;

    try {
        Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
        if (aReplace.equalsIgnoreAsciiCase("all")
         || aReplace.equalsIgnoreAsciiCase("document")) {
            Reference< XComponentLoader > xLoader;
            if (aFrame.is())
                xLoader.set(aFrame, UNO_QUERY);

            if (!xLoader.is())
                xLoader.set( Desktop::create(xContext), UNO_QUERY_THROW);

            // open the stream from the result...
            // build media descriptor
            Sequence< PropertyValue > descriptor(2);
            descriptor[0] = PropertyValue("InputStream",
                -1, makeAny(m_aResultStream), PropertyState_DIRECT_VALUE);
            descriptor[1] = PropertyValue("ReadOnly",
                -1, makeAny(true), PropertyState_DIRECT_VALUE);

            OUString aURL = m_aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
            OUString aTarget = "_default";
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

                    Reference< XNode > aImportedNode = aDocument->importNode(newRoot, true);
                    aDocument->replaceChild(aImportedNode, oldRoot);
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
        SAL_WARN( "forms.xforms", "Exception during replace: " << e);
    }
    return CSubmission::UNKNOWN_ERROR;
}

::std::unique_ptr< CSerialization > CSubmission::createSerialization(const Reference< XInteractionHandler >& _xHandler,Reference<XCommandEnvironment>& _rOutEnv)
{
    // PUT always uses application/xml
    ::std::unique_ptr< CSerialization > apSerialization(new CSerializationAppXML());
    apSerialization->setSource(m_aFragment);
    apSerialization->serialize();

    // create a commandEnvironment and use the default interaction handler
    CCommandEnvironmentHelper *pHelper = new CCommandEnvironmentHelper;
    if( _xHandler.is() )
        pHelper->m_aInteractionHandler = _xHandler;
    else
        pHelper->m_aInteractionHandler.set(
            InteractionHandler::createWithParent(m_xContext, nullptr), UNO_QUERY_THROW);

    CProgressHandlerHelper *pProgressHelper = new CProgressHandlerHelper;
    pHelper->m_aProgressHandler.set(pProgressHelper);

    // UCB has ownership of environment...
    _rOutEnv = pHelper;
    return apSerialization;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
