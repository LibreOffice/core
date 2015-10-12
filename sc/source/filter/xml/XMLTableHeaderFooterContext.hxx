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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLTABLEHEADERFOOTERCONTEXT_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLTABLEHEADERFOOTERCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <com/sun/star/sheet/XHeaderFooterContent.hpp>

namespace com { namespace sun { namespace star {
    namespace text { class XTextCursor; }
    namespace beans { class XPropertySet; }
} } }

class XMLTableHeaderFooterContext: public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > xPropSet;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::sheet::XHeaderFooterContent > xHeaderFooterContent;

    const OUString   sOn;
    const OUString   sShareContent;
    const OUString   sContent;
    const OUString   sContentLeft;
    OUString           sCont;

    bool    bDisplay;
    bool    bLeft;
    bool    bContainsLeft;
    bool    bContainsRight;
    bool    bContainsCenter;

public:
    TYPEINFO_OVERRIDE();

    XMLTableHeaderFooterContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const ::com::sun::star::uno::Reference <
                       ::com::sun::star::beans::XPropertySet > & rPageStylePropSet,
               bool bFooter, bool bLft );

    virtual ~XMLTableHeaderFooterContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void EndElement() override;
};

class XMLHeaderFooterRegionContext: public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor >& xTextCursor;
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextCursor > xOldTextCursor;

public:
    TYPEINFO_OVERRIDE();

    XMLHeaderFooterRegionContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            com::sun::star::uno::Reference< com::sun::star::text::XTextCursor >& xCursor );

    virtual ~XMLHeaderFooterRegionContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
