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

#include "submission_get.hxx"
#include "serialization_app_xml.hxx"
#include "serialization_urlencoded.hxx"

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <osl/file.hxx>
#include <comphelper/componentcontext.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/io/Pipe.hpp>

using namespace CSS::uno;
using namespace CSS::ucb;
using namespace CSS::task;
using namespace CSS::io;
using namespace osl;
using namespace ucbhelper;
using namespace std;

using ::rtl::OUString;
using ::rtl::OStringToOUString;
using ::rtl::OStringBuffer;


CSubmissionGet::CSubmissionGet(const rtl::OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
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
        pHelper->m_aInteractionHandler = CSS::uno::Reference< XInteractionHandler >(m_aFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.InteractionHandler"))), UNO_QUERY);
    OSL_ENSURE(pHelper->m_aInteractionHandler.is(), "failed to create IntreractionHandler");
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
        ucbhelper::Content aContent(aQueryURL, aEnvironment, comphelper::ComponentContext(m_aFactory).getUNOContext());
        CSS::uno::Reference< XOutputStream > aPipe( CSS::io::Pipe::create(comphelper::ComponentContext(m_aFactory).getUNOContext()), UNO_QUERY_THROW );
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
