/*************************************************************************
 *
 *  $RCSfile: identify.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:48 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#pragma hdrstop

#include "identify.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;

//=========================================================================
//
// ContentIdentifier Implementation.
//
//=========================================================================

ContentIdentifier::ContentIdentifier(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const OUString& ContentId )
: m_xSMgr( rxSMgr ),
  m_aContentId( ContentId )
{
}

//=========================================================================
// virtual
ContentIdentifier::~ContentIdentifier()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( ContentIdentifier,
                   XTypeProvider,
                   XContentIdentifier );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( ContentIdentifier,
                      XTypeProvider,
                         XContentIdentifier );

//=========================================================================
//
// XContentIdentifier methods.
//
//=========================================================================

// virtual
OUString SAL_CALL ContentIdentifier::getContentIdentifier()
    throw( RuntimeException )
{
    return m_aContentId;
}

//=========================================================================
// virtual
OUString SAL_CALL ContentIdentifier::getContentProviderScheme()
    throw( RuntimeException )
{
    if ( !m_aProviderScheme.getLength() && m_aContentId.getLength() )
    {
        // The content provider scheme is the part before the first ':'
        // within the content id.
        sal_Int32 nPos = m_aContentId.indexOf( ':', 0 );
        if ( nPos != -1 )
        {
            OUString aScheme( m_aContentId.copy( 0, nPos ) );
            m_aProviderScheme = aScheme.toLowerCase();
        }
    }

    return m_aProviderScheme;
}

