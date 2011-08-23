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

#ifndef _SVTOOLS_JAVAINTERACTION_HXX_
#define _SVTOOLS_JAVAINTERACTION_HXX_

#include "bf_svtools/svtdllapi.h"
#include <osl/mutex.hxx>
#include <tools/gen.hxx>

#include <com/sun/star/task/XInteractionRequest.hpp>

#include <bf_svtools/javacontext.hxx>

#define JAVA_INTERACTION_HANDLER_NAME "java-vm.interaction-handler"

namespace binfilter
{
// We cannot derive from  cppu::WeakImplHelper because we would export the inline
//generated class. This conflicts with other libraries if they use the same inline
//class.

class  JavaInteractionHandler:
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
    oslInterlockedCount	m_aRefCount;
    JavaInteractionHandler(JavaInteractionHandler const&); //not implemented
    JavaInteractionHandler& operator = (JavaInteractionHandler const &); //not implemented
    virtual ~JavaInteractionHandler();
    bool m_bShowErrorsOnce;
    bool m_bJavaDisabled_Handled;
    bool m_bInvalidSettings_Handled;
    bool m_bJavaNotFound_Handled;
    bool m_bVMCreationFailure_Handled;
    bool m_bRestartRequired_Handled;
    USHORT m_nResult_JavaDisabled;
};
}

#endif // _DESKTOP_JAVAINTERACTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
