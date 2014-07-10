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

// finds first token in rStr from index, separated by whitespace
// returns position of next token in index
OString lcl_getToken(const OString& rStr, sal_Int32& index)
{
    sal_Int32 substart, toklen = 0;

    while(index < rStr.getLength() &&
          (rStr[index] == ' ' || rStr[index] == '\n' || rStr[index] == '\t'))
        ++index;
    if(index == rStr.getLength())
    {
        index = -1;
        return OString();
    }
    substart = index;

    while(index < rStr.getLength() &&
          !(rStr[index] == ' ' || rStr[index] == '\n' || rStr[index] == '\t'))
    {
        ++index;
        ++toklen;
    }

    while(index < rStr.getLength() &&
          (rStr[index] == ' ' || rStr[index] == '\n' || rStr[index] == '\t'))
        ++index;
    if(index == rStr.getLength())
        index = -1;

    return rStr.copy(substart, toklen);
}

Palette::Palette( const OUString &rFPath, const OUString &rFName ) :
    mbLoadedPalette( false ),
    mbValidPalette( false ),
    maFName( rFName ),
    maFPath( rFPath )
{
    LoadPaletteHeader();
}

const OUString& Palette::GetName()
{
    return maName;
}

const Palette::ColorList& Palette::GetPaletteColors()
{
    LoadPalette();
    return maColors;
}

bool Palette::IsValid()
{
    return mbValidPalette;
}

bool Palette::ReadPaletteHeader(SvFileStream& rFileStream)
{
    OString aLine;
    OString aName;

    rFileStream.ReadLine(aLine);
    if( !aLine.startsWith("GIMP Palette") ) return false;
    rFileStream.ReadLine(aLine);
    if( aLine.startsWith("Name: ", &aName) )
    {
        maName = OStringToOUString(aName, RTL_TEXTENCODING_ASCII_US);
        rFileStream.ReadLine(aLine);
        if( aLine.startsWith("Columns: "))
            rFileStream.ReadLine(aLine); // we can ignore this
    }
    else
    {
        maName = maFName;
    }
    return true;
}

//TODO make this LoadPaletteHeader and set a bool if palette is incorrect
void Palette::LoadPaletteHeader()
{
    SvFileStream aFile(maFPath, STREAM_READ);
    mbValidPalette = ReadPaletteHeader( aFile );
}

void Palette::LoadPalette()
{
    if( mbLoadedPalette ) return;
    mbLoadedPalette = true;

    // TODO add error handling!!!
    SvFileStream aFile(maFPath, STREAM_READ);
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
            if(token == "" || nIndex == -1) continue;
            sal_Int32 r = token.toInt32();

            token = lcl_getToken(aLine, nIndex);
            if(token == "" || nIndex == -1) continue;
            sal_Int32 g = token.toInt32();

            token = lcl_getToken(aLine, nIndex);
            if(token == "") continue;
            sal_Int32 b = token.toInt32();

            OString name;
            if(nIndex != -1)
                name = aLine.copy(nIndex);

            maColors.push_back(std::make_pair(Color(r, g, b), name));
        }
    } while (aFile.ReadLine(aLine));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
