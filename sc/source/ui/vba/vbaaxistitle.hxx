/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbaaxistitle.hxx,v $
 * $Revision: 1.3 $
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
#ifndef SC_VBA_AXISTITLE_HXX
#define SC_VBA_AXISTITLE_HXX

#include "vbatitle.hxx"
#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XAxisTitle.hpp>

typedef  TitleImpl< cppu::WeakImplHelper1< ov::excel::XAxisTitle > > AxisTitleBase;

class ScVbaAxisTitle : public AxisTitleBase
{
public:
        ScVbaAxisTitle( const css::uno::Reference< ov::XHelperInterface >& xParent,   const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::drawing::XShape >& _xTitleShape );
        // XHelperInterface
        virtual rtl::OUString& getServiceImplName();
        virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif
