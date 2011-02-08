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

