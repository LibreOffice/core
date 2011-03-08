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
#ifndef _TEXTMARKUPHELPER_HXX_
#define _TEXTMARKUPHELPER_HXX_

#include <sal/types.h>
#include "com/sun/star/lang/IndexOutOfBoundsException.hdl"
#include "com/sun/star/lang/IllegalArgumentException.hdl"
#include "com/sun/star/uno/RuntimeException.hdl"
#include "com/sun/star/uno/Sequence.h"

namespace com { namespace sun { namespace star { namespace accessibility {
struct TextSegment;
} } } }

class SwAccessiblePortionData;
class SwTxtNode;
// --> OD 2010-02-19 #i108125#
class SwWrongList;
// <--

class SwTextMarkupHelper
{
    public:
        SwTextMarkupHelper( const SwAccessiblePortionData& rPortionData,
                            const SwTxtNode& rTxtNode );
        // --> OD 2010-02-19 #i108125#
        SwTextMarkupHelper( const SwAccessiblePortionData& rPortionData,
                            const SwWrongList& rTextMarkupList );
        // <--
        ~SwTextMarkupHelper() {}

        sal_Int32 getTextMarkupCount( const sal_Int32 nTextMarkupType )
                throw (::com::sun::star::lang::IllegalArgumentException,
                       ::com::sun::star::uno::RuntimeException);

        ::com::sun::star::accessibility::TextSegment getTextMarkup(
                                            const sal_Int32 nTextMarkupIndex,
                                            const sal_Int32 nTextMarkupType )
                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                       ::com::sun::star::lang::IllegalArgumentException,
                       ::com::sun::star::uno::RuntimeException);

        ::com::sun::star::uno::Sequence< ::com::sun::star::accessibility::TextSegment >
                getTextMarkupAtIndex( const sal_Int32 nCharIndex,
                                      const sal_Int32 nTextMarkupType )
                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                       ::com::sun::star::lang::IllegalArgumentException,
                       ::com::sun::star::uno::RuntimeException);

    private:
        SwTextMarkupHelper( const SwTextMarkupHelper& );
        SwTextMarkupHelper& operator=( const SwTextMarkupHelper& );

        const SwAccessiblePortionData& mrPortionData;

        // --> OD 2010-02-19 #i108125#
        const SwTxtNode* mpTxtNode;
        const SwWrongList* mpTextMarkupList;
        // <--
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
