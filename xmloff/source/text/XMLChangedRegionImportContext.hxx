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


#ifndef _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX
#define _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace text {
        class XTextCursor;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }

/**
 * Import <text:changed-region> elements contained in a
 * <text:tracked-changes> element.
 */
class XMLChangedRegionImportContext : public SvXMLImportContext
{
    const OUString sEmpty;

    /// if we replace the current XTextCursor/XText by the ones for
    /// the redline, we remember the old cursor here.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextCursor> xOldCursor;

    /// redline-ID
    OUString sID;

    /// merge-last-paragraph flag
    sal_Bool bMergeLastPara;

public:

    TYPEINFO();

    XMLChangedRegionImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const OUString& rLocalName);

    ~XMLChangedRegionImportContext();

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
         const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();


    /// change info: To be called from change-info context
    void SetChangeInfo(const OUString& rType,
                       const OUString& rAuthor,
                       const OUString& rComment,
                       const OUString& rDate);

    /// create redline XText/XTextCursor on demand and register with
    /// XMLTextImportHelper
    void UseRedlineText();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
