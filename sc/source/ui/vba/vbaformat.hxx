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
    css::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    css::uno::Reference< css::util::XNumberFormatsSupplier > mxNumberFormatsSupplier;
    css::uno::Reference< css::util::XNumberFormats > xNumberFormats;
    css::uno::Reference< css::util::XNumberFormatTypes > xNumberFormatTypes;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertyState > xPropertyState;
    bool mbCheckAmbiguoity;
    bool mbAddIndent;
    /// @throws css::script::BasicErrorException
    bool isAmbiguous(const OUString& _sPropertyName);
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::beans::XPropertyState > const &  getXPropertyState();
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    void initializeNumberFormats();
    /// @throws css::uno::RuntimeException
    SfxItemSet*  getCurrentDataSet( );
protected:
    /// @throws css::uno::RuntimeException
    virtual ScCellRangesBase* getCellRangesBase();
public:
    /// @throws css::script::BasicErrorException
    ScVbaFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& xModel, bool bCheckAmbiguoity );
    virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() = 0;
     /// @throws css::uno::RuntimeException
     void SAL_CALL setAddIndent( const css::uno::Any& BAddIndent) { BAddIndent >>= mbAddIndent; }
     /// @throws css::uno::RuntimeException
     css::uno::Any SAL_CALL getAddIndent() { return css::uno::makeAny( mbAddIndent ); }
        // Interface Methods
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& Index );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Reference< ::ooo::vba::excel::XFont > SAL_CALL Font(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Reference< ::ooo::vba::excel::XInterior > SAL_CALL Interior(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setNumberFormat( const css::uno::Any& NumberFormat );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getNumberFormat(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setNumberFormatLocal( const css::uno::Any& NumberFormatLocal );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getNumberFormatLocal(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setIndentLevel( const css::uno::Any& IndentLevel );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getIndentLevel(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setHorizontalAlignment( const css::uno::Any& HorizontalAlignment );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getHorizontalAlignment(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setVerticalAlignment( const css::uno::Any& VerticalAlignment );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getVerticalAlignment(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setOrientation( const css::uno::Any& Orientation );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getOrientation(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setShrinkToFit( const css::uno::Any& ShrinkToFit );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getShrinkToFit(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setWrapText( const css::uno::Any& WrapText );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getWrapText(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setLocked( const css::uno::Any& Locked );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getLocked(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setFormulaHidden( const css::uno::Any& FormulaHidden );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getFormulaHidden(  );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) = 0;
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getMergeCells(  ) = 0;
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual void SAL_CALL setReadingOrder( const css::uno::Any& ReadingOrder );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL getReadingOrder(  );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
