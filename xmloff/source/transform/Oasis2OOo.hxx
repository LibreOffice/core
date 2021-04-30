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

#pragma once

#include "ActionMapTypesOASIS.hxx"
#include "TransformerBase.hxx"

class XMLTransformerOASISEventMap_Impl;

class Oasis2OOoTransformer : public XMLTransformerBase
{
    std::unique_ptr<XMLTransformerActions> m_aActions[MAX_OASIS_ACTIONS];
    XMLTransformerOASISEventMap_Impl *m_pEventMap;
    XMLTransformerOASISEventMap_Impl *m_pFormEventMap;

protected:

    virtual XMLTransformerContext *CreateUserDefinedContext(
                                      const TransformerAction_Impl& rAction,
                                      const OUString& rQName,
                                         bool bPersistent=false ) override;

    virtual XMLTransformerActions *GetUserDefinedActions( sal_uInt16 n ) override;

public:
    Oasis2OOoTransformer () noexcept;
    virtual ~Oasis2OOoTransformer() noexcept override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XUnoTunnel
    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId() noexcept;
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    virtual OUString GetEventName( const OUString& rName,
                                             bool bForm = false ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
