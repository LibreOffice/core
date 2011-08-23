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

#ifndef SC_DPSAVE_HXX
#define SC_DPSAVE_HXX

#include <tools/string.hxx>

#include <tools/list.hxx>

#include <com/sun/star/sheet/XDimensionsSupplier.hpp>

class SvStream;

// --------------------------------------------------------------------
//
//	classes to save Data Pilot settings
//

namespace binfilter {

class ScDPSaveMember
{
private:
    String		aName;
    USHORT		nVisibleMode;
    USHORT		nShowDetailsMode;

public:
                            ScDPSaveMember(const String& rName);
                            ScDPSaveMember(const ScDPSaveMember& r);
                            ScDPSaveMember(SvStream& rStream);
                            ~ScDPSaveMember();

    BOOL		 			operator== ( const ScDPSaveMember& r ) const;

    const String&			GetName()	{ return aName; }
    void					SetIsVisible(BOOL bSet);
    BOOL					GetIsVisible() { return BOOL(nVisibleMode); }
    void					SetShowDetails(BOOL bSet);
    BOOL					GetShowDetails() { return BOOL(nShowDetailsMode); }

    void					WriteToSource( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::uno::XInterface>& xMember );

    void					Store( SvStream& rStream ) const;
};


class ScDPSaveDimension
{
private:
    String		aName;
    String*		pLayoutName;		// alternative name for layout, not used (yet)
    BOOL		bIsDataLayout;
    BOOL		bDupFlag;
    USHORT		nOrientation;
    USHORT		nFunction;			// enum GeneralFunction, for data dimensions
    long		nUsedHierarchy;
    USHORT		nShowEmptyMode;		//!	at level
    BOOL		bSubTotalDefault;	//!	at level
    long		nSubTotalCount;
    USHORT*		pSubTotalFuncs;		// enum GeneralFunction
    List		aMemberList;

public:
                            ScDPSaveDimension(const String& rName, BOOL bDataLayout);
                            ScDPSaveDimension(const ScDPSaveDimension& r);
                            ScDPSaveDimension(SvStream& rStream);
                            ~ScDPSaveDimension();

    BOOL		 			operator== ( const ScDPSaveDimension& r ) const;

    const List&				GetMembers() { return aMemberList; }
    void					AddMember(ScDPSaveMember* pMember) { aMemberList.Insert(pMember, LIST_APPEND); };

    void					SetDupFlag(BOOL bSet)	{ bDupFlag = bSet; }
    BOOL					GetDupFlag() const		{ return bDupFlag; }

    const String&			GetName() const			{ return aName; }
    BOOL					IsDataLayout() const	{ return bIsDataLayout; }

    void					SetOrientation(USHORT nNew);
    void					SetSubTotals(long nCount, const USHORT* pFuncs);
    long					GetSubTotalsCount() { return nSubTotalCount; }
    USHORT					GetSubTotalFunc(long nIndex) { return pSubTotalFuncs[nIndex]; }
    void					SetShowEmpty(BOOL bSet);
    BOOL					GetShowEmpty() { return BOOL(nShowEmptyMode); }
    void					SetFunction(USHORT nNew);		// enum GeneralFunction
    USHORT					GetFunction() { return nFunction; }
    void					SetUsedHierarchy(long nNew);
    long					GetUsedHierarchy() { return nUsedHierarchy; }
    void					SetLayoutName(const String* pName);
    const String&			GetLayoutName() const;
    BOOL					HasLayoutName() const;
    void					ResetLayoutName();

    USHORT					GetOrientation() const	{ return nOrientation; }


    void					WriteToSource( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::uno::XInterface>& xDim );

    void					Store( SvStream& rStream ) const;
};


class ScDPSaveData
{
private:
    List		aDimList;
    USHORT		nColumnGrandMode;
    USHORT		nRowGrandMode;
    USHORT		nIgnoreEmptyMode;
    USHORT		nRepeatEmptyMode;

public:
                            ScDPSaveData();
                            ScDPSaveData(const ScDPSaveData& r);
                            ~ScDPSaveData();

    ScDPSaveData& 			operator= ( const ScDPSaveData& r );

    BOOL		 			operator== ( const ScDPSaveData& r ) const;

    const List&				GetDimensions() const { return aDimList; }
    void					AddDimension(ScDPSaveDimension* pDim) { aDimList.Insert(pDim, LIST_APPEND); }

    ScDPSaveDimension*		GetDimensionByName(const String& rName);
    ScDPSaveDimension*		GetDataLayoutDimension();

    ScDPSaveDimension*		DuplicateDimension(const String& rName);

    ScDPSaveDimension*		GetExistingDimensionByName(const String& rName);

    void					SetColumnGrand( BOOL bSet );
    BOOL					GetColumnGrand() const { return BOOL(nColumnGrandMode); }
    void					SetRowGrand( BOOL bSet );
    BOOL					GetRowGrand() const { return BOOL(nRowGrandMode); }
    void					SetIgnoreEmptyRows( BOOL bSet );
    BOOL					GetIgnoreEmptyRows() const { return BOOL(nIgnoreEmptyMode); }
    void					SetRepeatIfEmpty( BOOL bSet );
    BOOL					GetRepeatIfEmpty() const { return BOOL(nRepeatEmptyMode); }

    void					WriteToSource( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::sheet::XDimensionsSupplier>& xSource );

    void					Store( SvStream& rStream ) const;
    void					Load( SvStream& rStream );

};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
