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
#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTI_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTI_HXX

#include <xmloff/txtimp.hxx>

class XMLRedlineImportHelper;
class SvXMLImport;

class SwXMLTextImportHelper : public XMLTextImportHelper
{
    XMLRedlineImportHelper *pRedlineHelper;

protected:
    virtual SvXMLImportContext *CreateTableChildContext(
                SvXMLImport& rImport,
                sal_Int32 nElement,
                const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;

public:
    SwXMLTextImportHelper(
            const css::uno::Reference<css::frame::XModel>& rModel,
            SvXMLImport& rImport,
            const css::uno::Reference<css::beans::XPropertySet>& rInfoSet,
            bool bInsertM, bool bStylesOnlyM,
            bool bBlockM, bool bOrganizerM );
    virtual ~SwXMLTextImportHelper() override;

    virtual css::uno::Reference<css::beans::XPropertySet>
            createAndInsertOLEObject( SvXMLImport& rImport,
                                      const OUString& rHRef,
                                      const OUString& rStyleName,
                                      const OUString& rTableName,
                                      sal_Int32 nWidth, sal_Int32 nHeight ) override;
    virtual css::uno::Reference<css::beans::XPropertySet>
            createAndInsertOOoLink( SvXMLImport& rImport,
                                      const OUString& rHRef,
                                      const OUString& rStyleName,
                                      const OUString& rTableName,
                                      sal_Int32 nWidth, sal_Int32 nHeight ) override;
    virtual css::uno::Reference<css::beans::XPropertySet>
        createAndInsertApplet(
            const OUString &rName,
            const OUString &rCode,
            bool bMayScript,
            const OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight ) override;

    virtual css::uno::Reference<css::beans::XPropertySet>
        createAndInsertPlugin(
            const OUString &rMimeType,
            const OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight ) override;

    virtual css::uno::Reference<css::beans::XPropertySet>
        createAndInsertFloatingFrame(
            const OUString &rName,
            const OUString &rHRef,
            const OUString &rStyleName,
            sal_Int32 nWidth, sal_Int32 nHeight ) override;

    virtual void endAppletOrPlugin(
        const css::uno::Reference < css::beans::XPropertySet > &rPropSet,
        std::map < const OUString, OUString > &rParamMap) override;

    virtual bool IsInHeaderFooter() const override;

    // redlining helper methods
    // (here is the real implementation)
    virtual void RedlineAdd(
        const OUString& rType,       /// redline type (insert, del,... )
        const OUString& rId,         /// use to identify this redline
        const OUString& rAuthor,     /// name of the author
        const OUString& rComment,    /// redline comment
        const css::util::DateTime& rDateTime,  /// date+time
        bool bMergeLastPara) override;           /// merge last paragraph
    virtual css::uno::Reference<css::text::XTextCursor> RedlineCreateText(
            css::uno::Reference<css::text::XTextCursor> & rOldCursor, /// needed to get the document
            const OUString& rId) override;    /// ID used to RedlineAdd() call
    virtual void RedlineSetCursor(
        const OUString& rId,         /// ID used to RedlineAdd() call
        bool bStart,                    /// start or end Cursor
        bool bIsOutsideOfParagraph) override;
    virtual void RedlineAdjustStartNodeCursor() override;
    virtual void SetShowChanges( bool bShowChanges ) override;
    virtual void SetRecordChanges( bool bRecordChanges ) override;
    virtual void SetChangesProtectionKey(
        const css::uno::Sequence<sal_Int8> & rKey ) override;
};

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
