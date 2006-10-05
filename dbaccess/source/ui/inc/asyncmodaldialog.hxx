/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asyncmodaldialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-05 13:05:14 $
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

#ifndef DBACCESS_ASYNCMODALDIALOG_HXX
#define DBACCESS_ASYNCMODALDIALOG_HXX

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
/** === end UNO includes === **/

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= AsyncDialogExecutor
    //====================================================================
    /** helper class for executing (UNO) dialogs modal, but asynchronously
    */
    class AsyncDialogExecutor
    {
    public:
        /** executes the given dialog asynchronously, but still modal

            @raises IllegalArgumentException
                if the given dialog is <NULL/>
            @todo
                allow for a callback for the result
        */
        static void executeModalDialogAsync(
            const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XExecutableDialog >& _rxDialog
        );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ASYNCMODALDIALOG_HXX

