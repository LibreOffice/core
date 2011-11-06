/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef SC_VBA_STYLE_HXX
#define SC_VBA_STYLE_HXX
#include <ooo/vba/excel/XStyle.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "vbaformat.hxx"


typedef ScVbaFormat< ov::excel::XStyle > ScVbaStyle_BASE;

class ScVbaStyle : public ScVbaStyle_BASE
{
protected:
    css::uno::Reference< css::style::XStyle > mxStyle;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::container::XNameContainer > mxStyleFamilyNameContainer;
    void initialise() throw ( css::uno::RuntimeException );
public:
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const rtl::OUString& sStyleName, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    ScVbaStyle( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual ~ScVbaStyle(){}
    static css::uno::Reference< css::container::XNameAccess > getStylesNameContainer( const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< ov::XHelperInterface > thisHelperIface() { return this; };
    // XStyle Methods
    virtual ::sal_Bool SAL_CALL BuiltIn() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setName( const ::rtl::OUString& Name ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getName() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL setNameLocal( const ::rtl::OUString& NameLocal ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameLocal() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual void SAL_CALL Delete() throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XFormat
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getMergeCells(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
/*
    // XFormat
    virtual css::uno::Reference< ::ooo::vba::excel::XBorders > SAL_CALL Borders(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Reference< ::ooo::vba::excel::XFont > SAL_CALL Font(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Reference< ::ooo::vba::excel::XInterior > SAL_CALL Interior(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setNumberFormat( const css::uno::Any& NumberFormat ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getNumberFormat(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setNumberFormatLocal( const css::uno::Any& NumberFormatLocal ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getNumberFormatLocal(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setIndentLevel( const css::uno::Any& IndentLevel ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getIndentLevel(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setHorizontalAlignment( const css::uno::Any& HorizontalAlignment ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getHorizontalAlignment(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setVerticalAlignment( const css::uno::Any& VerticalAlignment ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getVerticalAlignment(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setOrientation( const css::uno::Any& Orientation ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getOrientation(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setShrinkToFit( const css::uno::Any& ShrinkToFit ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getShrinkToFit(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setWrapText( const css::uno::Any& WrapText ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getWrapText(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setLocked( const css::uno::Any& Locked ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getLocked(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setFormulaHidden( const css::uno::Any& FormulaHidden ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
/   virtual css::uno::Any SAL_CALL getFormulaHidden(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual void SAL_CALL setMergeCells( const css::uno::Any& MergeCells ) throw (css::script::BasicErrorException, css::uno::RuntimeException) = 0;
    virtual css::uno::Any SAL_CALL getMergeCells(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException) = 0;
    virtual void SAL_CALL setReadingOrder( const css::uno::Any& ReadingOrder ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Any SAL_CALL getReadingOrder(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
*/
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif //SC_VBA_AXIS_HXX
