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



#include "com/sun/star/ucb/HandleCookiesRequest.hpp"
#include "com/sun/star/ucb/XInteractionCookieHandling.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"

#include "vos/mutex.hxx"
#include "tools/list.hxx"
#include "svl/httpcook.hxx"
#include "vcl/svapp.hxx"

#include "cookiedg.hxx"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

class CookieList: public List
{
public:
    ~CookieList() SAL_THROW(());
};

CookieList::~CookieList() SAL_THROW(())
{
    while (Count() != 0)
        delete static_cast< CntHTTPCookie * >(Remove(Count() - 1));
}

void
executeCookieDialog(Window * pParent, CntHTTPCookieRequest & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    try
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        std::auto_ptr< ResMgr > xManager(
            ResMgr::CreateResMgr(CREATEVERSIONRESMGR_NAME(uui)));
        std::auto_ptr< CookiesDialog > xDialog(
            new CookiesDialog(pParent, &rRequest, xManager.get()));
        xDialog->Execute();
    }
    catch (std::bad_alloc const &)
    {
        throw uno::RuntimeException(
                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("out of memory")),
                  uno::Reference< uno::XInterface>());
    }
}

void
handleCookiesRequest_(
    Window * pParent,
    ucb::HandleCookiesRequest const & rRequest,
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > const &
        rContinuations)
    SAL_THROW((uno::RuntimeException))
{
    CookieList aCookies;
    for (sal_Int32 i = 0; i < rRequest.Cookies.getLength(); ++i)
    {
        try
        {
            std::auto_ptr< CntHTTPCookie > xCookie(new CntHTTPCookie);
            xCookie->m_aName = UniString(rRequest.Cookies[i].Name);
            xCookie->m_aValue = UniString(rRequest.Cookies[i].Value);
            xCookie->m_aDomain = UniString(rRequest.Cookies[i].Domain);
            xCookie->m_aPath = UniString(rRequest.Cookies[i].Path);
            xCookie->m_aExpires
                = DateTime(Date(rRequest.Cookies[i].Expires.Day,
                                rRequest.Cookies[i].Expires.Month,
                                rRequest.Cookies[i].Expires.Year),
                           Time(rRequest.Cookies[i].Expires.Hours,
                                rRequest.Cookies[i].Expires.Minutes,
                                rRequest.Cookies[i].Expires.Seconds,
                                rRequest.Cookies[i].Expires.HundredthSeconds));
            xCookie->m_nFlags
                = rRequest.Cookies[i].Secure ? CNTHTTP_COOKIE_FLAG_SECURE : 0;
            switch (rRequest.Cookies[i].Policy)
            {
            case ucb::CookiePolicy_CONFIRM:
                xCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_INTERACTIVE;
                break;

            case ucb::CookiePolicy_ACCEPT:
                xCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_ACCEPTED;
                break;

            case ucb::CookiePolicy_IGNORE:
                xCookie->m_nPolicy = CNTHTTP_COOKIE_POLICY_BANNED;
                break;

            default:
                OSL_ASSERT(false);
                break;
            }
            aCookies.Insert(xCookie.get(), LIST_APPEND);
            xCookie.release();
        }
        catch (std::bad_alloc const &)
        {
            throw uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                  "out of memory")),
                uno::Reference< uno::XInterface >());
        }
    }

    CntHTTPCookieRequest
    aRequest(rRequest.URL,
                 aCookies,
                 rRequest.Request == ucb::CookieRequest_RECEIVE
                     ? CNTHTTP_COOKIE_REQUEST_RECV
                     : CNTHTTP_COOKIE_REQUEST_SEND);
    executeCookieDialog(pParent, aRequest);
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        uno::Reference< ucb::XInteractionCookieHandling >
            xCookieHandling(rContinuations[i], uno::UNO_QUERY);
        if (xCookieHandling.is())
        {
            switch (aRequest.m_nRet)
            {
            case CNTHTTP_COOKIE_POLICY_INTERACTIVE:
                xCookieHandling->
                    setGeneralPolicy(ucb::CookiePolicy_CONFIRM);
                break;

            case CNTHTTP_COOKIE_POLICY_ACCEPTED:
                xCookieHandling->
                    setGeneralPolicy(ucb::CookiePolicy_ACCEPT);
                break;

            case CNTHTTP_COOKIE_POLICY_BANNED:
                xCookieHandling->
                    setGeneralPolicy(ucb::CookiePolicy_IGNORE);
                break;
            }
            for (sal_Int32 j = 0; j < rRequest.Cookies.getLength(); ++j)
                if (rRequest.Cookies[j].Policy
                    == ucb::CookiePolicy_CONFIRM)
                    switch (static_cast< CntHTTPCookie * >(aCookies.
                                                           GetObject(j))->
                            m_nPolicy)
                    {
                    case CNTHTTP_COOKIE_POLICY_ACCEPTED:
                        xCookieHandling->
                            setSpecificPolicy(rRequest.Cookies[j], true);
                        break;

                    case CNTHTTP_COOKIE_POLICY_BANNED:
                        xCookieHandling->
                            setSpecificPolicy(rRequest.Cookies[j], false);
                        break;
                    }
            xCookieHandling->select();
            break;
        }
    }
}

} // namespace

bool
UUIInteractionHelper::handleCookiesRequest(
    uno::Reference< task::XInteractionRequest > const & rRequest)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());

    ucb::HandleCookiesRequest aCookiesRequest;
    if (aAnyRequest >>= aCookiesRequest)
    {
        handleCookiesRequest_(getParentProperty(),
                              aCookiesRequest,
                              rRequest->getContinuations());
        return true;
    }
    return false;
}

