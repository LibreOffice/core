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
#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLREDLINEIMPORTHELPER_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLREDLINEIMPORTHELPER_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/util/DateTime.hpp>
#include <redline.hxx>

#include <map>

class SvXMLImport;
class RedlineInfo;
class SwRedlineData;
class SwDoc;
namespace com::sun::star {
    namespace text { class XTextCursor; }
    namespace text { class XTextRange; }
    namespace frame { class XModel; }
}

typedef std::map< OUString, RedlineInfo* > RedlineMapType;

class XMLRedlineImportHelper final
{
    SvXMLImport & m_rImport;

    const OUString m_sInsertion;
    const OUString m_sDeletion;
    const OUString m_sFormatChange;

    RedlineMapType m_aRedlineMap;

    // if true, no redlines should be inserted into document
    // (This typically happen when a document is loaded in 'insert'-mode.)
    bool m_bIgnoreRedlines;

    // save information for saving and reconstruction of the redline mode
    css::uno::Reference<css::beans::XPropertySet> m_xModelPropertySet;
    css::uno::Reference<css::beans::XPropertySet> m_xImportInfoPropertySet;
    bool m_bShowChanges;
    bool m_bRecordChanges;
    css::uno::Sequence<sal_Int8> m_aProtectionKey;

public:

    XMLRedlineImportHelper(
        SvXMLImport & rImport,
        bool bIgnoreRedlines,       // ignore redlines mode
        // property sets of model + import info for saving + restoring the
        // redline mode
        const css::uno::Reference<css::beans::XPropertySet> & rModel,
        const css::uno::Reference<css::beans::XPropertySet> & rImportInfoSet );
    ~XMLRedlineImportHelper();

    // create a redline object
    // (The redline will be inserted into the document after both start
    //  and end cursor has been set.)
    void Add(
        const OUString& rType,       // redline type (insert, del,... )
        const OUString& rId,         // use to identify this redline
        const OUString& rAuthor,     // name of the author
        const OUString& rComment,    // redline comment
        const css::util::DateTime& rDateTime, // date+time
        bool bMergeLastParagraph);      // merge last paragraph?

    // create a text section for the redline, and return an
    // XText/XTextCursor that may be used to write into it.
    css::uno::Reference<css::text::XTextCursor> CreateRedlineTextSection(
            css::uno::Reference<css::text::XTextCursor> const & xOldCursor, // needed to get the document
            const OUString& rId);    // ID used to RedlineAdd() call

    // Set start or end position for a redline in the text body.
    // Accepts XTextRange objects.
    void SetCursor(
        const OUString& rId,     // ID used in RedlineAdd() call
        bool bStart,                // start or end Range
        css::uno::Reference<css::text::XTextRange> const & rRange, // the actual XTextRange
        // text range is (from an XML view) outside of a paragraph
        // (i.e. before a table)
        bool bIsOutsideOfParagraph);

    /**
     * Adjust the start (end) position for a redline that begins in a
     * start node. It takes the cursor positions _inside_ the redlined
     * element (e.g. section or table).
     */
    void AdjustStartNodeCursor(
         const OUString& rId);        // ID used in RedlineAdd() call

    // set redline mode: show changes
    void SetShowChanges( bool bShowChanges );

    // set redline mode: record changes
    void SetRecordChanges( bool bRecordChanges );

    // set redline protection key
    void SetProtectionKey(const css::uno::Sequence<sal_Int8> & rKey );

private:

    static inline bool IsReady(const RedlineInfo* pRedline);

    void InsertIntoDocument(RedlineInfo* pRedline);

    SwRedlineData* ConvertRedline(
        RedlineInfo* pRedline,  // RedlineInfo to be converted
        SwDoc* pDoc);           // document needed for Author-ID conversion
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
