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
#ifndef SC_XMLANNOI_HXX
#define SC_XMLANNOI_HXX

#include <memory>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <editeng/editdata.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

class ScXMLImport;
class ScXMLTableRowCellContext;

struct ScXMLAnnotationStyleEntry
{
    sal_uInt16          mnFamily;
    rtl::OUString       maName;
    ESelection          maSelection;

    ScXMLAnnotationStyleEntry( sal_uInt16 nFam, const rtl::OUString& rNam, const ESelection& rSel ) :
        mnFamily( nFam ),
        maName( rNam ),
        maSelection( rSel )
    {
    }
};

struct ScXMLAnnotationData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        mxShape;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
                        mxShapes;
    ::rtl::OUString     maAuthor;
    ::rtl::OUString     maCreateDate;
    ::rtl::OUString     maSimpleText;
    ::rtl::OUString     maStyleName;
    ::rtl::OUString     maTextStyle;
    bool                mbUseShapePos;
    bool                mbShown;
    std::vector<ScXMLAnnotationStyleEntry> maContentStyles;

    explicit            ScXMLAnnotationData();
                        ~ScXMLAnnotationData();
};

class ScXMLAnnotationContext : public SvXMLImportContext
{
public:

    ScXMLAnnotationContext( ScXMLImport& rImport, sal_uInt16 nPrfx,
                        const ::rtl::OUString& rLName,
                        const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
                        ScXMLAnnotationData& rAnnotationData,
                        ScXMLTableRowCellContext* pCellContext);

    virtual ~ScXMLAnnotationContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList);

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();

    void SetShape(
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
        const ::rtl::OUString& rStyleName, const ::rtl::OUString& rTextStyle );

    void AddContentStyle( sal_uInt16 nFamily, const rtl::OUString& rName, const ESelection& rSelection );

private:
    ScXMLAnnotationData& mrAnnotationData;
    rtl::OUStringBuffer maTextBuffer;
    rtl::OUStringBuffer maAuthorBuffer;
    rtl::OUStringBuffer maCreateDateBuffer;
    rtl::OUStringBuffer maCreateDateStringBuffer;
    ScXMLTableRowCellContext* pCellContext;
    SvXMLImportContext* pShapeContext;

    const ScXMLImport& GetScImport() const { return (const ScXMLImport&)GetImport(); }
    ScXMLImport& GetScImport() { return (ScXMLImport&)GetImport(); }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
