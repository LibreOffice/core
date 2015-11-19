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
#ifndef INCLUDED_SW_SOURCE_CORE_LAYOUT_OBJECTFORMATTERLAYFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_LAYOUT_OBJECTFORMATTERLAYFRM_HXX

#include <objectformatter.hxx>

class SwLayoutFrame;

// Format floating screen objects, which are anchored at a given anchor text frame
// and registered at the given page frame.
class SwObjectFormatterLayFrame : public SwObjectFormatter
{
    private:
        // anchor layout frame
        SwLayoutFrame& mrAnchorLayFrame;

        SwObjectFormatterLayFrame( SwLayoutFrame& _rAnchorLayFrame,
                                 const SwPageFrame& _rPageFrame,
                                 SwLayAction* _pLayAction );

        /** method to format all anchored objects, which are registered at
            the page frame, whose 'anchor' isn't on this page frame and whose
            anchor frame is valid.

            OD 2004-07-02 #i28701#

            @return boolean
            indicates, if format was successful
        */
        bool _AdditionalFormatObjsOnPage();

    protected:

        virtual SwFrame& GetAnchorFrame() override;

    public:
        virtual ~SwObjectFormatterLayFrame();

        // #i40147# - add parameter <_bCheckForMovedFwd>.
        // Not relevant for objects anchored at layout frame.
        virtual bool DoFormatObj( SwAnchoredObject& _rAnchoredObj,
                                  const bool _bCheckForMovedFwd = false ) override;
        virtual bool DoFormatObjs() override;

        static SwObjectFormatterLayFrame* CreateObjFormatter(
                                                SwLayoutFrame& _rAnchorLayFrame,
                                                const SwPageFrame& _rPageFrame,
                                                SwLayAction* _pLayAction );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
