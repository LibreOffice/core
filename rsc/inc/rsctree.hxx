/*************************************************************************
 *
 *  $RCSfile: rsctree.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-03 17:24:07 $
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
#ifndef _RSCTREE_HXX
#define _RSCTREE_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _RSCTOOLS_HXX
#include <rsctools.hxx>
#endif

/****************** C L A S S E S ****************************************/
class BiNode
{
protected:
    BiNode*     pLeft;    // left subtree
    BiNode*     pRight;   // right subtree

public:

                         // Wandelt eine doppelt verkettete Liste in
                         // einen binaeren Baum um
            BiNode *    ChangeDLListBTree( BiNode * pList );

                        BiNode();
    virtual             ~BiNode();


                        // Wandelt einen binaeren Baum in eine doppelt
                        // verkettete Liste um
                        BiNode* ChangeBTreeDLList();

            BiNode *    Left() const { return pLeft  ; };
            BiNode *    Right() const{ return pRight ; };
            void        EnumNodes( Link aLink ) const;
};

/*************************************************************************/
class NameNode : public BiNode
{
    void                SubOrderTree( NameNode * pOrderNode );

protected:
                        // pCmp ist Zeiger auf Namen
            NameNode*   Search( const void * pCmp ) const;

public:
            NameNode*   Left() const { return (NameNode *)pLeft  ; };
            NameNode*   Right() const{ return (NameNode *)pRight ; };
            NameNode*   Search( const NameNode * pName ) const;
                        // insert a new node in the b-tree
            BOOL        Insert( NameNode * pTN, sal_uInt32 * nDepth );
            BOOL        Insert( NameNode* pTN );
    virtual COMPARE     Compare( const NameNode * ) const;
    virtual COMPARE     Compare( const void * ) const;
            NameNode*   SearchParent( const NameNode * ) const;
                        // return ist neue Root
            NameNode*   Remove( NameNode * );
            void        OrderTree();
            BOOL        IsOrderTree() const;

};

/*************************************************************************/
class IdNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const;
    virtual COMPARE Compare( const void * ) const;

public:

    IdNode*         Search( sal_uInt32 nTypName ) const;
    virtual sal_uInt32  GetId() const;
};

/*************************************************************************/
class StringNode : public NameNode
{
    virtual COMPARE Compare( const NameNode * ) const;
    virtual COMPARE Compare( const void * ) const;

protected:
    ByteString      aName;

public:
                    StringNode(){};
                    StringNode( const ByteString & rStr ) { aName = rStr; }

    StringNode*     Search( const char * ) const;
    ByteString      GetName() const { return aName; }
};

#endif // _RSCTREE_HXX
