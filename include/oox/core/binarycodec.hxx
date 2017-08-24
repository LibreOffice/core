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

#ifndef INCLUDED_OOX_CORE_BINARYCODEC_HXX
#define INCLUDED_OOX_CORE_BINARYCODEC_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <oox/dllapi.h>
#include <rtl/cipher.h>
#include <rtl/digest.h>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace beans { struct NamedValue; }
} } }

namespace oox { class AttributeList; }

namespace oox {
namespace core {


namespace CodecHelper
{
    /** Returns the password hash if it is in the required 16-bit limit. */
    OOX_DLLPUBLIC sal_uInt16   getPasswordHash( const AttributeList& rAttribs, sal_Int32 nElement );
}


} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
