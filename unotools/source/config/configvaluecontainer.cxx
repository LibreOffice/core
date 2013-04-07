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

#include <unotools/configvaluecontainer.hxx>
#include <unotools/confignode.hxx>
#include <tools/debug.hxx>
#include <comphelper/stl_types.hxx>
#include <uno/data.h>
#include <algorithm>

#ifdef DBG_UTIL
#include <rtl/strbuf.hxx>
#endif

//.........................................................................
namespace utl
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= NodeValueAccessor
    //=====================================================================
    enum LocationType
    {
        ltSimplyObjectInstance,
        ltAnyInstance,

        ltUnbound
    };

    struct NodeValueAccessor
    {
    private:
        OUString     sRelativePath;      // the relative path of the node
        LocationType        eLocationType;      // the type of location where the value is stored
        void*               pLocation;          // the pointer to the location
        Type                aDataType;          // the type object pointed to by pLocation

    public:
        NodeValueAccessor( const OUString& _rNodePath );

        void bind( void* _pLocation, const Type& _rType );

        bool                    isBound( ) const        { return ( ltUnbound != eLocationType ) && ( NULL != pLocation ); }
        const OUString&  getPath( ) const        { return sRelativePath; }
        LocationType            getLocType( ) const     { return eLocationType; }
        void*                   getLocation( ) const    { return pLocation; }
        const Type&             getDataType( ) const    { return aDataType; }

        bool operator == ( const NodeValueAccessor& rhs ) const;
        bool operator != ( const NodeValueAccessor& rhs ) const { return !operator == ( rhs ); }
    };

    NodeValueAccessor::NodeValueAccessor( const OUString& _rNodePath )
        :sRelativePath( _rNodePath )
        ,eLocationType( ltUnbound )
        ,pLocation( NULL )
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
        DBG_ASSERT( !isBound(), "NodeValueAccessor::bind: already bound!" );

        eLocationType = ltSimplyObjectInstance;
        pLocation = _pLocation;
        aDataType = _rType;
    }

    #ifndef UNX
    static
    #endif
    void lcl_copyData( const NodeValueAccessor& _rAccessor, const Any& _rData, ::osl::Mutex& _rMutex )
    {
        ::osl::MutexGuard aGuard( _rMutex );

        DBG_ASSERT( _rAccessor.isBound(), "::utl::lcl_copyData: invalid accessor!" );
        switch ( _rAccessor.getLocType() )
        {
            case ltSimplyObjectInstance:
            {
                if ( _rData.hasValue() )
                {
#ifdef DBG_UTIL
                    sal_Bool bSuccess =
#endif
                    // assign the value
                    uno_type_assignData(
                        _rAccessor.getLocation(), _rAccessor.getDataType().getTypeLibType(),
                        const_cast< void* >( _rData.getValue() ), _rData.getValueType().getTypeLibType(),
                        (uno_QueryInterfaceFunc)cpp_queryInterface, (uno_AcquireFunc)cpp_acquire, (uno_ReleaseFunc)cpp_release
                    );
                    #ifdef DBG_UTIL
                    OStringBuffer aBuf( 256 );
                    aBuf.append("::utl::lcl_copyData( Accessor, Any ): could not assign the data (node path: ");
                    aBuf.append( OUStringToOString( _rAccessor.getPath(), RTL_TEXTENCODING_ASCII_US ) );
                    aBuf.append( " !" );
                    DBG_ASSERT( bSuccess, aBuf.getStr() );
                    #endif
                }
                else {
                    DBG_WARNING( "::utl::lcl_copyData: NULL value lost!" );
                }
            }
            break;
            case ltAnyInstance:
                // a simple assignment of an Any ...
                *static_cast< Any* >( _rAccessor.getLocation() ) = _rData;
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

        DBG_ASSERT( _rAccessor.isBound(), "::utl::lcl_copyData: invalid accessor!" );
        switch ( _rAccessor.getLocType() )
        {
            case ltSimplyObjectInstance:
                // a simple setValue ....
                _rData.setValue( _rAccessor.getLocation(), _rAccessor.getDataType() );
                break;

            case ltAnyInstance:
                // a simple assignment of an Any ...
                _rData = *static_cast< Any* >( _rAccessor.getLocation() );
                break;
            default:
                break;
        }
    }

    //=====================================================================
    //= functors on NodeValueAccessor instances
    //=====================================================================

    /// base class for functors syncronizing between exchange locations and config sub nodes
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

    DECLARE_STL_VECTOR( NodeValueAccessor, NodeValueAccessors );

    //=====================================================================
    //= OConfigurationValueContainerImpl
    //=====================================================================
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

    //=====================================================================
    //= OConfigurationValueContainer
    //=====================================================================

    OConfigurationValueContainer::OConfigurationValueContainer(
            const Reference< XComponentContext >& _rxORB, ::osl::Mutex& _rAccessSafety,
            const sal_Char* _pConfigLocation, const sal_uInt16 _nAccessFlags, const sal_Int32 _nLevels )
        :m_pImpl( new OConfigurationValueContainerImpl( _rxORB, _rAccessSafety ) )
    {
        implConstruct( OUString::createFromAscii( _pConfigLocation ), _nAccessFlags, _nLevels );
    }

    OConfigurationValueContainer::~OConfigurationValueContainer()
    {
        delete m_pImpl;
    }

    void OConfigurationValueContainer::implConstruct( const OUString& _rConfigLocation,
        const sal_uInt16 _nAccessFlags, const sal_Int32 _nLevels )
    {
        DBG_ASSERT( !m_pImpl->aConfigRoot.isValid(), "OConfigurationValueContainer::implConstruct: already initialized!" );

        // .................................
        // create the configuration node we're about to work with
        m_pImpl->aConfigRoot = OConfigurationTreeRoot::createWithComponentContext(
            m_pImpl->xORB,
            _rConfigLocation,
            _nLevels,
            ( _nAccessFlags & CVC_UPDATE_ACCESS ) ? OConfigurationTreeRoot::CM_UPDATABLE : OConfigurationTreeRoot::CM_READONLY,
            ( _nAccessFlags & CVC_IMMEDIATE_UPDATE ) ? sal_False : sal_True
        );
        #ifdef DBG_UTIL
        OStringBuffer aBuf(256);
        aBuf.append("Could not access the configuration node located at ");
        aBuf.append( OUStringToOString( _rConfigLocation, RTL_TEXTENCODING_ASCII_US ) );
        aBuf.append( " !" );
        DBG_ASSERT( m_pImpl->aConfigRoot.isValid(), aBuf.getStr() );
        #endif
    }

    void OConfigurationValueContainer::registerExchangeLocation( const sal_Char* _pRelativePath,
        void* _pContainer, const Type& _rValueType )
    {
        // checks ....
        DBG_ASSERT( _pContainer, "OConfigurationValueContainer::registerExchangeLocation: invalid container location!" );
        DBG_ASSERT( (   TypeClass_CHAR      ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_BOOLEAN   ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_BYTE      ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_SHORT     ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_LONG      ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_DOUBLE    ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_STRING    ==  _rValueType.getTypeClass( ) )
                ||  (   TypeClass_SEQUENCE  ==  _rValueType.getTypeClass( ) ),
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

    void OConfigurationValueContainer::write( sal_Bool _bCommit )
    {
        // collect the current values in the exchange locations
        std::for_each(
            m_pImpl->aAccessors.begin(),
            m_pImpl->aAccessors.end(),
            UpdateToConfig( m_pImpl->aConfigRoot, m_pImpl->rMutex )
        );

        // commit the changes done (if requested)
        if ( _bCommit )
            commit( sal_False );
    }

    void OConfigurationValueContainer::commit( sal_Bool _bWrite )
    {
        // write the current values in the exchange locations (if requested)
        if ( _bWrite )
            write( sal_False );

        // commit the changes done
        m_pImpl->aConfigRoot.commit( );
    }

    void OConfigurationValueContainer::implRegisterExchangeLocation( const NodeValueAccessor& _rAccessor )
    {
        // some checks
        DBG_ASSERT( !m_pImpl->aConfigRoot.isValid() || m_pImpl->aConfigRoot.hasByHierarchicalName( _rAccessor.getPath() ),
            "OConfigurationValueContainer::implRegisterExchangeLocation: invalid relative path!" );

#ifdef DBG_UTIL
        // another check (should be the first container for this node)
        ConstNodeValueAccessorsIterator aExistent = ::std::find(
            m_pImpl->aAccessors.begin(),
            m_pImpl->aAccessors.end(),
            _rAccessor
        );
        DBG_ASSERT( m_pImpl->aAccessors.end() == aExistent, "OConfigurationValueContainer::implRegisterExchangeLocation: already registered a container for this subnode!" );
#endif

        // remember the accessor
        m_pImpl->aAccessors.push_back( _rAccessor );

        // and initially fill the value
        lcl_copyData( _rAccessor, m_pImpl->aConfigRoot.getNodeValue( _rAccessor.getPath() ), m_pImpl->rMutex );
    }

//.........................................................................
}   // namespace utl
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
