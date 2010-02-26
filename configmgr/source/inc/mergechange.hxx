/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_CONFIGMGR_MERGECHANGE_HXX
#define INCLUDED_CONFIGMGR_MERGECHANGE_HXX

#include "namecreator.hxx"
#include "change.hxx"
#include "treechangelist.hxx"

namespace configmgr
{
    // method that applies changes on a existing subtree
    void combineUpdates     (SubtreeChange  const& _anUpdate, SubtreeChange& _aCombinedUpdate);

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

