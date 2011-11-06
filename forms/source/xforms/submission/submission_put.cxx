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

#include "submission_put.hxx"
#include "serialization_app_xml.hxx"
#include "serialization_urlencoded.hxx"

#include <osl/file.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/processfactory.hxx>
#include <ucbhelper/content.hxx>

using namespace CSS::uno;
using namespace CSS::ucb;
using namespace CSS::task;
using namespace CSS::io;
using namespace rtl;
using namespace osl;
using namespace ucbhelper;
using namespace std;


CSubmissionPut::CSubmissionPut(const rtl::OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
    : CSubmission(aURL, aFragment)
{
}

CSubmission::SubmissionResult CSubmissionPut::submit(const CSS::uno::Reference< CSS::task::XInteractionHandler >& aInteractionHandler)
{
    CSS::uno::Reference< XCommandEnvironment > aEnvironment;
    auto_ptr< CSerialization > apSerialization(createSerialization(aInteractionHandler,aEnvironment));

    try
    {
        ucbhelper::Content aContent(m_aURLObj.GetMainURL(INetURLObject::NO_DECODE), aEnvironment);

        // insert serialized data to content -> PUT
        CSS::uno::Reference< XInputStream > aInStream = apSerialization->getInputStream();
        aContent.writeStream(aInStream, sal_True);
        //aContent.closeStream();

        // no content as a result of put...

    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        return UNKNOWN_ERROR;
    }


    return SUCCESS;
}

