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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

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

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;

// implemented in 'sfx2/source/appl/appopen.cxx'
extern sal_uInt32 CheckPasswd_Impl( SfxObjectShell* pDoc, SfxItemPool &rPool, SfxMedium* pFile );

// =======================================================================

namespace sfx2 {

// =======================================================================

// =======================================================================
// DocumentInserter
// =======================================================================

DocumentInserter::DocumentInserter(
    sal_Int64 _nFlags, const String& _rFactory, bool _bEnableMultiSelection ) :

      m_sDocFactory             ( _rFactory )
    , m_bMultiSelectionEnabled  ( _bEnableMultiSelection )
    , m_nDlgFlags               ( _nFlags | SFXWB_INSERT | WB_3DLOOK )
    , m_nError                  ( ERRCODE_NONE )
    , m_pFileDlg                ( NULL )
    , m_pItemSet                ( NULL )
    , m_pURLList                ( NULL )

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
    DELETEZ( m_pURLList );
    if ( !m_pFileDlg )
    {
        sal_Int64 nFlags = m_bMultiSelectionEnabled ? ( m_nDlgFlags | SFXWB_MULTISELECTION )
                                                    : m_nDlgFlags;
        m_pFileDlg = new FileDialogHelper( nFlags, m_sDocFactory );
    }
    m_pFileDlg->StartExecuteModal( LINK( this, DocumentInserter, DialogClosedHdl ) );
}

SfxMedium* DocumentInserter::CreateMedium()
{
    SfxMedium* pMedium = NULL;
    if ( !m_nError && m_pItemSet && m_pURLList && m_pURLList->Count() > 0 )
    {
        DBG_ASSERT( m_pURLList->Count() == 1, "DocumentInserter::CreateMedium(): invalid URL list count" );
        String sURL = *( m_pURLList->GetObject(0) );
        pMedium = new SfxMedium(
                sURL, SFX_STREAM_READONLY, sal_False,
                SFX_APP()->GetFilterMatcher().GetFilter4FilterName( m_sFilter ), m_pItemSet );
        pMedium->UseInteractionHandler( sal_True );
        SfxFilterMatcher* pMatcher = NULL;
        if ( m_sDocFactory.Len() )
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
    if ( !m_nError && m_pItemSet && m_pURLList && m_pURLList->Count() > 0 )
    {
        sal_Int32 i = 0;
        sal_Int32 nCount = m_pURLList->Count();
        for ( ; i < nCount; ++i )
        {
            String sURL = *( m_pURLList->GetObject( static_cast< sal_uInt16 >(i) ) );
            SfxMedium* pMedium = new SfxMedium(
                    sURL, SFX_STREAM_READONLY, sal_False,
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
                pMediumList->Insert( pMedium );
            else
                delete pMedium;
        }
    }

    return pMediumList;
}

void impl_FillURLList( sfx2::FileDialogHelper* _pFileDlg, SvStringsDtor*& _rpURLList )
{
    DBG_ASSERT( _pFileDlg, "DocumentInserter::fillURLList(): invalid file dialog" );
    DBG_ASSERT( !_rpURLList, "DocumentInserter::fillURLList(): URLList already exists" );
    Sequence < ::rtl::OUString > aPathSeq = _pFileDlg->GetSelectedFiles();

    if ( aPathSeq.getLength() )
    {
        _rpURLList = new SvStringsDtor;

        for ( sal_uInt16 i = 0; i < aPathSeq.getLength(); ++i )
        {
            INetURLObject aPathObj( aPathSeq[i] );
            String* pURL = new String( aPathObj.GetMainURL( INetURLObject::NO_DECODE ) );
            _rpURLList->Insert( pURL, _rpURLList->Count() );
        }
    }
}

IMPL_LINK( DocumentInserter, DialogClosedHdl, sfx2::FileDialogHelper*, EMPTYARG )
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

        // check, wether or not we have to display a password box
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
            catch( IllegalArgumentException ){}
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
            catch( IllegalArgumentException )
            {
                DBG_ERROR( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
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
                catch( IllegalArgumentException )
                {
                    DBG_ERROR( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
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
            catch( IllegalArgumentException ){}
        }
    }

    m_sFilter = m_pFileDlg->GetRealFilter();

    if ( m_aDialogClosedLink.IsSet() )
        m_aDialogClosedLink.Call( m_pFileDlg );

    return 0;
}

// =======================================================================

} // namespace sfx2

// =======================================================================

