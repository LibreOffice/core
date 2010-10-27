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

#ifndef SD_SLIDESORTER_VIEW_FONT_PROVIDER_HXX
#define SD_SLIDESORTER_VIEW_FONT_PROVIDER_HXX

#include "tools/SdGlobalResourceContainer.hxx"

#include <boost/shared_ptr.hpp>
#include <vcl/mapmod.hxx>

class Font;
class OutputDevice;
class VclWindowEvent;

namespace sd { namespace slidesorter { namespace view {

/** This simple singleton class provides fonts that are scaled so that they
    have a fixed height on the given device regardless of its map mode.
*/
class FontProvider
    : public SdGlobalResource
{
public:
    typedef ::boost::shared_ptr<Font> SharedFontPointer;

    /** Return the single instance of this class.  Throws a RuntimeException
        when no instance can be created.
    */
    static FontProvider& Instance (void);

    /** Return a font that is scaled according to the current map mode of
        the given device.  Repeated calls with a device, not necessarily the
        same device, with the same map mode will return the same font.  The
        first call with a different map mode will release the old font and
        create a new one that is correctly scaled.
    */
    SharedFontPointer GetFont (const OutputDevice& rDevice);

    /** Call this method to tell an object to release its currently used
        font.  The next call to GetFont() will then create a new one.
        Typically called after a DataChange event when for instance a system
        font has been modified.
    */
    void Invalidate (void);

private:
    static FontProvider* mpInstance;

    /** The font that was created by a previous call to GetFont().  It is
        replaced by another one only when GetFont() is called with a device
        with a different map mode or by a call to Invalidate().
    */
    SharedFontPointer maFont;
    /** The mape mode for which maFont was created.
    */
    MapMode maMapMode;

    FontProvider (void);
    virtual ~FontProvider (void);

    // Copy constructor is not implemented.
    FontProvider (const FontProvider&);
    // Assignment operator is not implemented.
    FontProvider& operator= (const FontProvider&);
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
