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

namespace oox {
namespace xls {

// ============================================================================

using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

CommentsFragment::CommentsFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    WorksheetFragmentBase( rHelper, rFragmentPath )
{
}

ContextHandlerRef CommentsFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
            if( nElement == XLS_TOKEN( author ) ) return this;  // collect author in onCharacters()
        break;
        case XLS_TOKEN( commentList ):
            if( nElement == XLS_TOKEN( comment ) ) { importComment( rAttribs ); return this; }
        break;
        case XLS_TOKEN( comment ):
            if( (nElement == XLS_TOKEN( text )) && mxComment.get() )
                return new RichStringContext( *this, mxComment->createText() );
        break;
    }
    return 0;
}

void CommentsFragment::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( XLS_TOKEN( author ) ) )
        getComments().appendAuthor( rChars );
}

void CommentsFragment::onEndElement()
{
    if( isCurrentElement( XLS_TOKEN( comment ) ) )
        mxComment.reset();
}

ContextHandlerRef CommentsFragment::onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nRecId == BIFF12_ID_COMMENTS ) return this;
        break;
        case BIFF12_ID_COMMENTS:
            if( nRecId == BIFF12_ID_COMMENTAUTHORS ) return this;
            if( nRecId == BIFF12_ID_COMMENTLIST ) return this;
        break;
        case BIFF12_ID_COMMENTAUTHORS:
            if( nRecId == BIFF12_ID_COMMENTAUTHOR ) getComments().appendAuthor( BiffHelper::readString( rStrm ) );
        break;
        case BIFF12_ID_COMMENTLIST:
            if( nRecId == BIFF12_ID_COMMENT ) { importComment( rStrm ); return this; }
        break;
        case BIFF12_ID_COMMENT:
            if( (nRecId == BIFF12_ID_COMMENTTEXT) && mxComment.get() )
                mxComment->createText()->importString( rStrm, true );
        break;
    }
    return 0;
}

void CommentsFragment::onEndRecord()
{
    if( isCurrentElement( BIFF12_ID_COMMENT ) )
        mxComment.reset();
}

const RecordInfo* CommentsFragment::getRecordInfos() const
{
    static const RecordInfo spRecInfos[] =
    {
        { BIFF12_ID_COMMENT,        BIFF12_ID_COMMENT + 1           },
        { BIFF12_ID_COMMENTAUTHORS, BIFF12_ID_COMMENTAUTHORS + 1    },
        { BIFF12_ID_COMMENTLIST,    BIFF12_ID_COMMENTLIST + 1       },
        { BIFF12_ID_COMMENTS,       BIFF12_ID_COMMENTS + 1          },
        { -1,                       -1                              }
    };
    return spRecInfos;
}

// private --------------------------------------------------------------------

void CommentsFragment::importComment( const AttributeList& rAttribs )
{
    mxComment = getComments().createComment();
    mxComment->importComment( rAttribs );
}

void CommentsFragment::importComment( SequenceInputStream& rStrm )
{
    mxComment = getComments().createComment();
    mxComment->importComment( rStrm );
}

// ============================================================================

} // namespace xls
} // namespace oox
