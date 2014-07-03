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

#include <svx/PaletteManager.hxx>
#include <osl/file.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/objsh.hxx>
#include "svx/drawitem.hxx"
#include <svx/dialogs.hrc>

PaletteManager::PaletteManager() :
    mnNumOfPalettes(2),
    mnCurrentPalette(0),
    mnColorCount(0)
{
    LoadPalettes();
    mnNumOfPalettes += maPalettes.size();
}

void PaletteManager::LoadPalettes()
{
    OUString aPalPath = SvtPathOptions().GetPalettePath();

    osl::Directory aDir(aPalPath);
    maPalettes.clear();
    osl::DirectoryItem aDirItem;
    osl::FileStatus aFileStat(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);
    if( aDir.open() == osl::FileBase::E_None )
    {
        while( aDir.getNextItem(aDirItem) == osl::FileBase::E_None )
        {
            aDirItem.getFileStatus(aFileStat);
            if(aFileStat.isRegular() || aFileStat.isLink())
            {
                OUString aPath = aFileStat.getFileURL();
                if(aPath.getLength() > 4 &&
                    aPath.copy(aPath.getLength()-4).toAsciiLowerCase() == ".gpl")
                {
                    maPalettes.push_back(Palette(aPath));
                }
            }
        }
    }
}

void PaletteManager::ReloadColorSet(SvxColorValueSet &rColorSet)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();

    if( mnCurrentPalette == 0 )
    {
        const SfxPoolItem* pItem = NULL;
        XColorListRef pColorList;

        if ( pDocSh )
        {
            if ( 0 != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
                pColorList = ( (SvxColorListItem*)pItem )->GetColorList();
        }

        if ( !pColorList.is() )
            pColorList = XColorList::CreateStdColorList();


        if ( pColorList.is() )
        {
            mnColorCount = pColorList->Count();
            rColorSet.Clear();
            rColorSet.addEntriesForXColorList(*pColorList);
        }
    }
    else if( mnCurrentPalette == mnNumOfPalettes - 1 )
    {
        // Add doc colors to palette
        std::vector<Color> aColors = pDocSh->GetDocColors();
        mnColorCount = aColors.size();
        rColorSet.Clear();
        rColorSet.loadColorVector(aColors, "Document Color ");
    }
    else
    {
        Palette& rPal = maPalettes[mnCurrentPalette-1];
        mnColorCount = rPal.GetPaletteColors().size();
        rColorSet.Clear();
        rColorSet.loadPalette(rPal);
    }
}

void PaletteManager::PrevPalette()
{
    mnCurrentPalette = mnCurrentPalette == 0 ? mnNumOfPalettes - 1 : mnCurrentPalette - 1;
}

void PaletteManager::NextPalette()
{
    mnCurrentPalette = mnCurrentPalette == mnNumOfPalettes - 1 ? 0 : mnCurrentPalette + 1;
}

long PaletteManager::GetColorCount()
{
    return mnColorCount;
}

OUString PaletteManager::GetPaletteName()
{
    if( mnCurrentPalette == 0 )
        return OUString("Default palette");
    else if( mnCurrentPalette == mnNumOfPalettes - 1 )
        return OUString("Document colors");
    else
        return OStringToOUString(maPalettes[mnCurrentPalette - 1].GetPaletteName(), RTL_TEXTENCODING_ASCII_US);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
