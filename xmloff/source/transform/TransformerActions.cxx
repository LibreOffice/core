/*************************************************************************
 *
 *  $RCSfile: TransformerActions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:00:05 $
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

#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif


using namespace ::rtl;
using namespace ::osl;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;

XMLTransformerActions::XMLTransformerActions( XMLTransformerActionInit *pInit )
{
    if( pInit )
    {
        XMLTransformerActions::key_type aKey;
        XMLTransformerActions::data_type aData;
        while( pInit->m_nActionType != XML_TACTION_EOT )
        {
            aKey.m_nPrefix = pInit->m_nPrefix;
            aKey.SetLocalName( pInit->m_eLocalName );

            OSL_ENSURE( find( aKey ) == end(), "duplicate action map entry" );

            aData.m_nActionType  = pInit->m_nActionType;
            aData.m_nParam1 = pInit->m_nParam1;
            aData.m_nParam2 = pInit->m_nParam2;
            aData.m_nParam3 = pInit->m_nParam3;
            XMLTransformerActions::value_type aVal( aKey, aData );

            insert( aVal );
            ++pInit;
        }
    }
}

XMLTransformerActions::~XMLTransformerActions()
{
}

void XMLTransformerActions::Add( XMLTransformerActionInit *pInit )
{
    if( pInit )
    {
        XMLTransformerActions::key_type aKey;
        XMLTransformerActions::data_type aData;
        while( pInit->m_nActionType != XML_TACTION_EOT )
        {
            aKey.m_nPrefix = pInit->m_nPrefix;
            aKey.SetLocalName( pInit->m_eLocalName );
            XMLTransformerActions::iterator aIter = find( aKey );
            if( aIter == end() )
            {
                aData.m_nActionType  = pInit->m_nActionType;
                aData.m_nParam1 = pInit->m_nParam1;
                aData.m_nParam2 = pInit->m_nParam2;
                aData.m_nParam3 = pInit->m_nParam3;
                XMLTransformerActions::value_type aVal( aKey, aData );
                insert( aVal );
            }

            ++pInit;
        }
    }
}
