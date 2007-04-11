/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: printer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:26:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFX_PRINTER_HXX
#define _SFX_PRINTER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
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

    void                    EnableRange( USHORT nRange );
    void                    DisableRange( USHORT nRange );
    BOOL                    IsRangeEnabled( USHORT nRange ) const;

    BOOL                    IsKnown() const { return bKnown; }
    BOOL                    IsOriginal() const { return bKnown; }

        using OutputDevice::GetFont;
    USHORT                  GetFontCount();
    const SfxFont*          GetFont( USHORT nNo ) const;
    const SfxFont*          GetFontByName( const String &rFontName );

    BOOL                    InitJob( Window* pUIParent, BOOL bAskAboutTransparentObjects );
};

#endif
