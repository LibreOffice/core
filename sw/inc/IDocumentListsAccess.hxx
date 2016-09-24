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

#ifndef INCLUDED_SW_INC_IDOCUMENTLISTSACCESS_HXX
#define INCLUDED_SW_INC_IDOCUMENTLISTSACCESS_HXX

#include <rtl/ustring.hxx>

class SwList;

/** Provides access to the lists of a document.
*/
class IDocumentListsAccess
{
    public:
        virtual SwList* createList( const OUString& rListId,
                                    const OUString& rDefaultListStyleName ) = 0;
        virtual void deleteList( const OUString& rListId ) = 0;
        virtual SwList* getListByName( const OUString& rListId ) const = 0;

        virtual void createListForListStyle( const OUString& rListStyleName ) = 0;
        virtual SwList* getListForListStyle( const OUString& rListStyleName ) const = 0;
        virtual void deleteListForListStyle( const OUString& rListStyleName ) = 0;
        virtual void deleteListsByDefaultListStyle( const OUString& rListStyleName ) = 0;
        // #i91400#
        virtual void trackChangeOfListStyleName( const OUString& rListStyleName,
                                                 const OUString& rNewListStyleName ) = 0;
    protected:
        virtual ~IDocumentListsAccess() {};
};

#endif // INCLUDED_SW_INC_IDOCUMENTLISTSACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
