/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contentcaps.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:38:38 $
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

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDINFO_HPP_
#include <com/sun/star/ucb/CommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _CONTENT_HXX
#include <provider/content.hxx>
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace rtl;

using namespace chelp;

// virtual
Sequence< Property > Content::getProperties( const Reference< XCommandEnvironment > & xEnv )
{
    (void)xEnv;

    bool withMediaType = m_aURLParameter.isFile() || m_aURLParameter.isRoot();
    bool isModule = m_aURLParameter.isModule();
    bool isFile = m_aURLParameter.isFile();

    sal_Int32 num = withMediaType ? 7 : 6;
    if( isModule ) num+=6;
    if( isFile )   num++;

    Sequence< Property > props(num);

    sal_Int32 idx = 0;
    props[idx++] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "ContentType" ) ),
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[idx++] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[idx++] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsErrorDocument" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[idx++] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[idx++] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
            -1,
            getCppuBooleanType(),
            PropertyAttribute::BOUND | PropertyAttribute::READONLY );

    props[idx++] =
        Property(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
            -1,
            getCppuType( static_cast< const OUString * >( 0 ) ),
            PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

    if( withMediaType )
        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

    if( isModule )
    {
        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "Order" ) ),
                      -1,
                      getCppuType( static_cast< sal_Int32* >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "KeywordList" ) ),
                      -1,
                      getCppuType( static_cast< const Sequence< OUString >* >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "KeywordRef" ) ),
                      -1,
                      getCppuType( static_cast< const Sequence< Sequence< OUString > >* >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "KeywordTitleForRef" ) ),
                      -1,
                      getCppuType( static_cast< const Sequence< Sequence< OUString > >* >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "KeywordAnchorForRef" ) ),
                      -1,
                      getCppuType( static_cast< const Sequence< Sequence< OUString > >* >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );

        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "SearchScopes" ) ),
                      -1,
                      getCppuType( static_cast< const Sequence< OUString >* >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );
    }

    if( isFile )
    {
        props[idx++] =
            Property( OUString( RTL_CONSTASCII_USTRINGPARAM( "AnchorName" ) ),
                      -1,
                      getCppuType( static_cast< const OUString * >( 0 ) ),
                      PropertyAttribute::BOUND  | PropertyAttribute::READONLY );
    }

    return props;
}



//=========================================================================
// virtual
Sequence< CommandInfo > Content::getCommands(
                            const Reference< XCommandEnvironment > & xEnv )
{
    (void)xEnv;
//  osl::MutexGuard aGuard( m_aMutex );

    //=================================================================
    //
    // Supported commands
    //
    //=================================================================

#define COMMAND_COUNT 5

    static CommandInfo aCommandInfoTable[] =
    {
        ///////////////////////////////////////////////////////////////
        // Required commands
        ///////////////////////////////////////////////////////////////
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
            -1,
            getCppuVoidType()
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
            -1,
            getCppuType( static_cast< Sequence< Property > * >( 0 ) )
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
            -1,
            getCppuType( static_cast< Sequence< PropertyValue > * >( 0 ) )
        ),
        CommandInfo(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
            -1,
            getCppuType( static_cast< OpenCommandArgument2 * >( 0 ) )
        )
    };

    return Sequence< CommandInfo >( aCommandInfoTable, COMMAND_COUNT );
}

