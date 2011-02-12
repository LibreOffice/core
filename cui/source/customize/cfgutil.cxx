/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>

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
#include <sfx2/macrconf.hxx>
#include <sfx2/minfitem.hxx>
#include <unotools/processfactory.hxx>
#include <comphelper/documentinfo.hxx>
#include <svtools/imagemgr.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/configmgr.hxx>
#include "dialmgr.hxx"
#include <svl/stritem.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;
namespace css = ::com::sun::star;

static ::rtl::OUString SERVICE_UICATEGORYDESCRIPTION (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.UICategoryDescription") );
static ::rtl::OUString SERVICE_UICMDDESCRIPTION      (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.UICommandDescription") );

SfxStylesInfo_Impl::SfxStylesInfo_Impl()
{}

void SfxStylesInfo_Impl::setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel)
{
    m_xDoc = xModel;
}

static ::rtl::OUString FAMILY_CHARACTERSTYLE (RTL_CONSTASCII_USTRINGPARAM("CharacterStyles") );
static ::rtl::OUString FAMILY_PARAGRAPHSTYLE (RTL_CONSTASCII_USTRINGPARAM("ParagraphStyles") );
static ::rtl::OUString FAMILY_FRAMESTYLE     (RTL_CONSTASCII_USTRINGPARAM("FrameStyles"    ) );
static ::rtl::OUString FAMILY_PAGESTYLE      (RTL_CONSTASCII_USTRINGPARAM("PageStyles"     ) );
static ::rtl::OUString FAMILY_NUMBERINGSTYLE (RTL_CONSTASCII_USTRINGPARAM("NumberingStyles") );

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

    if (aStyle.sFamily.getLength() && aStyle.sStyle.getLength())
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
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(const css::uno::Exception&)
        { aStyle.sLabel = ::rtl::OUString(); }

    if (!aStyle.sLabel.getLength())
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
        catch(const css::uno::RuntimeException& exRun)
            { throw exRun; }
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
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
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
        catch(const css::uno::RuntimeException& exRun)
            { throw exRun; }
        catch(const css::uno::Exception&)
            { continue; }

        lStyles.push_back(aStyleInfo);
    }
    return lStyles;
}
SV_IMPL_PTRARR(SfxGroupInfoArr_Impl, SfxGroupInfoPtr);
SfxConfigFunctionListBox_Impl::SfxConfigFunctionListBox_Impl( Window* pParent, const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
    , pCurEntry( 0 )
    , pStylesInfo( 0 )
{
    SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
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
/*  Beschreibung
    Timer-Handler f"ur die Einblendung eines Hilfetextes. Wenn nach Ablauf des Timers
    der Mauszeiger immer noch auf dem aktuell selektierten Eintrag steht, wird der
    Helptext des Entries als Balloon-Help eingeblendet.
*/
{
    (void)pTimer; // unused
    return 0L;
}

void SfxConfigFunctionListBox_Impl::ClearAll()
/*  Beschreibung
    L"oscht alle Eintr"age in der FunctionListBox, alle UserDaten und alle evtl.
    vorhandenen MacroInfos.
*/
{
    USHORT nCount = aArr.Count();
    for ( USHORT i=0; i<nCount; ++i )
    {
        SfxGroupInfo_Impl *pData = aArr[i];

        if ( pData->nKind == SFX_CFGFUNCTION_MACRO ||
                 pData->nKind == SFX_CFGFUNCTION_SCRIPT )
        {
            SfxMacroInfo *pInfo = (SfxMacroInfo*) pData->pObject;
            SFX_APP()->GetMacroConfig()->ReleaseSlotId( pInfo->GetSlotId() );
            delete pInfo;
        }

        if  (   pData->nKind == SFX_CFGGROUP_SCRIPTCONTAINER
            ||  pData->nKind == SFX_CFGGROUP_DOCBASICMGR
            )
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != NULL)
            {
                xi->release();
            }
        }

        delete pData;
    }

    aArr.Remove( 0, nCount );
    Clear();
}

SvLBoxEntry* SfxConfigFunctionListBox_Impl::GetEntry_Impl( const String& rName )
/*  Beschreibung
    Ermittelt den SvLBoxEntry zu einem "ubergebenen String. Das setzt voraus, da\s
    die Namen eindeutig sind.
*/
{
    SvLBoxEntry *pEntry = First();
    while ( pEntry )
    {
        if ( GetEntryText( pEntry ) == rName )
            return pEntry;
        pEntry = Next( pEntry );
    }

    return NULL;
}

SvLBoxEntry* SfxConfigFunctionListBox_Impl::GetEntry_Impl( USHORT nId )
/*  Beschreibung
    Ermittelt den SvLBoxEntry zu einer "ubergebenen Id.
*/
{
    SvLBoxEntry *pEntry = First();
    while ( pEntry )
    {
        SfxGroupInfo_Impl *pData = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pData && pData->nOrd == nId )
            return pEntry;
        pEntry = Next( pEntry );
    }

    return NULL;
}

SfxMacroInfo* SfxConfigFunctionListBox_Impl::GetMacroInfo()
/*  Beschreibung
    Gibt die MacroInfo des selektierten Entry zur"uck ( sofern vorhanden ).
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    if ( pEntry )
    {
        SfxGroupInfo_Impl *pData = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pData && ( pData->nKind == SFX_CFGFUNCTION_MACRO ||
                                        pData->nKind == SFX_CFGFUNCTION_SCRIPT ) )
            return (SfxMacroInfo*) pData->pObject;
    }

    return 0;
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

USHORT SfxConfigFunctionListBox_Impl::GetId( SvLBoxEntry *pEntry )
/*  Beschreibung
    Gibt die Ordnungsnummer ( SlotId oder Macro-Nummer ) des Eintrags zur"uck.
*/
{
    SfxGroupInfo_Impl *pData = pEntry ?
        (SfxGroupInfo_Impl*) pEntry->GetUserData() : 0;
    if ( pData )
        return pData->nOrd;
    return 0;
}

void SfxConfigFunctionListBox_Impl::FunctionSelected()
/*  Beschreibung
    Setzt die Balloonhelp zur"uck, da diese immer den Helptext des selektierten
    Entry anzeigen soll.
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
    String m_aScriptType;
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
    m_aScriptType(String(CUI_RES(STR_BASICNAME))),
    m_collapsedImage(CUI_RES(BMP_COLLAPSED)),
    m_expandedImage(CUI_RES(BMP_EXPANDED))
{
    FreeResource();
}

SfxConfigGroupListBox_Impl::SfxConfigGroupListBox_Impl(
    Window* pParent, const ResId& rResId, ULONG nConfigMode )
        : SvTreeListBox( pParent, rResId )
        , pImp(new SvxConfigGroupBoxResource_Impl()), pFunctionListBox(0), nMode( nConfigMode ), bShowSF( FALSE ), bShowBasic( TRUE ), pStylesInfo(0)
{
    SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT );
    SetNodeBitmaps( pImp->m_collapsedImage, pImp->m_expandedImage );

    // Check configuration to see whether only Basic macros,
    // only Scripting Framework scripts, or both should be listed
    Any value;
    sal_Bool tmp = false;

    value = ::utl::ConfigManager::GetConfigManager().GetLocalProperty(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Office.Scripting/ScriptDisplaySettings/ShowBasic" ) ) );

    value >>= tmp;

    if (tmp == sal_True) {
        bShowBasic = TRUE;
    }
    else {
        bShowBasic = FALSE;
    }

    value = ::utl::ConfigManager::GetConfigManager().GetLocalProperty(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Scripting/ScriptDisplaySettings/ShowSF" ) ) );

    value >>= tmp;

    if (tmp == sal_True) {
        bShowSF = TRUE;
    }
    else {
        bShowSF = FALSE;
    }
}


SfxConfigGroupListBox_Impl::~SfxConfigGroupListBox_Impl()
{
    ClearAll();
}

void SfxConfigGroupListBox_Impl::ClearAll()
{
    USHORT nCount = aArr.Count();
    for ( USHORT i=0; i<nCount; ++i )
    {
        SfxGroupInfo_Impl *pData = aArr[i];
        if  (   pData->nKind == SFX_CFGGROUP_SCRIPTCONTAINER
            ||  pData->nKind == SFX_CFGGROUP_DOCBASICMGR
            )
        {
            XInterface* xi = static_cast<XInterface *>(pData->pObject);
            if (xi != NULL)
            {
                xi->release();
            }
        }
        delete pData;
    }

    aArr.Remove( 0, nCount );
    Clear();
}

void SfxConfigGroupListBox_Impl::SetScriptType( const String& rScriptType )
{
    pImp->m_aScriptType = rScriptType;
    ULONG nPos=0;
    SvLBoxEntry *pEntry = (SvLBoxEntry*) GetModel()->GetEntryAtAbsPos( nPos );
    ++nPos;
    while ( pEntry )
    {
        SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pInfo->nKind == SFX_CFGGROUP_BASICLIB && ( IsExpanded( pEntry ) || pInfo->bWasOpened ) )
        {
            Collapse( pEntry );
            SvLBoxEntry *pChild = FirstChild( pEntry );
            while (pChild)
            {
                GetModel()->Remove( pChild );
                pChild = FirstChild( pEntry );
            }

            Expand( pEntry );
        }

        pEntry = (SvLBoxEntry*) GetModel()->GetEntryAtAbsPos( nPos );
        ++nPos;
    }
}

void SfxConfigGroupListBox_Impl::SetStylesInfo(SfxStylesInfo_Impl* pStyles)
{
    pStylesInfo = pStyles;
}

String SfxConfigGroupListBox_Impl::GetGroup()
/*  Beschreibung
    Gibt den Namen der selektierten Funktionsgruppe bzw. des selektierten
    Basics zur"uck.
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    while ( pEntry )
    {
        SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
        if ( pInfo->nKind == SFX_CFGGROUP_FUNCTION )
            return GetEntryText( pEntry );

        if ( pInfo->nKind == SFX_CFGGROUP_BASICMGR )
        {
            BasicManager *pMgr = (BasicManager*) pInfo->pObject;
            return pMgr->GetName();
        }

        if ( pInfo->nKind == SFX_CFGGROUP_DOCBASICMGR )
        {
            Reference< XModel > xDoc( static_cast< XModel* >( pInfo->pObject ) );
            return ::comphelper::DocumentInfo::getDocumentTitle( xDoc );
        }

        pEntry = GetParent( pEntry );
    }

    return String();
}

//-----------------------------------------------
BasicManager* SfxConfigGroupListBox_Impl::GetBasicManager( const SvLBoxEntry& _rEntry )
{
    BasicManager* pBasMgr = NULL;

    SfxGroupInfo_Impl* pInfo = (SfxGroupInfo_Impl*) _rEntry.GetUserData();
    switch ( pInfo->nKind )
    {
        case SFX_CFGGROUP_BASICMGR :
            pBasMgr = (BasicManager*) pInfo->pObject;
            break;
        case SFX_CFGGROUP_DOCBASICMGR :
        {
            Reference< XModel > xDoc( static_cast< XModel* >( pInfo->pObject ) );
            pBasMgr = ::basic::BasicManagerRepository::getDocumentBasicManager( xDoc );
        }
        break;
    }
    return pBasMgr;
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
                if (!sGroupName.getLength())
                    continue;
            }
            catch(const css::container::NoSuchElementException&)
                { continue; }

            SvLBoxEntry*        pEntry = InsertEntry(sGroupName, NULL);
            SfxGroupInfo_Impl* pInfo   = new SfxGroupInfo_Impl(SFX_CFGGROUP_FUNCTION, rGroupID);
            pEntry->SetUserData(pInfo);
        }
    }
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
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
    SetUpdateMode(FALSE);
    ClearAll(); // Remove all old entries from treelist box

    m_xFrame = xFrame;
    if ( xSMGR.is())
    {
        m_xSMGR           = xSMGR;
        m_sModuleLongName = sModuleLongName;

        m_xGlobalCategoryInfo = css::uno::Reference< css::container::XNameAccess >(m_xSMGR->createInstance(SERVICE_UICATEGORYDESCRIPTION), css::uno::UNO_QUERY_THROW);
        m_xModuleCategoryInfo = css::uno::Reference< css::container::XNameAccess >(m_xGlobalCategoryInfo->getByName(m_sModuleLongName)   , css::uno::UNO_QUERY_THROW);
        m_xUICmdDescription   = css::uno::Reference< css::container::XNameAccess >(m_xSMGR->createInstance(SERVICE_UICMDDESCRIPTION)     , css::uno::UNO_QUERY_THROW);

        InitModule();
        InitBasic();
        InitStyles();
    }

    SfxApplication *pSfxApp = SFX_APP();
    if ( bShowBasic )
    {
        // Basics einsammeln
        pSfxApp->EnterBasicCall();
        String aMacroName(' ');
        aMacroName += pImp->m_sDlgMacros;

        // Zuerst AppBasic
        BasicManager *pAppBasicMgr = pSfxApp->GetBasicManager();
        BOOL bInsert = TRUE;

        if ( bInsert )
        {
            pAppBasicMgr->SetName( pSfxApp->GetName() );
            if ( pAppBasicMgr->GetLibCount() )
            {
                // Nur einf"ugen, wenn Bibliotheken vorhanden
                String aAppBasTitle( pImp->m_aHumanAppName );
                aAppBasTitle += aMacroName;
                SvLBoxEntry *pEntry = InsertEntry( aAppBasTitle, 0 );
                SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_BASICMGR, 0, pAppBasicMgr );
                pEntry->SetUserData( pInfo );
                pEntry->EnableChildsOnDemand( TRUE );
            }
        }

        Reference< XModel > xDoc( lcl_getScriptableDocument_nothrow( m_xFrame ) );
        if ( xDoc.is() )
        {
            BasicManager* pBasicMgr = ::basic::BasicManagerRepository::getDocumentBasicManager( xDoc );
            if ( pBasicMgr != pAppBasicMgr && pBasicMgr->GetLibCount() )
            {
                String sDocTitle( ::comphelper::DocumentInfo::getDocumentTitle( xDoc ) );
                pBasicMgr->SetName( sDocTitle );

                // Nur einf"ugen, wenn eigenes Basic mit Bibliotheken
                SvLBoxEntry *pEntry = InsertEntry( sDocTitle.Append( aMacroName ), NULL );
                xDoc->acquire();
                SfxGroupInfo_Impl *pInfo =
                    new SfxGroupInfo_Impl( SFX_CFGGROUP_DOCBASICMGR, 0, xDoc.get() );
                pEntry->SetUserData( pInfo );
                pEntry->EnableChildsOnDemand( TRUE );
            }
        }

        pSfxApp->LeaveBasicCall();
    }

    OSL_TRACE("** ** About to initialise SF Scripts");
    if ( bShowSF )
    {
        OSL_TRACE("** ** bShowSF");
        // Add Scripting Framework entries
        Reference< browse::XBrowseNode > rootNode;
        Reference< XComponentContext > xCtx;
        try
        {
            Reference < beans::XPropertySet > xProps(
                ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
            xCtx.set( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), UNO_QUERY_THROW );
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
                pNewEntry->EnableChildsOnDemand( TRUE );
                aArr.Insert( pInfo, aArr.Count() );
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
                        BOOL bIsRootNode = FALSE;

                        ::rtl::OUString user( RTL_CONSTASCII_USTRINGPARAM("user") );
                        ::rtl::OUString share( RTL_CONSTASCII_USTRINGPARAM("share") );
                        if ( rootNode->getName().equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Root") ) ) )
                        {
                            bIsRootNode = TRUE;
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
                            BOOL bDisplay = TRUE;
                            ::rtl::OUString uiName = theChild->getName();
                            if ( bIsRootNode )
                            {
                                if (  ! ((theChild->getName().equals( user )  || theChild->getName().equals( share ) ||
                                    theChild->getName().equals( currentDocTitle ) ) ) )
                                {
                                    bDisplay=FALSE;
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
                                aArr.Insert( pInfo, aArr.Count() );

                                if ( children[n]->hasChildNodes() )
                                {
                                    Sequence< Reference< browse::XBrowseNode > > grandchildren =
                                        children[n]->getChildNodes();

                                    for ( sal_Int32 m = 0; m < grandchildren.getLength(); ++m )
                                    {
                                        if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                                        {
                                            pNewEntry->EnableChildsOnDemand( TRUE );
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
    }

    // add styles
    if ( m_xSMGR.is() )
    {
        String sStyle( pImp->m_aStrGroupStyles );
        SvLBoxEntry *pEntry = InsertEntry( sStyle, 0 );
        SfxGroupInfo_Impl *pInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_STYLES, 0, 0 ); // TODO last parameter should contain user data
        aArr.Insert( pInfo, aArr.Count() );
        pEntry->SetUserData( pInfo );
        pEntry->EnableChildsOnDemand( TRUE );
    }

    MakeVisible( GetEntry( 0,0 ) );
    SetUpdateMode( TRUE );
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
                Reference< ::com::sun::star::frame::XModuleManager >
                    xModuleManager(
                        xCtx->getServiceManager()
                            ->createInstanceWithContext(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("" // xxx todo
                                      "com.sun.star.frame.ModuleManager") ),
                                xCtx ),
                            UNO_QUERY_THROW );
                Reference<container::XNameAccess> xModuleConfig(
                    xModuleManager, UNO_QUERY_THROW );
                // get the long name of the document:
                ::rtl::OUString appModule( xModuleManager->identify(
                                    xDocumentModel ) );
                Sequence<beans::PropertyValue> moduleDescr;
                Any aAny = xModuleConfig->getByName(appModule);
                if( sal_True != ( aAny >>= moduleDescr ) )
                {
                    throw RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SFTreeListBox::Init: failed to get PropertyValue") ), Reference< XInterface >());
                }
                beans::PropertyValue const * pmoduleDescr =
                    moduleDescr.getConstArray();
                for ( sal_Int32 pos = moduleDescr.getLength(); pos--; )
                {
                    if (pmoduleDescr[ pos ].Name.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM(
                                "ooSetupFactoryEmptyDocumentURL") ))
                    {
                        pmoduleDescr[ pos ].Value >>= factoryURL;
                        OSL_TRACE("factory url for doc images is %s",
                        ::rtl::OUStringToOString( factoryURL , RTL_TEXTENCODING_ASCII_US ).pData->buffer );
                        break;
                    }
                }
            }
            if( factoryURL.getLength() > 0 )
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

::rtl::OUString SfxConfigGroupListBox_Impl::parseLocationName( const ::rtl::OUString& location )
{
    // strip out the last leaf of location name
    // e.g. file://dir1/dir2/Blah.sxw - > Blah.sxw
    ::rtl::OUString temp = location;
    sal_Int32 lastSlashIndex = temp.lastIndexOf( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/" ) ) );

    if ( ( lastSlashIndex + 1 ) <  temp.getLength()  )
    {
        temp = temp.copy( lastSlashIndex + 1 );
    }
    // maybe we should throw here!!!
    else
    {
        OSL_TRACE("Something wrong with name, perhaps we should throw an exception");
    }
    return temp;
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
    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    catch(css::uno::Exception&)
        { sUIName = ::rtl::OUString(); }

    // fallback for missing UINames !?
    if (!sUIName.getLength())
    {
        sUIName = sCommand;
    }

    return sUIName;
}

//-----------------------------------------------
void SfxConfigGroupListBox_Impl::GroupSelected()
/*  Beschreibung
    Eine Funktionsgruppe oder eine Basicmodul wurde selektiert. Alle Funktionen bzw.
    Macros werden in der Functionlistbox anzeigt.
*/
{
    SvLBoxEntry *pEntry = FirstSelected();
    SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    pFunctionListBox->SetUpdateMode(FALSE);
    pFunctionListBox->ClearAll();
    if ( pInfo->nKind != SFX_CFGGROUP_FUNCTION &&
             pInfo->nKind != SFX_CFGGROUP_BASICMOD &&
             pInfo->nKind != SFX_CFGGROUP_SCRIPTCONTAINER &&
             pInfo->nKind != SFX_CFGGROUP_STYLES )
    {
        pFunctionListBox->SetUpdateMode(TRUE);
        return;
    }

    switch ( pInfo->nKind )
    {
        case SFX_CFGGROUP_FUNCTION :
        {
            USHORT                                                          nGroup    = pInfo->nOrd;
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

        case SFX_CFGGROUP_BASICMOD :
        {
            SvLBoxEntry *pLibEntry = GetParent( pEntry );
            SfxGroupInfo_Impl *pLibInfo =
                (SfxGroupInfo_Impl*) pLibEntry->GetUserData();
            SvLBoxEntry *pBasEntry = GetParent( pLibEntry );
            SfxGroupInfo_Impl *pBasInfo =
                (SfxGroupInfo_Impl*) pBasEntry->GetUserData();

            StarBASIC *pLib = (StarBASIC*) pLibInfo->pObject;
            Reference< XModel > xDoc;
            if ( pBasInfo->nKind == SFX_CFGGROUP_DOCBASICMGR )
                xDoc = static_cast< XModel* >( pBasInfo->pObject );

            SbModule *pMod = (SbModule*) pInfo->pObject;
            for ( USHORT nMeth=0; nMeth < pMod->GetMethods()->Count(); ++nMeth )
            {
                SbxMethod *pMeth = (SbxMethod*)pMod->GetMethods()->Get(nMeth);
                SfxMacroInfoPtr pInf = new SfxMacroInfo( !xDoc.is(),
                                                         pLib->GetName(),
                                                         pMod->GetName(),
                                                         pMeth->GetName());
                if ( pMeth->GetInfo() )
                    pInf->SetHelpText( pMeth->GetInfo()->GetComment() );
                USHORT nId = SFX_APP()->GetMacroConfig()->GetSlotId( pInf );
                if ( !nId )
                    break;      // Kein Slot mehr frei

                SvLBoxEntry* pFuncEntry =
                    pFunctionListBox->InsertEntry( pMeth->GetName(), NULL );
                SfxGroupInfo_Impl *pGrpInfo =
                    new SfxGroupInfo_Impl( SFX_CFGFUNCTION_MACRO, nId, pInf );
                pFunctionListBox->aArr.Insert( pGrpInfo, pFunctionListBox->aArr.Count() );
                pFuncEntry->SetUserData( pGrpInfo );
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
                                    xPropSet->getPropertyValue( String::CreateFromAscii( "URI" ) );
                                value >>= uri;

                                SfxMacroInfo* aInfo = new SfxMacroInfo( (String)uri );
                                aInfo->SetHelpText( uri );
                                SFX_APP()->GetMacroConfig()->GetSlotId( aInfo );

                                SfxGroupInfo_Impl* pGrpInfo =
                                    new SfxGroupInfo_Impl(SFX_CFGFUNCTION_SCRIPT,
                                        aInfo->GetSlotId(), aInfo);

                                Image aImage = GetImage( children[n], Reference< XComponentContext >(), sal_False );
                                SvLBoxEntry* pNewEntry =
                                    pFunctionListBox->InsertEntry( children[n]->getName(), NULL );
                                pFunctionListBox->SetExpandedEntryBmp( pNewEntry, aImage );
                                pFunctionListBox->SetCollapsedEntryBmp(pNewEntry, aImage );

                                pGrpInfo->sCommand = uri;
                                pGrpInfo->sLabel = children[n]->getName();
                                pNewEntry->SetUserData( pGrpInfo );

                                pFunctionListBox->aArr.Insert(
                                    pGrpInfo, pFunctionListBox->aArr.Count() );

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
                    pFunctionListBox->aArr.Insert( pGrpInfo, pFunctionListBox->aArr.Count() );
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

    pFunctionListBox->SetUpdateMode(TRUE);
}

BOOL SfxConfigGroupListBox_Impl::Expand( SvLBoxEntry* pParent )
{
    BOOL bRet = SvTreeListBox::Expand( pParent );
    if ( bRet )
    {
        // Wieviele Entries k"onnen angezeigt werden ?
        ULONG nEntries = GetOutputSizePixel().Height() / GetEntryHeight();

        // Wieviele Kinder sollen angezeigt werden ?
        ULONG nChildCount = GetVisibleChildCount( pParent );

        // Passen alle Kinder und der parent gleichzeitig in die View ?
        if ( nChildCount+1 > nEntries )
        {
            // Wenn nicht, wenigstens parent ganz nach oben schieben
            MakeVisible( pParent, TRUE );
        }
        else
        {
            // An welcher relativen ViewPosition steht der aufzuklappende parent
            SvLBoxEntry *pEntry = GetFirstEntryInView();
            ULONG nParentPos = 0;
            while ( pEntry && pEntry != pParent )
            {
                ++nParentPos;
                pEntry = GetNextEntryInView( pEntry );
            }

            // Ist unter dem parent noch genug Platz f"ur alle Kinder ?
            if ( nParentPos + nChildCount + 1 > nEntries )
                ScrollOutputArea( (short)( nEntries - ( nParentPos + nChildCount + 1 ) ) );
        }
    }

    return bRet;
}

void SfxConfigGroupListBox_Impl::RequestingChilds( SvLBoxEntry *pEntry )
/*  Beschreibung
    Ein Basic oder eine Bibliothek werden ge"offnet
*/
{
    SfxGroupInfo_Impl *pInfo = (SfxGroupInfo_Impl*) pEntry->GetUserData();
    pInfo->bWasOpened = TRUE;
    switch ( pInfo->nKind )
    {
        case SFX_CFGGROUP_BASICMGR :
        case SFX_CFGGROUP_DOCBASICMGR :
        {
            if ( !GetChildCount( pEntry ) )
            {
                // Erstmaliges "Offnen
                BasicManager* pMgr( GetBasicManager( *pEntry ) );

                SvLBoxEntry *pLibEntry = 0;
                for ( USHORT nLib=0; nLib<pMgr->GetLibCount(); ++nLib )
                {
                    StarBASIC* pLib = pMgr->GetLib( nLib );
                    pLibEntry = InsertEntry( pMgr->GetLibName( nLib ), pEntry );
                    SfxGroupInfo_Impl *pGrpInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_BASICLIB, nLib, pLib );
                    aArr.Insert( pGrpInfo, aArr.Count() );
                    pLibEntry->SetUserData( pGrpInfo );
                    pLibEntry->EnableChildsOnDemand( TRUE );
                }
            }

            break;
        }

        case SFX_CFGGROUP_BASICLIB :
        {
            if ( !GetChildCount( pEntry ) )
            {
                // Erstmaliges "Offnen
                StarBASIC *pLib = (StarBASIC*) pInfo->pObject;
                if ( !pLib )
                {
                    // Lib mu\s nachgeladen werden
                    SvLBoxEntry *pParent = GetParent( pEntry );
                    BasicManager *pMgr( GetBasicManager( *pParent ) );

                    if ( pMgr->LoadLib( pInfo->nOrd ) )
                        pInfo->pObject = pLib = pMgr->GetLib( pInfo->nOrd );
                    else
                        break;
                }

                SvLBoxEntry *pModEntry = 0;
                for ( USHORT nMod=0; nMod<pLib->GetModules()->Count(); ++nMod )
                {
                    SbModule* pMod = (SbModule*)pLib->GetModules()->Get( nMod );

                    BOOL bIsStarScript = FALSE;
                    BOOL bWantsStarScript = pImp->m_aScriptType.EqualsAscii("StarScript");
                    if ( bIsStarScript != bWantsStarScript )
                        continue;
                    pModEntry = InsertEntry( pMod->GetName(), pEntry );
                    SfxGroupInfo_Impl *pGrpInfo = new SfxGroupInfo_Impl( SFX_CFGGROUP_BASICMOD, 0, pMod );
                    aArr.Insert( pGrpInfo, aArr.Count() );
                    pModEntry->SetUserData( pGrpInfo );
                }
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
                        BOOL bIsRootNode = FALSE;

                        ::rtl::OUString user( RTL_CONSTASCII_USTRINGPARAM("user") );
                        ::rtl::OUString share( RTL_CONSTASCII_USTRINGPARAM("share" ));
                        if ( rootNode->getName().equals(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Root") ) ) )
                        {
                            bIsRootNode = TRUE;
                        }

                        /* To mimic current starbasic behaviour we
                        need to make sure that only the current document
                        is displayed in the config tree. Tests below
                        set the bDisplay flag to FALSE if the current
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
                            BOOL bDisplay = TRUE;
                            if ( bIsRootNode )
                            {
                                if ( !( (aName.equals(user) || aName.equals(share) || aName.equals(currentDocTitle) ) ) )
                                    bDisplay=FALSE;
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
                                aArr.Insert( pGrpInfo, aArr.Count() );

                                if ( children[n]->hasChildNodes() )
                                {
                                    Sequence< Reference< browse::XBrowseNode > > grandchildren =
                                        children[n]->getChildNodes();

                                    for ( sal_Int32 m = 0; m < grandchildren.getLength(); ++m )
                                    {
                                        if ( grandchildren[m]->getType() == browse::BrowseNodeTypes::CONTAINER )
                                        {
                                            pNewEntry->EnableChildsOnDemand( TRUE );
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
                    aArr.Insert( pGrpInfo, aArr.Count() );
                    pStyleEntry->SetUserData( pGrpInfo );
                    pStyleEntry->EnableChildsOnDemand( FALSE );
                }
            }
            break;
        }

        default:
            DBG_ERROR( "Falscher Gruppentyp!" );
            break;
    }
}

void SfxConfigGroupListBox_Impl::AddAndSelect( const SfxStringItem* , const SfxStringItem* )
{
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
    USHORT nCount = rMacro.GetTokenCount('.');
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
