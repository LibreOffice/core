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
#include <svx/dialmgr.hxx>
#include <vcl/toolbox.hxx>
#include <svtools/colrdlg.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <stack>
#include <set>

PaletteManager::PaletteManager() :
    mnMaxRecentColors(Application::GetSettings().GetStyleSettings().GetColorValueSetColumnCount()),
    mnNumOfPalettes(2),
    mnCurrentPalette(0),
    mnColorCount(0),
    mpBtnUpdater(nullptr),
    mLastColor(COL_AUTO),
    maColorSelectFunction(PaletteManager::DispatchColorCommand)
{
    LoadPalettes();
    mnNumOfPalettes += m_Palettes.size();
}

PaletteManager::~PaletteManager()
{
}

void PaletteManager::LoadPalettes()
{
    m_Palettes.clear();
    OUString aPalPaths = SvtPathOptions().GetPalettePath();

    std::stack<OUString> aDirs;
    sal_Int32 nIndex = 0;
    do
    {
        aDirs.push(aPalPaths.getToken(0, ';', nIndex));
    }
    while (nIndex >= 0);

    std::set<OUString> aNames;
    //try all entries palette path list user first, then
    //system, ignoring duplicate file names
    while (!aDirs.empty())
    {
        OUString aPalPath = aDirs.top();
        aDirs.pop();

        osl::Directory aDir(aPalPath);
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
                    if (aNames.find(aFName) == aNames.end())
                    {
                        std::unique_ptr<Palette> pPalette;
                        if( aFName.endsWithIgnoreAsciiCase(".gpl") )
                            pPalette.reset(new PaletteGPL(aFileStat.getFileURL(), aFName));
                        else if( aFName.endsWithIgnoreAsciiCase(".soc") )
                            pPalette.reset(new PaletteSOC(aFileStat.getFileURL(), aFName));
                        else if ( aFName.endsWithIgnoreAsciiCase(".ase") )
                            pPalette.reset(new PaletteASE(aFileStat.getFileURL(), aFName));

                        if( pPalette && pPalette->IsValid() )
                            m_Palettes.push_back( std::move(pPalette) );
                        aNames.insert(aFName);
                    }
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
        XColorListRef pColorList;

        if ( pDocSh )
        {
            const SfxPoolItem* pItem = nullptr;
            if ( nullptr != ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
                pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
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
        std::set<Color> aColors = pDocSh->GetDocColors();
        mnColorCount = aColors.size();
        rColorSet.Clear();
        rColorSet.addEntriesForColorSet(aColors, SVX_RESSTR( RID_SVXSTR_DOC_COLOR_PREFIX ) + " " );
    }
    else
    {
        m_Palettes[mnCurrentPalette-1]->LoadColorSet( rColorSet );
        mnColorCount = rColorSet.GetItemCount();
    }
}

void PaletteManager::ReloadRecentColorSet(SvxColorValueSet& rColorSet)
{
    rColorSet.Clear();
    int nIx = 1;
    for(std::deque<Color>::const_iterator it = maRecentColors.begin();
        it != maRecentColors.end(); ++it)
    {
        rColorSet.InsertItem(nIx, *it, "");
        ++nIx;
    }
}

std::vector<OUString> PaletteManager::GetPaletteList()
{
    std::vector<OUString> aPaletteNames;

    aPaletteNames.push_back( SVX_RESSTR( RID_SVXSTR_DEFAULT_PAL ) );

    for (auto const& it : m_Palettes)
    {
        aPaletteNames.push_back( (*it).GetName() );
    }

    aPaletteNames.push_back( SVX_RESSTR ( RID_SVXSTR_DOC_COLORS ) );

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

long PaletteManager::GetRecentColorCount()
{
    return maRecentColors.size();
}

const Color& PaletteManager::GetLastColor()
{
    return mLastColor;
}

void PaletteManager::SetLastColor(const Color& rLastColor)
{
    mLastColor = rLastColor;
}

void PaletteManager::AddRecentColor(const Color& rRecentColor)
{
    std::deque<Color>::iterator itColor =
        std::find(maRecentColors.begin(), maRecentColors.end(), rRecentColor);
    // if recent color to be added is already in list, remove it
    if( itColor != maRecentColors.end() )
        maRecentColors.erase( itColor );

    maRecentColors.push_front( rRecentColor );
    if( maRecentColors.size() > mnMaxRecentColors )
        maRecentColors.pop_back();
}

void PaletteManager::SetBtnUpdater(svx::ToolboxButtonColorUpdater* pBtnUpdater)
{
    mpBtnUpdater = pBtnUpdater;
}

void PaletteManager::SetColorSelectFunction(const std::function<void(const OUString&, const Color&)>& aColorSelectFunction)
{
    maColorSelectFunction = aColorSelectFunction;
}

void PaletteManager::PopupColorPicker(const OUString& aCommand)
{
    // The calling object goes away during aColorDlg.Execute(), so we must copy this
    OUString aCommandCopy = aCommand;
    SvColorDialog aColorDlg( nullptr );
    aColorDlg.SetColor ( mLastColor );
    aColorDlg.SetMode( svtools::ColorPickerMode_MODIFY );
    if( aColorDlg.Execute() == RET_OK )
    {
        if (mpBtnUpdater)
            mpBtnUpdater->Update( aColorDlg.GetColor() );
        mLastColor = aColorDlg.GetColor();
        AddRecentColor( mLastColor );
        maColorSelectFunction(aCommandCopy, mLastColor);
    }
}

void PaletteManager::DispatchColorCommand(const OUString& aCommand, const Color& rColor)
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
        aArgs[0].Value = makeAny(sal_Int32(rColor.GetColor()));

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
