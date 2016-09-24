/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_COMMENTSBUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_COMMENTSBUFFER_HXX

#include "richstring.hxx"
#include "worksheethelper.hxx"
#include <com/sun/star/awt/Rectangle.hpp>

namespace oox {
namespace xls {

struct CommentModel
{
    css::table::CellRangeAddress
                        maRange;            /// Position of the comment in the worksheet.
    RichStringRef       mxText;             /// Formatted text of the comment (not used in BIFF8).
    sal_Int32           mnAuthorId;         /// Identifier of the comment's author (OOXML and BIFF12 only).
    bool                mbAutoFill;         /// Auto Selection of comment object's fill style
    bool                mbAutoScale;        /// Auto Scale comment text
    bool                mbColHidden;        /// Comment cell's Column is Hidden
    bool                mbLocked;           /// Comment changes Locked
    bool                mbRowHidden;        /// Comment cell's Row is Hidden
    sal_Int32           mnTHA;              /// Horizontal Alignment
    sal_Int32           mnTVA;              /// Vertical Alignment
    css::awt::Rectangle maAnchor;           /// Anchor parameters

    explicit            CommentModel();
};

class Comment : public WorksheetHelper
{
public:
    explicit            Comment( const WorksheetHelper& rHelper );

    /** Imports a cell comment from the passed attributes of the comment element. */
    void                importComment( const AttributeList& rAttribs );
    /** Imports a cell comment Properties from the passed attributes of the comment element. */
    void                importCommentPr( const AttributeList& rAttribs );
    /** Imports a cell comment from the passed stream of a COMMENT record. */
    void                importComment( SequenceInputStream& rStrm );

    /** Creates and returns a new rich-string object for the comment text. */
    RichStringRef const & createText();

    /** Finalizes the formatted string of the comment. */
    void                finalizeImport();

private:
    CommentModel        maModel;
};

typedef std::shared_ptr< Comment > CommentRef;

class CommentsBuffer : public WorksheetHelper
{
public:
    explicit            CommentsBuffer( const WorksheetHelper& rHelper );

    /** Appends a new author to the list of comment authors. */
    void                appendAuthor( const OUString& rAuthor );
    /** Creates and returns a new comment. */
    CommentRef          createComment();

    /** Finalizes the formatted string of all comments. */
    void                finalizeImport();

private:
    typedef RefVector< Comment >                CommentVector;

    std::vector< OUString > maAuthors;
    CommentVector       maComments;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
