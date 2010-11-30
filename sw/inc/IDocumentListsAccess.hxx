/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef IDOCUMENTLISTSACCESS_HXX_INCLUDED
#define IDOCUMENTLISTSACCESS_HXX_INCLUDED

#include <tools/string.hxx>

class SwList;

/** Provides access to the lists of a document.
*/
class IDocumentListsAccess
{
    public:
        virtual SwList* createList( String sListId,
                                    const String sDefaultListStyleName ) = 0;
        virtual void deleteList( const String sListId ) = 0;
        virtual SwList* getListByName( const String sListId ) const = 0;

        virtual SwList* createListForListStyle( const String sListStyleName ) = 0;
        virtual SwList* getListForListStyle( const String sListStyleName ) const = 0;
        virtual void deleteListForListStyle( const String sListStyleName ) = 0;
        // #i91400#
        virtual void trackChangeOfListStyleName( const String sListStyleName,
                                                 const String sNewListStyleName ) = 0;
    protected:
        virtual ~IDocumentListsAccess() {};
};

#endif // IDOCUMENTLISTSACCESS_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
