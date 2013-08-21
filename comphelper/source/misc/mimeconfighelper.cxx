/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>

#include <comphelper/fileformat.h>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/documentconstants.hxx>


using namespace ::com::sun::star;
using namespace comphelper;

//-----------------------------------------------------------------------
MimeConfigurationHelper::MimeConfigurationHelper( const uno::Reference< uno::XComponentContext >& rxContext )
: m_xContext( rxContext )
{
    if ( !m_xContext.is() )
        throw uno::RuntimeException();
}

//-----------------------------------------------------------------------
OUString MimeConfigurationHelper::GetStringClassIDRepresentation( const uno::Sequence< sal_Int8 >& aClassID )
{
    OUString aResult;

    if ( aClassID.getLength() == 16 )
    {
        for ( sal_Int32 nInd = 0; nInd < aClassID.getLength(); nInd++ )
        {
            if ( nInd == 4 || nInd == 6 || nInd == 8 || nInd == 10 )
                aResult += "-";

            sal_Int32 nDigit1 = (sal_Int32)( (sal_uInt8)aClassID[nInd] / 16 );
            sal_Int32 nDigit2 = (sal_uInt8)aClassID[nInd] % 16;
            aResult += OUString::number( nDigit1, 16 );
            aResult += OUString::number( nDigit2, 16 );
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
uno::Sequence< sal_Int8 > MimeConfigurationHelper::GetSequenceClassIDRepresentation( const OUString& aClassID )
{
    sal_Int32 nLength = aClassID.getLength();
    if ( nLength == 36 )
    {
        OString aCharClassID = OUStringToOString( aClassID, RTL_TEXTENCODING_ASCII_US );
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
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetConfigurationByPath( const OUString& aPath )
{
    osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< container::XNameAccess > xConfig;

    try
    {
        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = configuration::theDefaultProvider::get( m_xContext );

        uno::Sequence< uno::Any > aArgs( 1 );
        beans::PropertyValue aPathProp;
        aPathProp.Name = "nodepath";
        aPathProp.Value <<= aPath;
        aArgs[0] <<= aPathProp;

        xConfig = uno::Reference< container::XNameAccess >(
                            m_xConfigProvider->createInstanceWithArguments(
                                OUString( "com.sun.star.configuration.ConfigurationAccess" ),
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
                                         OUString( "/org.openoffice.Office.Embedding/Objects" ) );

    return m_xObjectConfig;
}

//-----------------------------------------------------------------------
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetVerbsConfiguration()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xVerbsConfig.is() )
        m_xVerbsConfig = GetConfigurationByPath(
                                        OUString( "/org.openoffice.Office.Embedding/Verbs" ));

    return m_xVerbsConfig;
}

//-----------------------------------------------------------------------
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetMediaTypeConfiguration()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xMediaTypeConfig.is() )
        m_xMediaTypeConfig = GetConfigurationByPath(
                    OUString( "/org.openoffice.Office.Embedding/MimeTypeClassIDRelations" ));

    return m_xMediaTypeConfig;
}

//-----------------------------------------------------------------------
uno::Reference< container::XNameAccess > MimeConfigurationHelper::GetFilterFactory()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_xFilterFactory.is() )
        m_xFilterFactory.set(
            m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.FilterFactory", m_xContext),
            uno::UNO_QUERY );

    return m_xFilterFactory;
}

//-------------------------------------------------------------------------
OUString MimeConfigurationHelper::GetDocServiceNameFromFilter( const OUString& aFilterName )
{
    OUString aDocServiceName;

    try
    {
        uno::Reference< container::XNameAccess > xFilterFactory(
            GetFilterFactory(),
            uno::UNO_SET_THROW );

        uno::Any aFilterAnyData = xFilterFactory->getByName( aFilterName );
        uno::Sequence< beans::PropertyValue > aFilterData;
        if ( aFilterAnyData >>= aFilterData )
        {
            for ( sal_Int32 nInd = 0; nInd < aFilterData.getLength(); nInd++ )
                if ( aFilterData[nInd].Name == "DocumentService" )
                    aFilterData[nInd].Value >>= aDocServiceName;
        }
    }
    catch( uno::Exception& )
    {}

    return aDocServiceName;
}

//-------------------------------------------------------------------------
OUString MimeConfigurationHelper::GetDocServiceNameFromMediaType( const OUString& aMediaType )
{
    uno::Reference< container::XContainerQuery > xTypeCFG(
            m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", m_xContext),
            uno::UNO_QUERY );

    if ( xTypeCFG.is() )
    {
        try
        {
            // make query for all types matching the properties
            uno::Sequence < beans::NamedValue > aSeq( 1 );
            aSeq[0].Name = "MediaType";
            aSeq[0].Value <<= aMediaType;

            uno::Reference < container::XEnumeration > xEnum = xTypeCFG->createSubSetEnumerationByProperties( aSeq );
            while ( xEnum->hasMoreElements() )
            {
                uno::Sequence< beans::PropertyValue > aType;
                if ( xEnum->nextElement() >>= aType )
                {
                    for ( sal_Int32 nInd = 0; nInd < aType.getLength(); nInd++ )
                    {
                        OUString aFilterName;
                        if ( aType[nInd].Name == "PreferredFilter"
                          && ( aType[nInd].Value >>= aFilterName ) && !aFilterName.isEmpty() )
                        {
                            OUString aDocumentName = GetDocServiceNameFromFilter( aFilterName );
                            if ( !aDocumentName.isEmpty() )
                                return aDocumentName;
                        }
                    }
                }
            }
        }
        catch( uno::Exception& )
        {}
    }

    return OUString();
}

//-------------------------------------------------------------------------
sal_Bool MimeConfigurationHelper::GetVerbByShortcut( const OUString& aVerbShortcut,
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
            if ( ( xVerbsProps->getByName("VerbID") >>= aTempDescr.VerbID )
              && ( xVerbsProps->getByName("VerbUIName") >>= aTempDescr.VerbName )
              && ( xVerbsProps->getByName("VerbFlags") >>= aTempDescr.VerbFlags )
              && ( xVerbsProps->getByName("VerbAttributes") >>= aTempDescr.VerbAttributes ) )
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
            uno::Sequence< OUString > aObjPropNames = xObjectProps->getElementNames();

            aResult.realloc( aObjPropNames.getLength() + 1 );
            aResult[0].Name = "ClassID";
            aResult[0].Value <<= aClassID;

            for ( sal_Int32 nInd = 0; nInd < aObjPropNames.getLength(); nInd++ )
            {
                aResult[nInd + 1].Name = aObjPropNames[nInd];

                if ( aObjPropNames[nInd] == "ObjectVerbs" )
                {
                    uno::Sequence< OUString > aVerbShortcuts;
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
OUString MimeConfigurationHelper::GetExplicitlyRegisteredObjClassID( const OUString& aMediaType )
{
    OUString aStringClassID;

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
                                                                const OUString& aStringClassID )
{
    uno::Sequence< beans::NamedValue > aObjProps;

    uno::Sequence< sal_Int8 > aClassID = GetSequenceClassIDRepresentation( aStringClassID );
    if ( ClassIDsEqual( aClassID, GetSequenceClassID( SO3_DUMMY_CLASSID ) ) )
    {
        aObjProps.realloc(2);
        aObjProps[0].Name = "ObjectFactory";
        aObjProps[0].Value <<= OUString( "com.sun.star.embed.OOoSpecialEmbeddedObjectFactory" );
        aObjProps[1].Name = "ClassID";
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
        aObjProps[0].Name = "ObjectFactory";
        aObjProps[0].Value <<= OUString( "com.sun.star.embed.OOoSpecialEmbeddedObjectFactory" );
        aObjProps[1].Name = "ClassID";
        aObjProps[1].Value <<= aClassID;
    }

    OUString aStringClassID = GetStringClassIDRepresentation( aClassID );
    if ( !aStringClassID.isEmpty() )
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
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByMediaType( const OUString& aMediaType )
{
    uno::Sequence< beans::NamedValue > aObject =
                                    GetObjectPropsByStringClassID( GetExplicitlyRegisteredObjClassID( aMediaType ) );
    if ( aObject.getLength() )
        return aObject;

    OUString aDocumentName = GetDocServiceNameFromMediaType( aMediaType );
    if ( !aDocumentName.isEmpty() )
        return GetObjectPropsByDocumentName( aDocumentName );

    return uno::Sequence< beans::NamedValue >();
}

//-----------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByFilter( const OUString& aFilterName )
{
    OUString aDocumentName = GetDocServiceNameFromFilter( aFilterName );
    if ( !aDocumentName.isEmpty() )
        return GetObjectPropsByDocumentName( aDocumentName );

    return uno::Sequence< beans::NamedValue >();
}

//-----------------------------------------------------------------------
uno::Sequence< beans::NamedValue > MimeConfigurationHelper::GetObjectPropsByDocumentName( const OUString& aDocName )
{
    if ( !aDocName.isEmpty() )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        if ( xObjConfig.is() )
        {
            try
            {
                uno::Sequence< OUString > aClassIDs = xObjConfig->getElementNames();
                for ( sal_Int32 nInd = 0; nInd < aClassIDs.getLength(); nInd++ )
                {
                    uno::Reference< container::XNameAccess > xObjectProps;
                    OUString aEntryDocName;

                    if ( ( xObjConfig->getByName( aClassIDs[nInd] ) >>= xObjectProps ) && xObjectProps.is()
                      && ( xObjectProps->getByName("ObjectDocumentServiceName") >>= aEntryDocName )
                      && aEntryDocName == aDocName )
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
OUString MimeConfigurationHelper::GetFactoryNameByClassID( const uno::Sequence< sal_Int8 >& aClassID )
{
    return GetFactoryNameByStringClassID( GetStringClassIDRepresentation( aClassID ) );
}

//-----------------------------------------------------------------------
OUString MimeConfigurationHelper::GetFactoryNameByStringClassID( const OUString& aStringClassID )
{
    OUString aResult;

    if ( !aStringClassID.isEmpty() )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        uno::Reference< container::XNameAccess > xObjectProps;
        try
        {
            if ( xObjConfig.is() && ( xObjConfig->getByName( aStringClassID.toAsciiUpperCase() ) >>= xObjectProps ) && xObjectProps.is() )
                xObjectProps->getByName("ObjectFactory") >>= aResult;
        }
        catch( uno::Exception& )
        {
            uno::Sequence< sal_Int8 > aClassID = GetSequenceClassIDRepresentation( aStringClassID );
            if ( ClassIDsEqual( aClassID, GetSequenceClassID( SO3_DUMMY_CLASSID ) ) )
                return OUString( "com.sun.star.embed.OOoSpecialEmbeddedObjectFactory" );
        }
    }

    return aResult;
}

//-----------------------------------------------------------------------
OUString MimeConfigurationHelper::GetFactoryNameByDocumentName( const OUString& aDocName )
{
    OUString aResult;

    if ( !aDocName.isEmpty() )
    {
        uno::Reference< container::XNameAccess > xObjConfig = GetObjConfiguration();
        if ( xObjConfig.is() )
        {
            try
            {
                uno::Sequence< OUString > aClassIDs = xObjConfig->getElementNames();
                for ( sal_Int32 nInd = 0; nInd < aClassIDs.getLength(); nInd++ )
                {
                    uno::Reference< container::XNameAccess > xObjectProps;
                    OUString aEntryDocName;

                    if ( ( xObjConfig->getByName( aClassIDs[nInd] ) >>= xObjectProps ) && xObjectProps.is()
                      && ( xObjectProps->getByName(
                                  OUString( "ObjectDocumentServiceName" ) ) >>= aEntryDocName )
                      && aEntryDocName == aDocName )
                    {
                        xObjectProps->getByName("ObjectFactory") >>= aResult;
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
OUString MimeConfigurationHelper::GetFactoryNameByMediaType( const OUString& aMediaType )
{
    OUString aResult = GetFactoryNameByStringClassID( GetExplicitlyRegisteredObjClassID( aMediaType ) );

    if ( aResult.isEmpty() )
    {
        OUString aDocumentName = GetDocServiceNameFromMediaType( aMediaType );
        if ( !aDocumentName.isEmpty() )
            aResult = GetFactoryNameByDocumentName( aDocumentName );
    }

    return aResult;
}

//-----------------------------------------------------------------------
OUString MimeConfigurationHelper::UpdateMediaDescriptorWithFilterName(
                                        uno::Sequence< beans::PropertyValue >& aMediaDescr,
                                        sal_Bool bIgnoreType )
{
    OUString aFilterName;

    for ( sal_Int32 nInd = 0; nInd < aMediaDescr.getLength(); nInd++ )
        if ( aMediaDescr[nInd].Name == "FilterName" )
            aMediaDescr[nInd].Value >>= aFilterName;

    if ( aFilterName.isEmpty() )
    {
        // filter name is not specified, so type detection should be done

        uno::Reference< document::XTypeDetection > xTypeDetection(
                m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", m_xContext),
                uno::UNO_QUERY );

        if ( !xTypeDetection.is() )
            throw uno::RuntimeException(); // TODO

        // typedetection can change the mode, add a stream and so on, thus a copy should be used
        uno::Sequence< beans::PropertyValue > aTempMD( aMediaDescr );

        // get TypeName
        OUString aTypeName = xTypeDetection->queryTypeByDescriptor( aTempMD, sal_True );

        // get FilterName
        for ( sal_Int32 nInd = 0; nInd < aTempMD.getLength(); nInd++ )
            if ( aTempMD[nInd].Name == "FilterName" )
                aTempMD[nInd].Value >>= aFilterName;

        if ( !aFilterName.isEmpty() )
        {
            sal_Int32 nOldLen = aMediaDescr.getLength();
            aMediaDescr.realloc( nOldLen + 1 );
            aMediaDescr[nOldLen].Name = "FilterName";
            aMediaDescr[ nOldLen ].Value <<= aFilterName;

        }
        else if ( !aTypeName.isEmpty() && !bIgnoreType )
        {
            uno::Reference< container::XNameAccess > xNameAccess( xTypeDetection, uno::UNO_QUERY );
            uno::Sequence< beans::PropertyValue > aTypes;

            if ( xNameAccess.is() && ( xNameAccess->getByName( aTypeName ) >>= aTypes ) )
            {
                for ( sal_Int32 nInd = 0; nInd < aTypes.getLength(); nInd++ )
                {
                    if ( aTypes[nInd].Name == "PreferredFilter" && ( aTypes[nInd].Value >>= aFilterName ) )
                    {
                        sal_Int32 nOldLen = aMediaDescr.getLength();
                        aMediaDescr.realloc( nOldLen + 1 );
                        aMediaDescr[nOldLen].Name = "FilterName";
                        aMediaDescr[ nOldLen ].Value = aTypes[nInd].Value;
                        break;
                    }
                }
            }
        }
    }

    return aFilterName;
}

OUString MimeConfigurationHelper::UpdateMediaDescriptorWithFilterName(
                        uno::Sequence< beans::PropertyValue >& aMediaDescr,
                        uno::Sequence< beans::NamedValue >& aObject )
{
    OUString aDocName;
    for ( sal_Int32 nInd = 0; nInd < aObject.getLength(); nInd++ )
        if ( aObject[nInd].Name == "ObjectDocumentServiceName" )
        {
            aObject[nInd].Value >>= aDocName;
            break;
        }

    OSL_ENSURE( !aDocName.isEmpty(), "The name must exist at this point!\n" );


    sal_Bool bNeedsAddition = sal_True;
    for ( sal_Int32 nMedInd = 0; nMedInd < aMediaDescr.getLength(); nMedInd++ )
        if ( aMediaDescr[nMedInd].Name == "DocumentService" )
        {
            aMediaDescr[nMedInd].Value <<= aDocName;
            bNeedsAddition = sal_False;
            break;
        }

    if ( bNeedsAddition )
    {
        sal_Int32 nOldLen = aMediaDescr.getLength();
        aMediaDescr.realloc( nOldLen + 1 );
        aMediaDescr[nOldLen].Name = "DocumentService";
        aMediaDescr[nOldLen].Value <<= aDocName;
    }

    return UpdateMediaDescriptorWithFilterName( aMediaDescr, sal_True );
}

#ifdef WNT

sal_Int32 MimeConfigurationHelper::GetFilterFlags( const OUString& aFilterName )
{
    sal_Int32 nFlags = 0;
    try
    {
        if ( !aFilterName.isEmpty() )
        {
            uno::Reference< container::XNameAccess > xFilterFactory(
                GetFilterFactory(),
                uno::UNO_SET_THROW );

            uno::Any aFilterAny = xFilterFactory->getByName( aFilterName );
            uno::Sequence< beans::PropertyValue > aData;
            if ( aFilterAny >>= aData )
            {
                SequenceAsHashMap aFilterHM( aData );
                nFlags = aFilterHM.getUnpackedValueOrDefault( "Flags", (sal_Int32)0 );
            }
        }
    } catch( uno::Exception& )
    {}

    return nFlags;
}

sal_Bool MimeConfigurationHelper::AddFilterNameCheckOwnFile(
                        uno::Sequence< beans::PropertyValue >& aMediaDescr )
{
    sal_Bool bResult = sal_False;

    OUString aFilterName = UpdateMediaDescriptorWithFilterName( aMediaDescr, sal_False );
    if ( !aFilterName.isEmpty() )
    {
        sal_Int32 nFlags = GetFilterFlags( aFilterName );
        // check the OWN flag
        bResult = ( nFlags & SFX_FILTER_OWN );
    }

    return bResult;
}
#endif

//-----------------------------------------------------------
OUString MimeConfigurationHelper::GetDefaultFilterFromServiceName( const OUString& aServiceName, sal_Int32 nVersion )
{
    OUString aResult;

    if ( !aServiceName.isEmpty() && nVersion )
        try
        {
            uno::Reference< container::XContainerQuery > xFilterQuery(
                GetFilterFactory(),
                uno::UNO_QUERY_THROW );

            uno::Sequence< beans::NamedValue > aSearchRequest( 2 );
            aSearchRequest[0].Name = "DocumentService";
            aSearchRequest[0].Value <<= aServiceName;
            aSearchRequest[1].Name = "FileFormatVersion";
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
                        sal_Int32 nFlags = aPropsHM.getUnpackedValueOrDefault( "Flags", (sal_Int32)0 );

                        // that should be import, export, own filter and not a template filter ( TemplatePath flag )
                        sal_Int32 nRequired = ( SFX_FILTER_OWN | SFX_FILTER_EXPORT | SFX_FILTER_IMPORT );
                        if ( ( ( nFlags & nRequired ) == nRequired ) && !( nFlags & SFX_FILTER_TEMPLATEPATH ) )
                        {
                            // if there are more than one filter the preffered one should be used
                            // if there is no preffered filter the first one will be used
                            if ( aResult.isEmpty() || ( nFlags & SFX_FILTER_PREFERED ) )
                                aResult = aPropsHM.getUnpackedValueOrDefault( "Name", OUString() );
                            if ( nFlags & SFX_FILTER_PREFERED )
                                break; // the preferred filter was found
                        }
                    }
                }
        }
        catch( uno::Exception& )
        {}

    return aResult;
}

//-------------------------------------------------------------------------
OUString MimeConfigurationHelper::GetExportFilterFromImportFilter( const OUString& aImportFilterName )
{
    OUString aExportFilterName;

    try
    {
        if ( !aImportFilterName.isEmpty() )
        {
            uno::Reference< container::XNameAccess > xFilterFactory(
                GetFilterFactory(),
                uno::UNO_SET_THROW );

            uno::Any aImpFilterAny = xFilterFactory->getByName( aImportFilterName );
            uno::Sequence< beans::PropertyValue > aImpData;
            if ( aImpFilterAny >>= aImpData )
            {
                SequenceAsHashMap aImpFilterHM( aImpData );
                sal_Int32 nFlags = aImpFilterHM.getUnpackedValueOrDefault( "Flags", (sal_Int32)0 );

                if ( !( nFlags & SFX_FILTER_IMPORT ) )
                {
                    OSL_FAIL( "This is no import filter!" );
                    throw uno::Exception();
                }

                if ( nFlags & SFX_FILTER_EXPORT )
                {
                    aExportFilterName = aImportFilterName;
                }
                else
                {
                    OUString aDocumentServiceName = aImpFilterHM.getUnpackedValueOrDefault( "DocumentService", OUString() );
                    OUString aTypeName = aImpFilterHM.getUnpackedValueOrDefault( "Type", OUString() );

                    OSL_ENSURE( !aDocumentServiceName.isEmpty() && !aTypeName.isEmpty(), "Incomplete filter data!" );
                    if ( !(aDocumentServiceName.isEmpty() || aTypeName.isEmpty()) )
                    {
                        uno::Sequence< beans::NamedValue > aSearchRequest( 2 );
                        aSearchRequest[0].Name = "Type";
                        aSearchRequest[0].Value <<= aTypeName;
                        aSearchRequest[1].Name = "DocumentService";
                        aSearchRequest[1].Value <<= aDocumentServiceName;

                        uno::Sequence< beans::PropertyValue > aExportFilterProps = SearchForFilter(
                            uno::Reference< container::XContainerQuery >( xFilterFactory, uno::UNO_QUERY_THROW ),
                            aSearchRequest,
                            SFX_FILTER_EXPORT,
                            SFX_FILTER_INTERNAL );

                        if ( aExportFilterProps.getLength() )
                        {
                            SequenceAsHashMap aExpPropsHM( aExportFilterProps );
                            aExportFilterName = aExpPropsHM.getUnpackedValueOrDefault( "Name", OUString() );
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {}

    return aExportFilterName;
}

//-------------------------------------------------------------------------
// static
uno::Sequence< beans::PropertyValue > MimeConfigurationHelper::SearchForFilter(
                                                        const uno::Reference< container::XContainerQuery >& xFilterQuery,
                                                        const uno::Sequence< beans::NamedValue >& aSearchRequest,
                                                        sal_Int32 nMustFlags,
                                                        sal_Int32 nDontFlags )
{
    uno::Sequence< beans::PropertyValue > aFilterProps;
    uno::Reference< container::XEnumeration > xFilterEnum =
                                            xFilterQuery->createSubSetEnumerationByProperties( aSearchRequest );

    // the first default filter will be taken,
    // if there is no filter with flag default the first acceptable filter will be taken
    if ( xFilterEnum.is() )
    {
        while ( xFilterEnum->hasMoreElements() )
        {
            uno::Sequence< beans::PropertyValue > aProps;
            if ( xFilterEnum->nextElement() >>= aProps )
            {
                SequenceAsHashMap aPropsHM( aProps );
                sal_Int32 nFlags = aPropsHM.getUnpackedValueOrDefault("Flags",
                                                                        (sal_Int32)0 );
                if ( ( ( nFlags & nMustFlags ) == nMustFlags ) && !( nFlags & nDontFlags ) )
                {
                    if ( ( nFlags & SFX_FILTER_DEFAULT ) == SFX_FILTER_DEFAULT )
                    {
                        aFilterProps = aProps;
                        break;
                    }
                    else if ( !aFilterProps.getLength() )
                        aFilterProps = aProps;
                }
            }
        }
    }

    return aFilterProps;
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

//-------------------------------------------------------------------------
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
