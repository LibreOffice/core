/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
