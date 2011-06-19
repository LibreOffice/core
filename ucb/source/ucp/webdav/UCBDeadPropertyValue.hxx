/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UCBDEADPROPERTYVALUE_HXX_
#define _UCBDEADPROPERTYVALUE_HXX_

#include <rtl/string.hxx>
#include <com/sun/star/uno/Any.hxx>

namespace webdav_ucp
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
