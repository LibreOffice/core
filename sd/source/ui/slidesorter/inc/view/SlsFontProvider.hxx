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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSFONTPROVIDER_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSFONTPROVIDER_HXX

#include "tools/SdGlobalResourceContainer.hxx"

#include <memory>
#include <vcl/mapmod.hxx>

namespace vcl { class Font; }

namespace sd { namespace slidesorter { namespace view {

/** This simple singleton class provides fonts that are scaled so that they
    have a fixed height on the given device regardless of its map mode.
*/
class FontProvider
    : public SdGlobalResource
{
public:
    typedef std::shared_ptr<vcl::Font> SharedFontPointer;

    /** Return the single instance of this class.  Throws a RuntimeException
        when no instance can be created.
    */
    static FontProvider& Instance();

    /** Call this method to tell an object to release its currently used
        font.  The next call to GetFont() will then create a new one.
        Typically called after a DataChange event when for instance a system
        font has been modified.
    */
    void Invalidate();

private:
    static FontProvider* mpInstance;

    /** The font that was created by a previous call to GetFont().  It is
        replaced by another one only when GetFont() is called with a device
        with a different map mode or by a call to Invalidate().
    */
    SharedFontPointer maFont;
    /** The map mode for which maFont was created.
    */
    MapMode maMapMode;

    FontProvider();
    virtual ~FontProvider();

    FontProvider (const FontProvider&) = delete;
    FontProvider& operator= (const FontProvider&) = delete;
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
