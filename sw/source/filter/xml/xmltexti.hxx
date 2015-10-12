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
                sal_uInt16 nPrefix, const OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

public:
    SwXMLTextImportHelper(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::frame::XModel>& rModel,
            SvXMLImport& rImport,
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet>& rInfoSet,
            bool bInsertM, bool bStylesOnlyM, bool bProgress,
            bool bBlockM, bool bOrganizerM,
            bool bPreserveRedlineMode );
    virtual ~SwXMLTextImportHelper();

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
            createAndInsertOLEObject( SvXMLImport& rImport,
                                      const OUString& rHRef,
                                      const OUString& rStyleName,
                                      const OUString& rTableName,
                                         sal_Int32 nWidth, sal_Int32 nHeight ) override;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
            createAndInsertOOoLink( SvXMLImport& rImport,
                                      const OUString& rHRef,
                                      const OUString& rStyleName,
                                      const OUString& rTableName,
                                         sal_Int32 nWidth, sal_Int32 nHeight ) override;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertApplet(
            const OUString &rName,
            const OUString &rCode,
            bool bMayScript,
            const OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight ) override;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertPlugin(
            const OUString &rMimeType,
            const OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight ) override;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertFloatingFrame(
            const OUString &rName,
            const OUString &rHRef,
            const OUString &rStyleName,
            sal_Int32 nWidth, sal_Int32 nHeight ) override;

    virtual void endAppletOrPlugin(
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > &rPropSet,
        ::std::map < const OUString, OUString > &rParamMap) override;

    virtual bool IsInHeaderFooter() const override;

    // redlining helper methods
    // (here is the real implementation)
    virtual void RedlineAdd(
        const OUString& rType,       /// redline type (insert, del,... )
        const OUString& rId,         /// use to identify this redline
        const OUString& rAuthor,     /// name of the author
        const OUString& rComment,    /// redline comment
        const ::com::sun::star::util::DateTime& rDateTime,  /// date+time
        bool bMergeLastPara) override;           /// merge last paragraph
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextCursor> RedlineCreateText(
            ::com::sun::star::uno::Reference<   /// needed to get the document
                    ::com::sun::star::text::XTextCursor> & rOldCursor,
            const OUString& rId) override;    /// ID used to RedlineAdd() call
    virtual void RedlineSetCursor(
        const OUString& rId,         /// ID used to RedlineAdd() call
        bool bStart,                    /// start or end Cursor
        bool bIsOutsideOfParagraph) override;
    virtual void RedlineAdjustStartNodeCursor( bool bStart) override;
    virtual void SetShowChanges( bool bShowChanges ) override;
    virtual void SetRecordChanges( bool bRecordChanges ) override;
    virtual void SetChangesProtectionKey(
        const ::com::sun::star::uno::Sequence<sal_Int8> & rKey ) override;
};

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
