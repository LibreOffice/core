/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: richstringcontext.cxx,v $
 * $Revision: 1.4 $
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

#include "oox/xls/richstringcontext.hxx"
#include "oox/xls/stylesfragment.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace xls {

// ============================================================================

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxRichStringContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() )
    {
        switch( nElement )
        {
            case XLS_TOKEN( t ):            mxPortion = mxString->importText( rAttribs );           return this;    // collect text in onEndElement()
            case XLS_TOKEN( r ):            mxPortion = mxString->importRun( rAttribs );            return this;
            case XLS_TOKEN( rPh ):          mxPhonetic = mxString->importPhoneticRun( rAttribs );   return this;
            case XLS_TOKEN( phoneticPr ):   mxString->importPhoneticPr( rAttribs );                 break;
        }
    }
    else switch( getCurrentElement() )
    {
        case XLS_TOKEN( r ):
            switch( nElement )
            {
                case XLS_TOKEN( rPr ):
                    if( mxPortion.get() )
                        return new OoxFontContext( *this, mxPortion->createFont() );
                break;

                case XLS_TOKEN( t ):
                    return this;    // collect portion text in onEndElement()
            }
        break;

        case XLS_TOKEN( rPh ):
            switch( nElement )
            {
                case XLS_TOKEN( t ):
                    return this;    // collect phonetic text in onEndElement()
            }
        break;
    }
    return 0;
}

void OoxRichStringContext::onEndElement( const OUString& rChars )
{
    if( getCurrentElement() == XLS_TOKEN( t ) )
    {
        switch( getPreviousElement() )
        {
            case XLS_TOKEN( rPh ):  if( mxPhonetic.get() ) mxPhonetic->setText( rChars );   break;
            default:                if( mxPortion.get() ) mxPortion->setText( rChars );     break;
        }
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

