/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asyncmodaldialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-07 14:48:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

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

