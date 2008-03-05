/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richstringcontext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:07:18 $
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

    virtual ContextWrapper onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs );
    virtual void        onStartElement( const AttributeList& rAttribs );
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

