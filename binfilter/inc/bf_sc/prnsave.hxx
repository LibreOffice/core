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

#ifndef SC_PRNSAVE_HXX
#define SC_PRNSAVE_HXX

#include <tools/solar.h>
namespace binfilter {

class ScRange;

class ScPrintSaverTab
{
    USHORT		nPrintCount;
    ScRange*	pPrintRanges;	// Array
    ScRange*	pRepeatCol;		// einzeln
    ScRange*	pRepeatRow;		// einzeln

public:
            ScPrintSaverTab();
            ~ScPrintSaverTab();

    void	SetAreas( USHORT nCount, const ScRange* pRanges );
    void	SetRepeat( const ScRange* pCol, const ScRange* pRow );

    USHORT			GetPrintCount() const	{ return nPrintCount; }
    const ScRange*	GetPrintRanges() const	{ return pPrintRanges; }
    const ScRange*	GetRepeatCol() const	{ return pRepeatCol; }
    const ScRange*	GetRepeatRow() const	{ return pRepeatRow; }

};

class ScPrintRangeSaver
{
    USHORT				nTabCount;
    ScPrintSaverTab*	pData;		// Array

public:
            ScPrintRangeSaver( USHORT nCount );
            ~ScPrintRangeSaver();

    USHORT					GetTabCount() const		{ return nTabCount; }
    ScPrintSaverTab&		GetTabData(USHORT nTab);

};


} //namespace binfilter
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
