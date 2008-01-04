/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docinfohelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:27:37 $
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
#include "precompiled_unotools.hxx"

#include <rtl/ustrbuf.hxx>
#include <tools/inetdef.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/docinfohelper.hxx>

using namespace ::com::sun::star;

namespace utl
{

::rtl::OUString DocInfoHelper::GetGeneratorString()
{
    rtl::OUStringBuffer aResult;

    // First product: branded name + version
    // version is <product_versions>_<product_extension>$<platform>
    utl::ConfigManager* pMgr = utl::ConfigManager::GetConfigManager();
    if ( pMgr )
    {
        // plain product name
        rtl::OUString aValue;
        uno::Any aAny = pMgr->GetDirectConfigProperty(
                                            utl::ConfigManager::PRODUCTNAME);
        if ( (aAny >>= aValue) && aValue.getLength() )
        {
            aResult.append( aValue.replace( ' ', '_' ) );
            aResult.append( (sal_Unicode)'/' );

            aAny = pMgr->GetDirectConfigProperty(
                                        utl::ConfigManager::PRODUCTVERSION);
            if ( (aAny >>= aValue) && aValue.getLength() )
            {
                aResult.append( aValue.replace( ' ', '_' ) );

                aAny = pMgr->GetDirectConfigProperty(
                                        utl::ConfigManager::PRODUCTEXTENSION);
                if ( (aAny >>= aValue) && aValue.getLength() )
                {
                    aResult.append( (sal_Unicode)'_' );
                    aResult.append( aValue.replace( ' ', '_' ) );
                }
            }

            aResult.append( (sal_Unicode)'$' );
            aResult.append( ::rtl::OUString::createFromAscii(
                                    TOOLS_INETDEF_OS ).replace( ' ', '_' ) );

            aResult.append( (sal_Unicode)' ' );
        }
    }

    // second product: OpenOffice.org_project/<build_information>
    // build_information has '(' and '[' encoded as '$', ')' and ']' ignored
    // and ':' replaced by '-'
    {
        aResult.appendAscii( "OpenOffice.org_project/" );
        ::rtl::OUString aDefault;
        ::rtl::OUString aBuildId( Bootstrap::getBuildIdData( aDefault ) );
        for( sal_Int32 i=0; i < aBuildId.getLength(); i++ )
        {
            sal_Unicode c = aBuildId[i];
            switch( c )
            {
            case '(':
            case '[':
                aResult.append( (sal_Unicode)'$' );
                break;
            case ')':
            case ']':
                break;
            case ':':
                aResult.append( (sal_Unicode)'-' );
                break;
            default:
                aResult.append( c );
                break;
            }
        }
    }

    return aResult.makeStringAndClear();
}

} // end of namespace utl

