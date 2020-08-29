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
#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLANNOI_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLANNOI_HXX

#include <xmloff/xmlictxt.hxx>
#include <rtl/ustrbuf.hxx>
#include <editeng/editdata.hxx>
#include "importcontext.hxx"

#include <vector>

class ScXMLImport;
enum class XmlStyleFamily;
namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::drawing { class XShapes; }

struct ScXMLAnnotationStyleEntry
{
    XmlStyleFamily       mnFamily;
    OUString             maName;
    ESelection           maSelection;

    ScXMLAnnotationStyleEntry( XmlStyleFamily nFam, const OUString& rNam, const ESelection& rSel ) :
        mnFamily( nFam ),
        maName( rNam ),
        maSelection( rSel )
    {
    }
};

struct ScXMLAnnotationData
{
    css::uno::Reference< css::drawing::XShape >
                        mxShape;
    css::uno::Reference< css::drawing::XShapes >
                        mxShapes;
    OUString     maAuthor;
    OUString     maCreateDate;
    OUString     maSimpleText;
    OUString     maStyleName;
    OUString     maTextStyle;
    bool                mbUseShapePos;
    bool                mbShown;
    std::vector<ScXMLAnnotationStyleEntry> maContentStyles;

    explicit            ScXMLAnnotationData();
                        ~ScXMLAnnotationData();
};

class ScXMLAnnotationContext : public ScXMLImportContext
{
public:

    ScXMLAnnotationContext( ScXMLImport& rImport, sal_Int32 nElement,
                        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
                        ScXMLAnnotationData& rAnnotationData);

    virtual ~ScXMLAnnotationContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    void SetShape(
        const css::uno::Reference< css::drawing::XShape >& rxShape,
        const css::uno::Reference< css::drawing::XShapes >& rxShapes,
        const OUString& rStyleName, const OUString& rTextStyle );

    void AddContentStyle( XmlStyleFamily nFamily, const OUString& rName, const ESelection& rSelection );

private:
    ScXMLAnnotationData& mrAnnotationData;
    OUStringBuffer maTextBuffer;
    OUStringBuffer maAuthorBuffer;
    OUStringBuffer maCreateDateBuffer;
    OUStringBuffer maCreateDateStringBuffer;
    std::unique_ptr<SvXMLImportContext> pShapeContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
