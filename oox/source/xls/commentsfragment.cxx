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

#include "oox/xls/commentsfragment.hxx"
#include "oox/xls/richstringcontext.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;
using ::oox::core::RecordInfo;

namespace oox {
namespace xls {

// ============================================================================

OoxCommentsFragment::OoxCommentsFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath )
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxCommentsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XLS_TOKEN( comments ) ) return this;
        break;
        case XLS_TOKEN( comments ):
            if( nElement == XLS_TOKEN( authors ) ) return this;
            if( nElement == XLS_TOKEN( commentList ) ) return this;
        break;
        case XLS_TOKEN( authors ):
            if( nElement == XLS_TOKEN( author ) ) return this;  // collect author in onEndElement()
        break;
        case XLS_TOKEN( commentList ):
            if( nElement == XLS_TOKEN( comment ) ) { importComment( rAttribs ); return this; }
        break;
        case XLS_TOKEN( comment ):
            if( (nElement == XLS_TOKEN( text )) && mxComment.get() )
                return new OoxRichStringContext( *this, mxComment->createText() );
        break;
    }
    return 0;
}

void OoxCommentsFragment::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XLS_TOKEN( author ):
            getComments().appendAuthor( rChars );
        break;
        case XLS_TOKEN( comment ):
            mxComment.reset();
        break;
    }
}

ContextHandlerRef OoxCommentsFragment::onCreateRecordContext( sal_Int32 nRecId, RecordInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == OOBIN_ID_COMMENTS ) return this;
        break;
        case OOBIN_ID_COMMENTS:
            if( nRecId == OOBIN_ID_COMMENTAUTHORS ) return this;
            if( nRecId == OOBIN_ID_COMMENTLIST ) return this;
        break;
        case OOBIN_ID_COMMENTAUTHORS:
            if( nRecId == OOBIN_ID_COMMENTAUTHOR ) getComments().appendAuthor( rStrm.readString() );
        break;
        case OOBIN_ID_COMMENTLIST:
            if( nRecId == OOBIN_ID_COMMENT ) { importComment( rStrm ); return this; }
        break;
        case OOBIN_ID_COMMENT:
            if( (nRecId == OOBIN_ID_COMMENTTEXT) && mxComment.get() )
                mxComment->createText()->importString( rStrm, true );
        break;
    }
    return 0;
}

void OoxCommentsFragment::onEndRecord()
{
    switch( getCurrentElement() )
    {
        case OOBIN_ID_COMMENT:
            mxComment.reset();
        break;
    }
}

// oox.core.FragmentHandler2 interface ----------------------------------------

const RecordInfo* OoxCommentsFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { OOBIN_ID_COMMENT,         OOBIN_ID_COMMENT + 1        },
        { OOBIN_ID_COMMENTAUTHORS,  OOBIN_ID_COMMENTAUTHORS + 1 },
        { OOBIN_ID_COMMENTLIST,     OOBIN_ID_COMMENTLIST + 1    },
        { OOBIN_ID_COMMENTS,        OOBIN_ID_COMMENTS + 1       },
        { -1,                       -1                          }
    };
    return spRecInfos;
}

// private --------------------------------------------------------------------

void OoxCommentsFragment::importComment( const AttributeList& rAttribs )
{
    mxComment = getComments().createComment();
    mxComment->importComment( rAttribs );
}

void OoxCommentsFragment::importComment( RecordInputStream& rStrm )
{
    mxComment = getComments().createComment();
    mxComment->importComment( rStrm );
}

// ============================================================================

} // namespace xls
} // namespace oox

