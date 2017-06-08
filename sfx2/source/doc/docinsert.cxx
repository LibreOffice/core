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
#include "appopen.hxx"
#include "openflag.hxx"
#include <sfx2/passwd.hxx>

#include <sfx2/sfxsids.hrc>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <memory>

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

DocumentInserter::DocumentInserter(
    const OUString& rFactory, const Mode mode) :

      m_sDocFactory             ( rFactory )
    , m_nDlgFlags               ( lcl_map_mode_to_flags(mode) )
    , m_nError                  ( ERRCODE_NONE )
    , m_pFileDlg                ( nullptr )
    , m_pItemSet                ( nullptr )
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
                m_nDlgFlags, m_sDocFactory ) );
    }
    m_pFileDlg->StartExecuteModal( LINK( this, DocumentInserter, DialogClosedHdl ) );
}

SfxMedium* DocumentInserter::CreateMedium(char const*const pFallbackHack)
{
    std::unique_ptr<SfxMedium> pMedium;
    if (!m_nError && m_pItemSet && !m_pURLList.empty())
    {
        DBG_ASSERT( m_pURLList.size() == 1, "DocumentInserter::CreateMedium(): invalid URL list count" );
        pMedium.reset(new SfxMedium(
                m_pURLList[0], SFX_STREAM_READONLY,
                SfxGetpApp()->GetFilterMatcher().GetFilter4FilterName( m_sFilter ), m_pItemSet ));
        pMedium->UseInteractionHandler( true );
        std::unique_ptr<SfxFilterMatcher> pMatcher;
        if ( !m_sDocFactory.isEmpty() )
            pMatcher.reset(new SfxFilterMatcher(m_sDocFactory));
        else
            pMatcher.reset(new SfxFilterMatcher());

        std::shared_ptr<const SfxFilter> pFilter;
        ErrCode nError = pMatcher->DetectFilter( *pMedium, pFilter );
        // tdf#101813 hack: check again if it's a global document
        if (ERRCODE_NONE != nError && pFallbackHack)
        {
            pMatcher.reset(new SfxFilterMatcher(OUString::createFromAscii(pFallbackHack)));
            nError = pMatcher->DetectFilter( *pMedium, pFilter );
        }
        if ( nError == ERRCODE_NONE && pFilter )
            pMedium->SetFilter( pFilter );
        else
            pMedium.reset();

        if ( pMedium && CheckPasswd_Impl( nullptr, pMedium.get() ) == ERRCODE_ABORT )
            pMedium.reset();
    }

    return pMedium.release();
}

SfxMediumList* DocumentInserter::CreateMediumList()
{
    SfxMediumList* pMediumList = new SfxMediumList;
    if (!m_nError && m_pItemSet && !m_pURLList.empty())
    {
        for(std::vector<OUString>::const_iterator i = m_pURLList.begin(); i != m_pURLList.end(); ++i)
        {
            SfxMedium* pMedium = new SfxMedium(
                    *i, SFX_STREAM_READONLY,
                    SfxGetpApp()->GetFilterMatcher().GetFilter4FilterName( m_sFilter ), m_pItemSet );

            pMedium->UseInteractionHandler( true );

            SfxFilterMatcher aMatcher( m_sDocFactory );
            std::shared_ptr<const SfxFilter> pFilter;
            ErrCode nError = aMatcher.DetectFilter( *pMedium, pFilter );
            if ( nError == ERRCODE_NONE && pFilter )
                pMedium->SetFilter( pFilter );
            else
                DELETEZ( pMedium );

            if( pMedium && CheckPasswd_Impl( nullptr, pMedium ) != ERRCODE_ABORT )
                pMediumList->push_back( pMedium );
            else
                delete pMedium;
        }
    }

    return pMediumList;
}

void impl_FillURLList( sfx2::FileDialogHelper* _pFileDlg, std::vector<OUString>& _rpURLList )
{
    DBG_ASSERT( _pFileDlg, "DocumentInserter::fillURLList(): invalid file dialog" );

    Sequence < OUString > aPathSeq = _pFileDlg->GetSelectedFiles();

    if ( aPathSeq.getLength() )
    {
        _rpURLList.clear();

        for ( sal_Int32 i = 0; i < aPathSeq.getLength(); ++i )
        {
            INetURLObject aPathObj( aPathSeq[i] );
            _rpURLList.push_back(aPathObj.GetMainURL(INetURLObject::DecodeMechanism::NONE));
        }
    }
}

IMPL_LINK_NOARG(DocumentInserter, DialogClosedHdl, sfx2::FileDialogHelper*, void)
{
    DBG_ASSERT( m_pFileDlg, "DocumentInserter::DialogClosedHdl(): no file dialog" );

    m_nError = m_pFileDlg->GetError();
    if ( ERRCODE_NONE == m_nError )
        impl_FillURLList( m_pFileDlg.get(), m_pURLList );

    Reference < XFilePicker2 > xFP = m_pFileDlg->GetFilePicker();
    Reference < XFilePickerControlAccess > xCtrlAccess( xFP, UNO_QUERY );
    if ( xCtrlAccess.is() )
    {
        // always create a new itemset
        m_pItemSet = new SfxAllItemSet( SfxGetpApp()->GetPool() );

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
                    ScopedVclPtrInstance< SfxPasswordDialog > aPasswordDlg(nullptr);
                    aPasswordDlg->ShowExtras( SfxShowExtras::CONFIRM );
                    short nRet = aPasswordDlg->Execute();
                    if ( RET_OK == nRet )
                    {
                        m_pItemSet->Put( SfxStringItem( SID_PASSWORD, aPasswordDlg->GetPassword() ) );
                    }
                    else
                    {
                        DELETEZ( m_pItemSet );
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
                    m_pItemSet->Put( SfxBoolItem( SID_SELECTION, bSelection ) );
            }
            catch( const IllegalArgumentException& )
            {
                OSL_FAIL( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
            }
        }


        // set the read-only flag. When inserting a file, this flag is always set
        if ( m_nDlgFlags & FileDialogFlags::Insert )
            m_pItemSet->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
        else
        {
            if ( TemplateDescription::FILEOPEN_READONLY_VERSION == nDlgType )
            {
                try
                {
                    Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 );
                    bool bReadOnly = false;
                    if ( ( aValue >>= bReadOnly ) && bReadOnly )
                        m_pItemSet->Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
                }
                catch( const IllegalArgumentException& )
                {
                    OSL_FAIL( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
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
                    m_pItemSet->Put( SfxInt16Item( SID_VERSION, (short)nVersion ) );
            }
            catch( const IllegalArgumentException& ){}
        }
    }

    m_sFilter = m_pFileDlg->GetRealFilter();

    m_aDialogClosedLink.Call( m_pFileDlg.get() );
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
