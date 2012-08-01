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

#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#define _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <rtl/ustring.hxx>
#include <xmloff/uniref.hxx>
#include <set>

class XMLFontAutoStylePool_Impl;
typedef std::set<rtl::OUString> XMLFontAutoStylePoolNames_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLFontAutoStylePool : public UniRefBase
{
    SvXMLExport& rExport;

    XMLFontAutoStylePool_Impl *pPool;
    XMLFontAutoStylePoolNames_Impl m_aNames;
    sal_uInt32 nName;

protected:

    SvXMLExport& GetExport() { return rExport; }

public:

    XMLFontAutoStylePool( SvXMLExport& rExport );
    ~XMLFontAutoStylePool();

    ::rtl::OUString Add(
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc );

    ::rtl::OUString Find(
            const ::rtl::OUString& rFamilyName,
            const ::rtl::OUString& rStyleName,
            sal_Int16 nFamily,
            sal_Int16 nPitch,
            rtl_TextEncoding eEnc )const;
    ::rtl::OUString Find( const ::rtl::OUString& rInternalName ) const;

    void exportXML();
};

#endif  //  _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
