/*************************************************************************
 *
 *  $RCSfile: mergechange.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2001-01-17 15:02:30 $
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

#include "change.hxx"
#include "treeprovider.hxx"

namespace configmgr
{

    class ONameCreator
    {
        std::vector<rtl::OUString> m_aNameList;
    public:
        ONameCreator(){}
        void init(const ConfigurationName &_aName);
        rtl::OUString createName(const rtl::OUString &aPlusName);
        void pushName(const rtl::OUString &aName);
        void popName();
    };

    // -----------------------------------------------------------------------------

    // Helperclass to search a change in a given changetree
    class OMergeSearchChange : private ChangeTreeModification, public ONameCreator
    {
        rtl::OUString              m_aSearchName;
        bool                       m_bFound;
        Change*                    m_pFoundChange;

    protected:
        bool isFound() { return m_bFound; }
        virtual void handle(ValueChange& _rValueNode);
        virtual void handle(AddNode& _rAddNode);
        virtual void handle(RemoveNode& _rRemoveNode);
        virtual void handle(SubtreeChange& _rSubtree);

    public:
        OMergeSearchChange(const rtl::OUString &_aName);
        Change* searchForChange(Change &aChange);
    };

    // -----------------------------------------------------------------------------
    class OMergeTreeChangeList : private ChangeTreeAction, public ONameCreator
    {
        TreeChangeList &m_aTreeChangeList;       // ChangeList, which will be grown
        SubtreeChange *m_pCurrentParent;         // our current position
        vector<SubtreeChange*> m_aTreePathStack; // how the name says, a stack for the given path


        // ------- Helper for Path stack -------
        void pushTree(SubtreeChange* _pTree);
        void popTree();
    public:
        // CTor
        OMergeTreeChangeList(TreeChangeList& _aTree);
        SubtreeChange* check(const ConfigurationName &_aName);

        // start function, with the Change we want to do.
        // WARNING this could be a big tree, because a change can contain subtreechanges!
        void handleChange(TreeChangeList &_rList);


    private:
        virtual void handle(ValueChange const& _rValueNode);
        virtual void handle(AddNode const& _rAddNode);
        virtual void handle(RemoveNode const& _rRemoveNode);
        virtual void handle(SubtreeChange const& _rSubtree);
    };

    // -----------------------------------------------------------------------------
    class OMergeChanges : private ChangeTreeAction, public ONameCreator
    {
        SubtreeChange &m_aSubtreeChange;          // ChangeList, which will be grown
        SubtreeChange *m_pCurrentParent;          // our current position
        vector<SubtreeChange*> m_aTreePathStack;  // how the name says, a stack for the given path

        // ------- Helper for Path stack -------
        void pushTree(SubtreeChange* _pTree);
        void popTree();

    public:
        // CTor
        OMergeChanges(SubtreeChange& _aTree);

        SubtreeChange* check(const ConfigurationName &_aName);

        // start function, with the Change we want to do.
        // WARNING this could be a big tree, because a change can contain subtreechanges!
        void handleChange(const SubtreeChange &_rList, const rtl::OUString &_aPathToRoot);

    private:
        virtual void handle(ValueChange const& _rValueNode);
        virtual void handle(AddNode const& _rAddNode);
        virtual void handle(RemoveNode const& _rRemoveNode);
        virtual void handle(SubtreeChange const& _rSubtree);

    };
} // namespace configmgr
