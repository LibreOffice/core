/*************************************************************************
 *
 *  $RCSfile: node.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:24 $
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


#ifndef NODE_HXX
#define NODE_HXX


#ifndef _DYNARY_HXX //autogen
#include <tools/dynary.hxx>
#endif

#include "parse.hxx"
#include "xchar.hxx"
#include "types.hxx"
#include "rect.hxx"
#include "format.hxx"


#define ATTR_BOLD       0x0001
#define ATTR_ITALIC     0x0002

#define FNTSIZ_ABSOLUT  1
#define FNTSIZ_PLUS     2
#define FNTSIZ_MINUS    3
#define FNTSIZ_MULTIPLY 4
#define FNTSIZ_DIVIDE   5

// flags to interdict respective status changes
#define FLG_FONT        0x0001
#define FLG_SIZE        0x0002
#define FLG_BOLD        0x0004
#define FLG_ITALIC      0x0008
#define FLG_COLOR       0x0010
#define FLG_VISIBLE     0x0020
#define FLG_HORALIGN    0x0040


extern SmFormat *pActiveFormat;

class SmNode;
DECLARE_DYNARRAY(SmNodeArray, SmNode *);

enum SmScaleMode    { SCALE_NONE, SCALE_WIDTH, SCALE_HEIGHT };

enum SmNodeType
{
    NTABLE,         NBRACE,         NBRACEBODY,     NOPER,          NALIGN,
    NATTRIBUT,      NFONT,          NUNHOR,         NBINHOR,        NBINVER,
    NBINDIAGONAL,   NSUBSUP,        NMATRIX,        NPLACE,         NTEXT,
    NSPECIAL,       NGLYPH_SPECIAL, NMATH,          NBLANK,         NERROR,
    NLINE,          NEXPRESSION,    NPOLYGON,       NPOLYLINE,      NROOT,
    NROOTSYMBOL,    NRECTANGLE,     NVERTICAL_BRACE
};


////////////////////////////////////////////////////////////////////////////////


class SmNode : public SmRect
{
    SmFace  aFace;

    SmToken         aNodeToken;
    SmNodeType      eType;
    SmScaleMode     eScaleMode;
    RectHorAlign    eRectHorAlign;
    USHORT          nFlags,
                    nAttributes;
    BOOL            bIsPhantom,
                    bIsDebug;
protected:
    SmNode(SmNodeType eNodeType, const SmToken &rNodeToken);

public:
    virtual             ~SmNode();

    virtual BOOL        IsVisible() const = 0;

    virtual USHORT      GetNumSubNodes() const = 0;
    virtual SmNode *    GetSubNode(USHORT nIndex) = 0;
            const SmNode * GetSubNode(USHORT nIndex) const
            {
                return ((SmNode *) this)->GetSubNode(nIndex);
            }

    virtual SmNode *       GetLeftMost();
            const SmNode * GetLeftMost() const
            {
                return ((SmNode *) this)->GetLeftMost();
            }

            USHORT &    Flags() { return nFlags; }
            USHORT &    Attributes() { return nAttributes; }

            BOOL IsDebug() const { return bIsDebug; }
            BOOL IsPhantom() const { return bIsPhantom; }
            void SetPhantom(BOOL bIsPhantom);
            void SetColor(const Color &rColor);

            void SetAttribut(USHORT nAttrib);
            void ClearAttribut(USHORT nAttrib);

            const SmFace & GetFont() const { return aFace; };
                  SmFace & GetFont()       { return aFace; };

            void SetFont(const SmFace &rFace);
            void SetFontSize(const Fraction &rRelSize, USHORT nType);
            void SetSize(const Fraction &rScale);

    virtual void Prepare(const SmFormat &rFormat);
    virtual void PrepareAttributes();

            void ToggleDebug() const;

    void         SetRectHorAlign(RectHorAlign eHorAlign);
    RectHorAlign GetRectHorAlign() const { return eRectHorAlign; }

    const SmRect & GetRect() const { return *this; }
          SmRect & GetRect()       { return *this; }

    virtual void Move(const Point &rPosition);
    void MoveTo(const Point &rPosition) { Move(rPosition - GetTopLeft()); }
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);

    virtual void Draw(OutputDevice &rDev, const Point &rPosition) const;

    USHORT  GetRow() const    { return aNodeToken.nRow; }
    USHORT  GetColumn() const { return aNodeToken.nCol; }

    SmScaleMode     GetScaleMode() const { return eScaleMode; }
    void            SetScaleMode(SmScaleMode eMode) { eScaleMode = eMode; }

    virtual void AdaptToX(const OutputDevice &rDev, ULONG nWidth) {};
    virtual void AdaptToY(const OutputDevice &rDev, ULONG nHeight) {};

    SmNodeType      GetType() const  { return eType; }
    const SmToken & GetToken() const { return aNodeToken; }

    const SmNode *  FindTokenAt(USHORT nRow, USHORT nCol) const;
    const SmNode *  FindRectClosestTo(const Point &rPoint) const;
};


////////////////////////////////////////////////////////////////////////////////

class SmStructureNode;

DECLARE_DYNARRAY(SmStructureNodeArray, SmStructureNode *);

class SmStructureNode : public SmNode
{
    SmNodeArray  aSubNodes;

protected:
    SmStructureNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmNode(eNodeType, rNodeToken)
    {}

public:
    virtual ~SmStructureNode();

    virtual BOOL        IsVisible() const { return FALSE; }

    virtual USHORT      GetNumSubNodes() const { return aSubNodes.GetSize(); }
            void        SetNumSubNodes(USHORT nSize) { aSubNodes.SetSize(nSize); }

    virtual SmNode *    GetSubNode(USHORT nIndex) { return aSubNodes.Get(nIndex); }
            void SetSubNodes(SmNode *pFirst, SmNode *pSecond,
                                SmNode *pThird = NULL);
            void SetSubNodes(const SmNodeArray &rNodeArray);
};


////////////////////////////////////////////////////////////////////////////////


class SmVisibleNode : public SmNode
{
protected:
    SmVisibleNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmNode(eNodeType, rNodeToken)
    {}

public:

    virtual BOOL        IsVisible() const { return TRUE; }
    virtual USHORT      GetNumSubNodes() const { return 0; }
    virtual SmNode *    GetSubNode(USHORT nIndex) { return NULL; }
};


////////////////////////////////////////////////////////////////////////////////


class SmGraphicNode : public SmVisibleNode
{
protected:
    SmGraphicNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmVisibleNode(eNodeType, rNodeToken)
    {}

public:
};


////////////////////////////////////////////////////////////////////////////////


class SmRectangleNode : public SmGraphicNode
{
    Size  aToSize;

public:
    SmRectangleNode(const SmToken &rNodeToken)
    :   SmGraphicNode(NRECTANGLE, rNodeToken)
    {}

    virtual void AdaptToX(const OutputDevice &rDev, ULONG nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, ULONG nHeight);

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual void Draw(OutputDevice &rDev, const Point &rPosition) const;
};


////////////////////////////////////////////////////////////////////////////////


class SmPolyLineNode : public SmGraphicNode
{
    Polygon     aPoly;
    Size        aToSize;
    long        nWidth;

public:
    SmPolyLineNode(const SmToken &rNodeToken);

    long         GetWidth() const { return nWidth; }

    virtual void AdaptToX(const OutputDevice &rDev, ULONG nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, ULONG nHeight);

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual void Draw(OutputDevice &rDev, const Point &rPosition) const;
};


////////////////////////////////////////////////////////////////////////////////


class SmPolygonNode : public SmGraphicNode
{
    SmPolygon  aPolygon;
    Size       aToSize;

protected:
    SmPolygonNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmGraphicNode(eNodeType, rNodeToken),
        aPolygon(rNodeToken.cMathChar)
    {}

public:
    SmPolygonNode(const SmToken &rNodeToken)
    :   SmGraphicNode(NPOLYGON, rNodeToken),
        aPolygon(rNodeToken.cMathChar)
    {}

    SmPolygon &                 GetPolygon() { return aPolygon; }
    inline const SmPolygon &    GetPolygon() const;

    virtual void AdaptToX(const OutputDevice &rDev, ULONG nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, ULONG nHeight);

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual void Draw(OutputDevice &rDev, const Point &rPosition) const;
};

inline const SmPolygon & SmPolygonNode::GetPolygon() const
{
    return ((SmPolygonNode *) this)->GetPolygon();
}


////////////////////////////////////////////////////////////////////////////////


class SmRootSymbolNode : public SmPolygonNode
{
    ULONG  nBodyWidth;  // width of body (argument) of root sign

    void DrawBar(OutputDevice &rDev, const Point &rPosition) const;

public:
    SmRootSymbolNode(const SmToken &rNodeToken)
    :   SmPolygonNode(NROOTSYMBOL, rNodeToken)
    {}


    virtual void AdaptToX(const OutputDevice &rDev, ULONG nWidth) { nBodyWidth = nWidth; }
    virtual void AdaptToY(const OutputDevice &rDev, ULONG nHeight);

    virtual void Draw(OutputDevice &rDev, const Point &rPosition) const;
};


////////////////////////////////////////////////////////////////////////////////


class SmTextNode : public SmVisibleNode
{
    XubString  aText;
    USHORT    nFontDesc;

protected:
    SmTextNode(SmNodeType eNodeType, const SmToken &rNodeToken, USHORT nFontDescP)
    :   SmVisibleNode(eNodeType, rNodeToken)
    {
        nFontDesc = nFontDescP;
    }

public:
    SmTextNode(const SmToken &rNodeToken, USHORT nFontDescP)
    :   SmVisibleNode(NTEXT, rNodeToken)
    {
        nFontDesc = nFontDescP;
    }


    USHORT              GetFontDesc() const { return nFontDesc; }
    void                SetText(const XubString &rText) { aText = rText; }
    const XubString &   GetText() const { return aText; }

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual void Draw(OutputDevice &rDev, const Point &rPosition) const;
};


////////////////////////////////////////////////////////////////////////////////


class SmSpecialNode : public SmTextNode
{
protected:
    SmSpecialNode(SmNodeType eNodeType, const SmToken &rNodeToken, USHORT nFontDesc)
    :   SmTextNode(eNodeType, rNodeToken, nFontDesc)
    {}

public:
    SmSpecialNode(const SmToken &rNodeToken)
    :   SmTextNode(NSPECIAL, rNodeToken, FNT_MATH)  //! default Font nicht immer richtig
    {}

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual void Draw(OutputDevice &rDev, const Point &rPosition) const;
};


////////////////////////////////////////////////////////////////////////////////


class SmGlyphSpecialNode : public SmSpecialNode
{
public:
    SmGlyphSpecialNode(const SmToken &rNodeToken)
    :   SmSpecialNode(NGLYPH_SPECIAL, rNodeToken, FNT_MATH)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmMathSymbolNode : public SmSpecialNode
{
protected:
    SmMathSymbolNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmSpecialNode(eNodeType, rNodeToken, FNT_MATH)
    {}

public:
    SmMathSymbolNode(const SmToken &rNodeToken);

    virtual void AdaptToX(const OutputDevice &rDev, ULONG nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, ULONG nHeight);

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmPlaceNode : public SmMathSymbolNode
{
public:
    SmPlaceNode(const SmToken &rNodeToken)
    :   SmMathSymbolNode(NPLACE, rNodeToken)
    {
        SetText((xub_Unicode) MS_PLACE);
    }

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmErrorNode : public SmMathSymbolNode
{
public:
    SmErrorNode(SmParseError eError, const SmToken &rNodeToken)
    :   SmMathSymbolNode(NERROR, rNodeToken)
    {
        SetText((xub_Unicode) MS_ERROR);
    }

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmTableNode : public SmStructureNode
{
public:
    SmTableNode(const SmToken &rNodeToken)
    :   SmStructureNode(NTABLE, rNodeToken)
    {}

    virtual SmNode * GetLeftMost() { return this; }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmLineNode : public SmStructureNode
{
protected:
    SmLineNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmStructureNode(eNodeType, rNodeToken)
    {}

public:
    SmLineNode(const SmToken &rNodeToken)
    :   SmStructureNode(NLINE, rNodeToken)
    {}

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmExpressionNode : public SmLineNode
{
public:
    SmExpressionNode(const SmToken &rNodeToken)
    :   SmLineNode(NEXPRESSION, rNodeToken)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmUnHorNode : public SmStructureNode
{
public:
    SmUnHorNode(const SmToken &rNodeToken)
    :   SmStructureNode(NUNHOR, rNodeToken)
    {
        SetNumSubNodes(2);
    }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmRootNode : public SmStructureNode
{
protected:
    void   GetHeightVerOffset(const SmRect &rRect,
                              long &rHeight, long &rVerOffset) const;
    Point  GetExtraPos(const SmRect &rRootSymbol, const SmRect &rExtra) const;

public:
    SmRootNode(const SmToken &rNodeToken)
    :   SmStructureNode(NROOT, rNodeToken)
    {
        SetNumSubNodes(3);
    }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmBinHorNode : public SmStructureNode
{
public:
    SmBinHorNode(const SmToken &rNodeToken)
    :   SmStructureNode(NBINHOR, rNodeToken)
    {
        SetNumSubNodes(3);
    }



    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmBinVerNode : public SmStructureNode
{
public:
    SmBinVerNode(const SmToken &rNodeToken)
    :   SmStructureNode(NBINVER, rNodeToken)
    {
        SetNumSubNodes(3);
    }

    virtual SmNode * GetLeftMost() { return this; }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmBinDiagonalNode : public SmStructureNode
{
    BOOL    bAscending;

    void    GetOperPosSize(Point &rPos, Size &rSize,
                           const Point &rDiagPoint, double fAngleDeg) const;

public:
    SmBinDiagonalNode(const SmToken &rNodeToken);

    BOOL    IsAscending() const { return bAscending; }
    void    SetAscending(BOOL bVal)  { bAscending = bVal; }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


// enums used to index sub-/supscripts in the 'aSubNodes' array
// in 'SmSubSupNode'
// See graphic for positions at char:
//
//      CSUP
//
// LSUP H  H RSUP
//      H  H
//      HHHH
//      H  H
// LSUB H  H RSUB
//
//      CSUB
//
enum SmSubSup
{   CSUB, CSUP, RSUB, RSUP, LSUB, LSUP
};

// numbers of entries in the above enum (that is: the number of possible
// sub-/supscripts)
#define SUBSUP_NUM_ENTRIES 6


class SmSubSupNode : public SmStructureNode
{
    BOOL  bUseLimits;

public:
    SmSubSupNode(const SmToken &rNodeToken)
    :   SmStructureNode(NSUBSUP, rNodeToken)
    {
        SetNumSubNodes(1 + SUBSUP_NUM_ENTRIES);
        bUseLimits = FALSE;
    }

    SmNode *       GetBody()    { return GetSubNode(0); }
    const SmNode * GetBody() const
    {
        return ((SmSubSupNode *) this)->GetBody();
    }

    void  SetUseLimits(BOOL bVal) { bUseLimits = bVal; }
    BOOL  IsUseLimits() const { return bUseLimits; };

    SmNode * GetSubSup(SmSubSup eSubSup) { return GetSubNode(1 + eSubSup); };

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmBraceNode : public SmStructureNode
{
public:
    SmBraceNode(const SmToken &rNodeToken)
    :   SmStructureNode(NBRACE, rNodeToken)
    {
        SetNumSubNodes(3);
    }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmBracebodyNode : public SmStructureNode
{
    long  nBodyHeight;

public:
    inline SmBracebodyNode(const SmToken &rNodeToken);

    virtual void    Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    long            GetBodyHeight() const { return nBodyHeight; }
};


inline SmBracebodyNode::SmBracebodyNode(const SmToken &rNodeToken)
:   SmStructureNode(NBRACEBODY, rNodeToken)
{
    nBodyHeight = 0;
}


////////////////////////////////////////////////////////////////////////////////


class SmVerticalBraceNode : public SmStructureNode
{
public:
    inline SmVerticalBraceNode(const SmToken &rNodeToken);

    virtual void    Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


SmVerticalBraceNode::SmVerticalBraceNode(const SmToken &rNodeToken)
:   SmStructureNode(NVERTICAL_BRACE, rNodeToken)
{
    SetNumSubNodes(3);
}


////////////////////////////////////////////////////////////////////////////////


class SmOperNode : public SmStructureNode
{
public:
    SmOperNode(const SmToken &rNodeToken)
    :   SmStructureNode(NOPER, rNodeToken)
    {
        SetNumSubNodes(2);
    }

    SmNode *       GetSymbol();
    const SmNode * GetSymbol() const
    {
        return ((SmOperNode *) this)->GetSymbol();
    }

    long CalcSymbolHeight(const SmNode &rSymbol, const SmFormat &rFormat) const;

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmAlignNode : public SmStructureNode
{
public:
    SmAlignNode(const SmToken &rNodeToken)
    :   SmStructureNode(NALIGN, rNodeToken)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmAttributNode : public SmStructureNode
{
public:
    SmAttributNode(const SmToken &rNodeToken)
    :   SmStructureNode(NATTRIBUT, rNodeToken)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmFontNode : public SmStructureNode
{
    USHORT      nSizeType;
    Fraction    aFontSize;

public:
    SmFontNode(const SmToken &rNodeToken)
    :   SmStructureNode(NFONT, rNodeToken)
    {
        nSizeType = FNTSIZ_MULTIPLY;
        aFontSize = Fraction(1L);
    }

    void SetSizeParameter(const Fraction &rValue, USHORT nType);

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmMatrixNode : public SmStructureNode
{
    USHORT  nNumRows,
            nNumCols;

public:
    SmMatrixNode(const SmToken &rNodeToken)
    :   SmStructureNode(NMATRIX, rNodeToken)
    {
        nNumRows = nNumCols = 0;
    }

    USHORT GetNumRows() {return nNumRows;}
    USHORT GetNumCols() {return nNumCols;}
    void SetRowCol(USHORT nMatrixRows, USHORT nMatrixCols);

    virtual SmNode * GetLeftMost() { return this; }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////


class SmBlankNode : public SmGraphicNode
{
    USHORT  nNum;

public:
    SmBlankNode(const SmToken &rNodeToken)
    :   SmGraphicNode(NBLANK, rNodeToken)
    {
        nNum = 0;
    }

    void         IncreaseBy(const SmToken &rToken);
    void         Clear() { nNum = 0; }

    virtual void Prepare(const SmFormat &rFormat);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
};


////////////////////////////////////////////////////////////////////////////////

#endif


