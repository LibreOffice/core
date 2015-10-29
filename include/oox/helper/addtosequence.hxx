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

#ifndef INCLUDED_OOX_HELPER_ADDTOSEQUENCE_HXX
#define INCLUDED_OOX_HELPER_ADDTOSEQUENCE_HXX

#include <com/sun/star/uno/Any.hxx>
#include <oox/dllapi.h>
#include <rtl/ustring.hxx>

namespace oox
{

/** this adds an any to another any.
    if rNewValue is empty, rOldValue is returned.
    if rOldValue is empty, rNewValue is returned.
    if rOldValue contains a value, a sequence with rOldValue and rNewValue is returned.
    if rOldValue contains a sequence, a new sequence with the old sequence and rNewValue is returned.
*/
OOX_DLLPUBLIC css::uno::Any addToSequence( const css::uno::Any& rOldValue, const css::uno::Any& rNewValue );

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
