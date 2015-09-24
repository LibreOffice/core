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



#ifndef _COMPHELPER_STILLREADWRITEINTERACTION_HXX_
#define _COMPHELPER_STILLREADWRITEINTERACTION_HXX_

//_______________________________________________
// includes
#include <ucbhelper/interceptedinteraction.hxx>

#ifndef __COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP__
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#include <ucbhelper/interceptedinteraction.hxx>
#include "comphelper/comphelperdllapi.h"

//_______________________________________________
// namespace

namespace comphelper{
class COMPHELPER_DLLPUBLIC StillReadWriteInteraction : public ::ucbhelper::InterceptedInteraction
{
private:
    static const sal_Int32 HANDLE_INTERACTIVEIOEXCEPTION       = 0;
    static const sal_Int32 HANDLE_UNSUPPORTEDDATASINKEXCEPTION = 1;
    static const sal_Int32 HANDLE_AUTHENTICATIONREQUESTEXCEPTION = 2;

    sal_Bool m_bUsed;
    sal_Bool m_bHandledByMySelf;
    sal_Bool m_bHandledByInternalHandler;

public:
    StillReadWriteInteraction(const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xHandler,
                              const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xAuthenticationHandler);

    void resetInterceptions();
    void resetErrorStates();
    sal_Bool wasWriteError();

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > m_xAuthenticationHandler;

private:
    virtual ucbhelper::InterceptedInteraction::EInterceptionState intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest&                         aRequest,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest);

};
}
#endif
