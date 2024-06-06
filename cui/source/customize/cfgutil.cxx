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

#include <cfgutil.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>
#include <com/sun/star/script/browse/theBrowseNodeFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ui/theUICategoryDescription.hpp>

#include <basic/basmgr.hxx>
#include <tools/urlobj.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <sfx2/minfitem.hxx>
#include <comphelper/SetFlagContextHelper.hxx>
#include <comphelper/documentinfo.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <svtools/imagemgr.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <dialmgr.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/help.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/string_view.hxx>

#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/Context.hxx>
#include <unotools/viewoptions.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;

SfxStylesInfo_Impl::SfxStylesInfo_Impl()
{}

void SfxStylesInfo_Impl::init(const OUString& rModuleName, const css::uno::Reference< css::frame::XModel >& xModel)
{
    m_aModuleName = rModuleName;
    m_xDoc = xModel;
}

const char CMDURL_STYLEPROT_ONLY[] = ".uno:StyleApply?";
const char CMDURL_SPART_ONLY    [] = "Style:string=";
const char CMDURL_FPART_ONLY    [] = "FamilyName:string=";

constexpr OUString STYLEPROP_UINAME = u"DisplayName"_ustr;
constexpr OUString MACRO_SELECTOR_CONFIGNAME = u"MacroSelectorDialog"_ustr;
constexpr OUString LAST_RUN_MACRO_INFO = u"LastRunMacro"_ustr;

OUString SfxStylesInfo_Impl::generateCommand(
    std::u16string_view sFamily, std::u16string_view sStyle)
{
    return OUString::Concat(".uno:StyleApply?Style:string=")
           + sStyle
           + "&FamilyName:string="
           + sFamily;
}

bool SfxStylesInfo_Impl::parseStyleCommand(SfxStyleInfo_Impl& aStyle)
{
    static const sal_Int32 LEN_STYLEPROT = strlen(CMDURL_STYLEPROT_ONLY);
    static const sal_Int32 LEN_SPART     = strlen(CMDURL_SPART_ONLY);
    static const sal_Int32 LEN_FPART     = strlen(CMDURL_FPART_ONLY);

    if (!aStyle.sCommand.startsWith(CMDURL_STYLEPROT_ONLY))
        return false;

    aStyle.sFamily.clear();
    aStyle.sStyle.clear();

    sal_Int32       nCmdLen  = aStyle.sCommand.getLength();
    OUString sCmdArgs = aStyle.sCommand.copy(LEN_STYLEPROT, nCmdLen-LEN_STYLEPROT);
    sal_Int32       i        = sCmdArgs.indexOf('&');
    if (i<0)
        return false;

    OUString sArg = sCmdArgs.copy(0, i);
    if (sArg.startsWith(CMDURL_SPART_ONLY))
        aStyle.sStyle = sArg.copy(LEN_SPART);
    else if (sArg.startsWith(CMDURL_FPART_ONLY))
        aStyle.sFamily = sArg.copy(LEN_FPART);

    sArg = sCmdArgs.copy(i+1, sCmdArgs.getLength()-i-1);
    if (sArg.startsWith(CMDURL_SPART_ONLY))
        aStyle.sStyle = sArg.copy(LEN_SPART);
    else if (sArg.startsWith(CMDURL_FPART_ONLY))
        aStyle.sFamily = sArg.copy(LEN_FPART);

    return !(aStyle.sFamily.isEmpty() || aStyle.sStyle.isEmpty());
}

void SfxStylesInfo_Impl::getLabel4Style(SfxStyleInfo_Impl& aStyle)
{
    try
    {
        css::uno::Reference< css::style::XStyleFamiliesSupplier > xModel(m_xDoc, css::uno::UNO_QUERY);

        css::uno::Reference< css::container::XNameAccess > xFamilies;
        if (xModel.is())
            xFamilies = xModel->getStyleFamilies();

        css::uno::Reference< css::container::XNameAccess > xStyleSet;
        if (xFamilies.is())
            xFamilies->getByName(aStyle.sFamily) >>= xStyleSet;

        css::uno::Reference< css::beans::XPropertySet > xStyle;
        if (xStyleSet.is())
            xStyleSet->getByName(aStyle.sStyle) >>= xStyle;

        aStyle.sLabel.clear();
        if (xStyle.is())
            xStyle->getPropertyValue(STYLEPROP_UINAME) >>= aStyle.sLabel;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { aStyle.sLabel.clear(); }

    if (aStyle.sLabel.isEmpty())
    {
        aStyle.sLabel = aStyle.sCommand;
    }
}

std::vector< SfxStyleInfo_Impl > SfxStylesInfo_Impl::getStyleFamilies() const
{
    // It's an optional interface!
    css::uno::Reference< css::style::XStyleFamiliesSupplier > xModel(m_xDoc, css::uno::UNO_QUERY);
    if (!xModel.is())
        return std::vector< SfxStyleInfo_Impl >();

    css::uno::Reference< css::container::XNameAccess > xCont = xModel->getStyleFamilies();
    const css::uno::Sequence< OUString > lFamilyNames = xCont->getElementNames();
    std::vector< SfxStyleInfo_Impl > lFamilies;
    for (const auto& aFamily : lFamilyNames)
    {
        if ((aFamily == "CellStyles" && m_aModuleName != "com.sun.star.sheet.SpreadsheetDocument") ||
             aFamily == "cell" || aFamily == "table" || aFamily == "Default")
            continue;

        SfxStyleInfo_Impl aFamilyInfo;
        aFamilyInfo.sFamily = aFamily;

        try
        {
            css::uno::Reference< css::beans::XPropertySet > xFamilyInfo;
            xCont->getByName(aFamilyInfo.sFamily) >>= xFamilyInfo;
            if (!xFamilyInfo.is())
            {
                // TODO_AS currently there is no support for an UIName property .. use internal family name instead
                aFamilyInfo.sLabel = aFamilyInfo.sFamily;
            }
            else
                xFamilyInfo->getPropertyValue(STYLEPROP_UINAME) >>= aFamilyInfo.sLabel;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { return std::vector< SfxStyleInfo_Impl >(); }

        lFamilies.push_back(aFamilyInfo);
    }

    return lFamilies;
}

std::vector< SfxStyleInfo_Impl > SfxStylesInfo_Impl::getStyles(const OUString& sFamily)
{
    css::uno::Sequence< OUString > lStyleNames;
    css::uno::Reference< css::style::XStyleFamiliesSupplier > xModel(m_xDoc, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameAccess > xFamilies = xModel->getStyleFamilies();
    css::uno::Reference< css::container::XNameAccess > xStyleSet;
    try
    {
        xFamilies->getByName(sFamily) >>= xStyleSet;
        lStyleNames = xStyleSet->getElementNames();
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { return std::vector< SfxStyleInfo_Impl >(); }

    std::vector< SfxStyleInfo_Impl > lStyles;
    sal_Int32                          c      = lStyleNames.getLength();
    sal_Int32                          i      = 0;
    for (i=0; i<c; ++i)
    {
        SfxStyleInfo_Impl aStyleInfo;
        aStyleInfo.sFamily  = sFamily;
        aStyleInfo.sStyle   = lStyleNames[i];
        aStyleInfo.sCommand = SfxStylesInfo_Impl::generateCommand(aStyleInfo.sFamily, aStyleInfo.sStyle);

        try
        {
            css::uno::Reference< css::beans::XPropertySet > xStyle;
            xStyleSet->getByName(aStyleInfo.sStyle) >>= xStyle;
            if (!xStyle.is())
                continue;
            xStyle->getPropertyValue(u"DisplayName"_ustr) >>= aStyleInfo.sLabel;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }

        lStyles.push_back(aStyleInfo);
    }
    return lStyles;
}

OUString CuiConfigFunctionListBox::GetCommandHelpText()
{
    SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(get_selected_id());
    if (pData)
    {
        if ( pData->nKind == SfxCfgKind::FUNCTION_SLOT )
        {
            return Application::GetHelp()->GetHelpText(pData->sCommand);
        }
        else if ( pData->nKind == SfxCfgKind::FUNCTION_SCRIPT )
        {
            return pData->sHelpText;
        }
    }
    return OUString();
}

OUString CuiConfigFunctionListBox::GetCurCommand() const
{
    SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(get_selected_id());
    if (!pData)
        return OUString();
    return pData->sCommand;
}

OUString CuiConfigFunctionListBox::GetCurLabel() const
{
    SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(get_selected_id());
    if (!pData)
        return OUString();
    if (!pData->sLabel.isEmpty())
        return pData->sLabel;
    return pData->sCommand;
}

CuiConfigFunctionListBox::CuiConfigFunctionListBox(std::unique_ptr<weld::TreeView> xTreeView)
    : m_xTreeView(std::move(xTreeView))
    , m_xScratchIter(m_xTreeView->make_iterator())
{
    m_xTreeView->make_sorted();
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 35, m_xTreeView->get_height_rows(9));
    m_xTreeView->connect_query_tooltip(LINK(this, CuiConfigFunctionListBox, QueryTooltip));
}

CuiConfigFunctionListBox::~CuiConfigFunctionListBox()
{
    ClearAll();
}

IMPL_LINK(CuiConfigFunctionListBox, QueryTooltip, const weld::TreeIter&, rIter, OUString)
{
    SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(m_xTreeView->get_id(rIter));
    if (!pData)
        return OUString();
    OUString aLabel = CuiResId(RID_CUISTR_COMMANDLABEL) + ": ";
    OUString aName = CuiResId(RID_CUISTR_COMMANDNAME) + ": ";
    OUString aTip = CuiResId(RID_CUISTR_COMMANDTIP) + ": ";
    return  aLabel + pData->sLabel + "\n" + aName + pData->sCommand+ "\n" + aTip + pData->sTooltip;
}

void CuiConfigFunctionListBox::ClearAll()
/*  Description
    Deletes all entries in the FunctionListBox, all UserData and all
    possibly existing MacroInfo.
*/
{
    sal_uInt16 nCount = aArr.size();
    for ( sal_uInt16 i=0; i<nCount; ++i )
    {
        SfxGroupInfo_Impl *pData = aArr[i].get();

        if ( pData->nKind == SfxCfgKind::FUNCTION_SCRIPT )
        {
            OUString* pScriptURI = static_cast<OUString*>(pData->pObject);
            delete pScriptURI;
        }

        if ( pData->nKind == SfxCfgKind::GROUP_SCRIPTCONTAINER )
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != nullptr)
            {
                xi->release();
            }
        }
    }

    aArr.clear();
    m_xTreeView->clear();
}

OUString CuiConfigFunctionListBox::GetSelectedScriptURI() const
{
    SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(get_selected_id());
    if (pData && pData->nKind == SfxCfgKind::FUNCTION_SCRIPT)
        return *static_cast<OUString*>(pData->pObject);
    return OUString();
}

struct SvxConfigGroupBoxResource_Impl
{
    OUString m_sMyMacros;
    OUString m_sProdMacros;
    OUString m_sDlgMacros;
    OUString m_aStrGroupStyles;
    OUString m_aStrGroupSidebarDecks;

    SvxConfigGroupBoxResource_Impl();
};

SvxConfigGroupBoxResource_Impl::SvxConfigGroupBoxResource_Impl() :
    m_sMyMacros(CuiResId(RID_CUISTR_MYMACROS)),
    m_sProdMacros(CuiResId(RID_CUISTR_PRODMACROS)),
    m_sDlgMacros(CuiResId(RID_CUISTR_PRODMACROS)),
    m_aStrGroupStyles(CuiResId(RID_CUISTR_GROUP_STYLES)),
    m_aStrGroupSidebarDecks(CuiResId(RID_CUISTR_GROUP_SIDEBARDECKS))
{
}

void CuiConfigGroupListBox::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    m_pStylesInfo = pStyles;
}

namespace
{

    /** examines a component whether it supports XEmbeddedScripts, or provides access to such a
        component by implementing XScriptInvocationContext.
        @return
            the model which supports the embedded scripts, or <NULL/> if it cannot find such a
            model
    */
    Reference< XModel > lcl_getDocumentWithScripts_throw( const Reference< XInterface >& _rxComponent )
    {
        Reference< XEmbeddedScripts > xScripts( _rxComponent, UNO_QUERY );
        if ( !xScripts.is() )
        {
            Reference< XScriptInvocationContext > xContext( _rxComponent, UNO_QUERY );
            if ( xContext.is() )
                xScripts = xContext->getScriptContainer();
        }

        return Reference< XModel >( xScripts, UNO_QUERY );
    }


    Reference< XModel > lcl_getScriptableDocument_nothrow( const Reference< XFrame >& _rxFrame )
    {
        Reference< XModel > xDocument;

        // examine our associated frame
        try
        {
            OSL_ENSURE( _rxFrame.is(), "lcl_getScriptableDocument_nothrow: you need to pass a frame to this dialog/tab page!" );
            if ( _rxFrame.is() )
            {
                // first try the model in the frame
                Reference< XController > xController( _rxFrame->getController(), UNO_SET_THROW );
                xDocument = lcl_getDocumentWithScripts_throw( xController->getModel() );

                if ( !xDocument.is() )
                {
                    // if there is no suitable document in the frame, try the controller
                    xDocument = lcl_getDocumentWithScripts_throw( _rxFrame->getController() );
                }
            }
        }
        catch( const Exception& )
        {
        }

        return xDocument;
    }
}

CuiConfigGroupListBox::CuiConfigGroupListBox(std::unique_ptr<weld::TreeView> xTreeView)
    : xImp(new SvxConfigGroupBoxResource_Impl())
    , m_pFunctionListBox(nullptr)
    , m_pStylesInfo(nullptr)
    , m_xTreeView(std::move(xTreeView))
    , m_xScratchIter(m_xTreeView->make_iterator())
{
    m_xTreeView->connect_expanding(LINK(this, CuiConfigGroupListBox, ExpandingHdl));
    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 35, m_xTreeView->get_height_rows(9));
}

CuiConfigGroupListBox::~CuiConfigGroupListBox()
{
    ClearAll();
}

void CuiConfigGroupListBox::ClearAll()
{
    sal_uInt16 nCount = aArr.size();
    for ( sal_uInt16 i=0; i<nCount; ++i )
    {
        SfxGroupInfo_Impl *pData = aArr[i].get();
        if (pData->nKind == SfxCfgKind::GROUP_STYLES && pData->pObject)
        {
            SfxStyleInfo_Impl* pStyle = static_cast<SfxStyleInfo_Impl*>(pData->pObject);
            delete pStyle;
        }
        else if (pData->nKind == SfxCfgKind::FUNCTION_SCRIPT && pData->pObject )
        {
            OUString* pScriptURI = static_cast<OUString*>(pData->pObject);
            delete pScriptURI;
        }
        else if (pData->nKind == SfxCfgKind::GROUP_SCRIPTCONTAINER)
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != nullptr)
            {
                xi->release();
            }
        }
    }

    aArr.clear();
    m_xTreeView->clear();
}

sal_Int32 CuiConfigGroupListBox::InitModule()
{
    try
    {
        // return the number of added groups
        css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider(m_xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Sequence< sal_Int16 > lGroups = xProvider->getSupportedCommandGroups();
        sal_Int32                       c1      = lGroups.getLength();
        sal_Int32                       i1      = 0;
        sal_Int32                       nAddedGroups = 0;

        for (i1=0; i1<c1; ++i1)
        {
            sal_Int16      nGroupID   = lGroups[i1];
            OUString sGroupID   = OUString::number(nGroupID);
            OUString sGroupName ;

            try
            {
                m_xModuleCategoryInfo->getByName(sGroupID) >>= sGroupName;
                if (sGroupName.isEmpty())
                    continue;
            }
            catch(const css::container::NoSuchElementException&)
                { continue; }

            aArr.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_FUNCTION, nGroupID ) );
            m_xTreeView->append(weld::toId(aArr.back().get()), sGroupName);
            nAddedGroups++;
        }
        return nAddedGroups;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
    return 0;
}

void CuiConfigGroupListBox::FillScriptList(const css::uno::Reference< css::script::browse::XBrowseNode >& xRootNode,
                                           const weld::TreeIter* pParentEntry)
{
    try {
        if ( xRootNode->hasChildNodes() )
        {
            // tdf#120362: Don't ask to enable disabled Java when filling script list
            css::uno::ContextLayer layer(comphelper::NoEnableJavaInteractionContext());

            const Sequence< Reference< browse::XBrowseNode > > children =
                xRootNode->getChildNodes();
            bool bIsRootNode = false;

            OUString user(u"user"_ustr);
            OUString share(u"share"_ustr);
            if ( xRootNode->getName() == "Root" )
            {
                bIsRootNode = true;
            }

            //To mimic current starbasic behaviour we
            //need to make sure that only the current document
            //is displayed in the config tree. Tests below
            //set the bDisplay flag to FALSE if the current
            //node is a first level child of the Root and is NOT
            //either the current document, user or share
            OUString currentDocTitle;
            Reference< XModel > xDocument( lcl_getScriptableDocument_nothrow( m_xFrame ) );
            if ( xDocument.is() )
            {
                currentDocTitle = ::comphelper::DocumentInfo::getDocumentTitle( xDocument );
            }

            for ( Reference< browse::XBrowseNode > const & theChild : children )
            {
                bool bDisplay = true;
                OUString uiName = theChild->getName();
                if ( bIsRootNode )
                {
                    if (  ! (uiName == user  || uiName == share ||
                             uiName == currentDocTitle ) )
                    {
                        bDisplay=false;
                    }
                    else
                    {
                        if ( uiName == user )
                        {
                            uiName = xImp->m_sMyMacros;
                        }
                        else if ( uiName == share )
                        {
                            uiName = xImp->m_sProdMacros;
                        }
                    }
                }
                if (theChild->getType() != browse::BrowseNodeTypes::SCRIPT  && bDisplay )
                {
//                  We call acquire on the XBrowseNode so that it does not
//                  get autodestructed and become invalid when accessed later.
                    theChild->acquire();

                    bool bChildOnDemand = false;

                    if ( theChild->hasChildNodes() )
                    {
                        const Sequence< Reference< browse::XBrowseNode > > grandchildren =
                            theChild->getChildNodes();

                        for ( const auto& rxNode : grandchildren )
                        {
                            if ( rxNode->getType() == browse::BrowseNodeTypes::CONTAINER )
                            {
                                bChildOnDemand = true;
                                break;
                            }
                        }
                    }

                    OUString aImage = GetImage(theChild, m_xContext, bIsRootNode);

                    aArr.push_back( std::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_SCRIPTCONTAINER,
                            0, static_cast<void *>( theChild.get())));

                    OUString sId(weld::toId(aArr.back().get()));
                    m_xTreeView->insert(pParentEntry, -1, &uiName, &sId, nullptr, nullptr, bChildOnDemand, m_xScratchIter.get());
                    m_xTreeView->set_image(*m_xScratchIter, aImage);
                }
            }
        }
    }
    catch (RuntimeException&) {
        // do nothing, the entry will not be displayed in the UI
    }
}

void CuiConfigGroupListBox::FillFunctionsList(const css::uno::Sequence<DispatchInformation>& xCommands)
{
    m_pFunctionListBox->freeze();
    for (const auto & rInfo : xCommands)
    {
        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rInfo.Command, m_sModuleLongName);

        OUString sUIName = MapCommand2UIName(rInfo.Command);
        aArr.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SLOT, 0 ) );
        SfxGroupInfo_Impl* pGrpInfo = aArr.back().get();
        pGrpInfo->sCommand = rInfo.Command;
        pGrpInfo->sLabel   = sUIName;
        pGrpInfo->sTooltip = vcl::CommandInfoProvider::GetTooltipForCommand(rInfo.Command, aProperties, m_xFrame);
        m_pFunctionListBox->append(weld::toId(pGrpInfo), sUIName);
    }
    m_pFunctionListBox->thaw();
}

void CuiConfigGroupListBox::Init(const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const css::uno::Reference< css::frame::XFrame >& xFrame,
    const OUString& sModuleLongName,
    bool bEventMode)
{
    m_xTreeView->freeze();
    ClearAll(); // Remove all old entries from treelist box

    m_xContext = xContext;
    m_xFrame = xFrame;
    sal_Int32 nAddedGroups = 0;
    if( bEventMode )
    {
        m_sModuleLongName = sModuleLongName;
        m_xGlobalCategoryInfo = css::ui::theUICategoryDescription::get( m_xContext );
        m_xModuleCategoryInfo.set(m_xGlobalCategoryInfo->getByName(m_sModuleLongName), css::uno::UNO_QUERY_THROW);
        m_xUICmdDescription   = css::frame::theUICommandDescription::get( m_xContext );

        nAddedGroups = InitModule();
    }

    SAL_INFO("cui.customize", "** ** About to initialise SF Scripts");
    // Add Scripting Framework entries
    Reference< browse::XBrowseNode > rootNode;
    try
    {
        Reference< browse::XBrowseNodeFactory > xFac = browse::theBrowseNodeFactory::get( m_xContext );
        rootNode.set( xFac->createView( browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("cui.customize", "Caught some exception whilst retrieving browse nodes from factory");
        // TODO exception handling
    }

    m_xTreeView->thaw();
    m_xTreeView->make_sorted();
    m_xTreeView->make_unsorted();
    m_xTreeView->freeze();

    // add All Commands to the top
    if ( bEventMode && nAddedGroups )
    {
        aArr.insert(aArr.begin(), std::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_ALLFUNCTIONS, 0));
        OUString sId(weld::toId(aArr.front().get()));
        OUString s(CuiResId(RID_CUISTR_ALLFUNCTIONS));
        m_xTreeView->insert(nullptr, 0, &s, &sId, nullptr, nullptr, false, nullptr);
    }

    // add application macros to the end
    if ( rootNode.is() )
    {
        if ( bEventMode )
        {
                //We call acquire on the XBrowseNode so that it does not
                //get autodestructed and become invalid when accessed later.
            rootNode->acquire();

            aArr.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_SCRIPTCONTAINER, 0,
                    static_cast<void *>(rootNode.get())));
            OUString aTitle(xImp->m_sDlgMacros);
            OUString sId(weld::toId(aArr.back().get()));
            m_xTreeView->insert(nullptr, -1, &aTitle, &sId, nullptr, nullptr, true, nullptr);
        }
        else
        {
             //We are only showing scripts not slot APIs so skip
             //Root node and show location nodes
            FillScriptList(rootNode, nullptr);
        }
    }

    // add styles and sidebar decks to the end
    if ( bEventMode )
    {
        aArr.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0, nullptr ) ); // TODO last parameter should contain user data
        OUString sStyle(xImp->m_aStrGroupStyles);
        OUString sId(weld::toId(aArr.back().get()));
        m_xTreeView->insert(nullptr, -1, &sStyle, &sId, nullptr, nullptr, true, nullptr);

        aArr.push_back( std::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_SIDEBARDECKS, 0));
        OUString sSidebarDecks(xImp->m_aStrGroupSidebarDecks);
        sId = weld::toId(aArr.back().get());
        m_xTreeView->insert(nullptr, -1, &sSidebarDecks, &sId, nullptr, nullptr, false, nullptr);
    }

    m_xTreeView->thaw();
    m_xTreeView->scroll_to_row(0);
    m_xTreeView->select(0);
}

OUString CuiConfigGroupListBox::GetImage(
    const Reference< browse::XBrowseNode >& node,
    Reference< XComponentContext > const & xCtx,
    bool bIsRootNode)
{
    OUString aImage;
    if ( bIsRootNode )
    {
        if (node->getName() == "user" || node->getName() == "share" )
        {
            aImage = RID_CUIBMP_HARDDISK;
        }
        else
        {
            OUString factoryURL;
            OUString nodeName = node->getName();
            Reference<XInterface> xDocumentModel = getDocumentModel(xCtx, nodeName );
            if ( xDocumentModel.is() )
            {
                Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(xCtx) );
                // get the long name of the document:
                OUString appModule( xModuleManager->identify(
                                    xDocumentModel ) );
                Sequence<beans::PropertyValue> moduleDescr;
                Any aAny = xModuleManager->getByName(appModule);
                if( !( aAny >>= moduleDescr ) )
                {
                    throw RuntimeException(u"SFTreeListBox::Init: failed to get PropertyValue"_ustr);
                }
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if (moduleDescr[pos].Name == "ooSetupFactoryEmptyDocumentURL")
                    {
                        moduleDescr[pos].Value >>= factoryURL;
                        SAL_INFO("cui.customize", "factory url for doc images is " << factoryURL);
                        break;
                    }
                }
            }
            if( !factoryURL.isEmpty() )
            {
                aImage = SvFileInformationManager::GetFileImageId(INetURLObject(factoryURL));
            }
            else
            {
                aImage = RID_CUIBMP_DOC;
            }
        }
    }
    else
    {
        if( node->getType() == browse::BrowseNodeTypes::SCRIPT )
            aImage = RID_CUIBMP_MACRO;
        else
            aImage = RID_CUIBMP_LIB;
    }
    return aImage;
}

Reference< XInterface  >
CuiConfigGroupListBox::getDocumentModel( Reference< XComponentContext > const & xCtx, std::u16string_view docName )
{
    Reference< XInterface > xModel;
    Reference< frame::XDesktop2 > desktop = frame::Desktop::create( xCtx );

    Reference< container::XEnumerationAccess > componentsAccess =
        desktop->getComponents();
    Reference< container::XEnumeration > components =
        componentsAccess->createEnumeration();
    while (components->hasMoreElements())
    {
        Reference< frame::XModel > model(
            components->nextElement(), UNO_QUERY );
        if ( model.is() )
        {
            OUString sTdocUrl =
                ::comphelper::DocumentInfo::getDocumentTitle( model );
            if( sTdocUrl == docName )
            {
                xModel = model;
                break;
            }
        }
    }
    return xModel;
}

OUString CuiConfigGroupListBox::MapCommand2UIName(const OUString& sCommand)
{
    OUString sUIName;
    try
    {
        css::uno::Reference< css::container::XNameAccess > xModuleConf;
        m_xUICmdDescription->getByName(m_sModuleLongName) >>= xModuleConf;
        if (xModuleConf.is())
        {
            ::comphelper::SequenceAsHashMap lProps(xModuleConf->getByName(sCommand));
            sUIName = lProps.getUnpackedValueOrDefault(u"Name"_ustr, OUString());
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

void CuiConfigGroupListBox::GroupSelected()
/*  Description
    A function group or a basic module has been selected.
    All functions/macros are displayed in the functionlistbox.
*/
{
    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_selected(xIter.get()))
        return;

    SfxGroupInfo_Impl *pInfo = weld::fromId<SfxGroupInfo_Impl*>(m_xTreeView->get_id(*xIter));
    m_pFunctionListBox->freeze();
    m_pFunctionListBox->ClearAll();

    switch ( pInfo->nKind )
    {
        case SfxCfgKind::GROUP_ALLFUNCTIONS:
        {
            css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider( m_xFrame, UNO_QUERY );
            bool bValidIter = m_xTreeView->get_iter_first(*xIter);
            while (bValidIter)
            {
                SfxGroupInfo_Impl *pCurrentInfo = weld::fromId<SfxGroupInfo_Impl*>(m_xTreeView->get_id(*xIter));
                if (pCurrentInfo->nKind == SfxCfgKind::GROUP_FUNCTION)
                {
                    css::uno::Sequence< css::frame::DispatchInformation > lCommands;
                    try
                    {
                        lCommands = xProvider->getConfigurableDispatchInformation( pCurrentInfo->nUniqueID );
                        FillFunctionsList( lCommands );
                    }
                    catch ( container::NoSuchElementException& )
                    {
                    }
                }
                bValidIter = m_xTreeView->iter_next(*xIter);
            }
            break;
        }

        case SfxCfgKind::GROUP_FUNCTION :
        {
            sal_uInt16                                                          nGroup    = pInfo->nUniqueID;
            css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider (m_xFrame, css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::frame::DispatchInformation >           lCommands = xProvider->getConfigurableDispatchInformation(nGroup);
            FillFunctionsList( lCommands );
            break;
        }

        case SfxCfgKind::GROUP_SCRIPTCONTAINER:
        {
            if (!m_xTreeView->iter_has_child(*xIter))
            {
                Reference< browse::XBrowseNode > rootNode(
                    static_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;

                try {
                    if ( rootNode->hasChildNodes() )
                    {
                        const Sequence< Reference< browse::XBrowseNode > > children =
                            rootNode->getChildNodes();

                        for ( const Reference< browse::XBrowseNode >& childNode : children )
                        {
                            if (childNode->getType() == browse::BrowseNodeTypes::SCRIPT)
                            {
                                OUString uri, description;

                                Reference < beans::XPropertySet >xPropSet( childNode, UNO_QUERY );
                                if (!xPropSet.is())
                                {
                                    continue;
                                }

                                Any value =
                                    xPropSet->getPropertyValue(u"URI"_ustr);
                                value >>= uri;

                                try
                                {
                                    value = xPropSet->getPropertyValue(u"Description"_ustr);
                                    value >>= description;
                                }
                                catch (Exception &) {
                                    // do nothing, the description will be empty
                                }

                                OUString* pScriptURI = new OUString( uri );

                                OUString aImage = GetImage(childNode, Reference< XComponentContext >(), false);
                                m_pFunctionListBox->aArr.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SCRIPT, 0, pScriptURI ));
                                m_pFunctionListBox->aArr.back()->sCommand = uri;
                                m_pFunctionListBox->aArr.back()->sLabel = childNode->getName();
                                m_pFunctionListBox->aArr.back()->sHelpText = description;

                                OUString sId(weld::toId(m_pFunctionListBox->aArr.back().get()));
                                m_pFunctionListBox->append(sId, childNode->getName(), aImage);
                            }
                        }
                    }
                }
                catch (RuntimeException&) {
                    // do nothing, the entry will not be displayed in the UI
                }
            }
            break;
        }

        case SfxCfgKind::GROUP_STYLES :
        {
            SfxStyleInfo_Impl* pFamily = static_cast<SfxStyleInfo_Impl*>(pInfo->pObject);
            if (pFamily)
            {
                const std::vector< SfxStyleInfo_Impl > lStyles = m_pStylesInfo->getStyles(pFamily->sFamily);
                for (auto const& lStyle : lStyles)
                {
                    SfxStyleInfo_Impl* pStyle = new SfxStyleInfo_Impl(lStyle);
                    m_pFunctionListBox->aArr.push_back(std::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_STYLES, 0, pStyle));
                    m_pFunctionListBox->aArr.back()->sCommand = pStyle->sCommand;
                    m_pFunctionListBox->aArr.back()->sLabel = pStyle->sLabel;
                    OUString sId(weld::toId(m_pFunctionListBox->aArr.back().get()));
                    m_pFunctionListBox->append(sId, pStyle->sLabel);
                }
            }
            break;
        }

        case SfxCfgKind::GROUP_SIDEBARDECKS:
        {
            sfx2::sidebar::ResourceManager aResourceManager;
            sfx2::sidebar::Context aContext(m_sModuleLongName, OUString());
            sfx2::sidebar::ResourceManager::DeckContextDescriptorContainer aDecks;
            aResourceManager.GetMatchingDecks(aDecks, aContext, false, m_xFrame->getController());

            for (auto const& rDeck : aDecks)
            {
                const OUString sCommand = ".uno:SidebarDeck." + rDeck.msId;
                m_pFunctionListBox->aArr.push_back(std::make_unique<SfxGroupInfo_Impl>(
                                                       SfxCfgKind::GROUP_SIDEBARDECKS, 0,
                                                       nullptr));
                m_pFunctionListBox->aArr.back()->sCommand = sCommand;
                m_pFunctionListBox->aArr.back()->sLabel = rDeck.msId;
                m_pFunctionListBox->aArr.back()->sTooltip =
                        vcl::CommandInfoProvider::GetCommandShortcut(sCommand, m_xFrame);
                m_pFunctionListBox->append(weld::toId(m_pFunctionListBox->aArr.back().get()),
                                           rDeck.msId);
            }

            break;
        }

        default:
            // Do nothing, the list box will stay empty
            SAL_INFO( "cui.customize", "Ignoring unexpected SfxCfgKind: " <<  static_cast<int>(pInfo->nKind) );
            break;
    }

    m_pFunctionListBox->thaw();

    if (m_pFunctionListBox->n_children())
        m_pFunctionListBox->select(0);
}

/*  Description
    A basic or a library is opened.
*/
IMPL_LINK(CuiConfigGroupListBox, ExpandingHdl, const weld::TreeIter&, rIter, bool)
{
    SfxGroupInfo_Impl *pInfo = weld::fromId<SfxGroupInfo_Impl*>(m_xTreeView->get_id(rIter));
    switch ( pInfo->nKind )
    {
        case SfxCfgKind::GROUP_SCRIPTCONTAINER:
        {
            if (!m_xTreeView->iter_has_child(rIter))
            {
                Reference< browse::XBrowseNode > rootNode(
                    static_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;
                FillScriptList(rootNode, &rIter);
            }
            break;
        }

        case SfxCfgKind::GROUP_STYLES:
        {
            if (!m_xTreeView->iter_has_child(rIter))
            {
                const std::vector<SfxStyleInfo_Impl> lStyleFamilies = m_pStylesInfo->getStyleFamilies();
                for (auto const& lStyleFamily : lStyleFamilies)
                {
                    SfxStyleInfo_Impl* pFamily = new SfxStyleInfo_Impl(lStyleFamily);
                    aArr.push_back( std::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0, pFamily ));
                    OUString sId(weld::toId(aArr.back().get()));
                    m_xTreeView->insert(&rIter, -1, &pFamily->sLabel, &sId, nullptr, nullptr, false, nullptr);
                }
            }
            break;
        }

        default:
            OSL_FAIL( "Wrong group type!" );
            break;
    }
    return true;
}

#if HAVE_FEATURE_SCRIPTING
void CuiConfigGroupListBox::SelectMacro( const SfxMacroInfoItem *pItem )
{
    auto const rMacro = pItem->GetQualifiedName();
    sal_Int32 nIdx {rMacro.lastIndexOf('.')};
    const std::u16string_view aMethod( rMacro.subView(nIdx + 1) );
    std::u16string_view aLib;
    std::u16string_view aModule;
    if ( nIdx>0 )
    {
        // string contains at least 2 tokens
        nIdx = rMacro.lastIndexOf('.', nIdx);
        if (nIdx != -1)
        {
            // string contains at least 3 tokens
            aLib = o3tl::getToken(rMacro, 0, '.' );
            sal_Int32 nIdx2 = nIdx + 1;
            aModule = o3tl::getToken(rMacro, 0, '.', nIdx2 );
        }
    }

    std::unique_ptr<weld::TreeIter> xIter = m_xTreeView->make_iterator();
    if (!m_xTreeView->get_iter_first(*xIter))
        return;

    do
    {
        OUString aEntryBas = m_xTreeView->get_text(*xIter);
        if (aEntryBas == xImp->m_sDlgMacros)
        {
            m_xTreeView->expand_row(*xIter);
            std::unique_ptr<weld::TreeIter> xLocationIter = m_xTreeView->make_iterator(xIter.get());
            if (m_xTreeView->iter_children(*xLocationIter))
            {
                do
                {
                    m_xTreeView->expand_row(*xLocationIter);
                    std::unique_ptr<weld::TreeIter> xLibIter = m_xTreeView->make_iterator(xLocationIter.get());
                    if (m_xTreeView->iter_children(*xLibIter))
                    {
                        do
                        {
                            OUString aEntryLib = m_xTreeView->get_text(*xLibIter);
                            if (aEntryLib == aLib)
                            {
                                m_xTreeView->expand_row(*xLibIter);
                                std::unique_ptr<weld::TreeIter> xModIter = m_xTreeView->make_iterator(xLibIter.get());
                                if (m_xTreeView->iter_children(*xModIter))
                                {
                                    do
                                    {
                                        OUString aEntryMod = m_xTreeView->get_text(*xModIter);
                                        if ( aEntryMod == aModule )
                                        {
                                            m_xTreeView->expand_row(*xModIter);
                                            m_xTreeView->scroll_to_row(*xModIter);
                                            m_xTreeView->select(*xModIter);
                                            GroupSelected();
                                            for (int i = 0, nCount = m_pFunctionListBox->n_children(); i < nCount; ++i)
                                            {
                                                OUString aEntryMethod = m_pFunctionListBox->get_text(i);
                                                if (aEntryMethod == aMethod)
                                                {
                                                    m_pFunctionListBox->select(i);
                                                    m_pFunctionListBox->scroll_to_row(i);
                                                    return;
                                                }
                                            }
                                            m_xTreeView->collapse_row(*xModIter);
                                        }
                                    } while (m_xTreeView->iter_next_sibling(*xModIter));
                                }
                                m_xTreeView->collapse_row(*xLibIter);
                            }
                        } while (m_xTreeView->iter_next_sibling(*xLibIter));
                    }
                    m_xTreeView->collapse_row(*xLocationIter);
                } while (m_xTreeView->iter_next_sibling(*xLocationIter));
            }
            // If the macro can't be located, preselect the "Application Macros" category:
            m_xTreeView->scroll_to_row(*xIter);
            m_xTreeView->select(*xIter);
            return;
        }
    } while (m_xTreeView->iter_next_sibling(*xIter));
}
#endif

/*
 * Implementation of SvxScriptSelectorDialog
 *
 * This dialog is used for selecting Slot API commands
 * and Scripting Framework Scripts.
 */

SvxScriptSelectorDialog::SvxScriptSelectorDialog(
    weld::Window* pParent, const css::uno::Reference< css::frame::XFrame >& xFrame)
    : GenericDialogController(pParent, u"cui/ui/macroselectordialog.ui"_ustr, u"MacroSelectorDialog"_ustr)
    , m_xDialogDescription(m_xBuilder->weld_label(u"helpmacro"_ustr))
    , m_xCategories(new CuiConfigGroupListBox(m_xBuilder->weld_tree_view(u"categories"_ustr)))
    , m_xCommands(new CuiConfigFunctionListBox(m_xBuilder->weld_tree_view(u"commands"_ustr)))
    , m_xLibraryFT(m_xBuilder->weld_label(u"libraryft"_ustr))
    , m_xMacronameFT(m_xBuilder->weld_label(u"macronameft"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xCancelButton(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xDescriptionText(m_xBuilder->weld_text_view(u"description"_ustr))
    , m_xDescriptionFrame(m_xBuilder->weld_frame(u"descriptionframe"_ustr))
{
    m_xCancelButton->show();
    m_xDialogDescription->show();
    m_xOKButton->show();

    m_xLibraryFT->set_visible(true);
    m_xMacronameFT->set_visible(true);

    const OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(xFrame));
    m_xCategories->SetFunctionListBox(m_xCommands.get());
    m_xCategories->Init(comphelper::getProcessComponentContext(), xFrame, aModuleName, /*bShowSlots*/false);

    m_xCategories->connect_changed(
            LINK( this, SvxScriptSelectorDialog, SelectHdl ) );
    m_xCommands->connect_changed( LINK( this, SvxScriptSelectorDialog, SelectHdl ) );
    m_xCommands->connect_row_activated( LINK( this, SvxScriptSelectorDialog, FunctionDoubleClickHdl ) );
    m_xCommands->connect_popup_menu( LINK( this, SvxScriptSelectorDialog, ContextMenuHdl ) );

    m_xOKButton->connect_clicked( LINK( this, SvxScriptSelectorDialog, ClickHdl ) );
    m_xCancelButton->connect_clicked( LINK( this, SvxScriptSelectorDialog, ClickHdl ) );

    m_sDefaultDesc = m_xDescriptionText->get_text();

    // Support style commands
    uno::Reference<frame::XController> xController;
    uno::Reference<frame::XModel> xModel;
    if (xFrame.is())
        xController = xFrame->getController();
    if (xController.is())
        xModel = xController->getModel();

    m_aStylesInfo.init(aModuleName, xModel);
    m_xCategories->SetStylesInfo(&m_aStylesInfo);

    // The following call is a workaround to make scroll_to_row work as expected in kf5/x11
    m_xDialog->resize_to_request();

    LoadLastUsedMacro();
    UpdateUI();

    if (comphelper::LibreOfficeKit::isActive())
        m_xDescriptionFrame->hide();
}

SvxScriptSelectorDialog::~SvxScriptSelectorDialog()
{
}

IMPL_LINK(SvxScriptSelectorDialog, SelectHdl, weld::TreeView&, rCtrl, void)
{
    if (&rCtrl == &m_xCategories->get_widget())
    {
        m_xCategories->GroupSelected();
    }
    UpdateUI();
}

IMPL_LINK_NOARG(SvxScriptSelectorDialog, FunctionDoubleClickHdl, weld::TreeView&, bool)
{
    if (m_xOKButton->get_sensitive())
        ClickHdl(*m_xOKButton);
    return true;
}

IMPL_LINK(SvxScriptSelectorDialog, ContextMenuHdl, const CommandEvent&, rCEvt, bool)
{
    weld::TreeView& xTreeView = m_xCommands->get_widget();
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu || !xTreeView.n_children())
         return false;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(&xTreeView, u"modules/BasicIDE/ui/sortmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu(u"sortmenu"_ustr));
    std::unique_ptr<weld::Menu> xDropMenu(xBuilder->weld_menu(u"sortsubmenu"_ustr));
    xDropMenu->set_active(u"alphabetically"_ustr, xTreeView.get_sort_order());
    xDropMenu->set_active(u"properorder"_ustr, !xTreeView.get_sort_order());

    OUString sCommand(xPopup->popup_at_rect(&xTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1))));
    if (sCommand == "alphabetically")
    {
        xTreeView.make_sorted();
    }
    else if (sCommand == "properorder")
    {
        xTreeView.make_unsorted();
        m_xCategories->GroupSelected();
    }
    else if (!sCommand.isEmpty())
    {
         SAL_WARN("cui.customize", "Unknown context menu action: " << sCommand );
    }

    return true;
}

// Check if command is selected and enable the OK button accordingly
// Grab the help text for this id if available and update the description field
void
SvxScriptSelectorDialog::UpdateUI()
{
    OUString url = GetScriptURL();
    if ( !url.isEmpty() )
    {
        OUString sMessage = m_xCommands->GetCommandHelpText();
        m_xDescriptionText->set_text(sMessage.isEmpty() ? m_sDefaultDesc : sMessage);
        m_xOKButton->set_sensitive(true);
    }
    else
    {
        m_xDescriptionText->set_text(m_sDefaultDesc);
        m_xOKButton->set_sensitive(false);
    }
}

IMPL_LINK(SvxScriptSelectorDialog, ClickHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xCancelButton.get())
    {
        m_xDialog->response(RET_CANCEL);
    }
    else if (&rButton == m_xOKButton.get())
    {
        SaveLastUsedMacro();
        m_xDialog->response(RET_OK);
    }
}

void
SvxScriptSelectorDialog::SetRunLabel()
{
    m_xOKButton->set_label(CuiResId(RID_CUISTR_SELECTOR_RUN));
}

OUString
SvxScriptSelectorDialog::GetScriptURL() const
{
    OUString result;

    std::unique_ptr<weld::TreeIter> xIter = m_xCommands->make_iterator();
    if (m_xCommands->get_selected(xIter.get()))
    {
        SfxGroupInfo_Impl *pData = weld::fromId<SfxGroupInfo_Impl*>(m_xCommands->get_id(*xIter));
        if  (   ( pData->nKind == SfxCfgKind::FUNCTION_SLOT )
            ||  ( pData->nKind == SfxCfgKind::FUNCTION_SCRIPT )
            ||  ( pData->nKind == SfxCfgKind::GROUP_STYLES )
            )
        {
            result = pData->sCommand;
        }
    }

    return result;
}

void
SvxScriptSelectorDialog::SaveLastUsedMacro()
{
    // Gets the current selection in the dialog as a series of selected entries
    OUString sMacroInfo;
    sMacroInfo = m_xCommands->get_selected_text();
    weld::TreeView& xCategories = m_xCategories->get_widget();
    std::unique_ptr<weld::TreeIter> xIter = xCategories.make_iterator();

    if (!xCategories.get_selected(xIter.get()))
        return;

    do
    {
        sMacroInfo = xCategories.get_text(*xIter) + "|" + sMacroInfo;
    } while (xCategories.iter_parent(*xIter));

    SvtViewOptions( EViewType::Dialog, MACRO_SELECTOR_CONFIGNAME ).SetUserItem(
        LAST_RUN_MACRO_INFO, Any(sMacroInfo));
}

void
SvxScriptSelectorDialog::LoadLastUsedMacro()
{
    SvtViewOptions aDlgOpt( EViewType::Dialog, MACRO_SELECTOR_CONFIGNAME );
    if (!aDlgOpt.Exists())
        return;

    OUString sMacroInfo;
    aDlgOpt.GetUserItem(LAST_RUN_MACRO_INFO) >>= sMacroInfo;
    if (sMacroInfo.isEmpty())
        return;

    // Counts how many entries exist in the macro info string
    sal_Int16 nInfoParts = 0;
    sal_Int16 nLastIndex = sMacroInfo.indexOf('|');
    if (nLastIndex > -1)
    {
        nInfoParts = 1;
        while ( nLastIndex != -1 )
        {
            nInfoParts++;
            nLastIndex = sMacroInfo.indexOf('|', nLastIndex + 1);
        }
    }

    weld::TreeView& xCategories = m_xCategories->get_widget();
    std::unique_ptr<weld::TreeIter> xIter = xCategories.make_iterator();
    if (!xCategories.get_iter_first(*xIter))
        return;

    // Expand the nodes in the category tree
    OUString sNodeToExpand;
    bool bIsIterValid;
    sal_Int16 nOpenedNodes = 0;
    for (sal_Int16 i=0; i<nInfoParts - 1; i++)
    {
        sNodeToExpand = sMacroInfo.getToken(i, '|');
        bIsIterValid = true;
        while (bIsIterValid && xCategories.get_text(*xIter) != sNodeToExpand)
            bIsIterValid = xCategories.iter_next_sibling(*xIter);

        if (bIsIterValid)
        {
            xCategories.expand_row(*xIter);
            nOpenedNodes++;
        }
        if (xCategories.iter_has_child(*xIter))
            (void)xCategories.iter_children(*xIter);
        else if (nOpenedNodes < nInfoParts - 1)
            // If the number of levels in the tree is smaller than the
            // number of parts in the macro info string, then return
            return;
    }
    xCategories.select(*xIter);
    xCategories.scroll_to_row(*xIter);
    m_xCategories->GroupSelected();

    // Select the macro in the command tree
    weld::TreeView& xCommands = m_xCommands->get_widget();
    xIter = xCommands.make_iterator();
    if (!xCommands.get_iter_first(*xIter))
        return;

    OUString sMacroName = sMacroInfo.getToken(nInfoParts - 1, '|');
    bIsIterValid = true;
    while (bIsIterValid && xCommands.get_text(*xIter) != sMacroName)
        bIsIterValid = xCommands.iter_next_sibling(*xIter);

    if (bIsIterValid)
    {
        xCommands.scroll_to_row(*xIter);
        xCommands.select(*xIter);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
