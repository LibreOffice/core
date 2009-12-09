/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#ifndef SW_VBA_RANGEHELPER_HXX
#define SW_VBA_RANGEHELPER_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XText.hpp>

class SwVbaRangeHelper
{
public:
    static css::uno::Reference< css::text::XTextRange > getRangeByPosition( const css::uno::Reference< css::text::XText >& rText, sal_Int32 _position )throw ( css::uno::RuntimeException );
    static void insertString( css::uno::Reference< css::text::XTextRange >& rTextRange, css::uno::Reference< css::text::XText >& rText, const rtl::OUString& rStr, sal_Bool _bAbsorb ) throw ( css::uno::RuntimeException );
    static css::uno::Reference< css::text::XTextCursor > initCursor( const css::uno::Reference< css::text::XTextRange >& rTextRange, const css::uno::Reference< css::text::XText >& rText ) throw ( css::uno::RuntimeException );
    static sal_Int32 getPosition( const css::uno::Reference< css::text::XText >& rText, const css::uno::Reference< css::text::XTextRange >& rTextRange ) throw ( css::uno::RuntimeException );

};
#endif /* SW_VBA_RANGEHELPER_HXX */
