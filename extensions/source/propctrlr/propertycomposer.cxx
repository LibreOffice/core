/*************************************************************************
 *
 *  $RCSfile: propertycomposer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:10:39 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCOMPOSER_HXX
#include "propertycomposer.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPBROWSERUI_HXX
#include "propbrowserui.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <functional>
#include <algorithm>
#include <set>
#include <map>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        typedef ::std::set< Property, LessPropertyByHandle >    PropertyBag;

        //----------------------------------------------------------------
        /** STL-compatible operator which detrmines whether a given <type>IPropertyHandler</type>
            supports an UI descriptor for a given property
        */
        struct SupportsUIDescriptor : public ::std::unary_function< ::rtl::Reference< IPropertyHandler >, bool >
        {
            PropertyId nId;
            SupportsUIDescriptor( PropertyId _nId ) : nId( _nId ) { }
            bool operator()( const ::rtl::Reference< IPropertyHandler >& _rHandler )
            {
                return _rHandler->supportsUIDescriptor( nId );
            }
        };

        //----------------------------------------------------------------
        /** STL-compatible operator which determines whether a given <type>IPropertyHandler</type>
            supports a given property
        */
        struct SupportsProperty : public ::std::unary_function< ::rtl::Reference< IPropertyHandler >, bool >
        {
            PropertyId nId;
            SupportsProperty( PropertyId _nId ) : nId( _nId ) { }
            bool operator()( const ::rtl::Reference< IPropertyHandler >& _rHandler )
            {
                const ::std::vector< Property > aSupported( _rHandler->getSupportedProperties( ) );
                return aSupported.end() != ::std::find_if( aSupported.begin(), aSupported.end(), FindPropertyByHandle( nId ) );
            }
        };

        //----------------------------------------------------------------
        struct SetPropertyValue : public ::std::unary_function< ::rtl::Reference< IPropertyHandler >, void >
        {
            PropertyId  nId;
            const Any&  rValue;
            SetPropertyValue( PropertyId _nId, const Any& _rValue ) : nId( _nId ), rValue( _rValue ) { }
            void operator()( const ::rtl::Reference< IPropertyHandler >& _rHandler )
            {
                _rHandler->setPropertyValue( nId, rValue );
            }
        };

        //----------------------------------------------------------------
        template < class BagType >
        void putIntoBag( const ::std::vector< typename BagType::value_type >& _rArray, BagType& /* [out] */ _rBag )
        {
            ::std::copy( _rArray.begin(), _rArray.end(),
                ::std::insert_iterator< BagType >( _rBag, _rBag.begin() ) );
        }

        //----------------------------------------------------------------
        template < class BagType >
        void copyBagToArray( const BagType& /* [out] */ _rBag, ::std::vector< typename BagType::value_type >& _rArray )
        {
            _rArray.resize( _rBag.size() );
            ::std::copy( _rBag.begin(), _rBag.end(), _rArray.begin() );
        }

        //================================================================
        //= ComposedPropertyUIUpdate
        //================================================================
        class ComposedPropertyUIUpdate : public IPropertyBrowserUI
        {
        private:
            typedef ::std::map< ::rtl::OUString, bool >                         MapStringToBool;
            typedef ::std::map< ::rtl::OUString, ::std::pair< bool, bool > >    MapStringToBoolPair;
            typedef ::std::set< ::rtl::OUString >                               StringBag;
            typedef ::std::map< EPropertyCategory, bool >                       MapCategoryToBool;

        private:
            IPropertyBrowserUI*     m_pMasterUpdater;
            MapStringToBool         m_aEnabledProperties;
            MapStringToBoolPair     m_EnabledButtons;
            StringBag               m_aRebuiltProperties;
            MapStringToBool         m_aShownProperties;
            StringBag               m_aHiddenProperties;
            MapCategoryToBool       m_aShownCategories;

        public:
            ComposedPropertyUIUpdate( IPropertyBrowserUI* _pMasterUpdater );
            ~ComposedPropertyUIUpdate();

            // IPropertyBrowserUI overridables
            virtual void    enablePropertyUI( const ::rtl::OUString& _rPropertyName, bool _bEnable );
            virtual void    enablePropertyButtons( const ::rtl::OUString& _rPropertyName, bool _bEnablePrimary, bool _bEnableSecondary );
            virtual void    rebuildPropertyUI( const ::rtl::OUString& _rPropertyName );
            virtual void    showPropertyUI( const ::rtl::OUString& _rPropertyName, bool _bRefreshIfExistent );
            virtual void    hidePropertyUI( const ::rtl::OUString& _rPropertyName );
            virtual void    showCategory( EPropertyCategory _eCategory, bool _bShow );

        private:
            ComposedPropertyUIUpdate();     // never implemented
        };

        //----------------------------------------------------------------
        ComposedPropertyUIUpdate::ComposedPropertyUIUpdate( IPropertyBrowserUI* _pMasterUpdater )
            :m_pMasterUpdater( _pMasterUpdater )
        {
            OSL_ENSURE( m_pMasterUpdater, "ComposedPropertyUIUpdate::ComposedPropertyUIUpdate: whom should I forward this to?" );
        }

        //----------------------------------------------------------------
        ComposedPropertyUIUpdate::~ComposedPropertyUIUpdate( )
        {
            if ( m_pMasterUpdater )
            {
                // forward the collected requests to the master updater
                // --- enablePropertyUI
                for ( MapStringToBool::const_iterator loop = m_aEnabledProperties.begin();
                    loop != m_aEnabledProperties.end();
                    ++loop
                    )
                {
                    m_pMasterUpdater->enablePropertyUI( loop->first, loop->second );
                }
                // --- enablePropertyButtons
                for ( MapStringToBoolPair::const_iterator loop = m_EnabledButtons.begin();
                    loop != m_EnabledButtons.end();
                    ++loop
                    )
                {
                    m_pMasterUpdater->enablePropertyButtons( loop->first, loop->second.first, loop->second.second );
                }
                // --- rebuildPropertyUI
                for ( StringBag::const_iterator loop = m_aRebuiltProperties.begin();
                      loop != m_aRebuiltProperties.end();
                      ++loop
                    )
                {
                    m_pMasterUpdater->rebuildPropertyUI( *loop );
                }
                // --- showPropertyUI
                for ( MapStringToBool::const_iterator loop = m_aShownProperties.begin();
                    loop != m_aShownProperties.end();
                    ++loop
                    )
                {
                    if ( m_aHiddenProperties.find( loop->first ) == m_aHiddenProperties.end() )
                        m_pMasterUpdater->showPropertyUI( loop->first, loop->second );
                }
                // --- hidePropertyUI
                for ( StringBag::const_iterator loop = m_aHiddenProperties.begin();
                      loop != m_aHiddenProperties.end();
                      ++loop
                    )
                {
                    m_pMasterUpdater->hidePropertyUI( *loop );
                }
                // --- showCategory
                for ( MapCategoryToBool::const_iterator loop = m_aShownCategories.begin();
                      loop != m_aShownCategories.end();
                      ++loop
                    )
                {
                    m_pMasterUpdater->showCategory( loop->first, loop->second );
                }
            }
        }

        //----------------------------------------------------------------
        void ComposedPropertyUIUpdate::enablePropertyUI( const ::rtl::OUString& _rPropertyName, bool _bEnable )
        {
            MapStringToBool::iterator aPos = m_aEnabledProperties.find( _rPropertyName );
            if ( aPos == m_aEnabledProperties.end() )
            {
                // encountered this property for the first time -> only remember the flag for now
                m_aEnabledProperties[ _rPropertyName ] = _bEnable;
            }
            else
            {
                // already encountered this property before. The property is enabled if and only if
                // *all* of our callees say it should be.
                aPos->second &= _bEnable;
            }
        }

        //----------------------------------------------------------------
        void ComposedPropertyUIUpdate::enablePropertyButtons( const ::rtl::OUString& _rPropertyName, bool _bEnablePrimary, bool _bEnableSecondary )
        {
            MapStringToBoolPair::iterator aPos = m_EnabledButtons.find( _rPropertyName );
            if ( aPos == m_EnabledButtons.end() )
            {
                m_EnabledButtons[ _rPropertyName ] = MapStringToBoolPair::data_type( _bEnablePrimary, _bEnableSecondary );
            }
            else
            {
                aPos->second.first &= _bEnablePrimary;
                aPos->second.second &= _bEnableSecondary;
            }
        }

        //----------------------------------------------------------------
        void ComposedPropertyUIUpdate::rebuildPropertyUI( const ::rtl::OUString& _rPropertyName )
        {
            m_aRebuiltProperties.insert( _rPropertyName );
        }

        //----------------------------------------------------------------
        void ComposedPropertyUIUpdate::showPropertyUI( const ::rtl::OUString& _rPropertyName, bool _bRefreshIfExistent )
        {
            MapStringToBool::iterator aPos = m_aShownProperties.find( _rPropertyName );
            if ( aPos == m_aShownProperties.end() )
            {
                // encountered this property for the first time -> only remember the flag for now
                m_aShownProperties[ _rPropertyName ] = _bRefreshIfExistent;
            }
            else
            {
                // already encountered this property before. The UI needs to be refreshed if at least one of
                // our callees says so
                aPos->second |= _bRefreshIfExistent;
            }
        }

        //----------------------------------------------------------------
        void ComposedPropertyUIUpdate::hidePropertyUI( const ::rtl::OUString& _rPropertyName )
        {
            m_aHiddenProperties.insert( _rPropertyName );
        }

        //----------------------------------------------------------------
        void ComposedPropertyUIUpdate::showCategory( EPropertyCategory _eCategory, bool _bShow )
        {
            MapCategoryToBool::iterator aPos = m_aShownCategories.find( _eCategory );
            if ( aPos == m_aShownCategories.end() )
            {
                // encountered this category for the first time -> only remember the flag for now
                m_aShownCategories[ _eCategory ] = _bShow;
            }
            else
            {
                // already encountered this category before. The category must be shown if *all*
                // of our callees say so
                aPos->second &= _bShow;
            }
        }
    }

    //====================================================================
    //= PropertyComposer
    //====================================================================

    // TODO: there are various places where we determine the first handler in our array which
    // supports a given property id. This is, at the moment, done with searching all handlers,
    // which is O( n * k ) at worst (n being the number of handlers, k being the maximum number
    // of supported properties per handler). Shouldn't we cache this? So that it is O( log k )?

    //--------------------------------------------------------------------
    PropertyComposer::PropertyComposer( const ::std::vector< ::rtl::Reference< IPropertyHandler > >& _rSlaveHandlers )
        :m_aSlaveHandlers( _rSlaveHandlers          )
        ,m_pInfoService  ( new OPropertyInfoService )
        ,m_refCount      ( 0                        )
    {
#if OSL_DEBUG_LEVEL > 0
        for ( HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            OSL_ENSURE( loop->is(), "PropertyComposer::PropertyComposer: invalid slave handler (NULL)!" );
        }
#endif
        OSL_ENSURE( !m_aSlaveHandlers.empty(), "PropertyComposer::PropertyComposer: handler array must not be empty! This will crash!" );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL PropertyComposer::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL PropertyComposer::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
           delete this;
           return 0;
        }
        return m_refCount;
    }

    //--------------------------------------------------------------------
    bool SAL_CALL PropertyComposer::supportsUIDescriptor( PropertyId _nPropId ) const
    {
        // if at least one of our slaves does, we do, too
        return m_aSlaveHandlers.end() != ::std::find_if( m_aSlaveHandlers.begin(), m_aSlaveHandlers.end(), SupportsUIDescriptor( _nPropId ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyComposer::getPropertyValue( PropertyId _nPropId, bool _bLazy ) const
    {
        return m_aSlaveHandlers.empty() ? Any() : m_aSlaveHandlers[0]->getPropertyValue( _nPropId, _bLazy );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::setPropertyValue( PropertyId _nPropId, const Any& _rValue )
    {
        ::std::for_each( m_aSlaveHandlers.begin(), m_aSlaveHandlers.end(), SetPropertyValue( _nPropId, _rValue ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyComposer::getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const
    {
        return m_aSlaveHandlers.empty() ? Any() : m_aSlaveHandlers[0]->getPropertyValueFromStringRep( _nPropId, _rStringRep );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL PropertyComposer::getStringRepFromPropertyValue( PropertyId _nPropId, const Any& _rValue ) const
    {
        return m_aSlaveHandlers.empty() ? ::rtl::OUString() : m_aSlaveHandlers[0]->getStringRepFromPropertyValue( _nPropId, _rValue );
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL PropertyComposer::getPropertyState( PropertyId _nPropId ) const
    {
        if ( m_aSlaveHandlers.empty() )
            return PropertyState_DIRECT_VALUE;

        // assume DIRECT for the moment. This will stay this way if *all* slaves
        // tell the property has DIRECT state, and if *all* values equal
        PropertyState eState = PropertyState_DIRECT_VALUE;

        // check the master state
        ::rtl::Reference< IPropertyHandler > pPrimary( *m_aSlaveHandlers.begin() );
        Any aPrimaryValue = pPrimary->getPropertyValue( _nPropId );
        eState = pPrimary->getPropertyState( _nPropId );

        // loop through the secondary sets
        PropertyState eSecondaryState = PropertyState_DIRECT_VALUE;
        for ( HandlerArray::const_iterator loop = ( m_aSlaveHandlers.begin() + 1 );
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            // the secondary state
            eSecondaryState = (*loop)->getPropertyState( _nPropId );

            // the secondary value
            Any aSecondaryValue( (*loop)->getPropertyValue( _nPropId ) );

            if  (   ( PropertyState_AMBIGUOUS_VALUE == eSecondaryState )    // secondary is ambiguous
                ||  ( aPrimaryValue != aSecondaryValue )                    // unequal values
                )
            {
                eState = PropertyState_AMBIGUOUS_VALUE;
                break;
            }
        }

        return eState;
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::startAllPropertyChangeListening( const Reference< XPropertyChangeListener >& _rxListener )
    {
        // TODO: place your code here
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::stopAllPropertyChangeListening( )
    {
        // TODO: place your code here
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > SAL_CALL PropertyComposer::getSupportedProperties() const
    {
        OSL_ENSURE( !m_aSlaveHandlers.empty(), "PropertyComposer::getSupportedProperties: too less handlers!" );
        if ( m_aSlaveHandlers.empty() )
            return ::std::vector< Property >();

        // we support a property if and only if all of our slaves support it

        // initially, use all the properties of an arbitrary handler (we take the first one)
        PropertyBag aCadidatesForSupported;
        putIntoBag( (*m_aSlaveHandlers.begin())->getSupportedProperties(), aCadidatesForSupported );

        // now intersect with the properties of *all* other handlers
        for ( HandlerArray::const_iterator loop = ( m_aSlaveHandlers.begin() + 1 );
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            PropertyBag aThisRound;
            putIntoBag( (*loop)->getSupportedProperties(), aThisRound );

            PropertyBag aIntersection;
            ::std::set_intersection( aThisRound.begin(), aThisRound.end(), aCadidatesForSupported.begin(), aCadidatesForSupported.end(),
                ::std::insert_iterator< PropertyBag >( aIntersection, aIntersection.begin() ), LessPropertyByHandle() );

            aCadidatesForSupported = aIntersection;
            if ( aCadidatesForSupported.empty() )
                break;
        }

        ::std::vector< Property > aSurvived;
        copyBagToArray( aCadidatesForSupported, aSurvived );
        return aSurvived;
    }

    //--------------------------------------------------------------------
    void uniteStringArrays( const PropertyComposer::HandlerArray& _rHandlers, ::std::vector< ::rtl::OUString > (SAL_CALL IPropertyHandler::*pGetter)( void ) const,
        ::std::vector< ::rtl::OUString >& /* [out] */ _rUnion )
    {
        ::std::set< ::rtl::OUString > aUnitedBag;

        ::std::vector< ::rtl::OUString > aThisRound;
        for ( PropertyComposer::HandlerArray::const_iterator loop = _rHandlers.begin();
              loop != _rHandlers.end();
              ++loop
            )
        {
            aThisRound = (loop->get()->*pGetter)();
            putIntoBag( aThisRound, aUnitedBag );
        }

        copyBagToArray( aUnitedBag, _rUnion );
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL PropertyComposer::getSupersededProperties( ) const
    {
        // we supersede those properties which are superseded by at least one of our slaves
        ::std::vector< ::rtl::OUString > aSuperseded;
        uniteStringArrays( m_aSlaveHandlers, &IPropertyHandler::getSupersededProperties, aSuperseded );
        return aSuperseded;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL PropertyComposer::getActuatingProperties( ) const
    {
        // we're interested in those properties which at least one handler wants to have
        ::std::vector< ::rtl::OUString > aActuating;
        uniteStringArrays( m_aSlaveHandlers, &IPropertyHandler::getActuatingProperties, aActuating );
        return aActuating;
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const
    {
        if ( !m_aSlaveHandlers.empty() )
            m_aSlaveHandlers[0]->describePropertyUI( _nPropId, _rDescriptor );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater )
    {
        if ( !m_aSlaveHandlers.empty() )
        {
            ComposedPropertyUIUpdate aComposedUpdate( _pUpdater );

            for ( HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
                  loop != m_aSlaveHandlers.end();
                  ++loop
                )
                (*loop)->initializePropertyUI( _nPropId, &aComposedUpdate );
        }
    }

    //--------------------------------------------------------------------
    bool SAL_CALL PropertyComposer::requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, Any& _rData )
    {
        if ( m_aSlaveHandlers.empty() )
            return false;

        // just ask the first of the handlers
        return (*m_aSlaveHandlers.begin())->requestUserInputOnButtonClick( _nPropId, _bPrimary, _rData );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::executeButtonClick( PropertyId _nPropId, bool _bPrimary, const Any& _rData, IPropertyBrowserUI* _pUpdater )
    {
        if ( !m_aSlaveHandlers.empty() )
        {
            ComposedPropertyUIUpdate aComposedUpdate( _pUpdater );

            for ( HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
                  loop != m_aSlaveHandlers.end();
                  ++loop
                )
                (*loop)->executeButtonClick( _nPropId, _bPrimary, _rData, &aComposedUpdate );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::updateDependentProperties( PropertyId _nActuatingPropId, const Any& _rNewValue, const Any& _rOldValue, IPropertyBrowserUI* _pUpdater )
    {
        // ask all handlers which expressed interest in this particular property, and "compose" their
        // commands for the UIUpdater
        ComposedPropertyUIUpdate aComposedUpdate( _pUpdater );

        for ( HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            // TODO: make this cheaper (cache it?)
            const ::std::vector< ::rtl::OUString > aThisHandlersActuatingProps = (*loop)->getActuatingProperties();
            for ( ::std::vector< ::rtl::OUString >::const_iterator loopProps = aThisHandlersActuatingProps.begin();
                  loopProps != aThisHandlersActuatingProps.end();
                  ++loopProps
                )
            {
                if ( m_pInfoService->getPropertyId( *loopProps ) == _nActuatingPropId )
                {
                    (*loop)->updateDependentProperties( _nActuatingPropId, _rNewValue, _rOldValue, &aComposedUpdate );
                    break;
                }
            }
        }
    }

//........................................................................
} // namespace pcr
//........................................................................

