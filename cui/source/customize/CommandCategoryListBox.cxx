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
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
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
#include <tools/diagnose_ex.h>
#include <unotools/textsearch.hxx>

#include <dialmgr.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/string.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/searchopt.hxx>
#include <sal/log.hxx>

#include <cfg.hxx> //for SaveInData

CommandCategoryListBox::CommandCategoryListBox(std::unique_ptr<weld::ComboBox> xControl)
    : pStylesInfo( nullptr )
    , m_xControl(std::move(xControl))
{
    //Initialize search util
    m_searchOptions.AlgorithmType2 = css::util::SearchAlgorithms2::ABSOLUTE;
    m_searchOptions.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_searchOptions.searchFlag |= (css::util::SearchFlags::REG_NOT_BEGINOFLINE
                                | css::util::SearchFlags::REG_NOT_ENDOFLINE);
}

CommandCategoryListBox::~CommandCategoryListBox()
{
    ClearAll();
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
    m_xControl->clear();
}

void CommandCategoryListBox::Init(
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::frame::XFrame >& xFrame,
        const OUString& sModuleLongName)
{
    // User will not see incomplete UI
    m_xControl->freeze();
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

        if ( nGroupsLength > 0 )
        {
            // Add the category of "All commands"
            m_aGroupInfo.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_ALLFUNCTIONS, 0 ) );
            m_xControl->append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), CuiResId(RID_SVXSTR_ALLFUNCTIONS));
        }

        // Separate the "All commands"category from the actual categories
        m_xControl->append_separator();

        typedef std::pair<OUString, sal_Int16> str_id;
        std::vector<str_id> aCategories;

        // Add the actual categories
        for (sal_Int32 i = 0; i < nGroupsLength; ++i)
        {
            sal_Int16   nGroupID = lGroups[i];
            OUString    sGroupID = OUString::number(nGroupID);
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
            aCategories.emplace_back(std::make_pair(sGroupName, nGroupID));
        }

        auto const sort = comphelper::string::NaturalStringSorter(
            comphelper::getProcessComponentContext(),
            Application::GetSettings().GetUILanguageTag().getLocale());

        std::sort(aCategories.begin(), aCategories.end(),
                  [&sort](const str_id& a, const str_id& b)
                  { return sort.compare(a.first, b.first) < 0; });

        // Add the actual categories
        for (const auto &a : aCategories)
        {
            const OUString& rGroupName = a.first;
            sal_Int16 nGroupID = a.second;
            m_aGroupInfo.push_back(std::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_FUNCTION, nGroupID));
            m_xControl->append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), rGroupName);
        }

        // Separate regular commands from styles and macros
        m_xControl->append_separator();

        // Add macros category
        m_aGroupInfo.push_back(
            std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_SCRIPTCONTAINER, 0, nullptr) );
        m_xControl->append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), CuiResId(RID_SVXSTR_MACROS));

        // Add styles category
        //TODO: last param should contain user data?
        m_aGroupInfo.push_back(
            std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0, nullptr ) );
        m_xControl->append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), CuiResId(RID_SVXSTR_GROUP_STYLES));
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
    {}

    // Reveal the updated UI to user
    m_xControl->thaw();
    m_xControl->set_active(0);
}

void CommandCategoryListBox::FillFunctionsList(
    const css::uno::Sequence<css::frame::DispatchInformation>& xCommands,
    CuiConfigFunctionListBox*  pFunctionListBox,
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

        css::uno::Reference<css::graphic::XGraphic> xImage;
        if (pCurrentSaveInData)
            xImage = pCurrentSaveInData->GetImage(rInfo.Command);

        m_aGroupInfo.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SLOT, 0 ) );
        SfxGroupInfo_Impl* pGrpInfo = m_aGroupInfo.back().get();
        pGrpInfo->sCommand = rInfo.Command;
        pGrpInfo->sLabel   = sUIName;
        pFunctionListBox->append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), sUIName, xImage);
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

void CommandCategoryListBox::categorySelected(CuiConfigFunctionListBox* pFunctionListBox,
                                              const OUString& filterTerm, SaveInData *pCurrentSaveInData)
{
    SfxGroupInfo_Impl *pInfo = reinterpret_cast<SfxGroupInfo_Impl*>(m_xControl->get_active_id().toInt64());
    std::vector<std::unique_ptr<weld::TreeIter>> aNodesToExpand;
    pFunctionListBox->freeze();
    pFunctionListBox->ClearAll();

    switch ( pInfo->nKind )
    {
        case SfxCfgKind::GROUP_ALLFUNCTIONS:
        {
            css::uno::Reference< css::frame::XDispatchInformationProvider >
                xProvider( m_xFrame, css::uno::UNO_QUERY );
            sal_Int32 nEntryCount = m_xControl->get_count();

            for (sal_Int32 nCurPos = 0; nCurPos < nEntryCount; ++nCurPos)
            {
                SfxGroupInfo_Impl *pCurrentInfo =
                    reinterpret_cast<SfxGroupInfo_Impl*>(m_xControl->get_id(nCurPos).toInt64());

                if (!pCurrentInfo) //separator
                    continue;

                if (pCurrentInfo->nKind == SfxCfgKind::GROUP_FUNCTION)
                {
                    css::uno::Sequence< css::frame::DispatchInformation > lCommands;
                    try
                    {
                        lCommands = xProvider->getConfigurableDispatchInformation(
                                        pCurrentInfo->nUniqueID );
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
            css::uno::Reference< css::frame::XDispatchInformationProvider >
                xProvider (m_xFrame, css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::frame::DispatchInformation > lCommands =
                xProvider->getConfigurableDispatchInformation(nGroup);
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
            catch( css::uno::Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN("cui.customize", "Caught some exception whilst retrieving browse nodes from factory... Exception: " << exceptionToString(ex));
                // TODO exception handling
            }

            if ( rootNode.is() && rootNode.get()->hasChildNodes() )
            {
                //We call acquire on the XBrowseNode so that it does not
                //get autodestructed and become invalid when accessed later.
                rootNode->acquire();

                m_aGroupInfo.push_back(
                    std::make_unique<SfxGroupInfo_Impl>(
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

                        m_aGroupInfo.push_back(
                            std::make_unique<SfxGroupInfo_Impl>(
                                SfxCfgKind::GROUP_SCRIPTCONTAINER, 0 ) );
                        std::unique_ptr<weld::TreeIter> xMacroGroup(pFunctionListBox->tree_append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), sUIName));

                        //Add the children and the grand children
                        addChildren(xMacroGroup.get(), childGroup, pFunctionListBox, filterTerm, pCurrentSaveInData, aNodesToExpand);

                        // Remove the main group if empty
                        if (!pFunctionListBox->iter_has_child(*xMacroGroup))
                        {
                            pFunctionListBox->remove(*xMacroGroup);
                        }
                        else if (!filterTerm.isEmpty())
                        {
                            aNodesToExpand.emplace_back(std::move(xMacroGroup));
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


                m_aGroupInfo.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0 ) );
                // pIt.sLabel is Name of the style family
                std::unique_ptr<weld::TreeIter> xFuncEntry(pFunctionListBox->tree_append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), pIt.sLabel));

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

                    m_aGroupInfo.push_back(
                                std::make_unique<SfxGroupInfo_Impl>(
                                    SfxCfgKind::GROUP_STYLES, 0, pStyle ) );

                    m_aGroupInfo.back()->sCommand = pStyle->sCommand;
                    m_aGroupInfo.back()->sLabel = pStyle->sLabel;

                    pFunctionListBox->append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), sUIName, xFuncEntry.get());
                }

                // Remove the style group from the list if no children
                if (!pFunctionListBox->iter_has_child(*xFuncEntry))
                {
                    pFunctionListBox->remove(*xFuncEntry);
                }
                else if (!filterTerm.isEmpty())
                {
                    aNodesToExpand.emplace_back(std::move(xFuncEntry));
                }
            }

            break;
        }
        default:
            // Do nothing, the list box will stay empty
            SAL_INFO( "cui.customize", "Ignoring unexpected SfxCfgKind: " <<  static_cast<int>(pInfo->nKind) );
            break;
    }

    pFunctionListBox->thaw();

    if (pFunctionListBox->n_children())
        pFunctionListBox->select(0);

    //post freeze
    for (const auto& it : aNodesToExpand)
        pFunctionListBox->expand_row(*it);
}

void CommandCategoryListBox::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    pStylesInfo = pStyles;
}

void CommandCategoryListBox::addChildren(
    weld::TreeIter* parentEntry, const css::uno::Reference< css::script::browse::XBrowseNode > &parentNode,
    CuiConfigFunctionListBox* pFunctionListBox, const OUString& filterTerm , SaveInData *pCurrentSaveInData,
    std::vector<std::unique_ptr<weld::TreeIter>> &rNodesToExpand)
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

            m_aGroupInfo.push_back( std::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_SCRIPTCONTAINER,
                                                                         0, static_cast<void *>( child.get())));
            std::unique_ptr<weld::TreeIter> xNewEntry(pFunctionListBox->tree_append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), sUIName, parentEntry));

            addChildren(xNewEntry.get(), child, pFunctionListBox, filterTerm, pCurrentSaveInData, rNodesToExpand);

            // Remove the group if empty
            if (!pFunctionListBox->iter_has_child(*xNewEntry))
                pFunctionListBox->remove(*xNewEntry);
            else
                rNodesToExpand.emplace_back(std::move(xNewEntry));
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

            css::uno::Reference<css::graphic::XGraphic> xImage;
            if (pCurrentSaveInData)
                xImage = pCurrentSaveInData->GetImage(uri);

            m_aGroupInfo.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SCRIPT, 0, pScriptURI ));
            m_aGroupInfo.back()->sCommand = uri;
            m_aGroupInfo.back()->sLabel = sUIName;
            m_aGroupInfo.back()->sHelpText = description;
            pFunctionListBox->append(OUString::number(reinterpret_cast<sal_Int64>(m_aGroupInfo.back().get())), sUIName, xImage, parentEntry);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
