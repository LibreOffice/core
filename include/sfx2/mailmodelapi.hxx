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
#ifndef INCLUDED_SFX_MAILMODEL_HXX
#define INCLUDED_SFX_MAILMODEL_HXX

#include <vector>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "tools/link.hxx"
#include <tools/string.hxx>
#include "sfx2/dllapi.h"

// class AddressList_Impl ------------------------------------------------
typedef ::std::vector< OUString > AddressList_Impl;

// class SfxMailModel_Impl -----------------------------------------------

class SFX2_DLLPUBLIC SfxMailModel
{
public:
    enum MailPriority
    {
        PRIO_HIGHEST,
        PRIO_HIGH,
        PRIO_NORMAL,
        PRIO_LOW,
        PRIO_LOWEST
    };

    enum AddressRole
    {
        ROLE_TO,
        ROLE_CC,
        ROLE_BCC
    };

    enum MailDocType
    {
        TYPE_SELF,
        TYPE_ASPDF
    };

protected:
    enum SaveResult
    {
        SAVE_SUCCESSFULL,
        SAVE_CANCELLED,
        SAVE_ERROR
    };
    ::std::vector< OUString > maAttachedDocuments;
    SaveResult          SaveDocumentAsFormat( const OUString& aSaveFileName,
                                              const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xFrameOrModel,
                                              const OUString& rType,
                                              OUString& rFileNamePath );

private:
    AddressList_Impl*   mpToList;
    AddressList_Impl*   mpCcList;
    AddressList_Impl*   mpBccList;
    OUString            maFromAddress;
    OUString            maSubject;
    MailPriority        mePriority;

    sal_Bool            mbLoadDone;

    SaveResult          ShowFilterOptionsDialog( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xSMGR,
                                                 const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel,
                                                 const OUString& rFilterName,
                                                 const OUString& rType,
                                                 bool bModified,
                                                 sal_Int32& rNumArgs,
                                                 ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs );

    DECL_LINK( DoneHdl, void* );

public:
    enum SendMailResult
    {
        SEND_MAIL_OK,
        SEND_MAIL_CANCELLED,
        SEND_MAIL_ERROR
    };

    SfxMailModel();
    ~SfxMailModel();

    void                AddAddress( const OUString& rAddress, AddressRole eRole );
    void                SetFromAddress( const OUString& rAddress )    { maFromAddress = rAddress; }
    void                SetSubject( const OUString& rSubject )        { maSubject = rSubject; }
    void                SetPriority( MailPriority ePrio )           { mePriority = ePrio; }

    /** attaches a document to the current attachment list, can be called more than once.
    *   at the moment there will be a dialog for export executed for every model which is going to be attached.
    *
    * \param sDocumentType
        The doc type to export. PDF will be at the moment only a direct export (no dialog).
    * \param xModel
        The current model to attach
    * \param sAttachmentTitle
        The title which will be used as attachment title
    * \return @see error code
    */
    SendMailResult      AttachDocument( const OUString& sDocumentType,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xFrameOrModel,
                                        const OUString& sAttachmentTitle );

    SendMailResult      SaveAndSend( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
                                     const OUString& rType );
    SendMailResult      Send( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame );

    sal_Bool            IsEmpty() const;
};

sal_Bool CreateFromAddress_Impl( OUString& rFrom );

#endif // INCLUDED_SFX_MAILMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
