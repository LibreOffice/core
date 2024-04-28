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

#include <svx/svxdlg.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/app.hxx>
#include <tools/urlobj.hxx>
#include <unotools/defaultoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/viewoptions.hxx>

#include <bitmaps.hlst>
#include <dialmgr.hxx>
#include <optpath.hxx>
#include <strings.hrc>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>

using namespace css;
using namespace css::beans;
using namespace css::ui::dialogs;
using namespace css::uno;
using namespace svx;

// define ----------------------------------------------------------------

constexpr OUStringLiteral POSTFIX_INTERNAL = u"_internal";
constexpr OUString POSTFIX_USER = u"_user"_ustr;
constexpr OUString POSTFIX_WRITABLE = u"_writable"_ustr;
constexpr OUStringLiteral VAR_ONE = u"%1";
constexpr OUStringLiteral IODLG_CONFIGNAME = u"FilePicker_Save";

// struct OptPath_Impl ---------------------------------------------------

struct OptPath_Impl
{
    OUString                    m_sMultiPathDlg;
    Reference< css::util::XPathSettings >   m_xPathSettings;

    OptPath_Impl()
        : m_sMultiPathDlg(CuiResId(RID_CUISTR_EDIT_PATHS))
    {
    }
};

namespace {

struct PathUserData_Impl
{
    SvtPathOptions::Paths  nRealId;
    bool            bItemStateSet;
    OUString        sUserPath;
    OUString        sWritablePath;
    bool            bReadOnly;

    explicit PathUserData_Impl(SvtPathOptions::Paths nId)
        : nRealId(nId)
        , bItemStateSet(false)
        , bReadOnly(false)
    {
    }
};

struct Handle2CfgNameMapping_Impl
{
    SvtPathOptions::Paths m_nHandle;
    const char* m_pCfgName;
};

}

Handle2CfgNameMapping_Impl const Hdl2CfgMap_Impl[] =
{
    { SvtPathOptions::Paths::AutoCorrect, "AutoCorrect" },
    { SvtPathOptions::Paths::AutoText,    "AutoText" },
    { SvtPathOptions::Paths::Backup,      "Backup" },
    { SvtPathOptions::Paths::Gallery,     "Gallery" },
    { SvtPathOptions::Paths::Graphic,     "Graphic" },
    { SvtPathOptions::Paths::Temp,        "Temp" },
    { SvtPathOptions::Paths::Template,    "Template" },
    { SvtPathOptions::Paths::Work,        "Work" },
    { SvtPathOptions::Paths::Dictionary,        "Dictionary" },
    { SvtPathOptions::Paths::Classification, "Classification" },
#if OSL_DEBUG_LEVEL > 1
    { SvtPathOptions::Paths::Linguistic,        "Linguistic" },
#endif
    { SvtPathOptions::Paths::LAST, nullptr }
};

static OUString getCfgName_Impl( SvtPathOptions::Paths _nHandle )
{
    OUString sCfgName;
    sal_uInt16 nIndex = 0;
    while ( Hdl2CfgMap_Impl[ nIndex ].m_nHandle != SvtPathOptions::Paths::LAST )
    {
        if ( Hdl2CfgMap_Impl[ nIndex ].m_nHandle == _nHandle )
        {
            // config name found
            sCfgName = OUString::createFromAscii( Hdl2CfgMap_Impl[ nIndex ].m_pCfgName );
            break;
        }
        ++nIndex;
    }

    return sCfgName;
}

#define MULTIPATH_DELIMITER     ';'

static OUString Convert_Impl( std::u16string_view rValue )
{
    if (rValue.empty())
        return OUString();

    sal_Int32 nPos = 0;
    OUStringBuffer aReturn;
    for (;;)
    {
        OUString aValue( o3tl::getToken(rValue, 0, MULTIPATH_DELIMITER, nPos ) );
        INetURLObject aObj( aValue );
        if ( aObj.GetProtocol() == INetProtocol::File )
            aReturn.append(aObj.PathToFileName());
        if ( nPos < 0 )
            break;
        aReturn.append(MULTIPATH_DELIMITER);
    }

    return aReturn.makeStringAndClear();
}

// functions -------------------------------------------------------------

static bool IsMultiPath_Impl( const SvtPathOptions::Paths nIndex )
{
#if OSL_DEBUG_LEVEL > 1
    return ( SvtPathOptions::Paths::AutoCorrect == nIndex ||
             SvtPathOptions::Paths::AutoText == nIndex ||
             SvtPathOptions::Paths::Basic == nIndex ||
             SvtPathOptions::Paths::Gallery == nIndex ||
             SvtPathOptions::Paths::Template == nIndex );
#else
    return ( SvtPathOptions::Paths::AutoCorrect == nIndex ||
             SvtPathOptions::Paths::AutoText == nIndex ||
             SvtPathOptions::Paths::Basic == nIndex ||
             SvtPathOptions::Paths::Gallery == nIndex ||
             SvtPathOptions::Paths::Template == nIndex ||
             SvtPathOptions::Paths::Linguistic == nIndex ||
             SvtPathOptions::Paths::Dictionary == nIndex  );
#endif
}

// class SvxPathTabPage --------------------------------------------------

SvxPathTabPage::SvxPathTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage( pPage, pController, "cui/ui/optpathspage.ui", "OptPathsPage", &rSet)
    , pImpl(new OptPath_Impl)
    , xDialogListener ( new ::svt::DialogClosedListener() )
    , m_xStandardBtn(m_xBuilder->weld_button("default"))
    , m_xPathBtn(m_xBuilder->weld_button("edit"))
    , m_xPathBox(m_xBuilder->weld_tree_view("paths"))
{
    m_xStandardBtn->connect_clicked(LINK(this, SvxPathTabPage, StandardHdl_Impl));
    m_xPathBtn->connect_clicked( LINK( this, SvxPathTabPage, PathHdl_Impl ) );

    m_xPathBox->set_size_request(m_xPathBox->get_approximate_digit_width() * 60,
                                 m_xPathBox->get_height_rows(20));

    m_xPathBox->connect_row_activated( LINK( this, SvxPathTabPage, DoubleClickPathHdl_Impl ) );
    m_xPathBox->connect_column_clicked(LINK(this, SvxPathTabPage, HeaderBarClick));
    m_xPathBox->connect_changed( LINK( this, SvxPathTabPage, PathSelect_Impl ) );
    m_xPathBox->set_selection_mode(SelectionMode::Multiple);

    xDialogListener->SetDialogClosedLink( LINK( this, SvxPathTabPage, DialogClosedHdl ) );
}

IMPL_LINK(SvxPathTabPage, HeaderBarClick, int, nColumn, void)
{
    bool bSortAtoZ = !m_xPathBox->get_sort_order();
    m_xPathBox->set_sort_order(bSortAtoZ);
    m_xPathBox->set_sort_indicator(bSortAtoZ ? TRISTATE_TRUE : TRISTATE_FALSE, nColumn);
}

SvxPathTabPage::~SvxPathTabPage()
{
    for (int i = 0, nEntryCount = m_xPathBox->n_children(); i < nEntryCount; ++i)
        delete weld::fromId<PathUserData_Impl*>(m_xPathBox->get_id(i));
}

std::unique_ptr<SfxTabPage> SvxPathTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                           const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxPathTabPage>( pPage, pController, *rAttrSet );
}

OUString SvxPathTabPage::GetAllStrings()
{
    OUString sAllStrings;
    if (const auto& pString = m_xBuilder->weld_label("label1"))
        sAllStrings += pString->get_label() + " ";
    return sAllStrings.replaceAll("_", "");
}

bool SvxPathTabPage::FillItemSet( SfxItemSet* )
{
    for (int i = 0, nEntryCount = m_xPathBox->n_children(); i < nEntryCount; ++i)
    {
        PathUserData_Impl* pPathImpl = weld::fromId<PathUserData_Impl*>(m_xPathBox->get_id(i));
        SvtPathOptions::Paths nRealId = pPathImpl->nRealId;
        if (pPathImpl->bItemStateSet )
            SetPathList( nRealId, pPathImpl->sUserPath, pPathImpl->sWritablePath );
    }
    return true;
}

void SvxPathTabPage::Reset( const SfxItemSet* )
{
    m_xPathBox->clear();
    m_xPathBox->make_unsorted();

    std::unique_ptr<weld::TreeIter> xIter = m_xPathBox->make_iterator();
    for( sal_uInt16 i = 0; i <= sal_uInt16(SvtPathOptions::Paths::Classification); ++i )
    {
        // only writer uses autotext
        if ( static_cast<SvtPathOptions::Paths>(i) == SvtPathOptions::Paths::AutoText
            && !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::EModule::WRITER ) )
            continue;

        TranslateId pId;

        switch (static_cast<SvtPathOptions::Paths>(i))
        {
            case SvtPathOptions::Paths::AutoCorrect:
                pId = RID_CUISTR_KEY_AUTOCORRECT_DIR;
                break;
            case SvtPathOptions::Paths::AutoText:
                pId = RID_CUISTR_KEY_GLOSSARY_PATH;
                break;
            case SvtPathOptions::Paths::Backup:
                pId = RID_CUISTR_KEY_BACKUP_PATH;
                break;
            case SvtPathOptions::Paths::Gallery:
                pId = RID_CUISTR_KEY_GALLERY_DIR;
                break;
            case SvtPathOptions::Paths::Graphic:
                pId = RID_CUISTR_KEY_GRAPHICS_PATH;
                break;
            case SvtPathOptions::Paths::Temp:
                pId = RID_CUISTR_KEY_TEMP_PATH;
                break;
            case SvtPathOptions::Paths::Template:
                pId = RID_CUISTR_KEY_TEMPLATE_PATH;
                break;
            case SvtPathOptions::Paths::Dictionary:
                pId = RID_CUISTR_KEY_DICTIONARY_PATH;
                break;
            case SvtPathOptions::Paths::Classification:
                pId = RID_CUISTR_KEY_CLASSIFICATION_PATH;
                break;
#if OSL_DEBUG_LEVEL > 1
            case SvtPathOptions::Paths::Linguistic:
                pId = RID_CUISTR_KEY_LINGUISTIC_DIR;
                break;
#endif
            case SvtPathOptions::Paths::Work:
                pId = RID_CUISTR_KEY_WORK_PATH;
                break;
            default: break;
        }

        if (pId)
        {
            m_xPathBox->append(xIter.get());

            OUString aStr(CuiResId(pId));
            m_xPathBox->set_text(*xIter, aStr, 0);

            OUString sInternal, sUser, sWritable;
            bool bReadOnly = false;
            GetPathList( static_cast<SvtPathOptions::Paths>(i), sInternal, sUser, sWritable, bReadOnly );

            if (bReadOnly)
                m_xPathBox->set_image(*xIter, RID_SVXBMP_LOCK);

            OUString sTmpPath = sUser;
            if ( !sTmpPath.isEmpty() && !sWritable.isEmpty() )
                sTmpPath += OUStringChar(MULTIPATH_DELIMITER);
            sTmpPath += sWritable;
            const OUString aValue = Convert_Impl( sTmpPath );

            m_xPathBox->set_text(*xIter, aValue, 1);

            const OUString aValueInternal = Convert_Impl( sInternal );

            m_xPathBox->set_text(*xIter, aValueInternal, 2);

            m_xPathBox->set_sensitive(*xIter, !bReadOnly, 0);
            m_xPathBox->set_sensitive(*xIter, !bReadOnly, 1);
            m_xPathBox->set_sensitive(*xIter, !bReadOnly, 2);

            PathUserData_Impl* pPathImpl = new PathUserData_Impl(static_cast<SvtPathOptions::Paths>(i));
            pPathImpl->sUserPath = sUser;
            pPathImpl->sWritablePath = sWritable;
            pPathImpl->bReadOnly = bReadOnly;

            OUString sId = weld::toId(pPathImpl);
            m_xPathBox->set_id(*xIter, sId);
        }
    }

    m_xPathBox->columns_autosize();
    m_xPathBox->make_sorted();
    PathSelect_Impl(*m_xPathBox);
}

IMPL_LINK_NOARG(SvxPathTabPage, PathSelect_Impl, weld::TreeView&, void)
{
    bool bEnable = false;
    int nEntry = m_xPathBox->get_selected_index();
    if (nEntry != -1)
    {
        PathUserData_Impl* pPathImpl = weld::fromId<PathUserData_Impl*>(m_xPathBox->get_id(nEntry));
        bEnable = !pPathImpl->bReadOnly;
    }
    sal_uInt16 nSelCount = m_xPathBox->count_selected_rows();
    m_xPathBtn->set_sensitive(1 == nSelCount && bEnable);
    m_xStandardBtn->set_sensitive(nSelCount > 0 && bEnable);
}

IMPL_LINK_NOARG(SvxPathTabPage, StandardHdl_Impl, weld::Button&, void)
{
    m_xPathBox->selected_foreach([this](weld::TreeIter& rEntry){
        PathUserData_Impl* pPathImpl = weld::fromId<PathUserData_Impl*>(m_xPathBox->get_id(rEntry));
        OUString aOldPath = SvtDefaultOptions::GetDefaultPath( pPathImpl->nRealId );

        if ( !aOldPath.isEmpty() )
        {
            OUString sInternal, sUser, sWritable, sTemp;
            bool bReadOnly = false;
            GetPathList( pPathImpl->nRealId, sInternal, sUser, sWritable, bReadOnly );

            sal_Int32 nOldPos = 0;
            do
            {
                bool bFound = false;
                const std::u16string_view sOnePath = o3tl::getToken(aOldPath, 0, MULTIPATH_DELIMITER, nOldPos );
                if ( !sInternal.isEmpty() )
                {
                    sal_Int32 nInternalPos = 0;
                    do
                    {
                        if ( o3tl::getToken(sInternal, 0, MULTIPATH_DELIMITER, nInternalPos ) == sOnePath )
                            bFound = true;
                    }
                    while ( !bFound && nInternalPos >= 0 );
                }
                if ( !bFound )
                {
                    if ( !sTemp.isEmpty() )
                        sTemp += OUStringChar(MULTIPATH_DELIMITER);
                    sTemp += sOnePath;
                }
            }
            while ( nOldPos >= 0 );

            OUString sWritablePath;
            OUStringBuffer sUserPath;
            if ( !sTemp.isEmpty() )
            {
                sal_Int32 nNextPos = 0;
                for (;;)
                {
                    const OUString sToken = sTemp.getToken( 0, MULTIPATH_DELIMITER, nNextPos );
                    if ( nNextPos<0 )
                    {
                        // Last token need a different handling
                        sWritablePath = sToken;
                        break;
                    }
                    if ( !sUserPath.isEmpty() )
                        sUserPath.append(MULTIPATH_DELIMITER);
                    sUserPath.append(sToken);
                }
            }
            m_xPathBox->set_text(rEntry, Convert_Impl(sTemp), 1);
            pPathImpl->bItemStateSet = true;
            pPathImpl->sUserPath = sUserPath.makeStringAndClear();
            pPathImpl->sWritablePath = sWritablePath;
        }
        return false;
    });
}

void SvxPathTabPage::ChangeCurrentEntry( const OUString& _rFolder )
{
    int nEntry = m_xPathBox->get_cursor_index();
    if (nEntry == -1)
    {
        SAL_WARN( "cui.options", "SvxPathTabPage::ChangeCurrentEntry(): no entry" );
        return;
    }

    OUString sInternal, sUser, sWritable;
    PathUserData_Impl* pPathImpl = weld::fromId<PathUserData_Impl*>(m_xPathBox->get_id(nEntry));
    bool bReadOnly = false;
    GetPathList( pPathImpl->nRealId, sInternal, sUser, sWritable, bReadOnly );
    sUser = pPathImpl->sUserPath;
    sWritable = pPathImpl->sWritablePath;

    // old path is a URL?
    INetURLObject aObj( sWritable );
    bool bURL = ( aObj.GetProtocol() != INetProtocol::NotValid );
    INetURLObject aNewObj( _rFolder );
    aNewObj.removeFinalSlash();

    // then the new path also a URL else system path
    OUString sNewPathStr = bURL ? _rFolder : aNewObj.getFSysPath( FSysStyle::Detect );

    bool bChanged =
#ifdef UNX
// Unix is case sensitive
        ( sNewPathStr != sWritable );
#else
        !sNewPathStr.equalsIgnoreAsciiCase( sWritable );
#endif

    if ( !bChanged )
        return;

    m_xPathBox->set_text(nEntry, Convert_Impl(sNewPathStr), 1);
    pPathImpl->bItemStateSet = true;
    pPathImpl->sWritablePath = sNewPathStr;
    if ( SvtPathOptions::Paths::Work == pPathImpl->nRealId )
    {
        // Remove view options entry so the new work path
        // will be used for the next open dialog.
        SvtViewOptions aDlgOpt( EViewType::Dialog, IODLG_CONFIGNAME );
        aDlgOpt.Delete();
        // Reset also last used dir in the sfx application instance
        SfxApplication *pSfxApp = SfxGetpApp();
        pSfxApp->ResetLastDir();
    }
}

IMPL_LINK_NOARG(SvxPathTabPage, DoubleClickPathHdl_Impl, weld::TreeView&, bool)
{
    PathHdl_Impl(*m_xPathBtn);
    return true;
}

IMPL_LINK_NOARG(SvxPathTabPage, PathHdl_Impl, weld::Button&, void)
{
    int nEntry = m_xPathBox->get_cursor_index();
    PathUserData_Impl* pPathImpl = nEntry != -1 ? weld::fromId<PathUserData_Impl*>(m_xPathBox->get_id(nEntry)) : nullptr;
    if (!pPathImpl || pPathImpl->bReadOnly)
        return;

    SvtPathOptions::Paths nPos = pPathImpl->nRealId;
    OUString sInternal, sUser, sWritable;
    bool bPickFile = false;
    bool bReadOnly = false;
    GetPathList( pPathImpl->nRealId, sInternal, sUser, sWritable, bReadOnly );
    sUser = pPathImpl->sUserPath;
    sWritable = pPathImpl->sWritablePath;
    bPickFile = pPathImpl->nRealId == SvtPathOptions::Paths::Classification;

    if (IsMultiPath_Impl(nPos))
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxMultiPathDialog> pMultiDlg(
            pFact->CreateSvxMultiPathDialog(GetFrameWeld()));

        OUString sPath( sUser );
        if ( !sPath.isEmpty() )
            sPath += OUStringChar(MULTIPATH_DELIMITER);
        sPath += sWritable;
        pMultiDlg->SetPath( sPath );

        const OUString sPathName = m_xPathBox->get_text(nEntry, 0);
        const OUString sNewTitle = pImpl->m_sMultiPathDlg.replaceFirst( VAR_ONE, sPathName );
        pMultiDlg->SetTitle( sNewTitle );

        if (pMultiDlg->Execute() == RET_OK)
        {
            sUser.clear();
            sWritable.clear();
            OUString sFullPath;
            OUString sNewPath = pMultiDlg->GetPath();
            if ( !sNewPath.isEmpty() )
            {
                sal_Int32 nNextPos = 0;
                for (;;)
                {
                    const OUString sToken(sNewPath.getToken( 0, MULTIPATH_DELIMITER, nNextPos ));
                    if ( nNextPos<0 )
                    {
                        // Last token need a different handling
                        sWritable = sToken;
                        break;
                    }
                    if ( !sUser.isEmpty() )
                        sUser += OUStringChar(MULTIPATH_DELIMITER);
                    sUser += sToken;
                }
                sFullPath = sUser;
                if ( !sFullPath.isEmpty() )
                    sFullPath += OUStringChar(MULTIPATH_DELIMITER);
                sFullPath += sWritable;
            }

            m_xPathBox->set_text(nEntry, Convert_Impl(sFullPath), 1);
            // save modified flag
            pPathImpl->bItemStateSet = true;
            pPathImpl->sUserPath = sUser;
            pPathImpl->sWritablePath = sWritable;
        }
    }
    else if (!bPickFile)
    {
        try
        {
            Reference < XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
            xFolderPicker = sfx2::createFolderPicker(xContext, GetFrameWeld());

            INetURLObject aURL( sWritable, INetProtocol::File );
            xFolderPicker->setDisplayDirectory( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

            Reference< XAsynchronousExecutableDialog > xAsyncDlg( xFolderPicker, UNO_QUERY );
            if ( xAsyncDlg.is() )
                xAsyncDlg->startExecuteModal( xDialogListener );
            else
            {
                short nRet = xFolderPicker->execute();
                if (ExecutableDialogResults::OK != nRet)
                    return;

                OUString sFolder(xFolderPicker->getDirectory());
                ChangeCurrentEntry(sFolder);
            }
        }
        catch( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "SvxPathTabPage::PathHdl_Impl: exception from folder picker" );
        }
    }
    else
    {
        try
        {
            sfx2::FileDialogHelper aHelper(ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, FileDialogFlags::NONE, GetFrameWeld());
            uno::Reference<ui::dialogs::XFilePicker3> xFilePicker = aHelper.GetFilePicker();
            xFilePicker->appendFilter(OUString(), "*.xml");
            if (xFilePicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
            {
                uno::Sequence<OUString> aPathSeq(xFilePicker->getSelectedFiles());
                ChangeCurrentEntry(aPathSeq[0]);
            }
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("cui.options", "exception from file picker");
        }
    }
}

IMPL_LINK( SvxPathTabPage, DialogClosedHdl, DialogClosedEvent*, pEvt, void )
{
    if (RET_OK == pEvt->DialogResult)
    {
        assert(xFolderPicker.is() && "SvxPathTabPage::DialogClosedHdl(): no folder picker");
        OUString sURL = xFolderPicker->getDirectory();
        ChangeCurrentEntry( sURL );
    }
}

void SvxPathTabPage::GetPathList(
    SvtPathOptions::Paths _nPathHandle, OUString& _rInternalPath,
    OUString& _rUserPath, OUString& _rWritablePath, bool& _rReadOnly )
{
    OUString sCfgName = getCfgName_Impl( _nPathHandle );

    try
    {
        // load PathSettings service if necessary
        if ( !pImpl->m_xPathSettings.is() )
        {
            Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
            pImpl->m_xPathSettings = css::util::thePathSettings::get( xContext );
        }

        // load internal paths
        Any aAny = pImpl->m_xPathSettings->getPropertyValue(
            sCfgName + POSTFIX_INTERNAL);
        Sequence< OUString > aPathSeq;
        if ( aAny >>= aPathSeq )
        {
            for (auto& path : aPathSeq)
            {
                if ( !_rInternalPath.isEmpty() )
                    _rInternalPath += ";";
                _rInternalPath += path;
            }
        }
        // load user paths
        aAny = pImpl->m_xPathSettings->getPropertyValue(
            sCfgName + POSTFIX_USER);
        if ( aAny >>= aPathSeq )
        {
            for (auto& path : aPathSeq)
            {
                if ( !_rUserPath.isEmpty() )
                    _rUserPath += ";";
                _rUserPath += path;
            }
        }
        // then the writable path
        aAny = pImpl->m_xPathSettings->getPropertyValue(
            sCfgName + POSTFIX_WRITABLE);
        OUString sWritablePath;
        if ( aAny >>= sWritablePath )
            _rWritablePath = sWritablePath;

        // and the readonly flag
        Reference< XPropertySetInfo > xInfo = pImpl->m_xPathSettings->getPropertySetInfo();
        Property aProp = xInfo->getPropertyByName(sCfgName);
        _rReadOnly = ( ( aProp.Attributes & PropertyAttribute::READONLY ) == PropertyAttribute::READONLY );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "cui.options", "SvxPathTabPage::GetPathList()" );
    }
}


void SvxPathTabPage::SetPathList(
    SvtPathOptions::Paths _nPathHandle, std::u16string_view _rUserPath, const OUString& _rWritablePath )
{
    OUString sCfgName = getCfgName_Impl( _nPathHandle );

    try
    {
        // load PathSettings service if necessary
        if ( !pImpl->m_xPathSettings.is() )
        {
            Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();
            pImpl->m_xPathSettings = css::util::thePathSettings::get( xContext );
        }

        // save user paths
        const sal_Int32 nCount = comphelper::string::getTokenCount(_rUserPath, MULTIPATH_DELIMITER);
        Sequence< OUString > aPathSeq( nCount );
        OUString* pArray = aPathSeq.getArray();
        sal_Int32 nPos = 0;
        for ( sal_Int32 i = 0; i < nCount; ++i )
            pArray[i] = o3tl::getToken(_rUserPath, 0, MULTIPATH_DELIMITER, nPos );
        Any aValue( aPathSeq );
        pImpl->m_xPathSettings->setPropertyValue(
            sCfgName + POSTFIX_USER, aValue);

        // then the writable path
        aValue <<= _rWritablePath;
        pImpl->m_xPathSettings->setPropertyValue(
            sCfgName + POSTFIX_WRITABLE, aValue);
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION("cui.options", "");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
