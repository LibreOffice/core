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
#ifndef INCLUDED_SFX_MAILMODEL_HXX
#define INCLUDED_SFX_MAILMODEL_HXX

#include <com/sun/star/frame/XFrame.hpp>

// class SfxMailModel_Impl -----------------------------------------------

class AddressList_Impl;

class SfxMailModel_Impl
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

private:
    enum SaveResult
    {
        SAVE_SUCCESSFULL,
        SAVE_CANCELLED,
        SAVE_ERROR
    };

    AddressList_Impl*   mpToList;
    AddressList_Impl*   mpCcList;
    AddressList_Impl*   mpBccList;
    String              maFromAddress;
    String              maSubject;
    MailPriority        mePriority;

    sal_Bool            mbLoadDone;

    void                ClearList( AddressList_Impl* pList );
    void                MakeValueList( AddressList_Impl* pList, String& rValueList );
    SaveResult          SaveDocumentAsFormat( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const rtl::OUString& rType, rtl::OUString& rFileNamePath );

    DECL_LINK( DoneHdl, void* );

public:
    enum SendMailResult
    {
        SEND_MAIL_OK,
        SEND_MAIL_CANCELLED,
        SEND_MAIL_ERROR
    };

    SfxMailModel_Impl();
    ~SfxMailModel_Impl();

    void                AddAddress( const String& rAddress, AddressRole eRole );
    void                SetFromAddress( const String& rAddress )    { maFromAddress = rAddress; }
    void                SetSubject( const String& rSubject )        { maSubject = rSubject; }
    void                SetPriority( MailPriority ePrio )           { mePriority = ePrio; }

    SendMailResult      Send( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const rtl::OUString& rType );
};

sal_Bool CreateFromAddress_Impl( String& rFrom );

#endif // INCLUDED_SFX_MAILMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
