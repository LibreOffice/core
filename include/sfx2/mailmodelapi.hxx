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
#ifndef INCLUDED_SFX2_MAILMODELAPI_HXX
#define INCLUDED_SFX2_MAILMODELAPI_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>
#include <vector>
#include <string_view>
#include <memory>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::uno { class XInterface; }
namespace com::sun::star::uno { template <class E> class Sequence; }

// class AddressList_Impl ------------------------------------------------
typedef ::std::vector< OUString > AddressList_Impl;

// class SfxMailModel_Impl -----------------------------------------------

class SFX2_DLLPUBLIC SfxMailModel
{
protected:
    enum SaveResult
    {
        SAVE_SUCCESSFULL,
        SAVE_CANCELLED,
        SAVE_ERROR
    };
    ::std::vector< OUString > maAttachedDocuments;
    static SaveResult   SaveDocumentAsFormat( const OUString& aSaveFileName,
                                              const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
                                              const OUString& rType,
                                              OUString& rFileNamePath );

private:
    std::unique_ptr<AddressList_Impl>   mpToList;
    OUString            maFromAddress;

    static SaveResult   ShowFilterOptionsDialog( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR,
                                                 const css::uno::Reference< css::frame::XModel >& xModel,
                                                 const OUString& rFilterName,
                                                 std::u16string_view rType,
                                                 bool bModified,
                                                 sal_Int32& rNumArgs,
                                                 css::uno::Sequence< css::beans::PropertyValue >& rArgs );

public:
    enum SendMailResult
    {
        SEND_MAIL_OK,
        SEND_MAIL_CANCELLED,
        SEND_MAIL_ERROR
    };

    SfxMailModel();
    ~SfxMailModel();

    void                AddToAddress( const OUString& rAddress );

    /** attaches a document to the current attachment list, can be called more than once.
    *   at the moment there will be a dialog for export executed for every model which is going to be attached.
    *
    * \param xModel
        The current model to attach
    * \param sAttachmentTitle
        The title which will be used as attachment title
    * \return @see error code
    */
    SendMailResult      AttachDocument( const css::uno::Reference< css::uno::XInterface >& xFrameOrModel,
                                        const OUString& sAttachmentTitle );

    SendMailResult      SaveAndSend( const css::uno::Reference< css::frame::XFrame >& xFrame,
                                     const OUString& rType );
    SendMailResult      Send( const css::uno::Reference< css::frame::XFrame >& xFrame );

    bool                IsEmpty() const;
};

bool CreateFromAddress_Impl( OUString& rFrom );

#endif // INCLUDED_SFX2_MAILMODELAPI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
