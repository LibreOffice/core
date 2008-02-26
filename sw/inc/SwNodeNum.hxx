/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwNodeNum.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:28:27 $
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

    // --> OD 2006-03-07 #131436#
    static void _UnregisterMeAndChildrenDueToRootDelete( SwNodeNum& rNodeNum );
    // <--
protected:
//    void SetTxtNode(SwTxtNode * pTxtNode);

    // --> OD 2006-04-26 #i64010#
    virtual bool HasCountedChildren() const;
    virtual bool IsCountedForNumbering() const;
    // <--

public:
    SwNodeNum();
    // --> OD 2007-10-26 #i83479#
    explicit SwNodeNum( SwTxtNode* pTxtNode );
    explicit SwNodeNum( SwNumRule* pRule );
    // <--
    SwNodeNum( const SwNodeNum & rNodeNum );

    // --> OD 2007-10-26 #i83479# - made public
    SwTxtNode* GetTxtNode() const;
    SwNumRule* GetNumRule() const;
    void SetNumRule(SwNumRule * pRule);
    // <--

    virtual ~SwNodeNum();

    virtual SwNumberTreeNode * Create() const;

    virtual SwNumberTreeNode * Copy() const;

    // --> OD 2007-10-25 #i83479#
    virtual void AddChild(SwNumberTreeNode * pChild, unsigned int nDepth = 0);
    // <--

    virtual void RemoveChild(SwNumberTreeNode * pChild);

    virtual bool IsNotifiable() const;

    virtual bool IsNotificationEnabled() const;

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

//    friend class SwTxtNode;
//    friend class SwNumRule;

    // --> OD 2005-11-16 #i57919# - direct access on <mnStart>, needed for HTML export
    inline const tSwNumTreeNumber GetStartValue() const
    {
        return mnStart;
    }
    // <--

    // --> OD 2006-03-07 #131436#
    // The number tree root node is deleted, when the corresponding numbering
    // rule is deleted. In this situation the number tree should be empty -
    // still registered text nodes aren't allowed. But it is possible, that
    // text nodes of the undo nodes array are still registered. These will be
    // unregistered.
    // Text nodes of the document nodes array aren't allowed to be registered
    // in this situation - this will be asserted.
    static void HandleNumberTreeRootNodeDelete( SwNodeNum& rNodeNum );
    // <--

    /** determines the <SwNodeNum> instance, which is preceding the given text node

        OD 2007-09-06 #i81002#

        @author OD
    */
    const SwNodeNum* GetPrecedingNodeNumOf( const SwTxtNode& rTxtNode ) const;
};

#endif // _SW_NODE_NUM_HXX
