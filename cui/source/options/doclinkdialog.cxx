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

#include "doclinkdialog.hxx"
#include "doclinkdialog.hrc"

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <comphelper/processfactory.hxx>
#include <cuires.hrc>
#include <svl/filenotation.hxx>
#include <vcl/msgbox.hxx>
#include <ucbhelper/content.hxx>
#include <dialmgr.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>
//......................................................................
namespace svx
{
//......................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::svt;

    //==================================================================
    //= ODocumentLinkDialog
    //==================================================================
    //------------------------------------------------------------------
    ODocumentLinkDialog::ODocumentLinkDialog( Window* _pParent, sal_Bool _bCreateNew )
        :ModalDialog( _pParent, CUI_RES(DLG_DOCUMENTLINK) )
        ,m_aURLLabel        (this, CUI_RES(FT_URL))
        ,m_aURL             (this, CUI_RES(CMB_URL))
        ,m_aBrowseFile      (this, CUI_RES(PB_BROWSEFILE))
        ,m_aNameLabel       (this, CUI_RES(FT_NAME))
        ,m_aName            (this, CUI_RES(ET_NAME))
        ,m_aBottomLine      (this, CUI_RES(FL_BOTTOM))
        ,m_aOK              (this, CUI_RES(BTN_OK))
        ,m_aCancel          (this, CUI_RES(BTN_CANCEL))
        ,m_aHelp            (this, CUI_RES(BTN_HELP))
        ,m_bCreatingNew(_bCreateNew)
    {
        String sText = String( CUI_RES( m_bCreatingNew ? STR_NEW_LINK : STR_EDIT_LINK ) );
        SetText(sText);

        FreeResource();

        rtl::OUString sTemp("*.odb");
        m_aURL.SetFilter(sTemp);

        m_aName.SetModifyHdl( LINK(this, ODocumentLinkDialog, OnTextModified) );
        m_aURL.SetModifyHdl( LINK(this, ODocumentLinkDialog, OnTextModified) );
        m_aBrowseFile.SetClickHdl( LINK(this, ODocumentLinkDialog, OnBrowseFile) );
        m_aOK.SetClickHdl( LINK(this, ODocumentLinkDialog, OnOk) );

        m_aURL.SetDropDownLineCount(10);

        validate();

        m_aURL.SetDropDownLineCount( 5 );
    }

    //------------------------------------------------------------------
    void ODocumentLinkDialog::set( const String& _rName, const String& _rURL )
    {
        m_aName.SetText(_rName);
        m_aURL.SetText(_rURL);
        validate();
    }

    //------------------------------------------------------------------
    void ODocumentLinkDialog::get( String& _rName, String& _rURL ) const
    {
        _rName = m_aName.GetText();
        _rURL = m_aURL.GetText();
    }

    //------------------------------------------------------------------
    void ODocumentLinkDialog::validate( )
    {

        m_aOK.Enable( (0 != m_aName.GetText().Len()) && ( 0 != m_aURL.GetText().Len() ) );
    }

    //------------------------------------------------------------------
    IMPL_LINK_NOARG(ODocumentLinkDialog, OnOk)
    {
        // get the current URL
        ::rtl::OUString sURL = m_aURL.GetText();
        OFileNotation aTransformer(sURL);
        sURL = aTransformer.get(OFileNotation::N_URL);

        // check for the existence of the selected file
        sal_Bool bFileExists = sal_False;
        try
        {
            ::ucbhelper::Content aFile(sURL, Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext());
            if (aFile.isDocument())
                bFileExists = sal_True;
        }
        catch(Exception&)
        {
        }

        if (!bFileExists)
        {
            String sMsg = String(CUI_RES(STR_LINKEDDOC_DOESNOTEXIST));
            sMsg.SearchAndReplaceAscii("$file$", m_aURL.GetText());
            ErrorBox aError(this, WB_OK , sMsg);
            aError.Execute();
            return 0L;
        } // if (!bFileExists)
        INetURLObject aURL( sURL );
        if ( aURL.GetProtocol() != INET_PROT_FILE )
        {
            String sMsg = String(CUI_RES(STR_LINKEDDOC_NO_SYSTEM_FILE));
            sMsg.SearchAndReplaceAscii("$file$", m_aURL.GetText());
            ErrorBox aError(this, WB_OK , sMsg);
            aError.Execute();
            return 0L;
        }

        String sCurrentText = m_aName.GetText();
        if ( m_aNameValidator.IsSet() )
        {
            if ( !m_aNameValidator.Call( &sCurrentText ) )
            {
                String sMsg = String(CUI_RES(STR_NAME_CONFLICT));
                sMsg.SearchAndReplaceAscii("$file$", sCurrentText);
                InfoBox aError(this, sMsg);
                aError.Execute();

                m_aName.SetSelection(Selection(0,sCurrentText.Len()));
                m_aName.GrabFocus();
                return 0L;
            }
        }

        EndDialog(RET_OK);
        return 0L;
    }

    //------------------------------------------------------------------
    IMPL_LINK_NOARG(ODocumentLinkDialog, OnBrowseFile)
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION, 0);
        const SfxFilter* pFilter = SfxFilter::GetFilterByName(rtl::OUString("StarOffice XML (Base)"));
        if ( pFilter )
        {
            aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
            aFileDlg.SetCurrentFilter(pFilter->GetUIName());
        }

        String sPath = m_aURL.GetText();
        if (sPath.Len())
        {
            OFileNotation aTransformer( sPath, OFileNotation::N_SYSTEM );
            aFileDlg.SetDisplayDirectory( aTransformer.get( OFileNotation::N_URL ) );
        }

        if (0 != aFileDlg.Execute())
            return 0L;

        if (0 == m_aName.GetText().Len())
        {   // default the name to the base of the chosen URL
            INetURLObject aParser;

            aParser.SetSmartProtocol(INET_PROT_FILE);
            aParser.SetSmartURL(aFileDlg.GetPath());

            m_aName.SetText(aParser.getBase(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET));

            m_aName.SetSelection(Selection(0,m_aName.GetText().Len()));
            m_aName.GrabFocus();
        }
        else
            m_aURL.GrabFocus();

        // get the path in system notation
        OFileNotation aTransformer(aFileDlg.GetPath(), OFileNotation::N_URL);
        m_aURL.SetText(aTransformer.get(OFileNotation::N_SYSTEM));

        validate();
        return 0L;
    }

    //------------------------------------------------------------------
    IMPL_LINK_NOARG(ODocumentLinkDialog, OnTextModified)
    {
        validate( );
        return 0L;
    }

//......................................................................
}   // namespace svx
//......................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
