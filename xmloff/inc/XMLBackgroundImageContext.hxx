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

#ifndef INCLUDED_XMLOFF_INC_XMLBACKGROUNDIMAGECONTEXT_HXX
#define INCLUDED_XMLOFF_INC_XMLBACKGROUNDIMAGECONTEXT_HXX

#include <com/sun/star/style/GraphicLocation.hpp>
#include "XMLElementPropertyContext.hxx"

namespace com::sun::star {
    namespace io { class XOutputStream; }
}

class XMLBackgroundImageContext final : public XMLElementPropertyContext
{
    XMLPropertyState aPosProp;
    sal_Int32 m_nBitmapModeIdx;
    XMLPropertyState aFilterProp;
    XMLPropertyState aTransparencyProp;

    css::style::GraphicLocation ePos;
    OUString m_sURL;
    OUString sFilter;
    sal_Int8 nTransparency;

    css::uno::Reference < css::io::XOutputStream > m_xBase64Stream;

private:
    void ProcessAttrs(
            const css::uno::Reference<css::xml::sax::XFastAttributeList > & xAttrList );

public:

    XMLBackgroundImageContext(
        SvXMLImport& rImport,
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList > & xAttrList,
        const XMLPropertyState& rProp,
        sal_Int32 nPosIdx,
        sal_Int32 nFilterIdx,
        sal_Int32 nTransparencyIdx,
        sal_Int32 nBitmapModeIdx,
        ::std::vector< XMLPropertyState > &rProps );

    virtual ~XMLBackgroundImageContext() override;

    SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                const OUString& rLocalName,
                 const css::uno::Reference<css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
