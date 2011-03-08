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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "asyncmodaldialog.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/IllegalArgumentException.hpp>
/** === end UNO includes === **/

#include <vcl/svapp.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::ui::dialogs::XExecutableDialog;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::uno::Exception;
    /** === end UNO using === **/

    //====================================================================
    //= AsyncDialogExecutor
    //====================================================================
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

    //--------------------------------------------------------------------
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

    //====================================================================
    //= AsyncDialogExecutor
    //====================================================================
    //--------------------------------------------------------------------
    void AsyncDialogExecutor::executeModalDialogAsync( const Reference< XExecutableDialog >& _rxDialog )
    {
        if ( !_rxDialog.is() )
            throw IllegalArgumentException();


        DialogExecutor_Impl* pExecutor = new DialogExecutor_Impl( _rxDialog );
        pExecutor->execute();
        // will delete itself
    }

//........................................................................
} // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
