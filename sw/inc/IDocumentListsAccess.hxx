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
        // --> OD 2008-07-08 #i91400#
        virtual void trackChangeOfListStyleName( const String sListStyleName,
                                                 const String sNewListStyleName ) = 0;
        // <--

    protected:
        virtual ~IDocumentListsAccess() {};
};

#endif // IDOCUMENTLISTSACCESS_HXX_INCLUDED
