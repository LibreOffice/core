/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <cstdio>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>

#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/propertyvalueset.hxx>

#include "cmis_content.hxx"
#include "cmis_provider.hxx"

using namespace com::sun::star;

namespace cmis
{
    Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
        ContentProvider *pProvider, const uno::Reference< ucb::XContentIdentifier >& Identifier)
            throw ( ucb::ContentCreationException )
        : ContentImplHelper( rxSMgr, pProvider, Identifier ),
        m_pProvider( pProvider )
    {
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "TODO - New Content ('%s')\n", rtl::OUStringToOString(m_xIdentifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8).getStr());
#endif
        // TODO Implement me
    }

    Content::Content( const uno::Reference< lang::XMultiServiceFactory >& rxSMgr, ContentProvider *pProvider,
        const uno::Reference< ucb::XContentIdentifier >& Identifier,
        sal_Bool /*bIsFolder*/)
            throw ( ucb::ContentCreationException )
        : ContentImplHelper( rxSMgr, pProvider, Identifier ),
        m_pProvider( pProvider )
    {
#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "TODO - Create Content ('%s')\n", rtl::OUStringToOString(m_xIdentifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8).getStr());
#endif
        // TODO Implement me
    }

    Content::~Content()
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::~Content()\n" );
#endif
        // TODO Implement me
    }

    bool Content::isFolder(const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/)
    {
        bool bRet = false;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::isFolder()\n" );
#endif
        // TODO Implement me

        return bRet;
    }

    uno::Any Content::getBadArgExcept()
    {
        return uno::makeAny( lang::IllegalArgumentException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Wrong argument type!")),
            static_cast< cppu::OWeakObject * >( this ), -1) );
    }

    uno::Reference< sdbc::XRow > Content::getPropertyValues(
            const uno::Sequence< beans::Property >& /*rProperties*/,
            const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
    {
        uno::Reference< sdbc::XRow > rRow;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::getPropertyValues()\n" );
#endif
        // TODO Implement me

        return rRow;
    }

    void Content::queryChildren( ContentRefList& /*rChildren*/ )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::queryChildren()\n" );
#endif
        // TODO Implement me
    }

    uno::Any Content::open(const ucb::OpenCommandArgument2 & /*rArg*/,
        const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
            throw( uno::Exception )
    {
        uno::Any aRet;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::open()\n" );
#endif
        // TODO Implement me

        return aRet;
    }

    void Content::transfer( const ucb::TransferInfo& /*rTransferInfo*/,
        const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
            throw( uno::Exception )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::transfer()\n" );
#endif
        // TODO Implement me
    }

    void Content::insert( const uno::Reference< io::XInputStream > & /*xInputStream*/,
        sal_Bool /*bReplaceExisting*/, const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
            throw( uno::Exception )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::insert()\n" );
#endif
        // TODO Implement me
    }

    void Content::destroy( sal_Bool /*bDeletePhysical*/ ) throw( uno::Exception )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::destroy()\n" );
#endif
        // TODO Implement me
    }

    const int TRANSFER_BUFFER_SIZE = 65536;

    void Content::copyData(
        uno::Reference< io::XInputStream > xIn,
        uno::Reference< io::XOutputStream > xOut )
    {
        uno::Sequence< sal_Int8 > theData( TRANSFER_BUFFER_SIZE );

        while ( xIn->readBytes( theData, TRANSFER_BUFFER_SIZE ) > 0 )
            xOut->writeBytes( theData );

        xOut->closeOutput();
    }

    uno::Sequence< uno::Any > Content::setPropertyValues(
            const uno::Sequence< beans::PropertyValue >& rValues,
            const uno::Reference< ucb::XCommandEnvironment >& )
    {
        sal_Int32 nCount = rValues.getLength();
        uno::Sequence< uno::Any > aRet( nCount );

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::setPropertyValue()\n" );
#endif
        // TODO Implement me

        return aRet;
    }

    sal_Bool Content::feedSink( uno::Reference< uno::XInterface> /*aSink*/,
        const uno::Reference< ucb::XCommandEnvironment >& /*xEnv*/ )
    {
        sal_Bool bRet = sal_False;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::feedSink()\n" );
#endif
        // TODO Implement me

        return bRet;
    }

    sal_Bool Content::exchangeIdentity( const uno::Reference< ucb::XContentIdentifier >& /*xNewId*/ )
    {
        sal_Bool bRet = sal_False;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::exchangeIdentity()\n" );
#endif
        // TODO Implement me

        return bRet;
    }

    uno::Sequence< beans::Property > Content::getProperties(
            const uno::Reference< ucb::XCommandEnvironment > & /*xEnv*/ )
    {
        static const beans::Property aGenericProperties[] =
        {
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsDocument" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFolder" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ),
                -1, getCppuType( static_cast< const rtl::OUString * >( 0 ) ),
                beans::PropertyAttribute::BOUND ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ),
                -1, getCppuBooleanType(),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateCreated" ) ),
                -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateModified" ) ),
                -1, getCppuType( static_cast< const util::DateTime * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
            beans::Property( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ),
                -1, getCppuType( static_cast< const sal_Int64 * >( 0 ) ),
                beans::PropertyAttribute::BOUND | beans::PropertyAttribute::READONLY ),
        };

        const int nProps = SAL_N_ELEMENTS(aGenericProperties);
        return uno::Sequence< beans::Property > ( aGenericProperties, nProps );
    }

    uno::Sequence< ucb::CommandInfo > Content::getCommands(
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    {
        static ucb::CommandInfo aCommandInfoTable[] =
        {
            // Required commands
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getCommandInfo" ) ),
              -1, getCppuVoidType() ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertySetInfo" ) ),
              -1, getCppuVoidType() ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "getPropertyValues" ) ),
              -1, getCppuType( static_cast<uno::Sequence< beans::Property > * >( 0 ) ) ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
              -1, getCppuType( static_cast<uno::Sequence< beans::PropertyValue > * >( 0 ) ) ),

            // Optional standard commands
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "delete" ) ),
              -1, getCppuBooleanType() ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "insert" ) ),
              -1, getCppuType( static_cast<ucb::InsertCommandArgument * >( 0 ) ) ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ),
              -1, getCppuType( static_cast<ucb::OpenCommandArgument2 * >( 0 ) ) ),

            // Folder Only, omitted if not a folder
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "transfer" ) ),
              -1, getCppuType( static_cast<ucb::TransferInfo * >( 0 ) ) ),
            ucb::CommandInfo
            ( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "createNewContent" ) ),
              -1, getCppuType( static_cast<ucb::ContentInfo * >( 0 ) ) )
        };

        const int nProps = SAL_N_ELEMENTS(aCommandInfoTable);
        return uno::Sequence< ucb::CommandInfo >(aCommandInfoTable, isFolder(xEnv) ? nProps : nProps - 2);
    }

    ::rtl::OUString Content::getParentURL()
    {
        rtl::OUString sRet;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::getParentURL()\n" );
#endif
        // TODO Implement me

        return sRet;
    }

    XTYPEPROVIDER_COMMON_IMPL( Content );

    void SAL_CALL Content::acquire() throw()
    {
        ContentImplHelper::acquire();
    }

    void SAL_CALL Content::release() throw()
    {
        ContentImplHelper::release();
    }

    uno::Any SAL_CALL Content::queryInterface( const uno::Type & rType ) throw ( uno::RuntimeException )
    {
        uno::Any aRet = cppu::queryInterface( rType, static_cast< ucb::XContentCreator * >( this ) );
        return aRet.hasValue() ? aRet : ContentImplHelper::queryInterface(rType);
    }

    rtl::OUString SAL_CALL Content::getImplementationName() throw( uno::RuntimeException )
    {
       return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.CmisContent"));
    }

    uno::Sequence< rtl::OUString > SAL_CALL Content::getSupportedServiceNames()
           throw( uno::RuntimeException )
    {
           uno::Sequence< rtl::OUString > aSNS( 1 );
           aSNS.getArray()[ 0 ] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.CmisContent"));
           return aSNS;
    }

    rtl::OUString SAL_CALL Content::getContentType() throw( uno::RuntimeException )
    {
        return isFolder(uno::Reference< ucb::XCommandEnvironment >())
            ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FOLDER_TYPE ))
            : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FILE_TYPE ));
    }

    uno::Any SAL_CALL Content::execute(
        const ucb::Command& aCommand,
        sal_Int32 /*CommandId*/,
        const uno::Reference< ucb::XCommandEnvironment >& xEnv )
            throw( uno::Exception, ucb::CommandAbortedException, uno::RuntimeException )
    {
        uno::Any aRet;

        if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getPropertyValues" ) ))
        {
            uno::Sequence< beans::Property > Properties;
            if ( !( aCommand.Argument >>= Properties ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet <<= getPropertyValues( Properties, xEnv );
        }
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getPropertySetInfo" ) ))
            aRet <<= getPropertySetInfo( xEnv, sal_False );
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "getCommandInfo" ) ))
            aRet <<= getCommandInfo( xEnv, sal_False );
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "open" ) ))
        {
            ucb::OpenCommandArgument2 aOpenCommand;
            if ( !( aCommand.Argument >>= aOpenCommand ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet = open( aOpenCommand, xEnv );
        }
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "transfer" ) ))
        {
            ucb::TransferInfo transferArgs;
            if ( !( aCommand.Argument >>= transferArgs ) )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            transfer( transferArgs, xEnv );
        }
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "setPropertyValues" ) ))
        {
            uno::Sequence< beans::PropertyValue > aProperties;
            if ( !( aCommand.Argument >>= aProperties ) || !aProperties.getLength() )
                ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet <<= setPropertyValues( aProperties, xEnv );
        }
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "createNewContent" ) )
                 && isFolder( xEnv ) )
        {
            ucb::ContentInfo arg;
            if ( !( aCommand.Argument >>= arg ) )
                    ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            aRet <<= createNewContent( arg );
        }
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "insert" ) ))
        {
            ucb::InsertCommandArgument arg;
            if ( !( aCommand.Argument >>= arg ) )
                    ucbhelper::cancelCommandExecution ( getBadArgExcept (), xEnv );
            insert( arg.Data, arg.ReplaceExisting, xEnv );
        }
        else if (aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "delete" ) ))
        {
            sal_Bool bDeletePhysical = sal_False;
            aCommand.Argument >>= bDeletePhysical;

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::execute() - delete\n" );
#endif
            // TODO Actually delete it

            destroy( bDeletePhysical );
        }
        else
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "cmis::Content::execute() - UNKNOWN COMMAND\n" );
#endif

            ucbhelper::cancelCommandExecution
                ( uno::makeAny( ucb::UnsupportedCommandException
                  ( rtl::OUString(),
                    static_cast< cppu::OWeakObject * >( this ) ) ),
                  xEnv );
        }

        return aRet;
    }

    void SAL_CALL Content::abort( sal_Int32 /*CommandId*/ ) throw( uno::RuntimeException )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "TODO - cmis::Content::abort()\n" );
#endif
        // TODO Implement me
    }

    uno::Sequence< ucb::ContentInfo > SAL_CALL Content::queryCreatableContentsInfo()
            throw( uno::RuntimeException )
    {
        return queryCreatableContentsInfo( uno::Reference< ucb::XCommandEnvironment >() );
    }

    uno::Reference< ucb::XContent > SAL_CALL Content::createNewContent(
            const ucb::ContentInfo& Info ) throw( uno::RuntimeException )
    {
        bool create_document;
        const char *name;

        if ( Info.Type.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( CMIS_FILE_TYPE ) ) )
            create_document = true;
        else if ( Info.Type.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( CMIS_FOLDER_TYPE ) ) )
            create_document = false;
        else
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "Failed to create new content '%s'",
                    rtl::OUStringToOString(Info.Type, RTL_TEXTENCODING_UTF8).getStr() );
#endif
            return uno::Reference< ucb::XContent >();
        }

#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "createNewContent (%d)", (int) create_document );
#endif

        rtl::OUString aURL = m_xIdentifier->getContentIdentifier();

        if ( ( aURL.lastIndexOf( '/' ) + 1 ) != aURL.getLength() )
                aURL += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

        name = create_document ? "[New_Content]" : "[New_Collection]";
        aURL += rtl::OUString::createFromAscii( name );

        uno::Reference< ucb::XContentIdentifier > xId(new ::ucbhelper::ContentIdentifier(m_xSMgr, aURL));

        try
        {
            return new ::cmis::Content( m_xSMgr, m_pProvider, xId, !create_document );
        } catch ( ucb::ContentCreationException & )
        {
                return uno::Reference< ucb::XContent >();
        }
    }

    uno::Sequence< uno::Type > SAL_CALL Content::getTypes() throw( uno::RuntimeException )
    {
        if ( isFolder( uno::Reference< ucb::XCommandEnvironment >() ) )
        {
            static cppu::OTypeCollection aFolderCollection
                (CPPU_TYPE_REF( lang::XTypeProvider ),
                 CPPU_TYPE_REF( lang::XServiceInfo ),
                 CPPU_TYPE_REF( lang::XComponent ),
                 CPPU_TYPE_REF( ucb::XContent ),
                 CPPU_TYPE_REF( ucb::XCommandProcessor ),
                 CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                 CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                 CPPU_TYPE_REF( beans::XPropertyContainer ),
                 CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                 CPPU_TYPE_REF( container::XChild ),
                 CPPU_TYPE_REF( ucb::XContentCreator ) );
            return aFolderCollection.getTypes();
        }
        else
        {
            static cppu::OTypeCollection aFileCollection
                (CPPU_TYPE_REF( lang::XTypeProvider ),
                 CPPU_TYPE_REF( lang::XServiceInfo ),
                 CPPU_TYPE_REF( lang::XComponent ),
                 CPPU_TYPE_REF( ucb::XContent ),
                 CPPU_TYPE_REF( ucb::XCommandProcessor ),
                 CPPU_TYPE_REF( beans::XPropertiesChangeNotifier ),
                 CPPU_TYPE_REF( ucb::XCommandInfoChangeNotifier ),
                 CPPU_TYPE_REF( beans::XPropertyContainer ),
                 CPPU_TYPE_REF( beans::XPropertySetInfoChangeNotifier ),
                 CPPU_TYPE_REF( container::XChild ) );

            return aFileCollection.getTypes();
        }
    }


    uno::Sequence< ucb::ContentInfo > Content::queryCreatableContentsInfo(
        const uno::Reference< ucb::XCommandEnvironment >& xEnv)
            throw( uno::RuntimeException )
    {
        if ( isFolder( xEnv ) )
        {
            uno::Sequence< ucb::ContentInfo > seq(2);

            // Minimum set of props we really need
            uno::Sequence< beans::Property > props( 1 );
            props[0] = beans::Property(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")),
                -1,
                getCppuType( static_cast< rtl::OUString* >( 0 ) ),
                beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::BOUND );

            // file
            seq[0].Type       = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FILE_TYPE ));
            seq[0].Attributes = ( ucb::ContentInfoAttribute::INSERT_WITH_INPUTSTREAM |
                                  ucb::ContentInfoAttribute::KIND_DOCUMENT );
            seq[0].Properties = props;

            // folder
            seq[1].Type       = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CMIS_FOLDER_TYPE ));
            seq[1].Attributes = ucb::ContentInfoAttribute::KIND_FOLDER;
            seq[1].Properties = props;

            return seq;
        }
        else
        {
            return uno::Sequence< ucb::ContentInfo >();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
