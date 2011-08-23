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

#ifndef OOX_XLS_RICHSTRINGCONTEXT_HXX
#define OOX_XLS_RICHSTRINGCONTEXT_HXX

#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/richstring.hxx"

namespace oox {
namespace xls {

// ============================================================================

class OoxRichStringContext : public OoxWorkbookContextBase
{
public:
    template< typename ParentType >
    explicit            OoxRichStringContext( ParentType& rParent, RichStringRef xString );

protected:
    // oox.core.ContextHandler2Helper interface -------------------------------

    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onEndElement( const ::rtl::OUString& rChars );

private:
    RichStringRef       mxString;       /// Processed string.
    RichStringPortionRef mxPortion;     /// Processed portion in the string.
    RichStringPhoneticRef mxPhonetic;   /// Processed phonetic text portion.
    FontRef             mxFont;         /// Processed font of the portion.
};

// ----------------------------------------------------------------------------

template< typename ParentType >
OoxRichStringContext::OoxRichStringContext( ParentType& rParent, RichStringRef xString ) :
    OoxWorkbookContextBase( rParent ),
    mxString( xString )
{
    OSL_ENSURE( mxString.get(), "OoxRichStringContext::OoxRichStringContext - missing string object" );
}

// ============================================================================

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
