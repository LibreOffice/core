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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
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





class SmModel : public SfxBaseModel,
                public comphelper::PropertySetHelper,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::view::XRenderable,
                public oox::FormulaExportBase,
                public oox::FormulaImportBase
{
    std::unique_ptr<SmPrintUIOptions> m_pPrintUIOptions;
protected:
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const ::com::sun::star::uno::Any* pValues )
        throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, std::exception) override;
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, ::com::sun::star::uno::Any* pValue )
        throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::lang::WrappedTargetException, std::exception) override;
public:
    SmModel( SfxObjectShell *pObjSh = 0 );
    virtual ~SmModel() throw ();

    //XInterface
    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XRenderable
    virtual sal_Int32 SAL_CALL getRendererCount( const ::com::sun::star::uno::Any& rSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rxOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getRenderer( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& rSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rxOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL render( sal_Int32 nRenderer, const ::com::sun::star::uno::Any& rSelection, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rxOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xParent ) throw( ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // oox::FormulaExportBase
    virtual void writeFormulaOoxml( ::sax_fastparser::FSHelperPtr m_pSerializer, oox::core::OoxmlVersion version ) override;
    virtual void writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding) override;
    // oox::FormulaImportBase
    virtual void readFormulaOoxml( oox::formulaimport::XmlStream& stream ) override;
    virtual Size getFormulaSize() const override;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
