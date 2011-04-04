/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SC_VBA_FORMAT_HXX
#define SC_VBA_FORMAT_HXX
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
#include <vbahelper/vbahelperinterface.hxx>

class ScCellRangesBase;

template< typename Ifc1 >
class ScVbaFormat : public InheritedHelperInterfaceImpl1< Ifc1 >
{
typedef InheritedHelperInterfaceImpl1< Ifc1 > ScVbaFormat_BASE;
    css::lang::Locale m_aDefaultLocale;
protected:
    css::lang::Locale getDefaultLocale() { return m_aDefaultLocale; }
    css::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    css::uno::Reference< css::util::XNumberFormatsSupplier > mxNumberFormatsSupplier;
    css::uno::Reference< css::util::XNumberFormats > xNumberFormats;
    css::uno::Reference< css::util::XNumberFormatTypes > xNumberFormatTypes;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::beans::XPropertyState > xPropertyState;
    sal_Bool mbCheckAmbiguoity;
    sal_Bool mbAddIndent;
    //NumberFormatter oNumberFormatter = null;
    css::uno::Reference< css::lang::XMultiServiceFactory > xMultiServiceFactory;
    bool isAmbiguous(const rtl::OUString& _sPropertyName) throw ( css::script::BasicErrorException );
    css::uno::Reference< css::beans::XPropertyState > getXPropertyState() throw ( css::uno::RuntimeException );
    void initializeNumberFormats() throw ( css::script::BasicErrorException );
    void setNumberFormat( css::lang::Locale _aLocale, const rtl::OUString& _sFormatString) throw( css::script::BasicErrorException );
    SfxItemSet*  getCurrentDataSet( ) throw ( css::uno::RuntimeException );
protected:
    virtual ScCellRangesBase* getCellRangesBase() throw ( css::uno::RuntimeException );
public:
    ScVbaFormat( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& xModel, bool bCheckAmbiguoity ) throw ( css::script::BasicErrorException );
    virtual ~ScVbaFormat() {}
    virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() = 0;
     void SAL_CALL setAddIndent( const css::uno::Any& _BAddIndent) throw( css::uno::RuntimeException ) { _BAddIndent >>= mbAddIndent; }
     css::uno::Any SAL_CALL getAddIndent() throw( css::uno::RuntimeException ) { return css::uno::makeAny( mbAddIndent ); }
        // Interface Methods
    virtual css::uno::Any SAL_CALL Borders( const css::uno::Any& Index ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::excel::XFont > SAL_CALL Font(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::excel::XInterior > SAL_CALL Interior(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setNumberFormat( const css::uno::Any& NumberFormat ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getNumberFormat(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
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
    virtual css::uno::Any SAL_CALL getWrapText(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setLocked( const css::uno::Any& Locked ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getLocked(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setFormulaHidden( const css::uno::Any& FormulaHidden ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFormulaHidden(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) throw (css::script::BasicErrorException, css::uno::RuntimeException) = 0;
    virtual css::uno::Any SAL_CALL getMergeCells(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) = 0;
    virtual void SAL_CALL setReadingOrder( const css::uno::Any& ReadingOrder ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getReadingOrder(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif
