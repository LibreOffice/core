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
#include <rtl/ustrbuf.hxx>

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
    Phantom  = 0x0020
};

namespace o3tl
{
    template<> struct typed_flags<FontChangeMask> : is_typed_flags<FontChangeMask, 0x003f> {};
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

    SmFace          maFace;
    SmToken         maNodeToken;
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

    virtual ~SmNode();

    /**
     * Checks node visibility.
     * Returns true if this is an instance of SmVisibleNode's subclass, false otherwise.
     * @return node visibility
     */
    virtual bool IsVisible() const = 0;

    /**
     * Gets the number of subnodes.
     * @return number of subnodes
     */
    virtual size_t GetNumSubNodes() const = 0;

    /**
     * Gets the subnode of index nIndex.
     * @param nIndex
     * @return subnode of index nIndex
     */
    virtual SmNode * GetSubNode(size_t nIndex) = 0;
    const   SmNode * GetSubNode(size_t nIndex) const
                     { return const_cast<SmNode *>(this)->GetSubNode(nIndex); }

    virtual const SmNode * GetLeftMost() const;

    /**
     * Gets the FontChangeMask flags.
     * @return FontChangeMask flags
     */
    FontChangeMask &Flags() { return mnFlags; }

    /**
     * Gets the font attributes.
     * @return font attributes
     */
    FontAttribute &Attributes() { return mnAttributes; }

    /**
     * Checks if it is a visible node rendered invisible.
     * @return rendered visibility
     */
    bool IsPhantom() const { return mbIsPhantom; }

    /**
     * Sets the render visibility of a visible node to bIsPhantom.
     * @param bIsPhantom
     * @return
     */
    void SetPhantom(bool bIsPhantom);

    /**
     * Sets the font color.
     * @param rColor
     * @return
     */
    void SetColor(const Color &rColor);

    /**
     * Sets the font attribute nAttrib.
     * Check FontAttribute class.
     * @param nAttrib
     * @return
     */
    void SetAttribut(FontAttribute nAttrib);

    /**
     * Clears the font attribute nAttrib.
     * Check FontAttribute class.
     * @param nAttrib
     * @return
     */
    void ClearAttribut(FontAttribute nAttrib);

    /**
     * Gets the font.
     * @return font
     */
    const SmFace & GetFont() const { return maFace; };
          SmFace & GetFont()       { return maFace; };

    /**
     * Sets the font to rFace.
     * @param rFace
     * @return
     */
    void SetFont(const SmFace &rFace);

    /**
     * Sets the font size to rRelSize with type nType.
     * Check FontSizeType for details.
     * @param rRelSize
     * @param nType
     * @return
     */
    void SetFontSize(const Fraction &rRelSize, FontSizeType nType);

    /**
     * Sets the font size to rRelSize with type FontSizeType::ABSOLUT.
     * @param rScale
     * @return
     */
    void SetSize(const Fraction &rScale);

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell, int nDepth);

    /**
     * Prepare preliminary font attributes
     * Called on Prepare(...).
     * @return
     */
    void PrepareAttributes();

    /**
     * Sets the alignment of the text.
     * Check RectHorAlign class for details.
     * The subtrees will be affected if bApplyToSubTree.
     * @param eHorAlign
     * @param bApplyToSubTree
     * @return
     */
    void SetRectHorAlign(RectHorAlign eHorAlign, bool bApplyToSubTree = true );

    /**
     * Gets the alignment of the text.
     * @return alignment of the text
     */
    RectHorAlign GetRectHorAlign() const { return meRectHorAlign; }

    /**
     * Parses itself to SmRect.
     * @return this
     */
    const SmRect & GetRect() const { return *this; }

    /**
     * Moves the rectangle by rVector.
     * @param rVector
     * @return
     */
    void Move(const Point &rVector);

    /**
     * Moves the rectangle to rPoint, being the top left corner the origin.
     * @param rPoint
     * @return
     */
    void MoveTo(const Point &rPoint) { Move(rPoint - GetTopLeft()); }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) = 0;

    /**
     * Appends to rText the node text.
     * @param rText
     * @return
     */
    virtual void GetAccessibleText( OUStringBuffer &rText ) const = 0;

    /**
     * Gets the node accessible index.
     * Used for visual editing.
     * @return node accessible index
     */
    sal_Int32 GetAccessibleIndex() const { return mnAccIndex; }

    /**
     * Sets the node accessible index to nAccIndex.
     * Used for visual editing.
     * @param nAccIndex
     * @return
     */
    void SetAccessibleIndex(sal_Int32 nAccIndex) { mnAccIndex = nAccIndex; }

    /**
     * Finds the node with accessible index nAccIndex.
     * Used for visual editing.
     * @param nAccIndex
     * @return node with accessible index nAccIndex
     */
    const SmNode * FindNodeWithAccessibleIndex(sal_Int32 nAccIndex) const;

    /**
     * Gets the line in the text where the node is located.
     * It is used to do the visual <-> text correspondence.
     * @return line
     */
    sal_uInt16 GetRow() const { return sal::static_int_cast<sal_uInt16>(maNodeToken.nRow); }

    /**
     * Gets the column of the line in the text where the node is located.
     * It is used to do the visual <-> text correspondence.
     * @return column
     */
    sal_uInt16 GetColumn() const { return sal::static_int_cast<sal_uInt16>(maNodeToken.nCol); }

    /**
     * Gets the scale mode.
     * @return scale mode
     */
    SmScaleMode GetScaleMode() const { return meScaleMode; }

    /**
     * Sets the scale mode to eMode.
     * @param eMode
     * @return
     */
    void SetScaleMode(SmScaleMode eMode) { meScaleMode = eMode; }

    //visual stuff TODO comment
    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth);
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight);

    /**
     * Gets the node type.
     * @return node type
     */
    SmNodeType GetType() const  { return meType; }

    /**
     * Gets the token.
     * The token contains the data extracted from the text mode.
     * Ej: text, type (sub, sup, int,...), row and column,...
     * @return node type
     */
    const SmToken & GetToken() const { return maNodeToken; }
          SmToken & GetToken()       { return maNodeToken; }

    /**
     * Finds the node from the position in the text.
     * It is used to do the visual <-> text correspondence.
     * @param nRow
     * @param nCol
     * @return the given node
     */
    const SmNode *  FindTokenAt(sal_uInt16 nRow, sal_uInt16 nCol) const;

    /**
     * Finds the closest rectangle in the screen.
     * @param rPoint
     * @return the given node
     */
    const SmNode *  FindRectClosestTo(const Point &rPoint) const;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    virtual void Accept(SmVisitor* pVisitor) = 0;

    /**
     * Checks if the node is selected.
     * @return the node is selected
     */
    bool IsSelected() const {return mbIsSelected;}

    /**
     * Sets the node to Selected.
     * @param Selected
     * @return
     */
    void SetSelected(bool Selected) {mbIsSelected = Selected;}

    /**
     * Gets the parent node of this node.
     * @return parent node
     */
    const SmStructureNode* GetParent() const { return mpParentNode; }
          SmStructureNode* GetParent()       { return mpParentNode; }

    /**
     * Sets the parent node.
     * @param parent
     * @return
     */
    void SetParent(SmStructureNode* parent){ mpParentNode = parent; }

    /**
     * Sets the token for this node.
     * @param token
     * @return
     */
    void SetToken(SmToken const & token){ maNodeToken = token; }

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
        , maSubNodes(nSize) {}

public:
    virtual ~SmStructureNode() override;

    /**
     * Checks node visibility.
     * Returns true if this is an instance of SmVisibleNode's subclass, false otherwise.
     * @return node visibility
     */
    virtual bool IsVisible() const override;

    /**
     * Gets the number of subnodes.
     * @return number of subnodes
     */
    virtual size_t GetNumSubNodes() const override;

    /**
     * Gets the subnode of index nIndex.
     * @param nIndex
     * @return subnode of index nIndex
     */
    using   SmNode::GetSubNode;
    virtual SmNode * GetSubNode(size_t nIndex) override;

    /**
     * Does the cleaning of the subnodes.
     * @return
     */
    void ClearSubNodes();

    /**
     * Sets subnodes, used for operators.
     * @param pFirst
     * @param pSecond
     * @param pThird
     * @return
     */
    void SetSubNodes(std::unique_ptr<SmNode> pFirst, std::unique_ptr<SmNode> pSecond,
                     std::unique_ptr<SmNode> pThird = nullptr);

    /**
     * Sets subnodes.
     * @param rNodeArray
     * @return
     */
    void SetSubNodes(SmNodeArray&& rNodeArray);

    /**
     * Appends to rText the node text.
     * @param rText
     * @return
     */
    virtual void  GetAccessibleText( OUStringBuffer &rText ) const override;

    /**
     * Gets the first subnode.
     * @return first subnode
     */
    SmNodeArray::iterator begin() {return maSubNodes.begin();}

    /**
     * Gets the last subnode.
     * @return last subnode
     */
    SmNodeArray::iterator end() {return maSubNodes.end();}

    /**
     * Gets the last subnode.
     * @return last subnode
     */
    SmNodeArray::reverse_iterator rbegin() {return maSubNodes.rbegin();}

    /**
     * Gets the first subnode.
     * @return first subnode
     */
    SmNodeArray::reverse_iterator rend() {return maSubNodes.rend();}

    /**
     * Get the index of the child node pSubNode.
     * Returns -1, if pSubNode isn't a subnode of this.
     * @param pSubNode
     * @return index of the child node
     */
    int IndexOfSubNode(SmNode const * pSubNode);

    /**
     * Sets the subnode pNode at nIndex.
     * If necessary increases the subnodes length.
     * @param nIndex
     * @param pNode
     * @return
     */
    void SetSubNode(size_t nIndex, SmNode* pNode);

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
    :   SmNode(eNodeType, rNodeToken) {}

public:

    /**
     * Checks node visibility.
     * Returns true if this is an instance of SmVisibleNode's subclass, false otherwise.
     * @return node visibility
     */
    virtual bool        IsVisible() const override;

    /**
     * Gets the number of subnodes.
     * @return number of subnodes
     */
    virtual size_t      GetNumSubNodes() const override;

    /**
     * Gets the subnode of index nIndex.
     * @param nIndex
     * @return subnode of index nIndex
     */
    using   SmNode::GetSubNode;
    virtual SmNode *    GetSubNode(size_t nIndex) override;
};


class SmGraphicNode : public SmVisibleNode
{
protected:
    SmGraphicNode(SmNodeType eNodeType, const SmToken &rNodeToken)
    :   SmVisibleNode(eNodeType, rNodeToken) {}

public:

    /**
     * Appends to rText the node text.
     * @param rText
     * @return
     */
    virtual void  GetAccessibleText( OUStringBuffer &rText ) const override;
};


/** Draws a rectangle
 *
 * Used for drawing the line in the OVER and OVERSTRIKE commands.
 */
class SmRectangleNode final : public SmGraphicNode
{
    Size maToSize;

public:
    explicit SmRectangleNode(const SmToken &rNodeToken)
        : SmGraphicNode(SmNodeType::Rectangle, rNodeToken)
    {}

    //visual stuff TODO comment
    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Polygon line node
 *
 * Used to draw the slash of the WIDESLASH command by SmBinDiagonalNode.
 */
class SmPolyLineNode final : public SmGraphicNode
{
    tools::Polygon maPoly;
    Size maToSize;
    long mnWidth;

public:
    explicit SmPolyLineNode(const SmToken &rNodeToken);

    /**
     * Gets the width of the rect.
     * @return width
     */
    long GetWidth() const { return mnWidth; }

    /**
     * Gets the polygon to draw the node.
     * @return polygon
     */
    tools::Polygon &GetPolygon()  { return maPoly; }

    //visual stuff TODO comment
    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Text node
 *
 * @remarks This class also serves as baseclass for all nodes that contains text.
 */
class SmTextNode : public SmVisibleNode
{

protected:
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

    /**
     * Returns the font type being used (text, variable, symbol, ...).
     * @return font type
     */
    sal_uInt16 GetFontDesc() const { return mnFontDesc; }

    /**
     * Sets the font type to fontdesc.
     * Definitions are on format.hxx.
     * @param fontdesc
     * @return
     */
    void SetFontDesc(sal_uInt16 fontdesc) { mnFontDesc=fontdesc; }

    /**
     * Sets the node text to rText.
     * @param rText
     * @return
     */
    void SetText(const OUString &rText) { maText = rText; }

    /**
     * Gets the node text.
     * @return node text
     */
    const OUString & GetText() const { return maText; }
          OUString & GetText()       { return maText; }

    /**
     * Change the text of this node, including the underlying token to rText.
     * @param rText
     * @return
     */
    void ChangeText(const OUString &rText);

    /**
     * Try to guess the correct FontDesc, used during visual editing
     * @return
     */
    void AdjustFontDesc();

    /**
     * Index within GetText() where the selection starts.
     * @remarks Only valid of SmNode::IsSelected() is true.
     * @return index.
     */
    sal_Int32 GetSelectionStart() const { return mnSelectionStart; }

    /**
     * Index within GetText() where the selection ends.
     * @remarks Only valid of SmNode::IsSelected() is true.
     * @return index.
     */
    sal_Int32 GetSelectionEnd() const  {return mnSelectionEnd; }

    /**
     * Sets the index within GetText() where the selection starts to index.
     * @param index
     * @return
     */
    void SetSelectionStart(sal_Int32 index) {mnSelectionStart = index;}

    /**
     * Sets the index within GetText() where the selection ends to index.
     * @param index
     * @return
     */
    void SetSelectionEnd(sal_Int32 index) {mnSelectionEnd = index;}

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Appends to rText the node text.
     * @param rText
     * @return
     */
    virtual void  GetAccessibleText( OUStringBuffer &rText ) const override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;

    /**
     * Converts the character from StarMath's private area symbols to a matching Unicode
     * character, if necessary. To be used when converting GetText() to a normal text.
     * @param nIn
     * @return unicode char
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

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Glyph node for custom operators
 *
 * This node is used with commands: oper, uoper and boper.
 * E.g. in "A boper op B", "op" will be an instance of SmGlyphSpecialNode.
 * "boper" simply interprets "op", the following token, as a binary operator.
 * The command "uoper" interprets the following token as unary operator.
 * For these commands an instance of SmGlyphSpecialNode is used for the
 * operator token, following the command.
 */
class SmGlyphSpecialNode final : public SmSpecialNode
{
public:
    explicit SmGlyphSpecialNode(const SmToken &rNodeToken)
        : SmSpecialNode(SmNodeType::GlyphSpecial, rNodeToken, FNT_MATH)
    {}

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
        if (u'\0' != cChar) SetText(OUString(cChar));
    }

public:
    explicit SmMathSymbolNode(const SmToken &rNodeToken);

    //visual stuff TODO comment
    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nWidth) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Math Identifier
 *
 * This behaves essentially the same as SmMathSymbolNode and is only used to
 * represent math symbols that should be exported as <mi> elements rather than
 * <mo> elements.
 */
class SmMathIdentifierNode final : public SmMathSymbolNode
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
class SmRootSymbolNode final : public SmMathSymbolNode
{
    sal_uLong mnBodyWidth;  // width of body (argument) of root sign

public:
    explicit SmRootSymbolNode(const SmToken &rNodeToken)
        : SmMathSymbolNode(SmNodeType::RootSymbol, rNodeToken)
        , mnBodyWidth(0) { }

    /**
     * Gets the body width.
     * Allows to know how long is the root and paint it.
     * @return body width
     */
    sal_uLong GetBodyWidth() const {return mnBodyWidth;};

    //visual stuff TODO comment
    virtual void AdaptToX(OutputDevice &rDev, sal_uLong nHeight) override;
    virtual void AdaptToY(OutputDevice &rDev, sal_uLong nHeight) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Place node
 *
 * Used to create the <?> command, that denotes place where something can be
 * written.
 * It is drawn as a square with a shadow.
 */
class SmPlaceNode final : public SmMathSymbolNode
{
public:
    explicit SmPlaceNode(const SmToken &rNodeToken)
        : SmMathSymbolNode(SmNodeType::Place, rNodeToken) { }
    SmPlaceNode() : SmMathSymbolNode(SmNodeType::Place, SmToken(TPLACE, MS_PLACE, "<?>")) { };

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Error node, for parsing errors
 *
 * This node is used for parsing errors and draws a questionmark turned upside
 * down (inverted question mark).
 */
class SmErrorNode final : public SmMathSymbolNode
{
public:
    explicit SmErrorNode(const SmToken &rNodeToken)
                : SmMathSymbolNode(SmNodeType::Error, rNodeToken) { SetText(OUString(MS_ERROR)); }

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Table node
 *
 * This is the root node for the formula tree. This node is also used for the
 * STACK and BINOM commands. When used for root node, its
 * children are instances of SmLineNode, and in some obscure cases the child
 * can be an instance of SmExpressionNode, mainly when errors occur.
 */
class SmTableNode final : public SmStructureNode
{
    long mnFormulaBaseline;
public:
    explicit SmTableNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Table, rNodeToken)
        , mnFormulaBaseline(0) { }

    virtual const SmNode * GetLeftMost() const override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Gets the formula baseline.
     * @return formula baseline
     */
    long GetFormulaBaseline() const;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
        , mbUseExtraSpaces(true) { }

public:
    explicit SmLineNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Line, rNodeToken)
        , mbUseExtraSpaces(true) { }

    /**
     * Sets if it going to use extra spaces.
     * It is used to set if there has to be space between node while rendering.
     * By default it is true.
     * @param bVal
     * @return
     */
    void  SetUseExtraSpaces(bool bVal) { mbUseExtraSpaces = bVal; }

    /**
     * Checks if it is using extra spaces.
     * It is used for calculating space between nodes when rendering.
     * By default it is true.
     * @return is using extra spaces
     */
    bool  IsUseExtraSpaces() const { return mbUseExtraSpaces; };

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Expression node
 *
 * Used whenever you have an expression such as "A OVER {B + C}", here there is
 * an expression node that allows "B + C" to be the denominator of the
 * SmBinVerNode, that the OVER command creates.
 */
class SmExpressionNode final : public SmLineNode
{
public:
    explicit SmExpressionNode(const SmToken &rNodeToken)
        : SmLineNode(SmNodeType::Expression, rNodeToken) { }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Unary horizontal node
 *
 * The same as SmBinHorNode except this is for unary operators.
 */
class SmUnHorNode final : public SmStructureNode
{
public:
    explicit SmUnHorNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::UnHor, rNodeToken, 2) { }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
class SmRootNode final : public SmStructureNode
{
public:
    explicit SmRootNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Root, rNodeToken, 3) { }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;

    /**
     * Returns the node containing the data of the order of the root.
     * @return order data
     */
    const SmNode* Argument() const { return const_cast<SmRootNode *>(this)->Argument(); }
          SmNode* Argument()       { assert( GetNumSubNodes() == 3 ); return GetSubNode( 0 ); }

    /**
     * Returns the node containing the data of the character used for the root.
     * @return symbol data
     */
    const SmRootSymbolNode* Symbol() const { return const_cast<SmRootNode *>(this)->Symbol(); }
          SmRootSymbolNode* Symbol()       { assert( GetNumSubNodes() == 3 );
                                             assert( GetSubNode( 1 )->GetType()
                                                        == SmNodeType::RootSymbol );
                                             return static_cast< SmRootSymbolNode* >
                                                ( GetSubNode( 1 )); }

    /**
     * Returns the node containing the data inside the root.
     * @return body data
     */
    const SmNode* Body() const { return const_cast<SmRootNode *>(this)->Body(); }
          SmNode* Body()       { assert( GetNumSubNodes() == 3 ); return GetSubNode( 2 ); }

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
class SmBinHorNode final : public SmStructureNode
{
public:
    explicit SmBinHorNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::BinHor, rNodeToken, 3) { }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;

    /**
     * Returns the node containing the data of the binary operator.
     * @return symbol data
     */
    const SmNode* Symbol() const { return const_cast<SmBinHorNode *>(this)->Symbol(); }
          SmNode* Symbol()       { assert( GetNumSubNodes() == 3 ); return GetSubNode( 1 ); }

    /**
     * Returns the node containing the data of the left opperand.
     * @return left opperand data
     */
    const SmNode* LeftOperand() const { return const_cast<SmBinHorNode *>(this)->LeftOperand(); }
          SmNode* LeftOperand()       { assert( GetNumSubNodes() == 3 ); return GetSubNode( 0 ); }

    /**
     * Returns the node containing the data of the right opperand.
     * @return right opperand data
     */
    const SmNode* RightOperand() const { return const_cast<SmBinHorNode *>(this)->RightOperand(); }
          SmNode* RightOperand()       { assert( GetNumSubNodes() == 3 ); return GetSubNode( 2 ); }
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
class SmBinVerNode final : public SmStructureNode
{
public:
    explicit SmBinVerNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::BinVer, rNodeToken, 3) { }

    virtual const SmNode * GetLeftMost() const override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
class SmBinDiagonalNode final : public SmStructureNode
{
    bool mbAscending;

    /**
     * Returns the position and size of the diagonal line by reference.
     * @param rPos
     * @param rSize
     * @param rDiagPoint
     * @param fAngleDeg
     * @return position and size of the diagonal line
     */
    void GetOperPosSize(Point &rPos, Size &rSize, const Point &rDiagPoint, double fAngleDeg) const;

public:
    explicit SmBinDiagonalNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::BinDiagonal, rNodeToken, 3)
        , mbAscending(false) { }

    /**
     * Checks if it is of ascending type.
     * Ascending:
     *     / b
     *    /
     * a /
     * Descending:
     * a \
     *    \
     *     \ b
     * @return ascending.
     */
    bool    IsAscending() const { return mbAscending; }

    /**
     * Sets if the wideslash is ascending to bVal.
     * @param bVal
     * @return
     */
    void    SetAscending(bool bVal)  { mbAscending = bVal; }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
class SmSubSupNode final : public SmStructureNode
{
    bool mbUseLimits;

public:
    explicit SmSubSupNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::SubSup, rNodeToken, 1 + SUBSUP_NUM_ENTRIES)
        , mbUseLimits(false) { }

    /**
     * Returns the node with the data of what has to be superindex or subindex.
     * @return body data
     */
    const SmNode * GetBody() const { return const_cast<SmSubSupNode *>(this)->GetBody(); }
          SmNode * GetBody()       { return GetSubNode(0); }

    /**
     * Checks if it is going to be used for a limit.
     * Example lim from { x toward 0 } { {sin x}over x } = 1
     * @return is a limit
     */
    bool  IsUseLimits() const { return mbUseLimits; };

    /**
     * Sets if it is going to be used for a limit to bVal.
     * @param bVal
     * @return
     */
    void  SetUseLimits(bool bVal) { mbUseLimits = bVal; }

    /**
     * Gets the node with the data of what has to be superindex or subindex.
     * The position to check is given by eSubSup.
     * @remarks this method may return NULL.
     * @param eSubSup
     * @return body data
     */
     const SmNode * GetSubSup(SmSubSup eSubSup) const { return const_cast< SmSubSupNode* >
                                                            ( this )->GetSubSup( eSubSup ); }
           SmNode * GetSubSup(SmSubSup eSubSup)       { return GetSubNode(1 + eSubSup); };

    /**
     * Sets the node with the data of what has to be superindex or subindex.
     * @param pScript
     */
    void SetBody(SmNode* pBody) { SetSubNode(0, pBody); }

     /**
     * Sets the node with the data of what has to be superindex or subindex.
     * The position to check is given by eSubSup.
     * @param eSubSup
     * @param pScript
     */
    void SetSubSup(SmSubSup eSubSup, SmNode* pScript) { SetSubNode( 1 + eSubSup, pScript); }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
class SmBraceNode final : public SmStructureNode
{
public:
    explicit SmBraceNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Brace, rNodeToken, 3) { }

    /**
     * Returns the node containing the data of the opening brace.
     * @return opening brace data
     */
    const SmMathSymbolNode* OpeningBrace() const { return const_cast<SmBraceNode *>
                                                       (this)->OpeningBrace(); }
    SmMathSymbolNode* OpeningBrace()             { assert( GetNumSubNodes() == 3 );
                                                   assert( GetSubNode( 0 )->GetType()
                                                               == SmNodeType::Math );
                                                   return static_cast< SmMathSymbolNode* >
                                                       ( GetSubNode( 0 )); }

    /**
     * Returns the node containing the data of what is between braces.
     * @return body data
     */
    const SmNode* Body() const { return const_cast<SmBraceNode *>(this)->Body(); }
    SmNode* Body()             { assert( GetNumSubNodes() == 3 ); return GetSubNode( 1 ); }

    /**
     * Returns the node containing the data of the closing brace.
     * @return closing brace data
     */
    const SmMathSymbolNode* ClosingBrace() const { return const_cast<SmBraceNode *>
                                                       (this)->ClosingBrace(); }
    SmMathSymbolNode* ClosingBrace()             { assert( GetNumSubNodes() == 3 );
                                                   assert( GetSubNode( 2 )->GetType()
                                                               == SmNodeType::Math );
                                                   return static_cast< SmMathSymbolNode* >
                                                       ( GetSubNode( 2 )); }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
class SmBracebodyNode final : public SmStructureNode
{
    long mnBodyHeight;

public:
    explicit SmBracebodyNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Bracebody, rNodeToken)
        , mnBodyHeight(0) { }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void    Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;
    long GetBodyHeight() const { return mnBodyHeight; }

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
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
class SmVerticalBraceNode final : public SmStructureNode
{
public:
    explicit SmVerticalBraceNode(const SmToken &rNodeToken)
    : SmStructureNode(SmNodeType::VerticalBrace, rNodeToken, 3) { }

    /**
     * Returns the node containing the data of what the brace is pointing for.
     * body { script }
     * @return body data
     */
    const SmNode* Body() const { return const_cast<SmVerticalBraceNode *>(this)->Body(); }
    SmNode* Body()             { assert( GetNumSubNodes() == 3 ); return GetSubNode( 0 ); }

    /**
     * Returns the node containing the data of the brace.
     * @return brace data
     */
    const SmMathSymbolNode* Brace() const { return const_cast<SmVerticalBraceNode *>
                                                (this)->Brace(); }
          SmMathSymbolNode* Brace()       { assert( GetNumSubNodes() == 3 );
                                            assert( GetSubNode( 1 )->GetType()
                                                        == SmNodeType::Math );
                                            return static_cast< SmMathSymbolNode* >
                                                ( GetSubNode( 1 )); }

    /**
     * Returns the node containing the data of what is in the brace.
     * body { script }
     * @return opening brace data
     */
    const SmNode* Script() const { return const_cast<SmVerticalBraceNode *>(this)->Script(); }
          SmNode* Script()       { assert( GetNumSubNodes() == 3 ); return GetSubNode( 2 ); }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};

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
class SmOperNode final : public SmStructureNode
{
public:
    explicit SmOperNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Oper, rNodeToken, 2) { }

    /**
     * Returns the node with the operator data
     * @return operator data
     */
    const SmNode * GetSymbol() const { return const_cast<SmOperNode *>(this)->GetSymbol(); }
          SmNode * GetSymbol();

    /**
     * Returns the height of the node in base to the symbol
     * ( rSymbol contains the operator data )
     * and the font format ( rFormat ).
     * @param rSymbol
     * @param rFormat
     * @return node's height
     */
    long CalcSymbolHeight(const SmNode &rSymbol, const SmFormat &rFormat) const;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Node used for alignment
 *
 * This node has exactly one child at index 0.
 */
class SmAlignNode final : public SmStructureNode
{
public:
    explicit SmAlignNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Align, rNodeToken) { }

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Attribute node
 *
 * Used to give an attribute to another node. Used for commands such as:
 * UNDERLINE, OVERLINE, OVERSTRIKE, WIDEVEC, WIDEHARPOON, WIDEHAT and WIDETILDE.
 *
 * Children:<BR>
 * 0: Attribute<BR>
 * 1: Body<BR>
 * None of these may be NULL.
 */
class SmAttributNode final : public SmStructureNode
{
public:
    explicit SmAttributNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Attribut, rNodeToken, 2) {}

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;

    /**
     * Gets the attribute data.
     * @return attribute data
     */
    const SmNode* Attribute() const { return const_cast<SmAttributNode *>(this)->Attribute(); }
          SmNode* Attribute()       { assert( GetNumSubNodes() == 2 ); return GetSubNode( 0 ); }

    /**
     * Gets the body data ( the nodes affected by the attribute ).
     * @return body data
     */
    const SmNode* Body() const { return const_cast<SmAttributNode *>(this)->Body(); }
          SmNode* Body()      { assert( GetNumSubNodes() == 2 ); return GetSubNode( 1 ); }
};


/** Font node
 *
 * Used to change the font of its children.
 */
class SmFontNode final : public SmStructureNode
{
    FontSizeType meSizeType;
    Fraction     maFontSize;

public:
    explicit SmFontNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Font, rNodeToken)
        , meSizeType(FontSizeType::MULTIPLY)
        , maFontSize(1) { }

    /**
     * Sets font size to rValue in nType mode.
     * Check FontSizeType for details.
     * @param rValue
     * @param nType
     * @return
     */
    void SetSizeParameter(const Fraction &rValue, FontSizeType nType)
                            { meSizeType = nType; maFontSize = rValue; }

    /**
     * Returns the font size.
     * @return font size.
     */
    const Fraction & GetSizeParameter() const {return maFontSize;}

    /**
     * Returns the font size type.
     * Check FontSizeType for details.
     * @return font size type.
     */
    FontSizeType GetSizeType() const {return meSizeType;}

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Matrix node
 *
 * Used to implement the MATRIX command, example:
 * "matrix{ 1 # 2 ## 3 # 4}".
 */
class SmMatrixNode final : public SmStructureNode
{
    sal_uInt16 mnNumRows,
               mnNumCols;

public:
    explicit SmMatrixNode(const SmToken &rNodeToken)
        : SmStructureNode(SmNodeType::Matrix, rNodeToken)
        , mnNumRows(0)
        , mnNumCols(0) { }

    /**
     * Gets the number of rows of the matrix.
     * @return rows number
     */
    sal_uInt16 GetNumRows() const {return mnNumRows;}

    /**
     * Gets the number of columns of the matrix.
     * @return columns number
     */
    sal_uInt16 GetNumCols() const {return mnNumCols;}

    /**
     * Sets the dimensions of the matrix.
     * @param nMatrixRows
     * @param nMatrixCols
     * @return
     */
    void SetRowCol(sal_uInt16 nMatrixRows, sal_uInt16 nMatrixCols)
                     { mnNumRows = nMatrixRows; mnNumCols = nMatrixCols; }

    virtual const SmNode * GetLeftMost() const override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;
};


/** Node for whitespace
 *
 * Used to implement the commands "~" and "`". This node is just a blank space.
 */
class SmBlankNode final : public SmGraphicNode
{
    sal_uInt16 mnNum;

public:
    explicit SmBlankNode(const SmToken &rNodeToken)
        : SmGraphicNode(SmNodeType::Blank, rNodeToken)
        , mnNum(0) { }

    void         IncreaseBy(const SmToken &rToken, sal_uInt32 nMultiplyBy = 1);
    void         Clear() { mnNum = 0; }
    sal_uInt16   GetBlankNum() const { return mnNum; }
    void         SetBlankNum(sal_uInt16 nNumber) { mnNum = nNumber; }

    /**
     * Prepare preliminary settings about font and text
     * (e.g. maFace, meRectHorAlign, mnFlags, mnAttributes, etc.)
     * @param rFormat
     * @param rDocShell
     * @param nDepth
     * @return
     */
    virtual void Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell,
                         int nDepth) override;

    /**
     * Prepares the SmRect to render.
     * @param rDev
     * @param rFormat
     * @return
     */
    virtual void Arrange(OutputDevice &rDev, const SmFormat &rFormat) override;

    /**
     * Accept a visitor.
     * Calls the method for this class on the visitor.
     * @param pVisitor
     * @return
     */
    void Accept(SmVisitor* pVisitor) override;

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
