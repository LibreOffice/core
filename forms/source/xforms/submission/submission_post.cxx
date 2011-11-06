/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
{
}

CSubmission::SubmissionResult CSubmissionPost::submit(const CSS::uno::Reference< CSS::task::XInteractionHandler >& aInteractionHandler)
{
    // PUT always uses application/xml
    CSS::uno::Reference< XCommandEnvironment > aEnvironment;
    auto_ptr< CSerialization > apSerialization(createSerialization(aInteractionHandler,aEnvironment));

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

