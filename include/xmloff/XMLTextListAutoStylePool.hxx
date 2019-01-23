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

#ifndef INCLUDED_XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#define INCLUDED_XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <memory>
#include <set>

namespace com { namespace sun { namespace star { namespace container {
    class XIndexReplace; } } } }
namespace com { namespace sun { namespace star { namespace ucb { class XAnyCompare; } } } }


class XMLTextListAutoStylePool_Impl;
typedef std::set<OUString> XMLTextListAutoStylePoolNames_Impl;
class XMLTextListAutoStylePoolEntry_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLTextListAutoStylePool
{
    SvXMLExport& rExport;

    OUString sPrefix;

    std::unique_ptr<XMLTextListAutoStylePool_Impl> pPool;
    XMLTextListAutoStylePoolNames_Impl m_aNames;
    sal_uInt32 nName;

    /** this is an optional NumRule compare component for applications where
        the NumRules don't have names */
    css::uno::Reference< css::ucb::XAnyCompare > mxNumRuleCompare;

    SAL_DLLPRIVATE sal_uInt32 Find( const XMLTextListAutoStylePoolEntry_Impl* pEntry )
        const;
public:

    XMLTextListAutoStylePool( SvXMLExport& rExport );
    ~XMLTextListAutoStylePool();

    void RegisterName( const OUString& rName );

    OUString Add(
            const css::uno::Reference< css::container::XIndexReplace > & rNumRules );

    OUString Find(
            const css::uno::Reference< css::container::XIndexReplace > & rNumRules ) const;
    OUString Find( const OUString& rInternalName ) const;

    void exportXML() const;
};


#endif // INCLUDED_XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
