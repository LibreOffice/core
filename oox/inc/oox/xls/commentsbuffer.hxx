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



#ifndef OOX_XLS_COMMENTSBUFFER_HXX
#define OOX_XLS_COMMENTSBUFFER_HXX

#include "oox/xls/richstring.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct CommentModel
{
    ::com::sun::star::table::CellRangeAddress
                        maRange;            /// Position of the comment in the worksheet.
    RichStringRef       mxText;             /// Formatted text of the comment (not used in BIFF8).
    ::rtl::OUString     maAuthor;           /// Comment author (BIFF8 only).
    sal_Int32           mnAuthorId;         /// Identifier of the comment's author (OOXML and BIFF12 only).
    sal_uInt16          mnObjId;            /// Drawing object identifier (BIFF8 only).
    bool                mbVisible;          /// True = comment is always shown (BIFF2-BIFF8 only).

    explicit            CommentModel();
};

// ----------------------------------------------------------------------------

class Comment : public WorksheetHelper
{
public:
    explicit            Comment( const WorksheetHelper& rHelper );

    /** Imports a cell comment from the passed attributes of the comment element. */
    void                importComment( const AttributeList& rAttribs );
    /** Imports a cell comment from the passed stream of a COMMENT record. */
    void                importComment( SequenceInputStream& rStrm );
    /** Imports a cell comment from the passed stream of a NOTE record. */
    void                importNote( BiffInputStream& rStrm );

    /** Creates and returns a new rich-string object for the comment text. */
    RichStringRef       createText();

    /** Finalizes the formatted string of the comment. */
    void                finalizeImport();

private:
    /** Reads a BIFF2-BIFF5 NOTE record. */
    void                importNoteBiff2( BiffInputStream& rStrm );
    /** Reads a BIFF8 NOTE record. */
    void                importNoteBiff8( BiffInputStream& rStrm );
    /** Reads a NOTESOUND record. */
    void                importNoteSound( BiffInputStream& rStrm );

private:
    CommentModel        maModel;
};

typedef ::boost::shared_ptr< Comment > CommentRef;

// ============================================================================

class CommentsBuffer : public WorksheetHelper
{
public:
    explicit            CommentsBuffer( const WorksheetHelper& rHelper );

    /** Appends a new author to the list of comment authors. */
    void                appendAuthor( const ::rtl::OUString& rAuthor );
    /** Creates and returns a new comment. */
    CommentRef          createComment();

    /** Finalizes the formatted string of all comments. */
    void                finalizeImport();

private:
    typedef ::std::vector< ::rtl::OUString >    OUStringVector;
    typedef RefVector< Comment >                CommentVector;

    OUStringVector      maAuthors;
    CommentVector       maComments;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
