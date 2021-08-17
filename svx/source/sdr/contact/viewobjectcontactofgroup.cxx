/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sdr/contact/viewobjectcontactofgroup.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdobj.hxx>


using namespace com::sun::star;


namespace sdr::contact
{
        ViewObjectContactOfGroup::ViewObjectContactOfGroup(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfGroup::~ViewObjectContactOfGroup()
        {
        }

        void ViewObjectContactOfGroup::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo, drawinglayer::primitive2d::Primitive2DContainer& rContainer) const
        {
            // check model-view visibility
            if(!isPrimitiveVisible(rDisplayInfo))
                return;

            const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());
            if(!nSubHierarchyCount)
            {
                // draw replacement object for group. This will use ViewContactOfGroup::createViewIndependentPrimitive2DSequence
                // which creates the replacement primitives for an empty group
                ViewObjectContactOfSdrObj::getPrimitive2DSequenceHierarchy(rDisplayInfo, rContainer);
                return;
            }

            const bool bDoGhostedDisplaying(
                GetObjectContact().DoVisualizeEnteredGroup()
                && !GetObjectContact().isOutputToPrinter()
                && GetObjectContact().getActiveViewContact() == &GetViewContact());

            if(bDoGhostedDisplaying)
            {
                rDisplayInfo.ClearGhostedDrawMode();
            }

            // create object hierarchy
            getPrimitive2DSequenceSubHierarchy(rDisplayInfo, rContainer);

            if(bDoGhostedDisplaying)
            {
                rDisplayInfo.SetGhostedDrawMode();
            }
        }

        bool ViewObjectContactOfGroup::isPrimitiveVisibleOnAnyLayer(const SdrLayerIDSet& aLayers) const
        {
            SdrLayerIDSet aObjectLayers;
            getSdrObject().getMergedHierarchySdrLayerIDSet(aObjectLayers);
            aObjectLayers &= aLayers;
            return !aObjectLayers.IsEmpty();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
