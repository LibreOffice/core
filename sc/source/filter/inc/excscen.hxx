/*************************************************************************
 *
 *  $RCSfile: excscen.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/



#ifndef _EXCSCEN_HXX
#define _EXCSCEN_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif


struct RootData;




class ExcScenarioCell
{
private:
    String                      aValue;
public:
    const UINT16                nCol;
    const UINT16                nRow;

                                ExcScenarioCell( const UINT16 nC, const UINT16 nR );
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

    const UINT16                nTab;

    void                        Apply( ScDocument& rDoc, const BOOL bLast = FALSE );
public:
                                ExcScenario( SvStream& rIn, const RootData& rRoot );
    virtual                     ~ExcScenario();
};




class ExcScenarioList : protected List
{
private:
    UINT16                      nLastScenario;
    inline ExcScenario*         _First( void )  { return ( ExcScenario* ) List::First(); }
    inline ExcScenario*         _Next( void )   { return ( ExcScenario* ) List::Next(); }
    inline ExcScenario*         _Last( void )   { return ( ExcScenario* ) List::Last(); }
    inline ExcScenario*         _Prev( void )   { return ( ExcScenario* ) List::Prev(); }
protected:
public:
    inline                      ExcScenarioList( void );
    virtual                     ~ExcScenarioList();

    inline void                 Append( ExcScenario* pNew );

    inline void                 SetLast( const UINT16 nIndex4Last );

    inline const ExcScenario*   First( void );
    inline const ExcScenario*   Next( void );

    List::Count;

    void                        Apply( ScDocument& rDoc );
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


inline void ExcScenarioList::SetLast( const UINT16 n )
{
    nLastScenario = n;
}


#endif

