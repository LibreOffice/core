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
#ifndef INCLUDED_PACKAGE_INC_CRC32_HXX
#define INCLUDED_PACKAGE_INC_CRC32_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }
class CRC32 final
{
    sal_uInt32 nCRC;
public:
    CRC32();
    ~CRC32();

    /// @throws css::uno::RuntimeException
    sal_Int64 updateStream (css::uno::Reference < css::io::XInputStream > const & xStream);
    /// @throws css::uno::RuntimeException
    void updateSegment(const css::uno::Sequence< sal_Int8 > &b, sal_Int32 len);
    /// @throws css::uno::RuntimeException
    void update(const css::uno::Sequence< sal_Int8 > &b);
    /// @throws css::uno::RuntimeException
    sal_Int32 getValue() const;
    /// @throws css::uno::RuntimeException
    void reset();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
