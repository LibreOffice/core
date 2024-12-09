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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLISTSMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLISTSMANAGER_HXX

#include <IDocumentListsAccess.hxx>
#include <o3tl/deleter.hxx>
#include <memory>
#include <unordered_map>
#include <names.hxx>

class SwList;
class SwDoc;

namespace sw
{

class DocumentListsManager final : public IDocumentListsAccess
{
    public:

        DocumentListsManager( SwDoc& i_rSwdoc );

        SwList* createList( const OUString& rListId,
                                    const UIName& rDefaultListStyleName ) override;
        SwList* getListByName( const OUString& rListId ) const override;

        void createListForListStyle( const UIName& rListStyleName ) override;
        SwList* getListForListStyle( const UIName& rListStyleName ) const override;
        void deleteListForListStyle( const UIName& rListStyleName ) override;
        void deleteListsByDefaultListStyle( const UIName& rListStyleName ) override;
        // #i91400#
        void trackChangeOfListStyleName( const UIName& rListStyleName,
                                                 const UIName& rNewListStyleName ) override;
        virtual ~DocumentListsManager() override;

    private:

        DocumentListsManager(DocumentListsManager const&) = delete;
        DocumentListsManager& operator=(DocumentListsManager const&) = delete;

        SwDoc& m_rDoc;

        typedef std::unique_ptr<SwList, o3tl::default_delete<SwList>> SwListPtr;
        // container to hold the lists of the text document
        std::unordered_map<OUString, SwListPtr> maLists;
        // relation between list style and its default list
        std::unordered_map<UIName, SwList*> maListStyleLists;

        OUString CreateUniqueListId();
        OUString MakeListIdUnique( const OUString& aSuggestedUniqueListId );
};

}

#endif // INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLISTSMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
