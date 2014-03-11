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

#ifndef INCLUDED_XMLOFF_INC_XMLINDEXBIBLIOGRAPHYCONFIGURATIONCONTEXT_HXX
#define INCLUDED_XMLOFF_INC_XMLINDEXBIBLIOGRAPHYCONFIGURATIONCONTEXT_HXX

#include <xmloff/xmlstyle.hxx>
#include <xmloff/languagetagodf.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustring.hxx>


#include <vector>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }


/**
 * Import bibliography configuration.
 *
 * Little cheat: Cover all child elements in CreateChildContext.
 */
class XMLIndexBibliographyConfigurationContext : public SvXMLStyleContext
{
    const OUString sFieldMaster_Bibliography;
    const OUString sBracketBefore;
    const OUString sBracketAfter;
    const OUString sIsNumberEntries;
    const OUString sIsSortByPosition;
    const OUString sSortKeys;
    const OUString sSortKey;
    const OUString sIsSortAscending;
    const OUString sSortAlgorithm;
    const OUString sLocale;

    OUString sSuffix;
    OUString sPrefix;
    OUString sAlgorithm;
    LanguageTagODF maLanguageTagODF;
    sal_Bool bNumberedEntries;
    sal_Bool bSortByPosition;

    ::std::vector< ::com::sun::star::uno::Sequence<
                        ::com::sun::star::beans::PropertyValue> > aSortKeys;

public:

    TYPEINFO();

    XMLIndexBibliographyConfigurationContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    ~XMLIndexBibliographyConfigurationContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void CreateAndInsert( sal_Bool bOverwrite );

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    void ProcessAttribute(
        sal_uInt16 nPrefix,
        const OUString& sLocalName,
        const OUString& sValue);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
