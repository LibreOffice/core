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

#include "submission_post.hxx"
#include "serialization_app_xml.hxx"
#include "serialization_urlencoded.hxx"

#include <osl/file.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/activedatasink.hxx>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>

using namespace css::uno;
using namespace css::ucb;
using namespace css::task;
using namespace css::io;
using namespace osl;
using namespace ucbhelper;
using namespace std;


CSubmissionPost::CSubmissionPost(const OUString& aURL, const css::uno::Reference< css::xml::dom::XDocumentFragment >& aFragment)
    : CSubmission(aURL, aFragment)
{
}

CSubmission::SubmissionResult CSubmissionPost::submit(const css::uno::Reference< css::task::XInteractionHandler >& aInteractionHandler)
{
    // PUT always uses application/xml
    css::uno::Reference< XCommandEnvironment > aEnvironment;
    unique_ptr< CSerialization > apSerialization(createSerialization(aInteractionHandler,aEnvironment));

    try {
        ucbhelper::Content aContent(m_aURLObj.GetMainURL(INetURLObject::NO_DECODE), aEnvironment, comphelper::getProcessComponentContext());

        // use post command
        OUString aCommandName("post");
        PostCommandArgument2 aPostArgument;
        aPostArgument.Source = apSerialization->getInputStream();
        css::uno::Reference< XActiveDataSink > aSink(new ucbhelper::ActiveDataSink);
        aPostArgument.Sink = aSink;
        aPostArgument.MediaType = "application/xml";
        aPostArgument.Referer.clear();
        Any aCommandArgument;
        aCommandArgument <<= aPostArgument;
        aContent.executeCommand( aCommandName, aCommandArgument);

        try {
            m_aResultStream = aSink->getInputStream();
        } catch (const Exception&) {
            OSL_FAIL("Cannot open reply stream from content");
        }
    } catch (const Exception&)
    {
        OSL_FAIL("Exception during UCB operatration.");
        return UNKNOWN_ERROR;
    }

    return SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
