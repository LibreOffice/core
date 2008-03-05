/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineinfo.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:49:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SW_LINEINFO_HXX
#define SW_LINEINFO_HXX

#ifndef _CALBCK_HXX
#include "calbck.hxx"
#endif
//#ifndef _NUMRULE_HXX
//#include <numrule.hxx>
//#endif
#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif
#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

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
    USHORT              nPosFromLeft;       //Position for paint
    USHORT              nCountBy;           //Paint only for every n line
    USHORT              nDividerCountBy;    //Interval for display of an user defined
                                            //string every n lines
    LineNumberPosition  ePos;               //Where should the display occur (number and divicer)
    BOOL                bPaintLineNumbers;  //Should anything be displayed?
    BOOL                bCountBlankLines;   //Count empty lines?
    BOOL                bCountInFlys;       //Count also within FlyFrames?
    BOOL                bRestartEachPage;   //Restart counting at the first paragraph of each page
                                            //(even on follows when paragraphs are splitted)

public:
    SwLineNumberInfo();
    SwLineNumberInfo(const SwLineNumberInfo&);

    SwLineNumberInfo& operator=(const SwLineNumberInfo&);
    BOOL operator==( const SwLineNumberInfo& rInf ) const;

    SwCharFmt *GetCharFmt( IDocumentStylePoolAccess& rIDSPA ) const;
    void SetCharFmt( SwCharFmt* );

    const SvxNumberType &GetNumType() const             { return aType; }
    void                SetNumType( SvxNumberType aNew ){ aType = aNew; }

    const String &GetDivider() const    { return aDivider; }
    void SetDivider( const String &r )  { aDivider = r; }
    USHORT GetDividerCountBy() const    { return nDividerCountBy; }
    void SetDividerCountBy( USHORT n )  { nDividerCountBy = n; }

    USHORT GetPosFromLeft() const       { return nPosFromLeft; }
    void   SetPosFromLeft( USHORT n)    { nPosFromLeft = n;    }

    USHORT GetCountBy() const           { return nCountBy; }
    void   SetCountBy( USHORT n)        { nCountBy = n;    }

    LineNumberPosition GetPos() const   { return ePos; }
    void SetPos( LineNumberPosition eP ){ ePos = eP;   }

    BOOL   IsPaintLineNumbers() const   { return bPaintLineNumbers; }
    void   SetPaintLineNumbers( BOOL b ){ bPaintLineNumbers = b;    }

    BOOL   IsCountBlankLines() const    { return bCountBlankLines;  }
    void   SetCountBlankLines( BOOL b ) { bCountBlankLines = b;     }

    BOOL   IsCountInFlys() const        { return bCountInFlys;      }
    void   SetCountInFlys( BOOL b )     { bCountInFlys = b;         }

    BOOL   IsRestartEachPage() const    { return bRestartEachPage;  }
    void   SetRestartEachPage( BOOL b ) { bRestartEachPage = b;     }

    virtual void Modify( SfxPoolItem*, SfxPoolItem* );
};



#endif

