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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAFORMAT_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAFORMAT_HXX
#include <ooo/vba/excel/XFormat.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <svl/itemset.hxx>
#include <vbahelper/vbahelperinterface.hxx>

class ScCellRangesBase;

template< typename... Ifc >
class ScVbaFormat : public InheritedHelperInterfaceWeakImpl< Ifc... >
{
typedef InheritedHelperInterfaceWeakImpl< Ifc... > ScVbaFormat_BASE;
    css::lang::Locale m_aDefaultLocale;
protected:
    const css::lang::Locale& getDefaultLocale() { return m_aDefaultLocale; }
    css::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    css::uno::Reference< css::util::XNumberFormatsSupplier > mxNumberFormatsSupplier;
    css::uno::Reference< css::util::XNumberFormats > xNumberFormats;
    css::uno::Reference< css::util::XNumberFormatTypes > xNumberFormatTypes;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertyState > xPropertyState;
    bool mbCheckAmbiguoity;
    bool mbAddIndent;
    bool isAmbiguous(const OUString& _sPropertyName) throw ( css::script::BasicErrorException );
    css::uno::Reference< css::beans::XPropertyState > const &  getXPropertyState() throw ( css::uno::RuntimeException );
    void initializeNumberFormats() throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    SfxItemSet*  getCurrentDataSet( ) throw (css::uno::RuntimeException, std::exception);
protected:
    virtual ScCellRangesBase* getCellRangesBase() throw ( css::uno::RuntimeException );
public:
    ScVbaFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& xModel, bool bCheckAmbiguoity ) throw ( css::script::BasicErrorException );
    virtual ~ScVbaFormat() {}
    virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() = 0;
     void SAL_CALL setAddIndent( const css::uno::Any& BAddIndent) throw( css::uno::RuntimeException ) { BAddIndent >>= mbAddIndent; }
     css::uno::Any SAL_CALL getAddIndent() throw( css::uno::RuntimeException ) { return css::uno::makeAny( mbAddIndent ); }
        // Interface Methods
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& Index ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::excel::XFont > SAL_CALL Font(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::excel::XInterior > SAL_CALL Interior(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setNumberFormat( const css::uno::Any& NumberFormat ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getNumberFormat(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setNumberFormatLocal( const css::uno::Any& NumberFormatLocal ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getNumberFormatLocal(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setIndentLevel( const css::uno::Any& IndentLevel ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getIndentLevel(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setHorizontalAlignment( const css::uno::Any& HorizontalAlignment ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getHorizontalAlignment(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setVerticalAlignment( const css::uno::Any& VerticalAlignment ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getVerticalAlignment(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setOrientation( const css::uno::Any& Orientation ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getOrientation(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setShrinkToFit( const css::uno::Any& ShrinkToFit ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getShrinkToFit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setWrapText( const css::uno::Any& WrapText ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getWrapText(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setLocked( const css::uno::Any& Locked ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getLocked(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setFormulaHidden( const css::uno::Any& FormulaHidden ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFormulaHidden(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) throw (css::script::BasicErrorException, css::uno::RuntimeException) = 0;
    virtual css::uno::Any SAL_CALL getMergeCells(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) = 0;
    virtual void SAL_CALL setReadingOrder( const css::uno::Any& ReadingOrder ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getReadingOrder(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
