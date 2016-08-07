/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"

#ifndef DBACCESS_ASYNCMODALDIALOG_HXX
#include "asyncmodaldialog.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

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

