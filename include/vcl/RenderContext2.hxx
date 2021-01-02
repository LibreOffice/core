/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/color.hxx>

#include <vcl/dllapi.h>
#include <vcl/font.hxx>
#include <vcl/settings.hxx>
#include <vcl/DrawModeFlags.hxx>
#include <vcl/RasterOp.hxx>

#include <memory>

class AllSettings;
class SalGraphics;

class VCL_DLLPUBLIC RenderContext2
{
public:
    RenderContext2();
    virtual ~RenderContext2() {}

    virtual AllSettings const& GetSettings() const;
    virtual void SetSettings(AllSettings const& rSettings);

    bool IsOpaqueLineColor() const;
    Color const& GetLineColor() const;
    virtual void SetLineColor(Color const& rColor = COL_TRANSPARENT);
    void FlagLineColorAsTransparent();
    void FlagLineColorAsOpaque();

    bool IsOpaqueFillColor() const;
    Color const& GetFillColor() const;
    virtual void SetFillColor(Color const& rColor = COL_TRANSPARENT);
    void FlagFillColorAsTransparent();
    void FlagFillColorAsOpaque();

    Color const& GetTextColor() const;
    virtual void SetTextColor(Color const& rColor);

    bool IsOpaqueTextLineColor() const;
    Color GetTextLineColor() const;
    virtual void SetTextLineColor(Color const& rColor = COL_TRANSPARENT);

    bool IsOpaqueTextFillColor() const;
    Color GetTextFillColor() const;
    virtual void SetTextFillColor(Color const& rColor = COL_TRANSPARENT);

    bool IsOpaqueOverlineColor() const;
    Color GetOverlineColor() const;
    virtual void SetOverlineColor(Color const& rColor = COL_TRANSPARENT);

    DrawModeFlags GetDrawMode() const;
    virtual void SetDrawMode(DrawModeFlags nDrawMode);

    RasterOp GetRasterOp() const;
    virtual void SetRasterOp(RasterOp eRasterOp);

protected:
    /** Acquire a graphics device that the output device uses to draw on.

     There is an LRU of OutputDevices that is used to get the graphics. The
     actual creation of a SalGraphics instance is done via the SalFrame
     implementation.

     However, the SalFrame instance will only return a valid SalGraphics
     instance if it is not in use or there wasn't one in the first place. When
     this happens, AcquireGraphics finds the least recently used OutputDevice
     in a different frame and "steals" it (releases it then starts using it).

     If there are no frames to steal an OutputDevice's SalGraphics instance from
     then it blocks until the graphics is released.

     Once it has acquired a graphics instance, then we add the OutputDevice to
     the LRU.

     @returns true if was able to initialize the graphics device, false otherwise.
     */
    virtual bool AcquireGraphics() const = 0;

    /** Release the graphics device, and remove it from the graphics device
     list.

     @param         bRelease    Determines whether to release the fonts of the
                                physically released graphics device.
     */
    virtual void ReleaseGraphics(bool bRelease = true) = 0;

    bool IsInitLineColor() const;
    void SetInitLineColorFlag(bool bFlag);
    bool IsInitFillColor() const;
    void SetInitFillColorFlag(bool bFlag);
    bool IsInitTextColor() const;
    void SetInitTextColorFlag(bool bFlag);

    // TODO these two init functions will need to become private once all related
    // functions are moved out of OutputDevice
    void InitLineColor();
    void InitFillColor();
    void InitTextColor();

    mutable SalGraphics* mpGraphics;
    std::unique_ptr<AllSettings> mxSettings;

    // TODO eventually make these private when all text/font functions migrated from
    // OutputDevice to RenderContext2
    vcl::Font maFont;
    Color maTextLineColor;

private:
    Color maTextColor;
    Color maLineColor;
    Color maFillColor;
    Color maOverlineColor;
    DrawModeFlags mnDrawMode;
    RasterOp meRasterOp;

    mutable bool mbOpaqueLineColor : 1;
    mutable bool mbInitLineColor : 1;
    mutable bool mbOpaqueFillColor : 1;
    mutable bool mbInitFillColor : 1;
    mutable bool mbInitTextColor : 1;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
