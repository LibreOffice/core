/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "sccollaboration.hxx"

#include "docsh.hxx"
#include "sendfunc.hxx"
#include <com/sun/star/document/XDocumentRecovery.hpp>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>

ScCollaboration::ScCollaboration( ScDocShell* pScDocShell ) :
    mpScDocShell( pScDocShell )
{
}

ScCollaboration::~ScCollaboration()
{
}

void ScCollaboration::EndCollaboration() const
{
    ScDocFuncSend* pSender = GetScDocFuncSend();
    if (pSender)
    {
        delete pSender;
        mpScDocShell->SetDocFunc( new ScDocFuncDirect( *mpScDocShell ) );
    }
}

void ScCollaboration::PacketReceived( const OString& rPacket ) const
{
    ScDocFuncSend* pSender = GetScDocFuncSend();
    if (pSender)
        return pSender->RecvMessage( rPacket );
}

void ScCollaboration::SaveAndSendFile( TpContact* pContact ) const
{
    OUString aTmpPath = utl::TempFile::CreateTempName();
    aTmpPath += ".ods";

    OUString aFileURL;
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aTmpPath, aFileURL );

    utl::MediaDescriptor aDescriptor;
    
    aDescriptor[utl::MediaDescriptor::PROP_DOCUMENTBASEURL()] <<= OUString();
    try {
        css::uno::Reference< css::document::XDocumentRecovery > xDocRecovery(
                    mpScDocShell->GetBaseModel(), css::uno::UNO_QUERY_THROW);

        xDocRecovery->storeToRecoveryFile( aFileURL, aDescriptor.getAsConstPropertyValueList() );
    } catch (const css::uno::Exception &ex) {
        SAL_WARN( "sc.tubes", "Exception when saving file " << aFileURL );
    }

    SendFile( pContact, aFileURL );

    
}

void ScCollaboration::StartCollaboration( TeleConference* pConference )
{
    SetConference( pConference );
    ScDocFunc* pDocFunc = &mpScDocShell->GetDocFunc();
    ScDocFuncSend* pSender = dynamic_cast<ScDocFuncSend*> (pDocFunc);
    if (!pSender)
    {
        
        pSender = new ScDocFuncSend( *mpScDocShell, dynamic_cast<ScDocFuncDirect*> (pDocFunc), this );
        mpScDocShell->SetDocFunc( pSender );
    }
}



ScDocFuncSend* ScCollaboration::GetScDocFuncSend() const
{
    return dynamic_cast<ScDocFuncSend*> (&mpScDocShell->GetDocFunc());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
