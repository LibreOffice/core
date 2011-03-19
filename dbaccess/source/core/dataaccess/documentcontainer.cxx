/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_dbaccess.hxx"

#include "documentcontainer.hxx"
#include "dbastrings.hrc"
#include "documentdefinition.hxx"
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <tools/debug.hxx>
#include <connectivity/dbtools.hxx>
#include "myucp_resultset.hxx"
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include "datasource.hxx"
#include <comphelper/classids.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/string.hxx>
#include <connectivity/sqlerror.hxx>
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <comphelper/namedvaluecollection.hxx>

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <comphelper/namedvaluecollection.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::io;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

namespace dbaccess
{

//==========================================================================
//= LocalNameApproval
//==========================================================================
class LocalNameApproval : public IContainerApprove
{
    ::connectivity::SQLError    m_aErrors;

public:
    LocalNameApproval( const Reference< XMultiServiceFactory >& _rxFactory )
        :m_aErrors( ::comphelper::ComponentContext( _rxFactory ) )
    {
    }
    virtual ~LocalNameApproval()
    {
    }

    void SAL_CALL   approveElement( const ::rtl::OUString& _rName, const Reference< XInterface >& _rxElement );
};

void SAL_CALL LocalNameApproval::approveElement( const ::rtl::OUString& _rName, const Reference< XInterface >& /*_rxElement*/ )
{
    if ( _rName.indexOf( '/' ) != -1 )
        throw IllegalArgumentException(
            m_aErrors.getErrorMessage( ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES ),
            NULL,
            0
        );
}

//==========================================================================
//= ODocumentContainer
//==========================================================================
DBG_NAME(ODocumentContainer)

ODocumentContainer::ODocumentContainer(const Reference< XMultiServiceFactory >& _xORB
                                    ,const Reference< XInterface >& _xParentContainer
                                    ,const TContentPtr& _pImpl
                                    , sal_Bool _bFormsContainer
                                    )
    :ODefinitionContainer(_xORB,_xParentContainer,_pImpl)
    ,OPropertyStateContainer(OContentHelper::rBHelper)
    ,m_bFormsContainer(_bFormsContainer)
{
    DBG_CTOR(ODocumentContainer, NULL);
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY | PropertyAttribute::CONSTRAINED,
                    &m_pImpl->m_aProps.aTitle, ::getCppuType(&m_pImpl->m_aProps.aTitle));

    setElementApproval( PContainerApprove( new LocalNameApproval ( _xORB ) ) );
}

ODocumentContainer::~ODocumentContainer()
{
    DBG_DTOR(ODocumentContainer, NULL);

    if ( !OContentHelper::rBHelper.bInDispose && !OContentHelper::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }
}

IMPLEMENT_FORWARD_XINTERFACE3( ODocumentContainer,ODefinitionContainer,ODocumentContainer_Base,OPropertyStateContainer)
IMPLEMENT_TYPEPROVIDER3(ODocumentContainer,ODefinitionContainer,OPropertyStateContainer,ODocumentContainer_Base);
IMPLEMENT_SERVICE_INFO_IMPLNAME(ODocumentContainer, "com.sun.star.comp.dba.ODocumentContainer");
IMPLEMENT_SERVICE_INFO_SUPPORTS(ODocumentContainer);
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS(ODocumentContainer)

Sequence< ::rtl::OUString > SAL_CALL ODocumentContainer::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = m_bFormsContainer ? SERVICE_NAME_FORM_COLLECTION : SERVICE_NAME_REPORT_COLLECTION;
    return aSupported;
}

::rtl::OUString ODocumentContainer::determineContentType() const
{
    return ::rtl::OUString();
}

Reference< XContent > ODocumentContainer::createObject( const ::rtl::OUString& _rName)
{
    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( _rName );
    OSL_ENSURE( aFind != rDefinitions.end(), "ODocumentContainer::createObject:Invalid entry in map!" );
    if ( aFind->second->m_aProps.bIsFolder )
        return new ODocumentContainer( m_aContext.getLegacyServiceFactory(), *this, aFind->second, m_bFormsContainer );
    return new ODocumentDefinition( *this, m_aContext.getLegacyServiceFactory(), aFind->second, m_bFormsContainer );
}

Reference< XInterface > SAL_CALL ODocumentContainer::createInstance( const ::rtl::OUString& aServiceSpecifier ) throw (Exception, RuntimeException)
{
    return createInstanceWithArguments( aServiceSpecifier, Sequence< Any >() );
}

namespace
{
    template< class TYPE >
    void lcl_extractAndRemove( ::comphelper::NamedValueCollection& io_rArguments, const ::rtl::OUString& i_rName, TYPE& o_rValue )
    {
        if ( io_rArguments.has( i_rName ) )
        {
            io_rArguments.get_ensureType( i_rName, o_rValue );
            io_rArguments.remove( i_rName );
        }
    }
}

Reference< XInterface > SAL_CALL ODocumentContainer::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const Sequence< Any >& _aArguments ) throw (Exception, RuntimeException)
{
    Reference< XInterface > xRet;
    Reference< XContent > xContent;
    if ( ServiceSpecifier == SERVICE_SDB_DOCUMENTDEFINITION )
    {
        MutexGuard aGuard(m_aMutex);

        // extract known arguments
        ::rtl::OUString sName, sPersistentName, sURL, sMediaType, sDocServiceName;
        Reference< XCommandProcessor > xCopyFrom;
        Reference< XConnection > xConnection;
        sal_Bool bAsTemplate( sal_False );
        Sequence< sal_Int8 > aClassID;

        ::comphelper::NamedValueCollection aArgs( _aArguments );
        lcl_extractAndRemove( aArgs, PROPERTY_NAME, sName );
        lcl_extractAndRemove( aArgs, PROPERTY_PERSISTENT_NAME, sPersistentName );
        lcl_extractAndRemove( aArgs, PROPERTY_URL, sURL );
        lcl_extractAndRemove( aArgs, PROPERTY_EMBEDDEDOBJECT, xCopyFrom );
        lcl_extractAndRemove( aArgs, PROPERTY_ACTIVE_CONNECTION, xConnection );
        lcl_extractAndRemove( aArgs, PROPERTY_AS_TEMPLATE, bAsTemplate );
        lcl_extractAndRemove( aArgs, INFO_MEDIATYPE, sMediaType );
        lcl_extractAndRemove( aArgs, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DocumentServiceName" ) ), sDocServiceName );

        // ClassID has two allowed types, so a special treatment here
        Any aClassIDArg = aArgs.get( "ClassID" );
        if ( aClassIDArg.hasValue() )
        {
            if ( !( aClassIDArg >>= aClassID ) )
            {
                // Extended for usage also with a string
                ::rtl::OUString sClassIDString;
                if ( !( aClassIDArg >>= sClassIDString ) )
                    throw IllegalArgumentException( ::rtl::OUString(), *this, 2 );

                aClassID = ::comphelper::MimeConfigurationHelper::GetSequenceClassIDRepresentation( sClassIDString );
            }

#if OSL_DEBUG_LEVEL > 0
            ::rtl::OUString sClassIDString = ::comphelper::MimeConfigurationHelper::GetStringClassIDRepresentation( aClassID );
            (void)sClassIDString;
#endif
            aArgs.remove( "ClassID" );
        }
        // Everything which now is still present in the arguments is passed to the embedded object
        const Sequence< PropertyValue > aCreationArgs( aArgs.getPropertyValues() );

        const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
        sal_Bool bNew = ( 0 == sPersistentName.getLength() );
        if ( bNew )
        {
            const static ::rtl::OUString sBaseName(RTL_CONSTASCII_USTRINGPARAM("Obj"));

            sPersistentName = sBaseName;
            sPersistentName += ::rtl::OUString::valueOf(sal_Int32(rDefinitions.size() + 1));
            Reference<XNameAccess> xElements(getContainerStorage(),UNO_QUERY);
            if ( xElements.is() )
                sPersistentName = ::dbtools::createUniqueName(xElements,sPersistentName);

            const bool bNeedClassID = ( aClassID.getLength() == 0 ) && ( 0 == sURL.getLength() );
            if ( xCopyFrom.is() )
            {
                Sequence<Any> aIni(2);
                aIni[0] <<= getContainerStorage();
                aIni[1] <<= sPersistentName;
                Command aCommand;
                aCommand.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("copyTo"));
                aCommand.Argument <<= aIni;

                xCopyFrom->execute(aCommand,-1,Reference< XCommandEnvironment >());
                Reference<XPropertySet> xProp(xCopyFrom,UNO_QUERY);
                if ( xProp.is() && xProp->getPropertySetInfo().is() && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_AS_TEMPLATE) )
                    xProp->getPropertyValue(PROPERTY_AS_TEMPLATE) >>= bAsTemplate;

                // if we do not have an own class ID, see if we can determine one from the copy we just created
                if ( bNeedClassID )
                    ODocumentDefinition::GetDocumentServiceFromMediaType( getContainerStorage(), sPersistentName, m_aContext, aClassID );
            }
            else
            {
                if ( bNeedClassID )
                {
                    if ( sMediaType.getLength() )
                        ODocumentDefinition::GetDocumentServiceFromMediaType( sMediaType, m_aContext, aClassID );
                    else if ( sDocServiceName.getLength() )
                    {
                        ::comphelper::MimeConfigurationHelper aConfigHelper( m_aContext.getLegacyServiceFactory() );
                        const Sequence< NamedValue > aProps( aConfigHelper.GetObjectPropsByDocumentName( sDocServiceName ) );
                        const ::comphelper::NamedValueCollection aMediaTypeProps( aProps );
                        aClassID = aMediaTypeProps.getOrDefault( "ClassID", Sequence< sal_Int8 >() );
                    }
                }
            }
        }

        ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( sName );
        TContentPtr pElementImpl;
        if ( bNew || ( aFind == rDefinitions.end() ) )
        {
            pElementImpl.reset( new OContentHelper_Impl );
            if ( !bNew )
                pElementImpl->m_aProps.aTitle = sName;

            pElementImpl->m_aProps.sPersistentName = sPersistentName;
            pElementImpl->m_aProps.bAsTemplate = bAsTemplate;
            pElementImpl->m_pDataSource = m_pImpl->m_pDataSource;
        }
        else
            pElementImpl = aFind->second;

        ::rtl::Reference< ODocumentDefinition > pDocDef = new ODocumentDefinition( *this, m_aContext.getLegacyServiceFactory(), pElementImpl, m_bFormsContainer );
        if ( aClassID.getLength() )
        {
            pDocDef->initialLoad( aClassID, aCreationArgs, xConnection );
        }
        else
        {
            OSL_ENSURE( aCreationArgs.getLength() == 0, "ODocumentContainer::createInstance: additional creation args are lost, if you do not provide a class ID." );
        }
        xContent = pDocDef.get();

        if ( sURL.getLength() )
        {
            Sequence<Any> aIni(2);
            aIni[0] <<= sURL;
            Command aCommand;
            aCommand.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("insert"));
            aCommand.Argument <<= aIni;
            Reference< XCommandProcessor > xCommandProcessor(xContent,UNO_QUERY);
            if ( xContent.is() )
            {
                xCommandProcessor->execute(aCommand,-1,Reference< XCommandEnvironment >());
            }
        }
    }
    else if ( ServiceSpecifier == SERVICE_NAME_FORM_COLLECTION || SERVICE_NAME_REPORT_COLLECTION == ServiceSpecifier )
    {
        const Any* pBegin = _aArguments.getConstArray();
        const Any* pEnd = pBegin + _aArguments.getLength();
        PropertyValue aValue;
        ::rtl::OUString sName;
        Reference<XNameAccess> xCopyFrom;
        for(;pBegin != pEnd;++pBegin)
        {
            *pBegin >>= aValue;
            if ( aValue.Name.equalsAscii(PROPERTY_NAME) )
            {
                aValue.Value >>= sName;
            }
            else if ( aValue.Name.equalsAscii(PROPERTY_EMBEDDEDOBJECT) )
            {
                xCopyFrom.set(aValue.Value,UNO_QUERY);
            }
        }
        OSL_ENSURE(sName.getLength(),"Invalid name for a document container!");
        const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
        ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( sName );
        TContentPtr pElementImpl;
        if ( aFind == rDefinitions.end() )
        {
            pElementImpl.reset(new ODefinitionContainer_Impl);
            pElementImpl->m_aProps.aTitle = sName;
            pElementImpl->m_pDataSource = m_pImpl->m_pDataSource;
        }
        else
            pElementImpl = aFind->second;
        OSL_ENSURE( pElementImpl ," Invalid entry in map!");
        xContent = new ODocumentContainer( m_aContext.getLegacyServiceFactory(), *this, pElementImpl, ServiceSpecifier == SERVICE_NAME_FORM_COLLECTION );

        // copy children
        if ( xCopyFrom.is() )
        {
            Sequence< ::rtl::OUString> aSeq = xCopyFrom->getElementNames();
            const ::rtl::OUString* elements = aSeq.getConstArray();
            const ::rtl::OUString* elementsEnd = elements + aSeq.getLength();
            Reference<XContent> xObjectToCopy;

            Reference<XMultiServiceFactory> xORB(xContent,UNO_QUERY);
            OSL_ENSURE(xORB.is(),"No service factory given");
            if ( xORB.is() )
            {
                for(;elements != elementsEnd;++elements)
                {
                    xCopyFrom->getByName(*elements) >>= xObjectToCopy;
                    Sequence< Any > aArguments(3);
                    PropertyValue aArgument;
                    // set as folder
                    aArgument.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name"));
                    aArgument.Value <<= *elements;
                    aArguments[0] <<= aArgument;
                    //parent
                    aArgument.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Parent"));
                    aArgument.Value <<= xContent;
                    aArguments[1] <<= aArgument;

                    aArgument.Name = PROPERTY_EMBEDDEDOBJECT;
                    aArgument.Value <<= xObjectToCopy;
                    aArguments[2] <<= aArgument;

                    ::rtl::OUString sServiceName;
                    if ( Reference< XNameAccess >( xObjectToCopy, UNO_QUERY ).is() )
                    {
                        if ( m_bFormsContainer )
                            sServiceName = SERVICE_NAME_FORM_COLLECTION;
                        else
                            sServiceName = SERVICE_NAME_REPORT_COLLECTION;
                    }
                    else
                        sServiceName = SERVICE_SDB_DOCUMENTDEFINITION;

                    Reference<XContent > xNew(xORB->createInstanceWithArguments(sServiceName,aArguments),UNO_QUERY);
                    Reference<XNameContainer> xNameContainer(xContent,UNO_QUERY);
                    if ( xNameContainer.is() )
                        xNameContainer->insertByName(*elements,makeAny(xNew));
                }
            }
        }
    }
    xRet = xContent;
    return xRet;
}

Sequence< ::rtl::OUString > SAL_CALL ODocumentContainer::getAvailableServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSe(3);
    aSe[0] = SERVICE_SDB_DOCUMENTDEFINITION;
    aSe[1] = SERVICE_NAME_FORM_COLLECTION;
    aSe[2] = SERVICE_NAME_REPORT_COLLECTION;
    return aSe;
}

Any SAL_CALL ODocumentContainer::execute( const Command& aCommand, sal_Int32 CommandId, const Reference< XCommandEnvironment >& Environment ) throw (Exception, CommandAbortedException, RuntimeException)
{
    Any aRet;
    if ( aCommand.Name.compareToAscii( "open" ) == 0 )
    {
        //////////////////////////////////////////////////////////////////
        // open command for a folder content
        //////////////////////////////////////////////////////////////////
        OpenCommandArgument2 aOpenCommand;
          if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        sal_Bool bOpenFolder =
            ( ( aOpenCommand.Mode == OpenMode::ALL ) ||
              ( aOpenCommand.Mode == OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == OpenMode::DOCUMENTS ) );

        if ( bOpenFolder )
        {
            // open as folder - return result set

            Reference< XDynamicResultSet > xSet
                            = new DynamicResultSet( m_aContext.getLegacyServiceFactory(),
                                                    this,
                                                    aOpenCommand,
                                                    Environment );
            aRet <<= xSet;
          }
        else
        {
            // Unsupported.
            ucbhelper::cancelCommandExecution(
                makeAny( UnsupportedOpenModeException(
                                rtl::OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                sal_Int16( aOpenCommand.Mode ) ) ),
                Environment );
                // Unreachable
        }
    }
    else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "insert" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // insert
        //////////////////////////////////////////////////////////////////

        InsertCommandArgument arg;
          if ( !( aCommand.Argument >>= arg ) )
        {
              OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                makeAny( IllegalArgumentException(
                                    rtl::OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
    }
    else if ( aCommand.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "delete" ) ) )
    {
        //////////////////////////////////////////////////////////////////
        // delete
        //////////////////////////////////////////////////////////////////
        Sequence< ::rtl::OUString> aSeq = getElementNames();
        const ::rtl::OUString* pIter = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
            removeByName(*pIter);

        dispose();
    }
    else
        aRet = OContentHelper::execute(aCommand,CommandId,Environment);
    return aRet;
}

namespace
{
    sal_Bool lcl_queryContent(const ::rtl::OUString& _sName,Reference< XNameContainer >& _xNameContainer,Any& _rRet,::rtl::OUString& _sSimpleName)
    {
        sal_Bool bRet = sal_False;
        sal_Int32 nIndex = 0;
        ::rtl::OUString sName = _sName.getToken(0,'/',nIndex);
        bRet = _xNameContainer->hasByName(sName);
        if ( bRet )
        {
            _rRet = _xNameContainer->getByName(_sSimpleName = sName);
            while ( nIndex != -1 && bRet )
            {
                sName = _sName.getToken(0,'/',nIndex);
                _xNameContainer.set(_rRet,UNO_QUERY);
                bRet = _xNameContainer.is();
                if ( bRet )
                {
                    bRet = _xNameContainer->hasByName(sName);
                    _sSimpleName = sName;
                    if ( bRet )
                        _rRet = _xNameContainer->getByName(sName);
                }
            }
        }
        if ( nIndex == -1 )
            _sSimpleName = sName; // a content
        else
            _xNameContainer.clear(); // a sub folder doesn't exist
        return bRet;
    }
}

Reference< XComponent > SAL_CALL ODocumentContainer::loadComponentFromURL( const ::rtl::OUString& _sURL
                                                                       , const ::rtl::OUString& /*TargetFrameName*/
                                                                       , sal_Int32 /*SearchFlags*/
                                                                       , const Sequence< PropertyValue >& Arguments ) throw (IOException, IllegalArgumentException, RuntimeException)
{
    ::SolarMutexGuard aSolarGuard;

    MutexGuard aGuard(m_aMutex);
    Reference< XComponent > xComp;
    try
    {
        Any aContent;
        Reference< XNameContainer > xNameContainer(this);
        ::rtl::OUString sName;
        if ( !lcl_queryContent(_sURL,xNameContainer,aContent,sName) )
        {
            ::rtl::OUString sMessage( DBA_RES( RID_STR_NAME_NOT_FOUND ) );
            ::comphelper::string::searchAndReplaceAsciiI( sMessage, "$name$", _sURL );
            throw IllegalArgumentException( sMessage, *this, 1 );
        }

        Reference< XCommandProcessor > xContent(aContent,UNO_QUERY);
        if ( xContent.is() )
        {
            Command aCommand;

            ::comphelper::NamedValueCollection aArgs( Arguments );
            aCommand.Name = aArgs.getOrDefault( "OpenMode", ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "open" ) ) );
            aArgs.remove( "OpenMode" );

            OpenCommandArgument2 aOpenCommand;
            aOpenCommand.Mode = OpenMode::DOCUMENT;
            aArgs.put( "OpenCommandArgument", aOpenCommand );

            aCommand.Argument <<= aArgs.getPropertyValues();
            xComp.set(xContent->execute(aCommand,xContent->createCommandIdentifier(),Reference< XCommandEnvironment >()),UNO_QUERY);
        }
    }
    catch(NoSuchElementException)
    {
        throw IllegalArgumentException();
    }
    catch(WrappedTargetException &e)
    {
        throw;
    }
    return xComp;
}

Any SAL_CALL ODocumentContainer::getByHierarchicalName( const ::rtl::OUString& _sName ) throw (NoSuchElementException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Any aContent;
    Reference< XNameContainer > xNameContainer(this);
    ::rtl::OUString sName;
    if ( lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        return aContent;
    throw NoSuchElementException(_sName,*this);
}

sal_Bool SAL_CALL ODocumentContainer::hasByHierarchicalName( const ::rtl::OUString& _sName ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Any aContent;
    Reference< XNameContainer > xNameContainer(this);
    ::rtl::OUString sName;
    return lcl_queryContent(_sName,xNameContainer,aContent,sName);
}

// XHierarchicalNameContainer
void SAL_CALL ODocumentContainer::insertByHierarchicalName( const ::rtl::OUString& _sName, const Any& _aElement ) throw (IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    Reference< XContent > xContent(_aElement,UNO_QUERY);
    if ( !xContent.is() )
        throw IllegalArgumentException();

    ClearableMutexGuard aGuard(m_aMutex);
    Any aContent;
    Reference< XNameContainer > xNameContainer(this);
    ::rtl::OUString sName;
    if ( lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        throw ElementExistException(_sName,*this);

    if ( !xNameContainer.is() )
    {
        ::rtl::OUString sMessage( DBA_RES( RID_STR_NO_SUB_FOLDER ) );
        sal_Int32 index = sName.getLength();
        ::comphelper::string::searchAndReplaceAsciiI( sMessage, "$folder$", _sName.getToken(0,'/',index) );
        throw IllegalArgumentException( sMessage, *this, 1 );
    }

    xNameContainer->insertByName(sName,_aElement);
}

void SAL_CALL ODocumentContainer::removeByHierarchicalName( const ::rtl::OUString& _sName ) throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if ( !_sName.getLength() )
        throw NoSuchElementException(_sName,*this);

    ClearableMutexGuard aGuard(m_aMutex);
    Any aContent;
    ::rtl::OUString sName;
    Reference< XNameContainer > xNameContainer(this);
    if ( !lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        throw NoSuchElementException(_sName,*this);

    xNameContainer->removeByName(sName);
}

// XHierarchicalNameReplace
void SAL_CALL ODocumentContainer::replaceByHierarchicalName( const ::rtl::OUString& _sName, const Any& _aElement ) throw (IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    Reference< XContent > xContent(_aElement,UNO_QUERY);
    if ( !xContent.is() )
        throw IllegalArgumentException();

    ClearableMutexGuard aGuard(m_aMutex);
    Any aContent;
    ::rtl::OUString sName;
    Reference< XNameContainer > xNameContainer(this);
    if ( !lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        throw NoSuchElementException(_sName,*this);

    xNameContainer->replaceByName(sName,_aElement);
}

::rtl::OUString SAL_CALL ODocumentContainer::getHierarchicalName() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return impl_getHierarchicalName( false );
}

::rtl::OUString SAL_CALL ODocumentContainer::composeHierarchicalName( const ::rtl::OUString& i_rRelativeName ) throw (IllegalArgumentException, NoSupportException, RuntimeException)
{
    ::rtl::OUStringBuffer aBuffer;
    aBuffer.append( getHierarchicalName() );
    aBuffer.append( sal_Unicode( '/' ) );
    aBuffer.append( i_rRelativeName );
    return aBuffer.makeStringAndClear();
}

::rtl::Reference<OContentHelper> ODocumentContainer::getContent(const ::rtl::OUString& _sName) const
{
    ::rtl::Reference<OContentHelper> pContent = NULL;
    try
    {
        Reference<XUnoTunnel> xUnoTunnel(const_cast<ODocumentContainer*>(this)->implGetByName( _sName, sal_True ), UNO_QUERY );
        if ( xUnoTunnel.is() )
            pContent = reinterpret_cast<OContentHelper*>(xUnoTunnel->getSomething(OContentHelper::getUnoTunnelImplementationId()));
    }
    catch(Exception)
    {
    }
    return pContent;
}

void ODocumentContainer::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    _rDefault.clear();
}

void SAL_CALL ODocumentContainer::commit(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Documents::iterator aIter = m_aDocumentMap.begin();
    Documents::iterator aEnd = m_aDocumentMap.end();
    for (; aIter != aEnd ; ++aIter)
    {
        Reference<XTransactedObject> xTrans(aIter->second.get(),UNO_QUERY);
        if ( xTrans.is() )
            xTrans->commit();
    }
    Reference<XTransactedObject> xTrans(getContainerStorage(),UNO_QUERY);
    if ( xTrans.is() )
        xTrans->commit();
}

void SAL_CALL ODocumentContainer::revert(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Documents::iterator aIter = m_aDocumentMap.begin();
    Documents::iterator aEnd = m_aDocumentMap.end();
    for (; aIter != aEnd ; ++aIter)
    {
        Reference<XTransactedObject> xTrans(aIter->second.get(),UNO_QUERY);
        if ( xTrans.is() )
            xTrans->revert();
    }
    Reference<XTransactedObject> xTrans(getContainerStorage(),UNO_QUERY);
    if ( xTrans.is() )
        xTrans->revert();
}

Reference< XStorage> ODocumentContainer::getContainerStorage() const
{
    return  m_pImpl->m_pDataSource
        ?   m_pImpl->m_pDataSource->getStorage( m_bFormsContainer ? ODatabaseModelImpl::E_FORM : ODatabaseModelImpl::E_REPORT )
        :   Reference< XStorage>();
}

void SAL_CALL ODocumentContainer::removeByName( const ::rtl::OUString& _rName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ResettableMutexGuard aGuard(m_aMutex);

    // check the arguments
    if (!_rName.getLength())
        throw IllegalArgumentException();

    if (!checkExistence(_rName))
        throw NoSuchElementException(_rName,*this);

    Reference< XCommandProcessor > xContent( implGetByName( _rName, sal_True ), UNO_QUERY );
    if ( xContent.is() )
    {
        Command aCommand;

        aCommand.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("delete"));
        xContent->execute(aCommand,xContent->createCommandIdentifier(),Reference< XCommandEnvironment >());
    }

    // do the removal
    implRemove(_rName);

    notifyByName( aGuard, _rName, NULL, NULL, E_REMOVED, ContainerListemers );
}

void SAL_CALL ODocumentContainer::rename( const ::rtl::OUString& newName ) throw (SQLException, ElementExistException, RuntimeException)
{
    try
    {
        osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
        if ( newName.equals( m_pImpl->m_aProps.aTitle ) )
            return;

        sal_Int32 nHandle = PROPERTY_ID_NAME;
        Any aOld = makeAny(m_pImpl->m_aProps.aTitle);
        Any aNew = makeAny(newName);

        aGuard.clear();
        fire(&nHandle, &aNew, &aOld, 1, sal_True );
        m_pImpl->m_aProps.aTitle = newName;
        fire(&nHandle, &aNew, &aOld, 1, sal_False );
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(newName,*this);
    }
}

}   // namespace dbaccess
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
