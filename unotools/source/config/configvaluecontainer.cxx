/*************************************************************************
 *
 *  $RCSfile: configvaluecontainer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-21 12:53:30 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef UNOTOOLS_CONFIGVALUECONTAINER_HXX
#include <unotools/configvaluecontainer.hxx>
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
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

    static void lcl_copyData( const NodeValueAccessor& _rAccessor, const Any& _rData, ::osl::Mutex& _rMutex )
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
                        cpp_queryInterface, cpp_acquire, cpp_release
                    );
                    DBG_ASSERT( bSuccess,
                        (   ::rtl::OString( "::utl::lcl_copyData( Accessor, Any ): could not assign the data (node path: " )
                        +=  ::rtl::OString( _rAccessor.getPath().getStr(), _rAccessor.getPath().getLength(), RTL_TEXTENCODING_ASCII_US )
                        +=  ::rtl::OString( " !" )
                        ).getStr()
                    );
                }
                else
                    DBG_WARNING( "::utl::lcl_copyData: NULL value lost!" );
            }
            break;
            case ltAnyInstance:
                // a simple assignment of an Any ...
                *static_cast< Any* >( _rAccessor.getLocation() ) = _rData;
                break;
        }
    }

    //---------------------------------------------------------------------
    //--- 21.08.01 12:06:43 -----------------------------------------------

    static void lcl_copyData( Any& _rData, const NodeValueAccessor& _rAccessor, ::osl::Mutex& _rMutex )
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
            lcl_copyData( _rAccessor, m_rRootNode.getNodeValue( _rAccessor.getPath( ) ), m_rMutex );
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
            ( _nAccessFlags & CVC_UPDATE_ACCESS ) ? OConfigurationTreeRoot::CM_PREFER_UPDATABLE : OConfigurationTreeRoot::CM_READONLY,
            ( _nAccessFlags & CVC_IMMEDIATE_UPDATE ) ? sal_False : sal_True
        );

        DBG_ASSERT( m_pImpl->aConfigRoot.isValid(),
                (   ::rtl::OString( "Could not access the configuration node located at " )
                +=  ::rtl::OString( _rConfigLocation.getStr(), _rConfigLocation.getLength(), RTL_TEXTENCODING_ASCII_US )
                +=  ::rtl::OString( " !" )
                ).getStr()
        );
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
        for_each(
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
        for_each(
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

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 20.08.01 15:47:36  fs
 ************************************************************************/

