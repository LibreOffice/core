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
#include <boost/utility.hpp>
#include <unordered_map>

class SwList;
class SwDoc;

namespace sw
{


class DocumentListsManager : public IDocumentListsAccess,
                             public ::boost::noncopyable
{
    public:

        DocumentListsManager( SwDoc& i_rSwdoc );

        SwList* createList( const OUString& rListId,
                                    const OUString& rDefaultListStyleName ) SAL_OVERRIDE;
        void deleteList( const OUString& rListId ) SAL_OVERRIDE;
        SwList* getListByName( const OUString& rListId ) const SAL_OVERRIDE;

        SwList* createListForListStyle( const OUString& rListStyleName ) SAL_OVERRIDE;
        SwList* getListForListStyle( const OUString& rListStyleName ) const SAL_OVERRIDE;
        void deleteListForListStyle( const OUString& rListStyleName ) SAL_OVERRIDE;
        void deleteListsByDefaultListStyle( const OUString& rListStyleName ) SAL_OVERRIDE;
        // #i91400#
        void trackChangeOfListStyleName( const OUString& rListStyleName,
                                                 const OUString& rNewListStyleName ) SAL_OVERRIDE;
        virtual ~DocumentListsManager();

    private:

        SwDoc& m_rDoc;

        typedef std::unordered_map<OUString, SwList*, OUStringHash> tHashMapForLists;
        // container to hold the lists of the text document
        tHashMapForLists maLists;
        // relation between list style and its default list
        tHashMapForLists maListStyleLists;

        const OUString CreateUniqueListId();
        const OUString MakeListIdUnique( const OUString& aSuggestedUniqueListId );
};

}

#endif // INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLISTSMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
