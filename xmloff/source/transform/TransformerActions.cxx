/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TransformerActions.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:16:29 $
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
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_TRANSFORMERACTIONS_HXX
#include "TransformerActions.hxx"
#endif


using ::rtl::OUString;

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
