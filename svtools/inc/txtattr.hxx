/*************************************************************************
 *
 *  $RCSfile: txtattr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
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

#ifndef _TXTATTR_HXX
#define _TXTATTR_HXX

#ifndef _SV_COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

class Font;

#define TEXTATTR_INVALID    0
#define TEXTATTR_FONTCOLOR  1
#define TEXTATTR_HYPERLINK  2



class TextAttrib
{
private:
    USHORT                  mnWhich;

protected:
                            TextAttrib( USHORT nWhich ) { mnWhich = nWhich; }
                            TextAttrib( const TextAttrib& rAttr ) { mnWhich = rAttr.mnWhich; }

public:

    virtual                 ~TextAttrib();

    USHORT                  Which() const   { return mnWhich; }

    virtual void            SetFont( Font& rFont ) const = 0;
    virtual TextAttrib*     Clone() const = 0;
    virtual int             operator==( const TextAttrib& rAttr ) const = 0;
    int                     operator!=( const TextAttrib& rAttr ) const
                                { return !(*this == rAttr ); }
};



class TextAttribFontColor : public TextAttrib
{
private:
    Color   maColor;

public:
                            TextAttribFontColor( const Color& rColor );
                            TextAttribFontColor( const TextAttribFontColor& rAttr );
                            ~TextAttribFontColor();

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};



class TextAttribHyperLink : public TextAttrib
{
private:
    XubString   maURL;
    XubString   maDescription;
    Color       maColor;

public:
                            TextAttribHyperLink( const XubString& rURL );
                            TextAttribHyperLink( const XubString& rURL, const XubString& rDescription );
                            TextAttribHyperLink( const TextAttribHyperLink& rAttr );
                            ~TextAttribHyperLink();

    void                    SetURL( const XubString& rURL )             { maURL = rURL; }
    const XubString&            GetURL() const                              { return maURL; }

    void                    SetDescription( const XubString& rDescr )   { maDescription = rDescr; }
    const XubString&            GetDescription() const                      { return maDescription; }

    void                    SetColor( const Color& rColor )             { maColor = rColor; }
    const Color&            GetColor() const                            { return maColor; }

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};



class TextCharAttrib
{
private:
    TextAttrib*     mpAttr;
    USHORT          mnStart;
    USHORT          mnEnd;

protected:

public:

                    TextCharAttrib( const TextAttrib& rAttr, USHORT nStart, USHORT nEnd );
                    TextCharAttrib( const TextCharAttrib& rTextCharAttrib );
                    ~TextCharAttrib();

    const TextAttrib&   GetAttr() const         { return *mpAttr; }

    USHORT          Which() const               { return mpAttr->Which(); }

    USHORT          GetStart() const            { return mnStart; }
    USHORT&         GetStart()                  { return mnStart; }

    USHORT          GetEnd() const              { return mnEnd; }
    USHORT&         GetEnd()                    { return mnEnd; }

    inline USHORT   GetLen() const;

    inline void     MoveForward( USHORT nDiff );
    inline void     MoveBackward( USHORT nDiff );

    inline void     Expand( USHORT nDiff );
    inline void     Collaps( USHORT nDiff );

    inline BOOL     IsIn( USHORT nIndex );
    inline BOOL     IsInside( USHORT nIndex );
    inline BOOL     IsEmpty();

};

inline USHORT TextCharAttrib::GetLen() const
{
    DBG_ASSERT( mnEnd >= mnStart, "TextCharAttrib: nEnd < nStart!" );
    return mnEnd-mnStart;
}

inline void TextCharAttrib::MoveForward( USHORT nDiff )
{
    DBG_ASSERT( ((long)mnEnd + nDiff) <= 0xFFFF, "TextCharAttrib: MoveForward?!" );
    mnStart += nDiff;
    mnEnd += nDiff;
}

inline void TextCharAttrib::MoveBackward( USHORT nDiff )
{
    DBG_ASSERT( ((long)mnStart - nDiff) >= 0, "TextCharAttrib: MoveBackward?!" );
    mnStart -= nDiff;
    mnEnd -= nDiff;
}

inline void TextCharAttrib::Expand( USHORT nDiff )
{
    DBG_ASSERT( ( ((long)mnEnd + nDiff) <= (long)0xFFFF ), "TextCharAttrib: Expand?!" );
    mnEnd += nDiff;
}

inline void TextCharAttrib::Collaps( USHORT nDiff )
{
    DBG_ASSERT( (long)mnEnd - nDiff >= (long)mnStart, "TextCharAttrib: Collaps?!" );
    mnEnd -= nDiff;
}

inline BOOL TextCharAttrib::IsIn( USHORT nIndex )
{
    return ( ( mnStart <= nIndex ) && ( mnEnd >= nIndex ) );
}

inline BOOL TextCharAttrib::IsInside( USHORT nIndex )
{
    return ( ( mnStart < nIndex ) && ( mnEnd > nIndex ) );
}

inline BOOL TextCharAttrib::IsEmpty()
{
    return mnStart == mnEnd;
}

#endif // _TXTATTR_HXX
