/*************************************************************************
 *
 *  $RCSfile: editattr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-02 15:25:36 $
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

#ifndef _EDITATTR_HXX
#define _EDITATTR_HXX

#include <eeitemid.hxx>

class SvxFont;
class SvxFontItem;
class SvxWeightItem;
class SvxPostureItem;
class SvxShadowedItem;
class SvxEscapementItem;
class SvxContourItem;
class SvxCrossedOutItem;
class SvxUnderlineItem;
class SvxFontHeightItem;
class SvxFontWidthItem;
class SvxColorItem;
class SvxAutoKernItem;
class SvxKerningItem;
class SvxCharSetColorItem;
class SvxWordLineModeItem;
class SvxFieldItem;

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif


class SfxVoidItem;

#define CH_FEATURE_OLD  (BYTE)          0xFF
#define CH_FEATURE      (sal_Unicode)   0x01

// DEF_METRIC: Bei meinem Pool sollte immer die DefMetric bei
// GetMetric( nWhich ) ankommen!
// => Zum ermitteln der DefMetrik einfach ein GetMetric( 0 )
#define DEF_METRIC  0

// -------------------------------------------------------------------------
// class EditAttrib
// -------------------------------------------------------------------------
class EditAttrib
{
private:
            EditAttrib() {;}
            EditAttrib( const EditAttrib & ) {;}

protected:
    const SfxPoolItem*  pItem;

                        EditAttrib( const SfxPoolItem& rAttr );
    virtual             ~EditAttrib();

public:
    // RemoveFromPool muss immer vorm DTOR Aufruf erfolgen!!
    void                RemoveFromPool( SfxItemPool& rPool );

    USHORT              Which() const   { return pItem->Which(); }
    const SfxPoolItem*  GetItem() const { return pItem; }
};

// -------------------------------------------------------------------------
// class EditCharAttrib
// -------------------------------------------------------------------------
// bFeature: Attribut darf nicht expandieren/schrumfen, Laenge immer 1
// bEdge: Attribut expandiert nicht, wenn genau an der Kante expandiert werden soll
class EditCharAttrib : public EditAttrib
{
protected:

    USHORT              nStart;
    USHORT              nEnd;
    BOOL                bFeature    :1;
    BOOL                bEdge       :1;

public:
            EditCharAttrib( const SfxPoolItem& rAttr, USHORT nStart, USHORT nEnd );

    USHORT&         GetStart()                  { return nStart; }
    USHORT&         GetEnd()                    { return nEnd; }

    USHORT          GetStart() const            { return nStart; }
    USHORT          GetEnd() const              { return nEnd; }

    inline USHORT   GetLen() const;

    inline void     MoveForward( USHORT nDiff );
    inline void     MoveBackward( USHORT nDiff );

    inline void     Expand( USHORT nDiff );
    inline void     Collaps( USHORT nDiff );

    virtual void    SetFont( SvxFont& rFont );

    BOOL    IsIn( USHORT nIndex )
                { return ( ( nStart <= nIndex ) && ( nEnd >= nIndex ) ); }
    BOOL    IsInside( USHORT nIndex )
                { return ( ( nStart < nIndex ) && ( nEnd > nIndex ) ); }
    BOOL    IsEmpty()
                { return nStart == nEnd; }

    BOOL    IsFeature() const   { return bFeature; }
    void    SetFeature( BOOL b) { bFeature = b; }

    BOOL    IsEdge() const      { return bEdge; }
    void    SetEdge( BOOL b )   { bEdge = b; }
};

inline USHORT EditCharAttrib::GetLen() const
{
    DBG_ASSERT( nEnd >= nStart, "EditCharAttrib: nEnd < nStart!" );
    return nEnd-nStart;
}

inline void EditCharAttrib::MoveForward( USHORT nDiff )
{
    DBG_ASSERT( ((long)nEnd + nDiff) <= 0xFFFF, "EditCharAttrib: MoveForward?!" );
    nStart += nDiff;
    nEnd += nDiff;
}

inline void EditCharAttrib::MoveBackward( USHORT nDiff )
{
    DBG_ASSERT( ((long)nStart - nDiff) >= 0, "EditCharAttrib: MoveBackward?!" );
    nStart -= nDiff;
    nEnd -= nDiff;
}

inline void EditCharAttrib::Expand( USHORT nDiff )
{
    DBG_ASSERT( ( ((long)nEnd + nDiff) <= (long)0xFFFF ), "EditCharAttrib: Expand?!" );
    DBG_ASSERT( !bFeature, "Bitte keine Features expandieren!" );
    nEnd += nDiff;
}

inline void EditCharAttrib::Collaps( USHORT nDiff )
{
    DBG_ASSERT( (long)nEnd - nDiff >= (long)nStart, "EditCharAttrib: Collaps?!" );
    DBG_ASSERT( !bFeature, "Bitte keine Features schrumpfen!" );
    nEnd -= nDiff;
}

// -------------------------------------------------------------------------
// class EditCharAttribFont
// -------------------------------------------------------------------------
class EditCharAttribFont: public EditCharAttrib
{
public:
    EditCharAttribFont( const SvxFontItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribWeight
// -------------------------------------------------------------------------
class EditCharAttribWeight : public EditCharAttrib
{
public:
    EditCharAttribWeight( const SvxWeightItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};
// -------------------------------------------------------------------------
// class EditCharAttribItalic
// -------------------------------------------------------------------------
class EditCharAttribItalic : public EditCharAttrib
{
public:
    EditCharAttribItalic( const SvxPostureItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribShadow
// -------------------------------------------------------------------------
class EditCharAttribShadow : public EditCharAttrib
{
public:
    EditCharAttribShadow( const SvxShadowedItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribEscapement
// -------------------------------------------------------------------------
class EditCharAttribEscapement : public EditCharAttrib
{
public:
    EditCharAttribEscapement( const SvxEscapementItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribOutline
// -------------------------------------------------------------------------
class EditCharAttribOutline : public EditCharAttrib
{
public:
    EditCharAttribOutline( const SvxContourItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribStrikeout
// -------------------------------------------------------------------------
class EditCharAttribStrikeout : public EditCharAttrib
{
public:
    EditCharAttribStrikeout( const SvxCrossedOutItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribUnderline
// -------------------------------------------------------------------------
class EditCharAttribUnderline : public EditCharAttrib
{
public:
    EditCharAttribUnderline( const SvxUnderlineItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribFontHeight
// -------------------------------------------------------------------------
class EditCharAttribFontHeight : public EditCharAttrib
{
public:
    EditCharAttribFontHeight( const SvxFontHeightItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribFontWidth
// -------------------------------------------------------------------------
class EditCharAttribFontWidth : public EditCharAttrib
{
public:
    EditCharAttribFontWidth( const SvxFontWidthItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribColor
// -------------------------------------------------------------------------
class EditCharAttribColor : public EditCharAttrib
{
public:
    EditCharAttribColor( const SvxColorItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribTab
// -------------------------------------------------------------------------
class EditCharAttribTab : public EditCharAttrib
{
public:
    EditCharAttribTab( const SfxVoidItem& rAttr, USHORT nPos );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribLineBreak
// -------------------------------------------------------------------------
class EditCharAttribLineBreak : public EditCharAttrib
{
public:
    EditCharAttribLineBreak( const SfxVoidItem& rAttr, USHORT nPos );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribField
// -------------------------------------------------------------------------
class EditCharAttribField: public EditCharAttrib
{
    XubString       aFieldValue;
    Color*          pTxtColor;
    Color*          pFldColor;

    EditCharAttribField& operator = ( const EditCharAttribField& rAttr ) const;

public:
    EditCharAttribField( const SvxFieldItem& rAttr, USHORT nPos );
    EditCharAttribField( const EditCharAttribField& rAttr );
    ~EditCharAttribField();

    BOOL operator == ( const EditCharAttribField& rAttr ) const;
    BOOL operator != ( const EditCharAttribField& rAttr ) const
                                    { return !(operator == ( rAttr ) ); }

    virtual void    SetFont( SvxFont& rFont );
    Color*&         GetTxtColor()           { return pTxtColor; }
    Color*&         GetFldColor()           { return pFldColor; }

    const XubString&    GetFieldValue() const   { return aFieldValue; }
    XubString&      GetFieldValue()         { return aFieldValue; }

    void            Reset()
                    {
                        aFieldValue.Erase();
                        delete pTxtColor; pTxtColor = 0;
                        delete pFldColor; pFldColor = 0;
                    }
};

// -------------------------------------------------------------------------
// class EditCharAttribPairKerning
// -------------------------------------------------------------------------
class EditCharAttribPairKerning : public EditCharAttrib
{
public:
    EditCharAttribPairKerning( const SvxAutoKernItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribKerning
// -------------------------------------------------------------------------
class EditCharAttribKerning : public EditCharAttrib
{
public:
    EditCharAttribKerning( const SvxKerningItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};

// -------------------------------------------------------------------------
// class EditCharAttribWordLineMode
// -------------------------------------------------------------------------
class EditCharAttribWordLineMode: public EditCharAttrib
{
public:
    EditCharAttribWordLineMode( const SvxWordLineModeItem& rAttr, USHORT nStart, USHORT nEnd );

    virtual void    SetFont( SvxFont& rFont );
};


#endif // _EDITATTR_HXX
