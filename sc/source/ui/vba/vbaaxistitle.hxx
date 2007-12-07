/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaaxistitle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:44:24 $
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
#ifndef SC_VBA_AXISTITLE_HXX
#define SC_VBA_AXISTITLE_HXX

#include "vbatitle.hxx"
#include <cppuhelper/implbase1.hxx>
#include <org/openoffice/excel/XAxisTitle.hpp>

typedef  TitleImpl< cppu::WeakImplHelper1< oo::excel::XAxisTitle > > AxisTitleBase;

class ScVbaAxisTitle : public AxisTitleBase
{
public:
        ScVbaAxisTitle( const css::uno::Reference< oo::vba::XHelperInterface >& xParent,   const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& _xTitleShape );
        // XHelperInterface
        virtual rtl::OUString& getServiceImplName();
        virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif
