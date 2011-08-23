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
#ifndef SW_LINEINFO_HXX
#define SW_LINEINFO_HXX


#include "calbck.hxx"
#include "numrule.hxx"
namespace binfilter {

class SwCharFmt;
class SwDoc;

enum LineNumberPosition
{
    LINENUMBER_POS_LEFT,
    LINENUMBER_POS_RIGHT,
    LINENUMBER_POS_INSIDE,
    LINENUMBER_POS_OUTSIDE
};

class SwLineNumberInfo : public SwClient //purpose of derivation from SwClient:
                                         //character style for displaying the numbers.
{
    SvxNumberType 		aType;				//e.g. roman linenumbers
    String				aDivider;        	//String for aditional interval (vert. lines user defined)
    USHORT				nPosFromLeft;		//Position for paint
    USHORT				nCountBy;			//Paint only for every n line
    USHORT				nDividerCountBy;	//Interval for display of an user defined
                                            //string every n lines
    LineNumberPosition	ePos;               //Where should the display occur (number and divicer)
    BOOL				bPaintLineNumbers;	//Should anything be displayed?
    BOOL				bCountBlankLines;	//Count empty lines?
    BOOL				bCountInFlys;		//Count also within FlyFrames?
    BOOL				bRestartEachPage;	//Restart counting at the first paragraph of each page
                                            //(even on follows when paragraphs are splitted)

public:
    SwLineNumberInfo();
    SwLineNumberInfo(const SwLineNumberInfo&);

    SwLineNumberInfo& operator=(const SwLineNumberInfo&);
    BOOL operator==( const SwLineNumberInfo& rInf ) const;

    SwCharFmt *GetCharFmt(SwDoc &rDoc) const;
    void SetCharFmt( SwCharFmt* );

    const SvxNumberType &GetNumType() const 			{ return aType; }
    void 			 	SetNumType( SvxNumberType aNew ){ aType = aNew; }

    const String &GetDivider() const 	{ return aDivider; }
    void SetDivider( const String &r )	{ aDivider = r;	}
    USHORT GetDividerCountBy() const	{ return nDividerCountBy; }
    void SetDividerCountBy( USHORT n )	{ nDividerCountBy = n; }

    USHORT GetPosFromLeft() const 		{ return nPosFromLeft; }
    void   SetPosFromLeft( USHORT n)	{ nPosFromLeft = n;    }

    USHORT GetCountBy() const 			{ return nCountBy; }
    void   SetCountBy( USHORT n)		{ nCountBy = n;    }

    LineNumberPosition GetPos() const	{ return ePos; }
    void SetPos( LineNumberPosition eP ){ ePos = eP;   }

    BOOL   IsPaintLineNumbers() const	{ return bPaintLineNumbers; }
    void   SetPaintLineNumbers( BOOL b ){ bPaintLineNumbers = b;	}

    BOOL   IsCountBlankLines() const 	{ return bCountBlankLines;  }
    void   SetCountBlankLines( BOOL b )	{ bCountBlankLines = b;		}

    BOOL   IsCountInFlys() const		{ return bCountInFlys; 		}
    void   SetCountInFlys( BOOL b )		{ bCountInFlys = b;			}

    BOOL   IsRestartEachPage() const	{ return bRestartEachPage;	}
    void   SetRestartEachPage( BOOL b )	{ bRestartEachPage = b;		}

    virtual	void Modify( SfxPoolItem*, SfxPoolItem* );
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
