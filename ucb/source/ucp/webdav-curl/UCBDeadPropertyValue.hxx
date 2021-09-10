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

#include <rtl/string.hxx>
#include <com/sun/star/uno/Any.hxx>

namespace http_dav_ucp
{

class UCBDeadPropertyValue
{
private:
    static const OUString aTypeString;
    static const OUString aTypeLong;
    static const OUString aTypeShort;
    static const OUString aTypeBoolean;
    static const OUString aTypeChar;
    static const OUString aTypeByte;
    static const OUString aTypeHyper;
    static const OUString aTypeFloat;
    static const OUString aTypeDouble;

    static const OUString aXMLPre;
    static const OUString aXMLMid;
    static const OUString aXMLEnd;

public:
    static bool supportsType( const css::uno::Type & rType );

    static bool createFromXML( const OString & rInData,
                               css::uno::Any & rOutData );
    static bool toXML( const css::uno::Any & rInData,
                       OUString & rOutData );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
