/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: desktopcontext.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:35:50 $
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
#include "precompiled_desktop.hxx"

#ifndef _DESKTOP_DESKTOPCONTEXT_HXX_
#include "desktopcontext.hxx"
#endif

#include <vcl/svapp.hxx>
#include <svtools/javainteractionhandler.hxx>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::task;

namespace desktop
{

DesktopContext::DesktopContext( const Reference< XCurrentContext > & ctx )
    : m_xNextContext( ctx )
{
}

Any SAL_CALL DesktopContext::getValueByName( const OUString& Name) throw (RuntimeException)
{
    Any retVal;

    if ( 0 == Name.compareToAscii( JAVA_INTERACTION_HANDLER_NAME ))
    {
        retVal = makeAny( Reference< XInteractionHandler >( new svt::JavaInteractionHandler()) );
    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}

}
