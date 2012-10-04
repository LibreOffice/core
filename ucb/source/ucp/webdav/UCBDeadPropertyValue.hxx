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



#ifndef _UCBDEADPROPERTYVALUE_HXX_
#define _UCBDEADPROPERTYVALUE_HXX_

#include <rtl/string.hxx>
#include <com/sun/star/uno/Any.hxx>

namespace http_dav_ucp
{

class UCBDeadPropertyValue
{
private:
    static const rtl::OUString aTypeString;
    static const rtl::OUString aTypeLong;
    static const rtl::OUString aTypeShort;
    static const rtl::OUString aTypeBoolean;
    static const rtl::OUString aTypeChar;
    static const rtl::OUString aTypeByte;
    static const rtl::OUString aTypeHyper;
    static const rtl::OUString aTypeFloat;
    static const rtl::OUString aTypeDouble;

    static const rtl::OUString aXMLPre;
    static const rtl::OUString aXMLMid;
    static const rtl::OUString aXMLEnd;

public:
    static bool supportsType( const com::sun::star::uno::Type & rType );

    static bool createFromXML( const rtl::OString & rInData,
                               com::sun::star::uno::Any & rOutData );
    static bool toXML( const com::sun::star::uno::Any & rInData,
                       rtl::OUString & rOutData );
};

}

#endif /* _UCBDEADPROPERTYVALUE_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
