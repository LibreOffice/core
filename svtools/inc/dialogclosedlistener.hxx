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

#ifndef _SVTOOLS_DIALOGCLOSEDLISTENER_HXX
#define _SVTOOLS_DIALOGCLOSEDLISTENER_HXX

#include "svtools/svtdllapi.h"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#include <tools/link.hxx>

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

