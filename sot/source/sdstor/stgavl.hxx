/*************************************************************************
 *
 *  $RCSfile: stgavl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:51 $
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
    void   Enum( short& );
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
