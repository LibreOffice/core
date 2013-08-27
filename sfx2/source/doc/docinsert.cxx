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
#include "sfx2/docinsert.hxx"
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/filedlghelper.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

// implemented in 'sfx2/source/appl/appopen.cxx'
extern sal_uInt32 CheckPasswd_Impl( SfxObjectShell* pDoc, SfxItemPool &rPool, SfxMedium* pFile );

namespace sfx2 {

DocumentInserter::DocumentInserter(
    const OUString& rFactory, bool const bEnableMultiSelection) :

      m_sDocFactory             ( rFactory )
    , m_nDlgFlags               ( (bEnableMultiSelection)
                                    ? (SFXWB_INSERT|SFXWB_MULTISELECTION)
                                    : SFXWB_INSERT )
    , m_nError                  ( ERRCODE_NONE )
    , m_pFileDlg                ( NULL )
    , m_pItemSet                ( NULL )
{
}

DocumentInserter::~DocumentInserter()
{
    delete m_pFileDlg;
}

void DocumentInserter::StartExecuteModal( const Link& _rDialogClosedLink )
{
    m_aDialogClosedLink = _rDialogClosedLink;
    m_nError = ERRCODE_NONE;
    if ( !m_pFileDlg )
    {
        m_pFileDlg = new FileDialogHelper(
                ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
                m_nDlgFlags, m_sDocFactory );
    }
    m_pFileDlg->StartExecuteModal( LINK( this, DocumentInserter, DialogClosedHdl ) );
}

SfxMedium* DocumentInserter::CreateMedium()
{
    SfxMedium* pMedium = NULL;
    if (!m_nError && m_pItemSet && !m_pURLList.empty())
    {
        DBG_ASSERT( m_pURLList.size() == 1, "DocumentInserter::CreateMedium(): invalid URL list count" );
        String sURL(m_pURLList[0]);
        pMedium = new SfxMedium(
                sURL, SFX_STREAM_READONLY,
                SFX_APP()->GetFilterMatcher().GetFilter4FilterName( m_sFilter ), m_pItemSet );
        pMedium->UseInteractionHandler( sal_True );
        SfxFilterMatcher* pMatcher = NULL;
        if ( !m_sDocFactory.isEmpty() )
            pMatcher = new SfxFilterMatcher( m_sDocFactory );
        else
            pMatcher = new SfxFilterMatcher();

        const SfxFilter* pFilter = NULL;
        sal_uInt32 nError = pMatcher->DetectFilter( *pMedium, &pFilter, sal_False );
        if ( nError == ERRCODE_NONE && pFilter )
            pMedium->SetFilter( pFilter );
        else
            DELETEZ( pMedium );

        if ( pMedium && CheckPasswd_Impl( 0, SFX_APP()->GetPool(), pMedium ) == ERRCODE_ABORT )
            pMedium = NULL;

        DELETEZ( pMatcher );
    }

    return pMedium;
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
                    SFX_APP()->GetFilterMatcher().GetFilter4FilterName( m_sFilter ), m_pItemSet );

            pMedium->UseInteractionHandler( sal_True );

            SfxFilterMatcher aMatcher( m_sDocFactory );
            const SfxFilter* pFilter = NULL;
            sal_uInt32 nError = aMatcher.DetectFilter( *pMedium, &pFilter, sal_False );
            if ( nError == ERRCODE_NONE && pFilter )
                pMedium->SetFilter( pFilter );
            else
                DELETEZ( pMedium );

            if( pMedium && CheckPasswd_Impl( 0, SFX_APP()->GetPool(), pMedium ) != ERRCODE_ABORT )
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

        for ( sal_uInt16 i = 0; i < aPathSeq.getLength(); ++i )
        {
            INetURLObject aPathObj( aPathSeq[i] );
            _rpURLList.push_back(aPathObj.GetMainURL(INetURLObject::NO_DECODE));
        }
    }
}

IMPL_LINK_NOARG(DocumentInserter, DialogClosedHdl)
{
    DBG_ASSERT( m_pFileDlg, "DocumentInserter::DialogClosedHdl(): no file dialog" );

    m_nError = m_pFileDlg->GetError();
    if ( ERRCODE_NONE == m_nError )
        impl_FillURLList( m_pFileDlg, m_pURLList );

    Reference < XFilePicker > xFP = m_pFileDlg->GetFilePicker();
    Reference < XFilePickerControlAccess > xCtrlAccess( xFP, UNO_QUERY );
    if ( xCtrlAccess.is() )
    {
        // always create a new itemset
        m_pItemSet = new SfxAllItemSet( SFX_APP()->GetPool() );

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
                sal_Bool bPassWord = sal_False;
                if ( ( aValue >>= bPassWord ) && bPassWord )
                {
                    // ask for the password
                    SfxPasswordDialog aPasswordDlg( NULL );
                    aPasswordDlg.ShowExtras( SHOWEXTRAS_CONFIRM );
                    short nRet = aPasswordDlg.Execute();
                    if ( RET_OK == nRet )
                    {
                        String aPasswd = aPasswordDlg.GetPassword();
                        m_pItemSet->Put( SfxStringItem( SID_PASSWORD, aPasswd ) );
                    }
                    else
                    {
                        DELETEZ( m_pItemSet );
                        return 0;
                    }
                }
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( SFXWB_EXPORT == ( m_nDlgFlags & SFXWB_EXPORT ) )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 );
                sal_Bool bSelection = sal_False;
                if ( aValue >>= bSelection )
                    m_pItemSet->Put( SfxBoolItem( SID_SELECTION, bSelection ) );
            }
            catch( const IllegalArgumentException& )
            {
                OSL_FAIL( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
            }
        }


        // set the read-only flag. When inserting a file, this flag is always set
        if ( SFXWB_INSERT == ( m_nDlgFlags & SFXWB_INSERT ) )
            m_pItemSet->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
        else
        {
            if ( ( TemplateDescription::FILEOPEN_READONLY_VERSION == nDlgType ) )
            {
                try
                {
                    Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 );
                    sal_Bool bReadOnly = sal_False;
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

    if ( m_aDialogClosedLink.IsSet() )
        m_aDialogClosedLink.Call( m_pFileDlg );

    return 0;
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
