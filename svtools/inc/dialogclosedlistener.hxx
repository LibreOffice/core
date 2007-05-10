/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dialogclosedlistener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 13:20:53 $
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

#ifndef _SVTOOLS_DIALOGCLOSEDLISTENER_HXX
#define _SVTOOLS_DIALOGCLOSEDLISTENER_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef  _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XDIALOGCLOSEDLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= ODialogClosedListener
    //=====================================================================
    /**
        C++ class to implement a ::com::sun::star::ui::dialogs::XDialogClosedListener
    */
    class SVT_DLLPUBLIC DialogClosedListener :
        public ::cppu::WeakImplHelper1< ::com::sun::star::ui::dialogs::XDialogClosedListener >
    {
    private:
        /**
            This link will be called when the dialog was closed.

            The link must have the type:
            DECL_LINK( DialogClosedHdl, ::com::sun::star::ui::dialogs::DialogClosedEvent* );
        */
        Link    m_aDialogClosedLink;

    public:
        DialogClosedListener();
        DialogClosedListener( const Link& rLink );

        inline void SetDialogClosedLink( const Link& rLink ) { m_aDialogClosedLink = rLink; }

        // XDialogClosedListener methods
        virtual void SAL_CALL   dialogClosed( const ::com::sun::star::ui::dialogs::DialogClosedEvent& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener methods
        virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif// COMPHELPER_DIALOGCLOSEDLISTENER_HXX

