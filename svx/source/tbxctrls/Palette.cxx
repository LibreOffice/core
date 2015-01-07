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


Palette::~Palette()
{
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
    return maName;
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
    maName( rFName )
{
}

PaletteSOC::~PaletteSOC()
{
}

const OUString& PaletteSOC::GetName()
{
    return maName;
}

void PaletteSOC::LoadColorSet( SvxColorValueSet& rColorSet )
{
    if( !mbLoadedPalette )
    {
        mbLoadedPalette = true;
        mpColorList = XPropertyList::AsColorList(XPropertyList::CreatePropertyListFromURL(XCOLOR_LIST, maFPath));
        mpColorList->Load();
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
