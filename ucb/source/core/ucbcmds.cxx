/*************************************************************************
 *
 *  $RCSfile: ucbcmds.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:26:39 $
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

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
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
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
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
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/NameClashException.hpp>
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
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDNAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
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
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _UCBHELPER_COMMANDENVIRONMENTPROXY_HXX
#include <ucbhelper/commandenvironmentproxy.hxx>
#endif
#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
#endif

#ifndef _UCBCMDS_HXX
#include "ucbcmds.hxx"
#endif
#ifndef _UCB_HXX
#include "ucb.hxx"
#endif

using namespace com::sun;
using namespace com::sun::star;

namespace ucb_commands
{

//=========================================================================
//
// struct TransferCommandContext.
//
//=========================================================================

struct TransferCommandContext
{
    uno::Reference< lang::XMultiServiceFactory >        xSMgr;
    uno::Reference< star::ucb::XCommandProcessor >      xProcessor;
    uno::Reference< star::ucb::XCommandEnvironment >    xEnv;
    star::ucb::GlobalTransferCommandArgument            aArg;

    TransferCommandContext(
        const uno::Reference< lang::XMultiServiceFactory > & rxSMgr,
        const uno::Reference< star::ucb::XCommandProcessor > & rxProcessor,
        const uno::Reference< star::ucb::XCommandEnvironment > & rxEnv,
        const star::ucb::GlobalTransferCommandArgument & rArg )
    : xSMgr( rxSMgr ), xProcessor( rxProcessor ), xEnv( rxEnv ), aArg( rArg ) {}
};

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
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

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
    throw()
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL ActiveDataSink::release()
    throw()
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

class CommandProcessorInfo : public cppu::OWeakObject,
                             public star::ucb::XCommandInfo
{
    uno::Sequence< star::ucb::CommandInfo > * m_pInfo;

public:
    CommandProcessorInfo();
    virtual ~CommandProcessorInfo();

    // XInterface methods
    virtual uno::Any SAL_CALL queryInterface( const uno::Type & rType )
        throw( uno::RuntimeException );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XCommandInfo methods
    virtual uno::Sequence< star::ucb::CommandInfo > SAL_CALL getCommands()
        throw( uno::RuntimeException );
    virtual star::ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const rtl::OUString& Name )
        throw( star::ucb::UnsupportedCommandException, uno::RuntimeException );
    virtual star::ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( star::ucb::UnsupportedCommandException, uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByName( const rtl::OUString& Name )
        throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByHandle( sal_Int32 Handle )
        throw( uno::RuntimeException );
};

//=========================================================================
CommandProcessorInfo::CommandProcessorInfo()
{
    m_pInfo = new uno::Sequence< star::ucb::CommandInfo >( 2 );

    (*m_pInfo)[ 0 ]
        = star::ucb::CommandInfo(
            rtl::OUString::createFromAscii( GETCOMMANDINFO_NAME ), // Name
            GETCOMMANDINFO_HANDLE, // Handle
            getCppuVoidType() ); // ArgType
    (*m_pInfo)[ 1 ]
        = star::ucb::CommandInfo(
            rtl::OUString::createFromAscii( GLOBALTRANSFER_NAME ), // Name
            GLOBALTRANSFER_HANDLE, // Handle
            getCppuType(
                static_cast<
                    star::ucb::GlobalTransferCommandArgument * >( 0 ) ) ); // ArgType
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
                        static_cast< star::ucb::XCommandInfo * >( this ) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//=========================================================================
// virtual
void SAL_CALL CommandProcessorInfo::acquire()
    throw()
{
    OWeakObject::acquire();
}

//=========================================================================
// virtual
void SAL_CALL CommandProcessorInfo::release()
    throw()
{
    OWeakObject::release();
}

//=========================================================================
// virtual
uno::Sequence< star::ucb::CommandInfo > SAL_CALL
CommandProcessorInfo::getCommands()
    throw( uno::RuntimeException )
{
    return uno::Sequence< star::ucb::CommandInfo >( *m_pInfo );
}

//=========================================================================
// virtual
star::ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByName( const rtl::OUString& Name )
    throw( star::ucb::UnsupportedCommandException, uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Name == Name )
            return star::ucb::CommandInfo( (*m_pInfo)[ n ] );
    }

    throw star::ucb::UnsupportedCommandException();
}

//=========================================================================
// virtual
star::ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByHandle( sal_Int32 Handle )
    throw( star::ucb::UnsupportedCommandException, uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Handle == Handle )
            return star::ucb::CommandInfo( (*m_pInfo)[ n ] );
    }

    throw star::ucb::UnsupportedCommandException();
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

static uno::Reference< star::ucb::XContent > createNew(
                    const TransferCommandContext & rContext,
                    const uno::Reference< star::ucb::XContent > & xTarget,
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

    uno::Reference< star::ucb::XContentCreator > xCreator(
                                                    xTarget, uno::UNO_QUERY );

    if ( !xCreator.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Folder")),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xEnv,
            rtl::OUString::createFromAscii( "Target is no XContentCreator!" ),
            rContext.xProcessor );
        // Unreachable
    }

    uno::Sequence< star::ucb::ContentInfo > aTypesInfo
                            = xCreator->queryCreatableContentsInfo();

    sal_Int32 nCount = aTypesInfo.getLength();
    if ( !nCount )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Folder")),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xEnv,
            rtl::OUString::createFromAscii( "No types creatable!" ),
            rContext.xProcessor );
        // Unreachable
    }

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Try to find a matching target type for the source object.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< star::ucb::XContent > xNew;
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        sal_Int32 nAttribs = aTypesInfo[ n ].Attributes;
        sal_Bool  bMatch   = sal_False;

        if ( rContext.aArg.Operation
                == star::ucb::TransferCommandOperation_LINK )
        {
            // Create link

            if ( nAttribs & star::ucb::ContentInfoAttribute::KIND_LINK )
            {
                // Match!
                bMatch = sal_True;
            }
        }
        else if ( ( rContext.aArg.Operation
                        == star::ucb::TransferCommandOperation_COPY ) ||
                  ( rContext.aArg.Operation
                        == star::ucb::TransferCommandOperation_MOVE ) )
        {
            // Copy / Move

            // Is source a link? Create link in target folder then.
            if ( bSourceIsLink )
            {
                if ( nAttribs & star::ucb::ContentInfoAttribute::KIND_LINK )
                {
                    // Match!
                    bMatch = sal_True;
                }
            }
            else
            {
                if ( ( bSourceIsFolder ==
                        !!( nAttribs
                            & star::ucb::ContentInfoAttribute::KIND_FOLDER ) )
                     &&
                      ( bSourceIsDocument ==
                        !!( nAttribs
                            & star::ucb::ContentInfoAttribute::KIND_DOCUMENT ) )
                   )
                {
                    // Match!
                    bMatch = sal_True;
                }
            }
        }
        else
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                        rtl::OUString::createFromAscii(
                                            "Unknown transfer operation!" ),
                                        rContext.xProcessor,
                                        -1 ) ),
                              rContext.xEnv );
            // Unreachable
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
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "Folder")),
                                 -1,
                                 uno::makeAny(rContext.aArg.TargetURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    star::ucb::IOErrorCode_CANT_CREATE,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xEnv,
                    rtl::OUString::createFromAscii(
                        "createNewContent failed!" ),
                    rContext.xProcessor );
                // Unreachable
            }
            break;
        }
    }

    return xNew;
}

//=========================================================================
static void transferProperties(
    const TransferCommandContext & rContext,
    const uno::Reference< star::ucb::XCommandProcessor > & xCommandProcessorS,
    const uno::Reference< star::ucb::XCommandProcessor > & xCommandProcessorN )
        throw( uno::Exception )
{
    star::ucb::Command aGetPropertySetInfoCommand(
                rtl::OUString::createFromAscii( "getPropertySetInfo" ),
                -1,
                uno::Any() );

    uno::Reference< beans::XPropertySetInfo > xInfo;
    xCommandProcessorS->execute( aGetPropertySetInfoCommand, 0, rContext.xEnv )
        >>= xInfo;

    if ( !xInfo.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xEnv,
            rtl::OUString::createFromAscii(
                "Unable to get propertyset info from source object!" ),
            rContext.xProcessor );
        // Unreachable
    }

    uno::Sequence< beans::Property > aAllProps = xInfo->getProperties();

    star::ucb::Command aGetPropsCommand1(
                rtl::OUString::createFromAscii( "getPropertyValues" ),
                -1,
                uno::makeAny( aAllProps ) );

    uno::Reference< sdbc::XRow > xRow1;
    xCommandProcessorS->execute(
        aGetPropsCommand1, 0, rContext.xEnv ) >>= xRow1;

    if ( !xRow1.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xEnv,
            rtl::OUString::createFromAscii(
                "Unable to get properties from source object!" ),
            rContext.xProcessor );
        // Unreachable
    }

    // Assemble data structure for setPropertyValues command.

    // Note: Make room for additional Title and TargetURL too. -> + 2
    uno::Sequence< beans::PropertyValue > aPropValues(
                                                aAllProps.getLength() + 2 );

    sal_Bool bHasTitle = ( rContext.aArg.NewTitle.getLength() == 0 );
    sal_Bool bHasTargetURL = ( rContext.aArg.Operation
                                != star::ucb::TransferCommandOperation_LINK );

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
                aValue <<= rContext.aArg.NewTitle;
            }
        }
        else if ( rCurrProp.Name.compareToAscii( "TargetURL" ) == 0 )
        {
            // Supply source URL as link target for the new link to create.
            if ( !bHasTargetURL )
            {
                bHasTargetURL = sal_True;
                aValue <<= rContext.aArg.SourceURL;
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
    if ( !bHasTitle && ( rContext.aArg.NewTitle.getLength() > 0 ) )
    {
        aPropValues[ nWritePos ].Name
            = rtl::OUString::createFromAscii( "Title" );
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rContext.aArg.NewTitle;

        nWritePos++;
    }

    // TargetURL needed, but not set yet?
    if ( !bHasTargetURL && ( rContext.aArg.Operation
                                == star::ucb::TransferCommandOperation_LINK ) )
    {
        aPropValues[ nWritePos ].Name
            = rtl::OUString::createFromAscii( "TargetURL" );
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rContext.aArg.SourceURL;

        nWritePos++;
    }

    aPropValues.realloc( nWritePos );

    // Set properties at new object.

    star::ucb::Command aSetPropsCommand(
                rtl::OUString::createFromAscii( "setPropertyValues" ),
                -1,
                uno::makeAny( aPropValues ) );

    xCommandProcessorN->execute( aSetPropsCommand, 0, rContext.xEnv );

    // @@@ What to do with source props that are not supported by the
    //     new object? addProperty ???
}

//=========================================================================
static uno::Reference< io::XInputStream > getInputStream(
    const TransferCommandContext & rContext,
    const uno::Reference< star::ucb::XCommandProcessor > & xCommandProcessorS )
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

        star::ucb::OpenCommandArgument2 aArg;
        aArg.Mode       = star::ucb::OpenMode::DOCUMENT;
        aArg.Priority   = 0; // unused
        aArg.Sink       = xSink;
        aArg.Properties = uno::Sequence< beans::Property >( 0 ); // unused

        star::ucb::Command aOpenCommand(
                                rtl::OUString::createFromAscii( "open" ),
                                -1,
                                uno::makeAny( aArg ) );

        xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv );
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
                rContext.xSMgr->createInstance(
                    rtl::OUString::createFromAscii( "com.sun.star.io.Pipe" ) ),
                uno::UNO_QUERY );

            if ( xOutputStream.is() )
            {
                star::ucb::OpenCommandArgument2 aArg;
                aArg.Mode       = star::ucb::OpenMode::DOCUMENT;
                aArg.Priority   = 0; // unused
                aArg.Sink       = xOutputStream;
                aArg.Properties = uno::Sequence< beans::Property >( 0 );

                star::ucb::Command aOpenCommand(
                                    rtl::OUString::createFromAscii( "open" ),
                                    -1,
                                    uno::makeAny( aArg ) );

                xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv );

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
    const TransferCommandContext & rContext,
    const uno::Reference< star::ucb::XCommandProcessor > & xCommandProcessorS )
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

    star::ucb::OpenCommandArgument2 aArg;
    aArg.Mode       = star::ucb::OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = 0;
    aArg.Properties = aProps;

    star::ucb::Command aOpenCommand( rtl::OUString::createFromAscii( "open" ),
                                     -1,
                                     uno::makeAny( aArg ) );
    try
    {
        uno::Reference< star::ucb::XDynamicResultSet > xSet;
        xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv ) >>= xSet;

        if ( xSet.is() )
            xResultSet = xSet->getStaticResultSet();
    }
    catch ( uno::Exception const & )
    {
    }

    return xResultSet;
}

//=========================================================================
static void globalTransfer(
        const TransferCommandContext & rContext,
        const uno::Reference< star::ucb::XContent > & xSource,
        const uno::Reference< star::ucb::XContent > & xTarget,
        const uno::Reference< sdbc::XRow > & xSourceProps )
    throw( uno::Exception )
{
    // IsFolder: property is required.
    sal_Bool bSourceIsFolder = xSourceProps->getBoolean( 1 );
    if ( !bSourceIsFolder && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            rtl::OUString::createFromAscii(
                                "Unable to get property 'IsFolder' "
                                "from source object!" ),
                            rContext.xProcessor ) ),
            rContext.xEnv );
        // Unreachable
    }

    // IsDocument: property is required.
    sal_Bool bSourceIsDocument = xSourceProps->getBoolean( 2 );
    if ( !bSourceIsDocument && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            rtl::OUString::createFromAscii(
                                "Unable to get property 'IsDocument' "
                                "from source object!" ),
                            rContext.xProcessor ) ),
            rContext.xEnv );
        // Unreachable
    }

    // TargetURL: property is optional.
    sal_Bool bSourceIsLink = ( xSourceProps->getString( 3 ).getLength() > 0 );

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to find a matching target type for the source object and
    //     create a new, empty object of that type.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< star::ucb::XContent > xNew = createNew( rContext,
                                                            xTarget,
                                                            bSourceIsFolder,
                                                            bSourceIsDocument,
                                                            bSourceIsLink );
    if ( !xNew.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "Folder")),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xEnv,
            rtl::OUString::createFromAscii(
                "No matching content type at target!" ),
            rContext.xProcessor );
        // Unreachable
    }

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Transfer property values from source to new object.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< star::ucb::XCommandProcessor > xCommandProcessorN(
                                                    xNew, uno::UNO_QUERY );
    if ( !xCommandProcessorN.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(
                                      xNew->getIdentifier()->
                                                getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xEnv,
            rtl::OUString::createFromAscii(
                "New content is not a XCommandProcessor!" ),
            rContext.xProcessor );
        // Unreachable
    }

    // Obtain all properties from source.

    uno::Reference< star::ucb::XCommandProcessor > xCommandProcessorS(
                                                    xSource, uno::UNO_QUERY );
    if ( !xCommandProcessorS.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xEnv,
            rtl::OUString::createFromAscii(
                "Source content is not a XCommandProcessor!" ),
            rContext.xProcessor );
        // Unreachable
    }

    transferProperties( rContext, xCommandProcessorS, xCommandProcessorN );

    //////////////////////////////////////////////////////////////////////
    //
    // (3) Try to obtain a data stream from source.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< io::XInputStream > xInputStream;

    if ( bSourceIsDocument && ( rContext.aArg.Operation
                                != star::ucb::TransferCommandOperation_LINK ) )
        xInputStream = getInputStream( rContext, xCommandProcessorS );

    //////////////////////////////////////////////////////////////////////
    //
    // (4) Try to obtain a resultset (children) from source.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< sdbc::XResultSet > xResultSet;

    if ( bSourceIsFolder && ( rContext.aArg.Operation
                                != star::ucb::TransferCommandOperation_LINK ) )
        xResultSet = getResultSet( rContext, xCommandProcessorS );

    //////////////////////////////////////////////////////////////////////
    //
    // (5) Insert (store) new content.
    //
    //////////////////////////////////////////////////////////////////////

    try
    {
        star::ucb::InsertCommandArgument aArg;
        aArg.Data = xInputStream;

        switch ( rContext.aArg.NameClash )
        {
            case star::ucb::NameClash::OVERWRITE:
                aArg.ReplaceExisting = sal_True;
                break;

            case star::ucb::NameClash::ERROR:
            case star::ucb::NameClash::RENAME:
            case star::ucb::NameClash::KEEP: // deprecated
            case star::ucb::NameClash::ASK:
                aArg.ReplaceExisting = sal_False;
                break;

            default:
                aArg.ReplaceExisting = sal_False;
                OSL_ENSURE( sal_False, "Unknown nameclash directive!" );
                break;
        }

        star::ucb::Command aInsertCommand(
                                rtl::OUString::createFromAscii( "insert" ),
                                  -1,
                                  uno::makeAny( aArg ) );

        xCommandProcessorN->execute( aInsertCommand, 0, rContext.xEnv );
    }
    catch ( star::ucb::NameClashException const & )
    {
        // The 'insert' command throws a NameClashException if the parameter
        // ReplaceExisting of the command's argument was set to false and
        // there exists a resource with a clashing name in the target folder
        // of the operation.

        // 'insert' command has no direct support for name clashes other
        // than ERROR ( ReplaceExisting == false ) and OVERWRITE
        // ( ReplaceExisting == true ). So we have to implement the
        // other name clash handling directives on top of the content.

        // @@@ 'insert' command should be extended that it accepts a
        //     name clash handling directive, exactly like 'transfer' command.

        switch ( rContext.aArg.NameClash )
        {
            case star::ucb::NameClash::OVERWRITE:
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        star::ucb::UnsupportedNameClashException(
                            rtl::OUString::createFromAscii(
                                "BUG: insert + replace == true MUST NOT "
                                "throw NameClashException." ),
                            rContext.xProcessor,
                            rContext.aArg.NameClash ) ),
                    rContext.xEnv );
                // Unreachable
            }

            case star::ucb::NameClash::ERROR:
                throw;

            case star::ucb::NameClash::RENAME:
            {
                // "invent" a new valid title.

                sal_Int32 nTry = 0;

                // Obtain old title.
                uno::Sequence< beans::Property > aProps( 1 );
                aProps[ 0 ].Name   = rtl::OUString::createFromAscii( "Title" );
                aProps[ 0 ].Handle = -1;

                star::ucb::Command aGetPropsCommand(
                        rtl::OUString::createFromAscii( "getPropertyValues" ),
                        -1,
                        uno::makeAny( aProps ) );

                uno::Reference< sdbc::XRow > xRow;
                xCommandProcessorN->execute(
                    aGetPropsCommand, 0, rContext.xEnv )  >>= xRow;

                if ( !xRow.is() )
                {
                    uno::Any aProps
                        = uno::makeAny(
                                 beans::PropertyValue(
                                     rtl::OUString(
                                         RTL_CONSTASCII_USTRINGPARAM("Uri")),
                                     -1,
                                     uno::makeAny(
                                         xNew->getIdentifier()->
                                                   getContentIdentifier()),
                                     beans::PropertyState_DIRECT_VALUE));
                    ucbhelper::cancelCommandExecution(
                        star::ucb::IOErrorCode_CANT_READ,
                        uno::Sequence< uno::Any >(&aProps, 1),
                        rContext.xEnv,
                        rtl::OUString::createFromAscii(
                            "Unable to get properties from new object!" ),
                        rContext.xProcessor );
                    // Unreachable
                }

                rtl::OUString aOldTitle = xRow->getString( 1 );
                if ( !aOldTitle.getLength() )
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny( beans::UnknownPropertyException(
                                        rtl::OUString::createFromAscii(
                                            "Unable to get property 'Title' "
                                            "from new object!" ),
                                        rContext.xProcessor ) ),
                        rContext.xEnv );
                    // Unreachable
                }

                // Some pseudo-intelligence for not destroying file extensions.
                rtl::OUString aOldTitlePre;
                rtl::OUString aOldTitlePost;
                sal_Int32 nPos = aOldTitle.lastIndexOf( '.' );
                if ( nPos != -1 )
                {
                    aOldTitlePre = aOldTitle.copy( 0, nPos );
                    aOldTitlePost = aOldTitle.copy( nPos );
                }
                else
                    aOldTitlePre = aOldTitle;

                if ( nPos > 0 )
                    aOldTitlePre += rtl::OUString::createFromAscii( "_" );

                sal_Bool bContinue = sal_True;
                do
                {
                    nTry++;

                    rtl::OUString aNewTitle = aOldTitlePre;
                    aNewTitle += rtl::OUString::valueOf( nTry );
                    aNewTitle += aOldTitlePost;

                    uno::Sequence< beans::PropertyValue > aValues( 1 );
                    aValues[ 0 ].Name
                        = rtl::OUString::createFromAscii( "Title" );
                    aValues[ 0 ].Handle = -1;
                    aValues[ 0 ].Value <<= aNewTitle;

                    star::ucb::Command aSetPropsCommand(
                        rtl::OUString::createFromAscii( "setPropertyValues" ),
                        -1,
                        uno::makeAny( aValues ) );

                    // Set new title
                    xCommandProcessorN->execute(
                        aSetPropsCommand, 0, rContext.xEnv );

                    // Retry inserting the content.
                    try
                    {
                        // Previous try may have read from stream. Seek to
                        // begin (if optional interface XSeekable is supported)
                        // or get a new stream.
                        if ( xInputStream.is() )
                        {
                            uno::Reference< io::XSeekable > xSeekable(
                                                xInputStream, uno::UNO_QUERY );
                            if ( xSeekable.is() )
                            {
                                try
                                {
                                    xSeekable->seek( 0 );
                                }
                                catch ( lang::IllegalArgumentException const & )
                                {
                                    xInputStream = 0;
                                }
                                catch ( io::IOException const & )
                                {
                                    xInputStream = 0;
                                }
                            }
                            else
                                xInputStream = 0;

                            if ( !xInputStream.is() )
                            {
                                xInputStream = getInputStream(
                                                rContext, xCommandProcessorS );
                                if ( !xInputStream.is() )
                                {
                                    uno::Any aProps
                                        = uno::makeAny(
                                                 beans::PropertyValue(
                                                     rtl::OUString(
                                                  RTL_CONSTASCII_USTRINGPARAM(
                                                             "Uri")),
                                                     -1,
                                                     uno::makeAny(
                                                         xNew->
                                                             getIdentifier()->
                                                      getContentIdentifier()),
                                          beans::PropertyState_DIRECT_VALUE));
                                    ucbhelper::cancelCommandExecution(
                                        star::ucb::IOErrorCode_CANT_READ,
                                        uno::Sequence< uno::Any >(&aProps, 1),
                                        rContext.xEnv,
                                        rtl::OUString::createFromAscii(
                                            "Got no data stream from source!" ),
                                        rContext.xProcessor );
                                    // Unreachable
                                }
                            }
                        }

                        star::ucb::InsertCommandArgument aArg;
                        aArg.Data = xInputStream;
                        aArg.ReplaceExisting = sal_False;

                        star::ucb::Command aInsertCommand(
                                    rtl::OUString::createFromAscii( "insert" ),
                                    -1,
                                    uno::makeAny( aArg ) );

                        xCommandProcessorN->execute(
                            aInsertCommand, 0, rContext.xEnv );

                        // Success!
                        bContinue = sal_False;
                    }
                    catch ( uno::Exception const & )
                    {
                    }
                }
                while ( bContinue && ( nTry < 50 ) );

                if ( nTry == 50 )
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            star::ucb::UnsupportedNameClashException(
                                rtl::OUString::createFromAscii(
                                    "Unable to resolve name clash!" ),
                                rContext.xProcessor,
                                star::ucb::NameClash::RENAME ) ),
                    rContext.xEnv );
                    // Unreachable
                }
            }
            break;

            case star::ucb::NameClash::KEEP: // deprecated
            case star::ucb::NameClash::ASK:
            default:
            {
                ucbhelper::cancelCommandExecution(
                    uno::makeAny(
                        star::ucb::UnsupportedNameClashException(
                            rtl::OUString(),
                            rContext.xProcessor,
                            rContext.aArg.NameClash ) ),
                    rContext.xEnv );
                // Unreachable
            }
        }
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
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "Uri")),
                                 -1,
                                 uno::makeAny(rContext.aArg.SourceURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    star::ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xEnv,
                    rtl::OUString::createFromAscii(
                        "Unable to get properties from children of source!" ),
                    rContext.xProcessor );
                // Unreachable
            }

            uno::Reference< star::ucb::XContentAccess > xChildAccess(
                                                xResultSet, uno::UNO_QUERY );

            if ( !xChildAccess.is() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                     "Uri")),
                                 -1,
                                 uno::makeAny(rContext.aArg.SourceURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    star::ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xEnv,
                    rtl::OUString::createFromAscii(
                        "Unable to get children of source!" ),
                    rContext.xProcessor );
                // Unreachable
            }

            if ( xResultSet->first() )
            {
                do
                {
                    uno::Reference< star::ucb::XContent > xChild
                                        = xChildAccess->queryContent();
                    if ( xChild.is() )
                    {
                        // Recursion!
                        star::ucb::GlobalTransferCommandArgument aTransArg(
                                rContext.aArg.Operation,      // Operation
                                xChild->getIdentifier()
                                    ->getContentIdentifier(), // SourceURL
                                xNew->getIdentifier()
                                    ->getContentIdentifier(), // TargetURL
                                rtl::OUString(),              // NewTitle;
                                rContext.aArg.NameClash );    // NameClash

                        ucb_commands::globalTransfer( rContext,
                                                      xChild,
                                                      xNew,
                                                      xChildRow );
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

uno::Reference< star::ucb::XCommandInfo >
UniversalContentBroker::getCommandInfo()
{
    return uno::Reference< star::ucb::XCommandInfo >(
                            new ucb_commands::CommandProcessorInfo() );
}

//=========================================================================
void UniversalContentBroker::globalTransfer(
            const star::ucb::GlobalTransferCommandArgument & rArg,
            const uno::Reference< star::ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    // Remote optimization: Supply own task environment, which caches (remote)
    // interfaces (progress handler, interaction handler, ...) locally.
    uno::Reference< star::ucb::XCommandEnvironment > xLocalEnv(
                                new ::ucb::CommandEnvironmentProxy( xEnv ) );

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to transfer the content using 'transfer' command.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< star::ucb::XContent > xTarget;
    uno::Reference< star::ucb::XContentIdentifier > xId
            = createContentIdentifier( rArg.TargetURL );
    if ( xId.is() )
    {
        try
        {
            xTarget = queryContent( xId );
        }
        catch ( star::ucb::IllegalIdentifierException const & )
        {
        }
    }

    if ( !xTarget.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "Uri")),
                                  -1,
                                  uno::makeAny(rArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Can't instanciate target object!" ),
            this );
        // Unreachable
    }

    if ( ( rArg.Operation == star::ucb::TransferCommandOperation_COPY ) ||
         ( rArg.Operation == star::ucb::TransferCommandOperation_MOVE ) )
    {
        uno::Reference< star::ucb::XCommandProcessor > xCommandProcessor(
                                                    xTarget, uno::UNO_QUERY );
        if ( !xCommandProcessor.is() )
        {
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "Uri")),
                             -1,
                             uno::makeAny(rArg.TargetURL),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                star::ucb::IOErrorCode_CANT_READ,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                rtl::OUString::createFromAscii(
                    "Target content is not a XCommandProcessor!" ),
                this );
            // Unreachable
        }

        try
        {
            star::ucb::TransferInfo aTransferArg(
                ( rArg.Operation
                    == star::ucb::TransferCommandOperation_MOVE ), // MoveData
                rArg.SourceURL,   // SourceURL
                rArg.NewTitle,    // NewTitle
                rArg.NameClash ); // NameClash

            star::ucb::Command aCommand(
                rtl::OUString::createFromAscii( "transfer" ), // Name
                -1,                                           // Handle
                uno::makeAny( aTransferArg ) );               // Argument

            xCommandProcessor->execute( aCommand, 0, xLocalEnv );

            // Command succeeded. We're done.
            return;
        }
        catch ( star::ucb::InteractiveBadTransferURLException const & )
        {
            // Source URL is not supported by target. Try to transfer
            // the content "manually".
        }
        catch( star::ucb::UnsupportedCommandException const & )
        {
            // 'transfer'-command is not supported by commandprocessor.
            // Try to transfer manually.
        }
/*
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

    uno::Reference< star::ucb::XContent > xSource;
    try
    {
        uno::Reference< star::ucb::XContentIdentifier > xId
            = createContentIdentifier( rArg.SourceURL );
        if ( xId.is() )
            xSource = queryContent( xId );
    }
    catch ( star::ucb::IllegalIdentifierException const & )
    {
    }

    if ( !xSource.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Can't instanciate source object!" ),
            this );
        // Unreachable
    }

    uno::Reference< star::ucb::XCommandProcessor > xCommandProcessor(
                                                xSource, uno::UNO_QUERY );
    if ( !xCommandProcessor.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Source content is not a XCommandProcessor!" ),
            this );
        // Unreachable
    }

    // Obtain interesting property values from source...

    uno::Sequence< beans::Property > aProps( 3 );

    aProps[ 0 ].Name   = rtl::OUString::createFromAscii( "IsFolder" );
    aProps[ 0 ].Handle = -1; /* unknown */
    aProps[ 1 ].Name   = rtl::OUString::createFromAscii( "IsDocument" );
    aProps[ 1 ].Handle = -1; /* unknown */
    aProps[ 2 ].Name   = rtl::OUString::createFromAscii( "TargetURL" );
    aProps[ 2 ].Handle = -1; /* unknown */

    star::ucb::Command aGetPropsCommand(
                rtl::OUString::createFromAscii( "getPropertyValues" ),
                -1,
                uno::makeAny( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessor->execute( aGetPropsCommand, 0, xLocalEnv ) >>= xRow;

    if ( !xRow.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            star::ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Unable to get properties from source object!" ),
            this );
        // Unreachable
    }

    // Do it!
    ucb_commands::globalTransfer(
        ucb_commands::TransferCommandContext( m_xSMgr, this, xLocalEnv, rArg ),
        xSource, xTarget, xRow );

    //////////////////////////////////////////////////////////////////////
    //
    // (3) Delete source, if operation is MOVE.
    //
    //////////////////////////////////////////////////////////////////////

    if ( rArg.Operation == star::ucb::TransferCommandOperation_MOVE )
    {
        try
        {
            star::ucb::Command aCommand(
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

