/*************************************************************************
 *
 *  $RCSfile: docholder.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2003-03-12 15:37:57 $
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

#include "docholder.hxx"

#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseAble.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif


using namespace ::com::sun::star;

// add mutex locking ???

DocumentHolder::DocumentHolder()
{
}

DocumentHolder::~DocumentHolder()
{
    if ( m_xDocument.is() )
        CloseDocument();
}

void DocumentHolder::CloseDocument()
{
    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xDocument, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
    {
        xBroadcaster->removeCloseListener( (util::XCloseListener*)this );

        uno::Reference< util::XCloseable > xCloseable( xBroadcaster, uno::UNO_QUERY );
        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( uno::Exception& )
            {}
        }
    }

    m_xDocument = uno::Reference< frame::XModel >();
}

void DocumentHolder::SetDocument( const uno::Reference< frame::XModel >& xDoc )
{
    if ( m_xDocument.is() )
        CloseDocument();

    m_xDocument = xDoc;
    uno::Reference< util::XCloseBroadcaster > xBroadcaster( m_xDocument, uno::UNO_QUERY );
    if ( xBroadcaster.is() )
        xBroadcaster->addCloseListener( (util::XCloseListener*)this );
}

void SAL_CALL DocumentHolder::disposing( const com::sun::star::lang::EventObject& aSource )
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
        m_xDocument = uno::Reference< frame::XModel >();
}

void SAL_CALL DocumentHolder::queryClosing( const lang::EventObject& aSource, sal_Bool bGetsOwnership )
        throw( util::CloseVetoException )
{
    if ( m_xDocument.is() && m_xDocument == aSource.Source )
        throw util::CloseVetoException();
}

void SAL_CALL DocumentHolder::notifyClosing( const lang::EventObject& aSource )
{
    uno::Reference< util::XCloseBroadcaster > xEventBroadcaster( aSource.Source, uno::UNO_QUERY );

    if ( xEventBroadcaster.is() )
        xEventBroadcaster->removeCloseListener( (util::XCloseListener*)this );

    if ( m_xDocument.is() && m_xDocument == aSource.Source )
        m_xDocument = uno::Reference< frame::XModel >();
}

