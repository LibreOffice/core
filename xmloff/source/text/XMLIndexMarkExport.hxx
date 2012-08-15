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

#ifndef _XMLOFF_XMLINDEXMARKEXPORT_HXX_
#define _XMLOFF_XMLINDEXMARKEXPORT_HXX_

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
 } } }
namespace rtl {
    class OUString;
    class OUStringBuffer;
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
    ::rtl::OUString sLevel;
    ::rtl::OUString sUserIndexName;
    ::rtl::OUString sPrimaryKey;
    ::rtl::OUString sSecondaryKey;
    ::rtl::OUString sDocumentIndexMark;
    ::rtl::OUString sIsStart;
    ::rtl::OUString sIsCollapsed;
    ::rtl::OUString sAlternativeText;
    ::rtl::OUString sTextReading;
    ::rtl::OUString sPrimaryKeyReading;
    ::rtl::OUString sSecondaryKeyReading;
    ::rtl::OUString sMainEntry;


    SvXMLExport& rExport;

public:
    XMLIndexMarkExport(SvXMLExport& rExp);

    ~XMLIndexMarkExport();

    /**
     * export by the property set of its *text* *portion*.
     *
     * The text portion supplies us with the properties of the index
     * mark itself, as well as the information whether we are at the
     * start or end of an index mark, or whether the index mark is
     * collapsed.
     */
    void ExportIndexMark(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_Bool bAutoStyles);

protected:

    /// export attributes of table-of-content index marks
    void ExportTOCMarkAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export attributes of user index marks
    void ExportUserIndexMarkAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export attributes of alphabetical  index marks
    void ExportAlphabeticalIndexMarkAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// create a numerical ID for this index mark
    /// (represented by its properties)
    void GetID(
        ::rtl::OUStringBuffer& sBuffer,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
