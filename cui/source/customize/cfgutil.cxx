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
#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/script/browse/BrowseNodeTypes.hpp>

#include <com/sun/star/script/browse/XBrowseNodeFactory.hpp>
#include <com/sun/star/script/browse/BrowseNodeFactoryViewTypes.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/frame/UICommandDescription.hpp>

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
#include <rtl/ustrbuf.hxx>
#include <unotools/configmgr.hxx>
#include "dialmgr.hxx"
#include <svl/stritem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
namespace css = ::com::sun::star;

static ::rtl::OUString SERVICE_UICATEGORYDESCRIPTION (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.UICategoryDescription") );

SfxStylesInfo_Impl::SfxStylesInfo_Impl()
{}

void SfxStylesInfo_Impl::setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel)
{
    m_xDoc = xModel;
}

static ::rtl::OUString CMDURL_SPART  (RTL_CONSTASCII_USTRINGPARAM(".uno:StyleApply?Style:string=") );
static ::rtl::OUString CMDURL_FPART2 (RTL_CONSTASCII_USTRINGPARAM("&FamilyName:string=") );

static ::rtl::OUString CMDURL_STYLEPROT_ONLY (RTL_CONSTASCII_USTRINGPARAM(".uno:StyleApply?") );
static ::rtl::OUString CMDURL_SPART_ONLY     (RTL_CONSTASCII_USTRINGPARAM("Style:string=") );
static ::rtl::OUString CMDURL_FPART_ONLY     (RTL_CONSTASCII_USTRINGPARAM("FamilyName:string=") );

static ::rtl::OUString STYLEPROP_UINAME (RTL_CONSTASCII_USTRINGPARAM("DisplayName") );

::rtl::OUString SfxStylesInfo_Impl::generateCommand(const ::rtl::OUString& sFamily, const ::rtl::OUString& sStyle)
{
    ::rtl::OUStringBuffer sCommand(1024);
    sCommand.append(CMDURL_SPART );
    sCommand.append(sStyle       );
    sCommand.append(CMDURL_FPART2);
    sCommand.append(sFamily      );
    return sCommand.makeStringAndClear();
}

sal_Bool SfxStylesInfo_Impl::parseStyleCommand(SfxStyleInfo_Impl& aStyle)
{
    static sal_Int32 LEN_STYLEPROT = CMDURL_STYLEPROT_ONLY.getLength();
    static sal_Int32 LEN_SPART     = CMDURL_SPART_ONLY.getLength();
    static sal_Int32 LEN_FPART     = CMDURL_FPART_ONLY.getLength();

    if (aStyle.sCommand.indexOf(CMDURL_STYLEPROT_ONLY, 0) != 0)
        return sal_False;

    aStyle.sFamily = ::rtl::OUString();
    aStyle.sStyle  = ::rtl::OUString();

    sal_Int32       nCmdLen  = aStyle.sCommand.getLength();
    ::rtl::OUString sCmdArgs = aStyle.sCommand.copy(LEN_STYLEPROT, nCmdLen-LEN_STYLEPROT);
    sal_Int32       i        = sCmdArgs.indexOf('&');
    if (i<0)
        return sal_False;

    ::rtl::OUString sArg = sCmdArgs.copy(0, i);
    if (sArg.indexOf(CMDURL_SPART_ONLY) == 0)
        aStyle.sStyle = sArg.copy(LEN_SPART, sArg.getLength()-LEN_SPART);
    else
    if (sArg.indexOf(CMDURL_FPART_ONLY) == 0)
        aStyle.sFamily = sArg.copy(LEN_FPART, sArg.getLength()-LEN_FPART);

    sArg = sCmdArgs.copy(i+1, sCmdArgs.getLength()-i-1);
    if (sArg.indexOf(CMDURL_SPART_ONLY) == 0)
        aStyle.sStyle = sArg.copy(LEN_SPART, sArg.getLength()-LEN_SPART);
    else
    if (sArg.indexOf(CMDURL_FPART_ONLY) == 0)
        aStyle.sFamily = sArg.copy(LEN_FPART, sArg.getLength()-LEN_FPART);

    if (!(aStyle.sFamily.isEmpty() || aStyle.sStyle.isEmpty()))
        return sal_True;

    return sal_False;
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

        aStyle.sLabel = ::rtl::OUString();
        if (xStyle.is())
            xStyle->getPropertyValue(STYLEPROP_UINAME) >>= aStyle.sLabel;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { aStyle.sLabel = ::rtl::OUString(); }

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
    css::uno::Sequence< ::rtl::OUString > lFamilyNames = xCont->getElementNames();
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

::std::vector< SfxStyleInfo_Impl > SfxStylesInfo_Impl::getStyles(const ::rtl::OUString& sFamily)
{
    static ::rtl::OUString PROP_UINAME (RTL_CONSTASCII_USTRINGPARAM("DisplayName") );

    css::uno::Sequence< ::rtl::OUString > lStyleNames;
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

SfxConfigFunctionListBox_Impl::SfxConfigFunctionListBox_Impl( Window* pParent, const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
    , pCurEntry( 0 )
    , pStylesInfo( 0 )
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    GetModel()->SetSortMode( SortAscending );

    // Timer for the BallonHelp
    aTimer.SetTimeout( 200 );
    aTimer.SetTimeoutHdl(
        LINK( this, SfxConfigFunctionListBox_Impl, TimerHdl ) );
}

SfxConfigFunctionListBox_Impl::~SfxConfigFunctionListBox_Impl()
{
    ClearAll();
}

void SfxConfigFunctionListBox_Impl::MouseMove( const MouseEvent& )
{
}

IMPL_LINK( SfxConfigFunctionListBox_Impl, TimerHdl, Timer*, pTimer)
/*  Description
    Timer-handler for showing a help-text. If the mouse pointer is
    still on the currently selected entry after the timer has run out,
    the entry's help-text is shown as a balloon-help.
*/
{
    (void)pTimer; // unused
    return 0L;
}

void SfxConfigFunctionListBox_Impl::ClearAll()
/*  Description
    Deletes all entries in the FunctionListBox, all UserData and all
    possibly existing MacroInfo.
*/
{
    sal_uInt16 nCount = aArr.size();
    for ( sal_uInt16 i=0; i<nCount; ++i )
    {
        SfxGroupInfo_Impl *pData = &aArr[i];

        if ( pData->nKind == SFX_CFGFUNCTION_SCRIPT )
        {
            String* pScriptURI = (String*)pData->pObject;
            delete pScriptURI;
        }

        if ( pData->nKind == SFX_CFGGROUP_SCRIPTCONTAINER )
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != NULL)
            {
                xi->release();
            }
        }
    }

    aArr.clear();
    Clear();
}

String SfxConfigFunctionListBox_Impl::GetSelectedScriptURI()
{
    SvLBoxEntry *pEntry = FirstSelected();
    if ( pEntry )
    {
        SfxGroupInfo_Impl *pData = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pData && ( pData->nKind == SFX_CFGFUNCTION_SCRIPT ) )
            return *(String*)pData->pObject;
    }
    return String();
}

String SfxConfigFunctionListBox_Impl::GetCurCommand()
{
    SvLBoxEntry *pEntry = FirstSelected();
    if (!pEntry)
        return String();
    SfxGroupInfo_Impl *pData = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    if (!pData)
        return String();
    return pData->sCommand;
}

String SfxConfigFunctionListBox_Impl::GetCurLabel()
{
    SvLBoxEntry *pEntry = FirstSelected();
    if (!pEntry)
        return String();
    SfxGroupInfo_Impl *pData = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    if (!pData)
        return String();
    if (pData->sLabel.Len())
        return pData->sLabel;
    return pData->sCommand;
}

void SfxConfigFunctionListBox_Impl::FunctionSelected()
/*  Description
    Resets the balloon-help because it shall
    always show the help-text of the selected entry.
*/
{
}

void SfxConfigFunctionListBox_Impl::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    pStylesInfo = pStyles;
}

struct SvxConfigGroupBoxResource_Impl : public Resource
{
    Image m_hdImage;
    Image m_libImage;
    Image m_macImage;
    Image m_docImage;
    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;
    String m_sMacros;
    String m_sDlgMacros;
    String m_aHumanAppName;
    String m_aStrGroupStyles;
    Image m_collapsedImage;
    Image m_expandedImage;

    SvxConfigGroupBoxResource_Impl();
};

SvxConfigGroupBoxResource_Impl::SvxConfigGroupBoxResource_Impl() :
    Resource(CUI_RES(RID_SVXPAGE_CONFIGGROUPBOX)),
    m_hdImage(CUI_RES(IMG_HARDDISK)),
    m_libImage(CUI_RES(IMG_LIB)),
    m_macImage(CUI_RES(IMG_MACRO)),
    m_docImage(CUI_RES(IMG_DOC)),
    m_sMyMacros(String(CUI_RES(STR_MYMACROS))),
    m_sProdMacros(String(CUI_RES(STR_PRODMACROS))),
    m_sMacros(String(CUI_RES(STR_BASICMACROS))),
    m_sDlgMacros(String(CUI_RES(STR_DLG_MACROS))),
    m_aHumanAppName(String(CUI_RES(STR_HUMAN_APPNAME))),
    m_aStrGroupStyles(String(CUI_RES(STR_GROUP_STYLES))),
    m_collapsedImage(CUI_RES(BMP_COLLAPSED)),
    m_expandedImage(CUI_RES(BMP_EXPANDED))
{
    FreeResource();
}

SfxConfigGroupListBox_Impl::SfxConfigGroupListBox_Impl(
    Window* pParent, const ResId& rResId, sal_uLong nConfigMode )
        : SvTreeListBox( pParent, rResId )
        , pImp(new SvxConfigGroupBoxResource_Impl()), pFunctionListBox(0), nMode( nConfigMode ), pStylesInfo(0)
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT );
    SetNodeBitmaps( pImp->m_collapsedImage, pImp->m_expandedImage );
}


SfxConfigGroupListBox_Impl::~SfxConfigGroupListBox_Impl()
{
    ClearAll();
}

void SfxConfigGroupListBox_Impl::ClearAll()
{
    sal_uInt16 nCount = aArr.size();
    for ( sal_uInt16 i=0; i<nCount; ++i )
    {
        SfxGroupInfo_Impl *pData = &aArr[i];
        if (pData->nKind == SFX_CFGGROUP_SCRIPTCONTAINER)
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != NULL)
            {
                xi->release();
            }
        }
    }

    aArr.clear();
    Clear();
}

void SfxConfigGroupListBox_Impl::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    pStylesInfo = pStyles;
}

String SfxConfigGroupListBox_Impl::GetGroup()
/*  Description
    Returns the name of the selected function group/the selected basic.
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    while ( pEntry )
    {
        SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pInfo->nKind == SFX_CFGGROUP_FUNCTION )
            return GetEntryText( pEntry );

        pEntry = GetParent( pEntry );
    }

    return String();
}

//-----------------------------------------------
void SfxConfigGroupListBox_Impl::InitModule()
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
            ::rtl::OUString sGroupID   = ::rtl::OUString::valueOf((sal_Int32)rGroupID);
            ::rtl::OUString sGroupName ;

            try
            {
                m_xModuleCategoryInfo->getByName(sGroupID) >>= sGroupName;
                if (sGroupName.isEmpty())
                    continue;
            }
            catch(const css::container::NoSuchElementException&)
                { continue; }

            SvLBoxEntry*        pEntry = InsertEntry(sGroupName, NULL);
            SfxGroupInfo_Impl* pInfo   = new SfxGroupInfo_Impl(SFX_CFGGROUP_FUNCTION, rGroupID);
            pEntry->SetUserData(pInfo);
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

//-----------------------------------------------
void SfxConfigGroupListBox_Impl::InitBasic()
{
}

//-----------------------------------------------
void SfxConfigGroupListBox_Impl::InitStyles()
{
}

//-----------------------------------------------
namespace
{
    //...........................................
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

    //...........................................
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

//-----------------------------------------------
void SfxConfigGroupListBox_Impl::Init(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR          ,
                                      const css::uno::Reference< css::frame::XFrame >&              xFrame         ,
                                      const ::rtl::OUString&                                        sModuleLongName)
{
    SetUpdateMode(sal_False);
    ClearAll(); // Remove all old entries from treelist box

    m_xFrame = xFrame;
    if ( xSMGR.is())
    {
        m_xSMGR           = xSMGR;
        m_sModuleLongName = sModuleLongName;

        m_xGlobalCategoryInfo = css::uno::Reference< css::container::XNameAccess >(m_xSMGR->createInstance(SERVICE_UICATEGORYDESCRIPTION), css::uno::UNO_QUERY_THROW);
        m_xModuleCategoryInfo = css::uno::Reference< css::container::XNameAccess >(m_xGlobalCategoryInfo->getByName(m_sModuleLongName)   , css::uno::UNO_QUERY_THROW);
        m_xUICmdDescription   = css::frame::UICommandDescription::create(
                ::comphelper::getComponentContext(m_xSMGR));

        InitModule();
        InitBasic();
        InitStyles();
    }

    OSL_TRACE("** ** About to initialise SF Scripts");
    // Add Scripting Framework entries
    Reference< browse::XBrowseNode > rootNode;
    Reference< XComponentContext > xCtx(
        comphelper::getProcessComponentContext() );
    try
    {
        Reference< browse::XBrowseNodeFactory > xFac( xCtx->getValueByName(
           ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/singletons/com.sun.star.script.browse.theBrowseNodeFactory") ) ), UNO_QUERY_THROW );
        rootNode.set( xFac->createView( browse::BrowseNodeFactoryViewTypes::MACROSELECTOR ) );
    }
    catch( Exception& e )
    {
        OSL_TRACE(" Caught some exception whilst retrieving browse nodes from factory... Exception: %s",
            ::rtl::OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
        // TODO exception handling
    }


    if ( rootNode.is() )
    {
        if ( nMode )
        {
                //We call acquire on the XBrowseNode so that it does not
                //get autodestructed and become invalid when accessed later.
            rootNode->acquire();

            SfxGroupInfo_Impl *pInfo =
                new SfxGroupInfo_Impl( SFX_CFGGROUP_SCRIPTCONTAINER, 0,
                    static_cast<void *>(rootNode.get()));

            String aTitle(pImp->m_sDlgMacros);
            SvLBoxEntry *pNewEntry = InsertEntry( aTitle, NULL );
            pNewEntry->SetUserData( pInfo );
            pNewEntry->EnableChildrenOnDemand( sal_True );
            aArr.push_back( pInfo );
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
                    sal_Bool bIsRootNode = sal_False;

                    ::rtl::OUString user( RTL_CONSTASCII_USTRINGPARAM("user") );
                    ::rtl::OUString share( RTL_CONSTASCII_USTRINGPARAM("share") );
                    if ( rootNode->getName() == "Root" )
                    {
                        bIsRootNode = sal_True;
                    }

                    //To mimic current starbasic behaviour we
                    //need to make sure that only the current document
                    //is displayed in the config tree. Tests below
                    //set the bDisplay flag to FALSE if the current
                    //node is a first level child of the Root and is NOT
                    //either the current document, user or share
                    ::rtl::OUString currentDocTitle;
                    Reference< XModel > xDocument( lcl_getScriptableDocument_nothrow( m_xFrame ) );
                    if ( xDocument.is() )
                    {
                        currentDocTitle = ::comphelper::DocumentInfo::getDocumentTitle( xDocument );
                    }

                    for ( sal_Int32 n = 0; n < children.getLength(); ++n )
                    {
                        Reference< browse::XBrowseNode >& theChild = children[n];
                        sal_Bool bDisplay = sal_True;
                        ::rtl::OUString uiName = theChild->getName();
                        if ( bIsRootNode )
                        {
                            if (  ! ((theChild->getName().equals( user )  || theChild->getName().equals( share ) ||
                                theChild->getName().equals( currentDocTitle ) ) ) )
                            {
                                bDisplay=sal_False;
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

                            SfxGroupInfo_Impl* pInfo =
                                new SfxGroupInfo_Impl(SFX_CFGGROUP_SCRIPTCONTAINER,
                                    0, static_cast<void *>( theChild.get()));

                            Image aImage = GetImage( theChild, xCtx, bIsRootNode );
                            SvLBoxEntry* pNewEntry =
                                InsertEntry( uiName, NULL);
                            SetExpandedEntryBmp(  pNewEntry, aImage );
                            SetCollapsedEntryBmp( pNewEntry, aImage );

                            pNewEntry->SetUserData( pInfo );
                            aArr.push_back( pInfo );

                            if ( children[n]->hasChildNodes() )
                            {
                                Sequence< Reference< browse::XBrowseNode > > grandchildren =
                                    children[n]->getChildNodes();

                                for ( sal_Int32 m = 0; m < grandchildren.getLength(); ++m )
                                {
                                    if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                                    {
                                        pNewEntry->EnableChildrenOnDemand( sal_True );
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
    if ( m_xSMGR.is() )
    {
        String sStyle( pImp->m_aStrGroupStyles );
        SvLBoxEntry *pEntry = InsertEntry( sStyle, 0 );
        SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_STYLES, 0, 0 ); // TODO last parameter should contain user data
        aArr.push_back( pInfo );
        pEntry->SetUserData( pInfo );
        pEntry->EnableChildrenOnDemand( sal_True );
    }

    MakeVisible( GetEntry( 0,0 ) );
    SetUpdateMode( sal_True );
}
Image SfxConfigGroupListBox_Impl::GetImage(
    Reference< browse::XBrowseNode > node,
    Reference< XComponentContext > xCtx,
    bool bIsRootNode
)
{
    Image aImage;
    if ( bIsRootNode )
    {
        ::rtl::OUString user( RTL_CONSTASCII_USTRINGPARAM("user") );
        ::rtl::OUString share( RTL_CONSTASCII_USTRINGPARAM("share") );
        if (node->getName().equals( user ) || node->getName().equals(share ) )
        {
            aImage = pImp->m_hdImage;
        }
        else
        {
            ::rtl::OUString factoryURL;
            ::rtl::OUString nodeName = node->getName();
            Reference<XInterface> xDocumentModel = getDocumentModel(xCtx, nodeName );
            if ( xDocumentModel.is() )
            {
                Reference< frame::XModuleManager2 > xModuleManager( frame::ModuleManager::create(xCtx) );
                // get the long name of the document:
                ::rtl::OUString appModule( xModuleManager->identify(
                                    xDocumentModel ) );
                Sequence<beans::PropertyValue> moduleDescr;
                Any aAny = xModuleManager->getByName(appModule);
                if( sal_True != ( aAny >>= moduleDescr ) )
                {
                    throw RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SFTreeListBox::Init: failed to get PropertyValue") ), Reference< XInterface >());
                }
                beans::PropertyValue const * pmoduleDescr =
                    moduleDescr.getConstArray();
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if ( pmoduleDescr[ pos ].Name == "ooSetupFactoryEmptyDocumentURL" )
                    {
                        pmoduleDescr[ pos ].Value >>= factoryURL;
                        OSL_TRACE("factory url for doc images is %s",
                        ::rtl::OUStringToOString( factoryURL , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                        break;
                    }
                }
            }
            if( !factoryURL.isEmpty() )
            {
                aImage = SvFileInformationManager::GetFileImage( INetURLObject(factoryURL), false );
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
SfxConfigGroupListBox_Impl::getDocumentModel( Reference< XComponentContext >& xCtx, ::rtl::OUString& docName )
{
    Reference< XInterface > xModel;
    Reference< lang::XMultiComponentFactory > mcf =
            xCtx->getServiceManager();
    Reference< frame::XDesktop > desktop (
        mcf->createInstanceWithContext(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop") ),                 xCtx ),
            UNO_QUERY );

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
            ::rtl::OUString sTdocUrl =
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

//-----------------------------------------------
::rtl::OUString SfxConfigGroupListBox_Impl::MapCommand2UIName(const ::rtl::OUString& sCommand)
{
    ::rtl::OUString sUIName;
    try
    {
        css::uno::Reference< css::container::XNameAccess > xModuleConf;
        m_xUICmdDescription->getByName(m_sModuleLongName) >>= xModuleConf;
        if (xModuleConf.is())
        {
            ::comphelper::SequenceAsHashMap lProps(xModuleConf->getByName(sCommand));
            sUIName = lProps.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name") ), ::rtl::OUString());
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(css::uno::Exception&)
        { sUIName = ::rtl::OUString(); }

    // fallback for missing UINames !?
    if (sUIName.isEmpty())
    {
        sUIName = sCommand;
    }

    return sUIName;
}

//-----------------------------------------------
void SfxConfigGroupListBox_Impl::GroupSelected()
/*  Description
    A function group or a basic module has been selected.
    All functions/macros are displayed in the functionlistbox.
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    pFunctionListBox->SetUpdateMode(sal_False);
    pFunctionListBox->ClearAll();
    if ( pInfo->nKind != SFX_CFGGROUP_FUNCTION &&
             pInfo->nKind != SFX_CFGGROUP_SCRIPTCONTAINER &&
             pInfo->nKind != SFX_CFGGROUP_STYLES )
    {
        pFunctionListBox->SetUpdateMode(sal_True);
        return;
    }

    switch ( pInfo->nKind )
    {
        case SFX_CFGGROUP_FUNCTION :
        {
            sal_uInt16                                                          nGroup    = pInfo->nUniqueID;
            css::uno::Reference< css::frame::XDispatchInformationProvider > xProvider (m_xFrame, css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::frame::DispatchInformation >           lCommands = xProvider->getConfigurableDispatchInformation(nGroup);
            sal_Int32                                                       c         = lCommands.getLength();
            sal_Int32                                                       i         = 0;

            for (i=0; i<c; ++i)
            {
                const css::frame::DispatchInformation& rInfo      = lCommands[i];
                ::rtl::OUString                        sUIName    = MapCommand2UIName(rInfo.Command);
                SvLBoxEntry*                           pFuncEntry = pFunctionListBox->InsertEntry(sUIName, NULL);
                SfxGroupInfo_Impl*                     pGrpInfo   = new SfxGroupInfo_Impl(SFX_CFGFUNCTION_SLOT, 0);
                pGrpInfo->sCommand = rInfo.Command;
                pGrpInfo->sLabel   = sUIName;
                pFuncEntry->SetUserData(pGrpInfo);
            }

            break;
        }

        case SFX_CFGGROUP_SCRIPTCONTAINER:
        {
            if ( !GetChildCount( pEntry ) )
            {
                Reference< browse::XBrowseNode > rootNode(
                    reinterpret_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;

                try {
                    if ( rootNode->hasChildNodes() )
                    {
                        Sequence< Reference< browse::XBrowseNode > > children =
                            rootNode->getChildNodes();

                        for ( sal_Int32 n = 0; n < children.getLength(); ++n )
                        {
                            if (children[n]->getType() == browse::BrowseNodeTypes::SCRIPT)
                            {
                                ::rtl::OUString uri;

                                Reference < beans::XPropertySet >xPropSet( children[n], UNO_QUERY );
                                if (!xPropSet.is())
                                {
                                    continue;
                                }

                                Any value =
                                    xPropSet->getPropertyValue( rtl::OUString("URI") );
                                value >>= uri;

                                String* pScriptURI = new String( uri );
                                SfxGroupInfo_Impl* pGrpInfo = new SfxGroupInfo_Impl( SFX_CFGFUNCTION_SCRIPT, 0, pScriptURI );

                                Image aImage = GetImage( children[n], Reference< XComponentContext >(), sal_False );
                                SvLBoxEntry* pNewEntry =
                                    pFunctionListBox->InsertEntry( children[n]->getName(), NULL );
                                pFunctionListBox->SetExpandedEntryBmp( pNewEntry, aImage );
                                pFunctionListBox->SetCollapsedEntryBmp(pNewEntry, aImage );

                                pGrpInfo->sCommand = uri;
                                pGrpInfo->sLabel = children[n]->getName();
                                pNewEntry->SetUserData( pGrpInfo );

                                pFunctionListBox->aArr.push_back( pGrpInfo );

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

        case SFX_CFGGROUP_STYLES :
        {
            SfxStyleInfo_Impl* pFamily = (SfxStyleInfo_Impl*)(pInfo->pObject);
            if (pFamily)
            {
                const ::std::vector< SfxStyleInfo_Impl > lStyles = pStylesInfo->getStyles(pFamily->sFamily);
                ::std::vector< SfxStyleInfo_Impl >::const_iterator pIt;
                for (  pIt  = lStyles.begin();
                       pIt != lStyles.end()  ;
                     ++pIt                   )
                {
                    SfxStyleInfo_Impl* pStyle = new SfxStyleInfo_Impl(*pIt);
                    SvLBoxEntry* pFuncEntry = pFunctionListBox->InsertEntry( pStyle->sLabel, NULL );
                    SfxGroupInfo_Impl *pGrpInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_STYLES, 0, pStyle );
                    pFunctionListBox->aArr.push_back( pGrpInfo );
                    pGrpInfo->sCommand = pStyle->sCommand;
                    pGrpInfo->sLabel = pStyle->sLabel;
                    pFuncEntry->SetUserData( pGrpInfo );
                }
            }
            break;
        }

        default:
            return;
    }

    if ( pFunctionListBox->GetEntryCount() )
        pFunctionListBox->Select( pFunctionListBox->GetEntry( 0, 0 ) );

    pFunctionListBox->SetUpdateMode(sal_True);
}

sal_Bool SfxConfigGroupListBox_Impl::Expand( SvLBoxEntry* pParent )
{
    sal_Bool bRet = SvTreeListBox::Expand( pParent );
    if ( bRet )
    {
        sal_uLong nEntries = GetOutputSizePixel().Height() / GetEntryHeight();

        sal_uLong nChildCount = GetVisibleChildCount( pParent );

        if ( nChildCount+1 > nEntries )
        {
            MakeVisible( pParent, sal_True );
        }
        else
        {
            SvLBoxEntry *pEntry = GetFirstEntryInView();
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

void SfxConfigGroupListBox_Impl::RequestingChildren( SvLBoxEntry *pEntry )
/*  Description
    A basic or a library is opened.
*/
{
    SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    pInfo->bWasOpened = sal_True;
    switch ( pInfo->nKind )
    {
        case SFX_CFGGROUP_SCRIPTCONTAINER:
        {
            if ( !GetChildCount( pEntry ) )
            {
                Reference< browse::XBrowseNode > rootNode(
                    reinterpret_cast< browse::XBrowseNode* >( pInfo->pObject ) ) ;

                try {
                    if ( rootNode->hasChildNodes() )
                    {
                        Sequence< Reference< browse::XBrowseNode > > children =
                            rootNode->getChildNodes();
                        sal_Bool bIsRootNode = sal_False;

                        ::rtl::OUString user( RTL_CONSTASCII_USTRINGPARAM("user") );
                        ::rtl::OUString share( RTL_CONSTASCII_USTRINGPARAM("share" ));
                        if ( rootNode->getName() == "Root" )
                        {
                            bIsRootNode = sal_True;
                        }

                        /* To mimic current starbasic behaviour we
                        need to make sure that only the current document
                        is displayed in the config tree. Tests below
                        set the bDisplay flag to sal_False if the current
                        node is a first level child of the Root and is NOT
                        either the current document, user or share */
                        ::rtl::OUString currentDocTitle;
                        Reference< XModel > xDocument( lcl_getScriptableDocument_nothrow( m_xFrame ) );
                        if ( xDocument.is() )
                        {
                            currentDocTitle = ::comphelper::DocumentInfo::getDocumentTitle( xDocument );
                        }

                        sal_Int32 nLen = children.getLength();
                        for ( sal_Int32 n = 0; n < nLen; ++n )
                        {
                            Reference< browse::XBrowseNode >& theChild = children[n];
                            ::rtl::OUString aName( theChild->getName() );
                            sal_Bool bDisplay = sal_True;
                            if ( bIsRootNode )
                            {
                                if ( !( (aName.equals(user) || aName.equals(share) || aName.equals(currentDocTitle) ) ) )
                                    bDisplay=sal_False;
                            }
                            if ( children[n].is() && children[n]->getType() != browse::BrowseNodeTypes::SCRIPT && bDisplay )
                            {

                                /*
                                    We call acquire on the XBrowseNode so that it does not
                                    get autodestructed and become invalid when accessed later.
                                */
                                theChild->acquire();

                                SfxGroupInfo_Impl* pGrpInfo =
                                    new SfxGroupInfo_Impl(SFX_CFGGROUP_SCRIPTCONTAINER,
                                        0, static_cast<void *>( theChild.get()));

                                Image aImage = GetImage( theChild, Reference< XComponentContext >(), sal_False );
                                SvLBoxEntry* pNewEntry =
                                    InsertEntry( theChild->getName(), pEntry );
                                SetExpandedEntryBmp( pNewEntry, aImage );
                                SetCollapsedEntryBmp(pNewEntry, aImage );

                                pNewEntry->SetUserData( pGrpInfo );
                                aArr.push_back( pGrpInfo );

                                if ( children[n]->hasChildNodes() )
                                {
                                    Sequence< Reference< browse::XBrowseNode > > grandchildren =
                                        children[n]->getChildNodes();

                                    for ( sal_Int32 m = 0; m < grandchildren.getLength(); ++m )
                                    {
                                        if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                                        {
                                            pNewEntry->EnableChildrenOnDemand( sal_True );
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

        case SFX_CFGGROUP_STYLES:
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
                    SvLBoxEntry* pStyleEntry = InsertEntry( pFamily->sLabel, pEntry );
                    SfxGroupInfo_Impl *pGrpInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_STYLES, 0, pFamily );
                    aArr.push_back( pGrpInfo );
                    pStyleEntry->SetUserData( pGrpInfo );
                    pStyleEntry->EnableChildrenOnDemand( sal_False );
                }
            }
            break;
        }

        default:
            OSL_FAIL( "Falscher Gruppentyp!" );
            break;
    }
}

void SfxConfigGroupListBox_Impl::SelectMacro( const SfxMacroInfoItem *pItem )
{
    SelectMacro( pItem->GetBasicManager()->GetName(),
                 pItem->GetQualifiedName() );
}

void SfxConfigGroupListBox_Impl::SelectMacro( const String& rBasic,
         const String& rMacro )
{
    String aBasicName( rBasic );
    aBasicName += ' ';
    aBasicName += pImp->m_sMacros;
    String aLib, aModule, aMethod;
    sal_uInt16 nCount = comphelper::string::getTokenCount(rMacro, '.');
    aMethod = rMacro.GetToken( nCount-1, '.' );
    if ( nCount > 2 )
    {
        aLib = rMacro.GetToken( 0, '.' );
        aModule = rMacro.GetToken( nCount-2, '.' );
    }

    SvLBoxEntry *pEntry = FirstChild(0);
    while ( pEntry )
    {
        String aEntryBas = GetEntryText( pEntry );
        if ( aEntryBas == aBasicName )
        {
            Expand( pEntry );
            SvLBoxEntry *pLib = FirstChild( pEntry );
            while ( pLib )
            {
                String aEntryLib = GetEntryText( pLib );
                if ( aEntryLib == aLib )
                {
                    Expand( pLib );
                    SvLBoxEntry *pMod = FirstChild( pLib );
                    while ( pMod )
                    {
                        String aEntryMod = GetEntryText( pMod );
                        if ( aEntryMod == aModule )
                        {
                            Expand( pMod );
                            MakeVisible( pMod );
                            Select( pMod );
                            SvLBoxEntry *pMethod = pFunctionListBox->First();
                            while ( pMethod )
                            {
                                String aEntryMethod = GetEntryText( pMethod );
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
