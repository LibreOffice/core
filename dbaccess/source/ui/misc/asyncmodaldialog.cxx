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

#include "asyncmodaldialog.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::ui::dialogs::XExecutableDialog;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Exception;

    // AsyncDialogExecutor
    class DialogExecutor_Impl
    {
        Reference< XExecutableDialog >  m_xDialog;

    public:
        DialogExecutor_Impl( const Reference< XExecutableDialog >& _rxDialog )
            :m_xDialog( _rxDialog )
        {
        }

        void execute()
        {
            Application::PostUserEvent( LINK( this, DialogExecutor_Impl, onExecute ) );
        }

    protected:
        ~DialogExecutor_Impl()
        {
        }

    private:
        DECL_LINK( onExecute, void* );
    };

    IMPL_LINK( DialogExecutor_Impl, onExecute, void*, /* _notInterestedIn */ )
    {
        try
        {
            m_xDialog->execute();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        delete this;
        return 0L;
    }

    // AsyncDialogExecutor
    void AsyncDialogExecutor::executeModalDialogAsync( const Reference< XExecutableDialog >& _rxDialog )
    {
        if ( !_rxDialog.is() )
            throw IllegalArgumentException();

        DialogExecutor_Impl* pExecutor = new DialogExecutor_Impl( _rxDialog );
        pExecutor->execute();
        // will delete itself
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
