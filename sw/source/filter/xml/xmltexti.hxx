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
#ifndef _XMLTEXTI_HXX
#define _XMLTEXTI_HXX

#include <xmloff/txtimp.hxx>

class XMLRedlineImportHelper;
class SvXMLImport;

class SwXMLTextImportHelper : public XMLTextImportHelper
{
    XMLRedlineImportHelper *pRedlineHelper;

protected:
    virtual SvXMLImportContext *CreateTableChildContext(
                SvXMLImport& rImport,
                sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

public:
    SwXMLTextImportHelper(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::frame::XModel>& rModel,
            SvXMLImport& rImport,
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet>& rInfoSet,
            bool bInsertM, bool bStylesOnlyM, bool bProgress,
            bool bBlockM, bool bOrganizerM,
            sal_Bool bPreserveRedlineMode );
    ~SwXMLTextImportHelper();

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
            createAndInsertOLEObject( SvXMLImport& rImport,
                                      const ::rtl::OUString& rHRef,
                                      const ::rtl::OUString& rStyleName,
                                      const ::rtl::OUString& rTblName,
                                         sal_Int32 nWidth, sal_Int32 nHeight );
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
            createAndInsertOOoLink( SvXMLImport& rImport,
                                      const ::rtl::OUString& rHRef,
                                      const ::rtl::OUString& rStyleName,
                                      const ::rtl::OUString& rTblName,
                                         sal_Int32 nWidth, sal_Int32 nHeight );
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertApplet(
            const ::rtl::OUString &rName,
            const ::rtl::OUString &rCode,
            sal_Bool bMayScript,
            const ::rtl::OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertPlugin(
            const ::rtl::OUString &rMimeType,
            const ::rtl::OUString& rHRef,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet>
        createAndInsertFloatingFrame(
            const ::rtl::OUString &rName,
            const ::rtl::OUString &rHRef,
            const ::rtl::OUString &rStyleName,
            sal_Int32 nWidth, sal_Int32 nHeight );

    virtual void endAppletOrPlugin(
        const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > &rPropSet,
        ::std::map < const ::rtl::OUString, ::rtl::OUString, ::comphelper::UStringLess> &rParamMap);

    virtual sal_Bool IsInHeaderFooter() const;

    // redlining helper methods
    // (here is the real implementation)
    virtual void RedlineAdd(
        const ::rtl::OUString& rType,       /// redline type (insert, del,... )
        const ::rtl::OUString& rId,         /// use to identify this redline
        const ::rtl::OUString& rAuthor,     /// name of the author
        const ::rtl::OUString& rComment,    /// redline comment
        const ::com::sun::star::util::DateTime& rDateTime,  /// date+time
        sal_Bool bMergeLastPara);           /// merge last paragraph
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextCursor> RedlineCreateText(
            ::com::sun::star::uno::Reference<   /// needed to get the document
                    ::com::sun::star::text::XTextCursor> & rOldCursor,
            const ::rtl::OUString& rId);    /// ID used to RedlineAdd() call
    virtual void RedlineSetCursor(
        const ::rtl::OUString& rId,         /// ID used to RedlineAdd() call
        sal_Bool bStart,                    /// start or end Cursor
        sal_Bool bIsOutsideOfParagraph);
    virtual void RedlineAdjustStartNodeCursor(
        sal_Bool bStart);
    virtual void SetShowChanges( sal_Bool bShowChanges );
    virtual void SetRecordChanges( sal_Bool bRecordChanges );
    virtual void SetChangesProtectionKey(
        const ::com::sun::star::uno::Sequence<sal_Int8> & rKey );
};

#endif  //  _XMLTEXTI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
