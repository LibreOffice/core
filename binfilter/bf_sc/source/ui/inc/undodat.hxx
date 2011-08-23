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

#ifndef SC_UNDODAT_HXX
#define SC_UNDODAT_HXX

#include "undobase.hxx"

#include "rangeutl.hxx"		// ScArea

#include "rangelst.hxx"		// ScRangeListRef

#include "markdata.hxx"

#include "sortparam.hxx"
namespace binfilter {


class ScDocShell;
class ScDocument;
class ScOutlineTable;
class ScRangeName;
class ScDBData;
class ScDBCollection;
class ScPivot;
class ScDPObject;

//----------------------------------------------------------------------------



















class ScUndoDBData: public ScSimpleUndo
{
public:
                    ScUndoDBData( ScDocShell* pNewDocShell,
                            ScDBCollection* pNewUndoColl, ScDBCollection* pNewRedoColl );
    virtual 		~ScUndoDBData();

    virtual BOOL	CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual String	GetComment() const;

private:
    ScDBCollection* pUndoColl;
    ScDBCollection* pRedoColl;
};












class ScUndoChartData: public ScSimpleUndo
{
public:
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const String& rName, const ScRange& rNew,
                                BOOL bColHdr, BOOL bRowHdr, BOOL bAdd );
                        ScUndoChartData( ScDocShell* pNewDocShell,
                                const String& rName, const ScRangeListRef& rNew,
                                BOOL bColHdr, BOOL bRowHdr, BOOL bAdd );
    virtual 			~ScUndoChartData();



private:
    String				aChartName;
    ScRangeListRef		aOldRangeListRef;
    BOOL				bOldColHeaders;
    BOOL				bOldRowHeaders;
//	ScRange				aNewRange;
    ScRangeListRef		aNewRangeListRef;
    BOOL				bNewColHeaders;
    BOOL				bNewRowHeaders;
    BOOL				bAddRange;

};




} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
