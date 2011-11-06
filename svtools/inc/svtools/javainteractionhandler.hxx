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



#ifndef _SVTOOLS_JAVAINTERACTION_HXX_
#define _SVTOOLS_JAVAINTERACTION_HXX_

#include "svtools/svtdllapi.h"
#include <osl/mutex.hxx>
#include <tools/gen.hxx>

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <svtools/javacontext.hxx>

#define JAVA_INTERACTION_HANDLER_NAME "java-vm.interaction-handler"

namespace svt
{
// We cannot derive from  cppu::WeakImplHelper because we would export the inline
//generated class. This conflicts with other libraries if they use the same inline
//class.

class SVT_DLLPUBLIC JavaInteractionHandler:
        public com::sun::star::task::XInteractionHandler
{
public:
    JavaInteractionHandler();
    JavaInteractionHandler(bool bReportErrorOnce);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    // XCurrentContext
    virtual void SAL_CALL handle( const com::sun::star::uno::Reference<
                                  com::sun::star::task::XInteractionRequest >& Request )
        throw (com::sun::star::uno::RuntimeException);

private:
    oslInterlockedCount m_aRefCount;
    SVT_DLLPRIVATE JavaInteractionHandler(JavaInteractionHandler const&); //not implemented
    SVT_DLLPRIVATE JavaInteractionHandler& operator = (JavaInteractionHandler const &); //not implemented
    SVT_DLLPRIVATE virtual ~JavaInteractionHandler();
    bool m_bShowErrorsOnce;
    bool m_bJavaDisabled_Handled;
    bool m_bInvalidSettings_Handled;
    bool m_bJavaNotFound_Handled;
    bool m_bVMCreationFailure_Handled;
    bool m_bRestartRequired_Handled;
    sal_uInt16 m_nResult_JavaDisabled;
};
}

#endif // _DESKTOP_JAVAINTERACTION_HXX_
