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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXALPHABETICALSOURCECONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXALPHABETICALSOURCECONTEXT_HXX

#include "XMLIndexSourceBaseContext.hxx"
#include <xmloff/languagetagodf.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/Locale.hpp>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }


/**
 * Import alphabetical (keyword) index source element
 */
class XMLIndexAlphabeticalSourceContext : public XMLIndexSourceBaseContext
{
    const OUString sMainEntryCharacterStyleName;
    const OUString sUseAlphabeticalSeparators;
    const OUString sUseCombinedEntries;
    const OUString sIsCaseSensitive;
    const OUString sUseKeyAsEntry;
    const OUString sUseUpperCase;
    const OUString sUseDash;
    const OUString sUsePP;
    const OUString sIsCommaSeparated;
    const OUString sSortAlgorithm;
    const OUString sLocale;

    LanguageTagODF maLanguageTagODF;
    OUString sAlgorithm;

    OUString sMainEntryStyleName;
    bool bMainEntryStyleNameOK;

    bool bSeparators;
    bool bCombineEntries;
    bool bCaseSensitive;
    bool bEntry;
    bool bUpperCase;
    bool bCombineDash;
    bool bCombinePP;
    bool bCommaSeparated;

public:

    TYPEINFO_OVERRIDE();

    XMLIndexAlphabeticalSourceContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    virtual ~XMLIndexAlphabeticalSourceContext();

protected:

    virtual void ProcessAttribute(
        enum IndexSourceParamEnum eParam,
        const OUString& rValue) override;

    virtual void EndElement() override;

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
