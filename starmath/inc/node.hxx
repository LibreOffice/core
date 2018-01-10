/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_STARMATH_INC_NODE_HXX
#define INCLUDED_STARMATH_INC_NODE_HXX

#include "types.hxx"
#include "token.hxx"
#include "rect.hxx"
#include "format.hxx"

#include <o3tl/typed_flags_set.hxx>

#include <cassert>
#include <vector>

enum class FontAttribute {
    None   = 0x0000,
    Bold   = 0x0001,
    Italic = 0x0002
};

namespace o3tl
{
    template<> struct typed_flags<FontAttribute> : is_typed_flags<FontAttribute, 0x0003> {};
}


enum class FontSizeType {
  ABSOLUT  = 1,
  PLUS     = 2,
  MINUS    = 3,
  MULTIPLY = 4,
  DIVIDE   = 5
};

// flags to interdict respective status changes
enum class FontChangeMask {
    None     = 0x0000,
    Face     = 0x0001,
    Size     = 0x0002,
    Bold     = 0x0004,
    Italic   = 0x0008,
    Color    = 0x0010,
    Phantom  = 0x0020,
    HorAlign = 0x0040
};

namespace o3tl
{
    template<> struct typed_flags<FontChangeMask> : is_typed_flags<FontChangeMask, 0x007f> {};
}


class SmVisitor;
class SmDocShell;
class SmNode;
class SmStructureNode;

typedef std::vector< SmNode * > SmNodeArray;

enum class SmScaleMode
{
    None,
    Width,
    Height
};

enum class SmNodeType
{
/* 0*/ Table,       Brace,         Bracebody,     Oper,        Align,
/* 5*/ Attribut,    Font,          UnHor,         BinHor,      BinVer,
/*10*/ BinDiagonal, SubSup,        Matrix,        Place,       Text,
/*15*/ Special,     GlyphSpecial,  Math,          Blank,       Error,
/*20*/ Line,        Expression,    PolyLine,      Root,        RootSymbol,
/*25*/ Rectangle,   VerticalBrace, MathIdent
};


class SmNode : public SmRect
{
    SmFace      maFace;

    SmToken     maNodeToken;
    SmNodeType      meType;
    SmScaleMode     meScaleMode;
    RectHorAlign    meRectHorAlign;
    FontChangeMask  mnFlags;
    FontAttribute   mnAttributes;
    bool            mbIsPhantom;
    bool            mbIsSelected;
    // index in accessible text; -1 if not (yet) applicable
    sal_Int32       mnAccIndex;

protected:
    SmNode(SmNodeType eNodeType, const SmToken &rNodeToken);

public:
    SmNode(const SmNode&) = delete;
    SmNode& operator=(const SmNode&) = delete;

    virtual             ~SmNode();

    /**
     * Returns true if this is a instance of SmVisibleNode's subclass, false otherwise.
     */
    virtual bool        IsVisible() const = 0;

    virtual size_t      GetNumSubNodes() const = 0;
    virtual SmNode *    GetSubNode(size_t nIndex) = 0;
            const SmNode * GetSubNode(size_t nIndex) const
            {
                return const_cast<SmNode *>(this)->GetSubNode(nIndex);
            }

    virtual const SmNode * GetLeftMost() const;

            FontChangeMask &Flags() { return mnFlags; }
            FontAttribute  &Attributes() { return mnAttributes; }

            bool IsPhantom() const { return mbIsPhantom; }
            void SetPhantom(bool bIsPhantom);
            void SetColor(const Color &rColor);

            void SetAttribut(FontAttribute nAttrib);
            void ClearAttribut(FontAttribute nAttrib);

            const SmFace & GetFont() const { return maFace; };
                  SmFace & GetFont()       { return maFace; };

            void SetFont(const SmFace &rFace);
            void SetFontSize(const Fraction &rRelSize, FontSizeType nType);
            void SetSize(const Fraction &rScale);

    /** Prepare preliminary settings about font and text
     *  (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth);
    void PrepareAttributes();

    void         SetRectHorAlign(RectHorAlign eHorAlign, bool bApplyToSubTree = true );
    RectHorAlign GetRectHorAlign() const { return meRectHorAlign; }

    const SmRect & GetRect() const { return *this; }

    void Move(const Point &rPosition);
    void MoveTo(const Point &rPosition) { Move(rPosition - GetTopLeft()); }
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) = 0;
    virtual void CreateTextFromNode(OUString &rText);

    virtual void    GetAccessibleText( OUStringBuffer &rText ) const = 0;
    sal_Int32       GetAccessibleIndex() const { return mnAccIndex; }
    void            SetAccessibleIndex(sal_Int32 nAccIndex) { mnAccIndex = nAccIndex; }
    const SmNode *  FindNodeWithAccessibleIndex(sal_Int32 nAccIndex) const;

    sal_uInt16  GetRow() const    { return sal::static_int_cast<sal_uInt16>(maNodeToken.nRow); }
    sal_uInt16  GetColumn() const { return sal::static_int_cast<sal_uInt16>(maNodeToken.nCol); }

    SmScaleMode     GetScaleMode() const { return meScaleMode; }
    void            SetScaleMode(SmScaleMode eMode) { meScaleMode = eMode; }

    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth);
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight);

    SmNodeType      GetType() const  { return meType; }
    const SmToken & GetToken() const { return maNodeToken; }

    const SmNode *  FindTokenAt(sal_uInt16 nRow, sal_uInt16 nCol) const;
    const SmNode *  FindRectClosestTo(const Point &rPoint) const;

    /** Accept a visitor
     * Calls the method for this class on the visitor
     */
    virtual void Accept(SmVisitor* pVisitor) = 0;

    /** True if this node is selected */
    bool IsSelected() const {return mbIsSelected;}
    void SetSelected(bool Selected) {mbIsSelected = Selected;}

    /** Get the parent node of this node */
    SmStructureNode* GetParent(){ return mpParentNode; }
    const SmStructureNode* GetParent() const { return mpParentNode; }
    /** Set the parent node */
    void SetParent(SmStructureNode* parent){
        mpParentNode = parent;
    }

    /** Set the token for this node */
    void SetToken(SmToken const & token){
        maNodeToken = token;
    }

private:
    SmStructureNode* mpParentNode;
};


/** Abstract baseclass for all composite node
 *
 * Subclasses of this class can have subnodes. Nodes that doesn't derivate from
 * this class does not have subnodes.
 */
class SmStructureNode : public SmNode
{
    SmNodeArray maSubNodes;

protected:
    SmStructureNode(SmNodeType eNodeType, const SmToken &rNodeToken, size_t nSize = 0)
        : SmNode(eNodeType, rNodeToken)
        , maSubNodes(nSize)
    {}

public:
    virtual ~SmStructureNode() override;

    virtual bool        IsVisible() const override;

    virtual size_t      GetNumSubNodes() const override;

    using   SmNode::GetSubNode;
    virtual SmNode *    GetSubNode(size_t nIndex) override;
            void SetSubNodes(SmNode *pFirst, SmNode *pSecond, SmNode *pThird = nullptr);
            void SetSubNodes(const SmNodeArray &rNodeArray);

    virtual void  GetAccessibleText( OUStringBuffer &rText ) const override;

    SmNodeArray::iterator begin() {return maSubNodes.begin();}
    SmNodeArray::iterator end() {return maSubNodes.end();}
    SmNodeArray::reverse_iterator rbegin() {return maSubNodes.rbegin();}
    SmNodeArray::reverse_iterator rend() {return maSubNodes.rend();}

    /** Get the index of a child node
     *
     * Returns -1, if pSubNode isn't a subnode of this.
     */
    int IndexOfSubNode(SmNode const * pSubNode)
    {
        size_t nSize = GetNumSubNodes();
        for (size_t i = 0; i < nSize; i++)
            if (pSubNode == GetSubNode(i))
                return i;
        return -1;
    }

    void SetSubNode(size_t nIndex, SmNode* pNode)
    {
        size_t size = maSubNodes.size();
        if (size <= nIndex)
        {
            //Resize subnodes array
            maSubNodes.resize(nIndex + 1);
            //Set new slots to NULL
            for (size_t i = size; i < nIndex+1; i++)
                maSubNodes[i] = nullptr;
        }
        maSubNodes[nIndex] = pNode;
        ClaimPaternity();
    }

private:
    /** Sets parent on children of this node */
    void ClaimPaternity();
};


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

    virtual bool        IsVisible() const override;
    virtual size_t      GetNumSubNodes() const override;
    using   SmNode::GetSubNode;
    virtual SmNode *    GetSubNode(size_t nIndex) override;
};


class SmGraphicNode : public SmVisibleNode
{
protected:
    SmGraphicNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmVisibleNode(eNodeType, rNodeToken)
    {}

public:

    virtual void  GetAccessibleText( OUStringBuffer &rText ) const override;
};


/** Draws a rectangle
 *
 * Used for drawing the line in the OVER and OVERSTRIKE commands.
 */
class SmRectangleNode : public SmGraphicNode
{
    Size maToSize;

public:
    explicit SmRectangleNode(const SmToken &rNodeToken)
        : SmGraphicNode(SmNodeType::Rectangle, rNodeToken)
    {}

    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Polygon line node
 *
 * Used to draw the slash of the WIDESLASH command by SmBinDiagonalNode.
 */
class SmPolyLineNode : public SmGraphicNode
{
    tools::Polygon maPoly;
    Size maToSize;
    long mnWidth;

public:
    explicit SmPolyLineNode(const SmToken &rNodeToken);

    long         GetWidth() const { return mnWidth; }
    tools::Polygon &GetPolygon() { return maPoly; }

    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    void Accept(SmVisitor* pVisitor) override;
};


/** Text node
 *
 * @remarks This class also serves as baseclass for all nodes that contains text.
 */
class SmTextNode : public SmVisibleNode
{
    OUString   maText;
    sal_uInt16 mnFontDesc;
    /** Index within text where the selection starts
     * @remarks Only valid if SmNode::IsSelected() is true
     */
    sal_Int32  mnSelectionStart;
    /** Index within text where the selection ends
     * @remarks Only valid if SmNode::IsSelected() is true
     */
    sal_Int32  mnSelectionEnd;

protected:
    SmTextNode(SmNodeType eNodeType, const SmToken &rNodeToken, sal_uInt16 nFontDescP );

public:
    SmTextNode(const SmToken &rNodeToken, sal_uInt16 nFontDescP );

    sal_uInt16              GetFontDesc() const { return mnFontDesc; }
    void                SetText(const OUString &rText) { maText = rText; }
    const OUString &    GetText() const { return maText; }
    /** Change the text of this node, including the underlying token */
    void                ChangeText(const OUString &rText) {
        maText = rText;
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
    sal_Int32           GetSelectionStart() const {return mnSelectionStart;}
    /** Index within GetText() where the selection end
     * @remarks Only valid of SmNode::IsSelected() is true
     */
    sal_Int32           GetSelectionEnd() const {return mnSelectionEnd;}
    /** Set the index within GetText() where the selection starts */
    void                SetSelectionStart(sal_Int32 index) {mnSelectionStart = index;}
    /** Set the index within GetText() where the selection end */
    void                SetSelectionEnd(sal_Int32 index) {mnSelectionEnd = index;}

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    virtual void CreateTextFromNode(OUString &rText) override;

    virtual void  GetAccessibleText( OUStringBuffer &rText ) const override;
    void Accept(SmVisitor* pVisitor) override;
    /**
      Converts the character from StarMath's private area symbols to a matching Unicode
      character, if necessary. To be used when converting GetText() to a normal text.
    */
    static sal_Unicode ConvertSymbolToUnicode(sal_Unicode nIn);
};


/** Special node for user defined characters
 *
 * Node used for pre- and user-defined characters from:
 * officecfg/registry/data/org/openoffice/Office/Math.xcu
 *
 * This is just single characters, I think.
 */
class SmSpecialNode : public SmTextNode
{
    bool mbIsFromGreekSymbolSet;

protected:
    SmSpecialNode(SmNodeType eNodeType, const SmToken &rNodeToken, sal_uInt16 _nFontDesc);

public:
    explicit SmSpecialNode(const SmToken &rNodeToken);

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    void Accept(SmVisitor* pVisitor) override;
};


/** Glyph node for custom operators
 *
 * This node is used with commands: oper, uoper and boper.
 * E.g. in "A boper op B", "op" will be an instance of SmGlyphSpecialNode.
 * "boper" simply interprets "op", the following token, as an binary operator.
 * The command "uoper" interprets the following token as unary operator.
 * For these commands an instance of SmGlyphSpecialNode is used for the
 * operator token, following the command.
 */
class SmGlyphSpecialNode : public SmSpecialNode
{
public:
    explicit SmGlyphSpecialNode(const SmToken &rNodeToken)
        : SmSpecialNode(SmNodeType::GlyphSpecial, rNodeToken, FNT_MATH)
    {}

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Math symbol node
 *
 * Use for math symbols such as plus, minus and integral in the INT command.
 */
class SmMathSymbolNode : public SmSpecialNode
{
protected:
    SmMathSymbolNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmSpecialNode(eNodeType, rNodeToken, FNT_MATH)
    {
        sal_Unicode cChar = GetToken().cMathChar;
        if (u'\0' != cChar)
            SetText(OUString(cChar));
    }

public:
    explicit SmMathSymbolNode(const SmToken &rNodeToken);

    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Math Identifier
 *
 * This behaves essentially the same as SmMathSymbolNode and is only used to
 * represent math symbols that should be exported as <mi> elements rather than
 * <mo> elements.
 */
class SmMathIdentifierNode : public SmMathSymbolNode
{
public:
    explicit SmMathIdentifierNode(const SmToken &rNodeToken)
        : SmMathSymbolNode(SmNodeType::MathIdent, rNodeToken) {}
};


/** Root symbol node
 *
 * Root symbol node used by SmRootNode to create the root symbol, in front of
 * the line with the line above. I don't think this node should be used for
 * anything else.
 */
class SmRootSymbolNode : public SmMathSymbolNode
{
    sal_uLong mnBodyWidth;  // width of body (argument) of root sign

public:
    explicit SmRootSymbolNode(const SmToken &rNodeToken)
        : SmMathSymbolNode(SmNodeType::RootSymbol, rNodeToken)
        , mnBodyWidth(0)
    {
    }

    sal_uLong GetBodyWidth() const {return mnBodyWidth;};
    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nHeight) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    void Accept(SmVisitor* pVisitor) override;
};


/** Place node
 *
 * Used to create the <?> command, that denotes place where something can be
 * written.
 * It is drawn as a square with a shadow.
 */
class SmPlaceNode : public SmMathSymbolNode
{
public:
    explicit SmPlaceNode(const SmToken &rNodeToken)
        : SmMathSymbolNode(SmNodeType::Place, rNodeToken)
    {
    }
    SmPlaceNode() : SmMathSymbolNode(SmNodeType::Place, SmToken(TPLACE, MS_PLACE, "<?>")) {};

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Error node, for parsing errors
 *
 * This node is used for parsing errors and draws an questionmark turned upside
 * down (inverted question mark).
 */
class SmErrorNode : public SmMathSymbolNode
{
public:
    explicit SmErrorNode(const SmToken &rNodeToken)
        : SmMathSymbolNode(SmNodeType::Error, rNodeToken)
    {
        SetText(OUString(MS_ERROR));
    }

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Table node
 *
 * This is the root node for the formula tree. This node is also used for the
 * STACK and BINOM commands. When used for root node, its
 * children are instances of SmLineNode, and in some obscure cases the a child
 * can be an instance of SmExpressionNode, mainly when errors occur.
 */
class SmTableNode : public SmStructureNode
{
    long mnFormulaBaseline;
public:
    explicit SmTableNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Table, rNodeToken)
        , mnFormulaBaseline(0)
    {
    }

    virtual const SmNode * GetLeftMost() const override;

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    long GetFormulaBaseline() const;

    void Accept(SmVisitor* pVisitor) override;
};


/** A line
 *
 * Used as child of SmTableNode when the SmTableNode is the root node of the
 * formula tree.
 */
class SmLineNode : public SmStructureNode
{
    bool mbUseExtraSpaces;

protected:
    SmLineNode(SmNodeType eNodeType, const SmToken &rNodeToken)
        : SmStructureNode(eNodeType, rNodeToken)
        , mbUseExtraSpaces(true)
    {
    }

public:
    explicit SmLineNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Line, rNodeToken)
        , mbUseExtraSpaces(true)
    {
    }

    void  SetUseExtraSpaces(bool bVal) { mbUseExtraSpaces = bVal; }
    bool  IsUseExtraSpaces() const { return mbUseExtraSpaces; };

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Expression node
 *
 * Used whenever you have an expression such as "A OVER {B + C}", here there is
 * an expression node that allows "B + C" to be the denominator of the
 * SmBinVerNode, that the OVER command creates.
 */
class SmExpressionNode : public SmLineNode
{
public:
    explicit SmExpressionNode(const SmToken &rNodeToken)
        : SmLineNode(SmNodeType::Expression, rNodeToken)
    {}

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Unary horizontal node
 *
 * The same as SmBinHorNode except this is for unary operators.
 */
class SmUnHorNode : public SmStructureNode
{
public:
    explicit SmUnHorNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::UnHor, rNodeToken, 2)
    {
    }

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Root node
 *
 * Used for create square roots and other roots, example:
 * \f$ \sqrt[\mbox{[Argument]}]{\mbox{[Body]}} \f$.
 *
 * Children:<BR>
 * 0: Argument (optional)<BR>
 * 1: Symbol (instance of SmRootSymbolNode)<BR>
 * 2: Body<BR>
 * Where argument is optional and may be NULL.
 */
class SmRootNode : public SmStructureNode
{
public:
    explicit SmRootNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Root, rNodeToken, 3)
    {
    }

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;

    SmNode* Argument();
    const SmNode* Argument() const;
    SmRootSymbolNode* Symbol();
    const SmRootSymbolNode* Symbol() const;
    SmNode* Body();
    const SmNode* Body() const;
};


/** Binary horizontal node
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
    explicit SmBinHorNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::BinHor, rNodeToken, 3)
    {
    }

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;

    SmNode* Symbol();
    const SmNode* Symbol() const;
    SmNode* LeftOperand();
    const SmNode* LeftOperand() const;
    SmNode* RightOperand();
    const SmNode* RightOperand() const;
};


/** Binary horizontal node
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
    explicit SmBinVerNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::BinVer, rNodeToken, 3)
    {
    }

    virtual const SmNode * GetLeftMost() const override;

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;
};


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
    bool mbAscending;

    void    GetOperPosSize(Point &rPos, Size &rSize,
                           const Point &rDiagPoint, double fAngleDeg) const;

public:
    explicit SmBinDiagonalNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::BinDiagonal, rNodeToken, 3)
        , mbAscending(false)
    {
    }

    bool    IsAscending() const { return mbAscending; }
    void    SetAscending(bool bVal)  { mbAscending = bVal; }

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Enum used to index sub-/supscripts in the 'maSubNodes' array
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
    bool mbUseLimits;

public:
    explicit SmSubSupNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::SubSup, rNodeToken, 1 + SUBSUP_NUM_ENTRIES)
        , mbUseLimits(false)
    {
    }

    /** Get body (Not NULL) */
    SmNode *       GetBody()    { return GetSubNode(0); }
    /** Get body (Not NULL) */
    const SmNode * GetBody() const
    {
        return const_cast<SmSubSupNode *>(this)->GetBody();
    }

    void  SetUseLimits(bool bVal) { mbUseLimits = bVal; }
    bool  IsUseLimits() const { return mbUseLimits; };

    /** Get super- or subscript
     * @remarks this method may return NULL.
     */
    SmNode * GetSubSup(SmSubSup eSubSup) { return GetSubNode(1 + eSubSup); };
    const SmNode * GetSubSup(SmSubSup eSubSup) const { return const_cast< SmSubSupNode* >( this )->GetSubSup( eSubSup ); }

    /** Set the body */
    void SetBody(SmNode* pBody) { SetSubNode(0, pBody); }
    void SetSubSup(SmSubSup eSubSup, SmNode* pScript) { SetSubNode( 1 + eSubSup, pScript); }

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;

};


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
 * Note that child 1 (Body) is usually SmBracebodyNode, but it can also be e.g. SmExpressionNode.
 */
class SmBraceNode : public SmStructureNode
{
public:
    explicit SmBraceNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Brace, rNodeToken, 3)
    {
    }

    SmMathSymbolNode* OpeningBrace();
    const SmMathSymbolNode* OpeningBrace() const;
    SmNode* Body();
    const SmNode* Body() const;
    SmMathSymbolNode* ClosingBrace();
    const SmMathSymbolNode* ClosingBrace() const;

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;
};


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
    long mnBodyHeight;

public:
    explicit SmBracebodyNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Bracebody, rNodeToken)
        , mnBodyHeight(0)
    {
    }

    virtual void    Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    long GetBodyHeight() const { return mnBodyHeight; }
    void Accept(SmVisitor* pVisitor) override;
};


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
    explicit inline SmVerticalBraceNode(const SmToken &rNodeToken);

    SmNode* Body();
    const SmNode* Body() const;
    SmMathSymbolNode* Brace();
    const SmMathSymbolNode* Brace() const;
    SmNode* Script();
    const SmNode* Script() const;

    virtual void    Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


inline SmVerticalBraceNode::SmVerticalBraceNode(const SmToken &rNodeToken)
    : SmStructureNode(SmNodeType::VerticalBrace, rNodeToken, 3)
{
}


/** Operation Node
 *
 * Used for commands like SUM, INT and similar.
 *
 * Children:<BR>
 * 0: Operation (instance of SmMathSymbolNode or SmSubSupNode)<BR>
 * 1: Body<BR>
 * None of the children may be NULL.
 *
 */
class SmOperNode : public SmStructureNode
{
public:
    explicit SmOperNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Oper, rNodeToken, 2)
    {
    }

    SmNode *       GetSymbol();
    const SmNode * GetSymbol() const
    {
        return const_cast<SmOperNode *>(this)->GetSymbol();
    }

    long CalcSymbolHeight(const SmNode &rSymbol, const SmFormat &rFormat) const;

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Node used for alignment
 *
 * This node has exactly one child at index 0.
 */
class SmAlignNode : public SmStructureNode
{
public:
    explicit SmAlignNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Align, rNodeToken)
    {}

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
};


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
    explicit SmAttributNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Attribut, rNodeToken, 2)
    {}

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;

    SmNode* Attribute();
    const SmNode* Attribute() const;
    SmNode* Body();
    const SmNode* Body() const;
};


/** Font node
 *
 * Used to change the font of its children.
 */
class SmFontNode : public SmStructureNode
{
    FontSizeType meSizeType;
    Fraction     maFontSize;

public:
    explicit SmFontNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Font, rNodeToken)
        , meSizeType(FontSizeType::MULTIPLY)
        , maFontSize(1)
    {
    }

    void SetSizeParameter(const Fraction &rValue, FontSizeType nType);
    const Fraction & GetSizeParameter() const {return maFontSize;}
    FontSizeType GetSizeType() const {return meSizeType;}

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Matrix node
 *
 * Used to implement the MATRIX command, example:
 * "matrix{ 1 # 2 ## 3 # 4}".
 */
class SmMatrixNode : public SmStructureNode
{
    sal_uInt16 mnNumRows,
               mnNumCols;

public:
    explicit SmMatrixNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Matrix, rNodeToken)
        , mnNumRows(0)
        , mnNumCols(0)
    {
    }

    sal_uInt16 GetNumRows() const {return mnNumRows;}
    sal_uInt16 GetNumCols() const {return mnNumCols;}
    void SetRowCol(sal_uInt16 nMatrixRows, sal_uInt16 nMatrixCols);

    virtual const SmNode * GetLeftMost() const override;

    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void CreateTextFromNode(OUString &rText) override;
    void Accept(SmVisitor* pVisitor) override;
};


/** Node for whitespace
 *
 * Used to implement the commands "~" and "`". This node is just a blank space.
 */
class SmBlankNode : public SmGraphicNode
{
    sal_uInt16 mnNum;

public:
    explicit SmBlankNode(const SmToken &rNodeToken)
        : SmGraphicNode(SmNodeType::Blank, rNodeToken)
        , mnNum(0)
    {
    }

    void         IncreaseBy(const SmToken &rToken, sal_uInt32 nMultiplyBy = 1);
    void         Clear() { mnNum = 0; }
    sal_uInt16   GetBlankNum() const { return mnNum; }
    void         SetBlankNum(sal_uInt16 nNumber) { mnNum = nNumber; }

    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth) override;
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    void Accept(SmVisitor* pVisitor) override;
    virtual void CreateTextFromNode(OUString &rText) override;
};


inline SmNode* SmRootNode::Argument()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 0 );
}
inline const SmNode* SmRootNode::Argument() const
{
    return const_cast< SmRootNode* >( this )->Argument();
}
inline SmRootSymbolNode* SmRootNode::Symbol()
{
    assert( GetNumSubNodes() == 3 );
    assert( GetSubNode( 1 )->GetType() == SmNodeType::RootSymbol );
    return static_cast< SmRootSymbolNode* >( GetSubNode( 1 ));
}
inline const SmRootSymbolNode* SmRootNode::Symbol() const
{
    return const_cast< SmRootNode* >( this )->Symbol();
}
inline SmNode* SmRootNode::Body()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 2 );
}
inline const SmNode* SmRootNode::Body() const
{
    return const_cast< SmRootNode* >( this )->Body();
}


inline SmNode* SmBinHorNode::Symbol()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 1 );
}
inline const SmNode* SmBinHorNode::Symbol() const
{
    return const_cast< SmBinHorNode* >( this )->Symbol();
}
inline SmNode* SmBinHorNode::LeftOperand()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 0 );
}
inline const SmNode* SmBinHorNode::LeftOperand() const
{
    return const_cast< SmBinHorNode* >( this )->LeftOperand();
}
inline SmNode* SmBinHorNode::RightOperand()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 2 );
}
inline const SmNode* SmBinHorNode::RightOperand() const
{
    return const_cast< SmBinHorNode* >( this )->RightOperand();
}

inline SmNode* SmAttributNode::Attribute()
{
    assert( GetNumSubNodes() == 2 );
    return GetSubNode( 0 );
}
inline const SmNode* SmAttributNode::Attribute() const
{
    return const_cast< SmAttributNode* >( this )->Attribute();
}
inline SmNode* SmAttributNode::Body()
{
    assert( GetNumSubNodes() == 2 );
    return GetSubNode( 1 );
}
inline const SmNode* SmAttributNode::Body() const
{
    return const_cast< SmAttributNode* >( this )->Body();
}

inline SmMathSymbolNode* SmBraceNode::OpeningBrace()
{
    assert( GetNumSubNodes() == 3 );
    assert( GetSubNode( 0 )->GetType() == SmNodeType::Math );
    return static_cast< SmMathSymbolNode* >( GetSubNode( 0 ));
}
inline const SmMathSymbolNode* SmBraceNode::OpeningBrace() const
{
    return const_cast< SmBraceNode* >( this )->OpeningBrace();
}
inline SmNode* SmBraceNode::Body()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 1 );
}
inline const SmNode* SmBraceNode::Body() const
{
    return const_cast< SmBraceNode* >( this )->Body();
}
inline SmMathSymbolNode* SmBraceNode::ClosingBrace()
{
    assert( GetNumSubNodes() == 3 );
    assert( GetSubNode( 2 )->GetType() == SmNodeType::Math );
    return static_cast< SmMathSymbolNode* >( GetSubNode( 2 ));
}
inline const SmMathSymbolNode* SmBraceNode::ClosingBrace() const
{
    return const_cast< SmBraceNode* >( this )->ClosingBrace();
}

inline SmNode* SmVerticalBraceNode::Body()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 0 );
}
inline const SmNode* SmVerticalBraceNode::Body() const
{
    return const_cast< SmVerticalBraceNode* >( this )->Body();
}
inline SmMathSymbolNode* SmVerticalBraceNode::Brace()
{
    assert( GetNumSubNodes() == 3 );
    assert( GetSubNode( 1 )->GetType() == SmNodeType::Math );
    return static_cast< SmMathSymbolNode* >( GetSubNode( 1 ));
}
inline const SmMathSymbolNode* SmVerticalBraceNode::Brace() const
{
    return const_cast< SmVerticalBraceNode* >( this )->Brace();
}
inline SmNode* SmVerticalBraceNode::Script()
{
    assert( GetNumSubNodes() == 3 );
    return GetSubNode( 2 );
}
inline const SmNode* SmVerticalBraceNode::Script() const
{
    return const_cast< SmVerticalBraceNode* >( this )->Script();
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
