/*************************************************************************
 *
 *  $RCSfile: configimporter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-03-23 14:11:40 $
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

#include <helper/configimporter.hxx>

#ifndef __FRAMEWORK_XML_TOOLBOXCONFIGURATION_HXX_
#include <xml/toolboxconfiguration.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace framework
{

sal_Bool UIConfigurationImporterOOo1x::ImportCustomToolbars(
    const uno::Reference< ui::XUIConfigurationManager >& rContainerFactory,
    uno::Sequence< uno::Reference< container::XIndexContainer > >& rSeqContainer,
    const uno::Reference< lang::XMultiServiceFactory >& rServiceManager,
    const uno::Reference< embed::XStorage >& rToolbarStorage )
{
    const char USERDEFTOOLBOX[] = "userdeftoolbox0.xml";
    uno::Reference< lang::XMultiServiceFactory > rSrvMgr( rServiceManager );

    sal_Bool bResult ( sal_False );
    if ( rToolbarStorage.is() && rContainerFactory.is() )
    {
        try
        {
            for ( sal_uInt16 i = 1; i <= 4; i++ )
            {
                rtl::OUStringBuffer aCustomTbxName( 20 );
                aCustomTbxName.appendAscii( USERDEFTOOLBOX );
                aCustomTbxName.setCharAt( 14, aCustomTbxName.charAt( 14 ) + i );

                rtl::OUString aTbxStreamName( aCustomTbxName.makeStringAndClear() );
                uno::Reference< io::XStream > xStream = rToolbarStorage->openStreamElement( aTbxStreamName, embed::ElementModes::READ );
                if ( xStream.is() )
                {
                    uno::Reference< io::XInputStream > xInputStream = xStream->getInputStream();
                    if ( xInputStream.is() )
                    {
                        uno::Reference< container::XIndexContainer > xContainer = rContainerFactory->createSettings();
                        if ( ToolBoxConfiguration::LoadToolBox( rSrvMgr, xInputStream, xContainer ))
                        {
                            sal_uInt32 nIndex = rSeqContainer.getLength();
                            rSeqContainer.realloc( nIndex+1 );
                            rSeqContainer[nIndex] = xContainer;
                            bResult = sal_True;
                        }
                    }
                }
            }
        }
        catch ( uno::RuntimeException& )
        {
            throw;
        }
        catch ( uno::Exception& )
        {
        }
    }

    return bResult;
}

} // namespace framework
