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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_COMMENTSFRAGMENT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_COMMENTSFRAGMENT_HXX

#include "commentsbuffer.hxx"
#include "excelhandlers.hxx"

namespace oox {
namespace xls {

class CommentsFragment : public WorksheetFragmentBase
{
public:
    explicit            CommentsFragment(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );
protected:
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs ) override;
    virtual void        onCharacters( const OUString& rChars ) override;
    virtual void        onEndElement() override;

    virtual ::oox::core::ContextHandlerRef onCreateRecordContext( sal_Int32 nRecId, SequenceInputStream& rStrm ) override;
    virtual void        onEndRecord() override;

    virtual const ::oox::core::RecordInfo* getRecordInfos() const override;

private:
    /** Imports comment data from the comment element. */
    void                importComment( const AttributeList& rAttribs );
    /** Imports comment data from the COMMENT record. */
    void                importComment( SequenceInputStream& rStrm );

private:
    CommentRef          mxComment;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
