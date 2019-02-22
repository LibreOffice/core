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
#ifndef INCLUDED_SFX2_FRMDESCR_HXX
#define INCLUDED_SFX2_FRMDESCR_HXX

#include <memory>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/sfxsids.hrc>
#include <tools/gen.hxx>
#include <tools/urlobj.hxx>

// The SfxFrame descriptors build a recursive structure, that covers all the
// required data in-order to display the frame document.
// Through a SfxFrameSetDescriptor access is given to the whole underlying
// structure. Due to the nature of the SfxFrames document, the
// SfxFramesSetDescriptor is not only the content of this document, but also
// describes the view on it. The FrameSet is made up of lines, which in turn,
// contains the actual window . A line can be horizontally or vertically
// aligned, from which also the alignment of the FrameSet is given.


enum class ScrollingMode
{
    Yes,
    No,
    Auto
};

#define SIZE_NOT_SET        -1L

class SfxItemSet;

class SFX2_DLLPUBLIC SfxFrameDescriptor
{
    INetURLObject           aURL;
    OUString                aName;
    Size                    aMargin;
    ScrollingMode           eScroll;
    bool                    bHasBorder;
    bool                    bHasBorderSet;
    std::unique_ptr<SfxItemSet> m_pArgs;

public:
                            SfxFrameDescriptor();
                            ~SfxFrameDescriptor();

                            // FileName/URL
    SfxItemSet*             GetArgs();
    const INetURLObject&    GetURL() const
                            { return aURL; }
    void                    SetURL( const OUString& rURL );
    void                    SetActualURL( const OUString& rURL );

                            // FrameName
    const OUString&         GetName() const
                            { return aName; }
    void                    SetName( const OUString& rName )
                            { aName = rName; }

                            // Margin, Scrolling
    const Size&             GetMargin() const
                            { return aMargin; }
    void                    SetMargin( const Size& rMargin )
                            { aMargin = rMargin; }
    ScrollingMode           GetScrollingMode() const
                            { return eScroll; }
    void                    SetScrollingMode( ScrollingMode eMode )
                            { eScroll = eMode; }

                            // FrameBorder
    bool                    HasFrameBorder() const
                            { return bHasBorder; }
    bool                    IsFrameBorderOn() const
                            { return bHasBorder; }
    void                    SetFrameBorder( bool bBorder )
                            { bHasBorder = bBorder; bHasBorderSet = true; }
    bool                    IsFrameBorderSet() const
                            { return bHasBorderSet; }
    void                    ResetBorder()
                            { bHasBorder = false; bHasBorderSet = false; }
};

#endif // INCLUDED_SFX2_FRMDESCR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
