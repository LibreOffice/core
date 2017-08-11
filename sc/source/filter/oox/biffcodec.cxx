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

#include "biffcodec.hxx"

#include <osl/thread.h>
#include <oox/helper/attributelist.hxx>
#include <string.h>

namespace oox {
namespace xls {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;


BiffDecoderBase::BiffDecoderBase() :
    mbValid( false )
{
}

BiffDecoderBase::~BiffDecoderBase()
{
}

::comphelper::DocPasswordVerifierResult BiffDecoderBase::verifyPassword( const OUString& rPassword, Sequence< NamedValue >& o_rEncryptionData )
{
    o_rEncryptionData = implVerifyPassword( rPassword );
    mbValid = o_rEncryptionData.hasElements();
    return mbValid ? ::comphelper::DocPasswordVerifierResult::OK : ::comphelper::DocPasswordVerifierResult::WrongPassword;
}

::comphelper::DocPasswordVerifierResult BiffDecoderBase::verifyEncryptionData( const Sequence< NamedValue >& rEncryptionData )
{
    mbValid = implVerifyEncryptionData( rEncryptionData );
    return mbValid ? ::comphelper::DocPasswordVerifierResult::OK : ::comphelper::DocPasswordVerifierResult::WrongPassword;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
