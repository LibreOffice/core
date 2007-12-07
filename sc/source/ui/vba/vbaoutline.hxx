/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaoutline.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:57:27 $
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
#ifndef SC_VBA_OUTLINE_HXX
#define SC_VBA_OUTLINE_HXX

#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <cppuhelper/implbase1.hxx>
#include <org/openoffice/excel/XOutline.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "vbahelperinterface.hxx"

typedef InheritedHelperInterfaceImpl1< oo::excel::XOutline > ScVbaOutline_BASE;

class ScVbaOutline :  public ScVbaOutline_BASE
{
    css::uno::Reference< css::sheet::XSheetOutline > mxOutline;
public:
    ScVbaOutline( const css::uno::Reference< oo::vba::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,
         css::uno::Reference<css::sheet::XSheetOutline> outline): ScVbaOutline_BASE( xParent, xContext) , mxOutline(outline)
    {}
    virtual ~ScVbaOutline(){}

    virtual void SAL_CALL ShowLevels( const css::uno::Any& RowLevels, const css::uno::Any& ColumnLevels ) throw (css::uno::RuntimeException) ;
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif
