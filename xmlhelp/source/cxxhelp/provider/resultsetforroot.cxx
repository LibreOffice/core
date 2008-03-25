/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resultsetforroot.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:23:39 $
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
#include "precompiled_xmlhelp.hxx"

#ifndef _COM_SUN_STAR_UCB_COMMAND_HPP_
#include <com/sun/star/ucb/Command.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif

#include "resultsetforroot.hxx"
#include "databases.hxx"

using namespace chelp;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



ResultSetForRoot::ResultSetForRoot( const uno::Reference< lang::XMultiServiceFactory >&  xMSF,
                                    const uno::Reference< XContentProvider >&  xProvider,
                                    sal_Int32 nOpenMode,
                                    const uno::Sequence< beans::Property >& seq,
                                    const uno::Sequence< NumberedSortingInfo >& seqSort,
                                    URLParameter& aURLParameter,
                                    Databases* pDatabases )
    : ResultSetBase( xMSF,xProvider,nOpenMode,seq,seqSort ),
      m_aURLParameter( aURLParameter ),
      m_pDatabases( pDatabases )
{
    m_aPath = m_pDatabases->getModuleList( m_aURLParameter.get_language() );
    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = rtl::OUString::createFromAscii( "getPropertyValues" );
    aCommand.Argument <<= m_sProperty;

    for( sal_uInt32 i = 0; i < m_aPath.size(); ++i )
    {
        m_aPath[i] =
            rtl::OUString::createFromAscii( "vnd.sun.star.help://" ) +
            m_aPath[i]                                               +
            rtl::OUString::createFromAscii( "?Language=" )           +
            m_aURLParameter.get_language()                           +
            rtl::OUString::createFromAscii( "&System=" )             +
            m_aURLParameter.get_system();

        m_nRow = sal_Int32( i );

        uno::Reference< XContent > content = queryContent();
        if( content.is() )
        {
            uno::Reference< XCommandProcessor > cmd( content,uno::UNO_QUERY );
            cmd->execute( aCommand,0,uno::Reference< XCommandEnvironment >( 0 ) ) >>= m_aItems[i]; //TODO: check return value of operator >>=
        }
        m_nRow = 0xffffffff;
    }
}
