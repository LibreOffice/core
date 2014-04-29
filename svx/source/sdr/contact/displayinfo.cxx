/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/contact/displayinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <svx/svdobj.hxx>
#include <vcl/gdimtf.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>

#define ALL_GHOSTED_DRAWMODES (DRAWMODE_GHOSTEDLINE|DRAWMODE_GHOSTEDFILL|DRAWMODE_GHOSTEDTEXT|DRAWMODE_GHOSTEDBITMAP|DRAWMODE_GHOSTEDGRADIENT)

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

        // Access to LayerInfos (which layers to process)
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

//////////////////////////////////////////////////////////////////////////////
// eof
