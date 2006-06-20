/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stgavl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:54:06 $
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

#ifndef _STGAVL_HXX
#define _STGAVL_HXX

#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif

// This class must be overloaded to define real, living nodes.
// Especially, the compare function must be implemented.

class StgAvlNode
{
    friend class StgAvlIterator;
private:
    short Locate( StgAvlNode*, StgAvlNode**, StgAvlNode**, StgAvlNode** );
    short Adjust( StgAvlNode**, StgAvlNode* );
    StgAvlNode* RotLL();
    StgAvlNode* RotLR();
    StgAvlNode* RotRR();
    StgAvlNode* RotRL();
    void   StgEnum( short& );
    static StgAvlNode* Rem( StgAvlNode**, StgAvlNode*, BOOL );
protected:
    short nId;                          // iterator ID
    short nBalance;                     // indicates tree balance
    StgAvlNode* pLeft, *pRight;         // leaves
    StgAvlNode();
public:
    virtual ~StgAvlNode();
    StgAvlNode* Find( StgAvlNode* );
    static BOOL Insert( StgAvlNode**, StgAvlNode* );
    static BOOL Remove( StgAvlNode**, StgAvlNode*, BOOL bDel = TRUE );
    static BOOL Move( StgAvlNode**, StgAvlNode**, StgAvlNode* );
    virtual short Compare( const StgAvlNode* ) const = 0;
};

// The iterator class provides single stepping through an AVL tree.

class StgAvlIterator {
    StgAvlNode* pRoot;                  // root entry (parent)
    short       nCount;                 // tree size
    short       nCur;                   // current element
    StgAvlNode* Find( short );
public:
    StgAvlIterator( StgAvlNode* );
    StgAvlNode* First();
    StgAvlNode* Last();
    StgAvlNode* Next();
    StgAvlNode* Prev();
};

#endif
