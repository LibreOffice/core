/*************************************************************************
 *
 *  $RCSfile: ucbcmds.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-29 11:54:22 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 - nameclash handling

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_GLOBALTRANSFERCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/GlobalTransferCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFRERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif

#ifndef _UCBHELPER_COMMANDENVIRONMENTPROXY_HXX
#include <ucbhelper/commandenvironmentproxy.hxx>
#endif

#ifndef _UCBCMDS_HXX
#include "ucbcmds.hxx"
#endif
#ifndef _UCB_HXX
#include "ucb.hxx"
#endif

using namespace com::sun::star;
using namespace com::sun::star::ucb;

namespace ucb_commands
{

//=========================================================================
//
// class ActiveDataSink.
//
//=========================================================================

class ActiveDataSink : public cppu::OWeakObject, public io::XActiveDataSink
{
    uno::Reference< io::XInputStream > m_xStream;

public:
    // XInterface methods
    virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType )
        throw( uno::RuntimeException );
    virtual void SAL_CALL acquire();
    virtual void SAL_CALL release();

    // XActiveDataSink methods.
    virtual void SAL_CALL setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
        throw( uno::RuntimeException );
    virtual uno::Reference< io::XInputStream > SAL_CALL getInputStream()
        throw( uno::RuntimeException );
};

//=========================================================================
// virtual
uno::Any SAL_CALL ActiveDataSink::queryInterface( const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface(
                        rType,
                           static_cast< io::XActiveDataSink * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//=========================================================================
// virtual
void SAL_CALL ActiveDataSink::acquire()
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL ActiveDataSink::release()
{
    OWeakObject::release();
}

//=========================================================================
// virtual
void SAL_CALL ActiveDataSink::setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
    throw( uno::RuntimeException )
{
    m_xStream = aStream;
}

//=========================================================================
// virtual
uno::Reference< io::XInputStream > SAL_CALL ActiveDataSink::getInputStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}

//=========================================================================
//
// class CommandProcessorInfo.
//
//=========================================================================

class CommandProcessorInfo : public cppu::OWeakObject, public XCommandInfo
{
    uno::Sequence< CommandInfo > * m_pInfo;

public:
    CommandProcessorInfo();
    virtual ~CommandProcessorInfo();

    // XInterface methods
    virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType )
        throw( uno::RuntimeException );
    virtual void SAL_CALL acquire();
    virtual void SAL_CALL release();

    // XCommandInfo methods
    virtual uno::Sequence< CommandInfo > SAL_CALL getCommands()
        throw( uno::RuntimeException );
    virtual CommandInfo SAL_CALL
    getCommandInfoByName( const rtl::OUString& Name )
        throw( UnsupportedCommandException, uno::RuntimeException );
    virtual CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( UnsupportedCommandException, uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByName( const rtl::OUString& Name )
        throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByHandle( sal_Int32 Handle )
        throw( uno::RuntimeException );
};

//=========================================================================
CommandProcessorInfo::CommandProcessorInfo()
{
    m_pInfo = new uno::Sequence< CommandInfo >( 2 );

    (*m_pInfo)[ 0 ]
        = CommandInfo(
            rtl::OUString::createFromAscii( GETCOMMANDINFO_NAME ), // Name
            GETCOMMANDINFO_HANDLE, // Handle
            getCppuVoidType() ); // ArgType
    (*m_pInfo)[ 1 ]
        = CommandInfo(
            rtl::OUString::createFromAscii( GLOBALTRANSFER_NAME ), // Name
            GLOBALTRANSFER_HANDLE, // Handle
            getCppuType( static_cast<
                    GlobalTransferCommandArgument * >( 0 ) ) ); // ArgType
}

//=========================================================================
// virtual
CommandProcessorInfo::~CommandProcessorInfo()
{
    delete m_pInfo;
}

//=========================================================================
// virtual
uno::Any SAL_CALL CommandProcessorInfo::queryInterface(
                                                const uno::Type & rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface(
                        rType,
                           static_cast< XCommandInfo * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//=========================================================================
// virtual
void SAL_CALL CommandProcessorInfo::acquire()
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL CommandProcessorInfo::release()
{
    OWeakObject::release();
}

//=========================================================================
// virtual
uno::Sequence< CommandInfo > SAL_CALL CommandProcessorInfo::getCommands()
    throw( uno::RuntimeException )
{
    return uno::Sequence< CommandInfo >( *m_pInfo );
}

//=========================================================================
// virtual
CommandInfo SAL_CALL CommandProcessorInfo::getCommandInfoByName(
                                                const rtl::OUString& Name )
    throw( UnsupportedCommandException, uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Name == Name )
            return CommandInfo( (*m_pInfo)[ n ] );
    }

    throw UnsupportedCommandException();
}

//=========================================================================
// virtual
CommandInfo SAL_CALL CommandProcessorInfo::getCommandInfoByHandle(
                                                sal_Int32 Handle )
    throw( UnsupportedCommandException, uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Handle == Handle )
            return CommandInfo( (*m_pInfo)[ n ] );
    }

    throw UnsupportedCommandException();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByName(
                                                const rtl::OUString& Name )
    throw( uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Name == Name )
            return sal_True;
    }

    return sal_False;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByHandle( sal_Int32 Handle )
    throw( uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Handle == Handle )
            return sal_True;
    }

    return sal_False;
}

//=========================================================================
//=========================================================================
//=========================================================================

static void abort( const sal_Char * pReason )
{
    OSL_ENSURE( sal_False, pReason );
    throw CommandAbortedException( rtl::OUString::createFromAscii( pReason ),
                                   uno::Reference< uno::XInterface >() );
}

//=========================================================================
static uno::Reference< XContent > createNew(
                            const uno::Reference< XContent > & xTarget,
                             const GlobalTransferCommandArgument & rArg,
                            sal_Bool bSourceIsFolder,
                            sal_Bool bSourceIsDocument,
                            sal_Bool bSourceIsLink )
    throw( uno::Exception )
{
    //////////////////////////////////////////////////////////////////////
    //
    // (1) Obtain creatable types from target.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< XContentCreator > xCreator( xTarget, uno::UNO_QUERY );

    if ( !xCreator.is() )
        ucb_commands::abort( "Target is no XContentCreator!" );

    uno::Sequence< ContentInfo > aTypesInfo
                            = xCreator->queryCreatableContentsInfo();

    sal_Int32 nCount = aTypesInfo.getLength();
    if ( !nCount )
        ucb_commands::abort( "No creatable types!" );

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Try to find a matching target type for the source object.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< XContent > xNew;
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        sal_Int32 nAttribs = aTypesInfo[ n ].Attributes;
        sal_Bool  bMatch   = sal_False;

        if ( rArg.Operation == TransferCommandOperation_LINK )
        {
            // Create link

            if ( nAttribs & ContentInfoAttribute::KIND_LINK )
            {
                // Match!
                bMatch = sal_True;
            }
        }
        else if ( ( rArg.Operation == TransferCommandOperation_COPY ) ||
                   ( rArg.Operation == TransferCommandOperation_MOVE ) )
        {
            // Copy / Move

            // Is source a link? Create link in target folder then.
            if ( bSourceIsLink )
            {
                if ( nAttribs & ContentInfoAttribute::KIND_LINK )
                {
                    // Match!
                    bMatch = sal_True;
                }
            }
            else
            {
                if ( ( bSourceIsFolder ==
                        !!( nAttribs & ContentInfoAttribute::KIND_FOLDER ) ) &&
                      ( bSourceIsDocument ==
                         !!( nAttribs & ContentInfoAttribute::KIND_DOCUMENT ) ) )
                {
                    // Match!
                    bMatch = sal_True;
                }
            }
        }
        else
        {
            ucb_commands::abort( "Unsupported transfer operation!" );
        }

        if ( bMatch )
        {
            //////////////////////////////////////////////////////////////
            //
            // (3) Create a new, empty object of matched type.
            //
            //////////////////////////////////////////////////////////////

            xNew = xCreator->createNewContent( aTypesInfo[ n ] );

            if ( !xNew.is() )
                ucb_commands::abort( "createNewContent failed!" );

            break;
        }
    }

    return xNew;
}

//=========================================================================
static void transferProperties(
                const uno::Reference< XCommandProcessor > & xCommandProcessorS,
                const uno::Reference< XCommandProcessor > & xCommandProcessorN,
                 const GlobalTransferCommandArgument & rArg,
                const uno::Reference< XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    Command aGetPropertySetInfoCommand(
                rtl::OUString::createFromAscii( "getPropertySetInfo" ),
                -1,
                uno::Any() );

    uno::Reference< beans::XPropertySetInfo > xInfo;
    xCommandProcessorS->execute( aGetPropertySetInfoCommand, 0, xEnv )
        >>= xInfo;

    if ( !xInfo.is() )
        ucb_commands::abort( "Unable to get propertyset info from source!" );

    uno::Sequence< beans::Property > aAllProps = xInfo->getProperties();

    Command aGetPropsCommand1(
                rtl::OUString::createFromAscii( "getPropertyValues" ),
                -1,
                uno::makeAny( aAllProps ) );

    uno::Reference< sdbc::XRow > xRow1;
    xCommandProcessorS->execute( aGetPropsCommand1, 0, xEnv ) >>= xRow1;

    if ( !xRow1.is() )
        ucb_commands::abort( "Unable to get properties from source object!" );

    // Assemble data structure for setPropertyValues command.

    // Note: Make room for additional Title and TargetURL too. -> + 2
    uno::Sequence< beans::PropertyValue > aPropValues(
                                                aAllProps.getLength() + 2 );

    sal_Bool bHasTitle
        = ( rArg.NewTitle.getLength() == 0 );
    sal_Bool bHasTargetURL
        = ( rArg.Operation != TransferCommandOperation_LINK );

    sal_Int32 nWritePos = 0;
    for ( sal_Int32 m = 0; m < aAllProps.getLength(); ++m )
    {
        const beans::Property & rCurrProp = aAllProps[ m ];
        beans::PropertyValue & rCurrValue = aPropValues[ nWritePos ];

        uno::Any aValue;

        if ( rCurrProp.Name.compareToAscii( "Title" ) == 0 )
        {
            // Supply new title, if given.
            if ( !bHasTitle )
            {
                bHasTitle = sal_True;
                aValue <<= rArg.NewTitle;
            }
        }
        else if ( rCurrProp.Name.compareToAscii( "TargetURL" ) == 0 )
        {
            // Supply source URL as link target for the new link to create.
            if ( !bHasTargetURL )
            {
                bHasTargetURL = sal_True;
                aValue <<= rArg.SourceURL;
            }
        }

        if ( !aValue.hasValue() )
        {
            try
            {
                aValue = xRow1->getObject(
                            m + 1, uno::Reference< container::XNameAccess >() );
            }
            catch ( sdbc::SQLException const & )
            {
                // Argh! But try to bring things to an end. Perhaps the
                // mad property is not really important...
            }
        }

        if ( aValue.hasValue() )
        {
            rCurrValue.Name   = rCurrProp.Name;
            rCurrValue.Handle = rCurrProp.Handle;
            rCurrValue.Value  = aValue;
//          rCurrValue.State  =

            nWritePos++;
        }
    }

    // Title needed, but not set yet?
    if ( !bHasTitle && ( rArg.NewTitle.getLength() > 0 ) )
    {
        aPropValues[ nWritePos ].Name
            = rtl::OUString::createFromAscii( "Title" );
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rArg.NewTitle;

        nWritePos++;
    }

    // TargetURL needed, but not set yet?
    if ( !bHasTargetURL && ( rArg.Operation == TransferCommandOperation_LINK ) )
    {
        aPropValues[ nWritePos ].Name
            = rtl::OUString::createFromAscii( "TargetURL" );
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rArg.SourceURL;

        nWritePos++;
    }

    aPropValues.realloc( nWritePos );

    // Set properties at new object.

    Command aSetPropsCommand(
                rtl::OUString::createFromAscii( "setPropertyValues" ),
                -1,
                uno::makeAny( aPropValues ) );

    xCommandProcessorN->execute( aSetPropsCommand, 0, xEnv );

    // @@@ What to do with source props that are not supported by the
    //     new object? addProperty ???
}

//=========================================================================
static uno::Reference< io::XInputStream > getInputStream(
                const uno::Reference< lang::XMultiServiceFactory > & xSMgr,
                const uno::Reference< XCommandProcessor > & xCommandProcessorS,
                const uno::Reference< XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    uno::Reference< io::XInputStream > xInputStream;

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to get data as XInputStream via XActiveDataSink.
    //
    //////////////////////////////////////////////////////////////////////

    try
    {
        uno::Reference< io::XActiveDataSink > xSink = new ActiveDataSink;

        OpenCommandArgument2 aArg;
        aArg.Mode       = OpenMode::DOCUMENT;
        aArg.Priority   = 0; // unused
        aArg.Sink       = xSink;
        aArg.Properties = uno::Sequence< beans::Property >( 0 ); // unused

        Command aOpenCommand( rtl::OUString::createFromAscii( "open" ),
                              -1,
                              uno::makeAny( aArg ) );

        xCommandProcessorS->execute( aOpenCommand, 0, xEnv );
        xInputStream = xSink->getInputStream();
    }
    catch ( uno::Exception const & )
    {
    }

    if ( !xInputStream.is() )
    {
        //////////////////////////////////////////////////////////////////
        //
        // (2) Try to get data via XOutputStream.
        //
        //////////////////////////////////////////////////////////////////

        try
        {
            uno::Reference< io::XOutputStream > xOutputStream(
                xSMgr->createInstance(
                    rtl::OUString::createFromAscii( "com.sun.star.io.Pipe" ) ),
                uno::UNO_QUERY );

            if ( xOutputStream.is() )
            {
                OpenCommandArgument2 aArg;
                aArg.Mode       = OpenMode::DOCUMENT;
                aArg.Priority   = 0; // unused
                aArg.Sink       = xOutputStream;
                aArg.Properties = uno::Sequence< beans::Property >( 0 );

                Command aOpenCommand( rtl::OUString::createFromAscii( "open" ),
                                          -1,
                                          uno::makeAny( aArg ) );

                xCommandProcessorS->execute( aOpenCommand, 0, xEnv );

                xInputStream = uno::Reference< io::XInputStream >(
                                        xOutputStream, uno::UNO_QUERY );
            }
        }
        catch ( uno::Exception const & )
        {
        }
    }

    return xInputStream;
}

//=========================================================================
static uno::Reference< sdbc::XResultSet > getResultSet(
                const uno::Reference< XCommandProcessor > & xCommandProcessorS,
                const uno::Reference< XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    uno::Reference< sdbc::XResultSet > xResultSet;

    uno::Sequence< beans::Property > aProps( 3 );

    aProps[ 0 ].Name   = rtl::OUString::createFromAscii( "IsFolder" );
    aProps[ 0 ].Handle = -1; /* unknown */
    aProps[ 1 ].Name   = rtl::OUString::createFromAscii( "IsDocument" );
    aProps[ 1 ].Handle = -1; /* unknown */
    aProps[ 2 ].Name   = rtl::OUString::createFromAscii( "TargetURL" );
    aProps[ 2 ].Handle = -1; /* unknown */

    OpenCommandArgument2 aArg;
    aArg.Mode       = OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = 0;
    aArg.Properties = aProps;

    Command aOpenCommand( rtl::OUString::createFromAscii( "open" ),
                          -1,
                          uno::makeAny( aArg ) );
    try
    {
        uno::Reference< XDynamicResultSet > xSet;
        xCommandProcessorS->execute( aOpenCommand, 0, xEnv ) >>= xSet;

        if ( xSet.is() )
            xResultSet = xSet->getStaticResultSet();
    }
    catch ( uno::Exception const & )
    {
    }

    return xResultSet;
}

//=========================================================================
static void globalTransfer( const uno::Reference<
                                    lang::XMultiServiceFactory > & xSMgr,
                            const uno::Reference< XContent > & xSource,
                                const uno::Reference< XContent > & xTarget,
                             const GlobalTransferCommandArgument & rArg,
                             const uno::Reference< sdbc::XRow > & xSourceProps,
                              const uno::Reference< XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    // IsFolder: property is required.
    sal_Bool bSourceIsFolder = xSourceProps->getBoolean( 1 );
    if ( !bSourceIsFolder && xSourceProps->wasNull() )
        ucb_commands::abort(
                "Unable to get property 'IsFolder' from source object!" );

    // IsDocument: property is required.
    sal_Bool bSourceIsDocument = xSourceProps->getBoolean( 2 );
    if ( !bSourceIsDocument && xSourceProps->wasNull() )
        ucb_commands::abort(
                "Unable to get property 'IsDocument' from source object!" );

    // TargetURL: property is optional.
    sal_Bool bSourceIsLink = ( xSourceProps->getString( 3 ).getLength() > 0 );

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to find a matching target type for the source object and
    //     create a new, empty object of that type.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< XContent > xNew = createNew( xTarget,
                                                 rArg,
                                                 bSourceIsFolder,
                                                 bSourceIsDocument,
                                                 bSourceIsLink );
    if ( !xNew.is() )
        ucb_commands::abort( "No matching content type at target!" );

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Transfer property values from source to new object.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< XCommandProcessor > xCommandProcessorN(
                                                xNew, uno::UNO_QUERY );
    if ( !xCommandProcessorN.is() )
        ucb_commands::abort( "New content is not a  XCommandProcessor!" );

    // Obtain all properties from source.

    uno::Reference< XCommandProcessor > xCommandProcessorS(
                                                xSource, uno::UNO_QUERY );
    if ( !xCommandProcessorS.is() )
        ucb_commands::abort( "Source is not a XCommandProcessor!" );

    transferProperties( xCommandProcessorS, xCommandProcessorN, rArg, xEnv );

    //////////////////////////////////////////////////////////////////////
    //
    // (3) Try to obtain a data stream from source.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< io::XInputStream > xInputStream;

    if ( bSourceIsDocument &&
         ( rArg.Operation != TransferCommandOperation_LINK ) )
        xInputStream = getInputStream( xSMgr, xCommandProcessorS, xEnv );

    //////////////////////////////////////////////////////////////////////
    //
    // (4) Try to obtain a resultset (children) from source.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< sdbc::XResultSet > xResultSet;

    if ( bSourceIsFolder &&
         ( rArg.Operation != TransferCommandOperation_LINK ) )
        xResultSet = getResultSet( xCommandProcessorS, xEnv );

    //////////////////////////////////////////////////////////////////////
    //
    // (5) Insert (store) new content.
    //
    //////////////////////////////////////////////////////////////////////

    try
    {
        InsertCommandArgument aArg;
        aArg.Data            = xInputStream;
        aArg.ReplaceExisting = sal_False; // ReplaceExisting;

        Command aInsertCommand( rtl::OUString::createFromAscii( "insert" ),
                                  -1,
                                  uno::makeAny( aArg ) );

        xCommandProcessorN->execute( aInsertCommand, 0, xEnv );
    }
/*
    catch ( CommandAbortedException const & )
    {
        // @@@ nameclash handling code
    }
*/
    catch ( uno::Exception const & )
    {
        OSL_ENSURE( sal_False, "Cannot insert new object!" );
        throw;
    }

    //////////////////////////////////////////////////////////////////////
    //
    // (6) Process children of source.
    //
    //////////////////////////////////////////////////////////////////////

    if ( xResultSet.is() )
    {
        try
        {
            // Iterate over children...

            uno::Reference< sdbc::XRow > xChildRow(
                                            xResultSet, uno::UNO_QUERY );

            if ( !xChildRow.is() )
                ucb_commands::abort(
                        "Unable to get props from children of source!" );

            uno::Reference< XContentAccess > xChildAccess(
                                            xResultSet, uno::UNO_QUERY );

            if ( !xChildAccess.is() )
                ucb_commands::abort( "Unable to get children of source!" );

            if ( xResultSet->first() )
            {
                do
                {
                    uno::Reference< XContent > xChild
                                        = xChildAccess->queryContent();
                    if ( xChild.is() )
                    {
                        // Recursion!
                         GlobalTransferCommandArgument aTransArg(
                                rArg.Operation,               // Operation
                                xChild->getIdentifier()
                                    ->getContentIdentifier(), // SourceURL
                                xNew->getIdentifier()
                                    ->getContentIdentifier(), // TargetURL
                                rtl::OUString(),              // NewTitle;
                                rArg.NameClash );             // NameClash

                        ucb_commands::globalTransfer(
                            xSMgr, xChild, xNew, aTransArg, xChildRow, xEnv );
                    }
                }
                while ( xResultSet->next() );
            }
        }
        catch ( sdbc::SQLException const & )
        {
        }
    }
}

} /* namescpace ucb_commands */

//=========================================================================
//
// UniversalContentBroker implementation ( XCommandProcessor commands ).
//
//=========================================================================

uno::Reference< XCommandInfo > UniversalContentBroker::getCommandInfo()
{
    return uno::Reference< XCommandInfo >(
                            new ucb_commands::CommandProcessorInfo() );
}

//=========================================================================
void UniversalContentBroker::globalTransfer(
                        const GlobalTransferCommandArgument & rArg,
                         const uno::Reference< XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    // Remote optimization: Supply own task environment, which caches (remote)
    // interfaces (progress handler, interaction handler, ...) locally.
    uno::Reference< XCommandEnvironment > xLocalEnv(
                                new ::ucb::CommandEnvironmentProxy( xEnv ) );

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to transfer the content using 'transfer' command.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< XContent > xTarget;
    try
    {
        uno::Reference< XContentIdentifier > xId
            = createContentIdentifier( rArg.TargetURL );
        if ( xId.is() )
            xTarget = queryContent( xId );
    }
    catch ( IllegalIdentifierException const & )
    {
    }

    if ( !xTarget.is() )
        throw CommandAbortedException(); // no target content

    if ( ( rArg.Operation == TransferCommandOperation_COPY ) ||
         ( rArg.Operation == TransferCommandOperation_MOVE ) )
    {
        uno::Reference< XCommandProcessor > xCommandProcessor(
                                                xTarget, uno::UNO_QUERY );
        if ( !xCommandProcessor.is() )
            ucb_commands::abort( "Target is not a XCommandProcessor!" );

        try
        {
            TransferInfo aTransferArg(
                ( rArg.Operation == TransferCommandOperation_MOVE ), // MoveData
                rArg.SourceURL,   // SourceURL
                rArg.NewTitle,    // NewTitle
                rArg.NameClash ); // NameClash

            Command aCommand(
                rtl::OUString::createFromAscii( "transfer" ), // Name
                -1,                                           // Handle
                uno::makeAny( aTransferArg ) );               // Argument

            xCommandProcessor->execute( aCommand, 0, xLocalEnv );

            // Command succeeded. We're done.
            return;
        }
        catch ( InteractiveBadTransferURLException const & )
        {
            // Source URL is not supported by target. Try to transfer
            // the content "manually".
        }
/*
        catch ( CommandAbortedException const & )
        {
        }
        catch ( uno::Exception const & )
        {
        }
*/
    }

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Try to transfer the content "manually".
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< XContent > xSource;
    try
    {
        uno::Reference< XContentIdentifier > xId
            = createContentIdentifier( rArg.SourceURL );
        if ( xId.is() )
            xSource = queryContent( xId );
    }
    catch ( IllegalIdentifierException const & )
    {
    }

    if ( !xSource.is() )
        throw CommandAbortedException(); // no source content

    uno::Reference< XCommandProcessor > xCommandProcessor(
                                                xSource, uno::UNO_QUERY );
    if ( !xCommandProcessor.is() )
        ucb_commands::abort( "Source is not a XCommandProcessor!" );

    // Obtain interesting property values from source...

    uno::Sequence< beans::Property > aProps( 3 );

    aProps[ 0 ].Name   = rtl::OUString::createFromAscii( "IsFolder" );
    aProps[ 0 ].Handle = -1; /* unknown */
    aProps[ 1 ].Name   = rtl::OUString::createFromAscii( "IsDocument" );
    aProps[ 1 ].Handle = -1; /* unknown */
    aProps[ 2 ].Name   = rtl::OUString::createFromAscii( "TargetURL" );
    aProps[ 2 ].Handle = -1; /* unknown */

    Command aGetPropsCommand(
                rtl::OUString::createFromAscii( "getPropertyValues" ),
                -1,
                uno::makeAny( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessor->execute( aGetPropsCommand, 0, xLocalEnv ) >>= xRow;

    if ( !xRow.is() )
        ucb_commands::abort( "Unable to get properties from source object!" );

    // Do it!
    ucb_commands::globalTransfer(
                        m_xSMgr, xSource, xTarget, rArg, xRow, xLocalEnv );

    //////////////////////////////////////////////////////////////////////
    //
    // (3) Delete source, if operation is MOVE.
    //
    //////////////////////////////////////////////////////////////////////

    if ( rArg.Operation == TransferCommandOperation_MOVE )
    {
        try
        {
            Command aCommand(
                rtl::OUString::createFromAscii( "delete" ), // Name
                -1,                                         // Handle
                uno::makeAny( sal_Bool( sal_True ) ) );     // Argument

            xCommandProcessor->execute( aCommand, 0, xLocalEnv );
        }
        catch ( uno::Exception const & )
        {
            OSL_ENSURE( sal_False, "Cannot delete source object!" );
            throw;
        }
    }
}

