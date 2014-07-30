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
#include <svx/drawitem.hxx>
#include <svx/dialogs.hrc>
#include <svtools/colrdlg.hxx>

#define STR_DEFAULT_PAL         "Default palette"
#define STR_DOC_COLORS          "Document colors"
#define STR_DOC_COLOR_PREFIX    "Document Color "

PaletteManager::PaletteManager() :
    mnNumOfPalettes(2),
    mnCurrentPalette(0),
    mnColorCount(0),
    mLastColor(COL_AUTO)
{
    LoadPalettes();
    mnNumOfPalettes += maPalettes.size();
}

PaletteManager::~PaletteManager()
{
    for( std::vector<Palette*>::iterator it = maPalettes.begin();
         it != maPalettes.end();
         ++it)
    {
        delete *it;
    }
}

void PaletteManager::LoadPalettes()
{
    OUString aPalPath = SvtPathOptions().GetPalettePath();

    osl::Directory aDir(aPalPath);
    maPalettes.clear();
    osl::DirectoryItem aDirItem;
    osl::FileStatus aFileStat( osl_FileStatus_Mask_FileName |
                               osl_FileStatus_Mask_FileURL  |
                               osl_FileStatus_Mask_Type     );
    if( aDir.open() == osl::FileBase::E_None )
    {
        while( aDir.getNextItem(aDirItem) == osl::FileBase::E_None )
        {
            aDirItem.getFileStatus(aFileStat);
            if(aFileStat.isRegular() || aFileStat.isLink())
            {
                OUString aFName = aFileStat.getFileName();
                Palette* pPalette = 0;
                if( aFName.endsWithIgnoreAsciiCase(".gpl") )
                    pPalette = new PaletteGPL( aFileStat.getFileURL(), aFName );
                else if( aFName.endsWithIgnoreAsciiCase(".soc") )
                    pPalette = new PaletteSOC( aFileStat.getFileURL(), aFName );

                if( pPalette && pPalette->IsValid() )
                    maPalettes.push_back( pPalette );
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
        rColorSet.loadColorVector(aColors, STR_DOC_COLOR_PREFIX );
    }
    else
    {
        maPalettes[mnCurrentPalette-1]->LoadColorSet( rColorSet );
        mnColorCount = rColorSet.GetItemCount();
    }
}

std::vector<OUString> PaletteManager::GetPaletteList()
{
    std::vector<OUString> aPaletteNames;

    aPaletteNames.push_back( STR_DEFAULT_PAL );

    for( std::vector<Palette*>::iterator it = maPalettes.begin();
         it != maPalettes.end();
         ++it)
    {
        aPaletteNames.push_back( (*it)->GetName() );
    }

    aPaletteNames.push_back( STR_DOC_COLORS );

    return aPaletteNames;
}

void PaletteManager::SetPalette( sal_Int32 nPos )
{
    mnCurrentPalette = nPos;
}

sal_Int32 PaletteManager::GetPalette()
{
    return mnCurrentPalette;
}

long PaletteManager::GetColorCount()
{
    return mnColorCount;
}

OUString PaletteManager::GetPaletteName()
{
    if( mnCurrentPalette == 0 )
        return OUString( STR_DEFAULT_PAL );
    else if( mnCurrentPalette == mnNumOfPalettes - 1 )
        return OUString( STR_DOC_COLORS );
    else
        return maPalettes[mnCurrentPalette - 1]->GetName();
}

const Color& PaletteManager::GetLastColor()
{
    return mLastColor;
}

void PaletteManager::SetLastColor(const Color& rLastColor)
{
    mLastColor = rLastColor;
}

void PaletteManager::SetBtnUpdater(svx::ToolboxButtonColorUpdater* pBtnUpdater)
{
    mpBtnUpdater = pBtnUpdater;
}

void PaletteManager::PopupColorPicker(const OUString aCommand)
{
    SvColorDialog aColorDlg( 0 );
    aColorDlg.SetColor ( mLastColor );
    aColorDlg.SetMode( svtools::ColorPickerMode_MODIFY );
    if( aColorDlg.Execute() == RET_OK )
    {
        mpBtnUpdater->Update( aColorDlg.GetColor() );
        mLastColor = aColorDlg.GetColor();
        DispatchColorCommand(aCommand, mLastColor);
    }
}

void PaletteManager::DispatchColorCommand(const OUString aCommand, const Color aColor)
{
    using namespace css::uno;
    using namespace css::frame;
    using namespace css::beans;
    using namespace css::util;

    Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
    Reference<XDesktop2> xDesktop = Desktop::create(xContext);
    Reference<XDispatchProvider> xDispatchProvider(xDesktop->getCurrentFrame(), UNO_QUERY );
    if (xDispatchProvider.is())
    {
        INetURLObject aObj( aCommand );

        Sequence<PropertyValue> aArgs(1);
        aArgs[0].Name = aObj.GetURLPath();
        aArgs[0].Value = makeAny(sal_Int32(aColor.GetColor()));

        URL aTargetURL;
        aTargetURL.Complete = aCommand;
        Reference<XURLTransformer> xURLTransformer(URLTransformer::create(comphelper::getProcessComponentContext()));
        xURLTransformer->parseStrict(aTargetURL);

        Reference<XDispatch> xDispatch = xDispatchProvider->queryDispatch(aTargetURL, OUString(), 0);
        if (xDispatch.is())
            xDispatch->dispatch(aTargetURL, aArgs);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
