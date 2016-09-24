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

#ifndef INCLUDED_SVL_PASSWORDHELPER_HXX
#define INCLUDED_SVL_PASSWORDHELPER_HXX

#include <svl/svldllapi.h>
#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>

class SvPasswordHelper
{
    static void     GetHashPasswordLittleEndian(css::uno::Sequence<sal_Int8>& rPassHash, const OUString& sPass);
    static void     GetHashPasswordBigEndian(css::uno::Sequence<sal_Int8>& rPassHash, const OUString& sPass);

public:
    SVL_DLLPUBLIC static void     GetHashPassword(css::uno::Sequence <sal_Int8>& rPassHash, const sal_Char* pPass, sal_uInt32 nLen);

    SVL_DLLPUBLIC static void     GetHashPassword(css::uno::Sequence<sal_Int8>& rPassHash, const OUString& sPass);
    /**
    Use this method to compare a given string with another given Hash value.
    This is necessary, because in older versions exists different hashes of the same string. They were endian dependent.
    We need this to handle old files. This method will compare against big and little endian. See #101326#
    */
    SVL_DLLPUBLIC static bool     CompareHashPassword(const css::uno::Sequence<sal_Int8>& rOldPassHash, const OUString& sNewPass);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
