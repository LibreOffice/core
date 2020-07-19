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

#ifndef INCLUDED_COMPHELPER_XMLSECHELPER_HXX
#define INCLUDED_COMPHELPER_XMLSECHELPER_HXX

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/security/CertificateKind.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>

namespace comphelper::xmlsec
{
COMPHELPER_DLLPUBLIC OUString GetCertificateKind(const css::security::CertificateKind& rKind);

COMPHELPER_DLLPUBLIC std::vector<std::pair<OUString, OUString>> parseDN(const OUString& rRawString);
COMPHELPER_DLLPUBLIC std::pair<OUString, OUString>
GetDNForCertDetailsView(const OUString& rRawString);
COMPHELPER_DLLPUBLIC OUString GetContentPart(const OUString& _rRawString,
                                             const css::security::CertificateKind& rKind);

COMPHELPER_DLLPUBLIC OUString GetHexString(const css::uno::Sequence<sal_Int8>& _rSeq,
                                           const char* _pSep, sal_uInt16 _nLineBreak = 0xFFFF);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
