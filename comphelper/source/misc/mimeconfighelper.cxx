/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mimeconfighelper.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>

#include <comphelper/fileformat.h>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/sequenceashashmap.hxx>


using namespace ::com::sun::star;
using namespace comphelper;

//-----------------------------------------------------------------------
MimeConfigurationHelper::MimeConfigurationHelper( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_xFactory( xFactory )
{
    if ( !m_xFactory.is() )
        throw uno::RuntimeException();
}

//-----------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetStringClassIDRepresentation( const uno::Sequence< sal_Int8 >& aClassID )
{
    ::rtl::OUString aResult;

    if ( aClassID.getLength() == 16 )
    {
        for ( sal_Int32 nInd = 0; nInd < aClassID.getLength(); nInd++ )
        {
            if ( nInd == 4 || nInd == 6 || nInd == 8 || nInd == 10 )
                aResult += ::rtl::OUString::createFromAscii( "-" );

            sal_Int32 nDigit1 = (sal_Int32)( (sal_uInt8)aClassID[nInd] / 16 );
            sal_Int32 nDigit2 = (sal_uInt8)aClassID[nInd] % 16;
            aResult += ::rtl::OUString::valueOf( nDigit1, 16 );
            aResult += ::rtl::OUString::valueOf( nDigit2, 16 );
        }
    }

    return aResult;
}

//-----------------------------------------------------------------------
sal_uInt8 GetDigit_Impl( sal_Char aChar )
{
    if ( aChar >= '0' && aChar <= '9' )
        return aChar - '0';
    else if ( aChar >= 'a' && aChar <= 'f' )
        return aChar - 'a' + 10;
    else if ( aChar >= 'A' && aChar <= 'F' )
        return aChar - 'A' + 10;
    else
        return 16;
}

//-----------------------------------------------------------------------
uno::Sequence< sal_Int8 > MimeConfigurationHelper::GetSequenceClassIDRepresentation( const ::rtl::OUString& aClassID )
{
    sal_Int32 nLength = aClassID.getLength();
    if ( nLength == 36 )
    {
        ::rtl::OString aCharClassID = ::rtl::OUStringToOString( aClassID, RTL_TEXTENCODING_ASCII_US );
        const sal_Char* pString = aCharClassID.getStr();
        if ( pString )
        {
            uno::Sequence< sal_Int8 > aResult( 16 );

            sal_Int32 nStrPointer = 0;
            sal_Int32 nSeqInd = 0;
            while( nSeqInd < 16 && nStrPointer + 1 < nLength )
            {
                sal_uInt8 nDigit1 = GetDigit_Impl( pString[nStrPointer++] );
                sal_uInt8 nDigit2 = GetDigit_Impl( pString[nStrPointer++] );

                if ( nDigit1 > 15 || nDigit2 > 15 )
                    break;

                aResult[nSeqInd++] = (sal_Int8)( nDigit1 * 16 + nDigit2 );

                if ( nStrPointer < nLength && pString[nStrPointer] == '-' )
                    nStrPointer++;
            }

            if ( nSeqInd == 16 && nStrPointer == nLength )
                return aResult;
        }
    }

    return uno::Sequence< sal_Int8 >();
}

//-----------------------------------------------------------------------
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetConfigurationByPath( const ::rtl::OUString& aPath )
{
    osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< container::XNameAccess > xConfig;

    try
    {
        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = uno::Reference< lang::XMultiServiceFactory >(
                m_xFactory->createInstance(
                    ::rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationProvider" ) ),
                uno::UNO_QUERY_THROW );

        uno::Sequence< uno::Any > aArgs( 1 );
        beans::PropertyValue aPathProp;
        aPathProp.Name = ::rtl::OUString::createFromAscii( "nodepath" );
        aPathProp.Value <<= aPath;
        aArgs[0] <<= aPathProp;

        xConfig = uno::Reference< container::XNameAccess >(
                            m_xConfigProvider->createInstanceWithArguments(
                                ::rtl::OUString::createFromAscii( "com.sun.star.configuration.ConfigurationAccess" ),
                                aArgs ),
                            uno::UNO_QUERY );
    }
    catch( uno::Exception& )
    {}

    return xConfig;
}

//-----------------------------------------------------------------------
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetObjConfiguration()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xObjectConfig.is() )
        m_xObjectConfig = GetConfigurationByPath(
                                        ::rtl::OUString::createFromAscii( "/org.openoffice.Office.Embedding/Objects" ) );

    return m_xObjectConfig;
}

//-----------------------------------------------------------------------
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetVerbsConfiguration()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xVerbsConfig.is() )
        m_xVerbsConfig = GetConfigurationByPath(
                                        ::rtl::OUString::createFromAscii( "/org.openoffice.Office.Embedding/Verbs" ) );

    return m_xVerbsConfig;
}

//-----------------------------------------------------------------------
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetMediaTypeConfiguration()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xMediaTypeConfig.is() )
        m_xMediaTypeConfig = GetConfigurationByPath(
                    ::rtl::OUString::createFromAscii( "/org.openoffice.Office.Embedding/MimeTypeClassIDRelations" ) );

    return m_xMediaTypeConfig;
}
//-------------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetDocServiceNameFromFilter( const ::rtl::OUString& aFilterName )
{
    ::rtl::OUString aDocServiceName;

    try
    {
        uno::Reference< container::XNameAccess > xFilterFactory(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
            uno::UNO_QUERY_THROW );

        uno::Any aFilterAnyData = xFilterFactory->getByName( aFilterName );
        uno::Sequence< beans::PropertyValue > aFilterData;
        if ( aFilterAnyData >>= aFilterData )
        {
            for ( sal_Int32 nInd = 0; nInd < aFilterData.getLength(); nInd++ )
                if ( aFilterData[nInd].Name.equalsAscii( "DocumentService" ) )
                    aFilterData[nInd].Value >>= aDocServiceName;
        }
    }
    catch( uno::Exception& )
    {}

    return aDocServiceName;
}

//-------------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetDocServiceNameFromMediaType( const ::rtl::OUString& aMediaType )
{
    uno::Reference< container::XContainerQuery > xTypeCFG(
            m_xFactory->createInstance(
                ::rtl::OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ),
            uno::UNO_QUERY );

    if ( xTypeCFG.is() )
    {
        try
        {
            // make query for all types matching the properties
            uno::Sequence < beans::NamedValue > aSeq( 1 );
            aSeq[0].Name = ::rtl::OUString::createFromAscii( "MediaType" );
            aSeq[0].Value <<= aMediaType;

            uno::Reference < container::XEnumeration > xEnum = xTypeCFG->createSubSetEnumerationByProperties( aSeq );
            while ( xEnum->hasMoreElements() )
            {
                uno::Sequence< beans::PropertyValue > aType;
                if ( xEnum->nextElement() >>= aType )
                {
                    for ( sal_Int32 nInd = 0; nInd < aType.getLength(); nInd++ )
                    {
                        ::rtl::OUString aFilterName;
                        if ( aType[nInd].Name.equalsAscii( "PreferredFilter" )
                          && ( aType[nInd].Value >>= aFilterName ) && aFilterName.getLength() )
                        {
                            ::rtl::OUString aDocumentName = GetDocServiceNameFromFilter( aFilterName );
                            if ( aDocumentName.getLength() )
                                return aDocumentName;
                        }
                    }
                }
            }
        }
        catch( uno::Exception& )
        {}
    }

    return ::rtl::OUString();
}

//-------------------------------------------------------------------------
sal_Bool MimeConfigurationHelper::GetVerbByShortcut( const ::rtl::OUString& aVerbShortcut,
                                                embed::VerbDescriptor& aDescriptor )
{
    sal_Bool bResult = sal_False;

    uno::Reference< container::XNameAccess > xVerbsConfig = GetVerbsConfiguration();
    uno::Reference< container::XNameAccess > xVerbsProps;
    try
    {
        if ( xVerbsConfig.is() && ( xVerbsConfig->getByName( aVerbShortcut ) >>= xVerbsProps ) && xVerbsProps.is() )
        {
            embed::VerbDescriptor aTempDescr;
            if ( ( xVerbsProps->getByName( ::rtl::OUString::createFromAscii( "VerbID" ) ) >>= aTempDescr.VerbID )
              && ( xVerbsProps->getByName( ::rtl::OUString::createFromAscii( "VerbUIName" ) ) >>= aTempDescr.VerbName )
              && ( xVerbsProps->getByName( ::rtl::OUString::createFromAscii( "VerbFlags" ) ) >>= aTempDescr.VerbFlags )
              && ( xVerbsProps->getByName( ::rtl::OUString::createFromAscii( "VerbAttributes" ) ) >>= aTempDescr.VerbAttributes ) )
            {
                  aDescriptor = aTempDescr;
                bResult = sal_True;
            }
        }
    }
    catch( uno::Exception& )
    {
    }

    return bResult;
}

//-------------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjPropsFromConfigEntry(
                                            const uno::Sequence< sal_Int8 >& aClassID,
                                            const uno::Reference< container::XNameAccess >& xObjectProps )
{
    uno::Sequence< beans::NamedValue > aResult;

    if ( aClassID.getLength() == 16 )
    {
        try
        {
            uno::Sequence< ::rtl::OUString > aObjPropNames = xObjectProps->getElementNames();

            aResult.realloc( aObjPropNames.getLength() + 1 );
            aResult[0].Name = ::rtl::OUString::createFromAscii( "ClassID" );
            aResult[0].Value <<= aClassID;

            for ( sal_Int32 nInd = 0; nInd < aObjPropNames.getLength(); nInd++ )
            {
                aResult[nInd + 1].Name = aObjPropNames[nInd];

                if ( aObjPropNames[nInd].equalsAscii( "ObjectVerbs" ) )
                {
                    uno::Sequence< ::rtl::OUString > aVerbShortcuts;
                    if ( xObjectProps->getByName( aObjPropNames[nInd] ) >>= aVerbShortcuts )
                    {
                        uno::Sequence< embed::VerbDescriptor > aVerbDescriptors( aVerbShortcuts.getLength() );
                        for ( sal_Int32 nVerbI = 0; nVerbI < aVerbShortcuts.getLength(); nVerbI++ )
                            if ( !GetVerbByShortcut( aVerbShortcuts[nVerbI], aVerbDescriptors[nVerbI] ) )
                                throw uno::RuntimeException();

                        aResult[nInd+1].Value <<= aVerbDescriptors;
                    }
                    else
                        throw uno::RuntimeException();
                }
                else
                    aResult[nInd+1].Value = xObjectProps->getByName( aObjPropNames[nInd] );
            }
        }
        catch( uno::Exception& )
        {
            aResult.realloc( 0 );
        }
    }

    return aResult;
}

//-----------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetExplicitlyRegisteredObjClassID( const ::rtl::OUString& aMediaType )
{
    ::rtl::OUString aStringClassID;

    uno::Reference< container::XNameAccess > xMediaTypeConfig = GetMediaTypeConfiguration();
    try
    {
        if ( xMediaTypeConfig.is() )
            xMediaTypeConfig->getByName( aMediaType ) >>= aStringClassID;
    }
    catch( uno::Exception& )
    {
    }

    return aStringClassID;

}

//-----------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByStringClassID(
                                                                const ::rtl::OUString& aStringClassID )
{
    uno::Sequence< beans::NamedValue > aObjProps;

    uno::Sequence< sal_Int8 > aClassID = GetSequenceClassIDRepresentation( aStringClassID );
    if ( ClassIDsEqual( aClassID, GetSequenceClassID( SO3_DUMMY_CLASSID ) ) )
    {
        aObjProps.realloc(2);
        aObjProps[0].Name = ::rtl::OUString::createFromAscii("ObjectFactory");
        aObjProps[0].Value <<= ::rtl::OUString::createFromAscii("com.sun.star.embed.OOoSpecialEmbeddedObjectFactory");
        aObjProps[1].Name = ::rtl::OUString::createFromAscii("ClassID");
        aObjProps[1].Value <<= aClassID;
        return aObjProps;
    }

    if ( aClassID.getLength() == 16 )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        uno::Reference< container::XNameAccess > xObjectProps;
        try
        {
            // TODO/LATER: allow to provide ClassID string in any format, only digits are counted
            if ( xObjConfig.is() && ( xObjConfig->getByName( aStringClassID.toAsciiUpperCase() ) >>= xObjectProps ) && xObjectProps.is() )
                aObjProps = GetObjPropsFromConfigEntry( aClassID, xObjectProps );
        }
        catch( uno::Exception& )
        {
        }
    }

    return aObjProps;
}

//-----------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByClassID(
                                                                const uno::Sequence< sal_Int8 >& aClassID )
{
    uno::Sequence< beans::NamedValue > aObjProps;
    if ( ClassIDsEqual( aClassID, GetSequenceClassID( SO3_DUMMY_CLASSID ) ) )
    {
        aObjProps.realloc(2);
        aObjProps[0].Name = ::rtl::OUString::createFromAscii("ObjectFactory");
        aObjProps[0].Value <<= ::rtl::OUString::createFromAscii("com.sun.star.embed.OOoSpecialEmbeddedObjectFactory");
        aObjProps[1].Name = ::rtl::OUString::createFromAscii("ClassID");
        aObjProps[1].Value <<= aClassID;
    }

    ::rtl::OUString aStringClassID = GetStringClassIDRepresentation( aClassID );
    if ( aStringClassID.getLength() )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        uno::Reference< container::XNameAccess > xObjectProps;
        try
        {
            if ( xObjConfig.is() && ( xObjConfig->getByName( aStringClassID.toAsciiUpperCase() ) >>= xObjectProps ) && xObjectProps.is() )
                aObjProps = GetObjPropsFromConfigEntry( aClassID, xObjectProps );
        }
        catch( uno::Exception& )
        {
        }
    }

    return aObjProps;
}

//-----------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByMediaType( const ::rtl::OUString& aMediaType )
{
    uno::Sequence< beans::NamedValue > aObject =
                                    GetObjectPropsByStringClassID( GetExplicitlyRegisteredObjClassID( aMediaType ) );
    if ( aObject.getLength() )
        return aObject;

    ::rtl::OUString aDocumentName = GetDocServiceNameFromMediaType( aMediaType );
    if ( aDocumentName.getLength() )
        return GetObjectPropsByDocumentName( aDocumentName );

    return uno::Sequence< beans::NamedValue >();
}

//-----------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByFilter( const ::rtl::OUString& aFilterName )
{
    ::rtl::OUString aDocumentName = GetDocServiceNameFromFilter( aFilterName );
    if ( aDocumentName.getLength() )
        return GetObjectPropsByDocumentName( aDocumentName );

    return uno::Sequence< beans::NamedValue >();
}

//-----------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByDocumentName( const ::rtl::OUString& aDocName )
{
    if ( aDocName.getLength() )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        if ( xObjConfig.is() )
        {
            try
            {
                uno::Sequence< ::rtl::OUString > aClassIDs = xObjConfig->getElementNames();
                for ( sal_Int32 nInd = 0; nInd < aClassIDs.getLength(); nInd++ )
                {
                    uno::Reference< container::XNameAccess > xObjectProps;
                    ::rtl::OUString aEntryDocName;

                    if ( ( xObjConfig->getByName( aClassIDs[nInd] ) >>= xObjectProps ) && xObjectProps.is()
                      && ( xObjectProps->getByName(
                                  ::rtl::OUString::createFromAscii( "ObjectDocumentServiceName" ) ) >>= aEntryDocName )
                      && aEntryDocName.equals( aDocName ) )
                    {
                        return GetObjPropsFromConfigEntry( GetSequenceClassIDRepresentation( aClassIDs[nInd] ),
                                                            xObjectProps );
                    }
                }
            }
            catch( uno::Exception& )
            {}
        }
    }

    return uno::Sequence< beans::NamedValue >();
}

//-----------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetFactoryNameByClassID( const uno::Sequence< sal_Int8 >& aClassID )
{
    return GetFactoryNameByStringClassID( GetStringClassIDRepresentation( aClassID ) );
}

//-----------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetFactoryNameByStringClassID( const ::rtl::OUString& aStringClassID )
{
    ::rtl::OUString aResult;

    if ( aStringClassID.getLength() )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        uno::Reference< container::XNameAccess > xObjectProps;
        try
        {
            if ( xObjConfig.is() && ( xObjConfig->getByName( aStringClassID.toAsciiUpperCase() ) >>= xObjectProps ) && xObjectProps.is() )
                xObjectProps->getByName( ::rtl::OUString::createFromAscii( "ObjectFactory" ) ) >>= aResult;
        }
        catch( uno::Exception& )
        {
            uno::Sequence< sal_Int8 > aClassID = GetSequenceClassIDRepresentation( aStringClassID );
            if ( ClassIDsEqual( aClassID, GetSequenceClassID( SO3_DUMMY_CLASSID ) ) )
                return ::rtl::OUString::createFromAscii("com.sun.star.embed.OOoSpecialEmbeddedObjectFactory");
        }
    }

    return aResult;
}

//-----------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetFactoryNameByDocumentName( const ::rtl::OUString& aDocName )
{
    ::rtl::OUString aResult;

    if ( aDocName.getLength() )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        if ( xObjConfig.is() )
        {
            try
            {
                uno::Sequence< ::rtl::OUString > aClassIDs = xObjConfig->getElementNames();
                for ( sal_Int32 nInd = 0; nInd < aClassIDs.getLength(); nInd++ )
                {
                    uno::Reference< container::XNameAccess > xObjectProps;
                    ::rtl::OUString aEntryDocName;

                    if ( ( xObjConfig->getByName( aClassIDs[nInd] ) >>= xObjectProps ) && xObjectProps.is()
                      && ( xObjectProps->getByName(
                                  ::rtl::OUString::createFromAscii( "ObjectDocumentServiceName" ) ) >>= aEntryDocName )
                      && aEntryDocName.equals( aDocName ) )
                    {
                        xObjectProps->getByName( ::rtl::OUString::createFromAscii( "ObjectFactory" ) ) >>= aResult;
                        break;
                    }
                }
            }
            catch( uno::Exception& )
            {}
        }
    }

    return aResult;
}


//-----------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetFactoryNameByMediaType( const ::rtl::OUString& aMediaType )
{
    ::rtl::OUString aResult = GetFactoryNameByStringClassID( GetExplicitlyRegisteredObjClassID( aMediaType ) );

    if ( !aResult.getLength() )
    {
        ::rtl::OUString aDocumentName = GetDocServiceNameFromMediaType( aMediaType );
        if ( aDocumentName.getLength() )
            aResult = GetFactoryNameByDocumentName( aDocumentName );
    }

    return aResult;
}

//-----------------------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::UpdateMediaDescriptorWithFilterName(
                                        uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                        sal_Bool bIgnoreType )
{
    ::rtl::OUString aFilterName;

    for ( sal_Int32 nInd = 0; nInd < aMediaDescr.getLength(); nInd++ )
        if ( aMediaDescr[nInd].Name.equalsAscii( "FilterName" ) )
            aMediaDescr[nInd].Value >>= aFilterName;

    if ( !aFilterName.getLength() )
    {
        // filter name is not specified, so type detection should be done

        uno::Reference< document::XTypeDetection > xTypeDetection(
                m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.TypeDetection" ) ),
                uno::UNO_QUERY );

        if ( !xTypeDetection.is() )
            throw uno::RuntimeException(); // TODO

        // typedetection can change the mode, add a stream and so on, thus a copy should be used
        uno::Sequence< beans::PropertyValue > aTempMD( aMediaDescr );

        // get TypeName
        ::rtl::OUString aTypeName = xTypeDetection->queryTypeByDescriptor( aTempMD, sal_True );

        // get FilterName
        for ( sal_Int32 nInd = 0; nInd < aTempMD.getLength(); nInd++ )
            if ( aTempMD[nInd].Name.equalsAscii( "FilterName" ) )
                aTempMD[nInd].Value >>= aFilterName;

        if ( aFilterName.getLength() )
        {
            sal_Int32 nOldLen = aMediaDescr.getLength();
            aMediaDescr.realloc( nOldLen + 1 );
            aMediaDescr[nOldLen].Name = ::rtl::OUString::createFromAscii( "FilterName" );
            aMediaDescr[ nOldLen ].Value <<= aFilterName;

        }
        else if ( aTypeName.getLength() && !bIgnoreType )
        {
            uno::Reference< container::XNameAccess > xNameAccess( xTypeDetection, uno::UNO_QUERY );
            uno::Sequence< beans::PropertyValue > aTypes;

            if ( xNameAccess.is() && ( xNameAccess->getByName( aTypeName ) >>= aTypes ) )
            {
                for ( sal_Int32 nInd = 0; nInd < aTypes.getLength(); nInd++ )
                {
                    if ( aTypes[nInd].Name.equalsAscii( "PreferredFilter" ) && ( aTypes[nInd].Value >>= aFilterName ) )
                    {
                        sal_Int32 nOldLen = aMediaDescr.getLength();
                        aMediaDescr.realloc( nOldLen + 1 );
                        aMediaDescr[nOldLen].Name = ::rtl::OUString::createFromAscii( "FilterName" );
                        aMediaDescr[ nOldLen ].Value = aTypes[nInd].Value;
                        break;
                    }
                }
            }
        }
    }

    return aFilterName;
}

::rtl::OUString MimeConfigurationHelper::UpdateMediaDescriptorWithFilterName(
                        uno::Sequence< beans::PropertyValue >& aMediaDescr,
                        uno::Sequence< beans::NamedValue >& aObject )
{
    ::rtl::OUString aDocName;
    for ( sal_Int32 nInd = 0; nInd < aObject.getLength(); nInd++ )
        if ( aObject[nInd].Name.equalsAscii( "ObjectDocumentServiceName" ) )
        {
            aObject[nInd].Value >>= aDocName;
            break;
        }

    OSL_ENSURE( aDocName.getLength(), "The name must exist at this point!\n" );


    sal_Bool bNeedsAddition = sal_True;
    for ( sal_Int32 nMedInd = 0; nMedInd < aMediaDescr.getLength(); nMedInd++ )
        if ( aMediaDescr[nMedInd].Name.equalsAscii( "DocumentService" ) )
        {
            aMediaDescr[nMedInd].Value <<= aDocName;
            bNeedsAddition = sal_False;
            break;
        }

    if ( bNeedsAddition )
    {
        sal_Int32 nOldLen = aMediaDescr.getLength();
        aMediaDescr.realloc( nOldLen + 1 );
        aMediaDescr[nOldLen].Name = ::rtl::OUString::createFromAscii( "DocumentService" );
        aMediaDescr[nOldLen].Value <<= aDocName;
    }

    return UpdateMediaDescriptorWithFilterName( aMediaDescr, sal_True );
}

sal_Bool MimeConfigurationHelper::AddFilterNameCheckOwnFile(
                        uno::Sequence< beans::PropertyValue >& aMediaDescr )
{
    ::rtl::OUString aFilterName = UpdateMediaDescriptorWithFilterName( aMediaDescr, sal_False );
    if ( aFilterName.getLength() )
    {
        try
        {
            uno::Reference< container::XNameAccess > xFilterFactory(
                m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
                uno::UNO_QUERY_THROW );

            uno::Any aFilterAnyData = xFilterFactory->getByName( aFilterName );
            uno::Sequence< beans::PropertyValue > aFilterData;
            if ( aFilterAnyData >>= aFilterData )
            {
                for ( sal_Int32 nInd = 0; nInd < aFilterData.getLength(); nInd++ )
                    if ( aFilterData[nInd].Name.equalsAscii( "Flags" ) )
                    {
                        uno::Any aVal = aFilterData[nInd].Value;
                        sal_Int32 nFlags = 0;
                        // check the OWN flag
                        if ( ( aFilterData[nInd].Value >>= nFlags ) && ( nFlags & 0x20 ) )
                            return sal_True;
                        break;
                    }
            }
        }
        catch( uno::Exception& )
        {}
    }

    return sal_False;
}

//-----------------------------------------------------------
::rtl::OUString MimeConfigurationHelper::GetDefaultFilterFromServiceName( const ::rtl::OUString& aServiceName, sal_Int32 nVersion )
{
    rtl::OUString aResult;

    if ( aServiceName.getLength() && nVersion )
        try
        {
            uno::Reference< container::XContainerQuery > xFilterQuery(
                m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.document.FilterFactory" ) ),
                uno::UNO_QUERY_THROW );

            uno::Sequence< beans::NamedValue > aSearchRequest( 2 );
            aSearchRequest[0].Name = ::rtl::OUString::createFromAscii( "DocumentService" );
            aSearchRequest[0].Value <<= aServiceName;
            aSearchRequest[1].Name = ::rtl::OUString::createFromAscii( "FileFormatVersion" );
            aSearchRequest[1].Value <<= nVersion;

            uno::Sequence< beans::PropertyValue > aFilterProps;
            uno::Reference< container::XEnumeration > xFilterEnum =
                                            xFilterQuery->createSubSetEnumerationByProperties( aSearchRequest );

            // use the first filter that is found
            if ( xFilterEnum.is() )
                while ( xFilterEnum->hasMoreElements() )
                {
                    uno::Sequence< beans::PropertyValue > aProps;
                    if ( xFilterEnum->nextElement() >>= aProps )
                    {
                        SequenceAsHashMap aPropsHM( aProps );
                        sal_Int32 nFlags = aPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "Flags" ),
                                                                                (sal_Int32)0 );

                        // that should be import, export, own filter and not a template filter ( TemplatePath flag )
                        if ( ( ( nFlags & 0x23L ) == 0x23L ) && !( nFlags & 0x10 ) )
                        {
                            // if there are more than one filter the preffered one should be used
                            // if there is no preffered filter the first one will be used
                            if ( !aResult.getLength() || ( nFlags & 0x10000000L ) )
                                aResult = aPropsHM.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii( "Name" ),
                                                                                ::rtl::OUString() );
                            if ( nFlags & 0x10000000L )
                                break; // the preffered filter was found
                        }
                    }
                }
        }
        catch( uno::Exception& )
        {}

    return aResult;
}
//-------------------------------------------------------------------------
sal_Bool MimeConfigurationHelper::ClassIDsEqual( const uno::Sequence< sal_Int8 >& aClassID1, const uno::Sequence< sal_Int8 >& aClassID2 )
{
    if ( aClassID1.getLength() != aClassID2.getLength() )
        return sal_False;

    for ( sal_Int32 nInd = 0; nInd < aClassID1.getLength(); nInd++ )
        if ( aClassID1[nInd] != aClassID2[nInd] )
            return sal_False;

    return sal_True;
}
//----------------------------------------------
uno::Sequence< sal_Int8 > MimeConfigurationHelper::GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
{
    uno::Sequence< sal_Int8 > aResult( 16 );
    aResult[0] = (sal_Int8)( n1 >> 24 );
    aResult[1] = (sal_Int8)( ( n1 << 8 ) >> 24 );
    aResult[2] = (sal_Int8)( ( n1 << 16 ) >> 24 );
    aResult[3] = (sal_Int8)( ( n1 << 24 ) >> 24 );
    aResult[4] = (sal_Int8)( n2 >> 8 );
    aResult[5] = (sal_Int8)( ( n2 << 8 ) >> 8 );
    aResult[6] = (sal_Int8)( n3 >> 8 );
    aResult[7] = (sal_Int8)( ( n3 << 8 ) >> 8 );
    aResult[8] = b8;
    aResult[9] = b9;
    aResult[10] = b10;
    aResult[11] = b11;
    aResult[12] = b12;
    aResult[13] = b13;
    aResult[14] = b14;
    aResult[15] = b15;

    return aResult;
}
uno::Sequence<sal_Int8> MimeConfigurationHelper::GetSequenceClassIDFromObjectName(const ::rtl::OUString& _sObjectName)
{
    uno::Sequence<sal_Int8> aClassId;
    uno::Reference< container::XNameAccess > xObjectNames = GetConfigurationByPath(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Embedding/ObjectNames")));
    uno::Reference< container::XNameAccess > xProps;
    if ( xObjectNames.is() && (xObjectNames->getByName(_sObjectName) >>= xProps) && xProps.is() )
    {
         ::rtl::OUString sValue;
         xProps->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ClassID"))) >>= sValue;
         aClassId = GetSequenceClassIDRepresentation(sValue);
    }
    return aClassId;
}

