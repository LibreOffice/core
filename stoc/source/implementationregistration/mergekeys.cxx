/*************************************************************************
 *
 *  $RCSfile: mergekeys.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-09-11 09:27:11 $
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

#include <vector>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/MergeConflictException.hpp>

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace stoc_impreg
{

struct Link
{
    OUString m_name;
    OUString m_target;

    inline Link( OUString const & name, OUString const & target ) SAL_THROW( () )
        : m_name( name )
        , m_target( target )
        {}
};
typedef ::std::vector< Link > t_links;

//==================================================================================================
static void mergeKeys(
    Reference< registry::XRegistryKey > const & xDest,
    Reference< registry::XRegistryKey > const & xSource,
    t_links & links )
    SAL_THROW( (registry::InvalidRegistryException, registry::MergeConflictException) )
{
    OSL_ASSERT( xDest->isValid() && xSource->isValid() );

    // write value
    switch (xSource->getValueType())
    {
    case registry::RegistryValueType_NOT_DEFINED:
        break;
    case registry::RegistryValueType_LONG:
        xDest->setLongValue( xSource->getLongValue() );
        break;
    case registry::RegistryValueType_ASCII:
        xDest->setAsciiValue( xSource->getAsciiValue() );
        break;
    case registry::RegistryValueType_STRING:
        xDest->setStringValue( xSource->getStringValue() );
        break;
    case registry::RegistryValueType_BINARY:
        xDest->setBinaryValue( xSource->getBinaryValue() );
        break;
    case registry::RegistryValueType_LONGLIST:
        xDest->setLongListValue( xSource->getLongListValue() );
        break;
    case registry::RegistryValueType_ASCIILIST:
        xDest->setAsciiListValue( xSource->getAsciiListValue() );
        break;
    case registry::RegistryValueType_STRINGLIST:
        xDest->setStringListValue( xSource->getStringListValue() );
        break;
    }

    // sub keys
    Sequence< OUString > sourceKeys( xSource->getKeyNames() );
    OUString const * pSourceKeys = sourceKeys.getConstArray();
    for ( sal_Int32 nPos = sourceKeys.getLength(); nPos--; )
    {
        // key name
        OUString name( pSourceKeys[ nPos ] );
        sal_Int32 nSlash = name.lastIndexOf( '/' );
        if (nSlash >= 0)
        {
            name = name.copy( nSlash +1 );
        }

        if (xSource->getKeyType( name ) == registry::RegistryKeyType_KEY)
        {
            // try to open exisiting dest key or create new one
            Reference< registry::XRegistryKey > xDestKey( xDest->createKey( name ) );
            Reference< registry::XRegistryKey > xSourceKey( xSource->openKey( name ) );
            mergeKeys( xDestKey, xSourceKey, links );
            xSourceKey->closeKey();
            xDestKey->closeKey();
        }
        else // link
        {
            // remove existing key
            Reference< registry::XRegistryKey > xDestKey( xDest->openKey( name ) );
            if (xDestKey.is() && xDestKey->isValid()) // something to remove
            {
                xDestKey->closeKey();
                if (xDest->getKeyType( name ) == registry::RegistryKeyType_LINK)
                {
                    xDest->deleteLink( name );
                }
                else
                {
                    xDest->deleteKey( name );
                }
            }

            links.push_back( Link(
                pSourceKeys[ nPos ], // abs path
                xSource->getResolvedName( name ) // abs resolved name
                ) );
        }
    }
}

//==================================================================================================
void SAL_CALL mergeKeys(
    Reference< registry::XRegistryKey > const & xDest,
    Reference< registry::XRegistryKey > const & xSource )
    SAL_THROW( (registry::InvalidRegistryException, registry::MergeConflictException) )
{
    t_links links;
    links.reserve( 16 );
    mergeKeys( xDest, xSource, links );

    for ( size_t nPos = links.size(); nPos--; )
    {
        Link const & r = links[ nPos ];
        OSL_VERIFY( xDest->createLink( r.m_name, r.m_target ) );
    }
}

}
