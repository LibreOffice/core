/*************************************************************************
 *
 *  $RCSfile: MNSTerminateListener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 18:32:55 $
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

#ifndef _MNSTERMINATELISTENER_HXX
#include <MNSTerminateListener.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _CONNECTIVITY_MAB_NS_INIT_HXX_
#include <MNSInit.hxx>
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
::com::sun::star::uno::Reference< ::com::sun::star::frame::XTerminateListener>          MNSTerminateListener::mxTerminateListener = new MNSTerminateListener();

// -----------------------------------------
// - MNSTerminateListener -
// -----------------------------------------

MNSTerminateListener::MNSTerminateListener(  )
{
}

// -----------------------------------------------------------------------------

MNSTerminateListener::~MNSTerminateListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::disposing( const EventObject& Source ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::queryTermination( const EventObject& aEvent ) throw( TerminationVetoException, RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL MNSTerminateListener::notifyTermination( const EventObject& aEvent ) throw( RuntimeException )
{
    MNS_Term(sal_True); //Force XPCOM to shutdown
}

void MNSTerminateListener::addTerminateListener()
{
    Reference< XMultiServiceFactory >   xFact( ::comphelper::getProcessServiceFactory() );

    if( xFact.is() )
    {
        Reference< XDesktop > xDesktop( xFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Desktop" ) ), UNO_QUERY );

        if( xDesktop.is() )
            xDesktop->addTerminateListener(mxTerminateListener);
    }
}

