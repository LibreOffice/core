/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbastyle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:03:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_VBA_STYLE_HXX
#define SC_VBA_STYLE_HXX
#include <org/openoffice/excel/XStyle.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "vbaformat.hxx"


typedef ScVbaFormat< oo::excel::XStyle > ScVbaStyle_BASE;

class ScVbaStyle : public ScVbaStyle_BASE
{
protected:
    css::uno::Reference< css::style::XStyle > mxStyle;
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::container::XNameContainer > mxStyleFamilyNameContainer;
    void initialise() throw ( css::uno::RuntimeException );
public:
    ScVbaStyle( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const rtl::OUString& sStyleName, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    ScVbaStyle( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& _xPropertySet, const css::uno::Reference< css::frame::XModel >& _xModel ) throw ( css::script::BasicErrorException, css::uno::RuntimeException );
    virtual ~ScVbaStyle(){}
    static css::uno::Reference< css::container::XNameAccess > getStylesNameContainer( const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException );
    virtual css::uno::Reference< oo::vba::XHelperInterface > thisHelperIface() { return this; };
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
    virtual css::uno::Reference< ::org::openoffice::excel::XBorders > SAL_CALL Borders(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Reference< ::org::openoffice::excel::XFont > SAL_CALL Font(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
    virtual css::uno::Reference< ::org::openoffice::excel::XInterior > SAL_CALL Interior(  ) throw (css::script::BasicErrorException, css::uno::RuntimeException);;
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
