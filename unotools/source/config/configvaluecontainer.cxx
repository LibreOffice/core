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
#include "precompiled_unotools.hxx"
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
        ::rtl::OUString     sRelativePath;      // the relative path of the node
        LocationType        eLocationType;      // the type of location where the value is stored
        void*               pLocation;          // the pointer to the location
        Type                aDataType;          // the type object pointed to by pLocation

    public:
        NodeValueAccessor( const ::rtl::OUString& _rNodePath );

        void bind( void* _pLocation, const Type& _rType );
        void bind( Any* _pLocation );

        bool                    isBound( ) const        { return ( ltUnbound != eLocationType ) && ( NULL != pLocation ); }
        const ::rtl::OUString&  getPath( ) const        { return sRelativePath; }
        LocationType            getLocType( ) const     { return eLocationType; }
        void*                   getLocation( ) const    { return pLocation; }
        const Type&             getDataType( ) const    { return aDataType; }

        bool operator == ( const NodeValueAccessor& rhs ) const;
        bool operator != ( const NodeValueAccessor& rhs ) const { return !operator == ( rhs ); }
    };

    //---------------------------------------------------------------------
    //--- 20.08.01 17:21:13 -----------------------------------------------

    NodeValueAccessor::NodeValueAccessor( const ::rtl::OUString& _rNodePath )
        :sRelativePath( _rNodePath )
        ,eLocationType( ltUnbound )
        ,pLocation( NULL )
    {
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 17:06:36 -----------------------------------------------

    bool NodeValueAccessor::operator == ( const NodeValueAccessor& rhs ) const
    {
        return  (   sRelativePath   ==  rhs.sRelativePath   )
            &&  (   eLocationType   ==  rhs.eLocationType   )
            &&  (   pLocation       ==  rhs.pLocation       );
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 17:47:43 -----------------------------------------------

    void NodeValueAccessor::bind( void* _pLocation, const Type& _rType )
    {
        DBG_ASSERT( !isBound(), "NodeValueAccessor::bind: already bound!" );

        eLocationType = ltSimplyObjectInstance;
        pLocation = _pLocation;
        aDataType = _rType;
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 17:48:47 -----------------------------------------------

    void NodeValueAccessor::bind( Any* _pLocation )
    {
        DBG_ASSERT( !isBound(), "NodeValueAccessor::bind: already bound!" );

        eLocationType = ltAnyInstance;
        pLocation = _pLocation;
        aDataType = ::getCppuType( _pLocation );
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 17:42:17 -----------------------------------------------

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
                    rtl::OStringBuffer aBuf( 256 );
                    aBuf.append("::utl::lcl_copyData( Accessor, Any ): could not assign the data (node path: ");
                    aBuf.append( rtl::OUStringToOString( _rAccessor.getPath(), RTL_TEXTENCODING_ASCII_US ) );
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

    //---------------------------------------------------------------------
    //--- 21.08.01 12:06:43 -----------------------------------------------

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

    //---------------------------------------------------------------------
    //--- 21.08.01 12:01:16 -----------------------------------------------

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

    //---------------------------------------------------------------------
    //--- 21.08.01 11:25:56 -----------------------------------------------

    struct UpdateFromConfig : public SubNodeAccess
    {
    public:
        UpdateFromConfig( const OConfigurationNode& _rRootNode, ::osl::Mutex& _rMutex ) : SubNodeAccess( _rRootNode, _rMutex ) { }

        void operator() ( NodeValueAccessor& _rAccessor )
        {
            ::utl::lcl_copyData( _rAccessor, m_rRootNode.getNodeValue( _rAccessor.getPath( ) ), m_rMutex );
        }
    };

    //---------------------------------------------------------------------
    //--- 21.08.01 11:25:56 -----------------------------------------------

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

    //---------------------------------------------------------------------
    //--- 20.08.01 16:58:24 -----------------------------------------------

    DECLARE_STL_VECTOR( NodeValueAccessor, NodeValueAccessors );

    //=====================================================================
    //= OConfigurationValueContainerImpl
    //=====================================================================
    struct OConfigurationValueContainerImpl
    {
        Reference< XMultiServiceFactory >       xORB;           // the service factory
        ::osl::Mutex&                           rMutex;         // the mutex for accessing the data containers
        OConfigurationTreeRoot                  aConfigRoot;    // the configuration node we're accessing

        NodeValueAccessors                      aAccessors;     // the accessors to the node values

        OConfigurationValueContainerImpl( const Reference< XMultiServiceFactory >& _rxORB, ::osl::Mutex& _rMutex )
            :xORB( _rxORB )
            ,rMutex( _rMutex )
        {
        }
    };

    //=====================================================================
    //= OConfigurationValueContainer
    //=====================================================================

    //---------------------------------------------------------------------
    //--- 20.08.01 15:53:35 -----------------------------------------------

    OConfigurationValueContainer::OConfigurationValueContainer(
            const Reference< XMultiServiceFactory >& _rxORB, ::osl::Mutex& _rAccessSafety,
            const sal_Char* _pConfigLocation, const sal_uInt16 _nAccessFlags, const sal_Int32 _nLevels )
        :m_pImpl( new OConfigurationValueContainerImpl( _rxORB, _rAccessSafety ) )
    {
        implConstruct( ::rtl::OUString::createFromAscii( _pConfigLocation ), _nAccessFlags, _nLevels );
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 15:55:20 -----------------------------------------------

    OConfigurationValueContainer::OConfigurationValueContainer(
            const Reference< XMultiServiceFactory >& _rxORB, ::osl::Mutex& _rAccessSafety,
            const ::rtl::OUString& _rConfigLocation, const sal_uInt16 _nAccessFlags, const sal_Int32 _nLevels )
        :m_pImpl( new OConfigurationValueContainerImpl( _rxORB, _rAccessSafety ) )
    {
        implConstruct( _rConfigLocation, _nAccessFlags, _nLevels );
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 16:01:29 -----------------------------------------------

    OConfigurationValueContainer::~OConfigurationValueContainer()
    {
        delete m_pImpl;
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 15:59:13 -----------------------------------------------

    const Reference< XMultiServiceFactory >& OConfigurationValueContainer::getServiceFactory( ) const
    {
        return m_pImpl->xORB;
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 16:02:07 -----------------------------------------------

    void OConfigurationValueContainer::implConstruct( const ::rtl::OUString& _rConfigLocation,
        const sal_uInt16 _nAccessFlags, const sal_Int32 _nLevels )
    {
        DBG_ASSERT( !m_pImpl->aConfigRoot.isValid(), "OConfigurationValueContainer::implConstruct: already initialized!" );

        // .................................
        // create the configuration node we're about to work with
        m_pImpl->aConfigRoot = OConfigurationTreeRoot::createWithServiceFactory(
            m_pImpl->xORB,
            _rConfigLocation,
            _nLevels,
            ( _nAccessFlags & CVC_UPDATE_ACCESS ) ? OConfigurationTreeRoot::CM_UPDATABLE : OConfigurationTreeRoot::CM_READONLY,
            ( _nAccessFlags & CVC_IMMEDIATE_UPDATE ) ? sal_False : sal_True
        );
        #ifdef DBG_UTIL
        rtl::OStringBuffer aBuf(256);
        aBuf.append("Could not access the configuration node located at ");
        aBuf.append( rtl::OUStringToOString( _rConfigLocation, RTL_TEXTENCODING_ASCII_US ) );
        aBuf.append( " !" );
        DBG_ASSERT( m_pImpl->aConfigRoot.isValid(), aBuf.getStr() );
        #endif
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 16:39:05 -----------------------------------------------

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
        NodeValueAccessor aNewAccessor( ::rtl::OUString::createFromAscii( _pRelativePath ) );
        aNewAccessor.bind( _pContainer, _rValueType );

        // insert it into our structure
        implRegisterExchangeLocation( aNewAccessor );
    }

    //---------------------------------------------------------------------
    //--- 21.08.01 14:44:45 -----------------------------------------------

    void OConfigurationValueContainer::registerNullValueExchangeLocation( const sal_Char* _pRelativePath, Any* _pContainer )
    {
        // build an accessor for this container
        NodeValueAccessor aNewAccessor( ::rtl::OUString::createFromAscii( _pRelativePath ) );
        aNewAccessor.bind( _pContainer );

        // insert it into our structure
        implRegisterExchangeLocation( aNewAccessor );
    }

    //---------------------------------------------------------------------
    //--- 21.08.01 10:23:34 -----------------------------------------------

    void OConfigurationValueContainer::read( )
    {
        std::for_each(
            m_pImpl->aAccessors.begin(),
            m_pImpl->aAccessors.end(),
            UpdateFromConfig( m_pImpl->aConfigRoot, m_pImpl->rMutex )
        );
    }

    //---------------------------------------------------------------------
    //--- 21.08.01 12:04:48 -----------------------------------------------

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

    //---------------------------------------------------------------------
    //--- 21.08.01 12:09:45 -----------------------------------------------

    void OConfigurationValueContainer::commit( sal_Bool _bWrite )
    {
        // write the current values in the exchange locations (if requested)
        if ( _bWrite )
            write( sal_False );

        // commit the changes done
        m_pImpl->aConfigRoot.commit( );
    }

    //---------------------------------------------------------------------
    //--- 20.08.01 17:29:27 -----------------------------------------------

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
