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

#ifndef INCLUDED_SFX2_SIGNATURESTATE_HXX
#define INCLUDED_SFX2_SIGNATURESTATE_HXX

#include <config_options.h>
#include <sfx2/dllapi.h>

#include <com/sun/star/security/DocumentSignatureInformation.hpp>

enum class SignatureState
{
    // FIXME: Do these values have to be these, and given at all, or is this just cargo cult?
    UNKNOWN = 0xffff, // used to be -1 when this was a sal_uInt16
    NOSIGNATURES = 0,
    OK = 1,
    BROKEN = 2,
    // State was SignatureState::OK, but doc is modified now
    INVALID = 3,
    // signature is OK, but certificate could not be validated
    NOTVALIDATED = 4,
    // signature and certificate are ok, but not all files are signed, as it was the case in
    // OOo 2.x - OOo 3.1.1. This state is only used together with document signatures.
    PARTIAL_OK = 5,
    /// Certificate could not be validated and the document is only partially signed.
    NOTVALIDATED_PARTIAL_OK = 6
};

namespace DocumentSignatures
{
/** Get document signature state */
UNLESS_MERGELIBS_MORE(SFX2_DLLPUBLIC)
SignatureState
getSignatureState(const css::uno::Sequence<css::security::DocumentSignatureInformation>& aInfos);
}

#endif // INCLUDED_SFX2_SIGNATURESTATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
