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
#ifndef INCLUDED_SVX_INC_PALETTE_HXX
#define INCLUDED_SVX_INC_PALETTE_HXX

#include <svx/Palette.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svx/SvxColorIconView.hxx>
#include <svx/xtable.hxx>

class SvFileStream;

typedef std::vector< NamedColor > ColorList;

// ASE = Adobe Swatch Exchange

class PaletteASE final : public Palette
{
    bool        mbValidPalette;
    OUString    maFPath;
    OUString    maASEPaletteName;
    ColorList   maColors;

    void        LoadPalette();
    PaletteASE(const PaletteASE&) = default;
public:
    PaletteASE( OUString aFPath, OUString aFName );
    virtual ~PaletteASE() override;

    virtual const OUString&     GetName() override;
    virtual const OUString&     GetPath() override;
    virtual void                LoadColorSet(SvxColorValueSet& rColorSet) override;
    virtual void                LoadColorSet(weld::IconView& pIconView) override;
    virtual std::vector< NamedColor >   GetColorList() override;

    virtual bool                IsValid() override;

    virtual Palette*            Clone() const override;
};

// GPL - this is *not* GNU Public License, but is the Gimp PaLette

class PaletteGPL final : public Palette
{
    bool        mbLoadedPalette;
    bool        mbValidPalette;
    OUString    maFName;
    OUString    maFPath;
    OUString    maGPLPaletteName;
    ColorList   maColors;

    bool        ReadPaletteHeader(SvFileStream& rFileStream);
    void        LoadPaletteHeader();
    void        LoadPalette();
    PaletteGPL(const PaletteGPL&) = default;
public:
    PaletteGPL( OUString aFPath, OUString aFName );
    virtual ~PaletteGPL() override;

    virtual const OUString&     GetName() override;
    virtual const OUString&     GetPath() override;
    virtual void                LoadColorSet(SvxColorValueSet& rColorSet) override;
    virtual void                LoadColorSet(weld::IconView& pIconView) override;
    virtual std::vector< NamedColor >   GetColorList() override;;

    virtual bool                IsValid() override;

    virtual Palette*            Clone() const override;
};

// SOC - Star Office Color-table

class PaletteSOC final : public Palette
{
    bool            mbLoadedPalette;
    OUString        maFPath;
    OUString        maSOCPaletteName;
    XColorListRef   mpColorList;
    PaletteSOC(const PaletteSOC&) = default;
public:
    PaletteSOC( OUString aFPath, OUString aFName );
    virtual ~PaletteSOC() override;

    virtual const OUString&     GetName() override;
    virtual const OUString&     GetPath() override;
    virtual void                LoadColorSet(SvxColorValueSet& rColorSet) override;
    virtual void                LoadColorSet(weld::IconView& pIconView) override;
    virtual std::vector< NamedColor >   GetColorList() override;;

    virtual bool                IsValid() override;

    virtual Palette*            Clone() const override;
};

#endif // INCLUDED_SVX_INC_PALETTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
