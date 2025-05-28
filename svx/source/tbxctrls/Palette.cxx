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

#include <palettes.hxx>
#include <utility>

Palette::~Palette()
{
}

PaletteASE::~PaletteASE()
{
}

PaletteASE::PaletteASE( OUString aFPath, OUString aFName ) :
    mbValidPalette( false ),
    maFPath (std::move( aFPath )),
    maASEPaletteName  (std::move( aFName ))
{
    LoadPalette();
}

void PaletteASE::LoadColorSet(SvxColorValueSet& rColorSet)
{
    rColorSet.Clear();
    int nIx = 1;
    for (const auto& rColor : maColors)
    {
        rColorSet.InsertItem(nIx, rColor.m_aColor, rColor.m_aName);
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

    dR = std::clamp( 1.0 - fCyan, 0.0, 1.0 );
    dG = std::clamp( 1.0 - fMagenta, 0.0, 1.0 );
    dB = std::clamp( 1.0 - fYellow, 0.0, 1.0 );
}

// This function based on code under ALv2 - Copyright 2013 István Ujj-Mészáros
// credit Avisek Das and István Ujj-Mészáros
static void lcl_XYZtoRGB( float fX, float fY, float fZ, float& dR, float& dG, float& dB)
{
    // Observer = 2°, Illuminant = D65
    fX = fX / 100;
    fY = fY / 100;
    fZ = fZ / 100;

    // X from 0 to 95.047
    dR = fX * 3.2406 + fY * -1.5372 + fZ * -0.4986;
    // Y from 0 to 100.000
    dG = fX * -0.9689 + fY * 1.8758 + fZ * 0.0415;
    // Z from 0 to 108.883
    dB = fX * 0.0557 + fY * -0.2040 + fZ * 1.0570;

    if (dR > 0.0031308)
    {
        dR = 1.055 * (std::pow(dR, 0.41666667)) - 0.055;
    }
    else
    {
        dR = 12.92 * dR;
    }

    if (dG > 0.0031308)
    {
        dG = 1.055 * (std::pow(dG, 0.41666667)) - 0.055;
    }
    else
    {
        dG = 12.92 * dG;
    }

    if (dB > 0.0031308)
    {
        dB = 1.055 * (std::pow(dB, 0.41666667)) - 0.055;
    }
    else
    {
        dB = 12.92 * dB;
    }
    dR *= 255;
    dG *= 255;
    dB *= 255;
}

// This function based on code under ALv2 - Copyright 2013 István Ujj-Mészáros
// credit Avisek Das and István Ujj-Mészáros
static void lcl_LABtoXYZ( float fL, float fa, float fb, float& dX, float& dY, float& dZ)
{
    dY = (fL + 16) / 116;
    dX = (fa / 500) + dY;
    dZ = dY - (fb / 200);

    if (std::pow(dY, 3) > 0.008856)
    {
        dY = std::pow(dY, 3);
    }
    else
    {
        dY = (dY - 0.137931034) /  7.787;
    }

    if (std::pow(dX, 3) > 0.008856)
    {
        dX = std::pow(dX, 3);
    }
    else
    {
        dX = (dX - 0.137931034) /  7.787;
    }

    if (std::pow(dZ, 3) > 0.008856)
    {
        dZ = std::pow(dZ, 3);
    }
    else
    {
        dZ = (dZ - 0.137931034) /  7.787;
    }

    // Observer = 2°, Illuminant = D65
    dX = 95.047 * dX;
    dY = 100.000 * dY;
    dZ = 108.883 * dZ;
}

static void lcl_LABtoRGB( float fL, float fa, float fb, float& dR, float& dG, float& dB)
{
    float x, y, z;
    lcl_LABtoXYZ(fL, fa, fb, x, y, z);

    lcl_XYZtoRGB(x, y, z, dR, dG, dB);
}

void PaletteASE::LoadPalette()
{
    SvFileStream aFile(maFPath, StreamMode::READ);
    aFile.SetEndian(SvStreamEndian::BIG);

    // Verify magic first 4 characters
    char cMagic[5] = {0};
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

        OUString aPaletteName(u""_ustr);
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

        char cColorModel[5] = {0};
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
        else if (aColorModel.equalsIgnoreAsciiCase("LAB "))
        {
            float fL = 0, fA = 0, fB = 0;
            aFile.ReadFloat(fL);
            aFile.ReadFloat(fA);
            aFile.ReadFloat(fB);
            lcl_LABtoRGB(fL, fA, fB, r, g, b);
        }

        // Ignore color type
        aFile.SeekRel(2);
        maColors.emplace_back(Color(r * 255, g * 255, b * 255), aPaletteName);
    }

    mbValidPalette = true;
}

// PaletteGPL ------------------------------------------------------------------

static OString lcl_getToken(OStringBuffer& rStr, sal_Int32& index);

PaletteGPL::PaletteGPL( OUString aFPath, OUString aFName ) :
    mbLoadedPalette( false ),
    mbValidPalette( false ),
    maFName(std::move( aFName )),
    maFPath(std::move( aFPath ))
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

void PaletteGPL::LoadColorSet(SvxColorValueSet& rColorSet)
{
    LoadPalette();

    rColorSet.Clear();
    int nIx = 1;
    for (const auto& rColor : maColors)
    {
        rColorSet.InsertItem(nIx, rColor.m_aColor, rColor.m_aName);
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
    std::string_view aPaletteName;

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

    OStringBuffer aLine;
    do {
        if (aLine.isEmpty())
            continue;

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

            std::string_view name;
            if(nIndex != -1)
                name = std::string_view(aLine).substr(nIndex);

            maColors.emplace_back(
                Color(r, g, b),
                OStringToOUString(name, RTL_TEXTENCODING_ASCII_US));
        }
    } while (aFile.ReadLine(aLine));
}

// finds first token in rStr from index, separated by whitespace
// returns position of next token in index
static OString lcl_getToken(OStringBuffer& rStr, sal_Int32& index)
{
    sal_Int32 substart, toklen = 0;
    OUString aWhitespaceChars( u" \n\t"_ustr );

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

    return OString(std::string_view(rStr).substr(substart, toklen));
}

// PaletteSOC ------------------------------------------------------------------

PaletteSOC::PaletteSOC( OUString aFPath, OUString aFName ) :
    mbLoadedPalette( false ),
    maFPath(std::move( aFPath )),
    maSOCPaletteName(std::move( aFName ))
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

void PaletteSOC::LoadColorSet(SvxColorValueSet& rColorSet)
{
    if( !mbLoadedPalette )
    {
        mbLoadedPalette = true;
        mpColorList = XPropertyList::AsColorList(XPropertyList::CreatePropertyListFromURL(XPropertyListType::Color, maFPath));
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
