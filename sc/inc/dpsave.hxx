/*************************************************************************
 *
 *  $RCSfile: dpsave.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
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

#ifndef SC_DPSAVE_HXX
#define SC_DPSAVE_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _LIST_HXX //autogen wg. List
#include <tools/list.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XDIMENSIONSSUPPLIER_HPP_
#include <com/sun/star/sheet/XDimensionsSupplier.hpp>
#endif

class SvStream;

// --------------------------------------------------------------------
//
//  classes to save Data Pilot settings
//


class ScDPSaveMember
{
private:
    String      aName;
    USHORT      nVisibleMode;
    USHORT      nShowDetailsMode;

public:
                            ScDPSaveMember(const String& rName);
                            ScDPSaveMember(const ScDPSaveMember& r);
                            ScDPSaveMember(SvStream& rStream);
                            ~ScDPSaveMember();

    BOOL                    operator== ( const ScDPSaveMember& r ) const;

    const String&           GetName()   { return aName; }
    void                    SetIsVisible(BOOL bSet);
    BOOL                    GetIsVisible() { return BOOL(nVisibleMode); }
    void                    SetShowDetails(BOOL bSet);
    BOOL                    GetShowDetails() { return BOOL(nShowDetailsMode); }

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::uno::XInterface>& xMember );

    void                    Store( SvStream& rStream ) const;
};


class ScDPSaveDimension
{
private:
    String      aName;
    String*     pLayoutName;        // alternative name for layout, not used (yet)
    BOOL        bIsDataLayout;
    BOOL        bDupFlag;
    USHORT      nOrientation;
    USHORT      nFunction;          // enum GeneralFunction, for data dimensions
    long        nUsedHierarchy;
    USHORT      nShowEmptyMode;     //! at level
    BOOL        bSubTotalDefault;   //! at level
    long        nSubTotalCount;
    USHORT*     pSubTotalFuncs;     // enum GeneralFunction
    List        aMemberList;

public:
                            ScDPSaveDimension(const String& rName, BOOL bDataLayout);
                            ScDPSaveDimension(const ScDPSaveDimension& r);
                            ScDPSaveDimension(SvStream& rStream);
                            ~ScDPSaveDimension();

    BOOL                    operator== ( const ScDPSaveDimension& r ) const;

    const List&             GetMembers() { return aMemberList; }
    void                    AddMember(ScDPSaveMember* pMember) { aMemberList.Insert(pMember, LIST_APPEND); };

    void                    SetDupFlag(BOOL bSet)   { bDupFlag = bSet; }
    BOOL                    GetDupFlag() const      { return bDupFlag; }

    const String&           GetName() const         { return aName; }
    BOOL                    IsDataLayout() const    { return bIsDataLayout; }

    void                    SetOrientation(USHORT nNew);
    void                    SetSubTotals(BOOL bSet);        // to be removed!
    void                    SetSubTotals(long nCount, const USHORT* pFuncs);
    long                    GetSubTotalsCount() { return nSubTotalCount; }
    USHORT                  GetSubTotalFunc(long nIndex) { return pSubTotalFuncs[nIndex]; }
    void                    SetShowEmpty(BOOL bSet);
    BOOL                    GetShowEmpty() { return BOOL(nShowEmptyMode); }
    void                    SetFunction(USHORT nNew);       // enum GeneralFunction
    USHORT                  GetFunction() { return nFunction; }
    void                    SetUsedHierarchy(long nNew);
    long                    GetUsedHierarchy() { return nUsedHierarchy; }
    void                    SetLayoutName(const String* pName);
    const String&           GetLayoutName() const;

    USHORT                  GetOrientation() const  { return nOrientation; }

    ScDPSaveMember*         GetMemberByName(const String& rName);

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::uno::XInterface>& xDim );

    void                    Store( SvStream& rStream ) const;
};


class ScDPSaveData
{
private:
    List        aDimList;
    USHORT      nColumnGrandMode;
    USHORT      nRowGrandMode;
    USHORT      nIgnoreEmptyMode;
    USHORT      nRepeatEmptyMode;

public:
                            ScDPSaveData();
                            ScDPSaveData(const ScDPSaveData& r);
                            ~ScDPSaveData();

    ScDPSaveData&           operator= ( const ScDPSaveData& r );

    BOOL                    operator== ( const ScDPSaveData& r ) const;

    const List&             GetDimensions() const { return aDimList; }
    void                    AddDimension(ScDPSaveDimension* pDim) { aDimList.Insert(pDim, LIST_APPEND); }

    ScDPSaveDimension*      GetDimensionByName(const String& rName);
    ScDPSaveDimension*      GetDataLayoutDimension();

    ScDPSaveDimension*      DuplicateDimension(const String& rName);

    ScDPSaveDimension*      GetExistingDimensionByName(const String& rName);
    ScDPSaveDimension*      GetNewDimensionByName(const String& rName);

    void                    SetPosition( ScDPSaveDimension* pDim, long nNew );
    void                    SetColumnGrand( BOOL bSet );
    BOOL                    GetColumnGrand() const { return BOOL(nColumnGrandMode); }
    void                    SetRowGrand( BOOL bSet );
    BOOL                    GetRowGrand() const { return BOOL(nRowGrandMode); }
    void                    SetIgnoreEmptyRows( BOOL bSet );
    BOOL                    GetIgnoreEmptyRows() const { return BOOL(nIgnoreEmptyMode); }
    void                    SetRepeatIfEmpty( BOOL bSet );
    BOOL                    GetRepeatIfEmpty() const { return BOOL(nRepeatEmptyMode); }

    void                    WriteToSource( const com::sun::star::uno::Reference<
                                            com::sun::star::sheet::XDimensionsSupplier>& xSource );

    void                    Store( SvStream& rStream ) const;
    void                    Load( SvStream& rStream );

    BOOL                    IsEmpty() const;
};


#endif

