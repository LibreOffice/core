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

#ifndef INCLUDED_SW_INC_LIST_HXX
#define INCLUDED_SW_INC_LIST_HXX

#include <rtl/ustring.hxx>
#include "SwNodeNum.hxx"
#include "pam.hxx"
#include <memory>

class SwDoc;
class SwNumRule;
class SwNodes;

enum class SwListRedlineType
{
    SHOW,
    HIDDEN,
    ORIGTEXT,
};

class SwList
{
    public:
        SwList( OUString sListId,
                SwNumRule& rDefaultListStyle,
                const SwNodes& rNodes );
        ~SwList() COVERITY_NOEXCEPT_FALSE;

        const OUString & GetListId() const { return msListId; }

        const UIName & GetDefaultListStyleName() const { return msDefaultListStyleName; }

        void SetDefaultListStyleName(UIName const&);

        void InsertListItem(SwNodeNum& rNodeNum,
                            SwListRedlineType eRedlines,
                            const int nLevel,
                            const SwDoc& rDoc);
        static void RemoveListItem(SwNodeNum& rNodeNum, const SwDoc& rDoc);

        void InvalidateListTree();
        void ValidateListTree(const SwDoc& rDoc);

        void MarkListLevel( const int nListLevel,
                            const bool bValue );

        bool IsListLevelMarked( const int nListLevel ) const;

        bool HasNodes() const;

    private:
        SwList( const SwList& ) = delete;
        SwList& operator=( const SwList& ) = delete;

        void NotifyItemsOnListLevel( const int nLevel );

        // unique identifier of the list
        const OUString msListId;
        // default list style for the list items, identified by the list style name
        UIName msDefaultListStyleName;

        // list trees for certain document ranges
        struct tListTreeForRange
        {
            /// tree always corresponds to document model
            std::unique_ptr<SwNodeNum> pRoot;
            /// Tree that is missing those nodes that are merged or hidden
            /// by delete redlines; this is only used if there is a layout
            /// that has IsHideRedlines() enabled.
            /// A second tree is needed because not only are the numbers in
            /// the nodes different, the structure of the tree may be different
            /// as well, if a high-level node is hidden its children go under
            /// the previous node on the same level.
            /// The nodes of pRootRLHidden are a subset of the nodes of pRoot.
            std::unique_ptr<SwNodeNum> pRootRLHidden;
            /// Tree that is missing those nodes that are merged or hidden
            /// by insert redlines; this is only used if there is a layout
            /// that has IsHideRedlines() disabled, and the numbering of the
            /// original text is also shown.
            /// A third tree is needed because not only are the numbers in
            /// the nodes different, the structure of the tree may be different
            /// The nodes of pRootOrigText are a subset of the nodes of pRoot.
            std::unique_ptr<SwNodeNum> pRootOrigText;
            /// top-level SwNodes section
            std::unique_ptr<SwPaM> pSection;
            tListTreeForRange(std::unique_ptr<SwNodeNum> p1, std::unique_ptr<SwNodeNum> p2,
                                                std::unique_ptr<SwNodeNum> p3, std::unique_ptr<SwPaM> p4)
                : pRoot(std::move(p1)), pRootRLHidden(std::move(p2)),
                                                pRootOrigText(std::move(p3)), pSection(std::move(p4)) {}
        };
        std::vector<tListTreeForRange> maListTrees;

        int mnMarkedListLevel;
};
#endif // INCLUDED_SW_INC_LIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
