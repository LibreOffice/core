/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbastyles.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:03:39 $
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
#ifndef SC_VBA_STYLES_HXX
#define SC_VBA_STYLES_HXX
#include <org/openoffice/excel/XStyles.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include "vbacollectionimpl.hxx"

typedef CollTestImplHelper< oo::excel::XStyles > ScVbaStyles_BASE;
class ScVbaStyles: public ScVbaStyles_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    // hard ref to parent ( perhaps we should try this in the
    // XHelperInterface itself
    css::uno::Reference< oo::vba::XHelperInterface > mxParent;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;
    css::uno::Reference< css::container::XNameContainer > mxNameContainerCellStyles;
public:
    ScVbaStyles( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xModel ) throw ( css::script::BasicErrorException );
    css::uno::Sequence< rtl::OUString > getStyleNames() throw ( css::uno::RuntimeException );
    void Delete(const rtl::OUString _sStyleName) throw ( css::script::BasicErrorException );
    // XStyles
     virtual css::uno::Reference< oo::excel::XStyle > SAL_CALL Add( const ::rtl::OUString& Name, const css::uno::Any& BasedOn ) throw (css::script::BasicErrorException, css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject(const css::uno::Any&);
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif
