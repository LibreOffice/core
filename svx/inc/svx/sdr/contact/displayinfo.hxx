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

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#define _SDR_CONTACT_DISPLAYINFO_HXX

#include <sal/types.h>
#include <svx/svdsob.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/region.hxx>
#include <vector>
#include "svx/svxdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPage;

namespace sdr { namespace contact {
    class ViewObjectContact;
}}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC DisplayInfo
        {
        protected:
            // The Layers which shall be processed (visible)
            SetOfByte                                       maProcessLayers;

            // The redraw area, in logical coordinates of OutputDevice. If Region
            // is empty, everything needs to be redrawn
            Region                                          maRedrawArea;

            // bitfield

            // Internal flag to know when the control layer is painted. Default is
            // false. If set to true, painting of the page, page borders and
            // the rasters will be suppressed as if mbPageProcessingActive is set (see there).
            // This flag is set internally from the processing mechanism to avoid double page
            // painting when the control layer needs to be painted as last layer
            unsigned                                        mbControlLayerProcessingActive : 1;

            // Internal flag to decide if page stuff (background, border, MasterPage, grid, etc...)
            // will be processed at all. This flag is user-defined and will not be changed from the
            // processing mechanism. Default is true, thus set to false if PagePainting should be suppressed.
            // For more granular switching page stuff painting on and off, use the according flags at the
            // view (->Is*Visible())
            unsigned                                        mbPageProcessingActive : 1;

            // Internal flag to remember if EnteredGroupDrawMode is active. Default is true
            // since this mode starts activated and gets switched off when reacing
            // the current group level. Should only be changed by instances which do
            // primitive processing
            unsigned                                        mbGhostedDrawModeActive : 1;

            // Internal flag to know if a MasterPage is processed as SubContent of another
            // page. Initialized to false, this should only be changed from the instance which
            // is processing the MasterPage asSubContent and knows what it does
            unsigned                                        mbSubContentActive : 1;

        public:
            // basic constructor.
            DisplayInfo();

            // destructor
            virtual ~DisplayInfo();

            // access to ProcessLayers
            void SetProcessLayers(const SetOfByte& rSet);
            const SetOfByte& GetProcessLayers() const { return maProcessLayers; }

            // access to RedrawArea
            void SetRedrawArea(const Region& rRegion);
            const Region& GetRedrawArea() const { return maRedrawArea; }

            // Access to ControlLayerProcessingActive flag
            void SetControlLayerProcessingActive(bool bDoPaint);
            bool GetControlLayerProcessingActive() const { return mbControlLayerProcessingActive; }

            // Access to PageProcessingActive flag
            void SetPageProcessingActive(bool bDoPaint);
            bool GetPageProcessingActive() const { return mbPageProcessingActive; }

            // Save the original DrawMode from outdev
            void ClearGhostedDrawMode();
            void SetGhostedDrawMode();
            bool IsGhostedDrawModeActive() const { return mbGhostedDrawModeActive; }

            // access to master page painting flag
            bool GetSubContentActive() const { return mbSubContentActive; }
            void SetSubContentActive(bool bNew);
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_DISPLAYINFO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
