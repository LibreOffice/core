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
#ifndef _SFX_PRINTER_HXX
#define _SFX_PRINTER_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif

class SfxFont;
class SfxTabPage;
class SfxItemSet;

struct SfxPrinter_Impl;

#define SFX_RANGE_NOTSET    ((USHORT)0xFFFF)

// class SfxFontSizeInfo -------------------------------------------------

class SfxFontSizeInfo
{
private:
    static USHORT           pStaticSizes[];
    Size*                   pSizes;
    USHORT                  nSizes;
    BOOL                    bScalable;

public:
    SfxFontSizeInfo( const SfxFont& rFont, const OutputDevice& rDevice );
    ~SfxFontSizeInfo();

    BOOL                    HasSize(const Size &rSize) const;
    BOOL                    IsScalable() const { return bScalable; }

    USHORT                  SizeCount() const { return nSizes; }
    const Size&             GetSize( USHORT nNo ) const
                            { return pSizes[nNo]; }
};

// class SfxFont ---------------------------------------------------------

class SFX2_DLLPUBLIC SfxFont
{
private:
    String                  aName;
    FontFamily              eFamily;
    FontPitch               ePitch;
    CharSet                 eCharSet;

    SfxFont&                operator=(const SfxFont& rFont); // not implemented

public:
    SfxFont( const FontFamily eFam,
             const String& aName,
             const FontPitch eFontPitch = PITCH_DONTKNOW,
             const CharSet eFontCharSet = RTL_TEXTENCODING_DONTKNOW );
    // ZugriffsMethoden:
    inline const String&    GetName() const { return aName; }
    inline FontFamily       GetFamily() const { return eFamily; }
    inline FontPitch        GetPitch() const { return ePitch; }
    inline CharSet          GetCharSet() const { return eCharSet; }
};

// class SfxPrinter ------------------------------------------------------

class SFX2_DLLPUBLIC SfxPrinter : public Printer
{
private:
    JobSetup                aOrigJobSetup;
    SfxItemSet*             pOptions;
    SfxPrinter_Impl*        pImpl;
    BOOL                    bKnown;

    SAL_DLLPRIVATE void operator =(SfxPrinter &); // not defined

    SAL_DLLPRIVATE void UpdateFonts_Impl();

public:
                            SfxPrinter( SfxItemSet *pTheOptions );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const String &rPrinterName );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const JobSetup &rTheOrigJobSetup );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const String &rPrinterName,
                                        const JobSetup &rTheOrigJobSetup );
                            SfxPrinter( const SfxPrinter &rPrinter );
                            ~SfxPrinter();

    SfxPrinter*             Clone() const;

    static SfxPrinter*      Create( SvStream &rStream, SfxItemSet *pOptions );
    SvStream&               Store( SvStream &rStream ) const;

    const JobSetup&         GetOrigJobSetup() const { return aOrigJobSetup; }
    void                    SetOrigJobSetup( const JobSetup &rNewJobSetup );

    const SfxItemSet&       GetOptions() const { return *pOptions; }
    void                    SetOptions( const SfxItemSet &rNewOptions );

    BOOL                    IsKnown() const { return bKnown; }
    BOOL                    IsOriginal() const { return bKnown; }

        using OutputDevice::GetFont;
    USHORT                  GetFontCount();
    const SfxFont*          GetFont( USHORT nNo ) const;
    const SfxFont*          GetFontByName( const String &rFontName );
};

#endif
