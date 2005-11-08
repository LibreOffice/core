/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwNodeNum.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 17:14:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <svx/svxenum.hxx>
#include <SwNodeNum.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>

SwNodeNum::SwNodeNum()
    : SwNumberTreeNode(), mpTxtNode(NULL), mpNumRule(NULL), mnStart(1),
      mbRestart(false)
{
}

SwNodeNum::SwNodeNum(const SwNodeNum & rNodeNum)
    : SwNumberTreeNode(rNodeNum), mpTxtNode(NULL),
      mpNumRule(NULL), mnStart(rNodeNum.mnStart),
      mbRestart(rNodeNum.mbRestart)
{
}

SwNodeNum::~SwNodeNum()
{
}

void SwNodeNum::SetTxtNode(SwTxtNode * pTxtNode)
{
    mpTxtNode = pTxtNode;
}

SwTxtNode * SwNodeNum::GetTxtNode() const
{
    return mpTxtNode;
}

void SwNodeNum::SetNumRule(SwNumRule * pRule)
{
    mpNumRule = pRule;
}

SwNumRule * SwNodeNum::GetNumRule() const
{
    return mpNumRule;
}

SwPosition SwNodeNum::GetPosition() const
{
    return SwPosition(*mpTxtNode);
}

SwNumberTreeNode * SwNodeNum::Create() const
{
    SwNodeNum * pResult = new SwNodeNum();

    pResult->SetNumRule(mpNumRule);

    return pResult;
}

SwNumberTreeNode * SwNodeNum::Copy() const
{
    return new SwNodeNum(*this);
}

void SwNodeNum::RemoveChild(SwNumberTreeNode * _pChild)
{
    SwNodeNum * pChild = static_cast<SwNodeNum*>(_pChild);

    pChild->SetNumRule(NULL);

    SwNumberTreeNode::RemoveChild(_pChild);
}

bool SwNodeNum::IsNotifiable() const
{
    bool aResult = true;

    if (mpTxtNode)
        aResult = mpTxtNode->IsNotifiable();

    return aResult;
}

bool SwNodeNum::IsContinuous() const
{
    bool aResult = false;

    if (mpTxtNode)
        aResult = mpTxtNode->IsContinuous();

    return aResult;
}

bool SwNodeNum::IsCounted() const
{
    bool aResult = false;

    if (mpTxtNode)
    {
        const SwNumFmt * pNumFmt = GetNumFmt();

        if (pNumFmt)
        {
            sal_Int16 nType = pNumFmt->GetNumberingType();

            if ( nType != SVX_NUM_NUMBER_NONE)
                aResult = mpTxtNode->IsCounted();
        }
    }
    else
        aResult = SwNumberTreeNode::IsCounted();

    return aResult;
}

void SwNodeNum::NotifyNode()
{
    ValidateMe();

    if (mpTxtNode)
    {
        mpTxtNode->NumRuleChgd();
    }
}

bool SwNodeNum::LessThan(const SwNumberTreeNode & rNode) const
{
    bool bResult = false;
    const SwNodeNum & rTmpNode = static_cast<const SwNodeNum &>(rNode);

    if (mpTxtNode == NULL && rTmpNode.mpTxtNode != NULL)
        bResult = true;
    else if (mpTxtNode != NULL && rTmpNode.mpTxtNode != NULL)
    {
        SwPosition aMyPos(*mpTxtNode);
        SwPosition aHisPos(*rTmpNode.mpTxtNode);

        bResult = (aMyPos < aHisPos) ? true : false;
    }

    return bResult;
}

void SwNodeNum::SetRestart(bool bRestart)
{
    // --> OD 2005-10-19 #126009#
    // - improvement: invalidation only, if <IsRestart()> state changes.
    const bool bInvalidate( mbRestart != bRestart );
    // <--
    mbRestart = bRestart;

    // --> OD 2005-10-19 #126009#
    if ( bInvalidate )
    {
        InvalidateMe();
        NotifyInvalidSiblings();
    }
    // <--
}

bool SwNodeNum::IsRestart() const
{
    return mbRestart;
}

void SwNodeNum::SetStart(SwNumberTreeNode::tSwNumTreeNumber nStart)
{
    // --> OD 2005-10-19 #126009#
    // - improvement: invalidation only, if <IsRestart()> state changes.
    const bool bInvalidate( mnStart != nStart );
    // <--
    mnStart = nStart;

    // --> OD 2005-10-19 #126009#
    if ( bInvalidate )
    {
        InvalidateMe();
        NotifyInvalidSiblings();
    }
}

bool SwNodeNum::IsCountPhantoms() const
{
    bool bResult = true;

    if (mpNumRule)
        bResult = mpNumRule->IsCountPhantoms();

    return bResult;
}

SwNumberTreeNode::tSwNumTreeNumber SwNodeNum::GetStart() const
{
    tSwNumTreeNumber aResult = 1;

    if (IsRestart())
        aResult = mnStart;
    else
    {
        SwNumRule * pRule = GetNumRule();

        if (pRule)
        {
            int nLevel = GetLevel();

            if (nLevel >= 0 && nLevel < MAXLEVEL)
            {
                const SwNumFmt * pFmt = pRule->GetNumFmt(nLevel);

                if (pFmt)
                    aResult = pFmt->GetStart();
            }
        }
    }

    return aResult;
}

String SwNodeNum::ToString() const
{
    String aResult("[ ", RTL_TEXTENCODING_ASCII_US);

    if (GetTxtNode())
    {
        char aBuffer[256];

        sprintf(aBuffer, "%p ", GetTxtNode());

        aResult += String(aBuffer, RTL_TEXTENCODING_ASCII_US);
        aResult += String::CreateFromInt32(GetPosition().nNode.GetIndex());
    }
    else
        aResult += String("*", RTL_TEXTENCODING_ASCII_US);

    aResult += String(" ", RTL_TEXTENCODING_ASCII_US);

    unsigned int nLvl = GetLevel();
    aResult += String::CreateFromInt32(nLvl);

    aResult += String(": ", RTL_TEXTENCODING_ASCII_US);

    tNumberVector aNumVector;

    _GetNumberVector(aNumVector, false);

    for (unsigned int n = 0; n < aNumVector.size(); n++)
    {
        if (n > 0)
            aResult += String(", ", RTL_TEXTENCODING_ASCII_US);

        aResult += String::CreateFromInt32(aNumVector[n]);
    }

    if (IsCounted())
//        aResult += String(" counted", RTL_TEXTENCODING_ASCII_US);
        aResult += String(" C", RTL_TEXTENCODING_ASCII_US);

    if (IsRestart())
    {
//        aResult += String(" restart(", RTL_TEXTENCODING_ASCII_US);
        aResult += String(" R(", RTL_TEXTENCODING_ASCII_US);
        aResult += String::CreateFromInt32(GetStart());
        aResult += String(")", RTL_TEXTENCODING_ASCII_US);
    }

    if (! IsValid())
//        aResult += String(" invalid", RTL_TEXTENCODING_ASCII_US);
        aResult += String(" I", RTL_TEXTENCODING_ASCII_US);

    aResult += String(" ]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

void SwNodeNum::SetLevel(unsigned int nLevel)
{
    ASSERT(nLevel >= 0 && nLevel < MAXLEVEL, "illegal level");

    if (mpParent)
    {
        SwNumRule * pRule = GetNumRule();

        if (pRule != mpNumRule || nLevel != GetLevel())
        {
            RemoveMe();

            if (pRule)
                pRule->AddNumber(this, nLevel);
        }
    }
}

const SwNumFmt * SwNodeNum::GetNumFmt() const
{
    const SwNumFmt * pResult = NULL;

    unsigned int nLevel = GetLevel();
    if (mpNumRule && nLevel >= 0 && nLevel < MAXLEVEL)
        pResult = &mpNumRule->Get(nLevel);

    return pResult;
}
