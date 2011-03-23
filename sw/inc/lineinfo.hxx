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
#include <editeng/numitem.hxx>
#include "swdllapi.h"

class SwCharFmt;
class IDocumentStylePoolAccess;

enum LineNumberPosition
{
    LINENUMBER_POS_LEFT,
    LINENUMBER_POS_RIGHT,
    LINENUMBER_POS_INSIDE,
    LINENUMBER_POS_OUTSIDE
};

class SW_DLLPUBLIC SwLineNumberInfo : public SwClient //purpose of derivation from SwClient:
                                         //character style for displaying the numbers.
{
    SvxNumberType       aType;              //e.g. roman linenumbers
    String              aDivider;           //String for aditional interval (vert. lines user defined)
    sal_uInt16              nPosFromLeft;       //Position for paint
    sal_uInt16              nCountBy;           //Paint only for every n line
    sal_uInt16              nDividerCountBy;    //Interval for display of an user defined
                                            //string every n lines
    LineNumberPosition  ePos;               //Where should the display occur (number and divicer)
    sal_Bool                bPaintLineNumbers;  //Should anything be displayed?
    sal_Bool                bCountBlankLines;   //Count empty lines?
    sal_Bool                bCountInFlys;       //Count also within FlyFrames?
    sal_Bool                bRestartEachPage;   //Restart counting at the first paragraph of each page
                                            //(even on follows when paragraphs are splitted)
protected:
   virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    SwLineNumberInfo();
    SwLineNumberInfo(const SwLineNumberInfo&);

    SwLineNumberInfo& operator=(const SwLineNumberInfo&);
    sal_Bool operator==( const SwLineNumberInfo& rInf ) const;

    SwCharFmt *GetCharFmt( IDocumentStylePoolAccess& rIDSPA ) const;
    void SetCharFmt( SwCharFmt* );

    const SvxNumberType &GetNumType() const             { return aType; }
    void                SetNumType( SvxNumberType aNew ){ aType = aNew; }

    const String &GetDivider() const    { return aDivider; }
    void SetDivider( const String &r )  { aDivider = r; }
    sal_uInt16 GetDividerCountBy() const    { return nDividerCountBy; }
    void SetDividerCountBy( sal_uInt16 n )  { nDividerCountBy = n; }

    sal_uInt16 GetPosFromLeft() const       { return nPosFromLeft; }
    void   SetPosFromLeft( sal_uInt16 n)    { nPosFromLeft = n;    }

    sal_uInt16 GetCountBy() const           { return nCountBy; }
    void   SetCountBy( sal_uInt16 n)        { nCountBy = n;    }

    LineNumberPosition GetPos() const   { return ePos; }
    void SetPos( LineNumberPosition eP ){ ePos = eP;   }

    sal_Bool   IsPaintLineNumbers() const   { return bPaintLineNumbers; }
    void   SetPaintLineNumbers( sal_Bool b ){ bPaintLineNumbers = b;    }

    sal_Bool   IsCountBlankLines() const    { return bCountBlankLines;  }
    void   SetCountBlankLines( sal_Bool b ) { bCountBlankLines = b;     }

    sal_Bool   IsCountInFlys() const        { return bCountInFlys;      }
    void   SetCountInFlys( sal_Bool b )     { bCountInFlys = b;         }

    sal_Bool   IsRestartEachPage() const    { return bRestartEachPage;  }
    void   SetRestartEachPage( sal_Bool b ) { bRestartEachPage = b;     }

    bool   HasCharFormat() const { return GetRegisteredIn() != 0; }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
