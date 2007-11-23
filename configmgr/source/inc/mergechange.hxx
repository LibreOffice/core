/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mergechange.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:20:18 $
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
    // method that applies changes on a existing subtree
    void applyUpdateWithAdjustment(TreeChangeList & _anUpdate, data::NodeAddress const & _aBaseAddress);
    bool adjustUpdate(TreeChangeList & _anUpdate, data::NodeAddress const & _aBaseAddress);
    void mergeLayer         (TreeChangeList & _aLayer,      ISubtree& _aTree);
    void combineUpdates     (SubtreeChange  const& _anUpdate, SubtreeChange& _aCombinedUpdate);
    void applyLayerUpdate   (TreeChangeList const& _anUpdate, TreeChangeList& _aLayer);

    // -----------------------------------------------------------------------------
    class OStripDefaults : private ChangeTreeModification
    {
        SubtreeChange& m_rParent;
    public:
        OStripDefaults(SubtreeChange& _rSubtree) : m_rParent(_rSubtree) {}

        bool isEmpty() const { return m_rParent.size() == 0; }

        OStripDefaults& strip();

        static bool strip(SubtreeChange& _rSubtree)
        {
            return OStripDefaults(_rSubtree).strip().isEmpty();
        }
    private:
        void stripOne(Change& _rChange);

        virtual void handle(ValueChange& _rValueNode);
        virtual void handle(AddNode& _rAddNode);
        virtual void handle(RemoveNode& _rRemoveNode);
        virtual void handle(SubtreeChange& _rSubtree);
    };

    // -----------------------------------------------------------------------------
} // namespace configmgr

#endif

