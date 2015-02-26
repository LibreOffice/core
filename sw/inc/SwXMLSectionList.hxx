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
#ifndef INCLUDED_SW_INC_SWXMLSECTIONLIST_HXX
#define INCLUDED_SW_INC_SWXMLSECTIONLIST_HXX

#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <sax/fastattribs.hxx>

using namespace css::xml::sax;
using namespace xmloff::token;

class SwXMLSectionList : public SvXMLImport
{
protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateFastContext( sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) SAL_OVERRIDE;
public:
    std::vector<OUString*> &rSectionList;

    SwXMLSectionList(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rContext,
        std::vector<OUString*> &rNewSectionList );

    virtual ~SwXMLSectionList ( )
        throw();
};

class SvXMLSectionListContext : public SvXMLImportContext
{
private:
    SwXMLSectionList & rLocalRef;
public:
    SvXMLSectionListContext ( SwXMLSectionList& rImport, sal_Int32 Element,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList > & xAttrList );
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList )
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) SAL_OVERRIDE;
    virtual ~SvXMLSectionListContext ( void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
