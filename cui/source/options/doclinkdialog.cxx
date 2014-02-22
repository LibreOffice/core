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

#include "doclinkdialog.hxx"

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

    ODocumentLinkDialog::ODocumentLinkDialog( Window* _pParent, sal_Bool _bCreateNew )
        : ModalDialog(_pParent, "DatabaseLinkDialog",
            "cui/ui/databaselinkdialog.ui")
        ,m_bCreatingNew(_bCreateNew)
    {
        get(m_pURL, "url");
        get(m_pOK, "ok");
        get(m_pName, "name");
        get(m_pBrowseFile, "browse");

        if (!m_bCreatingNew)
            SetText(get<FixedText>("alttitle")->GetText());

        m_pURL->SetFilter("*.odb");

        m_pName->SetModifyHdl( LINK(this, ODocumentLinkDialog, OnTextModified) );
        m_pURL->SetModifyHdl( LINK(this, ODocumentLinkDialog, OnTextModified) );
        m_pBrowseFile->SetClickHdl( LINK(this, ODocumentLinkDialog, OnBrowseFile) );
        m_pOK->SetClickHdl( LINK(this, ODocumentLinkDialog, OnOk) );

        m_pURL->SetDropDownLineCount(10);

        validate();

        m_pURL->SetDropDownLineCount( 5 );
    }


    void ODocumentLinkDialog::setLink( const OUString& _rName, const OUString& _rURL )
    {
        m_pName->SetText(_rName);
        m_pURL->SetText(_rURL);
        validate();
    }


    void ODocumentLinkDialog::getLink( OUString& _rName, OUString& _rURL ) const
    {
        _rName = m_pName->GetText();
        _rURL = m_pURL->GetText();
    }


    void ODocumentLinkDialog::validate( )
    {

        m_pOK->Enable( ( !m_pName->GetText().isEmpty()) && ( !m_pURL->GetText().isEmpty() ) );
    }


    IMPL_LINK_NOARG(ODocumentLinkDialog, OnOk)
    {
        // get the current URL
        OUString sURL = m_pURL->GetText();
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
            OUString sMsg = CUI_RES(STR_LINKEDDOC_DOESNOTEXIST);
            sMsg = sMsg.replaceFirst("$file$", m_pURL->GetText());
            ErrorBox aError(this, WB_OK , sMsg);
            aError.Execute();
            return 0L;
        } // if (!bFileExists)
        INetURLObject aURL( sURL );
        if ( aURL.GetProtocol() != INET_PROT_FILE )
        {
            OUString sMsg = CUI_RES(STR_LINKEDDOC_NO_SYSTEM_FILE);
            sMsg = sMsg.replaceFirst("$file$", m_pURL->GetText());
            ErrorBox aError(this, WB_OK , sMsg);
            aError.Execute();
            return 0L;
        }

        OUString sCurrentText = m_pName->GetText();
        if ( m_aNameValidator.IsSet() )
        {
            if ( !m_aNameValidator.Call( &sCurrentText ) )
            {
                OUString sMsg = CUI_RES(STR_NAME_CONFLICT);
                sMsg = sMsg.replaceFirst("$file$", sCurrentText);
                InfoBox aError(this, sMsg);
                aError.Execute();

                m_pName->SetSelection(Selection(0,sCurrentText.getLength()));
                m_pName->GrabFocus();
                return 0L;
            }
        }

        EndDialog(RET_OK);
        return 0L;
    }


    IMPL_LINK_NOARG(ODocumentLinkDialog, OnBrowseFile)
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION, 0);
        const SfxFilter* pFilter = SfxFilter::GetFilterByName(OUString("StarOffice XML (Base)"));
        if ( pFilter )
        {
            aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
            aFileDlg.SetCurrentFilter(pFilter->GetUIName());
        }

        OUString sPath = m_pURL->GetText();
        if (!sPath.isEmpty())
        {
            OFileNotation aTransformer( sPath, OFileNotation::N_SYSTEM );
            aFileDlg.SetDisplayDirectory( aTransformer.get( OFileNotation::N_URL ) );
        }

        if (0 != aFileDlg.Execute())
            return 0L;

        if (m_pName->GetText().isEmpty())
        {   // default the name to the base of the chosen URL
            INetURLObject aParser;

            aParser.SetSmartProtocol(INET_PROT_FILE);
            aParser.SetSmartURL(aFileDlg.GetPath());

            m_pName->SetText(aParser.getBase(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET));

            m_pName->SetSelection(Selection(0,m_pName->GetText().getLength()));
            m_pName->GrabFocus();
        }
        else
            m_pURL->GrabFocus();

        // get the path in system notation
        OFileNotation aTransformer(aFileDlg.GetPath(), OFileNotation::N_URL);
        m_pURL->SetText(aTransformer.get(OFileNotation::N_SYSTEM));

        validate();
        return 0L;
    }


    IMPL_LINK_NOARG(ODocumentLinkDialog, OnTextModified)
    {
        validate( );
        return 0L;
    }

//......................................................................
}   // namespace svx
//......................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
