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

#ifndef INCLUDED_XMLOFF_XMLMETAI_HXX
#define INCLUDED_XMLOFF_XMLMETAI_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <xmloff/xmlictxt.hxx>


namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace document { class XDocumentProperties; } } } }
namespace com { namespace sun { namespace star { namespace xml { namespace dom { class XSAXDocumentBuilder2; } } } } }

/// handles the top-level office:document-meta element of meta.xml documents
// NB: virtual inheritance is needed so that the context that handles the
//     flat xml file format can multiply inherit properly
class XMLOFF_DLLPUBLIC SvXMLMetaDocumentContext
    : public virtual SvXMLImportContext
{
private:
    css::uno::Reference< css::document::XDocumentProperties> mxDocProps;
    css::uno::Reference< css::xml::dom::XSAXDocumentBuilder2> mxDocBuilder;

public:
    SvXMLMetaDocumentContext(SvXMLImport& rImport,
        const css::uno::Reference< css::document::XDocumentProperties>& xDocProps);

    virtual ~SvXMLMetaDocumentContext() override;

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

public:
    static void setBuildId(const OUString & rGenerator,
        const css::uno::Reference< css::beans::XPropertySet>& xImportInfo );
};

#endif // _ INCLUDED_XMLOFF_XMLMETAI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
