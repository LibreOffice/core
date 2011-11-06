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



#ifndef SC_AREASAVE_HXX
#define SC_AREASAVE_HXX

#include "collect.hxx"
#include "global.hxx"
#include "address.hxx"

class ScDocument;
class ScAreaLink;


class ScAreaLinkSaver : public ScDataObject
{
private:
    String      aFileName;
    String      aFilterName;
    String      aOptions;
    String      aSourceArea;
    ScRange     aDestArea;
    sal_uLong       nRefresh;

public:
                ScAreaLinkSaver( const ScAreaLink& rSource );
                ScAreaLinkSaver( const ScAreaLinkSaver& rCopy );
    virtual     ~ScAreaLinkSaver();

    virtual ScDataObject*   Clone() const;

    sal_Bool        IsEqual( const ScAreaLink& rCompare ) const;
    sal_Bool        IsEqualSource( const ScAreaLink& rCompare ) const;

    void        WriteToLink( ScAreaLink& rLink ) const;
    void        InsertNewLink( ScDocument* pDoc ) const;
};


class ScAreaLinkSaveCollection : public ScCollection
{
public:
                ScAreaLinkSaveCollection();
                ScAreaLinkSaveCollection( const ScAreaLinkSaveCollection& rCopy );
    virtual     ~ScAreaLinkSaveCollection();

    virtual ScDataObject*   Clone() const;

    ScAreaLinkSaver*    operator[](sal_uInt16 nIndex) const {return (ScAreaLinkSaver*)At(nIndex);}

    sal_Bool        IsEqual( const ScDocument* pDoc ) const;
    void        Restore( ScDocument* pDoc ) const;

    // returns NULL if empty
    static ScAreaLinkSaveCollection* CreateFromDoc( const ScDocument* pDoc );
};


#endif

