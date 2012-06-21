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

#ifndef _XMLOFF_XMLINDEXALPHABETICALSOURCECONTEXT_HXX_
#define _XMLOFF_XMLINDEXALPHABETICALSOURCECONTEXT_HXX_

#include "XMLIndexSourceBaseContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/Locale.hpp>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
namespace rtl { class OUString; }


/**
 * Import alphabetical (keyword) index source element
 */
class XMLIndexAlphabeticalSourceContext : public XMLIndexSourceBaseContext
{
    const ::rtl::OUString sMainEntryCharacterStyleName;
    const ::rtl::OUString sUseAlphabeticalSeparators;
    const ::rtl::OUString sUseCombinedEntries;
    const ::rtl::OUString sIsCaseSensitive;
    const ::rtl::OUString sUseKeyAsEntry;
    const ::rtl::OUString sUseUpperCase;
    const ::rtl::OUString sUseDash;
    const ::rtl::OUString sUsePP;
    const ::rtl::OUString sIsCommaSeparated;
    const ::rtl::OUString sSortAlgorithm;
    const ::rtl::OUString sLocale;

    ::com::sun::star::lang::Locale aLocale;
    ::rtl::OUString sAlgorithm;

    ::rtl::OUString sMainEntryStyleName;
    sal_Bool bMainEntryStyleNameOK;

    sal_Bool bSeparators;
    sal_Bool bCombineEntries;
    sal_Bool bCaseSensitive;
    sal_Bool bEntry;
    sal_Bool bUpperCase;
    sal_Bool bCombineDash;
    sal_Bool bCombinePP;
    sal_Bool bCommaSeparated;

public:

    TYPEINFO();

    XMLIndexAlphabeticalSourceContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    ~XMLIndexAlphabeticalSourceContext();

protected:

    virtual void ProcessAttribute(
        enum IndexSourceParamEnum eParam,
        const ::rtl::OUString& rValue);

    virtual void EndElement();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
