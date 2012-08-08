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

#ifndef _COMPHELPER_STILLREADWRITEINTERACTION_HXX_
#define _COMPHELPER_STRILLREADWRITEINTERACTION_HXX_

#include <ucbhelper/interceptedinteraction.hxx>

#include <com/sun/star/task/XInteractionHandler.hpp>

#include "comphelper/comphelperdllapi.h"

//_______________________________________________
// namespace

namespace comphelper{
class COMPHELPER_DLLPUBLIC StillReadWriteInteraction : public ::ucbhelper::InterceptedInteraction
{
private:
    static const sal_Int32 HANDLE_INTERACTIVEIOEXCEPTION       = 0;
    static const sal_Int32 HANDLE_UNSUPPORTEDDATASINKEXCEPTION = 1;

    sal_Bool m_bUsed;
    sal_Bool m_bHandledByMySelf;
    sal_Bool m_bHandledByInternalHandler;

public:
    StillReadWriteInteraction(const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xHandler);

    void resetInterceptions();
    void resetErrorStates();
    sal_Bool wasWriteError();

private:
    virtual ucbhelper::InterceptedInteraction::EInterceptionState intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest&                         aRequest,
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest);

};
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
