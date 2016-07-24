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

#include <svx/Palette.hxx>
#include <tools/stream.hxx>


Palette::~Palette()
{
}

PaletteASE::~PaletteASE()
{
}

PaletteASE::PaletteASE( const OUString &rFPath, const OUString &rFName ) :
    mbValidPalette( false ),
    maFPath ( rFPath ),
    maASEPaletteName  ( rFName )
{
    LoadPalette();
}

void PaletteASE::LoadColorSet( SvxColorValueSet& rColorSet )
{
    rColorSet.Clear();
    int nIx = 1;
    for (ColorList::const_iterator it = maColors.begin(); it != maColors.end(); ++it)
    {
        rColorSet.InsertItem(nIx, it->first, it->second);
        ++nIx;
    }
}

const OUString& PaletteASE::GetName()
{
    return maASEPaletteName;
}

const OUString& PaletteASE::GetPath()
{
    return maFPath;
}

bool PaletteASE::IsValid()
{
    return mbValidPalette;
}

// CMYK values from 0 to 1
// TODO: Deduplicate me (taken from core/cui/source/dialogs/colorpicker.cxx)
static void lcl_CMYKtoRGB( float fCyan, float fMagenta, float fYellow, float fKey, float& dR, float& dG, float& dB )
{
    fCyan = (fCyan * ( 1.0 - fKey )) + fKey;
    fMagenta = (fMagenta * ( 1.0 - fKey )) + fKey;
    fYellow = (fYellow * ( 1.0 - fKey )) + fKey;

    dR = std::max( std::min( ( 1.0 - fCyan ), 1.0), 0.0 );
    dG = std::max( std::min( ( 1.0 - fMagenta ), 1.0), 0.0 );
    dB = std::max( std::min( ( 1.0 - fYellow ), 1.0), 0.0 );
}

void PaletteASE::LoadPalette()
{
    SvFileStream aFile(maFPath, StreamMode::READ);
    aFile.SetEndian(SvStreamEndian::BIG);

    // Verify magic first 4 characters
    sal_Char cMagic[5] = {0};
    if ((aFile.ReadBytes(cMagic, 4) != 4) || (strncmp(cMagic, "ASEF", 4) != 0))
    {
        mbValidPalette = false;
        return;
    }

    // Ignore the version number
    aFile.SeekRel(4);

    sal_uInt32 nBlocks = 0;
    aFile.ReadUInt32(nBlocks);
    for (sal_uInt32 nI = 0; nI < nBlocks; nI++) {
        sal_uInt32 nChunkType = 0;
        aFile.ReadUInt32(nChunkType);
        // End chunk
        if (nChunkType == 0)
           break;

        // Grab chunk size, name length
        sal_uInt16 nChunkSize = 0;
        sal_uInt16 nChars = 0;
        aFile.ReadUInt16(nChunkSize);
        aFile.ReadUInt16(nChars);

        OUString aPaletteName("");
        if (nChars > 1)
            aPaletteName = read_uInt16s_ToOUString(aFile, nChars);
        else
            aFile.SeekRel(2);

        if (nChunkType == 0xC0010000)
        {
            // Got a start chunk, so set palette name
            maASEPaletteName = aPaletteName;
            // Is there color data? (shouldn't happen in a start block, but check anyway)
            if (nChunkSize > ((nChars * 2) + 2))
                aPaletteName.clear();
            else
                continue;
        }

        sal_Char cColorModel[5] = {0};
        aFile.ReadBytes(cColorModel, 4);
        OString aColorModel(cColorModel);
        // r, g, and b are floats ranging from 0 to 1
        float r = 0, g = 0, b = 0;

        if (aColorModel.equalsIgnoreAsciiCase("cmyk"))
        {
            float c = 0, m = 0, y = 0, k = 0;
            aFile.ReadFloat(c);
            aFile.ReadFloat(m);
            aFile.ReadFloat(y);
            aFile.ReadFloat(k);
            lcl_CMYKtoRGB(c, m, y, k, r, g, b);
        }
        else if (aColorModel.equalsIgnoreAsciiCase("rgb "))
        {
            aFile.ReadFloat(r);
            aFile.ReadFloat(g);
            aFile.ReadFloat(b);
        }
        else if (aColorModel.equalsIgnoreAsciiCase("gray"))
        {
            float nVal = 0;
            aFile.ReadFloat(nVal);
            r = g = b = nVal;
        }
        else
        {
            float nL = 0, nA = 0, nB = 0;
            aFile.ReadFloat(nL);
            aFile.ReadFloat(nA);
            aFile.ReadFloat(nB);
            // TODO: How to convert LAB to RGB?
            r = g = b = 0;
        }

        // Ignore color type
        aFile.SeekRel(2);
        maColors.push_back(std::make_pair(Color(r * 255, g * 255, b * 255), aPaletteName));
    }

    mbValidPalette = true;
}

// PaletteGPL ------------------------------------------------------------------

OString lcl_getToken(const OString& rStr, sal_Int32& index);

PaletteGPL::PaletteGPL( const OUString &rFPath, const OUString &rFName ) :
    mbLoadedPalette( false ),
    mbValidPalette( false ),
    maFName( rFName ),
    maFPath( rFPath )
{
    LoadPaletteHeader();
}

PaletteGPL::~PaletteGPL()
{
}

const OUString& PaletteGPL::GetName()
{
    return maGPLPaletteName;
}

const OUString& PaletteGPL::GetPath()
{
    return maFPath;
}

void PaletteGPL::LoadColorSet( SvxColorValueSet& rColorSet )
{
    LoadPalette();

    rColorSet.Clear();
    int nIx = 1;
    for (ColorList::const_iterator it = maColors.begin(); it != maColors.end(); ++it)
    {
        rColorSet.InsertItem(nIx, it->first, it->second);
        ++nIx;
    }
}

bool PaletteGPL::IsValid()
{
    return mbValidPalette;
}

bool PaletteGPL::ReadPaletteHeader(SvFileStream& rFileStream)
{
    OString aLine;
    OString aPaletteName;

    rFileStream.ReadLine(aLine);
    if( !aLine.startsWith("GIMP Palette") ) return false;
    rFileStream.ReadLine(aLine);
    if( aLine.startsWith("Name: ", &aPaletteName) )
    {
        maGPLPaletteName = OStringToOUString(aPaletteName, RTL_TEXTENCODING_ASCII_US);
        rFileStream.ReadLine(aLine);
        if( aLine.startsWith("Columns: "))
            rFileStream.ReadLine(aLine); // we can ignore this
    }
    else
    {
        maGPLPaletteName = maFName;
    }
    return true;
}

void PaletteGPL::LoadPaletteHeader()
{
    SvFileStream aFile(maFPath, StreamMode::READ);
    mbValidPalette = ReadPaletteHeader( aFile );
}

void PaletteGPL::LoadPalette()
{
    if( mbLoadedPalette ) return;
    mbLoadedPalette = true;

    // TODO add error handling!!!
    SvFileStream aFile(maFPath, StreamMode::READ);
    mbValidPalette = ReadPaletteHeader( aFile );

    if( !mbValidPalette ) return;

    OString aLine;
    do {
        if (aLine[0] != '#' && aLine[0] != '\n')
        {
            // TODO check if r,g,b are 0<= x <=255, or just clamp?
            sal_Int32 nIndex = 0;
            OString token;

            token = lcl_getToken(aLine, nIndex);
            if(token.isEmpty() || nIndex == -1) continue;
            sal_Int32 r = token.toInt32();

            token = lcl_getToken(aLine, nIndex);
            if(token.isEmpty() || nIndex == -1) continue;
            sal_Int32 g = token.toInt32();

            token = lcl_getToken(aLine, nIndex);
            if(token.isEmpty()) continue;
            sal_Int32 b = token.toInt32();

            OString name;
            if(nIndex != -1)
                name = aLine.copy(nIndex);

            maColors.push_back(std::make_pair(
                Color(r, g, b),
                OStringToOUString(name, RTL_TEXTENCODING_ASCII_US)));
        }
    } while (aFile.ReadLine(aLine));
}

// finds first token in rStr from index, separated by whitespace
// returns position of next token in index
OString lcl_getToken(const OString& rStr, sal_Int32& index)
{
    sal_Int32 substart, toklen = 0;
    OUString aWhitespaceChars( " \n\t" );

    while(index < rStr.getLength() &&
            aWhitespaceChars.indexOf( rStr[index] ) != -1)
        ++index;
    if(index == rStr.getLength())
    {
        index = -1;
        return OString();
    }
    substart = index;

    //counts length of token
    while(index < rStr.getLength() &&
            aWhitespaceChars.indexOf( rStr[index] ) == -1 )
    {
        ++index;
        ++toklen;
    }

    //counts to position of next token
    while(index < rStr.getLength() &&
            aWhitespaceChars.indexOf( rStr[index] ) != -1 )
        ++index;
    if(index == rStr.getLength())
        index = -1;

    return rStr.copy(substart, toklen);
}

// PaletteSOC ------------------------------------------------------------------

PaletteSOC::PaletteSOC( const OUString &rFPath, const OUString &rFName ) :
    mbLoadedPalette( false ),
    maFPath( rFPath ),
    maSOCPaletteName( rFName )
{
}

PaletteSOC::~PaletteSOC()
{
}

const OUString& PaletteSOC::GetName()
{
    return maSOCPaletteName;
}

const OUString& PaletteSOC::GetPath()
{
    return maFPath;
}

void PaletteSOC::LoadColorSet( SvxColorValueSet& rColorSet )
{
    if( !mbLoadedPalette )
    {
        mbLoadedPalette = true;
        mpColorList = XPropertyList::AsColorList(XPropertyList::CreatePropertyListFromURL(XCOLOR_LIST, maFPath));
        (void)mpColorList->Load();
    }
    rColorSet.Clear();
    if( mpColorList.is() )
        rColorSet.addEntriesForXColorList( *mpColorList );
}

bool PaletteSOC::IsValid()
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
