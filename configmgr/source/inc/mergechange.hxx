/*************************************************************************
 *
 *  $RCSfile: mergechange.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dg $ $Date: 2001-09-18 19:11:44 $
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

#ifndef INCLUDED_CONFIGMGR_MERGECHANGE_HXX
#define INCLUDED_CONFIGMGR_MERGECHANGE_HXX

#ifndef INCLUDED_CONFIGMGR_NAMECREATOR_HXX
#include "namecreator.hxx"
#endif

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_TREECHANGELIST_HXX
#include "treechangelist.hxx"
#endif

namespace configmgr
{
    // -----------------------------------------------------------------------------

    class OMergeTreeChangeList : private ChangeTreeAction, private OPathCreator<AbsolutePath>
    {
        TreeChangeList &m_aTreeChangeList;       // ChangeList, which will be grown
        SubtreeChange *m_pCurrentParent;         // our current position

        // ------- Helper for Path stack -------
        SubtreeChange* pushTree(SubtreeChange& _rTree);
        void popTree(SubtreeChange* _pSaveTree);
    public:
        // CTor
        OMergeTreeChangeList(TreeChangeList& _aTree);

        // start function, with the Change we want to do.
        // WARNING this could be a big tree, because a change can contain subtreechanges!
        void mergeChanges(TreeChangeList const&_rList);

    private:
        void initRoot(TreeChangeList const& _aChanges);

    private:
        virtual void handle(ValueChange const& _rValueNode);
        virtual void handle(AddNode const& _rAddNode);
        virtual void handle(RemoveNode const& _rRemoveNode);
        virtual void handle(SubtreeChange const& _rSubtree);
    };

    // method that applies changes on a existing subtree
    void applyChanges(TreeChangeList & _aTreeChangeList, ISubtree& _aSubtree);

    // -----------------------------------------------------------------------------
    class OMergeChanges : private ChangeTreeAction, private OPathCreator<RelativePath>
    {
        SubtreeChange &m_rSubtreeChange;          // ChangeList, which will be grown
        SubtreeChange *m_pCurrentParent;          // our current position

        typedef configuration::RelativePath RelativePath;
        // ------- Helper for Path stack -------
        SubtreeChange* pushTree(SubtreeChange& _rTree);
        void popTree(SubtreeChange* _pSaveTree);

    public:
        // CTor
        OMergeChanges(SubtreeChange& _rTree);

        // start function, with the Change we want to do.
        // WARNING this could be a big tree, because a change can contain subtreechanges!
        void mergeChanges(const SubtreeChange &_rChange, const RelativePath& _aPathToChange);
        void mergeChanges(const SubtreeChange &_rChange);

    private:
        void initRoot(const SubtreeChange &_rRootChange, const RelativePath& _aPathToChange);
    private:
        virtual void handle(ValueChange const& _rValueNode);
        virtual void handle(AddNode const& _rAddNode);
        virtual void handle(RemoveNode const& _rRemoveNode);
        virtual void handle(SubtreeChange const& _rSubtree);

    };
} // namespace configmgr

#endif

