/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwNodeNum.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 17:11:05 $
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

#ifndef _SW_NODE_NUM_HXX
#define _SW_NODE_NUM_HXX

#include <SwNumberTree.hxx>

class SwTxtNode;
struct SwPosition;
class SwNumRule;
class SwNumFmt;

class SW_DLLPUBLIC SwNodeNum : public SwNumberTreeNode
{
    SwTxtNode * mpTxtNode;
    SwNumRule * mpNumRule;

    tSwNumTreeNumber mnStart;
    bool mbRestart;

protected:
    void SetTxtNode(SwTxtNode * pTxtNode);
    SwTxtNode * GetTxtNode() const;

    void SetNumRule(SwNumRule * pRule);
    SwNumRule * GetNumRule() const;

public:
    SwNodeNum();
    SwNodeNum(const SwNodeNum & rNodeNum);
    virtual ~SwNodeNum();

    virtual SwNumberTreeNode * Create() const;

    virtual SwNumberTreeNode * Copy() const;

    virtual void RemoveChild(SwNumberTreeNode * pChild);

    virtual bool IsNotifiable() const;

    virtual bool IsContinuous() const;

    virtual bool IsCounted() const;

    virtual bool IsCountPhantoms() const;

    virtual void NotifyNode();

    virtual bool LessThan(const SwNumberTreeNode & rNode) const;

    void SetRestart(bool bRestart);

    virtual bool IsRestart() const;

    void SetStart(tSwNumTreeNumber nStart);

    virtual tSwNumTreeNumber GetStart() const;

    String ToString() const;

    void SetLevel(unsigned int nLevel);

    SwPosition GetPosition() const;

    const SwNumFmt * GetNumFmt() const;

    friend class SwTxtNode;
    friend class SwNumRule;
};

#endif // _SW_NODE_NUM_HXX
