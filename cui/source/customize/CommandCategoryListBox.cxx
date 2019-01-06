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

#include <CommandCategoryListBox.hxx>
#include <vcl/treelistentry.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider2.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/theUICategoryDescription.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/browse/theBrowseNodeFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>
#include <vcl/builderfactory.hxx>
#include <vcl/commandinfoprovider.hxx>

// include search util
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <unotools/textsearch.hxx>

#include <dialmgr.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/make_unique.hxx>
#include <i18nutil/searchopt.hxx>
#include <sal/log.hxx>

#include <cfg.hxx> //for SaveInData

CommandCategoryListBox::CommandCategoryListBox(vcl::Window* pParent)
    : ListBox( pParent, WB_BORDER | WB_DROPDOWN | WB_SORT )
    , pStylesInfo( nullptr )
{
    SetDropDownLineCount(25);

    //Initialize search util
    m_searchOptions.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;
    m_searchOptions.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_searchOptions.searchFlag |= (css::util::SearchFlags::REG_NOT_BEGINOFLINE
                                | css::util::SearchFlags::REG_NOT_ENDOFLINE);
}

VCL_BUILDER_FACTORY(CommandCategoryListBox);

CommandCategoryListBox::~CommandCategoryListBox()
{
    disposeOnce();
}

void CommandCategoryListBox::dispose()
{
    ClearAll();
    ListBox::dispose();
}

void CommandCategoryListBox::ClearAll()
{
    // Clear objects from m_aGroupInfo vector to avoid memory leak
    for (const auto & It : m_aGroupInfo)
    {
        if ( It->nKind == SfxCfgKind::GROUP_STYLES && It->pObject )
        {
            SfxStyleInfo_Impl* pStyle = static_cast<SfxStyleInfo_Impl*>(It->pObject);
            delete pStyle;
        }
        else if ( It->nKind == SfxCfgKind::FUNCTION_SCRIPT && It->pObject )
        {
            OUString* pScriptURI = static_cast<OUString*>(It->pObject);
            delete pScriptURI;
        }
        else if ( It->nKind == SfxCfgKind::GROUP_SCRIPTCONTAINER && It->pObject)
        {
            css::uno::XInterface* xi = static_cast<css::uno::XInterface *>(It->pObject);
            if (xi != nullptr)
            {
                xi->release();
            }
        }
    }

    m_aGroupInfo.clear();
    Clear();
}

void CommandCategoryListBox::Init(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XFrame >& xFrame,
        const OUString& sModuleLongName)
{
    // User will not see incomplete UI
    SetUpdateMode(false);
    ClearAll();

    m_xContext = xContext;
    m_xFrame = xFrame;

    m_sModuleLongName = sModuleLongName;
    m_xGlobalCategoryInfo = css::ui::theUICategoryDescription::get( m_xContext );
    m_xModuleCategoryInfo.set(m_xGlobalCategoryInfo->getByName(m_sModuleLongName), css::uno::UNO_QUERY_THROW);
    m_xUICmdDescription   = css::frame::theUICommandDescription::get( m_xContext );

    // Support style commands
    css::uno::Reference<css::frame::XController> xController;
    css::uno::Reference<css::frame::XModel> xModel;
    if (xFrame.is())
        xController = xFrame->getController();
    if (xController.is())
        xModel = xController->getModel();

    m_aStylesInfo.init(sModuleLongName, xModel);
    SetStylesInfo(&m_aStylesInfo);

    try
    {
        css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider(m_xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Sequence< sal_Int16 > lGroups = xProvider->getSupportedCommandGroups();

        sal_Int32 nGroupsLength = lGroups.getLength();
        sal_Int32 nEntryPos = 0;

        if ( nGroupsLength > 0 )
        {
            // Add the category of "All commands"
            nEntryPos = InsertEntry( CuiResId(RID_SVXSTR_ALLFUNCTIONS) );
            m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_ALLFUNCTIONS, 0 ) );
            SetEntryData( nEntryPos, m_aGroupInfo.back().get() );
        }

        // Separate the "All commands"category from the actual categories
        AddSeparator( 0 );

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
            m_aGroupInfo.push_back(
              o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_FUNCTION, rGroupID ) );
            SetEntryData( nEntryPos, m_aGroupInfo.back().get() );
        }

        // Separate regular commands from styles and macros
        AddSeparator( GetEntryCount() - 1 );

        // Stop sorting, and add Macros and Styles to the end of the list
        SetStyle(GetStyle() & ~WB_SORT);

        // Add macros category
        OUString sMacros( CuiResId(RID_SVXSTR_MACROS) );
        nEntryPos = InsertEntry( sMacros );
        m_aGroupInfo.push_back(
            o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_SCRIPTCONTAINER, 0, nullptr) );
        SetEntryData( nEntryPos, m_aGroupInfo.back().get() );

        // Add styles category
        OUString sStyle( CuiResId(RID_SVXSTR_GROUP_STYLES) );
        nEntryPos = InsertEntry( sStyle );
        //TODO: last param should contain user data?
        m_aGroupInfo.push_back(
            o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0, nullptr ) );
        SetEntryData( nEntryPos, m_aGroupInfo.back().get() );
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
    {}

    // Reveal the updated UI to user
    SetUpdateMode(true);
    SelectEntryPos(0);
}

void CommandCategoryListBox::FillFunctionsList(
    const css::uno::Sequence<css::frame::DispatchInformation>& xCommands,
    const VclPtr<SfxConfigFunctionListBox>&  pFunctionListBox,
    const OUString& filterTerm,
    SaveInData *pCurrentSaveInData )
{
    // Setup search filter parameters
    m_searchOptions.searchString = filterTerm;
    utl::TextSearch textSearch( m_searchOptions );

    for (const auto & rInfo : xCommands)
    {
        OUString sUIName    = getCommandName(rInfo.Command);
        OUString sLabel     = vcl::CommandInfoProvider::GetLabelForCommand(rInfo.Command, m_sModuleLongName);
        OUString sTooltipLabel = vcl::CommandInfoProvider::GetTooltipForCommand( rInfo.Command, m_xFrame);
        OUString sPopupLabel =
                (vcl::CommandInfoProvider::GetPopupLabelForCommand(rInfo.Command, m_sModuleLongName))
                .replaceFirst("~", "");

        // Apply the search filter
        if (!filterTerm.isEmpty()
            && !textSearch.searchForward( sUIName )
            && !textSearch.searchForward( sLabel )
            && !textSearch.searchForward( sTooltipLabel )
            && !textSearch.searchForward( sPopupLabel ) )
        {
            continue;
        }

        Image aImage;
        if (pCurrentSaveInData)
            aImage = pCurrentSaveInData->GetImage(rInfo.Command);

        SvTreeListEntry* pFuncEntry = pFunctionListBox->InsertEntry(sUIName, aImage, aImage );

        m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SLOT, 0 ) );
        SfxGroupInfo_Impl* pGrpInfo = m_aGroupInfo.back().get();
        pGrpInfo->sCommand = rInfo.Command;
        pGrpInfo->sLabel   = sUIName;
        pFuncEntry->SetUserData(pGrpInfo);
    }
}

OUString CommandCategoryListBox::getCommandName(const OUString& sCommand)
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

void CommandCategoryListBox::categorySelected(  const VclPtr<SfxConfigFunctionListBox>&  pFunctionListBox,
                                                const OUString& filterTerm , SaveInData *pCurrentSaveInData)
{
    SfxGroupInfo_Impl *pInfo = static_cast<SfxGroupInfo_Impl*>(GetSelectedEntryData());
    pFunctionListBox->SetUpdateMode(false);
    pFunctionListBox->ClearAll();

    switch ( pInfo->nKind )
    {
        case SfxCfgKind::GROUP_ALLFUNCTIONS:
        {
            css::uno::Reference< css::frame::XDispatchInformationProvider2 >
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
                        lCommands = xProvider->getConfigurableDispatchInformation2(
                                        pCurrentInfo->nUniqueID, sal_uInt32(pFunctionListBox->GetSlotMode()));
                        FillFunctionsList( lCommands, pFunctionListBox, filterTerm, pCurrentSaveInData );
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
            css::uno::Reference< css::frame::XDispatchInformationProvider2 >
                xProvider (m_xFrame, css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::frame::DispatchInformation > lCommands =
                xProvider->getConfigurableDispatchInformation2(nGroup, sal_uInt32(pFunctionListBox->GetSlotMode()));
            FillFunctionsList( lCommands, pFunctionListBox, filterTerm, pCurrentSaveInData );
            break;
        }
        case SfxCfgKind::GROUP_SCRIPTCONTAINER: //Macros
        {
            SAL_INFO("cui.customize", "** ** About to initialise SF Scripts");
            // Add Scripting Framework entries
            css::uno::Reference< css::script::browse::XBrowseNode > rootNode;
            try
            {
                css::uno::Reference< css::script::browse::XBrowseNodeFactory > xFac
                    = css::script::browse::theBrowseNodeFactory::get( m_xContext );
                rootNode.set( xFac->createView( css::script::browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) );
            }
            catch( css::uno::Exception& e )
            {
                SAL_WARN("cui.customize", "Caught some exception whilst retrieving browse nodes from factory... Exception: " << e);
                // TODO exception handling
            }

            if ( rootNode.is() && rootNode.get()->hasChildNodes() )
            {
                //We call acquire on the XBrowseNode so that it does not
                //get autodestructed and become invalid when accessed later.
                rootNode->acquire();

                m_aGroupInfo.push_back(
                    o3tl::make_unique<SfxGroupInfo_Impl>(
                        SfxCfgKind::GROUP_SCRIPTCONTAINER, 0, static_cast<void *>(rootNode.get()) ) );

                // Add main macro groups
                for ( auto const & childGroup : rootNode.get()->getChildNodes() )
                {
                    OUString sUIName;
                    childGroup.get()->acquire();

                    if ( childGroup.get()->hasChildNodes() )
                    {
                        if ( childGroup.get()->getName() == "user" )
                        {
                            sUIName = CuiResId( RID_SVXSTR_MYMACROS );
                        }
                        else if ( childGroup.get()->getName() == "share" )
                        {
                            sUIName = CuiResId( RID_SVXSTR_PRODMACROS );
                        }
                        else
                        {
                            sUIName = childGroup.get()->getName();
                        }

                        if (sUIName.isEmpty())
                        {
                            continue;
                        }

                        SvTreeListEntry* pMacroGroup = pFunctionListBox->InsertEntry( sUIName );
                        m_aGroupInfo.push_back(
                            o3tl::make_unique<SfxGroupInfo_Impl>(
                                SfxCfgKind::GROUP_SCRIPTCONTAINER, 0 ) );
                        SfxGroupInfo_Impl* pGrpInfo = m_aGroupInfo.back().get();
                        pMacroGroup->SetUserData(pGrpInfo);
                        pMacroGroup->EnableChildrenOnDemand();

                        //Add the children and the grand children
                        addChildren( pMacroGroup, childGroup, pFunctionListBox, filterTerm, pCurrentSaveInData );

                        // Remove the main group if empty
                        if (!pMacroGroup->HasChildren())
                        {
                            pFunctionListBox->RemoveEntry( pMacroGroup );
                        }
                        else if (!filterTerm.isEmpty())
                        {
                            pFunctionListBox->Expand( pMacroGroup );
                        }
                    }
                }
            }

            break;
        }
        case SfxCfgKind::GROUP_STYLES:
        {
            const std::vector< SfxStyleInfo_Impl > lStyleFamilies = pStylesInfo->getStyleFamilies();

            for ( const auto & pIt : lStyleFamilies )
            {
                if ( pIt.sLabel.isEmpty() )
                {
                    continue;
                }

                SvTreeListEntry* pFuncEntry = pFunctionListBox->InsertEntry( pIt.sLabel ); // Name of the style family

                m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0 ) );
                SfxGroupInfo_Impl* pGrpInfo = m_aGroupInfo.back().get();
                pFuncEntry->SetUserData(pGrpInfo);
                pFuncEntry->EnableChildrenOnDemand();

                const std::vector< SfxStyleInfo_Impl > lStyles = pStylesInfo->getStyles(pIt.sFamily);

                // Setup search filter parameters
                m_searchOptions.searchString = filterTerm;
                utl::TextSearch textSearch( m_searchOptions );

                // Insert children (styles)
                for ( const auto & pStyleIt : lStyles )
                {
                    OUString sUIName = pStyleIt.sLabel;
                    sal_Int32 aStartPos = 0;
                    sal_Int32 aEndPos = sUIName.getLength();

                    // Apply the search filter
                    if (!filterTerm.isEmpty()
                            && !textSearch.SearchForward( sUIName, &aStartPos, &aEndPos ) )
                    {
                        continue;
                    }

                    SfxStyleInfo_Impl* pStyle = new SfxStyleInfo_Impl(pStyleIt);

                    SvTreeListEntry* pSubFuncEntry = pFunctionListBox->InsertEntry(
                                sUIName, pFuncEntry );

                    m_aGroupInfo.push_back(
                                o3tl::make_unique<SfxGroupInfo_Impl>(
                                    SfxCfgKind::GROUP_STYLES, 0, pStyle ) );

                    m_aGroupInfo.back()->sCommand = pStyle->sCommand;
                    m_aGroupInfo.back()->sLabel = pStyle->sLabel;
                    pSubFuncEntry->SetUserData( m_aGroupInfo.back().get() );
                }

                // Remove the style group from the list if no children
                if (!pFuncEntry->HasChildren())
                {
                    pFunctionListBox->RemoveEntry(pFuncEntry);
                }
                else if (!filterTerm.isEmpty())
                {
                    pFunctionListBox->Expand(pFuncEntry);
                }
            }

            break;
        }
        default:
            // Do nothing, the list box will stay empty
            SAL_INFO( "cui.customize", "Ignoring unexpected SfxCfgKind: " <<  static_cast<int>(pInfo->nKind) );
            break;
    }

    if ( pFunctionListBox->GetEntryCount() )
        pFunctionListBox->Select( pFunctionListBox->GetEntry( nullptr, 0 ) );

    pFunctionListBox->SetUpdateMode(true);
}

void CommandCategoryListBox::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    pStylesInfo = pStyles;
}

void CommandCategoryListBox::addChildren(
    SvTreeListEntry* parentEntry, const css::uno::Reference< css::script::browse::XBrowseNode > &parentNode,
    const VclPtr<SfxConfigFunctionListBox>&  pFunctionListBox, const OUString& filterTerm , SaveInData *pCurrentSaveInData)
{
    // Setup search filter parameters
    m_searchOptions.searchString = filterTerm;
    utl::TextSearch textSearch( m_searchOptions );

    for (auto const & child : parentNode.get()->getChildNodes())
    {
        // Acquire to prevent auto-destruction
        child.get()->acquire();

        if (child.get()->hasChildNodes())
        {
            OUString sUIName = child.get()->getName();

            SvTreeListEntry* pNewEntry = pFunctionListBox->InsertEntry( sUIName, parentEntry );

            m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_SCRIPTCONTAINER,
                                                                         0, static_cast<void *>( child.get())));
            pNewEntry->SetUserData( m_aGroupInfo.back().get() );
            pNewEntry->EnableChildrenOnDemand();

            addChildren(pNewEntry, child, pFunctionListBox, filterTerm, pCurrentSaveInData);

            // Remove the group if empty
            if (!pNewEntry->HasChildren())
                pFunctionListBox->RemoveEntry( pNewEntry );
            else
                pFunctionListBox->Expand( pNewEntry );

        }
        else if ( child.get()->getType() == css::script::browse::BrowseNodeTypes::SCRIPT )
        {
            // Prepare for filtering
            OUString sUIName = child.get()->getName();
            sal_Int32 aStartPos = 0;
            sal_Int32 aEndPos = sUIName.getLength();

            // Apply the search filter
            if (!filterTerm.isEmpty()
                    && !textSearch.SearchForward( sUIName, &aStartPos, &aEndPos ) )
            {
                continue;
            }

            OUString uri, description;

            css::uno::Reference < css::beans::XPropertySet >xPropSet( child.get(), css::uno::UNO_QUERY );

            if (!xPropSet.is())
            {
                continue;
            }

            css::uno::Any value = xPropSet->getPropertyValue("URI");
            value >>= uri;

            try
            {
                value = xPropSet->getPropertyValue("Description");
                value >>= description;
            }
            catch (css::uno::Exception &) {
                // do nothing, the description will be empty
            }

            if (description.isEmpty())
            {
                description = CuiResId( RID_SVXSTR_NOMACRODESC );
            }

            OUString* pScriptURI = new OUString( uri );

            Image aImage;
            if (pCurrentSaveInData)
                aImage = pCurrentSaveInData->GetImage(uri);

            SvTreeListEntry* pNewEntry = pFunctionListBox->InsertEntry( sUIName, aImage, aImage, parentEntry );

            m_aGroupInfo.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SCRIPT, 0, pScriptURI ));
            m_aGroupInfo.back()->sCommand = uri;
            m_aGroupInfo.back()->sLabel = sUIName;
            m_aGroupInfo.back()->sHelpText = description;
            pNewEntry->SetUserData( m_aGroupInfo.back().get() );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
