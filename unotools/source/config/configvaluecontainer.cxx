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

#include <sal/config.h>

#include <sal/log.hxx>
#include <unotools/configvaluecontainer.hxx>
#include <unotools/confignode.hxx>
#include <uno/data.h>
#include <algorithm>
#include <vector>

namespace utl
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //= NodeValueAccessor

    enum class LocationType
    {
        SimplyObjectInstance,
        Unbound
    };

    struct NodeValueAccessor
    {
    private:
        OUString            sRelativePath;      // the relative path of the node
        LocationType        eLocationType;      // the type of location where the value is stored
        void*               pLocation;          // the pointer to the location
        Type                aDataType;          // the type object pointed to by pLocation

    public:
        explicit NodeValueAccessor( const OUString& _rNodePath );

        void bind( void* _pLocation, const Type& _rType );

        bool                    isBound( ) const        { return ( LocationType::Unbound != eLocationType ) && ( nullptr != pLocation ); }
        const OUString&  getPath( ) const        { return sRelativePath; }
        LocationType            getLocType( ) const     { return eLocationType; }
        void*                   getLocation( ) const    { return pLocation; }
        const Type&             getDataType( ) const    { return aDataType; }

        bool operator == ( const NodeValueAccessor& rhs ) const;
    };

    NodeValueAccessor::NodeValueAccessor( const OUString& _rNodePath )
        :sRelativePath( _rNodePath )
        ,eLocationType( LocationType::Unbound )
        ,pLocation( nullptr )
    {
    }

    bool NodeValueAccessor::operator == ( const NodeValueAccessor& rhs ) const
    {
        return  (   sRelativePath   ==  rhs.sRelativePath   )
            &&  (   eLocationType   ==  rhs.eLocationType   )
            &&  (   pLocation       ==  rhs.pLocation       );
    }

    void NodeValueAccessor::bind( void* _pLocation, const Type& _rType )
    {
        SAL_WARN_IF(isBound(), "unotools.config", "NodeValueAccessor::bind: already bound!");

        eLocationType = LocationType::SimplyObjectInstance;
        pLocation = _pLocation;
        aDataType = _rType;
    }

    #ifndef UNX
    static
    #endif
    void lcl_copyData( const NodeValueAccessor& _rAccessor, const Any& _rData, ::osl::Mutex& _rMutex )
    {
        ::osl::MutexGuard aGuard( _rMutex );

        SAL_WARN_IF(!_rAccessor.isBound(), "unotools.config", "::utl::lcl_copyData: invalid accessor!");
        switch ( _rAccessor.getLocType() )
        {
            case LocationType::SimplyObjectInstance:
            {
                if ( _rData.hasValue() )
                {
                    // assign the value
                    bool bSuccess = uno_type_assignData(
                        _rAccessor.getLocation(), _rAccessor.getDataType().getTypeLibType(),
                        const_cast< void* >( _rData.getValue() ), _rData.getValueType().getTypeLibType(),
                        cpp_queryInterface, cpp_acquire, cpp_release
                    );
                    SAL_WARN_IF(!bSuccess, "unotools.config",
                        "::utl::lcl_copyData( Accessor, Any ): could not assign the data (node path: \"" << _rAccessor.getPath() << "\"");
                }
                else {
                    SAL_INFO("unotools.config", "::utl::lcl_copyData: NULL value lost!");
                }
            }
            break;
            default:
                break;
        }
    }

    #ifndef UNX
    static
    #endif
    void lcl_copyData( Any& _rData, const NodeValueAccessor& _rAccessor, ::osl::Mutex& _rMutex )
    {
        ::osl::MutexGuard aGuard( _rMutex );

        SAL_WARN_IF(!_rAccessor.isBound(), "unotools.config", "::utl::lcl_copyData: invalid accessor!" );
        switch ( _rAccessor.getLocType() )
        {
            case LocationType::SimplyObjectInstance:
                // a simple setValue ....
                _rData.setValue( _rAccessor.getLocation(), _rAccessor.getDataType() );
                break;

            default:
                break;
        }
    }

    //= functors on NodeValueAccessor instances

    /// base class for functors synchronizing between exchange locations and config sub nodes
    struct SubNodeAccess : public ::std::unary_function< NodeValueAccessor, void >
    {
    protected:
        const OConfigurationNode&   m_rRootNode;
        ::osl::Mutex&               m_rMutex;

    public:
        SubNodeAccess( const OConfigurationNode& _rRootNode, ::osl::Mutex& _rMutex )
            :m_rRootNode( _rRootNode )
            ,m_rMutex( _rMutex )
        {
        }
    };

    struct UpdateFromConfig : public SubNodeAccess
    {
    public:
        UpdateFromConfig( const OConfigurationNode& _rRootNode, ::osl::Mutex& _rMutex ) : SubNodeAccess( _rRootNode, _rMutex ) { }

        void operator() ( NodeValueAccessor& _rAccessor )
        {
            ::utl::lcl_copyData( _rAccessor, m_rRootNode.getNodeValue( _rAccessor.getPath( ) ), m_rMutex );
        }
    };

    struct UpdateToConfig : public SubNodeAccess
    {
    public:
        UpdateToConfig( const OConfigurationNode& _rRootNode, ::osl::Mutex& _rMutex ) : SubNodeAccess( _rRootNode, _rMutex ) { }

        void operator() ( NodeValueAccessor& _rAccessor )
        {
            Any aNewValue;
            lcl_copyData( aNewValue, _rAccessor, m_rMutex );
            m_rRootNode.setNodeValue( _rAccessor.getPath( ), aNewValue );
        }
    };

    typedef std::vector<NodeValueAccessor> NodeValueAccessors;

    //= OConfigurationValueContainerImpl

    struct OConfigurationValueContainerImpl
    {
        Reference< XComponentContext >          xORB;           // the service factory
        ::osl::Mutex&                           rMutex;         // the mutex for accessing the data containers
        OConfigurationTreeRoot                  aConfigRoot;    // the configuration node we're accessing

        NodeValueAccessors                      aAccessors;     // the accessors to the node values

        OConfigurationValueContainerImpl( const Reference< XComponentContext >& _rxORB, ::osl::Mutex& _rMutex )
            :xORB( _rxORB )
            ,rMutex( _rMutex )
        {
        }
    };

    //= OConfigurationValueContainer

    OConfigurationValueContainer::OConfigurationValueContainer(
            const Reference< XComponentContext >& _rxORB, ::osl::Mutex& _rAccessSafety,
            const sal_Char* _pConfigLocation, const sal_Int32 _nLevels )
        :m_pImpl( new OConfigurationValueContainerImpl( _rxORB, _rAccessSafety ) )
    {
        implConstruct( OUString::createFromAscii( _pConfigLocation ), _nLevels );
    }

    OConfigurationValueContainer::~OConfigurationValueContainer()
    {
    }

    void OConfigurationValueContainer::implConstruct( const OUString& _rConfigLocation,
        const sal_Int32 _nLevels )
    {
        SAL_WARN_IF(m_pImpl->aConfigRoot.isValid(), "unotools.config", "OConfigurationValueContainer::implConstruct: already initialized!");

        // create the configuration node we're about to work with
        m_pImpl->aConfigRoot = OConfigurationTreeRoot::createWithComponentContext(
            m_pImpl->xORB,
            _rConfigLocation,
            _nLevels
        );
        SAL_WARN_IF(!m_pImpl->aConfigRoot.isValid(), "unotools.config",
            "Could not access the configuration node located at " << _rConfigLocation);
    }

    void OConfigurationValueContainer::registerExchangeLocation( const sal_Char* _pRelativePath,
        void* _pContainer, const Type& _rValueType )
    {
        // checks ....
        SAL_WARN_IF(!_pContainer, "unotools.config",
            "OConfigurationValueContainer::registerExchangeLocation: invalid container location!");
        SAL_WARN_IF(!( (TypeClass_CHAR      ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_BOOLEAN   ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_BYTE      ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_SHORT     ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_LONG      ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_DOUBLE    ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_STRING    ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_SEQUENCE  ==  _rValueType.getTypeClass( ) )),
            "unotools.config",
            "OConfigurationValueContainer::registerExchangeLocation: invalid type!" );

        // build an accessor for this container
        NodeValueAccessor aNewAccessor( OUString::createFromAscii( _pRelativePath ) );
        aNewAccessor.bind( _pContainer, _rValueType );

        // insert it into our structure
        implRegisterExchangeLocation( aNewAccessor );
    }

    void OConfigurationValueContainer::read( )
    {
        std::for_each(
            m_pImpl->aAccessors.begin(),
            m_pImpl->aAccessors.end(),
            UpdateFromConfig( m_pImpl->aConfigRoot, m_pImpl->rMutex )
        );
    }

    void OConfigurationValueContainer::commit()
    {
        // write the current values in the exchange locations
        std::for_each(
            m_pImpl->aAccessors.begin(),
            m_pImpl->aAccessors.end(),
            UpdateToConfig( m_pImpl->aConfigRoot, m_pImpl->rMutex )
        );

        // commit the changes done
        m_pImpl->aConfigRoot.commit( );
    }

    void OConfigurationValueContainer::implRegisterExchangeLocation( const NodeValueAccessor& _rAccessor )
    {
        // some checks
        SAL_WARN_IF(m_pImpl->aConfigRoot.isValid() && !m_pImpl->aConfigRoot.hasByHierarchicalName(_rAccessor.getPath()),
            "unotools.config",
            "OConfigurationValueContainer::implRegisterExchangeLocation: invalid relative path!" );

        // another check (should be the first container for this node)
        SAL_WARN_IF(!(m_pImpl->aAccessors.end() == ::std::find(
            m_pImpl->aAccessors.begin(),
            m_pImpl->aAccessors.end(),
            _rAccessor)),
            "unotools.config",
            "OConfigurationValueContainer::implRegisterExchangeLocation: already registered a container for this subnode!" );

        // remember the accessor
        m_pImpl->aAccessors.push_back( _rAccessor );

        // and initially fill the value
        lcl_copyData( _rAccessor, m_pImpl->aConfigRoot.getNodeValue( _rAccessor.getPath() ), m_pImpl->rMutex );
    }

}   // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
