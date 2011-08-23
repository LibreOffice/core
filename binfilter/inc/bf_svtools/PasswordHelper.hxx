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

#ifndef _SVTOOLS_PASSWORDHELPER_HXX
#define _SVTOOLS_PASSWORDHELPER_HXX

#ifndef INCLUDED_SVLDLLAPI_H

#endif

#include "sal/types.h"

#include "com/sun/star/uno/Sequence.hxx"

class String;

namespace binfilter
{

class SvPasswordHelper
{
    static void		GetHashPassword(com::sun::star::uno::Sequence <sal_Int8>& rPassHash, const sal_Char* pPass, sal_uInt32 nLen);
    static void     GetHashPasswordLittleEndian(com::sun::star::uno::Sequence<sal_Int8>& rPassHash, const String& sPass);
    static void     GetHashPasswordBigEndian(com::sun::star::uno::Sequence<sal_Int8>& rPassHash, const String& sPass);

public:
     static void     GetHashPassword(com::sun::star::uno::Sequence<sal_Int8>& rPassHash, const String& sPass);
    /**
    Use this method to compare a given string with another given Hash value.
    This is necessary, because in older versions exists different hashs of the same string. They were endian dependent.
    We need this to handle old files. This method will compare against big and little endian. See #101326#
    */
     static bool     CompareHashPassword(const com::sun::star::uno::Sequence<sal_Int8>& rOldPassHash, const String& sNewPass);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
