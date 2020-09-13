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

#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>

#include <map>

class CMimeContentType : public
    cppu::WeakImplHelper< css::datatransfer::XMimeContentType >
{
public:
    explicit CMimeContentType(const OUString& rCntType);

    // XMimeContentType

    virtual OUString SAL_CALL getMediaType(  ) override;
    virtual OUString SAL_CALL getMediaSubtype(  ) override;
    virtual OUString SAL_CALL getFullMediaType(  ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getParameters(  ) override;

    virtual sal_Bool SAL_CALL hasParameter( const OUString& aName ) override;

    virtual OUString SAL_CALL getParameterValue( const OUString& aName ) override;

private:
    /// @throws css::lang::IllegalArgumentException
    void init( const OUString& aCntType );

private:
    OUString                            m_MediaType;
    OUString                            m_MediaSubtype;
    std::map< OUString, OUString > m_ParameterMap;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
