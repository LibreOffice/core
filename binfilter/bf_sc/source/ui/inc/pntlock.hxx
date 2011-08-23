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

#ifndef SC_PNTLOCK_HXX
#define SC_PNTLOCK_HXX

#include "rangelst.hxx"
namespace binfilter {

class ScPaintLockData
{
private:
    ScRangeListRef	xRangeList;
    USHORT			nMode;
    USHORT			nLevel;
    USHORT			nDocLevel;
    USHORT			nParts;
    BOOL			bModified;

public:
                    ScPaintLockData(USHORT nNewMode);
                    ~ScPaintLockData();

    void			AddRange( const ScRange& rRange, USHORT nP );

    void			SetModified()	{ bModified = TRUE; }
    void			IncLevel(sal_Bool bDoc)
                        { if (bDoc) ++nDocLevel; else ++nLevel; }
    void			DecLevel(sal_Bool bDoc)
                        { if (bDoc) --nDocLevel; else --nLevel; }

    const ScRangeListRef&	GetRangeList() const			{ return xRangeList; }
    USHORT					GetParts() const				{ return nParts; }
    USHORT					GetLevel(sal_Bool bDoc) const
                                { return bDoc ? nDocLevel : nLevel; }
    BOOL					GetModified() const				{ return bModified; }

                    // fuer Wiederherstellen nach Reset
    void			SetLevel(USHORT nNew, sal_Bool bDoc)
                        { if (bDoc) nDocLevel = nNew; else nLevel = nNew; }
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
