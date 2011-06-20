/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef SW_VBA_INFORMATIONHELPER_HXX
#define SW_VBA_INFORMATIONHELPER_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

class SwVbaInformationHelper
{
public:
    static sal_Int32 handleWdActiveEndPageNumber( const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor ) throw( css::uno::RuntimeException );
    static sal_Int32 handleWdNumberOfPagesInDocument( const css::uno::Reference< css::frame::XModel >& xModel ) throw( css::uno::RuntimeException );
    static double handleWdVerticalPositionRelativeToPage( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor ) throw( css::uno::RuntimeException );
    //static double verticalPositionRelativeToPageBoundary( const css::uno::Reference< css::frame::XModel >& xModel, const css::uno::Reference< css::text::XTextViewCursor >& xTVCursor, const css::uno::Reference< css::beans::XPropertySet >& xStyleProps ) throw( css::uno::RuntimeException );

};
#endif /* SW_VBA_INFORMATIONHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
