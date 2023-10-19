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

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.h>

class SvXMLExport;
namespace com::sun::star {
    namespace beans { class XPropertySet; }
}


/**
 * This class handles the export of index marks for table of content,
 * alphabetical and user index.
 *
 * Marks for bibliography indices are internally modelled as text
 * fields and thus handled in txtparae.cxx
 */
class XMLIndexMarkExport
{
    static constexpr OUString gsLevel = u"Level"_ustr;
    static constexpr OUString gsUserIndexName = u"UserIndexName"_ustr;
    static constexpr OUString gsPrimaryKey = u"PrimaryKey"_ustr;
    static constexpr OUString gsSecondaryKey = u"SecondaryKey"_ustr;
    static constexpr OUString gsDocumentIndexMark = u"DocumentIndexMark"_ustr;
    static constexpr OUString gsIsStart = u"IsStart"_ustr;
    static constexpr OUString gsIsCollapsed = u"IsCollapsed"_ustr;
    static constexpr OUString gsAlternativeText = u"AlternativeText"_ustr;
    static constexpr OUString gsTextReading = u"TextReading"_ustr;
    static constexpr OUString gsPrimaryKeyReading = u"PrimaryKeyReading"_ustr;
    static constexpr OUString gsSecondaryKeyReading = u"SecondaryKeyReading"_ustr;
    static constexpr OUString gsMainEntry = u"IsMainEntry"_ustr;

    SvXMLExport& rExport;

public:
    explicit XMLIndexMarkExport(SvXMLExport& rExp);

    /**
     * export by the property set of its *text* *portion*.
     *
     * The text portion supplies us with the properties of the index
     * mark itself, as well as the information whether we are at the
     * start or end of an index mark, or whether the index mark is
     * collapsed.
     */
    void ExportIndexMark(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        bool bAutoStyles);

private:

    /// export attributes of table-of-content index marks
    void ExportTOCMarkAttributes(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// export attributes of user index marks
    void ExportUserIndexMarkAttributes(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// export attributes of alphabetical  index marks
    void ExportAlphabeticalIndexMarkAttributes(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// create a numerical ID for this index mark
    /// (represented by its properties)
    static void GetID(
        OUStringBuffer& sBuffer,
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
