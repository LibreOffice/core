/*************************************************************************
 *
 *  $RCSfile: cliplistener.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2001-07-19 19:30:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vos/mutex.hxx>

#include "cliplistener.hxx"
#include "transfer.hxx"

using namespace ::com::sun::star;

// -----------------------------------------------------------------------------

TransferableClipboardListener::TransferableClipboardListener( const Link& rCallback ) :
    aLink( rCallback )
{
}

TransferableClipboardListener::~TransferableClipboardListener()
{
}

void SAL_CALL TransferableClipboardListener::disposing( const lang::EventObject& rEventObject )
                                                        throw(uno::RuntimeException)
{
}

void SAL_CALL TransferableClipboardListener::changedContents(
                            const datatransfer::clipboard::ClipboardEvent& rEventObject )
                                                        throw(uno::RuntimeException)
{
    if ( aLink.IsSet() )
    {
        const ::vos::OGuard aGuard( Application::GetSolarMutex() );

        TransferableDataHelper aDataHelper( rEventObject.Contents );
        aLink.Call( &aDataHelper );
    }
}

void TransferableClipboardListener::AddRemoveListener( Window* pWin, BOOL bAdd )
{
    try
    {
        if ( pWin )
        {
            uno::Reference<datatransfer::clipboard::XClipboard> xClipboard = pWin->GetClipboard();
            uno::Reference<datatransfer::clipboard::XClipboardNotifier> xClpbrdNtfr( xClipboard, uno::UNO_QUERY );
            if( xClpbrdNtfr.is() )
            {
                uno::Reference<datatransfer::clipboard::XClipboardListener> xClipEvtLstnr( this );
                if( bAdd )
                    xClpbrdNtfr->addClipboardListener( xClipEvtLstnr );
                else
                    xClpbrdNtfr->removeClipboardListener( xClipEvtLstnr );
            }
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

