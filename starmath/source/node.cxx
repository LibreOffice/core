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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_starmath.hxx"

#include "node.hxx"
#include "rect.hxx"
#include "symbol.hxx"
#include "smmod.hxx"
#include "document.hxx"
#include "view.hxx"
#include "mathtype.hxx"

#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include <rtl/math.hxx>
#include <tools/color.hxx>
#include <vcl/metric.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <sfx2/module.hxx>

#include <math.h>
#include <float.h>


#define APPEND(str,ascii) str.AppendAscii(RTL_CONSTASCII_STRINGPARAM(ascii))

// define this to draw rectangles for debugging
//#define SM_RECT_DEBUG


using ::rtl::OUString;


////////////////////////////////////////
// SmTmpDevice
// Allows for font and color changes. The original settings will be restored
// in the destructor.
// It's main purpose is to allow for the "const" in the 'OutputDevice'
// argument in the 'Arrange' functions and restore changes made in the 'Draw'
// functions.
// Usually a MapMode of 1/100th mm will be used.
//

class SmTmpDevice
{
    OutputDevice  &rOutDev;

    // disallow use of copy-constructor and assignment-operator
    SmTmpDevice(const SmTmpDevice &rTmpDev);
    SmTmpDevice & operator = (const SmTmpDevice &rTmpDev);

    Color   Impl_GetColor( const Color& rColor );

public:
    SmTmpDevice(OutputDevice &rTheDev, sal_Bool bUseMap100th_mm);
    ~SmTmpDevice()  { rOutDev.Pop(); }

    void SetFont(const Font &rNewFont);

    void SetLineColor( const Color& rColor )    { rOutDev.SetLineColor( Impl_GetColor(rColor) ); }
    void SetFillColor( const Color& rColor )    { rOutDev.SetFillColor( Impl_GetColor(rColor) ); }
    void SetTextColor( const Color& rColor )    { rOutDev.SetTextColor( Impl_GetColor(rColor) ); }

    operator OutputDevice & () { return rOutDev; }
};


SmTmpDevice::SmTmpDevice(OutputDevice &rTheDev, sal_Bool bUseMap100th_mm) :
    rOutDev(rTheDev)
{
    rOutDev.Push( PUSH_FONT | PUSH_MAPMODE |
                  PUSH_LINECOLOR | PUSH_FILLCOLOR | PUSH_TEXTCOLOR );
    if (bUseMap100th_mm  &&  MAP_100TH_MM != rOutDev.GetMapMode().GetMapUnit())
    {
        DBG_ERROR( "incorrect MapMode?" );
        rOutDev.SetMapMode( MAP_100TH_MM );     //Immer fuer 100% fomatieren
    }
}


Color SmTmpDevice::Impl_GetColor( const Color& rColor )
{
    ColorData nNewCol = rColor.GetColor();
    if (COL_AUTO == nNewCol)
    {
        if (OUTDEV_PRINTER == rOutDev.GetOutDevType())
            nNewCol = COL_BLACK;
        else
        {
            Color aBgCol( rOutDev.GetBackground().GetColor() );
            if (OUTDEV_WINDOW == rOutDev.GetOutDevType())
                aBgCol = ((Window &) rOutDev).GetDisplayBackground().GetColor();

            nNewCol = SM_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;

            Color aTmpColor( nNewCol );
            if (aBgCol.IsDark() && aTmpColor.IsDark())
                nNewCol = COL_WHITE;
            else if (aBgCol.IsBright() && aTmpColor.IsBright())
                nNewCol = COL_BLACK;
        }
    }
    return Color( nNewCol );
}


void SmTmpDevice::SetFont(const Font &rNewFont)
{
    rOutDev.SetFont( rNewFont );
    rOutDev.SetTextColor( Impl_GetColor( rNewFont.GetColor() ) );
}


///////////////////////////////////////////////////////////////////////////


SmNode::SmNode(SmNodeType eNodeType, const SmToken &rNodeToken)
{
    eType      = eNodeType;
    eScaleMode = SCALE_NONE;
    aNodeToken = rNodeToken;
    nAccIndex  = -1;
}


SmNode::~SmNode()
{
}


sal_Bool SmNode::IsVisible() const
{
    return sal_False;
}


sal_uInt16 SmNode::GetNumSubNodes() const
{
    return 0;
}


SmNode * SmNode::GetSubNode(sal_uInt16 /*nIndex*/)
{
    return NULL;
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


void SmNode::SetPhantom(sal_Bool bIsPhantomP)
{
    if (! (Flags() & FLG_VISIBLE))
        bIsPhantom = bIsPhantomP;

    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->SetPhantom(bIsPhantom);
}


void SmNode::SetColor(const Color& rColor)
{
    if (! (Flags() & FLG_COLOR))
        GetFont().SetColor(rColor);

    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->SetColor(rColor);
}


void SmNode::SetAttribut(sal_uInt16 nAttrib)
{
    if (
        (nAttrib == ATTR_BOLD && !(Flags() & FLG_BOLD)) ||
        (nAttrib == ATTR_ITALIC && !(Flags() & FLG_ITALIC))
       )
    {
        nAttributes |= nAttrib;
    }

    SmNode *pNode;
    sal_uInt16 nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->SetAttribut(nAttrib);
}


void SmNode::ClearAttribut(sal_uInt16 nAttrib)
{
    if (
        (nAttrib == ATTR_BOLD && !(Flags() & FLG_BOLD)) ||
        (nAttrib == ATTR_ITALIC && !(Flags() & FLG_ITALIC))
       )
    {
        nAttributes &= ~nAttrib;
    }

    SmNode *pNode;
    sal_uInt16 nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->ClearAttribut(nAttrib);
}


void SmNode::SetFont(const SmFace &rFace)
{
    if (!(Flags() & FLG_FONT))
        GetFont() = rFace;

    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->SetFont(rFace);
}


void SmNode::SetFontSize(const Fraction &rSize, sal_uInt16 nType)
    //! 'rSize' is in units of pts
{
    Size  aFntSize;

    if (!(Flags() & FLG_SIZE))
    {
        Fraction  aVal (SmPtsTo100th_mm(rSize.GetNumerator()),
                        rSize.GetDenominator());
        //long    nHeight = ::rtl::math::round(aVal);
        long      nHeight = (long)aVal;

        aFntSize = GetFont().GetSize();
        aFntSize.Width() = 0;
        switch(nType)
        {
            case FNTSIZ_ABSOLUT:
                aFntSize.Height() = nHeight;
                break;

            case FNTSIZ_PLUS:
                aFntSize.Height() += nHeight;
                break;

            case FNTSIZ_MINUS:
                aFntSize.Height() -= nHeight;
                break;

            case FNTSIZ_MULTIPLY:
                aFntSize.Height()   = (long) (Fraction(aFntSize.Height()) * rSize);
                break;

            case FNTSIZ_DIVIDE:
                if (rSize != Fraction(0L))
                    aFntSize.Height()   = (long) (Fraction(aFntSize.Height()) / rSize);
                break;
            default:
                break;
        }

        // check the requested size against maximum value
        static int __READONLY_DATA  nMaxVal = SmPtsTo100th_mm(128);
        if (aFntSize.Height() > nMaxVal)
            aFntSize.Height() = nMaxVal;

        GetFont().SetSize(aFntSize);
    }

    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->SetFontSize(rSize, nType);
}


void SmNode::SetSize(const Fraction &rSize)
{
    GetFont() *= rSize;

    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->SetSize(rSize);
}


void SmNode::SetRectHorAlign(RectHorAlign eHorAlign, sal_Bool bApplyToSubTree )
{
    if (!(Flags() & FLG_HORALIGN))
        eRectHorAlign = eHorAlign;

    if (bApplyToSubTree)
    {
        SmNode *pNode;
        sal_uInt16  nSize = GetNumSubNodes();
        for (sal_uInt16 i = 0; i < nSize; i++)
            if (NULL != (pNode = GetSubNode(i)))
                pNode->SetRectHorAlign(eHorAlign);
    }
}


void SmNode::PrepareAttributes()
{
    GetFont().SetWeight((Attributes() & ATTR_BOLD)   ? WEIGHT_BOLD   : WEIGHT_NORMAL);
    GetFont().SetItalic((Attributes() & ATTR_ITALIC) ? ITALIC_NORMAL : ITALIC_NONE);
}


void SmNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
#if OSL_DEBUG_LEVEL > 1
    bIsDebug    = sal_True;
#else
    bIsDebug    = sal_False;
#endif
    bIsPhantom  = sal_False;
    nFlags      = 0;
    nAttributes = 0;

    switch (rFormat.GetHorAlign())
    {   case AlignLeft:     eRectHorAlign = RHA_LEFT;   break;
        case AlignCenter:   eRectHorAlign = RHA_CENTER; break;
        case AlignRight:    eRectHorAlign = RHA_RIGHT;  break;
    }

    GetFont() = rFormat.GetFont(FNT_MATH);
    //GetFont().SetCharSet(RTL_TEXTENCODING_SYMBOL);
    DBG_ASSERT( GetFont().GetCharSet() == RTL_TEXTENCODING_UNICODE,
            "unexpected CharSet" );
    GetFont().SetWeight(WEIGHT_NORMAL);
    GetFont().SetItalic(ITALIC_NONE);

    SmNode *pNode;
    sal_uInt16      nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->Prepare(rFormat, rDocShell);
}


#if OSL_DEBUG_LEVEL > 1
void  SmNode::ToggleDebug() const
    // toggle 'bIsDebug' in current subtree
{
    SmNode *pThis = (SmNode *) this;

    pThis->bIsDebug = bIsDebug ? sal_False : sal_True;

    SmNode *pNode;
    sal_uInt16      nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = pThis->GetSubNode(i)))
            pNode->ToggleDebug();
}
#endif


void SmNode::Move(const Point& rPosition)
{
    if (rPosition.X() == 0  &&  rPosition.Y() == 0)
        return;

    SmRect::Move(rPosition);

    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->Move(rPosition);
}


void SmNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->Arrange(rDev, rFormat);
}

void SmNode::CreateTextFromNode(String &rText)
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    if (nSize > 1)
        rText.Append('{');
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->CreateTextFromNode(rText);
    if (nSize > 1)
    {
        rText.EraseTrailingChars();
        APPEND(rText,"} ");
    }
}


void SmNode::AdaptToX(const OutputDevice &/*rDev*/, sal_uLong /*nWidth*/)
{
}


void SmNode::AdaptToY(const OutputDevice &/*rDev*/, sal_uLong /*nHeight*/)
{
}


void SmNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    const SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
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

const SmNode * SmNode::FindTokenAt(sal_uInt16 nRow, sal_uInt16 nCol) const
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
        sal_uInt16  nNumSubNodes = GetNumSubNodes();
        for (sal_uInt16  i = 0;  i < nNumSubNodes;  i++)
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
        sal_uInt16  nNumSubNodes = GetNumSubNodes();
        for (sal_uInt16  i = 0;  i < nNumSubNodes;  i++)
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

void SmNode::GetAccessibleText( String &/*rText*/ ) const
{
    DBG_ERROR( "SmNode: GetAccessibleText not overloaded" );
}

const SmNode * SmNode::FindNodeWithAccessibleIndex(xub_StrLen nAccIdx) const
{
    const SmNode *pResult = 0;

    sal_Int32 nIdx = GetAccessibleIndex();
    String aTxt;
    if (nIdx >= 0)
        GetAccessibleText( aTxt );  // get text if used in following 'if' statement

    if (nIdx >= 0
        &&  nIdx <= nAccIdx  &&  nAccIdx < nIdx + aTxt.Len())
        pResult = this;
    else
    {
        sal_uInt16  nNumSubNodes = GetNumSubNodes();
        for (sal_uInt16  i = 0;  i < nNumSubNodes;  i++)
        {
            const SmNode *pNode = GetSubNode(i);
            if (!pNode)
                continue;

            pResult = pNode->FindNodeWithAccessibleIndex(nAccIdx);
            if (pResult)
                return pResult;
        }
    }

    return pResult;
}


long SmNode::GetFormulaBaseline() const
{
    DBG_ASSERT( 0, "This dummy implementation should not have been called." );
    return 0;
}

///////////////////////////////////////////////////////////////////////////

SmStructureNode::SmStructureNode( const SmStructureNode &rNode ) :
    SmNode( rNode.GetType(), rNode.GetToken() )
{
    sal_uLong i;
    for (i = 0;  i < aSubNodes.size();  i++)
        delete aSubNodes[i];
    aSubNodes.resize(0);

    sal_uLong nSize = rNode.aSubNodes.size();
    aSubNodes.resize( nSize );
    for (i = 0;  i < nSize;  ++i)
    {
        SmNode *pNode = rNode.aSubNodes[i];
        aSubNodes[i] = pNode ? new SmNode( *pNode ) : 0;
    }
}


SmStructureNode::~SmStructureNode()
{
    SmNode *pNode;

    for (sal_uInt16 i = 0;  i < GetNumSubNodes();  i++)
        if (NULL != (pNode = GetSubNode(i)))
            delete pNode;
}


SmStructureNode & SmStructureNode::operator = ( const SmStructureNode &rNode )
{
    SmNode::operator = ( rNode );

    sal_uLong i;
    for (i = 0;  i < aSubNodes.size();  i++)
        delete aSubNodes[i];
    aSubNodes.resize(0);

    sal_uLong nSize = rNode.aSubNodes.size();
    aSubNodes.resize( nSize );
    for (i = 0;  i < nSize;  ++i)
    {
        SmNode *pNode = rNode.aSubNodes[i];
        aSubNodes[i] = pNode ? new SmNode( *pNode ) : 0;
    }

    return *this;
}


void SmStructureNode::SetSubNodes(SmNode *pFirst, SmNode *pSecond, SmNode *pThird)
{
    size_t nSize = pThird ? 3 : (pSecond ? 2 : (pFirst ? 1 : 0));
    aSubNodes.resize( nSize );
    if (pFirst)
        aSubNodes[0] = pFirst;
    if (pSecond)
        aSubNodes[1] = pSecond;
    if (pThird)
        aSubNodes[2] = pThird;
}


void SmStructureNode::SetSubNodes(const SmNodeArray &rNodeArray)
{
    aSubNodes = rNodeArray;
}


sal_Bool SmStructureNode::IsVisible() const
{
    return sal_False;
}


sal_uInt16 SmStructureNode::GetNumSubNodes() const
{
    return (sal_uInt16) aSubNodes.size();
}


SmNode * SmStructureNode::GetSubNode(sal_uInt16 nIndex)
{
    return aSubNodes[nIndex];
}


void SmStructureNode::GetAccessibleText( String &rText ) const
{
    sal_uInt16 nNodes = GetNumSubNodes();
    for (sal_uInt16 i = 0;  i < nNodes;  ++i)
    {
        const SmNode *pNode = ((SmStructureNode *) this)->GetSubNode(i);
        if (pNode)
        {
            if (pNode->IsVisible())
                ((SmStructureNode *) pNode)->nAccIndex = rText.Len();
            pNode->GetAccessibleText( rText );
//            if (rText.Len()  &&  ' ' != rText.GetChar( rText.Len() - 1 ))
//                rText += String::CreateFromAscii( " " );
        }
    }
}

///////////////////////////////////////////////////////////////////////////


sal_Bool SmVisibleNode::IsVisible() const
{
    return sal_True;
}


sal_uInt16 SmVisibleNode::GetNumSubNodes() const
{
    return 0;
}


SmNode * SmVisibleNode::GetSubNode(sal_uInt16 /*nIndex*/)
{
    return NULL;
}


///////////////////////////////////////////////////////////////////////////

void SmGraphicNode::GetAccessibleText( String &rText ) const
{
    rText += GetToken().aText;
}

///////////////////////////////////////////////////////////////////////////


void SmExpressionNode::CreateTextFromNode(String &rText)
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    if (nSize > 1)
        rText.Append('{');
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
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

    SmNode *pNode;
    sal_uInt16  nSize   = GetNumSubNodes();

    // make distance depend on font size
    long  nDist = +(rFormat.GetDistance(DIS_VERTICAL)
                    * GetFont().GetSize().Height()) / 100L;

    if (nSize < 1)
        return;

    // arrange subnodes and get maximum width of them
    long  nMaxWidth = 0,
          nTmp;
    sal_uInt16 i;
    for (i = 0; i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
        {   pNode->Arrange(rDev, rFormat);
            if ((nTmp = pNode->GetItalicWidth()) > nMaxWidth)
                nMaxWidth = nTmp;
        }

    Point  aPos;
    SmRect::operator = (SmRect(nMaxWidth, 1));
    for (i = 0;  i < nSize;  i++)
    {   if (NULL != (pNode = GetSubNode(i)))
        {   const SmRect &rNodeRect = pNode->GetRect();
            const SmNode *pCoNode   = pNode->GetLeftMost();
            //SmTokenType   eType    = pCoNode->GetToken().eType;
            RectHorAlign  eHorAlign = pCoNode->GetRectHorAlign();

            aPos = rNodeRect.AlignTo(*this, RP_BOTTOM,
                        eHorAlign, RVA_BASELINE);
            if (i)
                aPos.Y() += nDist;
            pNode->MoveTo(aPos);
            ExtendBy(rNodeRect, nSize > 1 ? RCP_NONE : RCP_ARG);
        }
    }
    // --> 4.7.2010 #i972#
    if (HasBaseline())
        nFormulaBaseline = GetBaseline();
    else
    {
        SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
        aTmpDev.SetFont(GetFont());

        SmRect aRect = (SmRect(aTmpDev, &rFormat, C2S("a"),
                               GetFont().GetBorderWidth()));
        nFormulaBaseline = GetAlignM();
        // move from middle position by constant - distance
        // between middle and baseline for single letter
        nFormulaBaseline += aRect.GetBaseline() - aRect.GetAlignM();
    }
    // <--
}


SmNode * SmTableNode::GetLeftMost()
{
    return this;
}


long SmTableNode::GetFormulaBaseline() const
{
    return nFormulaBaseline;
}


/**************************************************************************/


void SmLineNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    //! wir verwenden hier den 'FNT_VARIABLE' Font, da er vom Ascent und Descent
    //! ia besser zum Rest der Formel passt als der 'FNT_MATH' Font.
    GetFont() = rFormat.GetFont(FNT_VARIABLE);
    Flags() |= FLG_FONT;
}


/**************************************************************************/


void SmLineNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
    // arranges all subnodes in one row with some extra space between
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    sal_uInt16 i;
    for (i = 0; i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->Arrange(rDev, rFormat);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    if (nSize < 1)
    {
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
        return;
    }

    // make distance depend on font size
    long nDist = (rFormat.GetDistance(DIS_HORIZONTAL) * GetFont().GetSize().Height()) / 100L;
    if (!IsUseExtraSpaces())
        nDist = 0;

    Point   aPos;
    // copy the first node into LineNode and extend by the others
    if (NULL != (pNode = GetSubNode(0)))
        SmRect::operator = (pNode->GetRect());

    for (i = 1;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
        {
            aPos = pNode->AlignTo(*this, RP_RIGHT, RHA_CENTER, RVA_BASELINE);

            // add horizontal space to the left for each but the first sub node
            aPos.X() += nDist;

            pNode->MoveTo(aPos);
            ExtendBy( *pNode, RCP_XOR );
        }
}


/**************************************************************************/


void SmExpressionNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
    // as 'SmLineNode::Arrange' but keeps alignment of leftmost subnode
{
    SmLineNode::Arrange(rDev, rFormat);

    //  copy alignment of leftmost subnode if any
    SmNode *pNode = GetLeftMost();
    if (pNode)
        SetRectHorAlign(pNode->GetRectHorAlign(), sal_False);
}


/**************************************************************************/


void SmUnHorNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    sal_Bool  bIsPostfix = GetToken().eType == TFACT;

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

    // font specialist advised to change the width first
    pRootSym->AdaptToY(rDev, nHeight);
    pRootSym->AdaptToX(rDev, pBody->GetItalicWidth());

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
        ExtendBy(*pExtra, RCP_THIS, (sal_Bool) sal_True);
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

    sal_Bool  bIsTextmode = rFormat.IsTextmode();
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

    // font specialist advised to change the width first
    pLine->AdaptToY(rDev, nThick);
    pLine->AdaptToX(rDev, nWidth + 2 * nExtLen);
    pLine->Arrange(rDev, rFormat);

    // get horizontal alignment for numerator
    const SmNode *pLM       = pNum->GetLeftMost();
    RectHorAlign  eHorAlign = pLM->GetRectHorAlign();

    // move numerator to its position
    Point  aPos = pNum->AlignTo(*pLine, RP_TOP, eHorAlign, RVA_BASELINE);
    aPos.Y() -= nNumDist;
    pNum->MoveTo(aPos);

    // get horizontal alignment for denominator
    pLM       = pDenom->GetLeftMost();
    eHorAlign = pLM->GetRectHorAlign();

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
    //      *pLine  = GetSubNode(1),
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
    // zurueck
{
    return rHeading1.X() * rHeading2.Y() - rHeading1.Y() * rHeading2.X();
}


sal_Bool IsPointInLine(const Point &rPoint1,
                   const Point &rPoint2, const Point &rHeading2)
    // ergibt sal_True genau dann, wenn der Punkt 'rPoint1' zu der Gerade gehoert die
    // durch den Punkt 'rPoint2' geht und den Richtungsvektor 'rHeading2' hat
{
    DBG_ASSERT(rHeading2 != Point(), "Sm : 0 vector");

    sal_Bool bRes = sal_False;
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


sal_uInt16 GetLineIntersectionPoint(Point &rResult,
                                const Point& rPoint1, const Point &rHeading1,
                                const Point& rPoint2, const Point &rHeading2)
{
    DBG_ASSERT(rHeading1 != Point(), "Sm : 0 vector");
    DBG_ASSERT(rHeading2 != Point(), "Sm : 0 vector");

    sal_uInt16 nRes = 1;
    const double eps = 5.0 * DBL_EPSILON;

    // sind die Richtumgsvektoren linear abhaengig ?
    double  fDet = Det(rHeading1, rHeading2);
    if (fabs(fDet) < eps)
    {
        nRes    = IsPointInLine(rPoint1, rPoint2, rHeading2) ? USHRT_MAX : 0;
        rResult = nRes ? rPoint1 : Point();
    }
    else
    {
        // hier achten wir nicht auf Rechengenauigkeit
        // (das wuerde aufwendiger und lohnt sich hier kaum)
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
    bAscending = sal_False;
    SetNumSubNodes(3);
}


void SmBinDiagonalNode::GetOperPosSize(Point &rPos, Size &rSize,
                        const Point &rDiagPoint, double fAngleDeg) const
    // gibt die Position und Groesse fuer den Diagonalstrich zurueck.
    // Vor.: das SmRect des Nodes gibt die Begrenzung vor(!), muss also selbst
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

    long  nLeft, nRight, nTop, nBottom;     // Raender des Rechtecks fuer die
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
            // es muss einen Schnittpunkt mit dem rechten Rand geben!
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
            // es muss einen Schnittpunkt mit dem linken Rand geben!
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
            // es muss einen Schnittpunkt mit dem linken Rand geben!
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
            // es muss einen Schnittpunkt mit dem rechten Rand geben!
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
    //! die beiden Argumente muessen in den Subnodes vor dem Operator kommen,
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
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
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
    long nTmpBaseline = IsAscending() ? (pLeft->GetBottom() + pRight->GetTop()) / 2
                        : (pLeft->GetTop() + pRight->GetBottom()) / 2;
    Point  aLogCenter ((pLeft->GetItalicRight() + pRight->GetItalicLeft()) / 2,
                       nTmpBaseline);

    SmRect::operator = (*pLeft);
    ExtendBy(*pRight, RCP_NONE);


    // Position und Groesse des Diagonalstrich ermitteln
    Size  aTmpSize;
    GetOperPosSize(aPos, aTmpSize, aLogCenter, IsAscending() ? 60.0 : -60.0);

    // font specialist advised to change the width first
    pOper->AdaptToY(aTmpDev, aTmpSize.Height());
    pOper->AdaptToX(aTmpDev, aTmpSize.Width());
    // und diese wirksam machen
    pOper->Arrange(aTmpDev, rFormat);

    pOper->MoveTo(aPos);

    ExtendBy(*pOper, RCP_NONE, nTmpBaseline);
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
                default:
                    break;
            }

        // prevent sub-/supscripts from diminishing in size
        // (as would be in "a_{1_{2_{3_4}}}")
        if (GetFont().GetSize().Height() > rFormat.GetBaseSize().Height() / 3)
        {
            sal_uInt16 nIndex = (eSubSup == CSUB  ||  eSubSup == CSUP) ?
                                    SIZ_LIMITS : SIZ_INDEX;
            Fraction  aFraction ( rFormat.GetRelSize(nIndex), 100 );
            pSubSup->SetSize(aFraction);
        }

        pSubSup->Arrange(rDev, rFormat);

        sal_Bool  bIsTextmode = rFormat.IsTextmode();
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
                DBG_ASSERT(sal_False, "Sm: unbekannter Fall");
                break;
        }

        pSubSup->MoveTo(aPos);
        ExtendBy(*pSubSup, RCP_THIS, (sal_Bool) sal_True);

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

    if (NULL != (pNode = GetSubNode(LSUB+1)))
    {
        APPEND(rText,"lsub ");
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(LSUP+1)))
    {
        APPEND(rText,"lsup ");
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(CSUB+1)))
    {
        APPEND(rText,"csub ");
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(CSUP+1)))
    {
        APPEND(rText,"csup ");
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(RSUB+1)))
    {
        rText.EraseTrailingChars();
        rText.Append('_');
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(RSUP+1)))
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
    {
        String aStr;
        GetSubNode(0)->CreateTextFromNode(aStr);
        aStr.EraseLeadingAndTrailingChars();
        aStr.EraseLeadingChars('\\');
        if (aStr.Len())
        {
            if (aStr.EqualsAscii("divides"))
                APPEND(rText,"lline");
            else if (aStr.EqualsAscii("parallel"))
                APPEND(rText,"ldline");
            else if (aStr.EqualsAscii("<"))
                APPEND(rText,"langle");
            else
                rText.Append(aStr);
            rText.Append(' ');
        }
        else
            APPEND(rText,"none ");
    }
    GetSubNode(1)->CreateTextFromNode(rText);
    if (GetScaleMode() == SCALE_HEIGHT)
        APPEND(rText,"right ");
    {
        String aStr;
        GetSubNode(2)->CreateTextFromNode(aStr);
        aStr.EraseLeadingAndTrailingChars();
        aStr.EraseLeadingChars('\\');
        if (aStr.Len())
        {
            if (aStr.EqualsAscii("divides"))
                APPEND(rText,"rline");
            else if (aStr.EqualsAscii("parallel"))
                APPEND(rText,"rdline");
            else if (aStr.EqualsAscii(">"))
                APPEND(rText,"rangle");
            else
                rText.Append(aStr);
            rText.Append(' ');
        }
        else
            APPEND(rText,"none ");
    }
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

    sal_Bool  bIsScaleNormal = rFormat.IsScaleNormalBrackets(),
          bScale         = pBody->GetHeight() > 0  &&
                           (GetScaleMode() == SCALE_HEIGHT  ||  bIsScaleNormal),
          bIsABS         = GetToken().eType == TABS;

    long  nFaceHeight = GetFont().GetSize().Height();

    // Uebergroesse in % ermitteln
    sal_uInt16  nPerc = 0;
    if (!bIsABS && bScale)
    {   // im Fall von Klammern mit Uebergroesse...
        sal_uInt16 nIndex = GetScaleMode() == SCALE_HEIGHT ?
                            DIS_BRACKETSIZE : DIS_NORMALBRACKETSIZE;
        nPerc = rFormat.GetDistance(nIndex);
    }

    // ermitteln der Hoehe fuer die Klammern
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

    // sofern erwuenscht skalieren der Klammern auf die gewuenschte Groesse
    if (bScale)
    {
        Size  aTmpSize (pLeft->GetFont().GetSize());
        DBG_ASSERT(pRight->GetFont().GetSize() == aTmpSize,
                    "Sm : unterschiedliche Fontgroessen");
        aTmpSize.Width() = Min((long) nBraceHeight * 60L / 100L,
                            rFormat.GetBaseSize().Height() * 3L / 2L);
        // correction factor since change from StarMath to OpenSymbol font
        // because of the different font width in the FontMetric
        aTmpSize.Width() *= 182;
        aTmpSize.Width() /= 267;

        xub_Unicode cChar = pLeft->GetToken().cMathChar;
        if (cChar != MS_LINE  &&  cChar != MS_DLINE)
            pLeft ->GetFont().SetSize(aTmpSize);

        cChar = pRight->GetToken().cMathChar;
        if (cChar != MS_LINE  &&  cChar != MS_DLINE)
            pRight->GetFont().SetSize(aTmpSize);

        pLeft ->AdaptToY(rDev, nBraceHeight);
        pRight->AdaptToY(rDev, nBraceHeight);
    }

    pLeft ->Arrange(rDev, rFormat);
    pRight->Arrange(rDev, rFormat);

    // damit auch "\(a\) - (a) - left ( a right )" vernuenftig aussieht
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
    sal_uInt16  nNumSubNodes = GetNumSubNodes();
    if (nNumSubNodes == 0)
        return;

    // arrange arguments
    sal_uInt16 i;
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
    sal_Bool bScale  = GetScaleMode() == SCALE_HEIGHT  ||  rFormat.IsScaleNormalBrackets();
    long nHeight = bScale ? aRefRect.GetHeight() : GetFont().GetSize().Height();
    sal_uInt16 nIndex  = GetScaleMode() == SCALE_HEIGHT ?
                        DIS_BRACKETSIZE : DIS_NORMALBRACKETSIZE;
    sal_uInt16 nPerc   = rFormat.GetDistance(nIndex);
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
        sal_Bool          bIsSeparator = i % 2 != 0;
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
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

    SmTokenType  eTmpType = GetToken().eType;
    if (eTmpType == TLIM  ||  eTmpType == TLIMINF  ||  eTmpType == TLIMSUP)
        return nHeight;

    if (!rFormat.IsTextmode())
    {
        // set minimum size ()
        nHeight += (nHeight * 20L) / 100L;

        nHeight += nHeight
                   * rFormat.GetDistance(DIS_OPERATORSIZE) / 100L;
        nHeight = nHeight * 686L / 845L;
    }

    // correct user-defined symbols to match height of sum from used font
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

    RectHorAlign  eHorAlign = RHA_CENTER;
    switch (GetToken().eType)
    {
        case TALIGNL:   eHorAlign = RHA_LEFT;   break;
        case TALIGNC:   eHorAlign = RHA_CENTER; break;
        case TALIGNR:   eHorAlign = RHA_RIGHT;  break;
        default:
            break;
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
    ExtendBy(*pAttr, RCP_THIS, (sal_Bool) sal_True);
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
                rText += String( ::rtl::math::doubleToUString(
                            static_cast<double>(aFontSize),
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', sal_True));
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


void SmFontNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    //! prepare subnodes first
    SmNode::Prepare(rFormat, rDocShell);

    int  nFnt = -1;
    switch (GetToken().eType)
    {
        case TFIXED:    nFnt = FNT_FIXED;   break;
        case TSANS:     nFnt = FNT_SANS;    break;
        case TSERIF:    nFnt = FNT_SERIF;   break;
        default:
            break;
    }
    if (nFnt != -1)
    {   GetFont() = rFormat.GetFont( sal::static_int_cast< sal_uInt16 >(nFnt) );
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

        case TPHANTOM : SetPhantom(sal_True);               break;
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
            DBG_ASSERT(sal_False, "Sm: unbekannter Fall");
    }

    pNode->Arrange(rDev, rFormat);

    SmRect::operator = (pNode->GetRect());
}


void SmFontNode::SetSizeParameter(const Fraction& rValue, sal_uInt16 Type)
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


void SmPolyLineNode::AdaptToX(const OutputDevice &/*rDev*/, sal_uLong nNewWidth)
{
    aToSize.Width() = nNewWidth;
}


void SmPolyLineNode::AdaptToY(const OutputDevice &/*rDev*/, sal_uLong nNewHeight)
{
    GetFont().FreezeBorderWidth();
    aToSize.Height() = nNewHeight;
}


void SmPolyLineNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    //! some routines being called extract some info from the OutputDevice's
    //! font (eg the space to be used for borders OR the font name(!!)).
    //! Thus the font should reflect the needs and has to be set!
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_False);
    aTmpDev.SetLineColor( GetFont().GetColor() );

    rDev.DrawPolyLine(aPoly, aInfo);

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


/**************************************************************************/

void SmRootSymbolNode::AdaptToX(const OutputDevice &/*rDev*/, sal_uLong nWidth)
{
    nBodyWidth = nWidth;
}


void SmRootSymbolNode::AdaptToY(const OutputDevice &rDev, sal_uLong nHeight)
{
    // etwas extra Laenge damit der horizontale Balken spaeter ueber dem
    // Argument positioniert ist
    SmMathSymbolNode::AdaptToY(rDev, nHeight + nHeight / 10L);
}


void SmRootSymbolNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    // draw root-sign itself
    SmMathSymbolNode::Draw(rDev, rPosition);

    SmTmpDevice  aTmpDev( (OutputDevice &) rDev, sal_True );
    aTmpDev.SetFillColor(GetFont().GetColor());
    rDev.SetLineColor();
    aTmpDev.SetFont( GetFont() );

    // since the width is always unscaled it corresponds ot the _original_
    // _unscaled_ font height to be used, we use that to calculate the
    // bar height. Thus it is independent of the arguments height.
    // ( see display of sqrt QQQ versus sqrt stack{Q#Q#Q#Q} )
    long nBarHeight = GetWidth() * 7L / 100L;
    long nBarWidth = nBodyWidth + GetBorderWidth();
    Point aBarOffset( GetWidth(), +GetBorderWidth() );
    Point aBarPos( rPosition + aBarOffset );

    Rectangle  aBar(aBarPos, Size( nBarWidth, nBarHeight) );
    //! avoid GROWING AND SHRINKING of drawn rectangle when constantly
    //! increasing zoomfactor.
    //  This is done by shifting it's output-position to a point that
    //  corresponds exactly to a pixel on the output device.
    Point  aDrawPos( rDev.PixelToLogic(rDev.LogicToPixel(aBar.TopLeft())) );
    //aDrawPos.X() = aBar.Left();     //! don't change X position
    aBar.SetPos( aDrawPos );

    rDev.DrawRect( aBar );

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


/**************************************************************************/


void SmRectangleNode::AdaptToX(const OutputDevice &/*rDev*/, sal_uLong nWidth)
{
    aToSize.Width() = nWidth;
}


void SmRectangleNode::AdaptToY(const OutputDevice &/*rDev*/, sal_uLong nHeight)
{
    GetFont().FreezeBorderWidth();
    aToSize.Height() = nHeight;
}


void SmRectangleNode::Arrange(const OutputDevice &rDev, const SmFormat &/*rFormat*/)
{
    long  nFontHeight = GetFont().GetSize().Height();
    long  nWidth  = aToSize.Width(),
          nHeight = aToSize.Height();
    if (nHeight == 0)
        nHeight = nFontHeight / 30;
    if (nWidth == 0)
        nWidth  = nFontHeight / 3;

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    // add some borderspace
    sal_uLong  nTmpBorderWidth = GetFont().GetBorderWidth();
    //nWidth  += nTmpBorderWidth;
    nHeight += 2 * nTmpBorderWidth;

    //! use this method in order to have 'SmRect::HasAlignInfo() == sal_True'
    //! and thus having the attribut-fences updated in 'SmRect::ExtendBy'
    SmRect::operator = (SmRect(nWidth, nHeight));
}


void SmRectangleNode::Draw(OutputDevice &rDev, const Point &rPosition) const
{
    if (IsPhantom())
        return;

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_False);
    aTmpDev.SetFillColor(GetFont().GetColor());
    rDev.SetLineColor();
    aTmpDev.SetFont(GetFont());

    sal_uLong  nTmpBorderWidth = GetFont().GetBorderWidth();

    // get rectangle and remove borderspace
    Rectangle  aTmp (AsRectangle() + rPosition - GetTopLeft());
    aTmp.Left()   += nTmpBorderWidth;
    aTmp.Right()  -= nTmpBorderWidth;
    aTmp.Top()    += nTmpBorderWidth;
    aTmp.Bottom() -= nTmpBorderWidth;

    DBG_ASSERT(aTmp.GetHeight() > 0  &&  aTmp.GetWidth() > 0,
               "Sm: leeres Rechteck");

    //! avoid GROWING AND SHRINKING of drawn rectangle when constantly
    //! increasing zoomfactor.
    //  This is done by shifting it's output-position to a point that
    //  corresponds exactly to a pixel on the output device.
    Point  aPos (rDev.PixelToLogic(rDev.LogicToPixel(aTmp.TopLeft())));
    aTmp.SetPos(aPos);

    rDev.DrawRect(aTmp);

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}


/**************************************************************************/


SmTextNode::SmTextNode( SmNodeType eNodeType, const SmToken &rNodeToken, sal_uInt16 nFontDescP ) :
    SmVisibleNode(eNodeType, rNodeToken)
{
    nFontDesc = nFontDescP;
}


SmTextNode::SmTextNode( const SmToken &rNodeToken, sal_uInt16 nFontDescP ) :
    SmVisibleNode(NTEXT, rNodeToken)
{
    nFontDesc = nFontDescP;
}


void SmTextNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    // default setting for horizontal alignment of nodes with TTEXT
    // content is as alignl (cannot be done in Arrange since it would
    // override the settings made by an SmAlignNode before)
    if (TTEXT == GetToken().eType)
        SetRectHorAlign( RHA_LEFT );

    aText = GetToken().aText;
    GetFont() = rFormat.GetFont(GetFontDesc());

    if (IsItalic( GetFont() ))
        Attributes() |= ATTR_ITALIC;
    if (IsBold( GetFont() ))
        Attributes() |= ATTR_BOLD;

    // special handling for ':' where it is a token on it's own and is likely
    // to be used for mathematical notations. (E.g. a:b = 2:3)
    // In that case it should not be displayed in italic.
    if (GetToken().aText.Len() == 1 && GetToken().aText.GetChar(0) == ':')
        Attributes() &= ~ATTR_ITALIC;
};


void SmTextNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    sal_uInt16  nSizeDesc = GetFontDesc() == FNT_FUNCTION ?
                            SIZ_FUNCTION : SIZ_TEXT;
    GetFont() *= Fraction (rFormat.GetRelSize(nSizeDesc), 100);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, aText, GetFont().GetBorderWidth()));
}

void SmTextNode::CreateTextFromNode(String &rText)
{
    sal_Bool bQuoted=sal_False;
    if (GetToken().eType == TTEXT)
    {
        rText.Append('\"');
        bQuoted=sal_True;
    }
    else
    {
        SmParser aParseTest;
        SmNode *pTable = aParseTest.Parse(GetToken().aText);
        bQuoted=sal_True;
        if ( (pTable->GetType() == NTABLE) && (pTable->GetNumSubNodes() == 1) )
        {
            SmNode *pResult = pTable->GetSubNode(0);
            if ( (pResult->GetType() == NLINE) &&
                (pResult->GetNumSubNodes() == 1) )
            {
                pResult = pResult->GetSubNode(0);
                if ( (pResult->GetType() == NEXPRESSION) &&
                    (pResult->GetNumSubNodes() == 1) )
                {
                    pResult = pResult->GetSubNode(0);
                    if (pResult->GetType() == NTEXT)
                        bQuoted=sal_False;
                }
            }
        }
        delete pTable;

        if ((GetToken().eType == TIDENT) && (GetFontDesc() == FNT_FUNCTION))
        {
            //Search for existing functions and remove extraenous keyword
            APPEND(rText,"func ");
        }
        else if (bQuoted)
            APPEND(rText,"italic ");

        if (bQuoted)
            rText.Append('\"');

    }

    rText.Append(GetToken().aText);

    if (bQuoted)
        rText.Append('\"');
    rText.Append(' ');
}

void SmTextNode::Draw(OutputDevice &rDev, const Point& rPosition) const
{
    if (IsPhantom()  ||  aText.Len() == 0  ||  aText.GetChar(0) == xub_Unicode('\0'))
        return;

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_False);
    aTmpDev.SetFont(GetFont());

    Point  aPos (rPosition);
    aPos.Y() += GetBaselineOffset();
    // auf Pixelkoordinaten runden
    aPos = rDev.PixelToLogic( rDev.LogicToPixel(aPos) );

#if OSL_DEBUG_LEVEL > 1
    sal_Int32 nPos = 0;
    sal_UCS4 cChar = OUString( aText ).iterateCodePoints( &nPos );
    (void) cChar;
#endif

    rDev.DrawStretchText(aPos, GetWidth(), aText);

#ifdef SM_RECT_DEBUG
    if (!IsDebug())
        return;

    int  nRFlags = SM_RECT_CORE | SM_RECT_ITALIC | SM_RECT_LINES | SM_RECT_MID;
    SmRect::Draw(rDev, rPosition, nRFlags);
#endif
}

void SmTextNode::GetAccessibleText( String &rText ) const
{
    rText += aText;
}

/**************************************************************************/

void SmMatrixNode::CreateTextFromNode(String &rText)
{
    APPEND(rText,"matrix {");
    for (sal_uInt16 i = 0;  i < nNumRows; i++)
    {
        for (sal_uInt16 j = 0;  j < nNumCols; j++)
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
    sal_uInt16  i, j;

    // initialize array that is to hold the maximum widhts of all
    // elements (subnodes) in that column.
    long *pColWidth = new long[nNumCols];
    for (j = 0;  j  < nNumCols;  j++)
        pColWidth[j] = 0;

    // arrange subnodes and calculate the aboves arrays contents
    sal_uInt16 nNodes = GetNumSubNodes();
    for (i = 0;  i < nNodes;  i++)
    {
        sal_uInt16 nIdx = nNodes - 1 - i;
        if (NULL != (pNode = GetSubNode(nIdx)))
        {
            pNode->Arrange(rDev, rFormat);
            int  nCol = nIdx % nNumCols;
            pColWidth[nCol] = Max(pColWidth[nCol], pNode->GetItalicWidth());
        }
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
        {   SmNode *pTmpNode = GetSubNode(i * nNumCols + j);
            DBG_ASSERT(pTmpNode, "Sm: NULL pointer");

            const SmRect &rNodeRect = pTmpNode->GetRect();

            // align all baselines in that row if possible
            aPos = rNodeRect.AlignTo(aLineRect, RP_RIGHT, RHA_CENTER, RVA_BASELINE);
            aPos.X() += nHorDist;

            // get horizontal alignment
            const SmNode *pCoNode   = pTmpNode->GetLeftMost();
            RectHorAlign  eHorAlign = pCoNode->GetRectHorAlign();

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

            pTmpNode->MoveTo(aPos);
            aLineRect.ExtendBy(rNodeRect, RCP_XOR);
        }

        aPos = aLineRect.AlignTo(*this, RP_BOTTOM, RHA_CENTER, RVA_BASELINE);
        aPos.Y() += nVerDist;

        // move 'aLineRect' and rectangles in that line to final position
        aDelta.X() = 0;     // since horizontal alignment is already done
        aDelta.Y() = aPos.Y() - aLineRect.GetTop();
        aLineRect.Move(aDelta);
        for (j = 0;  j < nNumCols;  j++)
            if (NULL != (pNode = GetSubNode(i * nNumCols + j)))
                pNode->Move(aDelta);

        ExtendBy(aLineRect, RCP_NONE);
    }

    delete [] pColLeft;
    delete [] pColWidth;
}


void SmMatrixNode::SetRowCol(sal_uInt16 nMatrixRows, sal_uInt16 nMatrixCols)
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

void SmMathSymbolNode::AdaptToX(const OutputDevice &rDev, sal_uLong nWidth)
{
    // Since there is no function to do this, we try to approximate it:
    Size  aFntSize (GetFont().GetSize());

    //! however the result is a bit better with 'nWidth' as initial font width
    aFntSize.Width() = nWidth;
    GetFont().SetSize(aFntSize);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    // get denominator of error factor for width
    long nTmpBorderWidth = GetFont().GetBorderWidth();
    long nDenom = SmRect(aTmpDev, NULL, GetText(), nTmpBorderWidth).GetItalicWidth();

    // scale fontwidth with this error factor
    aFntSize.Width() *= nWidth;
    aFntSize.Width() /= nDenom ? nDenom : 1;

    GetFont().SetSize(aFntSize);
}

void SmMathSymbolNode::AdaptToY(const OutputDevice &rDev, sal_uLong nHeight)
{
    GetFont().FreezeBorderWidth();
    Size  aFntSize (GetFont().GetSize());

    // da wir nur die Hoehe skalieren wollen muesen wir hier ggf die Fontweite
    // ermitteln um diese beizubehalten.
    if (aFntSize.Width() == 0)
    {
        OutputDevice &rDevNC = (OutputDevice &) rDev;
        rDevNC.Push(PUSH_FONT | PUSH_MAPMODE);
        rDevNC.SetFont(GetFont());
        aFntSize.Width() = rDev.GetFontMetric().GetSize().Width();
        rDevNC.Pop();
    }
    DBG_ASSERT(aFntSize.Width() != 0, "Sm: ");

    //! however the result is a bit better with 'nHeight' as initial
    //! font height
    aFntSize.Height() = nHeight;
    GetFont().SetSize(aFntSize);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    // get denominator of error factor for height
    long nTmpBorderWidth = GetFont().GetBorderWidth();
    long nDenom = SmRect(aTmpDev, NULL, GetText(), nTmpBorderWidth).GetHeight();

    // scale fontwidth with this error factor
    aFntSize.Height() *= nHeight;
    aFntSize.Height() /= nDenom ? nDenom : 1;

    GetFont().SetSize(aFntSize);
}


void SmMathSymbolNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    GetFont() = rFormat.GetFont(GetFontDesc());
    // use same font size as is used for variables
    GetFont().SetSize( rFormat.GetFont( FNT_VARIABLE ).GetSize() );

    DBG_ASSERT(GetFont().GetCharSet() == RTL_TEXTENCODING_SYMBOL  ||
               GetFont().GetCharSet() == RTL_TEXTENCODING_UNICODE,
        "incorrect charset for character from StarMath/OpenSymbol font");

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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, rText, GetFont().GetBorderWidth()));
}

void SmMathSymbolNode::CreateTextFromNode(String &rText)
{
    String sStr;
    MathType::LookupChar(GetToken().cMathChar, sStr);
    rText.Append(sStr);
}

void SmRectangleNode::CreateTextFromNode(String &rText)
{
    switch (GetToken().eType)
    {
    case TUNDERLINE:
        APPEND(rText,"underline ");
        break;
    case TOVERLINE:
        APPEND(rText,"overline ");
        break;
    case TOVERSTRIKE:
        APPEND(rText,"overstrike ");
        break;
    default:
        break;
    }
}

void SmAttributNode::CreateTextFromNode(String &rText)
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    DBG_ASSERT(nSize == 2, "Node missing members");
    rText.Append('{');
    sal_Unicode nLast=0;
    if (NULL != (pNode = GetSubNode(0)))
    {
        String aStr;
        pNode->CreateTextFromNode(aStr);
        if (aStr.Len() > 1)
            rText.Append(aStr);
        else
        {
            nLast = aStr.GetChar(0);
            switch (nLast)
            {
            case 0xAF:
                APPEND(rText,"overline ");
                break;
            case 0x2d9:
                APPEND(rText,"dot ");
                break;
            case 0x2dc:
                APPEND(rText,"widetilde ");
                break;
            case 0xA8:
                APPEND(rText,"ddot ");
                break;
            case 0xE082:
                break;
            case 0xE09B:
                APPEND(rText,"dddot ");
                break;
            default:
                rText.Append(nLast);
                break;
            }
        }
    }

    if (nSize == 2)
        if (NULL != (pNode = GetSubNode(1)))
            pNode->CreateTextFromNode(rText);

    rText.EraseTrailingChars();

    if (nLast == 0xE082)
        APPEND(rText," overbrace {}");

    APPEND(rText,"} ");
}

/**************************************************************************/

bool lcl_IsFromGreekSymbolSet( const String &rTokenText )
{
    bool bRes = false;

    // valid symbol name needs to have a '%' at pos 0 and at least an additonal char
    if (rTokenText.Len() > 2 && rTokenText.GetBuffer()[0] == (sal_Unicode)'%')
    {
        String aName( rTokenText.Copy(1) );
        SmSym *pSymbol = SM_MOD()->GetSymbolManager().GetSymbolByName( aName );
        if (pSymbol && GetExportSymbolSetName( pSymbol->GetSymbolSetName() ).EqualsAscii( "Greek" ) )
            bRes = true;
    }

    return bRes;
}


SmSpecialNode::SmSpecialNode(SmNodeType eNodeType, const SmToken &rNodeToken, sal_uInt16 _nFontDesc) :
    SmTextNode(eNodeType, rNodeToken, _nFontDesc)
{
    bIsFromGreekSymbolSet = lcl_IsFromGreekSymbolSet( rNodeToken.aText );
}


SmSpecialNode::SmSpecialNode(const SmToken &rNodeToken) :
    SmTextNode(NSPECIAL, rNodeToken, FNT_MATH)  //! default Font nicht immer richtig
{
    bIsFromGreekSymbolSet = lcl_IsFromGreekSymbolSet( rNodeToken.aText );
}


void SmSpecialNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    const SmSym   *pSym;
    SmModule  *pp = SM_MOD();

    String aName( GetToken().aText.Copy(1) );
    if (NULL != (pSym = pp->GetSymbolManager().GetSymbolByName( aName )))
    {
        sal_UCS4 cChar = pSym->GetCharacter();
        String aTmp( OUString( &cChar, 1 ) );
        SetText( aTmp );
        GetFont() = pSym->GetFace();
    }
    else
    {
        SetText( GetToken().aText );
        GetFont() = rFormat.GetFont(FNT_VARIABLE);
    }
    // use same font size as is used for variables
    GetFont().SetSize( rFormat.GetFont( FNT_VARIABLE ).GetSize() );

    //! eigentlich sollten nur WEIGHT_NORMAL und WEIGHT_BOLD vorkommen...
    //! In der sms-Datei gibt es jedoch zB auch 'WEIGHT_ULTRALIGHT'
    //! daher vergleichen wir hier mit  >  statt mit  !=  .
    //! (Langfristig sollte die Notwendigkeit fuer 'PrepareAttribut', und damit
    //! fuer dieses hier, mal entfallen.)
    //
    //! see also SmFontStyles::GetStyleName
    if (IsItalic( GetFont() ))
        SetAttribut(ATTR_ITALIC);
    if (IsBold( GetFont() ))
        SetAttribut(ATTR_BOLD);

    Flags() |= FLG_FONT;

    if (bIsFromGreekSymbolSet)
    {
        DBG_ASSERT( GetText().Len() == 1, "a symbol should only consist of 1 char!" );
        bool bItalic = false;
        sal_Int16 nStyle = rFormat.GetGreekCharStyle();
        DBG_ASSERT( nStyle >= 0 && nStyle <= 2, "unexpected value for GreekCharStyle" );
        if (nStyle == 1)
            bItalic = true;
        else if (nStyle == 2)
        {
            String aTmp( GetText() );
            if (aTmp.Len() > 0)
            {
                const sal_Unicode cUppercaseAlpha = 0x0391;
                const sal_Unicode cUppercaseOmega = 0x03A9;
                sal_Unicode cChar = aTmp.GetBuffer()[0];
                // uppercase letters should be straight and lowercase letters italic
                bItalic = !(cUppercaseAlpha <= cChar && cChar <= cUppercaseOmega);
            }
        }

        if (bItalic)
            Attributes() |= ATTR_ITALIC;
        else
            Attributes() &= ~ATTR_ITALIC;;
    }
};


void SmSpecialNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, GetText(),
                               GetFont().GetBorderWidth()).AsGlyphRect());
}


/**************************************************************************/


void SmPlaceNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    GetFont().SetColor(COL_GRAY);
    Flags() |= FLG_COLOR | FLG_FONT | FLG_ITALIC;
};


void SmPlaceNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, GetText(), GetFont().GetBorderWidth()));
}


/**************************************************************************/


void SmErrorNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    GetFont().SetColor(COL_RED);
    Flags() |= FLG_VISIBLE | FLG_BOLD | FLG_ITALIC
               | FLG_COLOR | FLG_FONT | FLG_SIZE;
}


void SmErrorNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    const XubString &rText = GetText();
    SmRect::operator = (SmRect(aTmpDev, &rFormat, rText, GetFont().GetBorderWidth()));
}


/**************************************************************************/


void SmBlankNode::IncreaseBy(const SmToken &rToken)
{
    switch(rToken.eType)
    {
        case TBLANK:    nNum += 4;  break;
        case TSBLANK:   nNum += 1;  break;
        default:
            break;
    }
}


void SmBlankNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    //! hier muss/sollte es lediglich nicht der StarMath Font sein,
    //! damit fuer das in Arrange verwendete Zeichen ein "normales"
    //! (ungecliptes) Rechteck erzeugt wird.
    GetFont() = rFormat.GetFont(FNT_VARIABLE);

    Flags() |= FLG_FONT | FLG_BOLD | FLG_ITALIC;
}


void SmBlankNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
    aTmpDev.SetFont(GetFont());

    // Abstand von der Fonthoehe abhaengig machen
    // (damit er beim skalieren (zB size *2 {a ~ b}) mitwaechst)
    long  nDist  = GetFont().GetSize().Height() / 10L,
          nSpace = nNum * nDist;

    // ein SmRect mit Baseline und allem drum und dran besorgen
    SmRect::operator = (SmRect(aTmpDev, &rFormat, XubString(xub_Unicode(' ')),
                               GetFont().GetBorderWidth()));

    // und dieses auf die gewuenschte Breite bringen
    SetItalicSpaces(0, 0);
    SetWidth(nSpace);
}



