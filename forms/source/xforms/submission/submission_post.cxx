/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: submission_post.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include <memory>

#include "submission_post.hxx"
#include "serialization_app_xml.hxx"
#include "serialization_urlencoded.hxx"

#include <osl/file.hxx>
#include <unotools/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/activedatasink.hxx>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>

using namespace CSS::uno;
using namespace CSS::ucb;
using namespace CSS::task;
using namespace CSS::io;
using namespace rtl;
using namespace osl;
using namespace ucbhelper;
using namespace std;


CSubmissionPost::CSubmissionPost(const rtl::OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
    : CSubmission(aURL, aFragment)
    , m_aFactory(utl::getProcessServiceFactory())
{
}

CSubmission::SubmissionResult CSubmissionPost::submit(const CSS::uno::Reference< CSS::task::XInteractionHandler >& aInteractionHandler)
{
    // PUT always uses application/xml
    auto_ptr< CSerialization > apSerialization(new CSerializationAppXML());
    apSerialization->setSource(m_aFragment);
    apSerialization->serialize();

    // create a commandEnvironment and use the default interaction handler
    CCommandEnvironmentHelper *pHelper = new CCommandEnvironmentHelper;
    if( aInteractionHandler.is() )
        pHelper->m_aInteractionHandler = aInteractionHandler;
    else
        pHelper->m_aInteractionHandler = CSS::uno::Reference< XInteractionHandler >(m_aFactory->createInstance(
            OUString::createFromAscii("com.sun.star.task.InteractionHandler")), UNO_QUERY);
    OSL_ENSURE(pHelper->m_aInteractionHandler.is(), "failed to create IntreractionHandler");
    CProgressHandlerHelper *pProgressHelper = new CProgressHandlerHelper;
    pHelper->m_aProgressHandler = CSS::uno::Reference< XProgressHandler >(pProgressHelper);
    // UCB has ownership of environment...
    CSS::uno::Reference< XCommandEnvironment > aEnvironment(pHelper);

    try {
        ucbhelper::Content aContent(m_aURLObj.GetMainURL(INetURLObject::NO_DECODE), aEnvironment);

        // use post command

        OUString aCommandName = OUString::createFromAscii("post");
        PostCommandArgument2 aPostArgument;
        aPostArgument.Source = apSerialization->getInputStream();
        //CSS::uno::Reference< XInterface > aSink( m_aFactory->createInstance(
        //    OUString::createFromAscii("com.sun.star.io.Pipe")), UNO_QUERY_THROW);
        CSS::uno::Reference< XActiveDataSink > aSink(new ucbhelper::ActiveDataSink);
        //    OUString::createFromAscii("com.sun.star.io.Pipe")), UNO_QUERY_THROW);
        aPostArgument.Sink = aSink;
        aPostArgument.MediaType = OUString::createFromAscii("application/xml");
        aPostArgument.Referer = OUString();
        Any aCommandArgument;
        aCommandArgument <<= aPostArgument;
        aContent.executeCommand( aCommandName, aCommandArgument);

        // wait for command to finish
        // pProgressHelper->m_cFinished.wait();

        // CSS::uno::Reference< XOutputStream > xOut(aSink, UNO_QUERY_THROW);
        // xOut->closeOutput();

        try {
            // m_aResultStream = CSS::uno::Reference< XInputStream >(aSink, UNO_QUERY_THROW);
            m_aResultStream = aSink->getInputStream();
        } catch (Exception&) {
            OSL_ENSURE(sal_False, "Cannot open reply stream from content");
        }
    } catch (Exception&)
    {
        // XXX
        OSL_ENSURE(sal_False, "Exception during UCB operatration.");
        return UNKNOWN_ERROR;
    }


    return SUCCESS;
}

