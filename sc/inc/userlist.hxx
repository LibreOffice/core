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



#ifndef SC_USERLIST_HXX
#define SC_USERLIST_HXX

#include <tools/stream.hxx>
#include "scdllapi.h"
#include "collect.hxx"

//------------------------------------------------------------------------
class SC_DLLPUBLIC ScUserListData : public ScDataObject
{
friend class ScUserList;
    String  aStr;
    sal_uInt16  nTokenCount;
    String* pSubStrings;
    String* pUpperSub;

    SC_DLLPRIVATE void  InitTokens();

public:
                    ScUserListData(const String& rStr);
                    ScUserListData(const ScUserListData& rData);
    virtual         ~ScUserListData();

    virtual ScDataObject*       Clone() const { return new ScUserListData(*this); }

    const   String&         GetString() const { return aStr; }
            void            SetString( const String& rStr);
            sal_uInt16          GetSubCount() const;
            sal_Bool            GetSubIndex(const String& rSubStr, sal_uInt16& rIndex) const;
            String          GetSubStr(sal_uInt16 nIndex) const;
            StringCompare   Compare(const String& rSubStr1, const String& rSubStr2) const;
            StringCompare   ICompare(const String& rSubStr1, const String& rSubStr2) const;
};

//------------------------------------------------------------------------
class SC_DLLPUBLIC ScUserList : public ScCollection
{
public:
                    ScUserList( sal_uInt16 nLim = 4, sal_uInt16 nDel = 4);
                    ScUserList( const ScUserList& rUserList ) : ScCollection ( rUserList ) {}

    virtual ScDataObject*       Clone() const;

            ScUserListData* GetData( const String& rSubStr ) const;
            /// If the list in rStr is already inserted
            sal_Bool            HasEntry( const String& rStr ) const;

    inline  ScUserListData* operator[]( const sal_uInt16 nIndex) const;
    inline  ScUserList&     operator= ( const ScUserList& r );
            sal_Bool            operator==( const ScUserList& r ) const;
    inline  sal_Bool            operator!=( const ScUserList& r ) const;
};

inline  ScUserList& ScUserList::operator=( const ScUserList& r )
    { return (ScUserList&)ScCollection::operator=( r ); }

inline ScUserListData* ScUserList::operator[]( const sal_uInt16 nIndex) const
    { return (ScUserListData*)At(nIndex); }

inline sal_Bool ScUserList::operator!=( const ScUserList& r ) const
    { return !operator==( r ); }

#endif

