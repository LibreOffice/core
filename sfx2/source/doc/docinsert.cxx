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

#include <sfx2/app.hxx>
#include <sfx2/docinsert.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
#include <appopen.hxx>
#include <openflag.hxx>
#include <sfx2/passwd.hxx>

#include <sfx2/sfxsids.hrc>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <tools/urlobj.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <memory>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

namespace
{

FileDialogFlags lcl_map_mode_to_flags(const sfx2::DocumentInserter::Mode mode)
{
    FileDialogFlags f {FileDialogFlags::NONE};
    switch (mode)
    {
        case sfx2::DocumentInserter::Mode::Insert:
            f = FileDialogFlags::Insert;
            break;
        case sfx2::DocumentInserter::Mode::InsertMulti:
            f = FileDialogFlags::Insert|FileDialogFlags::MultiSelection;
            break;
        case sfx2::DocumentInserter::Mode::Compare:
            f = FileDialogFlags::InsertCompare;
            break;
        case sfx2::DocumentInserter::Mode::Merge:
            f = FileDialogFlags::InsertMerge;
            break;
    }
    return f;
}

}

namespace sfx2 {

DocumentInserter::DocumentInserter(weld::Window* pParent, OUString sFactory, const Mode mode)
    : m_pParent                 ( pParent )
    , m_sDocFactory             (std::move( sFactory ))
    , m_nDlgFlags               ( lcl_map_mode_to_flags(mode) )
    , m_nError                  ( ERRCODE_NONE )
{
}

DocumentInserter::~DocumentInserter()
{
}

void DocumentInserter::StartExecuteModal( const Link<sfx2::FileDialogHelper*,void>& _rDialogClosedLink )
{
    m_aDialogClosedLink = _rDialogClosedLink;
    m_nError = ERRCODE_NONE;
    if ( !m_pFileDlg )
    {
        m_pFileDlg.reset( new FileDialogHelper(
                ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                m_nDlgFlags, m_sDocFactory, SfxFilterFlags::NONE, SfxFilterFlags::NONE, m_pParent ) );
    }
    m_pFileDlg->SetContext(FileDialogHelper::InsertDoc);
    m_pFileDlg->StartExecuteModal( LINK( this, DocumentInserter, DialogClosedHdl ) );
}

std::unique_ptr<SfxMedium> DocumentInserter::CreateMedium(char const*const pFallbackHack)
{
    std::unique_ptr<SfxMedium> pMedium;
    if (!m_nError && m_xItemSet && !m_pURLList.empty())
    {
        DBG_ASSERT( m_pURLList.size() == 1, "DocumentInserter::CreateMedium(): invalid URL list count" );
        pMedium.reset(new SfxMedium(
                m_pURLList[0], SFX_STREAM_READONLY,
                SfxGetpApp()->GetFilterMatcher().GetFilter4FilterName( m_sFilter ), m_xItemSet ));
        pMedium->UseInteractionHandler( true );
        std::optional<SfxFilterMatcher> pMatcher;
        if ( !m_sDocFactory.isEmpty() )
            pMatcher.emplace(m_sDocFactory);
        else
            pMatcher.emplace();

        std::shared_ptr<const SfxFilter> pFilter;
        ErrCode nError = pMatcher->DetectFilter( *pMedium, pFilter );
        // tdf#101813 hack: check again if it's a global document
        if (ERRCODE_NONE != nError && pFallbackHack)
        {
            pMatcher.emplace(OUString::createFromAscii(pFallbackHack));
            nError = pMatcher->DetectFilter( *pMedium, pFilter );
        }
        if ( nError == ERRCODE_NONE && pFilter )
            pMedium->SetFilter( pFilter );
        else
            pMedium.reset();

        if ( pMedium && CheckPasswd_Impl( nullptr, pMedium.get() ) == ERRCODE_ABORT )
            pMedium.reset();
    }

    return pMedium;
}

SfxMediumList DocumentInserter::CreateMediumList()
{
    SfxMediumList aMediumList;
    if (!m_nError && m_xItemSet && !m_pURLList.empty())
    {
        for (auto const& url : m_pURLList)
        {
            std::unique_ptr<SfxMedium> pMedium(new SfxMedium(
                    url, SFX_STREAM_READONLY,
                    SfxGetpApp()->GetFilterMatcher().GetFilter4FilterName( m_sFilter ), m_xItemSet ));

            pMedium->UseInteractionHandler( true );

            SfxFilterMatcher aMatcher( m_sDocFactory );
            std::shared_ptr<const SfxFilter> pFilter;
            ErrCode nError = aMatcher.DetectFilter( *pMedium, pFilter );
            if ( nError == ERRCODE_NONE && pFilter )
                pMedium->SetFilter( pFilter );
            else
                pMedium.reset();

            if( pMedium && CheckPasswd_Impl( nullptr, pMedium.get() ) != ERRCODE_ABORT )
                aMediumList.push_back( std::move(pMedium) );
        }
    }

    return aMediumList;
}

static void impl_FillURLList( sfx2::FileDialogHelper const * _pFileDlg, std::vector<OUString>& _rpURLList )
{
    assert(_pFileDlg && "DocumentInserter::fillURLList(): invalid file dialog");

    const Sequence < OUString > aPathSeq = _pFileDlg->GetSelectedFiles();

    if ( aPathSeq.hasElements() )
    {
        _rpURLList.clear();

        std::transform(aPathSeq.begin(), aPathSeq.end(), std::back_inserter(_rpURLList),
            [](const OUString& rPath) -> OUString {
                INetURLObject aPathObj( rPath );
                return aPathObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
            });
    }
}

IMPL_LINK_NOARG(DocumentInserter, DialogClosedHdl, sfx2::FileDialogHelper*, void)
{
    DBG_ASSERT( m_pFileDlg, "DocumentInserter::DialogClosedHdl(): no file dialog" );

    m_nError = m_pFileDlg->GetError();
    if ( ERRCODE_NONE == m_nError )
        impl_FillURLList( m_pFileDlg.get(), m_pURLList );

    Reference < XFilePicker3 > xFP = m_pFileDlg->GetFilePicker();
    Reference < XFilePickerControlAccess > xCtrlAccess( xFP, UNO_QUERY );
    if ( xCtrlAccess.is() )
    {
        // always create a new itemset
        m_xItemSet = std::make_shared<SfxAllItemSet>( SfxGetpApp()->GetPool() );

        short nDlgType = m_pFileDlg->GetDialogType();
        bool bHasPassword = (
               TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD == nDlgType
            || TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS == nDlgType );

        // check, whether or not we have to display a password box
        if ( bHasPassword && m_pFileDlg->IsPasswordEnabled() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
                bool bPassWord = false;
                if ( ( aValue >>= bPassWord ) && bPassWord )
                {
                    // ask for the password
                    SfxPasswordDialog aPasswordDlg(m_pParent);
                    aPasswordDlg.ShowExtras( SfxShowExtras::CONFIRM );
                    short nRet = aPasswordDlg.run();
                    if ( RET_OK == nRet )
                    {
                        m_xItemSet->Put( SfxStringItem( SID_PASSWORD, aPasswordDlg.GetPassword() ) );
                    }
                    else
                    {
                        m_xItemSet.reset();
                        return;
                    }
                }
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( m_nDlgFlags & FileDialogFlags::Export )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 );
                bool bSelection = false;
                if ( aValue >>= bSelection )
                    m_xItemSet->Put( SfxBoolItem( SID_SELECTION, bSelection ) );
            }
            catch( const IllegalArgumentException& )
            {
                TOOLS_WARN_EXCEPTION( "sfx.doc", "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
            }
        }


        // set the read-only flag. When inserting a file, this flag is always set
        if ( m_nDlgFlags & FileDialogFlags::Insert )
            m_xItemSet->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
        else
        {
            if ( TemplateDescription::FILEOPEN_READONLY_VERSION == nDlgType )
            {
                try
                {
                    Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 );
                    bool bReadOnly = false;
                    if ( ( aValue >>= bReadOnly ) && bReadOnly )
                        m_xItemSet->Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
                }
                catch( const IllegalArgumentException& )
                {
                    TOOLS_WARN_EXCEPTION( "sfx.doc", "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
                }
            }
        }

        if ( TemplateDescription::FILEOPEN_READONLY_VERSION == nDlgType )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                                                    ControlActions::GET_SELECTED_ITEM_INDEX );
                sal_Int32 nVersion = 0;
                if ( ( aValue >>= nVersion ) && nVersion > 0 )
                    // open a special version; 0 == current version
                    m_xItemSet->Put( SfxInt16Item( SID_VERSION, static_cast<short>(nVersion) ) );
            }
            catch( const IllegalArgumentException& ){}
        }
    }

    m_sFilter = m_pFileDlg->GetRealFilter();

    m_aDialogClosedLink.Call( m_pFileDlg.get() );
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
