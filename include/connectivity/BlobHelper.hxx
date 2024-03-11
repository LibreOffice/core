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

#include <config_options.h>
#include <connectivity/dbtoolsdllapi.hxx>
#include <com/sun/star/sdbc/XBlob.hpp>
#include <cppuhelper/implbase.hxx>

namespace connectivity
{
    class UNLESS_MERGELIBS_MORE(OOO_DLLPUBLIC_DBTOOLS) BlobHelper final : public ::cppu::WeakImplHelper< css::sdbc::XBlob >
    {
        css::uno::Sequence< sal_Int8 > m_aValue;
    public:
        BlobHelper(const css::uno::Sequence< sal_Int8 >& _val);
    private:
        virtual ::sal_Int64 SAL_CALL length(  ) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getBytes( ::sal_Int64 pos, ::sal_Int32 length ) override;
        virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream(  ) override;
        virtual ::sal_Int64 SAL_CALL position( const css::uno::Sequence< ::sal_Int8 >& pattern, ::sal_Int64 start ) override;
        virtual ::sal_Int64 SAL_CALL positionOfBlob( const css::uno::Reference< css::sdbc::XBlob >& pattern, ::sal_Int64 start ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
