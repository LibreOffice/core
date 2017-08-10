/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "CommandCategoryListBox.hxx"
#include <svtools/treelistentry.hxx>

#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/theUICategoryDescription.hpp>
#include <vcl/builderfactory.hxx>

#include "dialmgr.hxx"
#include "strings.hrc"
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/make_unique.hxx>

CommandCategoryListBox::CommandCategoryListBox(vcl::Window* pParent, WinBits nStyle)
    : ListBox( pParent, nStyle),
    pFunctionListBox( nullptr )
{
    SetDropDownLineCount(25);
}

VCL_BUILDER_FACTORY(CommandCategoryListBox);

CommandCategoryListBox::~CommandCategoryListBox()
{
    disposeOnce();
}

void CommandCategoryListBox::dispose()
{
    ClearAll();
    pFunctionListBox.clear();
    ListBox::dispose();
}

void CommandCategoryListBox::ClearAll()
{
    //TODO: Handle SfxCfgKind::GROUP_SCRIPTCONTAINER when it gets added to Init
    m_aGroupInfo.clear();
    Clear();
}

void CommandCategoryListBox::Init(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XFrame >& xFrame,
        const OUString& sModuleLongName)
{
    SetUpdateMode(false);
    ClearAll();

    m_xContext = xContext;
    m_xFrame = xFrame;

    m_sModuleLongName = sModuleLongName;
    m_xGlobalCategoryInfo = css::ui::theUICategoryDescription::get( m_xContext );
    m_xModuleCategoryInfo.set(m_xGlobalCategoryInfo->getByName(m_sModuleLongName), css::uno::UNO_QUERY_THROW);
    m_xUICmdDescription   = css::frame::theUICommandDescription::get( m_xContext );

/**** InitModule Start ****/
    try
    {
        css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider(m_xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Sequence< sal_Int16 > lGroups = xProvider->getSupportedCommandGroups();

        sal_Int32 nGroupsLength = lGroups.getLength();
        sal_Int32 nEntryPos;

        if ( nGroupsLength > 0 )
        {
            // Add the category of "All commands"
            nEntryPos = InsertEntry( CuiResId(RID_SVXSTR_ALLFUNCTIONS) );
            m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_ALLFUNCTIONS, 0 ) );
            SetEntryData( nEntryPos, m_aGroupInfo.back().get() );
        }

        // Add the actual categories
        for (sal_Int32 i = 0; i < nGroupsLength; ++i)
        {
            sal_Int16&  rGroupID = lGroups[i];
            OUString    sGroupID = OUString::number(rGroupID);
            OUString    sGroupName;

            try
            {
                m_xModuleCategoryInfo->getByName(sGroupID) >>= sGroupName;
                if (sGroupName.isEmpty())
                    continue;
            }
            catch(const css::container::NoSuchElementException&)
            {
                continue;
            }

            nEntryPos = InsertEntry( sGroupName );
            m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_FUNCTION, rGroupID ) );
            SetEntryData( nEntryPos, m_aGroupInfo.back().get() );
        }


    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
    {}
/**** InitModule End ****/

    SetUpdateMode(true);
    SelectEntryPos(0);
}

void CommandCategoryListBox::FillFunctionsList(const css::uno::Sequence<css::frame::DispatchInformation>& xCommands)
{
    for (const auto & rInfo : xCommands)
    {
        OUString sUIName = MapCommand2UIName(rInfo.Command);

        SvTreeListEntry* pFuncEntry = pFunctionListBox->InsertEntry(sUIName );

        m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SLOT, 0 ) );
        SfxGroupInfo_Impl* pGrpInfo = m_aGroupInfo.back().get();
        pGrpInfo->sCommand = rInfo.Command;
        pGrpInfo->sLabel   = sUIName;
        pFuncEntry->SetUserData(pGrpInfo);
    }
}

OUString CommandCategoryListBox::MapCommand2UIName(const OUString& sCommand)
{
    OUString sUIName;
    try
    {
        css::uno::Reference< css::container::XNameAccess > xModuleConf;
        m_xUICmdDescription->getByName(m_sModuleLongName) >>= xModuleConf;
        if (xModuleConf.is())
        {
            ::comphelper::SequenceAsHashMap lProps(xModuleConf->getByName(sCommand));
            sUIName = lProps.getUnpackedValueOrDefault("Name", OUString());
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(css::uno::Exception&)
        { sUIName.clear(); }

    // fallback for missing UINames !?
    if (sUIName.isEmpty())
    {
        sUIName = sCommand;
    }

    return sUIName;
}

void CommandCategoryListBox::categorySelected()
{
    SfxGroupInfo_Impl *pInfo = static_cast<SfxGroupInfo_Impl*>(GetSelectEntryData());
    pFunctionListBox->SetUpdateMode(false);
    pFunctionListBox->ClearAll();

    switch ( pInfo->nKind )
    {
        case SfxCfgKind::GROUP_ALLFUNCTIONS:
        {
            css::uno::Reference< css::frame::XDispatchInformationProvider >
                xProvider( m_xFrame, css::uno::UNO_QUERY );
            sal_Int32 nEntryCount = GetEntryCount();

            for (sal_Int32 nCurPos = 0; nCurPos < nEntryCount; ++nCurPos)
            {
                SfxGroupInfo_Impl *pCurrentInfo =
                    static_cast<SfxGroupInfo_Impl*>(GetEntryData(nCurPos));

                if (pCurrentInfo->nKind == SfxCfgKind::GROUP_FUNCTION)
                {
                    css::uno::Sequence< css::frame::DispatchInformation > lCommands;
                    try
                    {
                        lCommands = xProvider->getConfigurableDispatchInformation(
                                        pCurrentInfo->nUniqueID );
                        FillFunctionsList( lCommands );
                    }
                    catch( css::container::NoSuchElementException& )
                    {
                    }
                }
            }


            break;
        }
        case SfxCfgKind::GROUP_FUNCTION:
        {
            sal_uInt16 nGroup = pInfo->nUniqueID;
            css::uno::Reference< css::frame::XDispatchInformationProvider >
                xProvider (m_xFrame, css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::frame::DispatchInformation > lCommands =
                xProvider->getConfigurableDispatchInformation(nGroup);
            FillFunctionsList( lCommands );
            break;
        }
        case SfxCfgKind::GROUP_SCRIPTCONTAINER:
        {
            //TODO:Implement
            break;
        }
        case SfxCfgKind::GROUP_STYLES:
        {
            //TODO:Implement
            break;
        }
        default:
            break;
    }

    if ( pFunctionListBox->GetEntryCount() )
        pFunctionListBox->Select( pFunctionListBox->GetEntry( nullptr, 0 ) );

    pFunctionListBox->SetUpdateMode(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
