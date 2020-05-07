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
#ifndef INCLUDED_STARMATH_INC_UNOMODEL_HXX
#define INCLUDED_STARMATH_INC_UNOMODEL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/view/XRenderable.hpp>

#include <sfx2/sfxbasemodel.hxx>
#include <comphelper/propertysethelper.hxx>
#include <vcl/print.hxx>
#include <oox/mathml/export.hxx>
#include <oox/mathml/import.hxx>
#include <memory>


#define PRTUIOPT_TITLE_ROW          "TitleRow"
#define PRTUIOPT_FORMULA_TEXT       "FormulaText"
#define PRTUIOPT_BORDER             "Border"
#define PRTUIOPT_PRINT_FORMAT       "PrintFormat"
#define PRTUIOPT_PRINT_SCALE        "PrintScale"

class SmPrintUIOptions : public vcl::PrinterOptionsHelper
{
public:
    SmPrintUIOptions();
};


class SmModel final : public SfxBaseModel,
                public comphelper::PropertySetHelper,
                public css::lang::XServiceInfo,
                public css::view::XRenderable,
                public oox::FormulaExportBase,
                public oox::FormulaImportBase
{
    std::unique_ptr<SmPrintUIOptions> m_pPrintUIOptions;

    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const css::uno::Any* pValues ) override;
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, css::uno::Any* pValue ) override;
public:
    explicit SmModel( SfxObjectShell *pObjSh );
    virtual ~SmModel() throw () override;

    //XInterface
    virtual     css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const css::uno::Any& rSelection, const css::uno::Sequence< css::beans::PropertyValue >& rxOptions ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const css::uno::Any& rSelection, const css::uno::Sequence< css::beans::PropertyValue >& rxOptions ) override;
    virtual void SAL_CALL render( sal_Int32 nRenderer, const css::uno::Any& rSelection, const css::uno::Sequence< css::beans::PropertyValue >& rxOptions ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    virtual void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface >& xParent ) override;

    // oox::FormulaExportBase
    virtual void writeFormulaOoxml(::sax_fastparser::FSHelperPtr pSerializer,
            oox::core::OoxmlVersion version,
            oox::drawingml::DocumentType documentType, sal_Int8 nAlign) override;
    virtual void writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding) override;
    // oox::FormulaImportBase
    virtual void readFormulaOoxml( oox::formulaimport::XmlStream& stream ) override;
    virtual Size getFormulaSize() const override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
