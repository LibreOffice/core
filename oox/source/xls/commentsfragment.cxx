/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
