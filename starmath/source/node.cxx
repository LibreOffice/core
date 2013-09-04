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


#include "node.hxx"
#include "rect.hxx"
#include "symbol.hxx"
#include "smmod.hxx"
#include "document.hxx"
#include "view.hxx"
#include "mathtype.hxx"
#include "visitors.hxx"

#include <comphelper/string.hxx>
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
    SmTmpDevice(OutputDevice &rTheDev, bool bUseMap100th_mm);
    ~SmTmpDevice()  { rOutDev.Pop(); }

    void SetFont(const Font &rNewFont);

    void SetLineColor( const Color& rColor )    { rOutDev.SetLineColor( Impl_GetColor(rColor) ); }
    void SetFillColor( const Color& rColor )    { rOutDev.SetFillColor( Impl_GetColor(rColor) ); }
    void SetTextColor( const Color& rColor )    { rOutDev.SetTextColor( Impl_GetColor(rColor) ); }

    operator OutputDevice & () { return rOutDev; }
};


SmTmpDevice::SmTmpDevice(OutputDevice &rTheDev, bool bUseMap100th_mm) :
    rOutDev(rTheDev)
{
    rOutDev.Push( PUSH_FONT | PUSH_MAPMODE |
                  PUSH_LINECOLOR | PUSH_FILLCOLOR | PUSH_TEXTCOLOR );
    if (bUseMap100th_mm  &&  MAP_100TH_MM != rOutDev.GetMapMode().GetMapUnit())
    {
        OSL_FAIL( "incorrect MapMode?" );
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
    SetSelected(false);
    aParentNode = NULL;
}


SmNode::~SmNode()
{
}


bool SmNode::IsVisible() const
{
    return false;
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


void SmNode::SetPhantom(bool bIsPhantomP)
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
        static int const    nMaxVal = SmPtsTo100th_mm(128);
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


void SmNode::SetRectHorAlign(RectHorAlign eHorAlign, bool bApplyToSubTree )
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
    bIsDebug    = true;
#else
    bIsDebug    = false;
#endif
    bIsPhantom  = false;
    nFlags      = 0;
    nAttributes = 0;

    switch (rFormat.GetHorAlign())
    {   case AlignLeft:     eRectHorAlign = RHA_LEFT;   break;
        case AlignCenter:   eRectHorAlign = RHA_CENTER; break;
        case AlignRight:    eRectHorAlign = RHA_RIGHT;  break;
    }

    GetFont() = rFormat.GetFont(FNT_MATH);
    OSL_ENSURE( GetFont().GetCharSet() == RTL_TEXTENCODING_UNICODE,
            "unexpected CharSet" );
    GetFont().SetWeight(WEIGHT_NORMAL);
    GetFont().SetItalic(ITALIC_NONE);

    SmNode *pNode;
    sal_uInt16      nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize; i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->Prepare(rFormat, rDocShell);
}

sal_uInt16 SmNode::FindIndex() const
{
    const SmStructureNode* pParent = GetParent();
    if (!pParent) { return 0; }

    for (sal_uInt16 i = 0; i < pParent->GetNumSubNodes(); ++i) {
        if (pParent->GetSubNode(i) == this) {
            return i;
        }
    }

    DBG_ASSERT(false, "Connection between parent and child is inconsistent.");
    return 0;
}


#if OSL_DEBUG_LEVEL > 1
void  SmNode::ToggleDebug() const
    // toggle 'bIsDebug' in current subtree
{
    SmNode *pThis = (SmNode *) this;

    pThis->bIsDebug = bIsDebug ? false : true;

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

void SmNode::CreateTextFromNode(OUString &rText)
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    if (nSize > 1)
        rText += "{";
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->CreateTextFromNode(rText);
    if (nSize > 1)
    {
        rText = comphelper::string::stripEnd(rText, ' ');
        rText += "} ";
    }
}


void SmNode::AdaptToX(const OutputDevice &/*rDev*/, sal_uLong /*nWidth*/)
{
}


void SmNode::AdaptToY(const OutputDevice &/*rDev*/, sal_uLong /*nHeight*/)
{
}


const SmNode * SmNode::FindTokenAt(sal_uInt16 nRow, sal_uInt16 nCol) const
    // returns (first) ** visible ** (sub)node with the tokens text at
    // position 'nRow', 'nCol'.
    //! (there should be exactly one such node if any)
{
    if (    IsVisible()
        &&  nRow == GetToken().nRow
        &&  nCol >= GetToken().nCol  &&  nCol < GetToken().nCol + GetToken().aText.getLength())
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

void SmNode::GetAccessibleText( OUStringBuffer &/*rText*/ ) const
{
    OSL_FAIL( "SmNode: GetAccessibleText not overloaded" );
}

const SmNode * SmNode::FindNodeWithAccessibleIndex(xub_StrLen nAccIdx) const
{
    const SmNode *pResult = 0;

    sal_Int32 nIdx = GetAccessibleIndex();
    OUStringBuffer aTxt;
    if (nIdx >= 0)
        GetAccessibleText( aTxt );  // get text if used in following 'if' statement

    if (nIdx >= 0
        &&  nIdx <= nAccIdx  &&  nAccIdx < nIdx + aTxt.getLength())
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

#ifdef DEBUG_ENABLE_DUMPASDOT
void SmNode::DumpAsDot(std::ostream &out, OUString* label, int number, int& id, int parent) const
{
    //If this is the root start the file
    if(number == -1){
        out<<"digraph {"<<std::endl;
        if(label){
            out<<"labelloc = \"t\";"<<std::endl;
            OUString eq(*label);
            //CreateTextFromNode(eq);
            eq = eq.replaceAll("\n", " ");
            eq = eq.replaceAll("\\", "\\\\");
            eq = eq.replaceAll("\"", "\\\"");
            out<<"label= \"Equation: \\\"";
            out<< OUStringToOString(eq, RTL_TEXTENCODING_UTF8).getStr();
            out<<"\\\"\";"<<std::endl;
        }
    }

    //Some how out<<(int)this; doesn't work... So we  do this nasty workaround...
    char strid[100];
    sprintf(strid, "%i", id);

    char strnr[100];
    sprintf(strnr, "%i", number);

    //Dump connection to this node
    if( parent != -1 ){
        char pid[100];
        sprintf(pid, "%i", parent);
        out<<"n"<<pid<<" -> n"<<strid<<" [label=\""<<strnr<<"\"];"<<std::endl;
    //If doesn't have parent and isn't a rootnode:
    } else if(number != -1) {
        out<<"orphaned -> n"<<strid<<" [label=\""<<strnr<<"\"];"<<std::endl;
    }

    //Dump this node
    out<<"n"<< strid<<" [label=\"";
    switch( GetType() ) {
        case NTABLE:           out<<"SmTableNode"; break;
        case NBRACE:           out<<"SmBraceNode"; break;
        case NBRACEBODY:       out<<"SmBracebodyNode"; break;
        case NOPER:            out<<"SmOperNode"; break;
        case NALIGN:           out<<"SmAlignNode"; break;
        case NATTRIBUT:        out<<"SmAttributNode"; break;
        case NFONT:            out<<"SmFontNode"; break;
        case NUNHOR:           out<<"SmUnHorNode"; break;
        case NBINHOR:          out<<"SmBinHorNode"; break;
        case NBINVER:          out<<"SmBinVerNode"; break;
        case NBINDIAGONAL:     out<<"SmBinDiagonalNode"; break;
        case NSUBSUP:          out<<"SmSubSupNode"; break;
        case NMATRIX:          out<<"SmMatrixNode"; break;
        case NPLACE:           out<<"SmPlaceNode"; break;
        case NTEXT:
            out<<"SmTextNode: ";
            out<< OUStringToOString(((SmTextNode*)this)->GetText(), RTL_TEXTENCODING_UTF8).getStr();
            break;
        case NSPECIAL:             out<<"SmSpecialNode"; break;
        case NGLYPH_SPECIAL:   out<<"SmGlyphSpecialNode"; break;
        case NMATH:
            out<<"SmMathSymbolNode: ";
            out<< OUStringToOString(((SmMathSymbolNode*)this)->GetText(), RTL_TEXTENCODING_UTF8).getStr();
            break;
        case NBLANK:           out<<"SmBlankNode"; break;
        case NERROR:           out<<"SmErrorNode"; break;
        case NLINE:            out<<"SmLineNode"; break;
        case NEXPRESSION:      out<<"SmExpressionNode"; break;
        case NPOLYLINE:        out<<"SmPolyLineNode"; break;
        case NROOT:            out<<"SmRootNode"; break;
        case NROOTSYMBOL:      out<<"SmRootSymbolNode"; break;
        case NRECTANGLE:       out<<"SmRectangleNode"; break;
        case NVERTICAL_BRACE:  out<<"SmVerticalBraceNode"; break;
        case NMATHIDENT:       out<<"SmMathIdentifierNode"; break;
        default:
            out<<"Unknown Node";
    }
    out<<"\"";
    if(IsSelected())
        out<<", style=dashed";
    out<<"];"<<std::endl;

    //Dump subnodes
    int myid = id;
    const SmNode *pNode;
    sal_uInt16 nSize = GetNumSubNodes();
    for (sal_uInt16 i = 0; i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
            pNode->DumpAsDot(out, NULL, i, ++id, myid);

    //If this is the root end the file
    if( number == -1 )
        out<<"}"<<std::endl;
}
#endif /* DEBUG_ENABLE_DUMPASDOT */

long SmNode::GetFormulaBaseline() const
{
    OSL_FAIL( "This dummy implementation should not have been called." );
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
    ClaimPaternity();
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

    ClaimPaternity();

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

    ClaimPaternity();
}


void SmStructureNode::SetSubNodes(const SmNodeArray &rNodeArray)
{
    aSubNodes = rNodeArray;
    ClaimPaternity();
}


bool SmStructureNode::IsVisible() const
{
    return false;
}


sal_uInt16 SmStructureNode::GetNumSubNodes() const
{
    return (sal_uInt16) aSubNodes.size();
}


SmNode * SmStructureNode::GetSubNode(sal_uInt16 nIndex)
{
    return aSubNodes[nIndex];
}


void SmStructureNode::GetAccessibleText( OUStringBuffer &rText ) const
{
    sal_uInt16 nNodes = GetNumSubNodes();
    for (sal_uInt16 i = 0;  i < nNodes;  ++i)
    {
        const SmNode *pNode = ((SmStructureNode *) this)->GetSubNode(i);
        if (pNode)
        {
            if (pNode->IsVisible())
                ((SmStructureNode *) pNode)->nAccIndex = rText.getLength();
            pNode->GetAccessibleText( rText );
        }
    }
}

///////////////////////////////////////////////////////////////////////////


bool SmVisibleNode::IsVisible() const
{
    return true;
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

void SmGraphicNode::GetAccessibleText( OUStringBuffer &rText ) const
{
    rText.append(GetToken().aText);
}

///////////////////////////////////////////////////////////////////////////


void SmExpressionNode::CreateTextFromNode(OUString &rText)
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    if (nSize > 1)
        rText += "{";
    for (sal_uInt16 i = 0;  i < nSize;  i++)
        if (NULL != (pNode = GetSubNode(i)))
        {
            pNode->CreateTextFromNode(rText);
            //Just a bit of foo to make unary +asd -asd +-asd -+asd look nice
            if (pNode->GetType() == NMATH)
                if ((nSize != 2) || ((rText[rText.getLength()-1] != '+') &&
                    (rText[rText.getLength()-1] != '-')))
                    rText += " ";
        }

    if (nSize > 1)
    {
        rText = comphelper::string::stripEnd(rText, ' ');
        rText += "} ";
    }
}


///////////////////////////////////////////////////////////////////////////

void SmTableNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
    // arranges all subnodes in one column
{
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
            RectHorAlign  eHorAlign = pCoNode->GetRectHorAlign();

            aPos = rNodeRect.AlignTo(*this, RP_BOTTOM,
                        eHorAlign, RVA_BASELINE);
            if (i)
                aPos.Y() += nDist;
            pNode->MoveTo(aPos);
            ExtendBy(rNodeRect, nSize > 1 ? RCP_NONE : RCP_ARG);
        }
    }
    // #i972#
    if (HasBaseline())
        nFormulaBaseline = GetBaseline();
    else
    {
        SmTmpDevice  aTmpDev ((OutputDevice &) rDev, sal_True);
        aTmpDev.SetFont(GetFont());

        SmRect aRect = (SmRect(aTmpDev, &rFormat, OUString("a"),
                               GetFont().GetBorderWidth()));
        nFormulaBaseline = GetAlignM();
        // move from middle position by constant - distance
        // between middle and baseline for single letter
        nFormulaBaseline += aRect.GetBaseline() - aRect.GetAlignM();
    }
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

    // Here we use the 'FNT_VARIABLE' font since it's ascent and descent in general fit better
    // to the rest of the formula compared to the 'FNT_MATH' font.
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    if (nSize < 1)
    {
        // provide an empty rectangle with alignment parameters for the "current"
        // font (in order to make "a^1 {}_2^3 a_4" work correct, that is, have the
        // same sub-/supscript positions.)
        //! be sure to use a character that has explicitly defined HiAttribut
        //! line in rect.cxx such as 'a' in order to make 'vec a' look same to
        //! 'vec {a}'.
        SmRect::operator = (SmRect(aTmpDev, &rFormat, OUString("a"),
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
        SetRectHorAlign(pNode->GetRectHorAlign(), false);
}


/**************************************************************************/


void SmUnHorNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    bool  bIsPostfix = GetToken().eType == TFACT;

    SmNode *pOper = GetSubNode(bIsPostfix ? 1 : 0),
           *pBody = GetSubNode(bIsPostfix ? 0 : 1);
    OSL_ENSURE(pOper, "Sm: NULL pointer");
    OSL_ENSURE(pBody, "Sm: NULL pointer");

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

    OSL_ENSURE(rHeight    >= 0, "Sm : Ooops...");
    OSL_ENSURE(rVerOffset >= 0, "Sm : Ooops...");
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
    OSL_ENSURE(pRootSym, "Sm: NULL pointer");
    OSL_ENSURE(pBody,    "Sm: NULL pointer");

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
    //! overrride calculated vertical position
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
        ExtendBy(*pExtra, RCP_THIS, true);
}


void SmRootNode::CreateTextFromNode(OUString &rText)
{
    SmNode *pExtra = GetSubNode(0);
    if (pExtra)
    {
        rText += "nroot ";
        pExtra->CreateTextFromNode(rText);
    }
    else
        rText += "sqrt ";

    if (!pExtra && GetSubNode(2)->GetNumSubNodes() > 1)
        rText += "{ ";

    GetSubNode(2)->CreateTextFromNode(rText);

    if (!pExtra && GetSubNode(2)->GetNumSubNodes() > 1)
        rText += "} ";
}


/**************************************************************************/


void SmBinHorNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pLeft  = GetSubNode(0),
           *pOper  = GetSubNode(1),
           *pRight = GetSubNode(2);
    OSL_ENSURE(pLeft  != NULL, "Sm: NULL pointer");
    OSL_ENSURE(pOper  != NULL, "Sm: NULL pointer");
    OSL_ENSURE(pRight != NULL, "Sm: NULL pointer");

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
    OSL_ENSURE(pNum,   "Sm : NULL pointer");
    OSL_ENSURE(pLine,  "Sm : NULL pointer");
    OSL_ENSURE(pDenom, "Sm : NULL pointer");

    bool  bIsTextmode = rFormat.IsTextmode();
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
          nWidth      = std::max(pNum->GetItalicWidth(), pDenom->GetItalicWidth()),
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

void SmBinVerNode::CreateTextFromNode(OUString &rText)
{
    SmNode *pNum   = GetSubNode(0),
           *pDenom = GetSubNode(2);
    pNum->CreateTextFromNode(rText);
    rText += "over ";
    pDenom->CreateTextFromNode(rText);
}


SmNode * SmBinVerNode::GetLeftMost()
{
    return this;
}


/**************************************************************************/


/// @return value of the determinant formed by the two points
double Det(const Point &rHeading1, const Point &rHeading2)
{
    return rHeading1.X() * rHeading2.Y() - rHeading1.Y() * rHeading2.X();
}


/// Is true iff the point 'rPoint1' belongs to the straight line through 'rPoint2'
/// and has the direction vector 'rHeading2'
bool IsPointInLine(const Point &rPoint1,
                   const Point &rPoint2, const Point &rHeading2)
{
    OSL_ENSURE(rHeading2 != Point(), "Sm : 0 vector");

    bool bRes = false;
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
    OSL_ENSURE(rHeading1 != Point(), "Sm : 0 vector");
    OSL_ENSURE(rHeading2 != Point(), "Sm : 0 vector");

    sal_uInt16 nRes = 1;
    const double eps = 5.0 * DBL_EPSILON;

    // are the direction vectors linearly dependent?
    double  fDet = Det(rHeading1, rHeading2);
    if (fabs(fDet) < eps)
    {
        nRes    = IsPointInLine(rPoint1, rPoint2, rHeading2) ? USHRT_MAX : 0;
        rResult = nRes ? rPoint1 : Point();
    }
    else
    {
        // here we do not pay attention to the computational accurancy
        // (that would be more complicated and is not really worth it in this case)
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
    bAscending = false;
    SetNumSubNodes(3);
}


/// @return position and size of the diagonal line
/// premise: SmRect of the node defines the limitation(!) consequently it has to be known upfront
void SmBinDiagonalNode::GetOperPosSize(Point &rPos, Size &rSize,
                        const Point &rDiagPoint, double fAngleDeg) const

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

    long  nLeft, nRight, nTop, nBottom;     // margins of the rectangle for the diagonal
    Point aPoint;
    if (IsAscending())
    {
        // determine top right corner
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectTop), aRightHdg,
            rDiagPoint, aDiagHdg);
        // is there a point of intersection with the top border?
        if (aPoint.X() <= nRectRight)
        {
            nRight = aPoint.X();
            nTop   = nRectTop;
        }
        else
        {
            // there has to be a point of intersection with the right border!
            GetLineIntersectionPoint(aPoint,
                Point(nRectRight, nRectTop), aDownHdg,
                rDiagPoint, aDiagHdg);

            nRight = nRectRight;
            nTop   = aPoint.Y();
        }

        // determine bottom left corner
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectBottom), aRightHdg,
            rDiagPoint, aDiagHdg);
        // is there a point of intersection with the bottom border?
        if (aPoint.X() >= nRectLeft)
        {
            nLeft   = aPoint.X();
            nBottom = nRectBottom;
        }
        else
        {
            // there has to be a point of intersection with the left border!
            GetLineIntersectionPoint(aPoint,
                Point(nRectLeft, nRectTop), aDownHdg,
                rDiagPoint, aDiagHdg);

            nLeft   = nRectLeft;
            nBottom = aPoint.Y();
        }
    }
    else
    {
        // determine top left corner
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectTop), aRightHdg,
            rDiagPoint, aDiagHdg);
        // is there a point of intersection with the top border?
        if (aPoint.X() >= nRectLeft)
        {
            nLeft = aPoint.X();
            nTop  = nRectTop;
        }
        else
        {
            // there has to be a point of intersection with the left border!
            GetLineIntersectionPoint(aPoint,
                Point(nRectLeft, nRectTop), aDownHdg,
                rDiagPoint, aDiagHdg);

            nLeft = nRectLeft;
            nTop  = aPoint.Y();
        }

        // determine bottom right corner
        GetLineIntersectionPoint(aPoint,
            Point(nRectLeft, nRectBottom), aRightHdg,
            rDiagPoint, aDiagHdg);
        // is there a point of intersection with the bottom border?
        if (aPoint.X() <= nRectRight)
        {
            nRight  = aPoint.X();
            nBottom = nRectBottom;
        }
        else
        {
            // there has to be a point of intersection with the right border!
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
    // Both arguments have to get into the SubNodes before the Operator so that clicking
    // within the GraphicWindow sets the FormulaCursor correctly (cf. SmRootNode)
    SmNode *pLeft  = GetSubNode(0),
           *pRight = GetSubNode(1);
    OSL_ENSURE(pLeft, "Sm : NULL pointer");
    OSL_ENSURE(pRight, "Sm : NULL pointer");

    OSL_ENSURE(GetSubNode(2)->GetType() == NPOLYLINE, "Sm : wrong node type");
    SmPolyLineNode *pOper = (SmPolyLineNode *) GetSubNode(2);
    OSL_ENSURE(pOper, "Sm : NULL pointer");

    //! some routines being called extract some info from the OutputDevice's
    //! font (eg the space to be used for borders OR the font name(!!)).
    //! Thus the font should reflect the needs and has to be set!
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    pLeft->Arrange(aTmpDev, rFormat);
    pRight->Arrange(aTmpDev, rFormat);

    // determine implicitely the values (incl. the margin) of the diagonal line
    pOper->Arrange(aTmpDev, rFormat);

    long nDelta = pOper->GetWidth() * 8 / 10;

    // determine TopLeft position from the right argument
    Point aPos;
    aPos.X() = pLeft->GetItalicRight() + nDelta + pRight->GetItalicLeftSpace();
    if (IsAscending())
        aPos.Y() = pLeft->GetBottom() + nDelta;
    else
        aPos.Y() = pLeft->GetTop() - nDelta - pRight->GetHeight();

    pRight->MoveTo(aPos);

    // determine new baseline
    long nTmpBaseline = IsAscending() ? (pLeft->GetBottom() + pRight->GetTop()) / 2
                        : (pLeft->GetTop() + pRight->GetBottom()) / 2;
    Point  aLogCenter ((pLeft->GetItalicRight() + pRight->GetItalicLeft()) / 2,
                       nTmpBaseline);

    SmRect::operator = (*pLeft);
    ExtendBy(*pRight, RCP_NONE);


    // determine position and size of diagonal line
    Size  aTmpSize;
    GetOperPosSize(aPos, aTmpSize, aLogCenter, IsAscending() ? 60.0 : -60.0);

    // font specialist advised to change the width first
    pOper->AdaptToY(aTmpDev, aTmpSize.Height());
    pOper->AdaptToX(aTmpDev, aTmpSize.Width());
    // and make it active
    pOper->Arrange(aTmpDev, rFormat);

    pOper->MoveTo(aPos);

    ExtendBy(*pOper, RCP_NONE, nTmpBaseline);
}


/**************************************************************************/


void SmSubSupNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    OSL_ENSURE(GetNumSubNodes() == 1 + SUBSUP_NUM_ENTRIES,
               "Sm: wrong number of subnodes");

    SmNode *pBody = GetBody();
    OSL_ENSURE(pBody, "Sm: NULL pointer");

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

        bool  bIsTextmode = rFormat.IsTextmode();
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
                OSL_FAIL("Sm: unknown case");
                break;
        }

        pSubSup->MoveTo(aPos);
        ExtendBy(*pSubSup, RCP_THIS, true);

        // update rectangle to which  RSUB, RSUP, LSUB, LSUP
        // will be aligned to
        if (eSubSup == CSUB  ||  eSubSup == CSUP)
            aTmpRect = *this;
    }
}

void SmSubSupNode::CreateTextFromNode(OUString &rText)
{
    SmNode *pNode;
    GetSubNode(0)->CreateTextFromNode(rText);

    if (NULL != (pNode = GetSubNode(LSUB+1)))
    {
        rText += "lsub ";
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(LSUP+1)))
    {
        rText += "lsup ";
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(CSUB+1)))
    {
        rText += "csub ";
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(CSUP+1)))
    {
        rText += "csup ";
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(RSUB+1)))
    {
        rText = comphelper::string::stripEnd(rText, ' ');
        rText += "_";
        pNode->CreateTextFromNode(rText);
    }
    if (NULL != (pNode = GetSubNode(RSUP+1)))
    {
        rText = comphelper::string::stripEnd(rText, ' ');
        rText += "^";
        pNode->CreateTextFromNode(rText);
    }
}


/**************************************************************************/

void SmBraceNode::CreateTextFromNode(OUString &rText)
{
    if (GetScaleMode() == SCALE_HEIGHT)
        rText += "left ";
    {
        OUString aStr;
        GetSubNode(0)->CreateTextFromNode(aStr);
        aStr = comphelper::string::strip(aStr, ' ');
        aStr = comphelper::string::stripStart(aStr, '\\');
        if (!aStr.isEmpty())
        {
            if (aStr.equalsAscii("divides"))
                rText += "lline";
            else if (aStr.equalsAscii("parallel"))
                rText += "ldline";
            else if (aStr.equalsAscii("<"))
                rText += "langle";
            else
                rText += aStr;
            rText += " ";
        }
        else
            rText += "none ";
    }
    GetSubNode(1)->CreateTextFromNode(rText);
    if (GetScaleMode() == SCALE_HEIGHT)
        rText += "right ";
    {
        OUString aStr;
        GetSubNode(2)->CreateTextFromNode(aStr);
        aStr = comphelper::string::strip(aStr, ' ');
        aStr = comphelper::string::stripStart(aStr, '\\');
        if (!aStr.isEmpty())
        {
            if (aStr.equalsAscii("divides"))
                rText += "rline";
            else if (aStr.equalsAscii("parallel"))
                rText += "rdline";
            else if (aStr.equalsAscii(">"))
                rText += "rangle";
            else
                rText += aStr;
            rText += " ";
        }
        else
            rText += "none ";
    }
    rText += " ";

}

void SmBraceNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmNode *pLeft  = GetSubNode(0),
           *pBody  = GetSubNode(1),
           *pRight = GetSubNode(2);
    OSL_ENSURE(pLeft,  "Sm: NULL pointer");
    OSL_ENSURE(pBody,  "Sm: NULL pointer");
    OSL_ENSURE(pRight, "Sm: NULL pointer");

    pBody->Arrange(rDev, rFormat);

    bool  bIsScaleNormal = rFormat.IsScaleNormalBrackets(),
          bScale         = pBody->GetHeight() > 0  &&
                           (GetScaleMode() == SCALE_HEIGHT  ||  bIsScaleNormal),
          bIsABS         = GetToken().eType == TABS;

    long  nFaceHeight = GetFont().GetSize().Height();

    // determine oversize in %
    sal_uInt16  nPerc = 0;
    if (!bIsABS && bScale)
    {   // in case of oversize braces...
        sal_uInt16 nIndex = GetScaleMode() == SCALE_HEIGHT ?
                            DIS_BRACKETSIZE : DIS_NORMALBRACKETSIZE;
        nPerc = rFormat.GetDistance(nIndex);
    }

    // determine the height for the braces
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

    // distance to the argument
    nPerc = bIsABS ? 0 : rFormat.GetDistance(DIS_BRACKETSPACE);
    long  nDist = nFaceHeight * nPerc / 100L;

    // if wanted, scale the braces to the wanted size
    if (bScale)
    {
        Size  aTmpSize (pLeft->GetFont().GetSize());
        OSL_ENSURE(pRight->GetFont().GetSize() == aTmpSize,
                    "Sm : different font sizes");
        aTmpSize.Width() = std::min((long) nBraceHeight * 60L / 100L,
                            rFormat.GetBaseSize().Height() * 3L / 2L);
        // correction factor since change from StarMath to OpenSymbol font
        // because of the different font width in the FontMetric
        aTmpSize.Width() *= 182;
        aTmpSize.Width() /= 267;

        sal_Unicode cChar = pLeft->GetToken().cMathChar;
        if (cChar != MS_LINE  &&  cChar != MS_DLINE &&
            cChar != MS_VERTLINE  &&  cChar != MS_DVERTLINE)
            pLeft ->GetFont().SetSize(aTmpSize);

        cChar = pRight->GetToken().cMathChar;
        if (cChar != MS_LINE  &&  cChar != MS_DLINE &&
            cChar != MS_VERTLINE  &&  cChar != MS_DVERTLINE)
            pRight->GetFont().SetSize(aTmpSize);

        pLeft ->AdaptToY(rDev, nBraceHeight);
        pRight->AdaptToY(rDev, nBraceHeight);
    }

    pLeft ->Arrange(rDev, rFormat);
    pRight->Arrange(rDev, rFormat);

    // required in order to make "\(a\) - (a) - left ( a right )" look alright
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
    bool bScale  = GetScaleMode() == SCALE_HEIGHT  ||  rFormat.IsScaleNormalBrackets();
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
        bool          bIsSeparator = i % 2 != 0;
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
    OSL_ENSURE(pBody,   "Sm: NULL pointer!");
    OSL_ENSURE(pBrace,  "Sm: NULL pointer!");
    OSL_ENSURE(pScript, "Sm: NULL pointer!");

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    pBody->Arrange(aTmpDev, rFormat);

    // size is the same as for limits for this part
    pScript->SetSize( Fraction( rFormat.GetRelSize(SIZ_LIMITS), 100 ) );
    // braces are a bit taller than usually
    pBrace ->SetSize( Fraction(3, 2) );

    long  nItalicWidth = pBody->GetItalicWidth();
    if (nItalicWidth > 0)
        pBrace->AdaptToX(aTmpDev, nItalicWidth);

    pBrace ->Arrange(aTmpDev, rFormat);
    pScript->Arrange(aTmpDev, rFormat);

    // determine the relative position and the distances between each other
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
    OSL_ENSURE(pNode, "Sm: NULL pointer!");

    if (pNode->GetType() == NSUBSUP)
        pNode = ((SmSubSupNode *) pNode)->GetBody();

    OSL_ENSURE(pNode, "Sm: NULL pointer!");
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

    OSL_ENSURE(pOper, "Sm: missing subnode");
    OSL_ENSURE(pBody, "Sm: missing subnode");

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
    // set alignment within the entire subtree (including current node)
{
    OSL_ENSURE(GetNumSubNodes() > 0, "Sm: missing subnode");

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
    OSL_ENSURE(pBody, "Sm: body missing");
    OSL_ENSURE(pAttr, "Sm: attribute missing");

    pBody->Arrange(rDev, rFormat);

    if (GetScaleMode() == SCALE_WIDTH)
        pAttr->AdaptToX(rDev, pBody->GetItalicWidth());
    pAttr->Arrange(rDev, rFormat);

    // get relative position of attribute
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
    ExtendBy(*pAttr, RCP_THIS, true);
}


/**************************************************************************/




void SmFontNode::CreateTextFromNode(OUString &rText)
{
    switch (GetToken().eType)
    {
        case TBOLD:
            rText += "bold ";
            break;
        case TNBOLD:
            rText += "nbold ";
            break;
        case TITALIC:
            rText += "italic ";
            break;
        case TNITALIC:
            rText += "nitalic ";
            break;
        case TPHANTOM:
            rText += "phantom ";
            break;
        case TSIZE:
            {
                rText += "size ";
                switch (nSizeType)
                {
                    case FNTSIZ_PLUS:
                        rText += "+";
                        break;
                    case FNTSIZ_MINUS:
                        rText += "-";
                        break;
                    case FNTSIZ_MULTIPLY:
                        rText += "*'";
                        break;
                    case FNTSIZ_DIVIDE:
                        rText += "/";
                        break;
                    case FNTSIZ_ABSOLUT:
                    default:
                        break;
                }
                rText += ::rtl::math::doubleToUString(
                            static_cast<double>(aFontSize),
                            rtl_math_StringFormat_Automatic,
                            rtl_math_DecimalPlaces_Max, '.', sal_True);
                rText += " ";
            }
            break;
        case TBLACK:
            rText += "color black ";
            break;
        case TWHITE:
            rText += "color white ";
            break;
        case TRED:
            rText += "color red ";
            break;
        case TGREEN:
            rText += "color green ";
            break;
        case TBLUE:
            rText += "color blue ";
            break;
        case TCYAN:
            rText += "color cyan ";
            break;
        case TMAGENTA:
            rText += "color magenta ";
            break;
        case TYELLOW:
            rText += "color yellow ";
            break;
        case TSANS:
            rText += "font sans ";
            break;
        case TSERIF:
            rText += "font serif ";
            break;
        case TFIXED:
            rText += "font fixed ";
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
    OSL_ENSURE(pNode, "Sm: missing subnode");

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

        case TPHANTOM : SetPhantom(true);               break;
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
            OSL_FAIL("Sm: unknown case");
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
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    long  nBorderwidth = GetFont().GetBorderWidth();

    // create polygon using both endpoints
    OSL_ENSURE(aPoly.GetSize() == 2, "Sm : wrong number of points");
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
        OSL_ENSURE(GetToken().eType == TWIDEBACKSLASH, "Sm : unexpected token");
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


/**************************************************************************/

void SmRootSymbolNode::AdaptToX(const OutputDevice &/*rDev*/, sal_uLong nWidth)
{
    nBodyWidth = nWidth;
}


void SmRootSymbolNode::AdaptToY(const OutputDevice &rDev, sal_uLong nHeight)
{
    // some additional length so that the horizontal
    // bar will be positioned above the argument
    SmMathSymbolNode::AdaptToY(rDev, nHeight + nHeight / 10L);
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    // add some borderspace
    sal_uLong  nTmpBorderWidth = GetFont().GetBorderWidth();
    nHeight += 2 * nTmpBorderWidth;

    //! use this method in order to have 'SmRect::HasAlignInfo() == true'
    //! and thus having the attribut-fences updated in 'SmRect::ExtendBy'
    SmRect::operator = (SmRect(nWidth, nHeight));
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
    if (GetToken().aText.getLength() == 1 && GetToken().aText[0] == ':')
        Attributes() &= ~ATTR_ITALIC;
};


void SmTextNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    sal_uInt16  nSizeDesc = GetFontDesc() == FNT_FUNCTION ?
                            SIZ_FUNCTION : SIZ_TEXT;
    GetFont() *= Fraction (rFormat.GetRelSize(nSizeDesc), 100);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, aText, GetFont().GetBorderWidth()));
}

void SmTextNode::CreateTextFromNode(OUString &rText)
{
    bool bQuoted=false;
    if (GetToken().eType == TTEXT)
    {
        rText += "\"";
        bQuoted=true;
    }
    else
    {
        SmParser aParseTest;
        SmNode *pTable = aParseTest.Parse(GetToken().aText);
        bQuoted=true;
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
                        bQuoted=false;
                }
            }
        }
        delete pTable;

        if ((GetToken().eType == TIDENT) && (GetFontDesc() == FNT_FUNCTION))
        {
            //Search for existing functions and remove extraenous keyword
            rText += "func ";
        }
        else if (bQuoted)
            rText += "italic ";

        if (bQuoted)
            rText += "\"";

    }

    rText += GetToken().aText;

    if (bQuoted)
        rText += "\"";
    rText += " ";
}


void SmTextNode::GetAccessibleText( OUStringBuffer &rText ) const
{
    rText.append(aText);
}

void SmTextNode::AdjustFontDesc()
{
    if (GetToken().eType == TTEXT)
        nFontDesc = FNT_TEXT;
    else if(GetToken().eType == TFUNC)
        nFontDesc = FNT_FUNCTION;
    else {
        SmTokenType nTok;
        const SmTokenTableEntry *pEntry = SmParser::GetTokenTableEntry( aText );
        if (pEntry && pEntry->nGroup == TGFUNCTION) {
            nTok = pEntry->eType;
            nFontDesc = FNT_FUNCTION;
        } else {
            sal_Unicode firstChar = aText[0];
            if( ('0' <= firstChar && firstChar <= '9') || firstChar == '.' || firstChar == ',') {
                nFontDesc = FNT_NUMBER;
                nTok = TNUMBER;
            } else if (aText.getLength() > 1) {
                nFontDesc = FNT_VARIABLE;
                nTok = TIDENT;
            } else {
                nFontDesc = FNT_VARIABLE;
                nTok = TCHARACTER;
            }
        }
        SmToken tok = GetToken();
        tok.eType = nTok;
        SetToken(tok);
    }
}

sal_Unicode SmTextNode::ConvertSymbolToUnicode(sal_Unicode nIn)
{
    //Find the best match in accepted unicode for our private area symbols
    static const sal_Unicode aStarMathPrivateToUnicode[] =
    {
        0x2030, 0xF613, 0xF612, 0x002B, 0x003C, 0x003E, 0xE425, 0xE421, 0xE088, 0x2208,
        0x0192, 0x2026, 0x2192, 0x221A, 0x221A, 0x221A, 0xE090, 0x005E, 0x02C7, 0x02D8,
        0x00B4, 0x0060, 0x02DC, 0x00AF, 0x0362, 0xE099, 0xE09A, 0x20DB, 0xE09C, 0xE09D,
        0x0028, 0x0029, 0x2220, 0x22AF, 0xE0A2, 0xE0A3, 0xE0A4, 0xE0A5, 0xE0A6, 0xE0A7,
        0x002F, 0x005C, 0x274F, 0xE0AB, 0x0393, 0x0394, 0x0398, 0x039b, 0x039e, 0x03A0,
        0x03a3, 0x03a5, 0x03a6, 0x03a8, 0x03A9, 0x03B1, 0x03B2, 0x03b3, 0x03b4, 0x03b5,
        0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
        0x03c0, 0x03c1, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 0x03b5,
        0x03d1, 0x03d6, 0xE0D2, 0x03db, 0x2118, 0x2202, 0x2129, 0xE0D7, 0xE0D8, 0x22A4,
        0xE0DA, 0x2190, 0x2191, 0x2193
    };
    if ((nIn >= 0xE080) && (nIn <= 0xE0DD))
        nIn = aStarMathPrivateToUnicode[nIn-0xE080];

    //For whatever unicode glyph that equation editor doesn't ship with that
    //we have a possible match we can munge it to.
    switch (nIn)
    {
        case 0x2223:
            nIn = '|';
            break;
        default:
            break;
    }

    return nIn;
}

/**************************************************************************/

void SmMatrixNode::CreateTextFromNode(OUString &rText)
{
    rText += "matrix {";
    for (sal_uInt16 i = 0;  i < nNumRows; i++)
    {
        for (sal_uInt16 j = 0;  j < nNumCols; j++)
        {
            SmNode *pNode = GetSubNode(i * nNumCols + j);
            pNode->CreateTextFromNode(rText);
            if (j != nNumCols-1)
                rText += "# ";
        }
        if (i != nNumRows-1)
            rText += "## ";
    }
    rText = comphelper::string::stripEnd(rText, ' ');
    rText += "} ";
}


void SmMatrixNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
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
            pColWidth[nCol] = std::max(pColWidth[nCol], pNode->GetItalicWidth());
        }
    }

    // norm distance from which the following two are calcutated
    const int  nNormDist = 3 * GetFont().GetSize().Height();

    // define horizontal and vertical minimal distances that separate
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
            OSL_ENSURE(pTmpNode, "Sm: NULL pointer");

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
    sal_Unicode cChar = GetToken().cMathChar;
    if ((sal_Unicode) '\0' != cChar)
        SetText(OUString(cChar));
}

void SmMathSymbolNode::AdaptToX(const OutputDevice &rDev, sal_uLong nWidth)
{
    // Since there is no function to do this, we try to approximate it:
    Size  aFntSize (GetFont().GetSize());

    //! however the result is a bit better with 'nWidth' as initial font width
    aFntSize.Width() = nWidth;
    GetFont().SetSize(aFntSize);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
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

    // Since we only want to scale the height, we might have
    // to determine the font width in order to keep it
    if (aFntSize.Width() == 0)
    {
        OutputDevice &rDevNC = (OutputDevice &) rDev;
        rDevNC.Push(PUSH_FONT | PUSH_MAPMODE);
        rDevNC.SetFont(GetFont());
        aFntSize.Width() = rDev.GetFontMetric().GetSize().Width();
        rDevNC.Pop();
    }
    OSL_ENSURE(aFntSize.Width() != 0, "Sm: ");

    //! however the result is a bit better with 'nHeight' as initial
    //! font height
    aFntSize.Height() = nHeight;
    GetFont().SetSize(aFntSize);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
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

    OSL_ENSURE(GetFont().GetCharSet() == RTL_TEXTENCODING_SYMBOL  ||
               GetFont().GetCharSet() == RTL_TEXTENCODING_UNICODE,
        "wrong charset for character from StarMath/OpenSymbol font");

    Flags() |= FLG_FONT | FLG_ITALIC;
};


void SmMathSymbolNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    const OUString &rText = GetText();

    if (rText.isEmpty() || rText[0] == '\0')
    {   SmRect::operator = (SmRect());
        return;
    }

    PrepareAttributes();

    GetFont() *= Fraction (rFormat.GetRelSize(SIZ_TEXT), 100);

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, rText, GetFont().GetBorderWidth()));
}

void SmMathSymbolNode::CreateTextFromNode(OUString &rText)
{
    OUString sStr;
    MathType::LookupChar(GetToken().cMathChar, sStr);
    rText += sStr;
}

void SmRectangleNode::CreateTextFromNode(OUString &rText)
{
    switch (GetToken().eType)
    {
    case TUNDERLINE:
        rText += "underline ";
        break;
    case TOVERLINE:
        rText += "overline ";
        break;
    case TOVERSTRIKE:
        rText += "overstrike ";
        break;
    default:
        break;
    }
}

void SmAttributNode::CreateTextFromNode(OUString &rText)
{
    SmNode *pNode;
    sal_uInt16  nSize = GetNumSubNodes();
    OSL_ENSURE(nSize == 2, "Node missing members");
    rText += "{";
    sal_Unicode nLast=0;
    if (NULL != (pNode = GetSubNode(0)))
    {
        OUString aStr;
        pNode->CreateTextFromNode(aStr);
        if (aStr.getLength() > 1)
            rText += aStr;
        else
        {
            nLast = aStr[0];
            switch (nLast)
            {
            case MS_BAR: // MACRON
                rText += "overline ";
                break;
            case MS_DOT: // DOT ABOVE
                rText += "dot ";
                break;
            case 0x2dc: // SMALL TILDE
                rText += "widetilde ";
                break;
            case MS_DDOT: // DIAERESIS
                rText += "ddot ";
                break;
            case 0xE082:
                break;
            case 0xE09B:
            case MS_DDDOT: // COMBINING THREE DOTS ABOVE
                rText += "dddot ";
                break;
            case MS_ACUTE: // ACUTE ACCENT
            case MS_COMBACUTE: // COMBINING ACUTE ACCENT
                rText += "acute ";
                break;
            case MS_GRAVE: // GRAVE ACCENT
            case MS_COMBGRAVE: // COMBINING GRAVE ACCENT
                rText += "grave ";
                break;
            case MS_CHECK: // CARON
            case MS_COMBCHECK: // COMBINING CARON
                rText += "check ";
                break;
            case MS_BREVE: // BREVE
            case MS_COMBBREVE: // COMBINING BREVE
                rText += "breve ";
                break;
            case MS_CIRCLE: // RING ABOVE
            case MS_COMBCIRCLE: // COMBINING RING ABOVE
                rText += "circle ";
                break;
            case MS_RIGHTARROW: // RIGHTWARDS ARROW
            case MS_VEC: // COMBINING RIGHT ARROW ABOVE
                rText += "vec ";
                break;
            case MS_TILDE: // TILDE
            case MS_COMBTILDE: // COMBINING TILDE
                rText += "tilde ";
                break;
            case MS_HAT: // CIRCUMFLEX ACCENT
            case MS_COMBHAT: // COMBINING CIRCUMFLEX ACCENT
                rText += "hat ";
                break;
            case MS_COMBBAR: // COMBINING MACRON
                rText += "bar ";
                break;
            default:
                rText += OUString( nLast );
                break;
            }
        }
    }

    if (nSize == 2)
        if (NULL != (pNode = GetSubNode(1)))
            pNode->CreateTextFromNode(rText);

    rText = comphelper::string::stripEnd(rText, ' ');

    if (nLast == 0xE082)
        rText += " overbrace {}";

    rText += "} ";
}

/**************************************************************************/

static bool lcl_IsFromGreekSymbolSet( const OUString &rTokenText )
{
    bool bRes = false;

    // valid symbol name needs to have a '%' at pos 0 and at least an additonal char
    if (rTokenText.getLength() > 2 && rTokenText[0] == (sal_Unicode)'%')
    {
        OUString aName( rTokenText.copy(1) );
        SmSym *pSymbol = SM_MOD()->GetSymbolManager().GetSymbolByName( aName );
        if (pSymbol && GetExportSymbolSetName(pSymbol->GetSymbolSetName()) == "Greek")
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
    SmTextNode(NSPECIAL, rNodeToken, FNT_MATH)  // default Font isn't always correct!
{
    bIsFromGreekSymbolSet = lcl_IsFromGreekSymbolSet( rNodeToken.aText );
}


void SmSpecialNode::Prepare(const SmFormat &rFormat, const SmDocShell &rDocShell)
{
    SmNode::Prepare(rFormat, rDocShell);

    const SmSym   *pSym;
    SmModule  *pp = SM_MOD();

    OUString aName(GetToken().aText.copy(1));
    if (NULL != (pSym = pp->GetSymbolManager().GetSymbolByName( aName )))
    {
        sal_UCS4 cChar = pSym->GetCharacter();
        OUString aTmp( &cChar, 1 );
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

    // Actually only WEIGHT_NORMAL and WEIGHT_BOLD should occur... However, the sms-file also
    // contains e.g. 'WEIGHT_ULTRALIGHT'. Consequently, compare here with '>' instead of '!='.
    // (In the long term the necessity for 'PrepareAttribut' and thus also for this here should be dropped)
    //
    //! see also SmFontStyles::GetStyleName
    if (IsItalic( GetFont() ))
        SetAttribut(ATTR_ITALIC);
    if (IsBold( GetFont() ))
        SetAttribut(ATTR_BOLD);

    Flags() |= FLG_FONT;

    if (bIsFromGreekSymbolSet)
    {
        OSL_ENSURE( GetText().getLength() == 1, "a symbol should only consist of 1 char!" );
        bool bItalic = false;
        sal_Int16 nStyle = rFormat.GetGreekCharStyle();
        OSL_ENSURE( nStyle >= 0 && nStyle <= 2, "unexpected value for GreekCharStyle" );
        if (nStyle == 1)
            bItalic = true;
        else if (nStyle == 2)
        {
            const OUString& rTmp(GetText());
            if (rTmp.isEmpty())
            {
                const sal_Unicode cUppercaseAlpha = 0x0391;
                const sal_Unicode cUppercaseOmega = 0x03A9;
                sal_Unicode cChar = rTmp[0];
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    SmRect::operator = (SmRect(aTmpDev, &rFormat, GetText(), GetFont().GetBorderWidth()));
}

/**************************************************************************/


void SmGlyphSpecialNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    PrepareAttributes();

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
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

    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    const OUString &rText = GetText();
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

    // Here it need/should not be the StarMath font, so that for the character
    // used in Arrange a normal (non-clipped) rectangle is generated
    GetFont() = rFormat.GetFont(FNT_VARIABLE);

    Flags() |= FLG_FONT | FLG_BOLD | FLG_ITALIC;
}


void SmBlankNode::Arrange(const OutputDevice &rDev, const SmFormat &rFormat)
{
    SmTmpDevice  aTmpDev ((OutputDevice &) rDev, true);
    aTmpDev.SetFont(GetFont());

    // make distance depend on the font height
    // (so that it increases when scaling (e.g. size *2 {a ~ b})
    long  nDist  = GetFont().GetSize().Height() / 10L,
          nSpace = nNum * nDist;

    // get a SmRect with Baseline and all the bells and whistles
    SmRect::operator = (SmRect(aTmpDev, &rFormat, OUString(' '),
                               GetFont().GetBorderWidth()));

    // and resize it to the requested size
    SetItalicSpaces(0, 0);
    SetWidth(nSpace);
}

/**************************************************************************/
//Implementation of all accept methods for SmVisitor

void SmNode::Accept(SmVisitor*){
    //This method is only implemented to avoid making SmNode abstract because an
    //obscure copy constructor is used... I can't find it's implementation, and
    //don't want to figure out how to fix it... If you want to, just delete this
    //method, making SmNode abstract, and see where you can an problem with that.
    OSL_FAIL("SmNode should not be visitable!");
}

void SmTableNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmBraceNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmBracebodyNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmOperNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmAlignNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmAttributNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmFontNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmUnHorNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmBinHorNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmBinVerNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmBinDiagonalNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmSubSupNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmMatrixNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmPlaceNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmTextNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmSpecialNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmGlyphSpecialNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmMathSymbolNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmBlankNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmErrorNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmLineNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmExpressionNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmPolyLineNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmRootNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmRootSymbolNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmRectangleNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

void SmVerticalBraceNode::Accept(SmVisitor* pVisitor) {
    pVisitor->Visit(this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
