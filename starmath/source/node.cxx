/*************************************************************************
 *
 *  $RCSfile: node.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-23 10:11:19 $
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

#pragma hdrstop
#define APPEND(str,ascii) str.AppendAscii(RTL_CONSTASCII_STRINGPARAM(ascii))

#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _FRACT_HXX //autogen
#include <tools/fract.hxx>
#endif
#ifndef _TOOLS_SOLMATH_H
#include <tools/solmath.hxx>
#endif
#ifndef _SV_COLOR_HXX //autogen
#include <vcl/color.hxx>
#endif
#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _SV_LINEINFO_HXX
#include <vcl/lineinfo.hxx>
#endif
#ifndef _SV_OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifdef USE_POLYGON

#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#ifndef _XPOLY_HXX //autogen
#include <svx/xpoly.hxx>
#endif
#ifndef _XOUTX_HXX //autogen
#include <svx/xoutx.hxx>
#endif

#ifndef XCHAR_HXX
#include "xchar.hxx"
#endif

#endif //USE_POLYGON


#ifndef NODE_HXX
#include "node.hxx"
#endif
#ifndef SYMBOL_HXX
#include "symbol.hxx"
#endif
#ifndef _SMMOD_HXX
#include "smmod.hxx"
#endif
#ifndef _MATHTYPE_HXX
#include "mathtype.hxx"
#endif

#include <math.h>
#include <float.h>

// define this to draw rectangles for debugging
//#define SM_RECT_DEBUG

////////////////////////////////////////
// SmTmpDevice
// A little class that behaves like a "const OutputDevice &" object but
// allows for font changes. The original font will be restored in the
// destructor! (Thus logically it's "const".)
// It's main purpose is to allow for the "const" in the 'OutputDevice'
// argument in the 'Arrange' functions.
// In fact we need to change the font in the 'OutputDevice' to the one
// of the node to calculate sizes (rectangles) for text.
//

class SmTmpDevice
{
    OutputDevice  &rOutDev;

    // disallow use of copy-constructor and assignment-operator
    SmTmpDevice(const SmTmpDevice &rTmpDev);
    SmTmpDevice & operator = (const SmTmpDevice &rTmpDev);

public:
    SmTmpDevice(OutputDevice &rTheDev);
    ~SmTmpDevice()                      { rOutDev.Pop(); }

    void SetFont(const Font &rNewFont)  { rOutDev.SetFont(rNewFont); }

    // conversion operator that allows only const member-functions from
    // 'OutputDevice' to be called. (Thus only the font can be changed
    // via the above 'SetFont')
    operator const OutputDevice & () const { return rOutDev; }
};


SmTmpDevice::SmTmpDevice(OutputDevice &rTheDev) :
    rOutDev(rTheDev)
{
    rOutDev.Push( PUSH_FONT | PUSH_MAPMODE );
    rOutDev.SetMapMode( MAP_100TH_MM );     //Immer fuer 100% fomatieren
}


///////////////////////////////////////////////////////////////////////////


SmNode::SmNode(SmNodeType eNodeType, const SmToken &rNodeToken)
{
    eType      = eNodeType;
    eScaleMode = SCALE_NONE;
    aNodeToken = rNodeToken;
}


SmNode::~SmNode()
{
}


SmNode * SmNode::GetLeftMost()
    //  returns leftmost node of current subtree.
    //! (this assumes the one with index 0 is always the leftmost subnode
    //! for the current node).
{
    SmNode *pNode = GetNumSubNodes() > 0 ?
                        GetSubNode(0) : NULL;

    return pNode ? pNode->GetLeftMost() : this;
}


void SmNode::SetPhantom(BOOL bIsPhantomP)
{
    if (! (Flags() & FLG_VISIBLE))
        bIsPhantom = bIsPhantomP;

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
            pNode->SetPhantom(bIsPhantom);
}


void SmNode::SetColor(const Color& rColor)
{
    if (! (Flags() & FLG_COLOR))
        GetFont().SetColor(rColor);

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
            pNode->SetColor(rColor);
}


void SmNode::SetAttribut(USHORT nAttrib)
{
    if (   nAttrib == ATTR_BOLD  &&  !(Flags() & FLG_BOLD)
        || nAttrib == ATTR_ITALIC  &&  !(Flags() & FLG_ITALIC))
        nAttributes |= nAttrib;

    SmNode *pNode;
    USHORT nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
            pNode->SetAttribut(nAttrib);
}


void SmNode::ClearAttribut(USHORT nAttrib)
{
    if (   nAttrib == ATTR_BOLD  &&  !(Flags() & FLG_BOLD)
        || nAttrib == ATTR_ITALIC  &&  !(Flags() & FLG_ITALIC))
        nAttributes &= ~nAttrib;

    SmNode *pNode;
    USHORT nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
            pNode->ClearAttribut(nAttrib);
}


void SmNode::SetFont(const SmFace &rFace)
{
    if (!(Flags() & FLG_FONT))
        GetFont() = rFace;

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
            pNode->SetFont(rFace);
}


void SmNode::SetFontSize(const Fraction &rSize, USHORT nType)
    //! 'rSize' is in units of pts
{
    Size  aSize;

    if (!(Flags() & FLG_SIZE))
    {
        Fraction  aVal (SmPtsTo100th_mm(rSize.GetNumerator()),
                        rSize.GetDenominator());
        //long    nHeight = SolarMath::Round(aVal);
        long      nHeight = (long)aVal;

        aSize = GetFont().GetSize();
        aSize.Width() = 0;
        switch(nType)
        {
            case FNTSIZ_ABSOLUT:
                aSize.Height() = nHeight;
                break;

            case FNTSIZ_PLUS:
                aSize.Height() += nHeight;
                break;

            case FNTSIZ_MINUS:
                aSize.Height() -= nHeight;
                break;

            case FNTSIZ_MULTIPLY:
                aSize.Height()  = (long) (Fraction(aSize.Height()) * rSize);
                break;

            case FNTSIZ_DIVIDE:
                if (rSize != Fraction(0L))
                    aSize.Height()  = (long) (Fraction(aSize.Height()) / rSize);
                break;
        }

        // check the requested size against maximum value
        static int __READONLY_DATA  nMaxVal = SmPtsTo100th_mm(128);
        if (aSize.Height() > nMaxVal)
            aSize.Height() = nMaxVal;

        GetFont().SetSize(aSize);
    }

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0;  i < nSize;  i++)
        if (pNode = GetSubNode(i))
            pNode->SetFontSize(rSize, nType);
}


void SmNode::SetSize(const Fraction &rSize)
{
    GetFont() *= rSize;

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0;  i < nSize;  i++)
        if (pNode = GetSubNode(i))
            pNode->SetSize(rSize);
}


void SmNode::SetRectHorAlign(RectHorAlign eHorAlign)
{
    if (!(Flags() & FLG_HORALIGN))
        eRectHorAlign = eHorAlign;

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
            pNode->SetRectHorAlign(eHorAlign);
}


void SmNode::PrepareAttributes()
{
    GetFont().SetWeight((Attributes() & ATTR_BOLD)   ? WEIGHT_BOLD   : WEIGHT_NORMAL);
    GetFont().SetItalic((Attributes() & ATTR_ITALIC) ? ITALIC_NORMAL : ITALIC_NONE);
}


void SmNode::Prepare(const SmFormat &rFormat)
{
#ifdef DEBUG
    bIsDebug    = TRUE;
#else
    bIsDebug    = FALSE;
#endif
    bIsPhantom  = FALSE;
    nFlags      = 0;
    nAttributes = 0;

    switch (rFormat.GetHorAlign())
    {   case AlignLeft:     eRectHorAlign = RHA_LEFT;   break;
        case AlignCenter:   eRectHorAlign = RHA_CENTER; break;
        case AlignRight:    eRectHorAlign = RHA_RIGHT;  break;
    }

    GetFont() = rFormat.GetFont(FNT_MATH);
    GetFont().SetCharSet(RTL_TEXTENCODING_SYMBOL);
    GetFont().SetWeight(WEIGHT_NORMAL);
    GetFont().SetItalic(ITALIC_NONE);

    SmNode *pNode;
    USHORT      nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
            pNode->Prepare(rFormat);
}


void  SmNode::ToggleDebug() const
    // toggle 'bIsDebug' in current subtree
{
#ifdef DEBUG
    SmNode *pThis = (SmNode *) this;

    pThis->bIsDebug = bIsDebug ? FALSE : TRUE;

    SmNode *pNode;
    USHORT      nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = pThis->GetSubNode(i))
            pNode->ToggleDebug();
#endif
}


#ifdef MAC
#pragma segment FrmNode_02

#else

#endif


void SmNode::Move(const Point& rPosition)
{
    if (rPosition.X() == 0  &&  rPosition.Y() == 0)
        return;

    SmRect::Move(rPosition);

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0;  i < nSize;  i++)
        if (pNode = GetSubNode(i))
            pNode->Move(rPosition);
}


void SmNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0;  i < nSize;  i++)
        if (pNode = GetSubNode(i))
            pNode->Arrange(rDev, rFormat);
}

void SmNode::CreateTextFromNode(String &rText)
{
    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    if (nSize > 1)
        rText.Append('{');
    for (USHORT i = 0;  i < nSize;  i++)
        if (pNode = GetSubNode(i))
            pNode->CreateTextFromNode(rText);
    if (nSize > 1)
    {
        rText.EraseTrailingChars();
        APPEND(rText,"} ");
    }
}


void SmNode::AdaptToX(const OutputDevice &rDev, ULONG nWidth)
{
}


void SmNode::AdaptToY(const OutputDevice &rDev, ULONG nHeight)
{
}


void SmNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    const SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    for (USHORT i = 0; i < nSize; i++)
        if (pNode = GetSubNode(i))
        {   Point  aOffset (pNode->GetTopLeft() - GetTopLeft());
            pNode->Draw(rDev, rPosition + aOffset);
        }

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}

const SmNode * SmNode::FindTokenAt(USHORT nRow, USHORT nCol) const
    // returns (first) ** visible ** (sub)node with the tokens text at
    // position 'nRow', 'nCol'.
    //! (there should be exactly one such node if any)
{
    if (    IsVisible()
        &&  nRow == GetToken().nRow
        &&  nCol >= GetToken().nCol  &&  nCol < GetToken().nCol + GetToken().aText.Len())
        return this;
    else
    {
        USHORT  nNumSubNodes = GetNumSubNodes();
        for (USHORT  i = 0;  i < nNumSubNodes;  i++)
        {   const SmNode *pNode = GetSubNode(i);

            if (!pNode)
                continue;

            const SmNode *pResult = pNode->FindTokenAt(nRow, nCol);
            if (pResult)
                return pResult;
        }
    }

    return 0;
}


const SmNode * SmNode::FindRectClosestTo(const Point &rPoint) const
{
    long          nDist   = LONG_MAX;
    const SmNode *pResult = 0;

    if (IsVisible())
        pResult = this;
    else
    {
        USHORT  nNumSubNodes = GetNumSubNodes();
        for (USHORT  i = 0;  i < nNumSubNodes;  i++)
        {   const SmNode *pNode = GetSubNode(i);

            if (!pNode)
                continue;

            long  nTmp;
            const SmNode *pFound = pNode->FindRectClosestTo(rPoint);
            if (pFound  &&  (nTmp = pFound->OrientedDist(rPoint)) < nDist)
            {   nDist   = nTmp;
                pResult = pFound;

                // quit immediately if 'rPoint' is inside the *should not
                // overlap with other rectangles* part.
                // This (partly) serves for getting the attributes in eg
                // "bar overstrike a".
                // ('nDist < 0' is used as *quick shot* to avoid evaluation of
                // the following expression, where the result is already determined)
                if (nDist < 0  &&  pFound->IsInsideRect(rPoint))
                    break;
            }
        }
    }

    return pResult;
}


///////////////////////////////////////////////////////////////////////////


SmStructureNode::~SmStructureNode()
{
    SmNode *pNode;

    for (USHORT i = 0;  i < GetNumSubNodes();  i++)
        if (pNode = GetSubNode(i))
            delete pNode;
}


void SmStructureNode::SetSubNodes(SmNode *pFirst, SmNode *pSecond, SmNode *pThird)
{
    if (pFirst)
        aSubNodes.Put(0, pFirst);
    if (pSecond)
        aSubNodes.Put(1, pSecond);
    if (pThird)
        aSubNodes.Put(2, pThird);
}


void SmStructureNode::SetSubNodes(const SmNodeArray &rNodeArray)
{
    aSubNodes = rNodeArray;
}


BOOL SmStructureNode::IsVisible() const
{
    return FALSE;
}


USHORT SmStructureNode::GetNumSubNodes() const
{
    return (USHORT) aSubNodes.GetSize();
}


SmNode * SmStructureNode::GetSubNode(USHORT nIndex)
{
    return aSubNodes.Get(nIndex);
}


///////////////////////////////////////////////////////////////////////////


BOOL SmVisibleNode::IsVisible() const
{
    return TRUE;
}


USHORT SmVisibleNode::GetNumSubNodes() const
{
    return 0;
}


SmNode * SmVisibleNode::GetSubNode(USHORT nIndex)
{
    return NULL;
}


///////////////////////////////////////////////////////////////////////////


void SmExpressionNode::CreateTextFromNode(String &rText)
{
    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    if (nSize > 1)
        rText.Append('{');
    for (USHORT i = 0;  i < nSize;  i++)
        if (pNode = GetSubNode(i))
        {
            pNode->CreateTextFromNode(rText);
            //Just a bit of foo to make unary +asd -asd +-asd -+asd look nice
            if (pNode->GetType() == NMATH)
                if ((nSize != 2) || ((rText.GetChar(rText.Len()-1) != '+') &&
                    (rText.GetChar(rText.Len()-1) != '-')))
                    rText.Append(' ');
        }

    if (nSize > 1)
    {
        rText.EraseTrailingChars();
        APPEND(rText,"} ");
    }
}


///////////////////////////////////////////////////////////////////////////


void SmTableNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
    // arranges all subnodes in one column
{
    Point rPosition;

    USHORT  nWidth  = 0;
    USHORT  nHeight = 0;
    SmNode *pNode;
    USHORT  nSize   = GetNumSubNodes();

    // make distance depend on font size
    long  nDist = +(rFormat.GetDistance(DIS_VERTICAL)
                    * GetFont().GetSize().Height()) / 100L;

    if (nSize < 1)
        return;

    // arrange subnodes and get maximum width of them
    long  nMaxWidth = 0,
          nTmp;
    USHORT i;
    for (i = 0; i < nSize;  i++)
        if (pNode = GetSubNode(i))
        {   pNode->Arrange(rDev, rFormat);
            if ((nTmp = pNode->GetItalicWidth()) > nMaxWidth)
                nMaxWidth = nTmp;
        }

    Point  aPos;
    SmRect::operator = (SmRect(nMaxWidth, 0));
    for (i = 0;  i < nSize;  i++)
    {   if (pNode = GetSubNode(i))
        {   const SmRect &rNodeRect = pNode->GetRect();
            const SmNode *pCoNode   = pNode->GetLeftMost();
            SmTokenType   eType     = pCoNode->GetToken().eType;
            RectHorAlign  eHorAlign = eType == TTEXT ?
                                        RHA_LEFT : pCoNode->GetRectHorAlign();

            aPos = rNodeRect.AlignTo(*this, RP_BOTTOM,
                        eHorAlign, RVA_BASELINE);
            if (i)
                aPos.Y() += nDist;
            pNode->MoveTo(aPos);
            ExtendBy(rNodeRect, nSize > 1 ? RCP_NONE : RCP_ARG);
        }
    }
}


SmNode * SmTableNode::GetLeftMost()
{
    return this;
}


/**************************************************************************/


void SmLineNode::Prepare(const SmFormat &rFormat)
{
    SmNode::Prepare(rFormat);

    //! wir verwenden hier den 'FNT_VARIABLE' Font, da er vom Ascent und Descent
    //! ia besser zum Rest der Formel passt als der 'FNT_MATH' Font.
    GetFont() = rFormat.GetFont(FNT_VARIABLE);
    Flags() |= FLG_FONT;
}


/**************************************************************************/


void SmLineNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
    // arranges all subnodes in one row with some extra space between
{
    SmRect::operator = (SmRect());

    SmNode *pNode;
    USHORT  nSize = GetNumSubNodes();
    USHORT i;
    for (i = 0; i < nSize;  i++)
        if (pNode = GetSubNode(i))
            pNode->Arrange(rDev, rFormat);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    // provide an empty rectangle with alignment parameters for the "current"
    // font (in order to make "a^1 {}_2^3 a_4" work correct, that is, have the
    // same sub-/supscript positions.)
    //! be sure to use a character that has explicitly defined HiAttribut
    //! line in rect.cxx such as 'a' in order to make 'vec a' look same to
    //! 'vec {a}'.
    SmRect::operator = (SmRect(aTmpDev, &rFormat, C2S("a"),
                               GetFont().GetBorderWidth()));
    // make sure that the rectangle occupies (almost) no space
    SetWidth(1);
    SetItalicSpaces(0, 0);

    if (nSize < 1)
        return;

    // make distance depend on font size
    long  nDist = +(rFormat.GetDistance(DIS_HORIZONTAL)
                    * GetFont().GetSize().Height()) / 100L;

    Point   aPos;
    for (i = 0;  i < nSize;  i++)
        if (pNode = GetSubNode(i))
        {
            aPos = pNode->AlignTo(*this, RP_RIGHT, RHA_CENTER, RVA_BASELINE);

            // no horizontal space before first node
            if (i)
                aPos.X() += nDist;

            pNode->MoveTo(aPos);
            ExtendBy(*pNode, i ? RCP_XOR : RCP_ARG);
        }
}


/**************************************************************************/


void SmExpressionNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
    // as 'SmLineNode::Arrange' but keeps alignment of leftmost subnode
{
    DBG_ASSERT(GetNumSubNodes() > 0, "Sm: keine subnodes");

    SmLineNode::Arrange(rDev, rFormat);

    //  copy alignment of leftmost subnode if any
    //! (this assumes its the one with index 0)
    SmNode *pNode = GetSubNode(0);
    if (pNode)
        SetRectHorAlign(pNode->GetRectHorAlign());
}


/**************************************************************************/


void SmUnHorNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    BOOL  bIsPostfix = GetToken().eType == TFACT;

    SmNode *pOper = GetSubNode(bIsPostfix ? 1 : 0),
           *pBody = GetSubNode(bIsPostfix ? 0 : 1);
    DBG_ASSERT(pOper, "Sm: NULL pointer");
    DBG_ASSERT(pBody, "Sm: NULL pointer");

    pOper->SetSize(Fraction (rFormat.GetRelSize(SIZ_OPERATOR), 100));
    pOper->Arrange(rDev, rFormat);
    pBody->Arrange(rDev, rFormat);

    Point  aPos = pOper->AlignTo(*pBody, bIsPostfix ? RP_RIGHT : RP_LEFT,
                        RHA_CENTER, RVA_BASELINE);
    // add a bit space between operator and argument
    // (worst case -{1 over 2} where - and over have almost no space inbetween)
    long  nDelta = pOper->GetFont().GetSize().Height() / 20;
    if (bIsPostfix)
        aPos.X() += nDelta;
    else
        aPos.X() -= nDelta;
    pOper->MoveTo(aPos);

    SmRect::operator = (*pBody);
    long  nOldBot = GetBottom();

    ExtendBy(*pOper, RCP_XOR);

    // workaround for Bug 50865: "a^2 a^+2" have different baselines
    // for exponents (if size of exponent is large enough)
    SetBottom(nOldBot);
}


/**************************************************************************/


void SmRootNode::GetHeightVerOffset(const SmRect &rRect,
                                    long &rHeight, long &rVerOffset) const
    // calculate height and vertical offset of root sign suitable for 'rRect'
{
    rVerOffset = (rRect.GetBottom() - rRect.GetAlignB()) / 2;
    rHeight    = rRect.GetHeight() - rVerOffset;

    DBG_ASSERT(rHeight    >= 0, "Sm : Ooops...");
    DBG_ASSERT(rVerOffset >= 0, "Sm : Ooops...");
}


Point SmRootNode::GetExtraPos(const SmRect &rRootSymbol,
                              const SmRect &rExtra) const
{
    const Size &rSymSize = rRootSymbol.GetSize();

    Point  aPos = rRootSymbol.GetTopLeft()
            + Point((rSymSize.Width()  * 70) / 100,
                    (rSymSize.Height() * 52) / 100);

    // from this calculate topleft edge of 'rExtra'
    aPos.X() -= rExtra.GetWidth() + rExtra.GetItalicRightSpace();
    aPos.Y() -= rExtra.GetHeight();
    // if there's enough space move a bit less to the right
    // examples: "nroot i a", "nroot j a"
    // (it looks better if we don't use italic-spaces here)
    long  nX = rRootSymbol.GetLeft() + (rSymSize.Width() * 30) / 100;
    if (aPos.X() > nX)
        aPos.X() = nX;

    return aPos;
}


void SmRootNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    //! pExtra needs to have the smaller index than pRootSym in order to
    //! not to get the root symbol but the pExtra when clicking on it in the
    //! GraphicWindow. (That is because of the simplicity of the algorithm
    //! that finds the node corresponding to a mouseclick in the window.)
    SmNode *pExtra   = GetSubNode(0),
           *pRootSym = GetSubNode(1),
           *pBody    = GetSubNode(2);
    DBG_ASSERT(pRootSym, "Sm: NULL pointer");
    DBG_ASSERT(pBody,    "Sm: NULL pointer");

    pBody->Arrange(rDev, rFormat);

    long  nHeight,
          nVerOffset;
    GetHeightVerOffset(*pBody, nHeight, nVerOffset);
    nHeight += rFormat.GetDistance(DIS_ROOT)
               * GetFont().GetSize().Height() / 100L;

    pRootSym->AdaptToX(rDev, pBody->GetItalicWidth());
    pRootSym->AdaptToY(rDev, nHeight);

    pRootSym->Arrange(rDev, rFormat);

    Point  aPos = pRootSym->AlignTo(*pBody, RP_LEFT, RHA_CENTER, RVA_BASELINE);
    //! overrride calulated vertical position
    aPos.Y()  = pRootSym->GetTop() + pBody->GetBottom() - pRootSym->GetBottom();
    aPos.Y() -= nVerOffset;
    pRootSym->MoveTo(aPos);

    if (pExtra)
    {   pExtra->SetSize(Fraction(rFormat.GetRelSize(SIZ_INDEX), 100));
        pExtra->Arrange(rDev, rFormat);

        aPos = GetExtraPos(*pRootSym, *pExtra);
        pExtra->MoveTo(aPos);
    }

    SmRect::operator = (*pBody);
    ExtendBy(*pRootSym, RCP_THIS);
    if (pExtra)
        ExtendBy(*pExtra, RCP_THIS, (BOOL) TRUE);
}


void SmRootNode::CreateTextFromNode(String &rText)
{
    SmNode *pExtra = GetSubNode(0);
    if (pExtra)
    {
        APPEND(rText,"nroot ");
        pExtra->CreateTextFromNode(rText);
    }
    else
        APPEND(rText,"sqrt ");
    GetSubNode(2)->CreateTextFromNode(rText);
}


/**************************************************************************/


void SmBinHorNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pLeft  = GetSubNode(0),
           *pOper  = GetSubNode(1),
           *pRight = GetSubNode(2);
    DBG_ASSERT(pLeft  != NULL, "Sm: NULL pointer");
    DBG_ASSERT(pOper  != NULL, "Sm: NULL pointer");
    DBG_ASSERT(pRight != NULL, "Sm: NULL pointer");

    pOper->SetSize(Fraction (rFormat.GetRelSize(SIZ_OPERATOR), 100));

    pLeft ->Arrange(rDev, rFormat);
    pOper ->Arrange(rDev, rFormat);
    pRight->Arrange(rDev, rFormat);

    const SmRect &rOpRect = pOper->GetRect();

    long nDist = (rOpRect.GetWidth() *
                 rFormat.GetDistance(DIS_HORIZONTAL)) / 100L;

    SmRect::operator = (*pLeft);

    Point aPos;
    aPos = pOper->AlignTo(*this, RP_RIGHT, RHA_CENTER, RVA_BASELINE);
    aPos.X() += nDist;
    pOper->MoveTo(aPos);
    ExtendBy(*pOper, RCP_XOR);

    aPos = pRight->AlignTo(*this, RP_RIGHT, RHA_CENTER, RVA_BASELINE);
    aPos.X() += nDist;

    pRight->MoveTo(aPos);
    ExtendBy(*pRight, RCP_XOR);
}


/**************************************************************************/


void SmBinVerNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pNum   = GetSubNode(0),
           *pLine  = GetSubNode(1),
           *pDenom = GetSubNode(2);
    DBG_ASSERT(pNum,   "Sm : NULL pointer");
    DBG_ASSERT(pLine,  "Sm : NULL pointer");
    DBG_ASSERT(pDenom, "Sm : NULL pointer");

    BOOL  bIsTextmode = rFormat.IsTextmode();
    if (bIsTextmode)
    {
        Fraction  aFraction(rFormat.GetRelSize(SIZ_INDEX), 100);
        pNum  ->SetSize(aFraction);
        pLine ->SetSize(aFraction);
        pDenom->SetSize(aFraction);
    }

    pNum  ->Arrange(rDev, rFormat);
    pDenom->Arrange(rDev, rFormat);

    long  nFontHeight = GetFont().GetSize().Height(),
          nExtLen     = nFontHeight * rFormat.GetDistance(DIS_FRACTION) / 100L,
          nThick      = nFontHeight * rFormat.GetDistance(DIS_STROKEWIDTH) / 100L,
          nWidth      = Max(pNum->GetItalicWidth(), pDenom->GetItalicWidth()),
          nNumDist    = bIsTextmode ? 0 :
                            nFontHeight * rFormat.GetDistance(DIS_NUMERATOR)   / 100L,
          nDenomDist  = bIsTextmode ? 0 :
                            nFontHeight * rFormat.GetDistance(DIS_DENOMINATOR) / 100L;

    pLine->AdaptToX(rDev, nWidth + 2 * nExtLen);
    pLine->AdaptToY(rDev, nThick);
    pLine->Arrange(rDev, rFormat);

    // get horizontal alignment for numerator
    const SmNode *pLM       = pNum->GetLeftMost();
    RectHorAlign  eHorAlign = pLM->GetToken().eType == TTEXT ?
                                    RHA_LEFT : pLM->GetRectHorAlign();

    // move numerator to its position
    Point  aPos = pNum->AlignTo(*pLine, RP_TOP, eHorAlign, RVA_BASELINE);
    aPos.Y() -= nNumDist;
    pNum->MoveTo(aPos);

    // get horizontal alignment for denominator
    pLM       = pDenom->GetLeftMost();
    eHorAlign = pLM->GetToken().eType == TTEXT ?
                    RHA_LEFT : pLM->GetRectHorAlign();

    // move denominator to its position
    aPos = pDenom->AlignTo(*pLine, RP_BOTTOM, eHorAlign, RVA_BASELINE);
    aPos.Y() += nDenomDist;
    pDenom->MoveTo(aPos);

    SmRect::operator = (*pNum);
    ExtendBy(*pDenom, RCP_NONE).ExtendBy(*pLine, RCP_NONE, pLine->GetCenterY());
}

void SmBinVerNode::CreateTextFromNode(String &rText)
{
    SmNode *pNum   = GetSubNode(0),
           *pLine  = GetSubNode(1),
           *pDenom = GetSubNode(2);
    pNum->CreateTextFromNode(rText);
    APPEND(rText,"over ");
    pDenom->CreateTextFromNode(rText);
}


SmNode * SmBinVerNode::GetLeftMost()
{
    return this;
}


/**************************************************************************/


double Det(const Point &rHeading1, const Point &rHeading2)
    // gibt den Wert der durch die beiden Punkte gebildeten Determinante
    // zurück
{
    return rHeading1.X() * rHeading2.Y() - rHeading1.Y() * rHeading2.X();
}


BOOL IsPointInLine(const Point &rPoint1,
                   const Point &rPoint2, const Point &rHeading2)
    // ergibt TRUE genau dann, wenn der Punkt 'rPoint1' zu der Gerade gehört die
    // durch den Punkt 'rPoint2' geht und den Richtungsvektor 'rHeading2' hat
{
    DBG_ASSERT(rHeading2 != Point(), "Sm : 0 vector");

    BOOL bRes = FALSE;
    const double eps = 5.0 * DBL_EPSILON;

    double fLambda;
    if (labs(rHeading2.X()) > labs(rHeading2.Y()))
    {
        fLambda = (rPoint1.X() - rPoint2.X()) / (double) rHeading2.X();
        bRes = fabs(rPoint1.Y() - (rPoint2.Y() + fLambda * rHeading2.Y())) < eps;
    }
    else
    {
        fLambda = (rPoint1.Y() - rPoint2.Y()) / (double) rHeading2.Y();
        bRes = fabs(rPoint1.X() - (rPoint2.X() + fLambda * rHeading2.X())) < eps;
    }

    return bRes;
}


USHORT GetLineIntersectionPoint(Point &rResult,
                                const Point& rPoint1, const Point &rHeading1,
                                const Point& rPoint2, const Point &rHeading2)
{
    DBG_ASSERT(rHeading1 != Point(), "Sm : 0 vector");
    DBG_ASSERT(rHeading2 != Point(), "Sm : 0 vector");

    USHORT nRes = 1;
    const double eps = 5.0 * DBL_EPSILON;

    // sind die Richtumgsvektoren linear abhängig ?
    double  fDet = Det(rHeading1, rHeading2);
    if (fabs(fDet) < eps)
    {
        nRes    = IsPointInLine(rPoint1, rPoint2, rHeading2) ? USHRT_MAX : 0;
        rResult = nRes ? rPoint1 : Point();
    }
    else
    {
        // hier achten wir nicht auf Rechengenauigkeit
        // (das würde aufwendiger und lohnt sich hier kaum)
        double fLambda = (    (rPoint1.Y() - rPoint2.Y()) * rHeading2.X()
                            - (rPoint1.X() - rPoint2.X()) * rHeading2.Y())
                         / fDet;
        rResult = Point(rPoint1.X() + (long) (fLambda * rHeading1.X()),
                        rPoint1.Y() + (long) (fLambda * rHeading1.Y()));
    }

    return nRes;
}



SmBinDiagonalNode::SmBinDiagonalNode(const SmToken &rNodeToken)
:   SmStructureNode(NBINDIAGONAL, rNodeToken)
{
    bAscending = FALSE;
    SetNumSubNodes(3);
}


void SmBinDiagonalNode::GetOperPosSize(Point &rPos, Size &rSize,
                        const Point &rDiagPoint, double fAngleDeg) const
    // gibt die Position und Größe für den Diagonalstrich zurück.
    // Vor.: das SmRect des Nodes gibt die Begrenzung vor(!), muß also selbst
    //      bereits bekannt sein.

{
    const double  fPi   = 3.1415926535897932384626433;
    double  fAngleRad   = fAngleDeg / 180.0 * fPi;
    long    nRectLeft   = GetItalicLeft(),
            nRectRight  = GetItalicRight(),
            nRectTop    = GetTop(),
            nRectBottom = GetBottom();
    Point   aRightHdg     (100, 0),
            aDownHdg      (0, 100),
            aDiagHdg      ( (long)(100.0 * cos(fAngleRad)),
                            (long)(-100.0 * sin(fAngleRad)) );

    long  nLeft, nRight, nTop, nBottom;     // Ränder des Rechtecks für die
                                            // Diagonale
    Point aPoint;
    if (IsAscending())
    {
        //
        // obere rechte Ecke bestimmen
        //
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectTop), aRightHdg,
            rDiagPoint, aDiagHdg);
        //
        // gibt es einen Schnittpunkt mit dem oberen Rand ?
        if (aPoint.X() <= nRectRight)
        {
            nRight = aPoint.X();
            nTop   = nRectTop;
        }
        else
        {
            // es muß einen Schnittpunkt mit dem rechten Rand geben!
            GetLineIntersectionPoint(aPoint,
                Point(nRectRight, nRectTop), aDownHdg,
                rDiagPoint, aDiagHdg);

            nRight = nRectRight;
            nTop   = aPoint.Y();
        }

        //
        // untere linke Ecke bestimmen
        //
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectBottom), aRightHdg,
            rDiagPoint, aDiagHdg);
        //
        // gibt es einen Schnittpunkt mit dem unteren Rand ?
        if (aPoint.X() >= nRectLeft)
        {
            nLeft   = aPoint.X();
            nBottom = nRectBottom;
        }
        else
        {
            // es muß einen Schnittpunkt mit dem linken Rand geben!
            GetLineIntersectionPoint(aPoint,
                Point(nRectLeft, nRectTop), aDownHdg,
                rDiagPoint, aDiagHdg);

            nLeft   = nRectLeft;
            nBottom = aPoint.Y();
        }
    }
    else
    {
        //
        // obere linke Ecke bestimmen
        //
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectTop), aRightHdg,
            rDiagPoint, aDiagHdg);
        //
        // gibt es einen Schnittpunkt mit dem oberen Rand ?
        if (aPoint.X() >= nRectLeft)
        {
            nLeft = aPoint.X();
            nTop  = nRectTop;
        }
        else
        {
            // es muß einen Schnittpunkt mit dem linken Rand geben!
            GetLineIntersectionPoint(aPoint,
                Point(nRectLeft, nRectTop), aDownHdg,
                rDiagPoint, aDiagHdg);

            nLeft = nRectLeft;
            nTop  = aPoint.Y();
        }

        //
        // untere rechte Ecke bestimmen
        //
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectBottom), aRightHdg,
            rDiagPoint, aDiagHdg);
        //
        // gibt es einen Schnittpunkt mit dem unteren Rand ?
        if (aPoint.X() <= nRectRight)
        {
            nRight  = aPoint.X();
            nBottom = nRectBottom;
        }
        else
        {
            // es muß einen Schnittpunkt mit dem rechten Rand geben!
            GetLineIntersectionPoint(aPoint,
                Point(nRectRight, nRectTop), aDownHdg,
                rDiagPoint, aDiagHdg);

            nRight  = nRectRight;
            nBottom = aPoint.Y();
        }
    }

    rSize = Size(nRight - nLeft + 1, nBottom - nTop + 1);
    rPos.X() = nLeft;
    rPos.Y() = nTop;
}


void SmBinDiagonalNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    //! die beiden Argumente müssen in den Subnodes vor dem Operator kommen,
    //! damit das anklicken im GraphicWindow den FormulaCursor richtig setzt
    //! (vgl SmRootNode)
    SmNode *pLeft  = GetSubNode(0),
           *pRight = GetSubNode(1);
    DBG_ASSERT(pLeft, "Sm : NULL pointer");
    DBG_ASSERT(pRight, "Sm : NULL pointer");

    DBG_ASSERT(GetSubNode(2)->GetType() == NPOLYLINE, "Sm : falscher Nodetyp");
    SmPolyLineNode *pOper = (SmPolyLineNode *) GetSubNode(2);
    DBG_ASSERT(pOper, "Sm : NULL pointer");

    //! some routines being called extract some info from the OutputDevice's
    //! font (eg the space to be used for borders OR the font name(!!)).
    //! Thus the font should reflect the needs and has to be set!
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    pLeft->Arrange(aTmpDev, rFormat);
    pRight->Arrange(aTmpDev, rFormat);

    // implizit die Weite (incl Rand) des Diagonalstrichs ermitteln
    pOper->Arrange(aTmpDev, rFormat);

    long nDelta = pOper->GetWidth() * 8 / 10;

    // TopLeft Position vom rechten Argument ermitteln
    Point aPos;
    aPos.X() = pLeft->GetItalicRight() + nDelta + pRight->GetItalicLeftSpace();
    if (IsAscending())
        aPos.Y() = pLeft->GetBottom() + nDelta;
    else
        aPos.Y() = pLeft->GetTop() - nDelta - pRight->GetHeight();

    pRight->MoveTo(aPos);

    // neue Baseline bestimmen
    long nBaseline = IsAscending() ? (pLeft->GetBottom() + pRight->GetTop()) / 2
                        : (pLeft->GetTop() + pRight->GetBottom()) / 2;
    Point  aLogCenter ((pLeft->GetItalicRight() + pRight->GetItalicLeft()) / 2,
                       nBaseline);

    SmRect::operator = (*pLeft);
    ExtendBy(*pRight, RCP_NONE);


    // Position und Größe des Diagonalstrich ermitteln
    Size  aSize;
    GetOperPosSize(aPos, aSize, aLogCenter, IsAscending() ? 60.0 : -60.0);

    // die Größe setzen
    pOper->AdaptToX(rDev, aSize.Width());
    pOper->AdaptToY(rDev, aSize.Height());
    // und diese wirksam machen
    pOper->Arrange(aTmpDev, rFormat);

    pOper->MoveTo(aPos);

    ExtendBy(*pOper, RCP_NONE, nBaseline);
}


/**************************************************************************/


void SmSubSupNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    DBG_ASSERT(GetNumSubNodes() == 1 + SUBSUP_NUM_ENTRIES,
               "Sm: falsche Anzahl von subnodes");

    SmNode *pBody = GetBody();
    DBG_ASSERT(pBody, "Sm: NULL pointer");

    long  nOrigHeight = pBody->GetFont().GetSize().Height();

    pBody->Arrange(rDev, rFormat);

    const SmRect &rBodyRect = pBody->GetRect();
    SmRect::operator = (rBodyRect);

    // line that separates sub- and supscript rectangles
    long  nDelimLine = SmFromTo(GetAlignB(), GetAlignT(), 0.4);

    Point  aPos;
    long   nDelta, nDist;

    // iterate over all possible sub-/supscripts
    SmRect  aTmpRect (rBodyRect);
    for (int i = 0;  i < SUBSUP_NUM_ENTRIES;  i++)
    {   SmSubSup  eSubSup = (SmSubSup) i;   // cast
        SmNode *pSubSup = GetSubSup(eSubSup);

        if (!pSubSup)
            continue;

        // switch position of limits if we are in textmode
        if (rFormat.IsTextmode()  &&  (GetToken().nGroup & TGLIMIT))
            switch (eSubSup)
            {   case CSUB:  eSubSup = RSUB;     break;
                case CSUP:  eSubSup = RSUP;     break;
            }

        // prevent sub-/supscripts from diminishing in size
        // (as would be in "a_{1_{2_{3_4}}}")
        if (GetFont().GetSize().Height() > rFormat.GetBaseSize().Height() / 3)
        {
            USHORT nIndex = (eSubSup == CSUB  ||  eSubSup == CSUP) ?
                                    SIZ_LIMITS : SIZ_INDEX;
            Fraction  aFraction ( rFormat.GetRelSize(nIndex), 100 );
            pSubSup->SetSize(aFraction);
        }

        pSubSup->Arrange(rDev, rFormat);

        BOOL  bIsTextmode = rFormat.IsTextmode();
        nDist = 0;

        //! be sure that CSUB, CSUP are handled before the other cases!
        switch (eSubSup)
        {   case RSUB :
            case LSUB :
                if (!bIsTextmode)
                    nDist = nOrigHeight
                            * rFormat.GetDistance(DIS_SUBSCRIPT) / 100L;
                aPos  = pSubSup->GetRect().AlignTo(aTmpRect,
                                eSubSup == LSUB ? RP_LEFT : RP_RIGHT,
                                RHA_CENTER, RVA_BOTTOM);
                aPos.Y() += nDist;
                nDelta = nDelimLine - aPos.Y();
                if (nDelta > 0)
                    aPos.Y() += nDelta;
                break;
            case RSUP :
            case LSUP :
                if (!bIsTextmode)
                    nDist = nOrigHeight
                            * rFormat.GetDistance(DIS_SUPERSCRIPT) / 100L;
                aPos  = pSubSup->GetRect().AlignTo(aTmpRect,
                                eSubSup == LSUP ? RP_LEFT : RP_RIGHT,
                                RHA_CENTER, RVA_TOP);
                aPos.Y() -= nDist;
                nDelta = aPos.Y() + pSubSup->GetHeight() - nDelimLine;
                if (nDelta > 0)
                    aPos.Y() -= nDelta;
                break;
            case CSUB :
                if (!bIsTextmode)
                    nDist = nOrigHeight
                            * rFormat.GetDistance(DIS_LOWERLIMIT) / 100L;
                aPos = pSubSup->GetRect().AlignTo(rBodyRect, RP_BOTTOM,
                                RHA_CENTER, RVA_BASELINE);
                aPos.Y() += nDist;
                break;
            case CSUP :
                if (!bIsTextmode)
                    nDist = nOrigHeight
                            * rFormat.GetDistance(DIS_UPPERLIMIT) / 100L;
                aPos = pSubSup->GetRect().AlignTo(rBodyRect, RP_TOP,
                                RHA_CENTER, RVA_BASELINE);
                aPos.Y() -= nDist;
                break;
            default :
                DBG_ASSERT(FALSE, "Sm: unbekannter Fall");
        }

        pSubSup->MoveTo(aPos);
        ExtendBy(*pSubSup, RCP_THIS, (BOOL) TRUE);

        // update rectangle to which  RSUB, RSUP, LSUB, LSUP
        // will be aligned to
        if (eSubSup == CSUB  ||  eSubSup == CSUP)
            aTmpRect = *this;
    }
}

void SmSubSupNode::CreateTextFromNode(String &rText)
{
    SmNode *pNode;
    GetSubNode(0)->CreateTextFromNode(rText);

    if (pNode = GetSubNode(LSUB+1))
    {
        APPEND(rText,"lsub ");
        pNode->CreateTextFromNode(rText);
    }
    if (pNode = GetSubNode(LSUP+1))
    {
        APPEND(rText,"lsup ");
        pNode->CreateTextFromNode(rText);
    }
    if (pNode = GetSubNode(CSUB+1))
    {
        APPEND(rText,"csub ");
        pNode->CreateTextFromNode(rText);
    }
    if (pNode = GetSubNode(CSUP+1))
    {
        APPEND(rText,"csup ");
        pNode->CreateTextFromNode(rText);
    }
    if (pNode = GetSubNode(RSUB+1))
    {
        rText.EraseTrailingChars();
        rText.Append('_');
        pNode->CreateTextFromNode(rText);
    }
    if (pNode = GetSubNode(RSUP+1))
    {
        rText.EraseTrailingChars();
        rText.Append('^');
        pNode->CreateTextFromNode(rText);
    }
}


/**************************************************************************/

void SmBraceNode::CreateTextFromNode(String &rText)
{
    if (GetScaleMode() == SCALE_HEIGHT)
        APPEND(rText,"left ");
    GetSubNode(0)->CreateTextFromNode(rText);
    rText.Append(' ');
    GetSubNode(1)->CreateTextFromNode(rText);
    if (GetScaleMode() == SCALE_HEIGHT)
        APPEND(rText,"right ");
    GetSubNode(2)->CreateTextFromNode(rText);
    rText.Append(' ');

}

void SmBraceNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pLeft  = GetSubNode(0),
           *pBody  = GetSubNode(1),
           *pRight = GetSubNode(2);
    DBG_ASSERT(pLeft,  "Sm: NULL pointer");
    DBG_ASSERT(pBody,  "Sm: NULL pointer");
    DBG_ASSERT(pRight, "Sm: NULL pointer");

    pBody->Arrange(rDev, rFormat);

    BOOL  bIsScaleNormal = rFormat.IsScaleNormalBrackets(),
          bScale         = pBody->GetHeight() > 0  &&
                           (GetScaleMode() == SCALE_HEIGHT  ||  bIsScaleNormal),
          bIsABS         = GetToken().eType == TABS;

    long  nFaceHeight = GetFont().GetSize().Height();

    // Übergröße in % ermitteln
    USHORT  nPerc = 0;
    if (!bIsABS && bScale)
    {   // im Fall von Klammern mit Übergröße...
        int  nIndex = GetScaleMode() == SCALE_HEIGHT ?
                            DIS_BRACKETSIZE : DIS_NORMALBRACKETSIZE;
        nPerc = rFormat.GetDistance(nIndex);
    }

    // ermitteln der Höhe für die Klammern
    long  nBraceHeight;
    if (bScale)
    {
        nBraceHeight = pBody->GetType() == NBRACEBODY ?
                              ((SmBracebodyNode *) pBody)->GetBodyHeight()
                            : pBody->GetHeight();
        nBraceHeight += 2 * (nBraceHeight * nPerc / 100L);
    }
    else
        nBraceHeight = nFaceHeight;

    // Abstand zum Argument
    nPerc = bIsABS ? 0 : rFormat.GetDistance(DIS_BRACKETSPACE);
    long  nDist = nFaceHeight * nPerc / 100L;

    // sofern erwünscht skalieren der Klammern auf die gewünschte Größe
    if (bScale)
    {
        Size  aSize (pLeft->GetFont().GetSize());
        DBG_ASSERT(pRight->GetFont().GetSize() == aSize,
                    "Sm : unterschiedliche Fontgrößen");
        aSize.Width() = Min((long) nBraceHeight * 60L / 100L,
                            rFormat.GetBaseSize().Height() * 3L / 2L);

        xub_Unicode cChar = pLeft->GetToken().cMathChar;
        if (cChar != MS_LINE  &&  cChar != MS_DLINE)
            pLeft ->GetFont().SetSize(aSize);

        cChar = pRight->GetToken().cMathChar;
        if (cChar != MS_LINE  &&  cChar != MS_DLINE)
            pRight->GetFont().SetSize(aSize);

        pLeft ->AdaptToY(rDev, nBraceHeight);
        pRight->AdaptToY(rDev, nBraceHeight);
    }

    pLeft ->Arrange(rDev, rFormat);
    pRight->Arrange(rDev, rFormat);

    // damit auch "\(a\) - (a) - left ( a right )" vernünftig aussieht
    RectVerAlign  eVerAlign = bScale ? RVA_CENTERY : RVA_BASELINE;

    Point         aPos;
    aPos = pLeft->AlignTo(*pBody, RP_LEFT, RHA_CENTER, eVerAlign);
    aPos.X() -= nDist;
    pLeft->MoveTo(aPos);

    aPos = pRight->AlignTo(*pBody, RP_RIGHT, RHA_CENTER, eVerAlign);
    aPos.X() += nDist;
    pRight->MoveTo(aPos);

    SmRect::operator = (*pBody);
    ExtendBy(*pLeft, RCP_THIS).ExtendBy(*pRight, RCP_THIS);
}


/**************************************************************************/


void SmBracebodyNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    USHORT  nNumSubNodes = GetNumSubNodes();
    if (nNumSubNodes == 0)
        return;

    // arrange arguments
    USHORT i;
    for (i = 0;  i < nNumSubNodes;  i += 2)
        GetSubNode(i)->Arrange(rDev, rFormat);

    // build reference rectangle with necessary info for vertical alignment
    SmRect  aRefRect (*GetSubNode(0));
    for (i = 0;  i < nNumSubNodes;  i += 2)
    {
        SmRect aTmpRect (*GetSubNode(i));
        Point  aPos = aTmpRect.AlignTo(aRefRect, RP_RIGHT, RHA_CENTER, RVA_BASELINE);
        aTmpRect.MoveTo(aPos);
        aRefRect.ExtendBy(aTmpRect, RCP_XOR);
    }

    nBodyHeight = aRefRect.GetHeight();

    // scale separators to required height and arrange them
    BOOL bScale  = GetScaleMode() == SCALE_HEIGHT  ||  rFormat.IsScaleNormalBrackets();
    long nHeight = bScale ? aRefRect.GetHeight() : GetFont().GetSize().Height();
    int  nIndex  = GetScaleMode() == SCALE_HEIGHT ?
                        DIS_BRACKETSIZE : DIS_NORMALBRACKETSIZE;
    USHORT nPerc   = rFormat.GetDistance(nIndex);
    if (bScale)
        nHeight += 2 * (nHeight * nPerc / 100L);
    for (i = 1;  i < nNumSubNodes;  i += 2)
    {
        SmNode *pNode = GetSubNode(i);
        pNode->AdaptToY(rDev, nHeight);
        pNode->Arrange(rDev, rFormat);
    }

    // horizontal distance between argument and brackets or separators
    long  nDist = GetFont().GetSize().Height()
                  * rFormat.GetDistance(DIS_BRACKETSPACE) / 100L;

    SmNode *pLeft = GetSubNode(0);
    SmRect::operator = (*pLeft);
    for (i = 1;  i < nNumSubNodes;  i++)
    {
        BOOL          bIsSeparator = i % 2 != 0;
        RectVerAlign  eVerAlign    = bIsSeparator ? RVA_CENTERY : RVA_BASELINE;

        SmNode *pRight = GetSubNode(i);
        Point  aPosX = pRight->AlignTo(*pLeft,   RP_RIGHT, RHA_CENTER, eVerAlign),
               aPosY = pRight->AlignTo(aRefRect, RP_RIGHT, RHA_CENTER, eVerAlign);
        aPosX.X() += nDist;

        pRight->MoveTo(Point(aPosX.X(), aPosY.Y()));
        ExtendBy(*pRight, bIsSeparator ? RCP_THIS : RCP_XOR);

        pLeft = pRight;
    }
}


/**************************************************************************/


void SmVerticalBraceNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pBody   = GetSubNode(0),
           *pBrace  = GetSubNode(1),
           *pScript = GetSubNode(2);
    DBG_ASSERT(pBody,   "Sm: NULL pointer!");
    DBG_ASSERT(pBrace,  "Sm: NULL pointer!");
    DBG_ASSERT(pScript, "Sm: NULL pointer!");

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    pBody->Arrange(aTmpDev, rFormat);

    // Groesse wie bei Grenzen fuer diesen Teil
    pScript->SetSize( Fraction( rFormat.GetRelSize(SIZ_LIMITS), 100 ) );
    // etwas hoehere Klammern als normal
    pBrace ->SetSize( Fraction(3, 2) );

    long  nItalicWidth = pBody->GetItalicWidth();
    if (nItalicWidth > 0)
        pBrace->AdaptToX(aTmpDev, nItalicWidth);

    pBrace ->Arrange(aTmpDev, rFormat);
    pScript->Arrange(aTmpDev, rFormat);

    // die relativen Position und die Abstaende zueinander bestimmen
    RectPos  eRectPos;
    long nFontHeight = pBody->GetFont().GetSize().Height();
    long nDistBody   = nFontHeight * rFormat.GetDistance(DIS_ORNAMENTSIZE),
         nDistScript = nFontHeight;
    if (GetToken().eType == TOVERBRACE)
    {
        eRectPos = RP_TOP;
        nDistBody    = - nDistBody;
        nDistScript *= - rFormat.GetDistance(DIS_UPPERLIMIT);
    }
    else // TUNDERBRACE
    {
        eRectPos = RP_BOTTOM;
        nDistScript *= + rFormat.GetDistance(DIS_LOWERLIMIT);
    }
    nDistBody   /= 100L;
    nDistScript /= 100L;

    Point  aPos = pBrace->AlignTo(*pBody, eRectPos, RHA_CENTER, RVA_BASELINE);
    aPos.Y() += nDistBody;
    pBrace->MoveTo(aPos);

    aPos = pScript->AlignTo(*pBrace, eRectPos, RHA_CENTER, RVA_BASELINE);
    aPos.Y() += nDistScript;
    pScript->MoveTo(aPos);

    SmRect::operator = (*pBody);
    ExtendBy(*pBrace, RCP_THIS).ExtendBy(*pScript, RCP_THIS);
}


/**************************************************************************/

#ifdef MAC
#pragma segment FrmNode_03
#endif


SmNode * SmOperNode::GetSymbol()
{
    SmNode *pNode = GetSubNode(0);
    DBG_ASSERT(pNode, "Sm: NULL pointer!");

    if (pNode->GetType() == NSUBSUP)
        pNode = ((SmSubSupNode *) pNode)->GetBody();

    DBG_ASSERT(pNode, "Sm: NULL pointer!");
    return pNode;
}


long SmOperNode::CalcSymbolHeight(const SmNode &rSymbol,
                                  const SmFormat &rFormat) const
    // returns the font height to be used for operator-symbol
{
    long  nHeight = GetFont().GetSize().Height();

    SmTokenType  eType = GetToken().eType;
    if (eType == TLIM  ||  eType == TLIMINF ||  eType == TLIMSUP)
        return nHeight;

    if (!rFormat.IsTextmode())
    {
        // set minimum size ()
        nHeight += (nHeight * 20L) / 100L;

        nHeight += nHeight
                   * rFormat.GetDistance(DIS_OPERATORSIZE) / 100L;
        nHeight = nHeight * 686L / 845L;
    }

    // correct user-defined symbols to match height of sum from StarMath
    // font
    if (rSymbol.GetToken().eType == TSPECIAL)
        nHeight = nHeight * 845L / 686L;

    return nHeight;
}


void SmOperNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pOper = GetSubNode(0);
    SmNode *pBody = GetSubNode(1);

    DBG_ASSERT(pOper, "Sm: Subnode fehlt");
    DBG_ASSERT(pBody, "Sm: Subnode fehlt");

    SmNode *pSymbol = GetSymbol();
    pSymbol->SetSize(Fraction(CalcSymbolHeight(*pSymbol, rFormat),
                              pSymbol->GetFont().GetSize().Height()));

    pBody->Arrange(rDev, rFormat);
    pOper->Arrange(rDev, rFormat);

    long  nOrigHeight = GetFont().GetSize().Height(),
          nDist = nOrigHeight
                  * rFormat.GetDistance(DIS_OPERATORSPACE) / 100L;

    Point aPos = pOper->AlignTo(*pBody, RP_LEFT, RHA_CENTER, /*RVA_CENTERY*/RVA_MID);
    aPos.X() -= nDist;
    pOper->MoveTo(aPos);

    SmRect::operator = (*pBody);
    ExtendBy(*pOper, RCP_THIS);
}


/**************************************************************************/


void SmAlignNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
    // setzt im ganzen subtree (incl aktuellem node) das alignment
{
    DBG_ASSERT(GetNumSubNodes() > 0, "Sm: SubNode fehlt");

    SmNode  *pNode = GetSubNode(0);

    RectHorAlign  eHorAlign;
    switch (GetToken().eType)
    {
        case TALIGNL:   eHorAlign = RHA_LEFT;   break;
        case TALIGNC:   eHorAlign = RHA_CENTER; break;
        case TALIGNR:   eHorAlign = RHA_RIGHT;  break;
    }
    SetRectHorAlign(eHorAlign);

    pNode->Arrange(rDev, rFormat);

    SmRect::operator = (pNode->GetRect());
}


/**************************************************************************/


void SmAttributNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pAttr = GetSubNode(0),
           *pBody = GetSubNode(1);
    DBG_ASSERT(pBody, "Sm: Body fehlt");
    DBG_ASSERT(pAttr, "Sm: Attribut fehlt");

    pBody->Arrange(rDev, rFormat);

    if (GetScaleMode() == SCALE_WIDTH)
        pAttr->AdaptToX(rDev, pBody->GetItalicWidth());
    pAttr->Arrange(rDev, rFormat);

    // get relative position of attribut
    RectVerAlign  eVerAlign;
    long          nDist = 0;
    switch (GetToken().eType)
    {   case TUNDERLINE :
            eVerAlign = RVA_ATTRIBUT_LO;
            break;
        case TOVERSTRIKE :
            eVerAlign = RVA_ATTRIBUT_MID;
            break;
        default :
            eVerAlign = RVA_ATTRIBUT_HI;
            if (pBody->GetType() == NATTRIBUT)
                nDist = GetFont().GetSize().Height()
                        * rFormat.GetDistance(DIS_ORNAMENTSPACE) / 100L;
    }
    Point  aPos = pAttr->AlignTo(*pBody, RP_ATTRIBUT, RHA_CENTER, eVerAlign);
    aPos.Y() -= nDist;
    pAttr->MoveTo(aPos);

    SmRect::operator = (*pBody);
    ExtendBy(*pAttr, RCP_THIS, (BOOL) TRUE);
}


/**************************************************************************/




void SmFontNode::CreateTextFromNode(String &rText)
{
    switch (GetToken().eType)
    {
        case TBOLD:
            APPEND(rText,"bold ");
            break;
        case TNBOLD:
            APPEND(rText,"nbold ");
            break;
        case TITALIC:
            APPEND(rText,"italic ");
            break;
        case TNITALIC:
            APPEND(rText,"nitalic ");
            break;
        case TPHANTOM:
            APPEND(rText,"phantom ");
            break;
        case TSIZE:
            {
                APPEND(rText,"size ");
                switch (nSizeType)
                {
                    case FNTSIZ_PLUS:
                        rText.Append('+');
                        break;
                    case FNTSIZ_MINUS:
                        rText.Append('-');
                        break;
                    case FNTSIZ_MULTIPLY:
                        rText.Append('*');
                        break;
                    case FNTSIZ_DIVIDE:
                        rText.Append('/');
                        break;
                    case FNTSIZ_ABSOLUT:
                    default:
                        break;
                }
                String sResult;
                SolarMath::DoubleToString(sResult,
                    static_cast<double>(aFontSize),'A',INT_MAX,'.',sal_True);
                rText.Append(sResult);
                rText.Append(' ');
            }
            break;
        case TBLACK:
            APPEND(rText,"color black ");
            break;
        case TWHITE:
            APPEND(rText,"color white ");
            break;
        case TRED:
            APPEND(rText,"color red ");
            break;
        case TGREEN:
            APPEND(rText,"color green ");
            break;
        case TBLUE:
            APPEND(rText,"color blue ");
            break;
        case TCYAN:
            APPEND(rText,"color cyan ");
            break;
        case TMAGENTA:
            APPEND(rText,"color magenta ");
            break;
        case TYELLOW:
            APPEND(rText,"color yellow ");
            break;
        case TSANS:
            APPEND(rText,"font sans ");
            break;
        case TSERIF:
            APPEND(rText,"font serif ");
            break;
        case TFIXED:
            APPEND(rText,"font fixed ");
            break;
        default:
            break;
    }
    GetSubNode(1)->CreateTextFromNode(rText);
}


void SmFontNode::Prepare(const SmFormat &rFormat)
{
    //! prepare subnodes first
    SmNode::Prepare(rFormat);

    int  nFnt = -1;
    switch (GetToken().eType)
    {
        case TFIXED:    nFnt = FNT_FIXED;   break;
        case TSANS:     nFnt = FNT_SANS;    break;
        case TSERIF:    nFnt = FNT_SERIF;   break;
    }
    if (nFnt != -1)
    {   GetFont() = rFormat.GetFont(nFnt);
        SetFont(GetFont());
    }

    //! prevent overwrites of this font by 'Arrange' or 'SetFont' calls of
    //! other font nodes (those with lower depth in the tree)
    Flags() |= FLG_FONT;
}


void SmFontNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pNode = GetSubNode(1);
    DBG_ASSERT(pNode, "Sm: SubNode fehlt");

    switch (GetToken().eType)
    {   case TSIZE :
            pNode->SetFontSize(aFontSize, nSizeType);
            break;
        case TSANS :
        case TSERIF :
        case TFIXED :
            pNode->SetFont(GetFont());
            break;
        case TUNKNOWN : break;  // no assertion on "font <?> <?>"

        case TPHANTOM : SetPhantom(TRUE);               break;
        case TBOLD :    SetAttribut(ATTR_BOLD);         break;
        case TITALIC :  SetAttribut(ATTR_ITALIC);       break;
        case TNBOLD :   ClearAttribut(ATTR_BOLD);       break;
        case TNITALIC : ClearAttribut(ATTR_ITALIC);     break;

        case TBLACK :   SetColor(Color(COL_BLACK));     break;
        case TWHITE :   SetColor(Color(COL_WHITE));     break;
        case TRED :     SetColor(Color(COL_RED));       break;
        case TGREEN :   SetColor(Color(COL_GREEN));     break;
        case TBLUE :    SetColor(Color(COL_BLUE));      break;
        case TCYAN :    SetColor(Color(COL_CYAN));      break;
        case TMAGENTA : SetColor(Color(COL_MAGENTA));   break;
        case TYELLOW :  SetColor(Color(COL_YELLOW));    break;

        default:
            DBG_ASSERT(FALSE, "Sm: unbekannter Fall");
    }

    pNode->Arrange(rDev, rFormat);

    SmRect::operator = (pNode->GetRect());
}


void SmFontNode::SetSizeParameter(const Fraction& rValue, USHORT Type)
{
    nSizeType = Type;
    aFontSize = rValue;
}


/**************************************************************************/


SmPolyLineNode::SmPolyLineNode(const SmToken &rNodeToken)
:   SmGraphicNode(NPOLYLINE, rNodeToken)
{
    aPoly.SetSize(2);
    nWidth = 0;
}


void SmPolyLineNode::AdaptToX(const OutputDevice &rDev, ULONG nWidth)
{
    aToSize.Width() = nWidth;
}


void SmPolyLineNode::AdaptToY(const OutputDevice &rDev, ULONG nHeight)
{
    GetFont().FreezeBorderWidth();
    aToSize.Height() = nHeight;
}


void SmPolyLineNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    //! some routines being called extract some info from the OutputDevice's
    //! font (eg the space to be used for borders OR the font name(!!)).
    //! Thus the font should reflect the needs and has to be set!
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    long  nBorderwidth = GetFont().GetBorderWidth();

    //
    // Das Polygon mit den beiden Endpunkten bilden
    //
    DBG_ASSERT(aPoly.GetSize() == 2, "Sm : falsche Anzahl von Punkten");
    Point  aPointA, aPointB;
    if (GetToken().eType == TWIDESLASH)
    {
        aPointA.X() = nBorderwidth;
        aPointA.Y() = aToSize.Height() - nBorderwidth;
        aPointB.X() = aToSize.Width() - nBorderwidth;
        aPointB.Y() = nBorderwidth;
    }
    else
    {
        DBG_ASSERT(GetToken().eType == TWIDEBACKSLASH, "Sm : unerwartetes Token");
        aPointA.X() =
        aPointA.Y() = nBorderwidth;
        aPointB.X() = aToSize.Width() - nBorderwidth;
        aPointB.Y() = aToSize.Height() - nBorderwidth;
    }
    aPoly.SetPoint(aPointA, 0);
    aPoly.SetPoint(aPointB, 1);

    long  nThick       = GetFont().GetSize().Height()
                            * rFormat.GetDistance(DIS_STROKEWIDTH) / 100L;
    nWidth = nThick + 2 * nBorderwidth;

    SmRect::operator = (SmRect(aToSize.Width(), aToSize.Height()));
}


void SmPolyLineNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    long nBorderwidth = GetFont().GetBorderWidth();

    LineInfo  aInfo;
    aInfo.SetWidth(nWidth - 2 * nBorderwidth);

    Point aOffset (Point() - aPoly.GetBoundRect().TopLeft()
                   + Point(nBorderwidth, nBorderwidth)),
          aPos (rPosition + aOffset);
    ((Polygon &) aPoly).Move(aPos.X(), aPos.Y());

    Color aOldCol (rDev.GetLineColor());
    rDev.SetLineColor( GetFont().GetColor() );

    rDev.DrawPolyLine(aPoly, aInfo);

    rDev.SetLineColor( aOldCol );

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


/**************************************************************************/


void SmPolygonNode::AdaptToX(const OutputDevice &rDev, ULONG nWidth)
{
    aToSize.Width() = nWidth;
}


void SmPolygonNode::AdaptToY(const OutputDevice &rDev, ULONG nHeight)
{
    GetFont().FreezeBorderWidth();
    aToSize.Height() = nHeight;
}


void SmPolygonNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    //! some routines being called extract some info from the OutputDevice's
    //! font (eg the space to be used for borders OR the font name(!!)).
    //! Thus the font should reflect the needs and has to be set!
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    long nBorderWidth = GetFont().GetBorderWidth();

    // das Polygon der gewaehlten FontSize anpassen
    Size aSize     (rDev.GetFont().GetSize()),
         aOrigSize (aPolygon.GetOrigFontSize());
    double  fScaleY = (double) aSize.Height() / aOrigSize.Height(),
            fScaleX = aSize.Width() ?
                            (double) aSize.Width() / aOrigSize.Width() : fScaleY;
    aPolygon.ScaleBy(fScaleX / aPolygon.GetScaleX(), fScaleY / aPolygon.GetScaleY());

    long  nTmp;
    if ((nTmp = aToSize.Width())  > 0)
        // Breite anpassen und dabei Platz für Rand links und rechts lassen
        // (das resultierende SmRect soll den Rand beinhalten und trotzdem
        // nicht breiter als gewünscht sein)
        aPolygon.AdaptToX(aTmpDev, Max(nTmp - 2L * nBorderWidth, 10L));
    if ((nTmp = aToSize.Height()) > 0)
        // wie oben jedoch mit oberen und unterem Rand
        aPolygon.AdaptToY(aTmpDev, Max(nTmp - 2L * nBorderWidth, 10L));

    if (aPolygon.GetChar() == xub_Unicode('\0'))
        SmRect::operator = (SmRect());
    else
        SmRect::operator = (SmRect(aTmpDev, &rFormat, aPolygon, nBorderWidth));
}


void SmPolygonNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    rDev.Push(PUSH_FILLCOLOR | PUSH_LINECOLOR);
    rDev.SetFillColor(GetFont().GetColor());
    rDev.SetLineColor();

    // calculate offset to position the polygon centered within the
    // surrounding rectangle of the node
    // (the root sign will be positioned right within the rectangle in order
    // to attach the horizontal bar easily)
    Size   aPolySize (aPolygon.GetBoundRect(rDev).GetSize()),
           aRectSize (GetRect().GetSize());
    Point  aOffset   ((aRectSize.Width()  - aPolySize.Width())  /
                            (aPolygon.GetChar() == MS_SQRT ? 1 : 2),
                      (aRectSize.Height() - aPolySize.Height()) / 2);

    aPolygon.Draw(rDev, rPosition + aOffset);

    rDev.Pop();

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


/**************************************************************************/


void SmRootSymbolNode::DrawBar(OutputDevice &rDev, const Point &rPosition) const
{
    // get polygon and rectangle
    SmPolygon  aBarPoly = SmPolygon( (char) MS_BAR );

    // extra length to close small (wedge formed) gap between root-sign and
    // horizontal bar
    long  nExtraLen = aBarPoly.GetBoundRect(rDev).GetSize().Height() / 2;

    aBarPoly.AdaptToX((OutputDevice &) rDev, nBodyWidth + nExtraLen);
    aBarPoly.ScaleBy( 1.0, (double) GetFont().GetSize().Height()
                                / aBarPoly.GetOrigFontSize().Height());

    Point  aDrawPos (rPosition);
    aDrawPos.X() -= nExtraLen;
    aDrawPos = rDev.PixelToLogic(rDev.LogicToPixel(aDrawPos));

    rDev.Push(PUSH_FILLCOLOR | PUSH_LINECOLOR);
    rDev.SetFillColor(GetFont().GetColor());
    rDev.SetLineColor();

    aBarPoly.Draw(rDev, aDrawPos);

    rDev.Pop();
}


void SmRootSymbolNode::AdaptToY(const OutputDevice &rDev, ULONG nHeight)
{
    // etwas extra Länge damit der horizontale Balken später über dem
    // Argument positioniert ist
    SmPolygonNode::AdaptToY(rDev, nHeight + nHeight / 10L);
}


void SmRootSymbolNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    // draw root-sign itself
    SmPolygonNode::Draw(rDev, rPosition);

    // get offset for horizontal bar
    long   nPolyHeight = GetPolygon().GetBoundRect(rDev).GetSize().Height();
    Point  aBarOffset (GetWidth(), (GetHeight() - nPolyHeight) / 2);

    DrawBar(rDev, rPosition + aBarOffset);

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


void SmRootSymbolNode::AdaptToX(const OutputDevice &rDev, ULONG nWidth)
{
    nBodyWidth = nWidth;
}


/**************************************************************************/


void SmRectangleNode::AdaptToX(const OutputDevice &rDev, ULONG nWidth)
{
    aToSize.Width() = nWidth;
}


void SmRectangleNode::AdaptToY(const OutputDevice &rDev, ULONG nHeight)
{
    GetFont().FreezeBorderWidth();
    aToSize.Height() = nHeight;
}


void SmRectangleNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    long  nFontHeight = GetFont().GetSize().Height();
    long  nWidth  = aToSize.Width(),
          nHeight = aToSize.Height();
    if (nHeight == 0)
        nHeight = nFontHeight / 30;
    if (nWidth == 0)
        nWidth  = nFontHeight / 3;

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    // add some borderspace
    ULONG  nBorderWidth = GetFont().GetBorderWidth();
    //nWidth  += nBorderWidth;
    nHeight += 2 * nBorderWidth;

    //! use this method in order to have 'SmRect::HasAlignInfo() == TRUE'
    //! and thus having the attribut-fences updated in 'SmRect::ExtendBy'
    SmRect::operator = (SmRect(nWidth, nHeight));
}


void SmRectangleNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    rDev.Push(PUSH_FILLCOLOR | PUSH_LINECOLOR | PUSH_FONT);
    rDev.SetFillColor(GetFont().GetColor());
    rDev.SetLineColor();

    rDev.SetFont(GetFont());
    ULONG  nBorderWidth = GetFont().GetBorderWidth();

    // get rectangle and remove borderspace
    Rectangle  aTmp (AsRectangle() + rPosition - GetTopLeft());
    aTmp.Left()   += nBorderWidth;
    aTmp.Right()  -= nBorderWidth;
    aTmp.Top()    += nBorderWidth;
    aTmp.Bottom() -= nBorderWidth;

    DBG_ASSERT(aTmp.GetHeight() > 0  &&  aTmp.GetWidth() > 0,
               "Sm: leeres Rechteck");

    //! avoid GROWING AND SHRINKING of drawn rectangle when constantly
    //! increasing zoomfactor.
    //  This is done by shifting it's output-position to a point that
    //  corresponds exactly to a pixel on the output device.
    Point  aPos (rDev.PixelToLogic(rDev.LogicToPixel(aTmp.TopLeft())));
    aTmp.SetPos(aPos);

    rDev.DrawRect(aTmp);

    rDev.Pop();

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


/**************************************************************************/


void SmTextNode::Prepare(const SmFormat &rFormat)
{
    SmNode::Prepare(rFormat);

    aText = GetToken().aText;
    GetFont() = rFormat.GetFont(GetFontDesc());

    if (GetFont().GetItalic() == ITALIC_NORMAL)
        Attributes() |= ATTR_ITALIC;
    if (GetFont().GetWeight() == WEIGHT_BOLD)
        Attributes() |= ATTR_BOLD;

};


void SmTextNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    USHORT  nSizeDesc = GetFontDesc() == FNT_FUNCTION ?
                            SIZ_FUNCTION : SIZ_TEXT;
    GetFont() *= Fraction (rFormat.GetRelSize(nSizeDesc), 100);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, aText, GetFont().GetBorderWidth()));
}

void SmTextNode::CreateTextFromNode(String &rText)
{
    if (GetToken().eType == TTEXT)
        rText.Append('\"');
    else if ((GetToken().eType == TIDENT) &&
        (GetFontDesc() == FNT_FUNCTION))
    {
        //Search for existing functions and remove extraenous keyword
        APPEND(rText,"func ");
    }
    rText.Append(GetToken().aText);
    if (GetToken().eType == TTEXT)
        rText.Append('\"');
    rText.Append(' ');
}

void SmTextNode::Draw(OutputDevice &rDev, const Point& rPosition) const
{
    if (IsPhantom()  ||  aText.Len() == 0  ||  aText.GetChar(0) == xub_Unicode('\0'))
        return;

    rDev.Push(PUSH_FONT);
    rDev.SetFont(GetFont());

    Point  aPos (rPosition);
    aPos.Y() += GetBaselineOffset();
    // auf Pixelkoordinaten runden
    aPos = rDev.PixelToLogic( rDev.LogicToPixel(aPos) );

    rDev.DrawStretchText(aPos, GetWidth(), aText);

    rDev.Pop();

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


/**************************************************************************/

void SmMatrixNode::CreateTextFromNode(String &rText)
{
    APPEND(rText,"matrix {");
    for (int i = 0;  i < nNumRows;  i++)
    {
        for (int j = 0;  j < nNumCols;  j++)
        {
            SmNode *pNode = GetSubNode(i * nNumCols + j);
            pNode->CreateTextFromNode(rText);
            if (j != nNumCols-1)
                APPEND(rText,"# ");
        }
        if (i != nNumRows-1)
            APPEND(rText,"## ");
    }
    rText.EraseTrailingChars();
    APPEND(rText,"} ");
}


void SmMatrixNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    Point   aPosition,
            aOffset;
    SmNode *pNode;
    int     i, j;

    // initialize array that is to hold the maximum widhts of all
    // elements (subnodes) in that column.
    long *pColWidth = new long[nNumCols];
    for (j = 0;  j  < nNumCols;  j++)
        pColWidth[j] = 0;

    // arrange subnodes and calculate the aboves arrays contents
    for (i = GetNumSubNodes() - 1;  i >= 0; i--)
        if (pNode = GetSubNode(i))
        {   pNode->Arrange(rDev, rFormat);

            int  nCol = i % nNumCols;
            pColWidth[nCol]
                = Max(pColWidth[nCol], pNode->GetItalicWidth());
        }

    // norm distance from which the following two are calcutated
    const int  nNormDist = 3 * GetFont().GetSize().Height();

    // define horizontal and vertical minimal distances that seperate
    // the elements
    long  nHorDist = nNormDist * rFormat.GetDistance(DIS_MATRIXCOL) / 100L,
          nVerDist = nNormDist * rFormat.GetDistance(DIS_MATRIXROW) / 100L;

    // build array that holds the leftmost position for each column
    long *pColLeft = new long[nNumCols];
    long  nX = 0;
    for (j = 0;  j < nNumCols;  j++)
    {   pColLeft[j] = nX;
        nX += pColWidth[j] + nHorDist;
    }

    Point   aPos, aDelta;
    SmRect  aLineRect;
    SmRect::operator = (SmRect());
    for (i = 0;  i < nNumRows;  i++)
    {   aLineRect = SmRect();
        for (j = 0;  j < nNumCols;  j++)
        {   SmNode *pNode = GetSubNode(i * nNumCols + j);
            DBG_ASSERT(pNode, "Sm: NULL pointer");

            const SmRect &rNodeRect = pNode->GetRect();

            // align all baselines in that row if possible
            aPos = rNodeRect.AlignTo(aLineRect, RP_RIGHT, RHA_CENTER, RVA_BASELINE);
            aPos.X() += nHorDist;

            // get horizontal alignment
            const SmNode *pCoNode   = pNode->GetLeftMost();
            SmTokenType   eType     = pCoNode->GetToken().eType;
            RectHorAlign  eHorAlign = eType == TTEXT ?
                                        RHA_LEFT : pCoNode->GetRectHorAlign();

            // caculate horizontal position of element depending on column
            // and horizontal alignment
            switch (eHorAlign)
            {   case RHA_LEFT:
                    aPos.X() = rNodeRect.GetLeft() + pColLeft[j];
                    break;
                case RHA_CENTER:
                    aPos.X() = rNodeRect.GetLeft() + pColLeft[j]
                               + pColWidth[j] / 2
                               - rNodeRect.GetItalicCenterX();
                    break;
                case RHA_RIGHT:
                    aPos.X() = rNodeRect.GetLeft() + pColLeft[j]
                               + pColWidth[j] - rNodeRect.GetItalicWidth();
                    break;
            }

            pNode->MoveTo(aPos);
            aLineRect.ExtendBy(rNodeRect, RCP_XOR);
        }

        aPos = aLineRect.AlignTo(*this, RP_BOTTOM, RHA_CENTER, RVA_BASELINE);
        aPos.Y() += nVerDist;

        // move 'aLineRect' and rectangles in that line to final position
        aDelta.X() = 0;     // since horizontal alignment is already done
        aDelta.Y() = aPos.Y() - aLineRect.GetTop();
        aLineRect.Move(aDelta);
        for (j = 0;  j < nNumCols;  j++)
            if (pNode = GetSubNode(i * nNumCols + j))
                pNode->Move(aDelta);

        ExtendBy(aLineRect, RCP_NONE);
    }

    delete [] pColLeft;
    delete [] pColWidth;
}


void SmMatrixNode::SetRowCol(USHORT nMatrixRows, USHORT nMatrixCols)
{
    nNumRows = nMatrixRows;
    nNumCols = nMatrixCols;
}


SmNode * SmMatrixNode::GetLeftMost()
{
    return this;
}


/**************************************************************************/


SmMathSymbolNode::SmMathSymbolNode(const SmToken &rNodeToken)
:   SmSpecialNode(NMATH, rNodeToken, FNT_MATH)
{
    xub_Unicode cChar = GetToken().cMathChar;
    if ((xub_Unicode) '\0' != cChar)
        SetText( cChar );
}


void SmMathSymbolNode::AdaptToX(const OutputDevice &rDev, ULONG nWidth)
    // Scale charwidth to mach 'nWidth' while keeping the height
    // (that is: actually the font width will be scaled)
{
    // Since there is no function to do this, we try to approximate
    // it:
    //
    Font &rFace = GetFont();
    Size  aFntSize (rFace.GetSize());

    //! however the result is a bit better with 'nWidth' as initial
    //! font width
    aFntSize.Width() = nWidth;
    rFace.SetSize(aFntSize);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(rFace);

    // get denominator of error factor for width
    long  nDenom = SmRect(aTmpDev, NULL, GetText(),
                          GetFont().GetBorderWidth()).GetItalicWidth();

    // scale fontwidth with this error factor
    aFntSize.Width() *= nWidth;
    aFntSize.Width() /= nDenom ? nDenom : 1;

    GetFont().SetSize(aFntSize);
}


void SmMathSymbolNode::AdaptToY(const OutputDevice &rDev, ULONG nHeight)
{
    GetFont().FreezeBorderWidth();

    Font &rFace = GetFont();
    Size  aFntSize (rFace.GetSize());

    // da wir nur die Höhe skalieren wollen müsen wir hier ggf die Fontweite
    // ermitteln um diese beizubehalten.
    if (aFntSize.Width() == 0)
    {
        OutputDevice &rDevNC = (OutputDevice &) rDev;
        rDevNC.Push(PUSH_FONT);
        rDevNC.SetFont(rFace);
        aFntSize.Width() = rDev.GetFontMetric().GetSize().Width();
        rDevNC.Pop();
    }
    DBG_ASSERT(aFntSize.Width() != 0, "Sm: ");

    //! however the result is a bit better with 'nHeight' as initial
    //! font height
    aFntSize.Height() = nHeight;
    rFace.SetSize(aFntSize);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(rFace);

    // get denominator of error factor for height
    long  nDenom = SmRect(aTmpDev, NULL, GetText(),
                          GetFont().GetBorderWidth()).GetHeight();

    // scale fontwidth with this error factor
    aFntSize.Height() *= nHeight;
    aFntSize.Height() /= nDenom ? nDenom : 1;

    GetFont().SetSize(aFntSize);
}


void SmMathSymbolNode::Prepare(const SmFormat &rFormat)
{
    SmNode::Prepare(rFormat);

    DBG_ASSERT(GetFont().GetCharSet() == RTL_TEXTENCODING_SYMBOL,
        "Sm : falsches CHARSET für Zeichen aus dem StarMath Font");

    Flags() |= FLG_FONT | FLG_ITALIC;
};


void SmMathSymbolNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    const XubString &rText = GetText();

    if (rText.Len() == 0  ||  rText.GetChar(0) == xub_Unicode('\0'))
    {   SmRect::operator = (SmRect());
        return;
    }

    PrepareAttributes();

    GetFont() *= Fraction (rFormat.GetRelSize(SIZ_TEXT), 100);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, rText, GetFont().GetBorderWidth()));
}

void SmMathSymbolNode::CreateTextFromNode(String &rText)
{
//  rText.Append(MathType::aMathTypeTable[GetToken().cMathChar&0x00FF]);
    String sStr;
    MathType::LookupChar(MathType::aMathTypeTable[GetToken().cMathChar&0x00FF],
        sStr);
    sStr.EraseLeadingAndTrailingChars();
    rText.Append(sStr);
}



/**************************************************************************/


void SmSpecialNode::Prepare(const SmFormat &rFormat)
{
    SmNode::Prepare(rFormat);

    SmSym     *pSym;
    SmModule  *pp = SM_MOD1();

    Size  aOldSize = GetFont().GetSize();
    if (pSym = pp->GetSymSetManager()->GetSymbol(GetToken().aText))
    {   SetText( pSym->GetCharacter() );
        GetFont() = pSym->GetFace();

        if (GetFont().GetName().EqualsIgnoreCaseAscii("StarMath"))
            GetFont().SetCharSet(RTL_TEXTENCODING_SYMBOL);
    }
    else
    {   SetText( GetToken().aText );
        GetFont() = rFormat.GetFont(FNT_VARIABLE);
    }
    GetFont().SetSize(aOldSize);

    //! eigentlich sollten nur WEIGHT_NORMAL und WEIGHT_BOLD vorkommen...
    //! In der sms-Datei gibt es jedoch zB auch 'WEIGHT_ULTRALIGHT'
    //! daher vergleichen wir hier mit  >  statt mit  !=  .
    //! (Langfristig sollte die Notwendigkeit für 'PrepareAttribut', und damit
    //! für dieses hier, mal entfallen.)
    if (GetFont().GetWeight() > WEIGHT_NORMAL)
        SetAttribut(ATTR_BOLD);
    if (GetFont().GetItalic() != ITALIC_NONE)
        SetAttribut(ATTR_ITALIC);

    Flags() |= FLG_FONT;
};


void SmSpecialNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, GetText(), GetFont().GetBorderWidth()));
}


void SmSpecialNode::Draw(OutputDevice &rDev, const Point& rPosition) const
{
    //! since this chars might come from any font, that we may not have
    //! set to ALIGN_BASELINE yet, we do it now.
    ((SmSpecialNode *)this)->GetFont().SetAlign(ALIGN_BASELINE);

    SmTextNode::Draw(rDev, rPosition);
}


/**************************************************************************/


void SmGlyphSpecialNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, GetText(),
                               GetFont().GetBorderWidth()).AsGlyphRect());
}


/**************************************************************************/


void SmPlaceNode::Prepare(const SmFormat &rFormat)
{
    SmNode::Prepare(rFormat);

    GetFont().SetColor(COL_GRAY);
    Flags() |= FLG_COLOR | FLG_FONT | FLG_ITALIC;
};


void SmPlaceNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, GetText(), GetFont().GetBorderWidth()));
}


/**************************************************************************/


void SmErrorNode::Prepare(const SmFormat &rFormat)
{
    SmNode::Prepare(rFormat);

    GetFont().SetColor(COL_RED);
    Flags() |= FLG_VISIBLE | FLG_BOLD | FLG_ITALIC
               | FLG_COLOR | FLG_FONT | FLG_SIZE;
}


void SmErrorNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    const XubString &rText = GetText();
    DBG_ASSERT(rText.Len() == 1  &&  rText.GetChar(0) == (xub_Unicode) MS_ERROR,
                "Sm : Text ist kein ERROR Symbol");
    SmRect::operator = (SmRect(aTmpDev, &rFormat, rText, GetFont().GetBorderWidth()));
}


/**************************************************************************/


void SmBlankNode::IncreaseBy(const SmToken &rToken)
{
    switch(rToken.eType)
    {
        case TBLANK:    nNum += 4;  break;
        case TSBLANK:   nNum += 1;  break;
    }
}


void SmBlankNode::Prepare(const SmFormat &rFormat)
{
    SmNode::Prepare(rFormat);

    //! hier muß/sollte es lediglich nicht der StarMath Font sein,
    //! damit für das in Arrange verwendete Zeichen ein "normales"
    //! (ungecliptes) Rechteck erzeugt wird.
    GetFont() = rFormat.GetFont(FNT_VARIABLE);

    Flags() |= FLG_FONT | FLG_BOLD | FLG_ITALIC;
}


void SmBlankNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev);
    aTmpDev.SetFont(GetFont());

    // Abstand von der Fonthöhe abhängig machen
    // (damit er beim skalieren (zB size *2 {a ~ b}) mitwächst)
    long  nDist  = GetFont().GetSize().Height() / 10L,
          nSpace = nNum * nDist;

    // ein SmRect mit Baseline und allem drum und dran besorgen
    SmRect::operator = (SmRect(aTmpDev, &rFormat, XubString(xub_Unicode(' ')),
                               GetFont().GetBorderWidth()));

    // und dieses auf die gewünschte Breite bringen
    SetItalicSpaces(0, 0);
    SetWidth(nSpace);
}



