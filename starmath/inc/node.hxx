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

#ifndef NODE_HXX
#define NODE_HXX

#include <vector>
#include <fstream>
#include <iostream>
#include <stdio.h>

#include "parse.hxx"
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

class SmVisitor;
class SmDocShell;
class SmNode;
class SmStructureNode;

typedef std::vector< SmNode * > SmNodeArray;
typedef std::vector< SmStructureNode * > SmStructureNodeArray;


////////////////////////////////////////////////////////////////////////////////

enum SmScaleMode    { SCALE_NONE, SCALE_WIDTH, SCALE_HEIGHT };

enum SmNodeType
{
    NTABLE,         NBRACE,         NBRACEBODY,     NOPER,          NALIGN,
    NATTRIBUT,      NFONT,          NUNHOR,         NBINHOR,        NBINVER,
    NBINDIAGONAL,   NSUBSUP,        NMATRIX,        NPLACE,         NTEXT,
    NSPECIAL,       NGLYPH_SPECIAL, NMATH,          NBLANK,         NERROR,
    NLINE,          NEXPRESSION,    NPOLYLINE,      NROOT,          NROOTSYMBOL,
    NRECTANGLE,     NVERTICAL_BRACE
};


////////////////////////////////////////////////////////////////////////////////


class SmNode : public SmRect
{
    SmFace      aFace;

    SmToken     aNodeToken;
    SmNodeType      eType;
    SmScaleMode     eScaleMode;
    RectHorAlign    eRectHorAlign;
    sal_uInt16          nFlags,
                    nAttributes;
    bool            bIsPhantom,
                    bIsDebug;

    bool            bIsSelected;

protected:
    SmNode(SmNodeType eNodeType, const SmToken &rNodeToken);

    // index in accessible text -1 if not (yet) applicable
    sal_Int32       nAccIndex;

public:
    virtual             ~SmNode();

    virtual bool        IsVisible() const;

    virtual sal_uInt16      GetNumSubNodes() const;
    virtual SmNode *    GetSubNode(sal_uInt16 nIndex);
            const SmNode * GetSubNode(sal_uInt16 nIndex) const
            {
                return ((SmNode *) this)->GetSubNode(nIndex);
            }

    virtual SmNode *       GetLeftMost();
            const SmNode * GetLeftMost() const
            {
                return ((SmNode *) this)->GetLeftMost();
            }

            sal_uInt16 &    Flags() { return nFlags; }
            sal_uInt16 &    Attributes() { return nAttributes; }

            bool IsDebug() const { return bIsDebug; }
            bool IsPhantom() const { return bIsPhantom; }
            void SetPhantom(bool bIsPhantom);
            void SetColor(const Color &rColor);

            void SetAttribut(sal_uInt16 nAttrib);
            void ClearAttribut(sal_uInt16 nAttrib);

            const SmFace & GetFont() const { return aFace; };
                  SmFace & GetFont()       { return aFace; };

            void SetFont(const SmFace &rFace);
            void SetFontSize(const Fraction &rRelSize, sal_uInt16 nType);
            void SetSize(const Fraction &rScale);

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void PrepareAttributes();

#if OSL_DEBUG_LEVEL
            void ToggleDebug() const;
#endif

    void         SetRectHorAlign(RectHorAlign eHorAlign, bool bApplyToSubTree = true );
    RectHorAlign GetRectHorAlign() const { return eRectHorAlign; }

    const SmRect & GetRect() const { return *this; }
          SmRect & GetRect()       { return *this; }

    virtual void Move(const Point &rPosition);
    void MoveTo(const Point &rPosition) { Move(rPosition - GetTopLeft()); }
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual void CreateTextFromNode(String &rText);

    virtual void    GetAccessibleText( String &rText ) const;
    sal_Int32       GetAccessibleIndex() const { return nAccIndex; }
    const SmNode *  FindNodeWithAccessibleIndex(xub_StrLen nAccIndex) const;

    sal_uInt16  GetRow() const    { return (sal_uInt16)aNodeToken.nRow; }
    sal_uInt16  GetColumn() const { return (sal_uInt16)aNodeToken.nCol; }

    SmScaleMode     GetScaleMode() const { return eScaleMode; }
    void            SetScaleMode(SmScaleMode eMode) { eScaleMode = eMode; }

    virtual void AdaptToX(const OutputDevice &rDev, sal_uLong nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, sal_uLong nHeight);

    SmNodeType      GetType() const  { return eType; }
    const SmToken & GetToken() const { return aNodeToken; }

    const SmNode *  FindTokenAt(sal_uInt16 nRow, sal_uInt16 nCol) const;
    const SmNode *  FindRectClosestTo(const Point &rPoint) const;

    virtual long    GetFormulaBaseline() const;

    /** Accept a visitor
     * Calls the method for this class on the visitor
     */
    virtual void Accept(SmVisitor* pVisitor);

    /** True if this node is selected */
    bool IsSelected() const {return bIsSelected;}
    void SetSelected(bool Selected = true) {bIsSelected = Selected;}

#ifdef DEBUG_ENABLE_DUMPASDOT
    /** The tree as dot graph for graphviz, usable for debugging
     * Convert the output to a image using $ dot graph.gv -Tpng > graph.png
     */
    inline void DumpAsDot(std::ostream &out, String* label = NULL) const{
        int id = 0;
        DumpAsDot(out, label, -1, id, -1);
    }
#endif /* DEBUG_ENABLE_DUMPASDOT */

    /** Get the parent node of this node */
    SmStructureNode* GetParent(){ return aParentNode; }
    /** Set the parent node */
    void SetParent(SmStructureNode* parent){
        aParentNode = parent;
    }

    /** Get the index of a child node
     *
     * Returns -1, if pSubNode isn't a subnode of this.
     */
    int IndexOfSubNode(SmNode* pSubNode){
        sal_uInt16 nSize = GetNumSubNodes();
        for(sal_uInt16 i = 0; i < nSize; i++)
            if(pSubNode == GetSubNode(i))
                return i;
        return -1;
    }
    /** Set the token for this node */
    void SetToken(SmToken& token){
        aNodeToken = token;
    }
protected:
    /** Sets parent on children of this node */
    void ClaimPaternity(){
        SmNode* pNode;
        sal_uInt16  nSize = GetNumSubNodes();
        for (sal_uInt16 i = 0;  i < nSize;  i++)
            if (NULL != (pNode = GetSubNode(i)))
                pNode->SetParent((SmStructureNode*)this); //Cast is valid if we have children
    }
private:
    SmStructureNode* aParentNode;
    void DumpAsDot(std::ostream &out, String* label, int number, int& id, int parent) const;
};

////////////////////////////////////////////////////////////////////////////////

/** A simple auxiliary iterator class for SmNode
 *
 * Example of iteration over children of pMyNode:
 * \code
 *  //Node to iterate over:
 *  SmNode* pMyNode = 0;// A pointer from somewhere
 *  //The iterator:
 *  SmNodeIterator it(pMyNode);
 *  //The iteration:
 *  while(it.Next()) {
 *      it->SetSelected(true);
 *  }
 * \endcode
 */
class SmNodeIterator{
public:
    SmNodeIterator(SmNode* node, bool bReverse = false){
        pNode = node;
        nSize = pNode->GetNumSubNodes();
        nIndex = 0;
        pChildNode = NULL;
        bIsReverse = bReverse;
    }
    /** Get the subnode or NULL if none */
    SmNode* Next(){
        while(!bIsReverse && nIndex < nSize){
            if(NULL != (pChildNode = pNode->GetSubNode(nIndex++)))
                return pChildNode;
        }
        while(bIsReverse && nSize > 0){
            if(NULL != (pChildNode = pNode->GetSubNode((nSize--)-1)))
                return pChildNode;
        }
        pChildNode = NULL;
        return NULL;
    }
    /** Get the current child node, NULL if none */
    SmNode* Current(){
        return pChildNode;
    }
    /** Get the current child node, NULL if none */
    SmNode* operator->(){
        return pChildNode;
    }
private:
    /** Current child */
    SmNode* pChildNode;
    /** Node whos children we're iterating over */
    SmNode* pNode;
    /** Size of the node */
    sal_uInt16 nSize;
    /** Current index in the node */
    sal_uInt16 nIndex;
    /** Move reverse */
    bool bIsReverse;
};

////////////////////////////////////////////////////////////////////////////////

/** Abstract baseclass for all composite node
 *
 * Subclasses of this class can have subnodes. Nodes that doesn't derivate from
 * this class does not have subnodes.
 */
class SmStructureNode : public SmNode
{
    SmNodeArray  aSubNodes;

protected:
    SmStructureNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmNode(eNodeType, rNodeToken)
    {}

public:
            SmStructureNode( const SmStructureNode &rNode );
    virtual ~SmStructureNode();

    virtual bool        IsVisible() const;

    virtual sal_uInt16      GetNumSubNodes() const;
            void        SetNumSubNodes(sal_uInt16 nSize) { aSubNodes.resize(nSize); }

    using   SmNode::GetSubNode;
    virtual SmNode *    GetSubNode(sal_uInt16 nIndex);
            void SetSubNodes(SmNode *pFirst, SmNode *pSecond, SmNode *pThird = NULL);
            void SetSubNodes(const SmNodeArray &rNodeArray);

    virtual SmStructureNode & operator = ( const SmStructureNode &rNode );

    virtual void  GetAccessibleText( String &rText ) const;

    void SetSubNode(size_t nIndex, SmNode* pNode)
    {
        size_t size = aSubNodes.size();
        if (size <= nIndex)
        {
            //Resize subnodes array
            aSubNodes.resize(nIndex + 1);
            //Set new slots to NULL
            for (size_t i = size; i < nIndex+1; i++)
                aSubNodes[i] = NULL;
        }
        aSubNodes[nIndex] = pNode;
        ClaimPaternity();
    }
};


////////////////////////////////////////////////////////////////////////////////

/** Abstract base class for all visible node
 *
 * Nodes that doesn't derivate from this class doesn't draw anything, but their
 * children.
 */
class SmVisibleNode : public SmNode
{
protected:
    SmVisibleNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmNode(eNodeType, rNodeToken)
    {}

public:

    virtual bool        IsVisible() const;
    virtual sal_uInt16      GetNumSubNodes() const;
    using   SmNode::GetSubNode;
    virtual SmNode *    GetSubNode(sal_uInt16 nIndex);
};


////////////////////////////////////////////////////////////////////////////////


class SmGraphicNode : public SmVisibleNode
{
protected:
    SmGraphicNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmVisibleNode(eNodeType, rNodeToken)
    {}

public:

    virtual void  GetAccessibleText( String &rText ) const;
};


////////////////////////////////////////////////////////////////////////////////

/** Draws a rectangle
 *
 * Used for drawing the line in the OVER and OVERSTRIKE commands.
 */
class SmRectangleNode : public SmGraphicNode
{
    Size  aToSize;

public:
    SmRectangleNode(const SmToken &rNodeToken)
    :   SmGraphicNode(NRECTANGLE, rNodeToken)
    {}

    virtual void AdaptToX(const OutputDevice &rDev, sal_uLong nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, sal_uLong nHeight);

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);

    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Polygon line node
 *
 * Used to draw the slash of the WIDESLASH command by SmBinDiagonalNode.
 */
class SmPolyLineNode : public SmGraphicNode
{
    Polygon     aPoly;
    Size        aToSize;
    long        nWidth;

public:
    SmPolyLineNode(const SmToken &rNodeToken);

    long         GetWidth() const { return nWidth; }
    Size         GetToSize() const { return aToSize; }
    Polygon     &GetPolygon() { return aPoly; }

    virtual void AdaptToX(const OutputDevice &rDev, sal_uLong nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, sal_uLong nHeight);

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);

    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Text node
 *
 * @remarks This class also serves as baseclass for all nodes that contains text.
 */
class SmTextNode : public SmVisibleNode
{
    XubString   aText;
    sal_uInt16      nFontDesc;
    /** Index within text where the selection starts
     * @remarks Only valid if SmNode::IsSelected() is true
     */
    xub_StrLen  nSelectionStart;
    /** Index within text where the selection ends
     * @remarks Only valid if SmNode::IsSelected() is true
     */
    xub_StrLen  nSelectionEnd;

protected:
    SmTextNode(SmNodeType eNodeType, const SmToken &rNodeToken, sal_uInt16 nFontDescP );

public:
    SmTextNode(const SmToken &rNodeToken, sal_uInt16 nFontDescP );

    sal_uInt16              GetFontDesc() const { return nFontDesc; }
    void                SetText(const XubString &rText) { aText = rText; }
    const XubString &   GetText() const { return aText; }
    /** Change the text of this node, including the underlying token */
    void                ChangeText(const XubString &rText) {
        aText = rText;
        SmToken token = GetToken();
        token.aText = rText;
        SetToken(token); //TODO: Merge this with AdjustFontDesc for better performance
        AdjustFontDesc();
    }
    /** Try to guess the correct FontDesc, used during visual editing */
    void                AdjustFontDesc();
    /** Index within GetText() where the selection starts
     * @remarks Only valid of SmNode::IsSelected() is true
     */
    xub_StrLen          GetSelectionStart() const {return nSelectionStart;}
    /** Index within GetText() where the selection end
     * @remarks Only valid of SmNode::IsSelected() is true
     */
    xub_StrLen          GetSelectionEnd() const {return nSelectionEnd;}
    /** Set the index within GetText() where the selection starts */
    void                SetSelectionStart(xub_StrLen index) {nSelectionStart = index;}
    /** Set the index within GetText() where the selection end */
    void                SetSelectionEnd(xub_StrLen index) {nSelectionEnd = index;}

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual void CreateTextFromNode(String &rText);

    virtual void  GetAccessibleText( String &rText ) const;
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Special node for user defined characters
 *
 * Node used for pre- and user-defined characters from:
 * officecfg/registry/data/org/openoffice/Office/Math.xcu
 *
 * This is just single characters, I think.
 */
class SmSpecialNode : public SmTextNode
{
    bool    bIsFromGreekSymbolSet;

protected:
    SmSpecialNode(SmNodeType eNodeType, const SmToken &rNodeToken, sal_uInt16 _nFontDesc);

public:
    SmSpecialNode(const SmToken &rNodeToken);

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);

    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Glyph node for custom operators
 *
 * This node is used with commands: oper, uoper and boper.
 * E.g. in "A boper op B", "op" will be an instance of SmGlyphSpecialNode.
 * "boper" simply inteprets "op", the following token, as an binary operator.
 * The command "uoper" interprets the following token as unary operator.
 * For these commands an instance of SmGlyphSpecialNode is used for the
 * operator token, following the command.
 */
class SmGlyphSpecialNode : public SmSpecialNode
{
public:
    SmGlyphSpecialNode(const SmToken &rNodeToken)
    :   SmSpecialNode(NGLYPH_SPECIAL, rNodeToken, FNT_MATH)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Math symbol node
 *
 * Use for math symbols such as plus, minus and integrale in the INT command.
 */
class SmMathSymbolNode : public SmSpecialNode
{
protected:
    SmMathSymbolNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmSpecialNode(eNodeType, rNodeToken, FNT_MATH)
    {
        xub_Unicode cChar = GetToken().cMathChar;
        if ((xub_Unicode) '\0' != cChar)
            SetText( cChar );
    }

public:
    SmMathSymbolNode(const SmToken &rNodeToken);

    virtual void AdaptToX(const OutputDevice &rDev, sal_uLong nWidth);
    virtual void AdaptToY(const OutputDevice &rDev, sal_uLong nHeight);

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Root symbol node
 *
 * Root symbol node used by SmRootNode to create the root symbol, infront of
 * the line with the line above. I don't think this node should be used for
 * anything else.
 */
class SmRootSymbolNode : public SmMathSymbolNode
{
    sal_uLong  nBodyWidth;  // width of body (argument) of root sign

public:
    SmRootSymbolNode(const SmToken &rNodeToken)
    :   SmMathSymbolNode(NROOTSYMBOL, rNodeToken)
    {}

    sal_uLong GetBodyWidth() const {return nBodyWidth;};
    virtual void AdaptToX(const OutputDevice &rDev, sal_uLong nHeight);
    virtual void AdaptToY(const OutputDevice &rDev, sal_uLong nHeight);

    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Place node
 *
 * Used to create the <?> command, that denotes place where something can be
 * written.
 * It is drawn as a square with a shadow.
 */
class SmPlaceNode : public SmMathSymbolNode
{
public:
    SmPlaceNode(const SmToken &rNodeToken)
    :   SmMathSymbolNode(NPLACE, rNodeToken)
    {
    }
    SmPlaceNode() : SmMathSymbolNode(NPLACE, SmToken(TPLACE, MS_PLACE, "<?>")) {};

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Error node, for parsing errors
 *
 * This node is used for parsing errors and draws an questionmark turned upside
 * down (inverted question mark).
 */
class SmErrorNode : public SmMathSymbolNode
{
public:
    SmErrorNode(SmParseError /*eError*/, const SmToken &rNodeToken)
    :   SmMathSymbolNode(NERROR, rNodeToken)
    {
        SetText((xub_Unicode) MS_ERROR);
    }

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Table node
 *
 * This is the root node for the formula tree. This node is also used for the
 * STACK and BINOM commands. When used for root node, its
 * children are instances of SmLineNode, and in some obscure cases the a child
 * can be an instance of SmExpressionNode, mainly when errors occur.
 */
class SmTableNode : public SmStructureNode
{
    long nFormulaBaseline;
public:
    SmTableNode(const SmToken &rNodeToken)
    :   SmStructureNode(NTABLE, rNodeToken)
    {}

    using   SmNode::GetLeftMost;
    virtual SmNode * GetLeftMost();

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    virtual long GetFormulaBaseline() const;

    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** A line
 *
 * Used as child of SmTableNode when the SmTableNode is the root node of the
 * formula tree.
 */
class SmLineNode : public SmStructureNode
{
    bool  bUseExtraSpaces;

protected:
    SmLineNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmStructureNode(eNodeType, rNodeToken)
    {
        bUseExtraSpaces = true;
    }

public:
    SmLineNode(const SmToken &rNodeToken)
    :   SmStructureNode(NLINE, rNodeToken)
    {
        bUseExtraSpaces = true;
    }

    void  SetUseExtraSpaces(bool bVal) { bUseExtraSpaces = bVal; }
    bool  IsUseExtraSpaces() const { return bUseExtraSpaces; };

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Expression node
 *
 * Used whenever you have an expression such as "A OVER {B + C}", here there is
 * an expression node that allows "B + C" to be the denominator of the
 * SmBinVerNode, that the OVER command creates.
 */
class SmExpressionNode : public SmLineNode
{
public:
    SmExpressionNode(const SmToken &rNodeToken)
    :   SmLineNode(NEXPRESSION, rNodeToken)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Unary horizontical node
 *
 * The same as SmBinHorNode except this is for unary operators.
 */
class SmUnHorNode : public SmStructureNode
{
public:
    SmUnHorNode(const SmToken &rNodeToken)
    :   SmStructureNode(NUNHOR, rNodeToken)
    {
        SetNumSubNodes(2);
    }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Root node
 *
 * Used for create square roots and other roots, example:
 * \f$ \sqrt[\mbox{[Argument]}]{\mbox{[Body]}} \f$.
 *
 * Children:<BR>
 * 0: Argument (optional)<BR>
 * 1: Symbol (instance of SmRootSymbolNode)<BR>
 * 2: Body<BR>
 * Where argument is optinal and may be NULL.
 */
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
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Binary horizontial node
 *
 * This node is used for binary operators. In a formula such as "A + B".
 *
 * Children:<BR>
 * 0: Left operand<BR>
 * 1: Binary operator<BR>
 * 2: Right operand<BR>
 *
 * None of the children may be NULL.
 */
class SmBinHorNode : public SmStructureNode
{
public:
    SmBinHorNode(const SmToken &rNodeToken)
    :   SmStructureNode(NBINHOR, rNodeToken)
    {
        SetNumSubNodes(3);
    }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Binary horizontical node
 *
 * This node is used for creating the OVER command, consider the formula:
 * "numerator OVER denominator", which looks like
 * \f$ \frac{\mbox{numerator}}{\mbox{denominator}} \f$
 *
 * Children:<BR>
 * 0: Numerator<BR>
 * 1: Line (instance of SmRectangleNode)<BR>
 * 2: Denominator<BR>
 * None of the children may be NULL.
 */
class SmBinVerNode : public SmStructureNode
{
public:
    SmBinVerNode(const SmToken &rNodeToken)
    :   SmStructureNode(NBINVER, rNodeToken)
    {
        SetNumSubNodes(3);
    }

    using   SmNode::GetLeftMost;
    virtual SmNode * GetLeftMost();

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Binary diagonal node
 *
 * Used for implementing the WIDESLASH command, example: "A WIDESLASH B".
 *
 * Children:<BR>
 * 0: Left operand<BR>
 * 1: right operand<BR>
 * 2: Line (instance of SmPolyLineNode).<BR>
 * None of the children may be NULL.
 */
class SmBinDiagonalNode : public SmStructureNode
{
    bool    bAscending;

    void    GetOperPosSize(Point &rPos, Size &rSize,
                           const Point &rDiagPoint, double fAngleDeg) const;

public:
    SmBinDiagonalNode(const SmToken &rNodeToken);

    bool    IsAscending() const { return bAscending; }
    void    SetAscending(bool bVal)  { bAscending = bVal; }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////


/** Enum used to index sub-/supscripts in the 'aSubNodes' array
 * in 'SmSubSupNode'
 *
 * See graphic for positions at char:
 *
 * \code
 *      CSUP
 *
 * LSUP H  H RSUP
 *      H  H
 *      HHHH
 *      H  H
 * LSUB H  H RSUB
 *
 *      CSUB
 * \endcode
 */
enum SmSubSup
{   CSUB, CSUP, RSUB, RSUP, LSUB, LSUP
};

/** numbers of entries in the above enum (that is: the number of possible
 * sub-/supscripts)
 */
#define SUBSUP_NUM_ENTRIES 6

/** Super- and subscript node
 *
 * Used for creating super- and subscripts for commands such as:
 * "^", "_", "lsup", "lsub", "csup" and "csub".
 * Example: "A^2" which looks like: \f$ A^2 \f$
 *
 * This node is also used for creating limits on SmOperNode, when
 * "FROM" and "TO" commands are used with "INT", "SUM" or similar.
 *
 * Children of this node can be enumerated using the SmSubSup enum.
 * Please note that children may be NULL, except for the body.
 * It is recommended that you access children using GetBody() and
 * GetSubSup().
 */
class SmSubSupNode : public SmStructureNode
{
    bool  bUseLimits;

public:
    SmSubSupNode(const SmToken &rNodeToken)
    :   SmStructureNode(NSUBSUP, rNodeToken)
    {
        SetNumSubNodes(1 + SUBSUP_NUM_ENTRIES);
        bUseLimits = false;
    }

    /** Get body (Not NULL) */
    SmNode *       GetBody()    { return GetSubNode(0); }
    /** Get body (Not NULL) */
    const SmNode * GetBody() const
    {
        return ((SmSubSupNode *) this)->GetBody();
    }

    void  SetUseLimits(bool bVal) { bUseLimits = bVal; }
    bool  IsUseLimits() const { return bUseLimits; };

    /** Get super- or subscript
     * @remarks this method may return NULL.
     */
    SmNode * GetSubSup(SmSubSup eSubSup) { return GetSubNode( sal::static_int_cast< sal_uInt16 >(1 + eSubSup) ); };

    /** Set the body */
    void SetBody(SmNode* pBody) { SetSubNode(0, pBody); }
    void SetSubSup(SmSubSup eSubSup, SmNode* pScript) { SetSubNode( 1 + eSubSup, pScript); }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);

};


////////////////////////////////////////////////////////////////////////////////

/** Node for brace construction
 *
 * Used for "lbrace [body] rbrace" and similar constructions.
 * Should look like \f$ \{\mbox{[body]}\} \f$
 *
 * Children:<BR>
 * 0: Opening brace<BR>
 * 1: Body (usually SmBracebodyNode)<BR>
 * 2: Closing brace<BR>
 * None of the children can be NULL.
 *
 * Note that child 1 (Body) is usually SmBracebodyNode, I don't know if it can
 * be an SmExpressionNode, haven't seen the case. But didn't quite read parser.cxx
 * enought to exclude this possibility.
 */
class SmBraceNode : public SmStructureNode
{
public:
    SmBraceNode(const SmToken &rNodeToken)
    :   SmStructureNode(NBRACE, rNodeToken)
    {
        SetNumSubNodes(3);
    }

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Body of an SmBraceNode
 *
 * This usually only has one child an SmExpressionNode, however, it can also
 * have other children.
 * Consider the formula "lbrace [body1] mline [body2] rbrace", looks like:
 * \f$ \{\mbox{[body1] | [body2]}\} \f$.
 * In this case SmBracebodyNode will have three children, "[body1]", "|" and
 * [body2].
 */
class SmBracebodyNode : public SmStructureNode
{
    long  nBodyHeight;

public:
    inline SmBracebodyNode(const SmToken &rNodeToken);

    virtual void    Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    long            GetBodyHeight() const { return nBodyHeight; }
    void Accept(SmVisitor* pVisitor);
};


inline SmBracebodyNode::SmBracebodyNode(const SmToken &rNodeToken) :
    SmStructureNode(NBRACEBODY, rNodeToken)
{
    nBodyHeight = 0;
}


////////////////////////////////////////////////////////////////////////////////

/** Node for vertical brace construction
 *
 * Used to implement commands "[body] underbrace [script]" and
 * "[body] overbrace [script]".
 * Underbrace should look like this \f$ \underbrace{\mbox{body}}_{\mbox{script}}\f$.
 *
 * Children:<BR>
 * 0: body<BR>
 * 1: brace<BR>
 * 2: script<BR>
 * (None of these children are optional, e.g. they must all be not NULL).
 */
class SmVerticalBraceNode : public SmStructureNode
{
public:
    inline SmVerticalBraceNode(const SmToken &rNodeToken);

    virtual void    Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


inline SmVerticalBraceNode::SmVerticalBraceNode(const SmToken &rNodeToken) :
    SmStructureNode(NVERTICAL_BRACE, rNodeToken)
{
    SetNumSubNodes(3);
}


////////////////////////////////////////////////////////////////////////////////


/** Operation Node
 *
 * Used for commands like SUM, INT and similar.
 *
 * Children:<BR>
 * 0: Operation (instance of SmMathSymbolNode)<BR>
 * 1: Body<BR>
 * None of the children may be NULL.
 *
 * If there are boundaries on the operation the body will an instance of
 * SmSubSupNode.
 */
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
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Node used for alignment
 */
class SmAlignNode : public SmStructureNode
{
public:
    SmAlignNode(const SmToken &rNodeToken)
    :   SmStructureNode(NALIGN, rNodeToken)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Attribute node
 *
 * Used to give an attribute to another node. Used for commands such as:
 * UNDERLINE, OVERLINE, OVERSTRIKE, WIDEVEC, WIDEHAT and WIDETILDE.
 *
 * Children:<BR>
 * 0: Attribute<BR>
 * 1: Body<BR>
 * None of these may be NULL.
 */
class SmAttributNode : public SmStructureNode
{
public:
    SmAttributNode(const SmToken &rNodeToken)
    :   SmStructureNode(NATTRIBUT, rNodeToken)
    {}

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Font node
 *
 * Used to change the font of it's children.
 */
class SmFontNode : public SmStructureNode
{
    sal_uInt16      nSizeType;
    Fraction    aFontSize;

public:
    SmFontNode(const SmToken &rNodeToken)
    :   SmStructureNode(NFONT, rNodeToken)
    {
        nSizeType = FNTSIZ_MULTIPLY;
        aFontSize = Fraction(1L);
    }

    void SetSizeParameter(const Fraction &rValue, sal_uInt16 nType);
    const Fraction & GetSizeParameter() const {return aFontSize;}
    const sal_uInt16& GetSizeType() const {return nSizeType;}

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Matrix node
 *
 * Used to implement the MATRIX command, example:
 * "matrix{ 1 # 2 ## 3 # 4}".
 */
class SmMatrixNode : public SmStructureNode
{
    sal_uInt16  nNumRows,
            nNumCols;

public:
    SmMatrixNode(const SmToken &rNodeToken)
    :   SmStructureNode(NMATRIX, rNodeToken)
    {
        nNumRows = nNumCols = 0;
    }

    sal_uInt16 GetNumRows() const {return nNumRows;}
    sal_uInt16 GetNumCols() const {return nNumCols;}
    void SetRowCol(sal_uInt16 nMatrixRows, sal_uInt16 nMatrixCols);

    using   SmNode::GetLeftMost;
    virtual SmNode * GetLeftMost();

    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void CreateTextFromNode(String &rText);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

/** Node for whitespace
 *
 * Used to implement the "~" command. This node is just a blank space.
 */
class SmBlankNode : public SmGraphicNode
{
    sal_uInt16  nNum;

public:
    SmBlankNode(const SmToken &rNodeToken)
    :   SmGraphicNode(NBLANK, rNodeToken)
    {
        nNum = 0;
    }

    void         IncreaseBy(const SmToken &rToken);
    void         Clear() { nNum = 0; }
    sal_uInt16       GetBlankNum() const { return nNum; }
    void         SetBlankNum(sal_uInt16 nNumber) { nNum = nNumber; }

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell);
    virtual void Arrange(const OutputDevice &rDev, const SmFormat &rFormat);
    void Accept(SmVisitor* pVisitor);
};


////////////////////////////////////////////////////////////////////////////////

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
