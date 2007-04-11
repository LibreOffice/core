/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontcvt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:54:20 $
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

#ifndef _SV_FONTCVT_HXX
#define _SV_FONTCVT_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

// ------------------
// - FontToSubsFont -
// ------------------

#define FONTTOSUBSFONT_IMPORT                   ((ULONG)0x00000001)
#define FONTTOSUBSFONT_EXPORT                   ((ULONG)0x00000002)
#define FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS     ((ULONG)0x00000004)

typedef void* FontToSubsFontConverter;
VCL_DLLPUBLIC FontToSubsFontConverter     CreateFontToSubsFontConverter( const String& rFontName, ULONG nFlags );
VCL_DLLPUBLIC void                        DestroyFontToSubsFontConverter( FontToSubsFontConverter hConverter );
VCL_DLLPUBLIC sal_Unicode                 ConvertFontToSubsFontChar( FontToSubsFontConverter hConverter, sal_Unicode c );
VCL_DLLPUBLIC String                      GetFontToSubsFontName( FontToSubsFontConverter hConverter );

// ----------------
// - SubsFontName -
// ----------------

#define SUBSFONT_ONLYONE    ((ULONG)0x00000001)
#define SUBSFONT_MS         ((ULONG)0x00000002)
#define SUBSFONT_PS         ((ULONG)0x00000004)
#define SUBSFONT_HTML       ((ULONG)0x00000008)
VCL_DLLPUBLIC String GetSubsFontName( const String& rName, ULONG nFlags );

// -----------------
// - FontTokenName -
// -----------------

VCL_DLLPUBLIC String GetFontToken( const String& rName, xub_StrLen nToken, xub_StrLen& rIndex );
inline String GetFontToken( const String& rName, xub_StrLen nToken )
{
    xub_StrLen nTempIndex = 0;
    return GetFontToken( rName, nToken, nTempIndex );
}

// ---------------------------
// - StarSymbolToMSMultiFont -
// ---------------------------

class VCL_DLLPUBLIC StarSymbolToMSMultiFont
{
public:
    //Returns the name of the best windows symbol font which this char can be
    //mapped to. Sets rChar to the correct position for that font. If no
    //match found, then no name is returned, and rChar is unchanged. If you
    //want to convert a string, you don't want to use this.
    virtual String ConvertChar(sal_Unicode &rChar) = 0;

    //Starts converting the string at position rIndex. It converts as much of
    //the string that can be converted to the same symbol font and returns the
    //name of that font. rIndex is modified to the last index that was
    //converted. Typically you call if continously until rIndex ==
    //rString.Len() and handle each section as seperate 8bit strings using
    //seperate fonts. Will return an empty string for a continous section
    //that has no possible mapping.
    virtual String ConvertString(String &rString, xub_StrLen &rIndex) = 0;
    virtual ~StarSymbolToMSMultiFont() {}
};

//with bPerfect set the converter will only try and convert symbols which have
//perfect mappings to the windows symbols fonts. With it not set, it will
//allow somewhat more dubious transformations that are nevertheless
//recognizably similiar. Even in this mode there will be characters that fail.
//The users of this might want to make a distinction between failed characters
//which were inside and those outside the unicode private area.
VCL_DLLPUBLIC StarSymbolToMSMultiFont *CreateStarSymbolToMSMultiFont(bool bPerfectOnly=false);
#endif // _SV_FONTCVT_HXX
