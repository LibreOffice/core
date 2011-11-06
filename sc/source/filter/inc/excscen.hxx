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



#ifndef SC_EXCSCEN_HXX
#define SC_EXCSCEN_HXX

#include <tools/solar.h>
#include <tools/list.hxx>
#include <tools/string.hxx>


struct RootData;
class XclImpRoot;
class XclImpStream;
class ScDocument;



class ExcScenarioCell
{
private:
    String                      aValue;
public:
    const sal_uInt16                nCol;
    const sal_uInt16                nRow;

                                ExcScenarioCell( const sal_uInt16 nC, const sal_uInt16 nR );
    void                        SetValue( const String& rVal );
    inline const String&        GetValue( void ) const;
};




class ExcScenario : protected List
{
private:
    friend class ExcScenarioList;
protected:
    String*                     pName;
    String*                     pComment;
    String*                     pUserName;
    sal_uInt8                       nProtected;

    const sal_uInt16                nTab;

    void                        Apply( const XclImpRoot& rRoot, const sal_Bool bLast = sal_False );
public:
                                ExcScenario( XclImpStream& rIn, const RootData& rRoot );
    virtual                     ~ExcScenario();
};




class ExcScenarioList : protected List
{
private:
    sal_uInt16                      nLastScenario;
    inline ExcScenario*         _First( void )  { return ( ExcScenario* ) List::First(); }
    inline ExcScenario*         _Next( void )   { return ( ExcScenario* ) List::Next(); }
    inline ExcScenario*         _Last( void )   { return ( ExcScenario* ) List::Last(); }
    inline ExcScenario*         _Prev( void )   { return ( ExcScenario* ) List::Prev(); }
protected:
public:
                                ExcScenarioList( void );
    virtual                     ~ExcScenarioList();

    inline void                 Append( ExcScenario* pNew );

    inline void                 SetLast( const sal_uInt16 nIndex4Last );

    inline const ExcScenario*   First( void );
    inline const ExcScenario*   Next( void );

    using List::Count;

    void                        Apply( const XclImpRoot& rRoot );
};




inline const String& ExcScenarioCell::GetValue( void ) const
{
    return aValue;
}




inline ExcScenarioList::ExcScenarioList( void )
{
    nLastScenario = 0;
}


inline void ExcScenarioList::Append( ExcScenario* p )
{
    List::Insert( p, LIST_APPEND );
}


inline const ExcScenario* ExcScenarioList::First( void )
{
    return ( const ExcScenario* ) List::First();
}


inline const ExcScenario* ExcScenarioList::Next( void )
{
    return ( const ExcScenario* ) List::Next();
}


inline void ExcScenarioList::SetLast( const sal_uInt16 n )
{
    nLastScenario = n;
}


#endif

