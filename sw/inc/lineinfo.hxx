/*************************************************************************
 *
 *  $RCSfile: lineinfo.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:27 $
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
#ifndef SW_LINEINFO_HXX
#define SW_LINEINFO_HXX


#ifndef _CALBCK_HXX
#include "calbck.hxx"
#endif
#ifndef _NUMRULE_HXX
#include "numrule.hxx"
#endif

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
    SwNumType           aType;              //e.g. roman linenumbers
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

    SwCharFmt *GetCharFmt(SwDoc &rDoc) const;
    void SetCharFmt( SwCharFmt* );

    const SwNumType &GetNumType() const             { return aType; }
    void             SetNumType( SwNumType aNew )   { aType = aNew; }

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

