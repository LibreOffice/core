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

#include "submission_get.hxx"
#include "serialization_app_xml.hxx"
#include "serialization_urlencoded.hxx"

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>

using namespace CSS::uno;
using namespace CSS::ucb;
using namespace CSS::task;
using namespace CSS::io;
using namespace osl;
using namespace ucbhelper;
using namespace std;



CSubmissionGet::CSubmissionGet(const OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
    : CSubmission(aURL, aFragment)
{
}

CSubmission::SubmissionResult CSubmissionGet::submit(const CSS::uno::Reference< CSS::task::XInteractionHandler >& aInteractionHandler)
{
    // GET always uses apllicatin/x-www-formurlencoded
    auto_ptr< CSerialization > apSerialization(new CSerializationURLEncoded());
    apSerialization->setSource(m_aFragment);
    apSerialization->serialize();

    CSS::uno::Reference< XInputStream > aInStream = apSerialization->getInputStream();

    // create a commandEnvironment and use the default interaction handler
    CCommandEnvironmentHelper *pHelper = new CCommandEnvironmentHelper;
    if( aInteractionHandler.is() )
        pHelper->m_aInteractionHandler = aInteractionHandler;
    else
        pHelper->m_aInteractionHandler.set(
            CSS::task::InteractionHandler::createWithParent(m_xContext, 0), UNO_QUERY_THROW);
    CProgressHandlerHelper *pProgressHelper = new CProgressHandlerHelper;
    pHelper->m_aProgressHandler = CSS::uno::Reference< XProgressHandler >(pProgressHelper);

    // UCB has ownership of environment...
    CSS::uno::Reference< XCommandEnvironment > aEnvironment(pHelper);

    // append query string to the URL
    try {
        OStringBuffer aUTF8QueryURL(OUStringToOString(m_aURLObj.GetMainURL(INetURLObject::NO_DECODE),
            RTL_TEXTENCODING_UTF8));
        OStringBuffer aQueryString;

        const sal_Int32 size = 1024;
        sal_Int32 n = 0;
        Sequence< sal_Int8 > aByteBuffer(size);
        while ((n = aInStream->readSomeBytes(aByteBuffer, size-1)) != 0)
            aQueryString.append((sal_Char*)aByteBuffer.getArray(), n);
        if (aQueryString.getLength() > 0 && m_aURLObj.GetProtocol() != INET_PROT_FILE)
        {
            aUTF8QueryURL.append('?');
            aUTF8QueryURL.append(aQueryString.makeStringAndClear());
        }
        OUString aQueryURL = OStringToOUString(aUTF8QueryURL.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
        ucbhelper::Content aContent(aQueryURL, aEnvironment, m_xContext);
        CSS::uno::Reference< XOutputStream > aPipe( CSS::io::Pipe::create(m_xContext), UNO_QUERY_THROW );
        aContent.openStream(aPipe);
        // get reply
        try {
            m_aResultStream = aContent.openStream();
        } catch (const Exception&) {
            OSL_FAIL("Cannot open reply stream from content");
        }
    } catch (const Exception&)
    {
        // XXX
        OSL_FAIL("Exception during UCB operatration.");
        return UNKNOWN_ERROR;
    }

    return SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
