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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLISTITEMSMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLISTITEMSMANAGER_HXX

#include <IDocumentListItems.hxx>
#include <boost/noncopyable.hpp>
#include <set>

namespace sw
{

class DocumentListItemsManager : public IDocumentListItems,
                                 public ::boost::noncopyable
{
public:

    DocumentListItemsManager();

    void addListItem( const SwNodeNum& rNodeNum ) override;
    void removeListItem( const SwNodeNum& rNodeNum ) override;

    OUString getListItemText( const SwNodeNum& rNodeNum,
                                    const bool bWithNumber = true,
                                    const bool bWithSpacesForLevel = false ) const override;

    void getNumItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const override;

    virtual ~DocumentListItemsManager();


    //Non Interface
    struct lessThanNodeNum
    {
        bool operator()( const SwNodeNum* pNodeNumOne,
                         const SwNodeNum* pNodeNumTwo ) const;
    };

    typedef ::std::set< const SwNodeNum*, lessThanNodeNum > tImplSortedNodeNumList;

private:

    tImplSortedNodeNumList* mpListItemsList;
};

}

 #endif // INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTLISTITEMSMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
