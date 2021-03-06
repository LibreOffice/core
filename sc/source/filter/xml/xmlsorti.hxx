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

#include <xmloff/languagetagodf.hxx>
#include <com/sun/star/table/CellAddress.hpp>

#include "importcontext.hxx"

namespace com::sun::star::util { struct SortField; }
namespace sax_fastparser { class FastAttributeList; }

class ScXMLImport;
class ScXMLDatabaseRangeContext;

class ScXMLSortContext : public ScXMLImportContext
{
    ScXMLDatabaseRangeContext* pDatabaseRangeContext;

    css::uno::Sequence <css::util::SortField> aSortFields;
    css::table::CellAddress aOutputPosition;
    LanguageTagODF maLanguageTagODF;
    OUString    sAlgorithm;
    sal_Int16   nUserListIndex;
    bool        bCopyOutputData;
    bool        bBindFormatsToContent;
    bool        bIsCaseSensitive;
    bool        bEnabledUserList;

public:

    ScXMLSortContext( ScXMLImport& rImport,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLDatabaseRangeContext* pTempDatabaseRangeContext);

    virtual ~ScXMLSortContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    void AddSortField(const OUString& sFieldNumber, const OUString& sDataType, std::u16string_view sOrder);
};

class ScXMLSortByContext : public ScXMLImportContext
{
    ScXMLSortContext* pSortContext;

    OUString   sFieldNumber;
    OUString   sDataType;
    OUString   sOrder;

public:

    ScXMLSortByContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
                        ScXMLSortContext* pTempSortContext);

    virtual ~ScXMLSortByContext() override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
