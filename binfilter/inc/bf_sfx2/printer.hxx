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
#ifndef _SFX_PRINTER_HXX
#define _SFX_PRINTER_HXX

#include <vcl/print.hxx>
namespace binfilter {
class SfxItemSet;

class SfxFont;
class SfxTabPage;

struct SfxPrinter_Impl;

#define SFX_RANGE_NOTSET	((USHORT)0xFFFF)

#define SFX_PRINTER_PRINTER			 1 	// ohne JOBSETUP => temporaer
#define SFX_PRINTER_JOBSETUP    	 2
#define SFX_PRINTER_OPTIONS      	 4
#define SFX_PRINTER_CHG_ORIENTATION  8
#define SFX_PRINTER_CHG_SIZE 		16
#define SFX_PRINTER_ALL         	31

#define SFX_PRINTER_CHG_ORIENTATION_FLAG  3
#define SFX_PRINTER_CHG_SIZE_FLAG 		  4

// class SfxFontSizeInfo -------------------------------------------------


// class SfxFont ---------------------------------------------------------

class SfxFont
{
private:
    String					aName;
    FontFamily				eFamily;
    FontPitch				ePitch;
    CharSet 				eCharSet;

    SfxFont&				operator=(const SfxFont& rFont); // not implemented

public:
    SfxFont( const FontFamily eFam,
             const String& aName,
             const FontPitch eFontPitch = PITCH_DONTKNOW,
             const CharSet eFontCharSet = RTL_TEXTENCODING_DONTKNOW );
    // ZugriffsMethoden:
    inline const String&	GetName() const { return aName; }
    inline FontFamily		GetFamily() const { return eFamily; }
    inline FontPitch		GetPitch() const { return ePitch; }
    inline CharSet			GetCharSet() const { return eCharSet; }
};

// class SfxPrinter ------------------------------------------------------

class SfxPrinter : public Printer
{
private:
    JobSetup				aOrigJobSetup;
    SfxItemSet*				pOptions;
    SfxPrinter_Impl*		pImpl;
    BOOL					bKnown;


public:
                            SfxPrinter( SfxItemSet *pTheOptions );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const String &rPrinterName );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const JobSetup &rTheOrigJobSetup );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const String &rPrinterName,
                                        const JobSetup &rTheOrigJobSetup );
                            ~SfxPrinter();


    static SfxPrinter*		Create( SvStream &rStream, SfxItemSet *pOptions );
    SvStream&				Store( SvStream &rStream ) const;


    const SfxItemSet&		GetOptions() const { return *pOptions; }
    void					SetOptions( const SfxItemSet &rNewOptions );


    BOOL					IsKnown() const { return bKnown; }
    BOOL					IsOriginal() const { return bKnown; }

    const SfxFont*          GetFontByName( const String &rFontName );

};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
