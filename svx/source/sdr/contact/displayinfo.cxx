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

#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdobj.hxx>
#include <vcl/gdimtf.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        DisplayInfo::DisplayInfo()
        :   maProcessLayers(true), // init layer info with all bits set to draw everything on default
            maRedrawArea(),
            mbControlLayerProcessingActive(false),
            mbPageProcessingActive(true),
            mbGhostedDrawModeActive(false),
            mbSubContentActive(false)
        {
        }

        DisplayInfo::~DisplayInfo()
        {
        }

        // Access to LayerInfos (which layers to proccess)
        void DisplayInfo::SetProcessLayers(const SetOfByte& rSet)
        {
            maProcessLayers = rSet;
        }

        // access to RedrawArea
        void DisplayInfo::SetRedrawArea(const Region& rRegion)
        {
            maRedrawArea = rRegion;
        }

        void DisplayInfo::SetControlLayerProcessingActive(bool bDoProcess)
        {
            if((bool)mbControlLayerProcessingActive != bDoProcess)
            {
                mbControlLayerProcessingActive = bDoProcess;
            }
        }

        void DisplayInfo::SetPageProcessingActive(bool bDoProcess)
        {
            if((bool)mbPageProcessingActive != bDoProcess)
            {
                mbPageProcessingActive = bDoProcess;
            }
        }

        void DisplayInfo::ClearGhostedDrawMode()
        {
            mbGhostedDrawModeActive = false;
        }

        void DisplayInfo::SetGhostedDrawMode()
        {
            mbGhostedDrawModeActive = true;
        }

        void DisplayInfo::SetSubContentActive(bool bNew)
        {
            if((bool)mbSubContentActive != bNew)
            {
                mbSubContentActive = bNew;
            }
        }

    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
