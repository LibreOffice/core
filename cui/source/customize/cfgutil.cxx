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

#include "cfgutil.hxx"

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
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ui/theUICategoryDescription.hpp>

#include "acccfg.hrc"
#include "helpid.hrc"
#include <basic/sbx.hxx>
#include <basic/basicmanagerrepository.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/basmgr.hxx>
#include <tools/urlobj.hxx>
#include "cuires.hrc"
#include <sfx2/app.hxx>
#include <sfx2/minfitem.hxx>
#include <comphelper/documentinfo.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/string.hxx>
#include <svtools/imagemgr.hxx>
#include "svtools/treelistentry.hxx"
#include <rtl/ustrbuf.hxx>
#include <unotools/configmgr.hxx>
#include "dialmgr.hxx"
#include <svl/stritem.hxx>
#include <vcl/builderfactory.hxx>
#include <o3tl/make_unique.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;

SfxStylesInfo_Impl::SfxStylesInfo_Impl()
{}

void SfxStylesInfo_Impl::setModel(const css::uno::Reference< css::frame::XModel >& xModel)
{
    m_xDoc = xModel;
}

static const char CMDURL_SPART [] = ".uno:StyleApply?Style:string=";
static const char CMDURL_FPART2[] = "&FamilyName:string=";

static const char CMDURL_STYLEPROT_ONLY[] = ".uno:StyleApply?";
static const char CMDURL_SPART_ONLY    [] = "Style:string=";
static const char CMDURL_FPART_ONLY    [] = "FamilyName:string=";

static const char STYLEPROP_UINAME[] = "DisplayName";

OUString SfxStylesInfo_Impl::generateCommand(const OUString& sFamily, const OUString& sStyle)
{
    OUStringBuffer sCommand(1024);
    sCommand.append(CMDURL_SPART );
    sCommand.append(sStyle       );
    sCommand.append(CMDURL_FPART2);
    sCommand.append(sFamily      );
    return sCommand.makeStringAndClear();
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
        aStyle.sStyle = sArg.copy(LEN_SPART, sArg.getLength()-LEN_SPART);
    else if (sArg.startsWith(CMDURL_FPART_ONLY))
        aStyle.sFamily = sArg.copy(LEN_FPART, sArg.getLength()-LEN_FPART);

    sArg = sCmdArgs.copy(i+1, sCmdArgs.getLength()-i-1);
    if (sArg.startsWith(CMDURL_SPART_ONLY))
        aStyle.sStyle = sArg.copy(LEN_SPART, sArg.getLength()-LEN_SPART);
    else if (sArg.startsWith(CMDURL_FPART_ONLY))
        aStyle.sFamily = sArg.copy(LEN_FPART, sArg.getLength()-LEN_FPART);

    if (!(aStyle.sFamily.isEmpty() || aStyle.sStyle.isEmpty()))
        return true;

    return false;
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

::std::vector< SfxStyleInfo_Impl > SfxStylesInfo_Impl::getStyleFamilies()
{
    // Its an optional interface!
    css::uno::Reference< css::style::XStyleFamiliesSupplier > xModel(m_xDoc, css::uno::UNO_QUERY);
    if (!xModel.is())
        return ::std::vector< SfxStyleInfo_Impl >();

    css::uno::Reference< css::container::XNameAccess > xCont = xModel->getStyleFamilies();
    css::uno::Sequence< OUString > lFamilyNames = xCont->getElementNames();
    ::std::vector< SfxStyleInfo_Impl > lFamilies;
    sal_Int32 c = lFamilyNames.getLength();
    sal_Int32 i = 0;
    for(i=0; i<c; ++i)
    {
        SfxStyleInfo_Impl aFamilyInfo;
        aFamilyInfo.sFamily = lFamilyNames[i];

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
            { return ::std::vector< SfxStyleInfo_Impl >(); }

        lFamilies.push_back(aFamilyInfo);
    }

    return lFamilies;
}

::std::vector< SfxStyleInfo_Impl > SfxStylesInfo_Impl::getStyles(const OUString& sFamily)
{
    static const char PROP_UINAME[] = "DisplayName";

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
        { return ::std::vector< SfxStyleInfo_Impl >(); }

    ::std::vector< SfxStyleInfo_Impl > lStyles;
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
            xStyle->getPropertyValue(PROP_UINAME) >>= aStyleInfo.sLabel;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }

        lStyles.push_back(aStyleInfo);
    }
    return lStyles;
}

SfxConfigFunctionListBox::SfxConfigFunctionListBox(vcl::Window* pParent, WinBits nStyle)
    : SvTreeListBox( pParent, nStyle )
    , pStylesInfo( nullptr )
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    GetModel()->SetSortMode( SortAscending );
}

VCL_BUILDER_DECL_FACTORY(SfxConfigFunctionListBox)
{
    WinBits nWinBits = WB_TABSTOP;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<SfxConfigFunctionListBox>::Create(pParent, nWinBits);
}

SfxConfigFunctionListBox::~SfxConfigFunctionListBox()
{
    disposeOnce();
}

void SfxConfigFunctionListBox::dispose()
{
    ClearAll();
    SvTreeListBox::dispose();
}

void SfxConfigFunctionListBox::MouseMove( const MouseEvent& )
{
}

void SfxConfigFunctionListBox::ClearAll()
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
    Clear();
}

OUString SfxConfigFunctionListBox::GetSelectedScriptURI()
{
    SvTreeListEntry *pEntry = FirstSelected();
    if ( pEntry )
    {
        SfxGroupInfo_Impl *pData = static_cast<SfxGroupInfo_Impl*>(pEntry->GetUserData());
        if ( pData && ( pData->nKind == SfxCfgKind::FUNCTION_SCRIPT ) )
            return *static_cast<OUString*>(pData->pObject);
    }
    return OUString();
}

OUString SfxConfigFunctionListBox::GetCurCommand()
{
    SvTreeListEntry *pEntry = FirstSelected();
    if (!pEntry)
        return OUString();
    SfxGroupInfo_Impl *pData = static_cast<SfxGroupInfo_Impl*>(pEntry->GetUserData());
    if (!pData)
        return OUString();
    return pData->sCommand;
}

OUString SfxConfigFunctionListBox::GetCurLabel()
{
    SvTreeListEntry *pEntry = FirstSelected();
    if (!pEntry)
        return OUString();
    SfxGroupInfo_Impl *pData = static_cast<SfxGroupInfo_Impl*>(pEntry->GetUserData());
    if (!pData)
        return OUString();
    if (!pData->sLabel.isEmpty())
        return pData->sLabel;
    return pData->sCommand;
}

void SfxConfigFunctionListBox::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    pStylesInfo = pStyles;
}

struct SvxConfigGroupBoxResource_Impl : public Resource
{
    Image m_hdImage;
    Image m_libImage;
    Image m_macImage;
    Image m_docImage;
    OUString m_sMyMacros;
    OUString m_sProdMacros;
    OUString m_sMacros;
    OUString m_sDlgMacros;
    OUString m_aStrGroupStyles;
    Image m_collapsedImage;
    Image m_expandedImage;

    SvxConfigGroupBoxResource_Impl();
};

SvxConfigGroupBoxResource_Impl::SvxConfigGroupBoxResource_Impl() :
    Resource(CUI_RES(RID_SVXPAGE_CONFIGGROUPBOX)),
    m_hdImage(CUI_RES(RID_CUIIMG_HARDDISK)),
    m_libImage(CUI_RES(RID_CUIIMG_LIB)),
    m_macImage(CUI_RES(RID_CUIIMG_MACRO)),
    m_docImage(CUI_RES(RID_CUIIMG_DOC)),
    m_sMyMacros(CUI_RESSTR(RID_SVXSTR_MYMACROS)),
    m_sProdMacros(CUI_RESSTR(RID_SVXSTR_PRODMACROS)),
    m_sMacros(CUI_RESSTR(STR_BASICMACROS)),
    m_sDlgMacros(CUI_RESSTR(RID_SVXSTR_PRODMACROS)),
    m_aStrGroupStyles(CUI_RESSTR(STR_GROUP_STYLES)),
    m_collapsedImage(CUI_RES(BMP_COLLAPSED)),
    m_expandedImage(CUI_RES(BMP_EXPANDED))
{
    FreeResource();
}

SfxConfigGroupListBox::SfxConfigGroupListBox(vcl::Window* pParent, WinBits nStyle)
    : SvTreeListBox(pParent, nStyle)
    , pImp(new SvxConfigGroupBoxResource_Impl()), pFunctionListBox(nullptr), pStylesInfo(nullptr)
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT );
    SetNodeBitmaps( pImp->m_collapsedImage, pImp->m_expandedImage );
}

VCL_BUILDER_DECL_FACTORY(SfxConfigGroupListBox)
{
    WinBits nWinBits = WB_TABSTOP;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<SfxConfigGroupListBox>::Create(pParent, nWinBits);
}

SfxConfigGroupListBox::~SfxConfigGroupListBox()
{
    disposeOnce();
}

void SfxConfigGroupListBox::dispose()
{
    ClearAll();
    pFunctionListBox.clear();
    SvTreeListBox::dispose();
}

void SfxConfigGroupListBox::ClearAll()
{
    sal_uInt16 nCount = aArr.size();
    for ( sal_uInt16 i=0; i<nCount; ++i )
    {
        SfxGroupInfo_Impl *pData = aArr[i].get();
        if (pData->nKind == SfxCfgKind::GROUP_SCRIPTCONTAINER)
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != nullptr)
            {
                xi->release();
            }
        }
    }

    aArr.clear();
    Clear();
}

void SfxConfigGroupListBox::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    pStylesInfo = pStyles;
}


void SfxConfigGroupListBox::InitModule()
{
    try
    {
        css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider(m_xFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Sequence< sal_Int16 > lGroups = xProvider->getSupportedCommandGroups();
        sal_Int32                       c1      = lGroups.getLength();
        sal_Int32                       i1      = 0;

        for (i1=0; i1<c1; ++i1)
        {
            sal_Int16&      rGroupID   = lGroups[i1];
            OUString sGroupID   = OUString::number(rGroupID);
            OUString sGroupName ;

            try
            {
                m_xModuleCategoryInfo->getByName(sGroupID) >>= sGroupName;
                if (sGroupName.isEmpty())
                    continue;
            }
            catch(const css::container::NoSuchElementException&)
                { continue; }

            SvTreeListEntry*        pEntry = InsertEntry(sGroupName);
            SfxGroupInfo_Impl* pInfo   = new SfxGroupInfo_Impl(SfxCfgKind::GROUP_FUNCTION, rGroupID);
            pEntry->SetUserData(pInfo);
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}


namespace
{

    /** examines a component whether it supports XEmbeddedScripts, or provides access to such a
        component by implementing XScriptInvocationContext.
        @return
            the model which supports the embedded scripts, or <NULL/> if it cannot find such a
            model
    */
    static Reference< XModel > lcl_getDocumentWithScripts_throw( const Reference< XInterface >& _rxComponent )
    {
        Reference< XEmbeddedScripts > xScripts( _rxComponent, UNO_QUERY );
        if ( !xScripts.is() )
        {
            Reference< XScriptInvocationContext > xContext( _rxComponent, UNO_QUERY );
            if ( xContext.is() )
                xScripts.set( xContext->getScriptContainer(), UNO_QUERY );
        }

        return Reference< XModel >( xScripts, UNO_QUERY );
    }


    static Reference< XModel > lcl_getScriptableDocument_nothrow( const Reference< XFrame >& _rxFrame )
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


void SfxConfigGroupListBox::Init(const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const css::uno::Reference< css::frame::XFrame >& xFrame,
    const OUString& sModuleLongName,
    bool bEventMode)
{
    SetUpdateMode(false);
    ClearAll(); // Remove all old entries from treelist box

    m_xFrame = xFrame;
    if( xContext.is() )
    {
        m_xContext        = xContext;
        m_sModuleLongName = sModuleLongName;

        m_xGlobalCategoryInfo = css::ui::theUICategoryDescription::get( m_xContext );
        m_xModuleCategoryInfo.set(m_xGlobalCategoryInfo->getByName(m_sModuleLongName), css::uno::UNO_QUERY_THROW);
        m_xUICmdDescription   = css::frame::theUICommandDescription::get( m_xContext );

        InitModule();
    }

    SAL_INFO("cui.customize", "** ** About to initialise SF Scripts");
    // Add Scripting Framework entries
    Reference< browse::XBrowseNode > rootNode;
    Reference< XComponentContext > xCtx(
        comphelper::getProcessComponentContext() );
    try
    {
        Reference< browse::XBrowseNodeFactory > xFac = browse::theBrowseNodeFactory::get( xCtx );
        rootNode.set( xFac->createView( browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) );
    }
    catch( Exception& e )
    {
        SAL_INFO("cui.customize", "Caught some exception whilst retrieving browse nodes from factory... Exception: " << e.Message);
        // TODO exception handling
    }


    if ( rootNode.is() )
    {
        if ( bEventMode )
        {
                //We call acquire on the XBrowseNode so that it does not
                //get autodestructed and become invalid when accessed later.
            rootNode->acquire();

            aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_SCRIPTCONTAINER, 0,
                    static_cast<void *>(rootNode.get())));
            OUString aTitle(pImp->m_sDlgMacros);
            SvTreeListEntry *pNewEntry = InsertEntry( aTitle );
            pNewEntry->SetUserData( aArr.back().get() );
            pNewEntry->EnableChildrenOnDemand();
        }
        else
        {
             //We are only showing scripts not slot APIs so skip
             //Root node and show location nodes
            try {
                if ( rootNode->hasChildNodes() )
                {
                    Sequence< Reference< browse::XBrowseNode > > children =
                        rootNode->getChildNodes();
                    bool bIsRootNode = false;

                    OUString user("user");
                    OUString share("share");
                    if ( rootNode->getName() == "Root" )
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

                    for ( sal_Int32 n = 0; n < children.getLength(); ++n )
                    {
                        Reference< browse::XBrowseNode >& theChild = children[n];
                        bool bDisplay = true;
                        OUString uiName = theChild->getName();
                        if ( bIsRootNode )
                        {
                            if (  ! ((theChild->getName().equals( user )  || theChild->getName().equals( share ) ||
                                theChild->getName().equals( currentDocTitle ) ) ) )
                            {
                                bDisplay=false;
                            }
                            else
                            {
                                if ( uiName.equals( user ) )
                                {
                                    uiName = pImp->m_sMyMacros;
                                }
                                else if ( uiName.equals( share ) )
                                {
                                    uiName = pImp->m_sProdMacros;
                                }
                            }
                        }
                        if (children[n]->getType() != browse::BrowseNodeTypes::SCRIPT  && bDisplay )
                        {
//                              We call acquire on the XBrowseNode so that it does not
//                              get autodestructed and become invalid when accessed later.
                            theChild->acquire();

                            Image aImage = GetImage( theChild, xCtx, bIsRootNode );
                            SvTreeListEntry* pNewEntry =
                                InsertEntry( uiName );
                            SetExpandedEntryBmp(  pNewEntry, aImage );
                            SetCollapsedEntryBmp( pNewEntry, aImage );

                            aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>(SfxCfgKind::GROUP_SCRIPTCONTAINER,
                                    0, static_cast<void *>( theChild.get())));

                            pNewEntry->SetUserData( aArr.back().get() );

                            if ( children[n]->hasChildNodes() )
                            {
                                Sequence< Reference< browse::XBrowseNode > > grandchildren =
                                    children[n]->getChildNodes();

                                for ( sal_Int32 m = 0; m < grandchildren.getLength(); ++m )
                                {
                                    if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                                    {
                                        pNewEntry->EnableChildrenOnDemand();
                                        m = grandchildren.getLength();
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (RuntimeException&) {
                // do nothing, the entry will not be displayed in the UI
            }
        }
    }

    // add styles
    if ( m_xContext.is() )
    {
        OUString sStyle( pImp->m_aStrGroupStyles );
        SvTreeListEntry *pEntry = InsertEntry( sStyle );
        aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0, nullptr ) ); // TODO last parameter should contain user data
        pEntry->SetUserData( aArr.back().get() );
        pEntry->EnableChildrenOnDemand();
    }

    MakeVisible( GetEntry( nullptr,0 ) );
    SetUpdateMode( true );
}

Image SfxConfigGroupListBox::GetImage(
    Reference< browse::XBrowseNode > node,
    Reference< XComponentContext > xCtx,
    bool bIsRootNode
)
{
    Image aImage;
    if ( bIsRootNode )
    {
        OUString user("user");
        OUString share("share");
        if (node->getName().equals( user ) || node->getName().equals(share ) )
        {
            aImage = pImp->m_hdImage;
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
                    throw RuntimeException("SFTreeListBox::Init: failed to get PropertyValue");
                }
                beans::PropertyValue const * pmoduleDescr =
                    moduleDescr.getConstArray();
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if ( pmoduleDescr[ pos ].Name == "ooSetupFactoryEmptyDocumentURL" )
                    {
                        pmoduleDescr[ pos ].Value >>= factoryURL;
                        SAL_INFO("cui.customize", "factory url for doc images is " << factoryURL);
                        break;
                    }
                }
            }
            if( !factoryURL.isEmpty() )
            {
                aImage = SvFileInformationManager::GetFileImage( INetURLObject(factoryURL) );
            }
            else
            {
                aImage = pImp->m_docImage;
            }
        }
    }
    else
    {
        if( node->getType() == browse::BrowseNodeTypes::SCRIPT )
            aImage = pImp->m_macImage;
        else
            aImage = pImp->m_libImage;
    }
    return aImage;
}

Reference< XInterface  >
SfxConfigGroupListBox::getDocumentModel( Reference< XComponentContext >& xCtx, OUString& docName )
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
            if( sTdocUrl.equals( docName ) )
            {
                xModel = model;
                break;
            }
        }
    }
    return xModel;
}


OUString SfxConfigGroupListBox::MapCommand2UIName(const OUString& sCommand)
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


void SfxConfigGroupListBox::GroupSelected()
/*  Description
    A function group or a basic module has been selected.
    All functions/macros are displayed in the functionlistbox.
*/
{
    SvTreeListEntry *pEntry = FirstSelected();
    SfxGroupInfo_Impl *pInfo = static_cast<SfxGroupInfo_Impl*>(pEntry->GetUserData());
    pFunctionListBox->SetUpdateMode(false);
    pFunctionListBox->ClearAll();
    if ( pInfo->nKind != SfxCfgKind::GROUP_FUNCTION &&
             pInfo->nKind != SfxCfgKind::GROUP_SCRIPTCONTAINER &&
             pInfo->nKind != SfxCfgKind::GROUP_STYLES )
    {
        pFunctionListBox->SetUpdateMode(true);
        return;
    }

    switch ( pInfo->nKind )
    {
        case SfxCfgKind::GROUP_FUNCTION :
        {
            sal_uInt16                                                          nGroup    = pInfo->nUniqueID;
            css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider (m_xFrame, css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::frame::DispatchInformation >           lCommands = xProvider->getConfigurableDispatchInformation(nGroup);
            sal_Int32                                                       c         = lCommands.getLength();
            sal_Int32                                                       i         = 0;

            for (i=0; i<c; ++i)
            {
                const css::frame::DispatchInformation& rInfo      = lCommands[i];
                OUString                        sUIName    = MapCommand2UIName(rInfo.Command);
                SvTreeListEntry*                           pFuncEntry = pFunctionListBox->InsertEntry(sUIName);
                SfxGroupInfo_Impl*                     pGrpInfo   = new SfxGroupInfo_Impl(SfxCfgKind::FUNCTION_SLOT, 0);
                pGrpInfo->sCommand = rInfo.Command;
                pGrpInfo->sLabel   = sUIName;
                pFuncEntry->SetUserData(pGrpInfo);
            }

            break;
        }

        case SfxCfgKind::GROUP_SCRIPTCONTAINER:
        {
            if ( !GetChildCount( pEntry ) )
            {
                Reference< browse::XBrowseNode > rootNode(
                    static_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;

                try {
                    if ( rootNode->hasChildNodes() )
                    {
                        Sequence< Reference< browse::XBrowseNode > > children =
                            rootNode->getChildNodes();

                        for ( sal_Int32 n = 0; n < children.getLength(); ++n )
                        {
                            if (children[n]->getType() == browse::BrowseNodeTypes::SCRIPT)
                            {
                                OUString uri;

                                Reference < beans::XPropertySet >xPropSet( children[n], UNO_QUERY );
                                if (!xPropSet.is())
                                {
                                    continue;
                                }

                                Any value =
                                    xPropSet->getPropertyValue("URI");
                                value >>= uri;

                                OUString* pScriptURI = new OUString( uri );

                                Image aImage = GetImage( children[n], Reference< XComponentContext >(), false );
                                SvTreeListEntry* pNewEntry =
                                    pFunctionListBox->InsertEntry( children[n]->getName() );
                                pFunctionListBox->SetExpandedEntryBmp( pNewEntry, aImage );
                                pFunctionListBox->SetCollapsedEntryBmp(pNewEntry, aImage );

                                pFunctionListBox->aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::FUNCTION_SCRIPT, 0, pScriptURI ));
                                pFunctionListBox->aArr.back()->sCommand = uri;
                                pFunctionListBox->aArr.back()->sLabel = children[n]->getName();
                                pNewEntry->SetUserData( pFunctionListBox->aArr.back().get() );
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
                const ::std::vector< SfxStyleInfo_Impl > lStyles = pStylesInfo->getStyles(pFamily->sFamily);
                ::std::vector< SfxStyleInfo_Impl >::const_iterator pIt;
                for (  pIt  = lStyles.begin();
                       pIt != lStyles.end()  ;
                     ++pIt                   )
                {
                    SfxStyleInfo_Impl* pStyle = new SfxStyleInfo_Impl(*pIt);
                    SvTreeListEntry* pFuncEntry = pFunctionListBox->InsertEntry( pStyle->sLabel );
                    pFunctionListBox->aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0, pStyle ) );
                    pFunctionListBox->aArr.back()->sCommand = pStyle->sCommand;
                    pFunctionListBox->aArr.back()->sLabel = pStyle->sLabel;
                    pFuncEntry->SetUserData( pFunctionListBox->aArr.back().get() );
                }
            }
            break;
        }

        default:
            return;
    }

    if ( pFunctionListBox->GetEntryCount() )
        pFunctionListBox->Select( pFunctionListBox->GetEntry( nullptr, 0 ) );

    pFunctionListBox->SetUpdateMode(true);
}

bool SfxConfigGroupListBox::Expand( SvTreeListEntry* pParent )
{
    bool bRet = SvTreeListBox::Expand( pParent );
    if ( bRet )
    {
        sal_uLong nEntries = GetOutputSizePixel().Height() / GetEntryHeight();

        sal_uLong nChildCount = GetVisibleChildCount( pParent );

        if ( nChildCount+1 > nEntries )
        {
            MakeVisible( pParent, true );
        }
        else
        {
            SvTreeListEntry *pEntry = GetFirstEntryInView();
            sal_uLong nParentPos = 0;
            while ( pEntry && pEntry != pParent )
            {
                ++nParentPos;
                pEntry = GetNextEntryInView( pEntry );
            }

            if ( nParentPos + nChildCount + 1 > nEntries )
                ScrollOutputArea( (short)( nEntries - ( nParentPos + nChildCount + 1 ) ) );
        }
    }

    return bRet;
}

void SfxConfigGroupListBox::RequestingChildren( SvTreeListEntry *pEntry )
/*  Description
    A basic or a library is opened.
*/
{
    SfxGroupInfo_Impl *pInfo = static_cast<SfxGroupInfo_Impl*>(pEntry->GetUserData());
    pInfo->bWasOpened = true;
    switch ( pInfo->nKind )
    {
        case SfxCfgKind::GROUP_SCRIPTCONTAINER:
        {
            if ( !GetChildCount( pEntry ) )
            {
                Reference< browse::XBrowseNode > rootNode(
                    static_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;

                try {
                    if ( rootNode->hasChildNodes() )
                    {
                        Sequence< Reference< browse::XBrowseNode > > children =
                            rootNode->getChildNodes();
                        bool bIsRootNode = false;

                        OUString user("user");
                        OUString share("share" );
                        if ( rootNode->getName() == "Root" )
                        {
                            bIsRootNode = true;
                        }

                        /* To mimic current starbasic behaviour we
                        need to make sure that only the current document
                        is displayed in the config tree. Tests below
                        set the bDisplay flag to sal_False if the current
                        node is a first level child of the Root and is NOT
                        either the current document, user or share */
                        OUString currentDocTitle;
                        Reference< XModel > xDocument( lcl_getScriptableDocument_nothrow( m_xFrame ) );
                        if ( xDocument.is() )
                        {
                            currentDocTitle = ::comphelper::DocumentInfo::getDocumentTitle( xDocument );
                        }

                        sal_Int32 nLen = children.getLength();
                        for ( sal_Int32 n = 0; n < nLen; ++n )
                        {
                            Reference< browse::XBrowseNode >& theChild = children[n];
                            OUString aName( theChild->getName() );
                            bool bDisplay = true;
                            if ( bIsRootNode )
                            {
                                if ( !( (aName.equals(user) || aName.equals(share) || aName.equals(currentDocTitle) ) ) )
                                    bDisplay=false;
                            }
                            if ( children[n].is() && children[n]->getType() != browse::BrowseNodeTypes::SCRIPT && bDisplay )
                            {

                                /*
                                    We call acquire on the XBrowseNode so that it does not
                                    get autodestructed and become invalid when accessed later.
                                */
                                theChild->acquire();

                                Image aImage = GetImage( theChild, Reference< XComponentContext >(), false );
                                SvTreeListEntry* pNewEntry =
                                    InsertEntry( theChild->getName(), pEntry );
                                SetExpandedEntryBmp( pNewEntry, aImage );
                                SetCollapsedEntryBmp(pNewEntry, aImage );

                                aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>(
                                    SfxCfgKind::GROUP_SCRIPTCONTAINER,
                                        0, static_cast<void *>( theChild.get())));

                                pNewEntry->SetUserData( aArr.back().get() );

                                if ( children[n]->hasChildNodes() )
                                {
                                    Sequence< Reference< browse::XBrowseNode > > grandchildren =
                                        children[n]->getChildNodes();

                                    for ( sal_Int32 m = 0; m < grandchildren.getLength(); ++m )
                                    {
                                        if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                                        {
                                            pNewEntry->EnableChildrenOnDemand();
                                            m = grandchildren.getLength();
                                        }
                                    }
                                }
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

        case SfxCfgKind::GROUP_STYLES:
        {
            if ( !GetChildCount( pEntry ) )
            {
                const ::std::vector< SfxStyleInfo_Impl >                 lStyleFamilies = pStylesInfo->getStyleFamilies();
                      ::std::vector< SfxStyleInfo_Impl >::const_iterator pIt;
                for (  pIt  = lStyleFamilies.begin();
                       pIt != lStyleFamilies.end()  ;
                     ++pIt                          )
                {
                    SfxStyleInfo_Impl* pFamily = new SfxStyleInfo_Impl(*pIt);
                    SvTreeListEntry* pStyleEntry = InsertEntry( pFamily->sLabel, pEntry );
                    aArr.push_back( o3tl::make_unique<SfxGroupInfo_Impl>( SfxCfgKind::GROUP_STYLES, 0, pFamily ));
                    pStyleEntry->SetUserData( aArr.back().get() );
                    pStyleEntry->EnableChildrenOnDemand( false );
                }
            }
            break;
        }

        default:
            OSL_FAIL( "Falscher Gruppentyp!" );
            break;
    }
}

void SfxConfigGroupListBox::SelectMacro( const SfxMacroInfoItem *pItem )
{
    SelectMacro( pItem->GetBasicManager()->GetName(),
                 pItem->GetQualifiedName() );
}

void SfxConfigGroupListBox::SelectMacro( const OUString& rBasic,
         const OUString& rMacro )
{
    const OUString aBasicName( rBasic + " " + pImp->m_sMacros );
    const sal_Int32 nCount = comphelper::string::getTokenCount(rMacro, '.');
    const OUString aMethod( rMacro.getToken( nCount-1, '.' ) );
    OUString aLib;
    OUString aModule;
    if ( nCount > 2 )
    {
        aLib = rMacro.getToken( 0, '.' );
        aModule = rMacro.getToken( nCount-2, '.' );
    }

    SvTreeListEntry *pEntry = FirstChild(nullptr);
    while ( pEntry )
    {
        OUString aEntryBas = GetEntryText( pEntry );
        if ( aEntryBas == aBasicName )
        {
            Expand( pEntry );
            SvTreeListEntry *pLib = FirstChild( pEntry );
            while ( pLib )
            {
                OUString aEntryLib = GetEntryText( pLib );
                if ( aEntryLib == aLib )
                {
                    Expand( pLib );
                    SvTreeListEntry *pMod = FirstChild( pLib );
                    while ( pMod )
                    {
                        OUString aEntryMod = GetEntryText( pMod );
                        if ( aEntryMod == aModule )
                        {
                            Expand( pMod );
                            MakeVisible( pMod );
                            Select( pMod );
                            SvTreeListEntry *pMethod = pFunctionListBox->First();
                            while ( pMethod )
                            {
                                OUString aEntryMethod = GetEntryText( pMethod );
                                if ( aEntryMethod == aMethod )
                                {
                                    pFunctionListBox->Select( pMethod );
                                    pFunctionListBox->MakeVisible( pMethod );
                                    return;
                                }
                                pMethod = pFunctionListBox->Next( pMethod );
                            }
                        }
                        pMod = NextSibling( pMod );
                    }
                }
                pLib = NextSibling( pLib );
            }
        }
        pEntry = NextSibling( pEntry );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
