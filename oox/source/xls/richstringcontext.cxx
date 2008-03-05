/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richstringcontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:05:13 $
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

#include "oox/xls/richstringcontext.hxx"

using ::rtl::OUString;

namespace oox {
namespace xls {

// ============================================================================

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxRichStringContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( si ):
        case XLS_TOKEN( is ):
        case XLS_TOKEN( text ):
            return  (nElement == XLS_TOKEN( t )) ||
                    (nElement == XLS_TOKEN( r )) ||
                    (nElement == XLS_TOKEN( rPh )) ||
                    (nElement == XLS_TOKEN( phoneticPr ));
        case XLS_TOKEN( r ):
            return  (nElement == XLS_TOKEN( rPr )) ||
                    (nElement == XLS_TOKEN( t ));
        case XLS_TOKEN( rPh ):
            return  (nElement == XLS_TOKEN( t ));
        case XLS_TOKEN( rPr ):
            return  Font::isSupportedContext( nElement, getCurrentElement() );
    }
    return false;
}

void OoxRichStringContext::onStartElement( const AttributeList& rAttribs )
{
    sal_Int32 nCurrContext = getCurrentElement();
    switch( nCurrContext )
    {
        case XLS_TOKEN( t ):
            if( !isPreviousElement( XLS_TOKEN( r ) ) && !isPreviousElement( XLS_TOKEN( rPh ) ) )
                mxPortion = mxString->importText( rAttribs );
        break;
        case XLS_TOKEN( r ):
            mxPortion = mxString->importRun( rAttribs );
        break;
        case XLS_TOKEN( rPr ):
            if( mxPortion.get() ) mxFont = mxPortion->importFont( rAttribs );
        break;
        case XLS_TOKEN( rPh ):
            mxPhonetic = mxString->importPhoneticRun( rAttribs );
        break;
        case XLS_TOKEN( phoneticPr ):
            mxString->importPhoneticPr( rAttribs );
        break;
        default:
            if( isPreviousElement( XLS_TOKEN( rPr ) ) && mxFont.get() )
                mxFont->importAttribs( nCurrContext, rAttribs );
    }
}

void OoxRichStringContext::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( t ):
            switch( getPreviousElement() )
            {
                case XLS_TOKEN( rPh ):
                    if( mxPhonetic.get() ) mxPhonetic->setText( rChars );
                break;
                default:
                    if( mxPortion.get() ) mxPortion->setText( rChars );
            }
        break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

