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



#ifndef LIST_HXX_INCLUDED
#define LIST_HXX_INCLUDED

#include <tools/string.hxx>

class SwNumRule;
class SwNodes;
class SwNodeNum;

class SwListImpl;

class SwList
{
    public:
        SwList( const String sListId,
                SwNumRule& rDefaultListStyle,
                const SwNodes& rNodes );
        ~SwList();

        const String GetListId() const;

        const String GetDefaultListStyleName() const;

        void InsertListItem( SwNodeNum& rNodeNum,
                             const int nLevel );
        void RemoveListItem( SwNodeNum& rNodeNum );

        void InvalidateListTree();
        void ValidateListTree();

        void MarkListLevel( const int nListLevel,
                            const sal_Bool bValue );

        bool IsListLevelMarked( const int nListLevel ) const;

//        void ContinueList( SwList& rList );
//        const SwList* GetContinuedList() const;
//        void ClearContinuation();

    private:
        // no copy constructor and no assignment operator
        SwList( const SwList& );
        SwList& operator=( const SwList& );

        SwListImpl* mpListImpl;
};
#endif // LIST_HXX_INCLUDED
