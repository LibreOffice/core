/*************************************************************************
 *
 *  $RCSfile: submission_put.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 11:01:25 $
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

#include <memory>

#include "submission_put.hxx"
#include "serialization_app_xml.hxx"
#include "serialization_urlencoded.hxx"

#include <osl/file.hxx>
#include <unotools/processfactory.hxx>
#include <ucbhelper/content.hxx>

using namespace CSS::uno;
using namespace CSS::ucb;
using namespace CSS::task;
using namespace CSS::io;
using namespace rtl;
using namespace osl;
using namespace ucb;
using namespace std;


CSubmissionPut::CSubmissionPut(const rtl::OUString& aURL, const CSS::uno::Reference< CSS::xml::dom::XDocumentFragment >& aFragment)
    : CSubmission(aURL, aFragment)
    , m_aFactory(utl::getProcessServiceFactory())
{
}

CSubmission::SubmissionResult CSubmissionPut::submit()
{
    // PUT always uses application/xml
    auto_ptr< CSerialization > apSerialization(new CSerializationAppXML());
    apSerialization->setSource(m_aFragment);
    apSerialization->serialize();

    // create a commandEnvironment and use the default interaction handler
    CCommandEnvironmentHelper *pHelper = new CCommandEnvironmentHelper;
    pHelper->m_aInteractionHandler = Reference< XInteractionHandler >(m_aFactory->createInstance(
        OUString::createFromAscii("com.sun.star.task.InteractionHandler")), UNO_QUERY);
    OSL_ENSURE(pHelper->m_aInteractionHandler.is(), "failed to create IntreractionHandler");

    CProgressHandlerHelper *pProgressHelper = new CProgressHandlerHelper;
    pHelper->m_aProgressHandler = Reference< XProgressHandler >(pProgressHelper);

    // UCB has ownership of environment...
    Reference< XCommandEnvironment > aEnvironment(pHelper);

    try {
        ucb::Content aContent(m_aURLObj.GetMainURL(INetURLObject::NO_DECODE), aEnvironment);

        // insert serialized data to content -> PUT
        Reference< XInputStream > aInStream = apSerialization->getInputStream();
        aContent.writeStream(aInStream, sal_True);
        //aContent.closeStream();
    } catch (Exception& e)
    {
        // XXX
        OSL_ENSURE(sal_False, "Exception during UCB operatration.");
        return UNKNOWN_ERROR;
    }


    return SUCCESS;
}

