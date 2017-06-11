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
#include <strings.hrc>
#include <svl/filenotation.hxx>
#include <vcl/layout.hxx>
#include <ucbhelper/content.hxx>
#include <dialmgr.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>

namespace svx
{


    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::svt;

    ODocumentLinkDialog::ODocumentLinkDialog( vcl::Window* _pParent, bool _bCreateNew )
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

    ODocumentLinkDialog::~ODocumentLinkDialog()
    {
        disposeOnce();
    }

    void ODocumentLinkDialog::dispose()
    {
        m_pURL.clear();
        m_pBrowseFile.clear();
        m_pName.clear();
        m_pOK.clear();
        ModalDialog::dispose();
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


    IMPL_LINK_NOARG(ODocumentLinkDialog, OnOk, Button*, void)
    {
        // get the current URL
        OUString sURL = m_pURL->GetText();
        OFileNotation aTransformer(sURL);
        sURL = aTransformer.get(OFileNotation::N_URL);

        // check for the existence of the selected file
        bool bFileExists = false;
        try
        {
            ::ucbhelper::Content aFile(sURL, Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext());
            if (aFile.isDocument())
                bFileExists = true;
        }
        catch(Exception&)
        {
        }

        if (!bFileExists)
        {
            OUString sMsg = CuiResId(STR_LINKEDDOC_DOESNOTEXIST);
            sMsg = sMsg.replaceFirst("$file$", m_pURL->GetText());
            ScopedVclPtrInstance< MessageDialog > aError(this, sMsg);
            aError->Execute();
            return;
        } // if (!bFileExists)
        INetURLObject aURL( sURL );
        if ( aURL.GetProtocol() != INetProtocol::File )
        {
            OUString sMsg = CuiResId(STR_LINKEDDOC_NO_SYSTEM_FILE);
            sMsg = sMsg.replaceFirst("$file$", m_pURL->GetText());
            ScopedVclPtrInstance< MessageDialog > aError(this, sMsg);
            aError->Execute();
            return;
        }

        OUString sCurrentText = m_pName->GetText();
        if ( m_aNameValidator.IsSet() )
        {
            if ( !m_aNameValidator.Call( sCurrentText ) )
            {
                OUString sMsg = CuiResId(STR_NAME_CONFLICT);
                sMsg = sMsg.replaceFirst("$file$", sCurrentText);
                ScopedVclPtrInstance< MessageDialog > aError(this, sMsg, VclMessageType::Info);
                aError->Execute();

                m_pName->SetSelection(Selection(0,sCurrentText.getLength()));
                m_pName->GrabFocus();
                return;
            }
        }

        EndDialog(RET_OK);
    }


    IMPL_LINK_NOARG(ODocumentLinkDialog, OnBrowseFile, Button*, void)
    {
        ::sfx2::FileDialogHelper aFileDlg(
                ui::dialogs::TemplateDescription::FILEOPEN_READONLY_VERSION);
        std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetFilterByName("StarOffice XML (Base)");
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

        if (ERRCODE_NONE != aFileDlg.Execute())
            return;

        if (m_pName->GetText().isEmpty())
        {   // default the name to the base of the chosen URL
            INetURLObject aParser;

            aParser.SetSmartProtocol(INetProtocol::File);
            aParser.SetSmartURL(aFileDlg.GetPath());

            m_pName->SetText(aParser.getBase(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset));

            m_pName->SetSelection(Selection(0,m_pName->GetText().getLength()));
            m_pName->GrabFocus();
        }
        else
            m_pURL->GrabFocus();

        // get the path in system notation
        OFileNotation aTransformer(aFileDlg.GetPath(), OFileNotation::N_URL);
        m_pURL->SetText(aTransformer.get(OFileNotation::N_SYSTEM));

        validate();
    }


    IMPL_LINK_NOARG(ODocumentLinkDialog, OnTextModified, Edit&, void)
    {
        validate( );
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
