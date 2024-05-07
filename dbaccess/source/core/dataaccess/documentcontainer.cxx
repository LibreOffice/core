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

#include "documentcontainer.hxx"
#include <stringconstants.hxx>
#include "documentdefinition.hxx"
#include <ModelImpl.hxx>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <connectivity/dbtools.hxx>
#include "myucp_resultset.hxx"
#include <ucbhelper/cancelcommandexecution.hxx>
#include <com/sun/star/ucb/UnsupportedOpenModeException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#include <comphelper/mimeconfighelper.hxx>
#include <connectivity/sqlerror.hxx>
#include <core_resource.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <o3tl/string_view.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

namespace dbaccess
{

namespace {

// LocalNameApproval
class LocalNameApproval : public IContainerApprove
{
    ::connectivity::SQLError    m_aErrors;

public:
    void approveElement( const OUString& _rName ) override;
};

}

void LocalNameApproval::approveElement( const OUString& _rName )
{
    if ( _rName.indexOf( '/' ) != -1 )
        throw IllegalArgumentException(
            m_aErrors.getErrorMessage( ErrorCondition::DB_OBJECT_NAME_WITH_SLASHES ),
            nullptr,
            0
        );
}

// ODocumentContainer

ODocumentContainer::ODocumentContainer(const Reference< XComponentContext >& _xORB
                                    ,const Reference< XInterface >& _xParentContainer
                                    ,const TContentPtr& _pImpl
                                    , bool _bFormsContainer
                                    )
    :ODefinitionContainer(_xORB,_xParentContainer,_pImpl)
    ,OPropertyStateContainer(OContentHelper::rBHelper)
    ,m_bFormsContainer(_bFormsContainer)
{
    registerProperty(PROPERTY_NAME, PROPERTY_ID_NAME, PropertyAttribute::BOUND | PropertyAttribute::READONLY | PropertyAttribute::CONSTRAINED,
                    &m_pImpl->m_aProps.aTitle, cppu::UnoType<decltype(m_pImpl->m_aProps.aTitle)>::get());

    setElementApproval( std::make_shared<LocalNameApproval>() );
}

ODocumentContainer::~ODocumentContainer()
{

    if ( !OContentHelper::rBHelper.bInDispose && !OContentHelper::rBHelper.bDisposed )
    {
        acquire();
        dispose();
    }
}

IMPLEMENT_FORWARD_XINTERFACE3( ODocumentContainer,ODefinitionContainer,ODocumentContainer_Base,OPropertyStateContainer)

css::uno::Sequence<sal_Int8> ODocumentContainer::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

css::uno::Sequence< css::uno::Type > ODocumentContainer::getTypes()
{
    return  ::comphelper::concatSequences(
        ODefinitionContainer::getTypes( ),
        OPropertyStateContainer::getTypes( ),
        ODocumentContainer_Base::getTypes( )
    );
}
OUString SAL_CALL ODocumentContainer::getImplementationName()
    {
        return u"com.sun.star.comp.dba.ODocumentContainer"_ustr;
    };
sal_Bool SAL_CALL ODocumentContainer::supportsService(const OUString& _rServiceName)
    {
        const css::uno::Sequence< OUString > aSupported(getSupportedServiceNames());
        for (const OUString& s : aSupported)
            if (s == _rServiceName)
                return true;

        return false;
    };
css::uno::Reference< css::beans::XPropertySetInfo >  SAL_CALL ODocumentContainer::getPropertySetInfo()
{
    Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
::cppu::IPropertyArrayHelper& ODocumentContainer::getInfoHelper()
{
    return *ODocumentContainer::getArrayHelper();
}
::cppu::IPropertyArrayHelper* ODocumentContainer::createArrayHelper( ) const
{
    css::uno::Sequence< css::beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}


Sequence< OUString > SAL_CALL ODocumentContainer::getSupportedServiceNames(  )
{
    return { m_bFormsContainer ? SERVICE_NAME_FORM_COLLECTION : SERVICE_NAME_REPORT_COLLECTION };
}

OUString ODocumentContainer::determineContentType() const
{
    return OUString();
}

Reference< XContent > ODocumentContainer::createObject( const OUString& _rName)
{
    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( _rName );
    OSL_ENSURE( aFind != rDefinitions.end(), "ODocumentContainer::createObject:Invalid entry in map!" );
    if ( aFind->second->m_aProps.bIsFolder )
        return new ODocumentContainer( m_aContext, *this, aFind->second, m_bFormsContainer );
    return new ODocumentDefinition( *this, m_aContext, aFind->second, m_bFormsContainer );
}

Reference< XInterface > SAL_CALL ODocumentContainer::createInstance( const OUString& aServiceSpecifier )
{
    return createInstanceWithArguments( aServiceSpecifier, Sequence< Any >() );
}

namespace
{
    template< class TYPE >
    void lcl_extractAndRemove( ::comphelper::NamedValueCollection& io_rArguments, const OUString& i_rName, TYPE& o_rValue )
    {
        if ( io_rArguments.has( i_rName ) )
        {
            io_rArguments.get_ensureType( i_rName, o_rValue );
            io_rArguments.remove( i_rName );
        }
    }
}

Reference< XInterface > SAL_CALL ODocumentContainer::createInstanceWithArguments( const OUString& ServiceSpecifier, const Sequence< Any >& _aArguments )
{
    Reference< XInterface > xRet;
    Reference< XContent > xContent;
    if ( ServiceSpecifier == SERVICE_SDB_DOCUMENTDEFINITION )
    {
        MutexGuard aGuard(m_aMutex);

        // extract known arguments
        OUString sName, sPersistentName, sURL, sMediaType, sDocServiceName;
        Reference< XCommandProcessor > xCopyFrom;
        Reference< XConnection > xConnection;
        bool bAsTemplate( false );
        Sequence< sal_Int8 > aClassID;

        ::comphelper::NamedValueCollection aArgs( _aArguments );
        lcl_extractAndRemove( aArgs, PROPERTY_NAME, sName );
        lcl_extractAndRemove( aArgs, PROPERTY_PERSISTENT_NAME, sPersistentName );
        lcl_extractAndRemove( aArgs, PROPERTY_URL, sURL );
        lcl_extractAndRemove( aArgs, PROPERTY_EMBEDDEDOBJECT, xCopyFrom );
        lcl_extractAndRemove( aArgs, PROPERTY_ACTIVE_CONNECTION, xConnection );
        lcl_extractAndRemove( aArgs, PROPERTY_AS_TEMPLATE, bAsTemplate );
        lcl_extractAndRemove( aArgs, INFO_MEDIATYPE, sMediaType );
        lcl_extractAndRemove( aArgs, u"DocumentServiceName"_ustr , sDocServiceName );

        // ClassID has two allowed types, so a special treatment here
        Any aClassIDArg = aArgs.get( u"ClassID"_ustr );
        if ( aClassIDArg.hasValue() )
        {
            if ( !( aClassIDArg >>= aClassID ) )
            {
                // Extended for usage also with a string
                OUString sClassIDString;
                if ( !( aClassIDArg >>= sClassIDString ) )
                    throw IllegalArgumentException( OUString(), *this, 2 );

                aClassID = ::comphelper::MimeConfigurationHelper::GetSequenceClassIDRepresentation( sClassIDString );
            }

#if OSL_DEBUG_LEVEL > 0
            OUString sClassIDString = ::comphelper::MimeConfigurationHelper::GetStringClassIDRepresentation( aClassID );
            (void)sClassIDString;
#endif
            aArgs.remove( u"ClassID"_ustr );
        }
        // Everything which now is still present in the arguments is passed to the embedded object
        const Sequence< PropertyValue > aCreationArgs( aArgs.getPropertyValues() );

        const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
        bool bNew = sPersistentName.isEmpty();
        if ( bNew )
        {
            sPersistentName = "Obj" + OUString::number(rDefinitions.size() + 1);
            Reference<XNameAccess> xElements = getContainerStorage();
            if ( xElements.is() )
                sPersistentName = ::dbtools::createUniqueName(xElements,sPersistentName);

            const bool bNeedClassID = !aClassID.hasElements() && sURL.isEmpty() ;
            if ( xCopyFrom.is() )
            {
                Sequence<Any> aIni{ Any(getContainerStorage()), Any(sPersistentName) };
                Command aCommand;
                aCommand.Name = "copyTo";
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
                    if ( !sMediaType.isEmpty() )
                        ODocumentDefinition::GetDocumentServiceFromMediaType( sMediaType, m_aContext, aClassID );
                    else if ( !sDocServiceName.isEmpty() )
                    {
                        ::comphelper::MimeConfigurationHelper aConfigHelper( m_aContext );
                        const Sequence< NamedValue > aProps( aConfigHelper.GetObjectPropsByDocumentName( sDocServiceName ) );
                        const ::comphelper::NamedValueCollection aMediaTypeProps( aProps );
                        aClassID = aMediaTypeProps.getOrDefault( u"ClassID"_ustr, Sequence< sal_Int8 >() );
                    }
                }
            }
        }

        ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( sName );
        TContentPtr pElementImpl;
        if ( bNew || ( aFind == rDefinitions.end() ) )
        {
            pElementImpl = std::make_shared<OContentHelper_Impl>();
            if ( !bNew )
                pElementImpl->m_aProps.aTitle = sName;

            pElementImpl->m_aProps.sPersistentName = sPersistentName;
            pElementImpl->m_aProps.bAsTemplate = bAsTemplate;
            pElementImpl->m_pDataSource = m_pImpl->m_pDataSource;
        }
        else
            pElementImpl = aFind->second;

        ::rtl::Reference< ODocumentDefinition > pDocDef = new ODocumentDefinition( *this, m_aContext, pElementImpl, m_bFormsContainer );
        if ( aClassID.hasElements() )
        {
            pDocDef->initialLoad( aClassID, aCreationArgs, xConnection );
        }
        else
        {
            OSL_ENSURE( !aCreationArgs.hasElements(), "ODocumentContainer::createInstance: additional creation args are lost, if you do not provide a class ID." );
        }
        xContent = pDocDef.get();

        if ( !sURL.isEmpty() )
        {
            Sequence<Any> aIni{ Any(sURL) };
            Command aCommand;
            aCommand.Name = "insert";
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
        OUString sName;
        Reference<XNameAccess> xCopyFrom;
        for (auto& arg : _aArguments)
        {
            PropertyValue aValue;
            arg >>= aValue;
            if ( aValue.Name == PROPERTY_NAME)
            {
                aValue.Value >>= sName;
            }
            else if ( aValue.Name == PROPERTY_EMBEDDEDOBJECT)
            {
                xCopyFrom.set(aValue.Value,UNO_QUERY);
            }
        }
        OSL_ENSURE(!sName.isEmpty(),"Invalid name for a document container!");
        const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
        ODefinitionContainer_Impl::const_iterator aFind = rDefinitions.find( sName );
        TContentPtr pElementImpl;
        if ( aFind == rDefinitions.end() )
        {
            pElementImpl = std::make_shared<ODefinitionContainer_Impl>();
            pElementImpl->m_aProps.aTitle = sName;
            pElementImpl->m_pDataSource = m_pImpl->m_pDataSource;
        }
        else
            pElementImpl = aFind->second;
        OSL_ENSURE( pElementImpl ," Invalid entry in map!");
        xContent = new ODocumentContainer( m_aContext, *this, pElementImpl, ServiceSpecifier == SERVICE_NAME_FORM_COLLECTION );

        // copy children
        if ( xCopyFrom.is() )
        {
            Reference<XContent> xObjectToCopy;

            Reference<XMultiServiceFactory> xORB(xContent,UNO_QUERY);
            OSL_ENSURE(xORB.is(),"No service factory given");
            if ( xORB.is() )
            {
                for (auto& element : xCopyFrom->getElementNames())
                {
                    xCopyFrom->getByName(element) >>= xObjectToCopy;
                    Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
                    {
                        {"Name", Any(element)}, // set as folder
                        {"Parent", Any(xContent)},
                        {PROPERTY_EMBEDDEDOBJECT, Any(xObjectToCopy)},
                    }));

                    OUString sServiceName;
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
                        xNameContainer->insertByName(element, Any(xNew));
                }
            }
        }
    }
    xRet = xContent;
    return xRet;
}

Sequence< OUString > SAL_CALL ODocumentContainer::getAvailableServiceNames(  )
{
    return
    {
        SERVICE_SDB_DOCUMENTDEFINITION,
        SERVICE_NAME_FORM_COLLECTION,
        SERVICE_NAME_REPORT_COLLECTION
    };
}

Any SAL_CALL ODocumentContainer::execute( const Command& aCommand, sal_Int32 CommandId, const Reference< XCommandEnvironment >& Environment )
{
    Any aRet;
    if ( aCommand.Name == "open" )
    {
        // open command for a folder content
        OpenCommandArgument2 aOpenCommand;
        if ( !( aCommand.Argument >>= aOpenCommand ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                Any( IllegalArgumentException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
        bool bOpenFolder =
            ( ( aOpenCommand.Mode == OpenMode::ALL ) ||
              ( aOpenCommand.Mode == OpenMode::FOLDERS ) ||
              ( aOpenCommand.Mode == OpenMode::DOCUMENTS ) );

        if ( bOpenFolder )
        {
            // open as folder - return result set

            Reference< XDynamicResultSet > xSet
                            = new DynamicResultSet( m_aContext,
                                                    this,
                                                    aOpenCommand,
                                                    Environment );
            aRet <<= xSet;
        }
        else
        {
            // Unsupported.
            ucbhelper::cancelCommandExecution(
                Any( UnsupportedOpenModeException(
                                OUString(),
                                static_cast< cppu::OWeakObject * >( this ),
                                sal_Int16( aOpenCommand.Mode ) ) ),
                Environment );
                // Unreachable
        }
    }
    else if ( aCommand.Name == "insert" )
    {
        // insert

        InsertCommandArgument arg;
        if ( !( aCommand.Argument >>= arg ) )
        {
            OSL_FAIL( "Wrong argument type!" );
            ucbhelper::cancelCommandExecution(
                Any( IllegalArgumentException(
                                    OUString(),
                                    static_cast< cppu::OWeakObject * >( this ),
                                    -1 ) ),
                Environment );
            // Unreachable
        }
    }
    else if ( aCommand.Name == "delete" )
    {
        // delete
        for (auto& name : getElementNames())
            removeByName(name);

        dispose();
    }
    else
        aRet = OContentHelper::execute(aCommand,CommandId,Environment);
    return aRet;
}

namespace
{
    bool lcl_queryContent(std::u16string_view _sName,Reference< XNameContainer >& _xNameContainer,Any& _rRet,OUString& _sSimpleName)
    {
        sal_Int32 nIndex = 0;
        OUString sName( o3tl::getToken(_sName,0,'/',nIndex) );
        bool bRet = _xNameContainer->hasByName(sName);
        if ( bRet )
        {
            _sSimpleName = sName;
            _rRet = _xNameContainer->getByName(_sSimpleName);
            while ( nIndex != -1 && bRet )
            {
                sName = o3tl::getToken(_sName,0,'/',nIndex);
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

Reference< XComponent > SAL_CALL ODocumentContainer::loadComponentFromURL( const OUString& _sURL
                                                                       , const OUString& /*TargetFrameName*/
                                                                       , sal_Int32 /*SearchFlags*/
                                                                       , const Sequence< PropertyValue >& Arguments )
{
    ::SolarMutexGuard aSolarGuard;

    MutexGuard aGuard(m_aMutex);
    Reference< XComponent > xComp;
    try
    {
        Any aContent;
        Reference< XNameContainer > xNameContainer(this);
        OUString sName;
        if ( !lcl_queryContent(_sURL,xNameContainer,aContent,sName) )
        {
            OUString sMessage(
                DBA_RES(RID_STR_NAME_NOT_FOUND).replaceFirst("$name$", _sURL));
            throw IllegalArgumentException( sMessage, *this, 1 );
        }

        Reference< XCommandProcessor > xContent(aContent,UNO_QUERY);
        if ( xContent.is() )
        {
            Command aCommand;

            ::comphelper::NamedValueCollection aArgs( Arguments );
            aCommand.Name = aArgs.getOrDefault( u"OpenMode"_ustr, u"open"_ustr );
            aArgs.remove( u"OpenMode"_ustr );

            OpenCommandArgument2 aOpenCommand;
            aOpenCommand.Mode = OpenMode::DOCUMENT;
            aArgs.put( u"OpenCommandArgument"_ustr, aOpenCommand );

            aCommand.Argument <<= aArgs.getPropertyValues();
            xComp.set(xContent->execute(aCommand,xContent->createCommandIdentifier(),Reference< XCommandEnvironment >()),UNO_QUERY);
        }
    }
    catch(const NoSuchElementException&)
    {
        throw IllegalArgumentException();
    }
    catch(const WrappedTargetException &e)
    {
        throw WrappedTargetRuntimeException(e.Message, e.Context, e.TargetException);
    }
    return xComp;
}

Any SAL_CALL ODocumentContainer::getByHierarchicalName( const OUString& _sName )
{
    MutexGuard aGuard(m_aMutex);
    Any aContent;
    Reference< XNameContainer > xNameContainer(this);
    OUString sName;
    if ( lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        return aContent;
    throw NoSuchElementException(_sName,*this);
}

sal_Bool SAL_CALL ODocumentContainer::hasByHierarchicalName( const OUString& _sName )
{
    MutexGuard aGuard(m_aMutex);
    Any aContent;
    Reference< XNameContainer > xNameContainer(this);
    OUString sName;
    return lcl_queryContent(_sName,xNameContainer,aContent,sName);
}

// XHierarchicalNameContainer
void SAL_CALL ODocumentContainer::insertByHierarchicalName( const OUString& _sName, const Any& _aElement )
{
    Reference< XContent > xContent(_aElement,UNO_QUERY);
    if ( !xContent.is() )
        throw IllegalArgumentException();

    MutexGuard aGuard(m_aMutex);
    Any aContent;
    Reference< XNameContainer > xNameContainer(this);
    OUString sName;
    if ( lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        throw ElementExistException(_sName,*this);

    if ( !xNameContainer.is() )
    {
        sal_Int32 index = sName.getLength();
        OUString sMessage(
            DBA_RES(RID_STR_NO_SUB_FOLDER).replaceFirst("$folder$",
                o3tl::getToken(_sName, 0,'/',index)));
        throw IllegalArgumentException( sMessage, *this, 1 );
    }

    xNameContainer->insertByName(sName,_aElement);
}

void SAL_CALL ODocumentContainer::removeByHierarchicalName( const OUString& _sName )
{
    if ( _sName.isEmpty() )
        throw NoSuchElementException(_sName,*this);

    MutexGuard aGuard(m_aMutex);
    Any aContent;
    OUString sName;
    Reference< XNameContainer > xNameContainer(this);
    if ( !lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        throw NoSuchElementException(_sName,*this);

    xNameContainer->removeByName(sName);
}

// XHierarchicalNameReplace
void SAL_CALL ODocumentContainer::replaceByHierarchicalName( const OUString& _sName, const Any& _aElement )
{
    Reference< XContent > xContent(_aElement,UNO_QUERY);
    if ( !xContent.is() )
        throw IllegalArgumentException();

    MutexGuard aGuard(m_aMutex);
    Any aContent;
    OUString sName;
    Reference< XNameContainer > xNameContainer(this);
    if ( !lcl_queryContent(_sName,xNameContainer,aContent,sName) )
        throw NoSuchElementException(_sName,*this);

    xNameContainer->replaceByName(sName,_aElement);
}

OUString SAL_CALL ODocumentContainer::getHierarchicalName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return impl_getHierarchicalName( false );
}

OUString SAL_CALL ODocumentContainer::composeHierarchicalName( const OUString& i_rRelativeName )
{
    OUString aBuffer = getHierarchicalName() + "/" + i_rRelativeName;
    return aBuffer;
}

::rtl::Reference<OContentHelper> ODocumentContainer::getContent(const OUString& _sName) const
{
    ::rtl::Reference<OContentHelper> pContent;
    try
    {
        pContent = dynamic_cast<OContentHelper*>(const_cast<ODocumentContainer*>(this)->implGetByName( _sName, true ).get());
    }
    catch(const Exception&)
    {
    }
    return pContent;
}

void ODocumentContainer::getPropertyDefaultByHandle( sal_Int32 /*_nHandle*/, Any& _rDefault ) const
{
    _rDefault.clear();
}

void SAL_CALL ODocumentContainer::commit(  )
{
    MutexGuard aGuard(m_aMutex);
    for (auto const& elem : m_aDocumentMap)
    {
        Reference<XTransactedObject> xTrans(elem.second.get(),UNO_QUERY);
        if ( xTrans.is() )
            xTrans->commit();
    }
    Reference<XTransactedObject> xTrans(getContainerStorage(),UNO_QUERY);
    if ( xTrans.is() )
        xTrans->commit();
}

void SAL_CALL ODocumentContainer::revert(  )
{
    MutexGuard aGuard(m_aMutex);
    for (auto const& elem : m_aDocumentMap)
    {
        Reference<XTransactedObject> xTrans(elem.second.get(),UNO_QUERY);
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
        ?   m_pImpl->m_pDataSource->getStorage( m_bFormsContainer ? ODatabaseModelImpl::ObjectType::Form : ODatabaseModelImpl::ObjectType::Report )
        :   Reference< XStorage>();
}

void SAL_CALL ODocumentContainer::removeByName( const OUString& _rName )
{
    ResettableMutexGuard aGuard(m_aMutex);

    // check the arguments
    if (_rName.isEmpty())
        throw IllegalArgumentException();

    if (!checkExistence(_rName))
        throw NoSuchElementException(_rName,*this);

    Reference< XCommandProcessor > xContent( implGetByName( _rName, true ), UNO_QUERY );
    if ( xContent.is() )
    {
        Command aCommand;

        aCommand.Name = "delete";
        xContent->execute(aCommand,xContent->createCommandIdentifier(),Reference< XCommandEnvironment >());
    }

    // do the removal
    implRemove(_rName);

    notifyByName( aGuard, _rName, nullptr, nullptr, E_REMOVED, ContainerListemers );
}

void SAL_CALL ODocumentContainer::rename( const OUString& newName )
{
    try
    {
        osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
        if ( newName == m_pImpl->m_aProps.aTitle )
            return;

        sal_Int32 nHandle = PROPERTY_ID_NAME;
        Any aOld(m_pImpl->m_aProps.aTitle);
        Any aNew(newName);

        aGuard.clear();
        fire(&nHandle, &aNew, &aOld, 1, true );
        m_pImpl->m_aProps.aTitle = newName;
        fire(&nHandle, &aNew, &aOld, 1, false );
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(newName,*this);
    }
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
