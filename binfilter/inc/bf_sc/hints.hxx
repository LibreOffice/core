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

#ifndef SC_HINTS_HXX
#define SC_HINTS_HXX

#include "global.hxx"

#include <bf_svtools/hint.hxx>
#include <tools/debug.hxx>
namespace binfilter {

// ---------------------------------------------------------------------------

class ScPaintHint : public SfxHint
{
    ScRange		aRange;
    USHORT		nParts;
    BOOL		bPrint;		//	Flag, ob auch Druck/Vorschau betroffen ist

public:
                    TYPEINFO();
                    ScPaintHint( const ScRange& rRng, USHORT nPaint = PAINT_ALL );
                    ~ScPaintHint();

    void			SetPrintFlag(BOOL bSet)	{ bPrint = bSet; }

    const ScRange&	GetRange() const		{ return aRange; }
    USHORT          GetStartCol() const     { return aRange.aStart.Col(); }
    USHORT          GetStartRow() const     { return aRange.aStart.Row(); }
    USHORT          GetStartTab() const     { return aRange.aStart.Tab(); }
    USHORT          GetEndCol() const       { return aRange.aEnd.Col(); }
    USHORT          GetEndRow() const       { return aRange.aEnd.Row(); }
    USHORT          GetEndTab() const       { return aRange.aEnd.Tab(); }
    USHORT          GetParts() const        { return nParts; }
    BOOL			GetPrintFlag() const	{ return bPrint; }
};


class ScUpdateRefHint : public SfxHint
{
    UpdateRefMode	eUpdateRefMode;
    ScRange			aRange;
    short			nDx;
    short			nDy;
    short			nDz;

public:
                    TYPEINFO();

                    ScUpdateRefHint( UpdateRefMode eMode, const ScRange& rR,
                                        short nX, short nY, short nZ );
                    ~ScUpdateRefHint();

    UpdateRefMode	GetMode() const			{ return eUpdateRefMode; }
    const ScRange&	GetRange() const		{ return aRange; }
    short			GetDx() const			{ return nDx; }
    short			GetDy() const			{ return nDy; }
    short			GetDz() const			{ return nDz; }
};


#define SC_POINTERCHANGED_NUMFMT	1

class ScPointerChangedHint : public SfxHint
{
    USHORT			nFlags;

public:
                    TYPEINFO();

                    ScPointerChangedHint( USHORT nF );
                    ~ScPointerChangedHint();

    USHORT			GetFlags() const			{ return nFlags; }
};


//!	move ScLinkRefreshedHint to a different file?

#define SC_LINKREFTYPE_NONE		0
#define SC_LINKREFTYPE_SHEET	1
#define SC_LINKREFTYPE_AREA		2
#define SC_LINKREFTYPE_DDE		3

class ScLinkRefreshedHint : public SfxHint
{
    USHORT		nLinkType;	// SC_LINKREFTYPE_...
    String		aUrl;		// used for sheet links
    String		aDdeAppl;	// used for dde links:
    String		aDdeTopic;
    String		aDdeItem;
    BYTE		nDdeMode;
    ScAddress	aDestPos;	// used to identify area links
                            //!	also use source data for area links?

public:
                    TYPEINFO();
                    ScLinkRefreshedHint();
                    ~ScLinkRefreshedHint();

    void			SetSheetLink( const String& rSourceUrl );
    void			SetAreaLink( const ScAddress& rPos );

    USHORT				GetLinkType() const	{ return nLinkType; }
    const String&		GetUrl() const		{ return aUrl; }
    const String&		GetDdeAppl() const	{ return aDdeAppl; }
    const String&		GetDdeTopic() const	{ return aDdeTopic; }
    const String&		GetDdeItem() const	{ return aDdeItem; }
    BYTE				GetDdeMode() const	{ return nDdeMode; }
    const ScAddress&	GetDestPos() const	{ return aDestPos; }
};


//!	move ScAutoStyleHint to a different file?

class ScAutoStyleHint : public SfxHint
{
public:
                    TYPEINFO();
                    ScAutoStyleHint( const ScRange& rR, const String& rSt1,
                                        ULONG nT, const String& rSt2 ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ULONG nT, const String& rSt2 );

};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
