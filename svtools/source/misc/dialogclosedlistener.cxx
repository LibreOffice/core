/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dialogclosedlistener.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 08:22:21 $
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

#include "precompiled_svtools.hxx"

#ifndef _SVTOOLS_DIALOGCLOSEDLISTENER_HXX
#include "dialogclosedlistener.hxx"
#endif

//.........................................................................
namespace svt
{
//.........................................................................

    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ui::dialogs;

    //=====================================================================
    //= DialogClosedListener
    //=====================================================================
    //---------------------------------------------------------------------
    DialogClosedListener::DialogClosedListener()
    {
    }

    DialogClosedListener::DialogClosedListener( const Link& rLink ) :

        m_aDialogClosedLink( rLink )

    {
    }

    // XDialogClosedListener methods
    void SAL_CALL DialogClosedListener::dialogClosed( const DialogClosedEvent& aEvent ) throw (RuntimeException)
    {
        if ( m_aDialogClosedLink.IsSet() )
        {
            ::com::sun::star::ui::dialogs::DialogClosedEvent aEvt( aEvent );
            m_aDialogClosedLink.Call( &aEvt );
        }
    }

    // XEventListener methods
    void SAL_CALL DialogClosedListener::disposing( const EventObject& ) throw(RuntimeException)
    {
        m_aDialogClosedLink = Link();
    }

//.........................................................................
}   // namespace svt
//.........................................................................


