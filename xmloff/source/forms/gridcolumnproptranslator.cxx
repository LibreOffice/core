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

#include "gridcolumnproptranslator.hxx"

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>

#include <algorithm>

namespace xmloff
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::style;

    namespace
    {
        OUString getParaAlignProperty()
        {
            return OUString( "ParaAdjust" );
        }

        OUString getAlignProperty()
        {
            return OUString( "Align" );
        }

        sal_Int32 findStringElement( const Sequence< OUString >& _rNames, const OUString& _rName )
        {
            const OUString* pStart = _rNames.getConstArray();
            const OUString* pEnd = _rNames.getConstArray() + _rNames.getLength();
            const OUString* pPos = ::std::find( pStart, pEnd, _rName );
            if ( pPos != pEnd )
                return pPos - pStart;
            return -1;
        }

        struct AlignmentTranslationEntry
        {
            ParagraphAdjust nParagraphValue;
            sal_Int16       nControlValue;
        }
        AlignmentTranslations[] =
        {
            // note that order matters:
            // valueAlignToParaAdjust and valueParaAdjustToAlign search this map from the _beginning_
            // and use the first matching entry
            { ParagraphAdjust_LEFT,             awt::TextAlign::LEFT     },
            { ParagraphAdjust_CENTER,           awt::TextAlign::CENTER   },
            { ParagraphAdjust_RIGHT,            awt::TextAlign::RIGHT    },
            { ParagraphAdjust_BLOCK,            awt::TextAlign::RIGHT    },
            { ParagraphAdjust_STRETCH,          awt::TextAlign::LEFT     },
            { ParagraphAdjust_MAKE_FIXED_SIZE,  awt::TextAlign::LEFT     },
            { ParagraphAdjust_MAKE_FIXED_SIZE,  -1 }
        };

        void valueAlignToParaAdjust(Any& rValue)
        {
            sal_Int16 nValue = 0;
            rValue >>= nValue;
            const AlignmentTranslationEntry* pTranslation = AlignmentTranslations;
            while (-1 != pTranslation->nControlValue)
            {
                if ( nValue == pTranslation->nControlValue )
                {
                    rValue <<= pTranslation->nParagraphValue;
                    return;
                }
                ++pTranslation;
            }
            OSL_FAIL( "valueAlignToParaAdjust: unreachable!" );
        }

        void valueParaAdjustToAlign(Any& rValue)
        {
            sal_Int32 nValue = 0;
            rValue >>= nValue;
            const AlignmentTranslationEntry* pTranslation = AlignmentTranslations;
            while ( ParagraphAdjust_MAKE_FIXED_SIZE != pTranslation->nParagraphValue)
            {
                if ( nValue == pTranslation->nParagraphValue)
                {
                    rValue <<= pTranslation->nControlValue;
                    return;
                }
                ++pTranslation;
            }
            OSL_FAIL( "valueParaAdjustToAlign: unreachable!" );
        }

        //= OMergedPropertySetInfo
        typedef ::cppu::WeakAggImplHelper   <   XPropertySetInfo
                                            >   OMergedPropertySetInfo_Base;
        class OMergedPropertySetInfo : public OMergedPropertySetInfo_Base
        {
        private:
            Reference< XPropertySetInfo >   m_xMasterInfo;

        public:
            explicit OMergedPropertySetInfo( const Reference< XPropertySetInfo >& _rxMasterInfo );

        protected:
            virtual ~OMergedPropertySetInfo();

            // XPropertySetInfo
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL getProperties(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual ::com::sun::star::beans::Property SAL_CALL getPropertyByName( const OUString& aName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
            virtual sal_Bool SAL_CALL hasPropertyByName( const OUString& Name ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        };

        OMergedPropertySetInfo::OMergedPropertySetInfo( const Reference< XPropertySetInfo >& _rxMasterInfo )
            :m_xMasterInfo( _rxMasterInfo )
        {
            OSL_ENSURE( m_xMasterInfo.is(), "OMergedPropertySetInfo::OMergedPropertySetInfo: hmm?" );
        }

        OMergedPropertySetInfo::~OMergedPropertySetInfo()
        {
        }

        Sequence< Property > SAL_CALL OMergedPropertySetInfo::getProperties(  ) throw (RuntimeException, std::exception)
        {
            // add a "ParaAdjust" property to the master properties
            Sequence< Property > aProperties;
            if ( m_xMasterInfo.is() )
                aProperties = m_xMasterInfo->getProperties();

            sal_Int32 nOldLength = aProperties.getLength();
            aProperties.realloc( nOldLength + 1 );
            aProperties[ nOldLength ] = getPropertyByName( getParaAlignProperty() );

            return aProperties;
        }

        Property SAL_CALL OMergedPropertySetInfo::getPropertyByName( const OUString& aName ) throw (UnknownPropertyException, RuntimeException, std::exception)
        {
            if ( aName == getParaAlignProperty() )
                return Property( getParaAlignProperty(), -1,
                    ::cppu::UnoType<ParagraphAdjust>::get(), 0 );

            if ( !m_xMasterInfo.is() )
                return Property();

            return m_xMasterInfo->getPropertyByName( aName );
        }

        sal_Bool SAL_CALL OMergedPropertySetInfo::hasPropertyByName( const OUString& Name ) throw (RuntimeException, std::exception)
        {
            if ( Name == getParaAlignProperty() )
                return sal_True;

            if ( !m_xMasterInfo.is() )
                return sal_False;

            return m_xMasterInfo->hasPropertyByName( Name );
        }
    }

    //= OGridColumnPropertyTranslator
    OGridColumnPropertyTranslator::OGridColumnPropertyTranslator( const Reference< XMultiPropertySet >& _rxGridColumn )
        :m_xGridColumn( _rxGridColumn )
    {
        OSL_ENSURE( m_xGridColumn.is(), "OGridColumnPropertyTranslator: invalid grid column!" );
    }

    OGridColumnPropertyTranslator::~OGridColumnPropertyTranslator()
    {
    }

    Reference< XPropertySetInfo > SAL_CALL OGridColumnPropertyTranslator::getPropertySetInfo(  ) throw (RuntimeException, std::exception)
    {
        Reference< XPropertySetInfo > xColumnPropInfo;
        if ( m_xGridColumn.is() )
            xColumnPropInfo = m_xGridColumn->getPropertySetInfo();
        return new OMergedPropertySetInfo( xColumnPropInfo );
    }

    void SAL_CALL OGridColumnPropertyTranslator::setPropertyValue( const OUString& _rPropertyName, const Any& aValue ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
    {
        // we implement this by delegating it to setPropertyValues, which is to ignore unknown properties. On the other hand, our
        // contract requires us to throw a UnknownPropertyException for unknown properties, so check this first.

        if ( !getPropertySetInfo()->hasPropertyByName( _rPropertyName ) )
            throw UnknownPropertyException( _rPropertyName, *this );

        Sequence< OUString > aNames( &_rPropertyName, 1 );
        Sequence< Any >             aValues( &aValue, 1 );
        setPropertyValues( aNames, aValues );
    }

    Any SAL_CALL OGridColumnPropertyTranslator::getPropertyValue( const OUString& PropertyName ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
    {
        Sequence< OUString > aNames( &PropertyName, 1 );
        Sequence< Any > aValues = getPropertyValues( aNames );
        OSL_ENSURE( aValues.getLength() == 1, "OGridColumnPropertyTranslator::getPropertyValue: nonsense!" );
        if ( aValues.getLength() == 1 )
            return aValues[0];
        return Any();
    }

    void SAL_CALL OGridColumnPropertyTranslator::addPropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
    {
        OSL_FAIL( "OGridColumnPropertyTranslator::addPropertyChangeListener: not implemented - this should not be needed!" );
    }

    void SAL_CALL OGridColumnPropertyTranslator::removePropertyChangeListener( const OUString&, const Reference< XPropertyChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
    {
        OSL_FAIL( "OGridColumnPropertyTranslator::removePropertyChangeListener: not implemented - this should not be needed!" );
    }

    void SAL_CALL OGridColumnPropertyTranslator::addVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
    {
        OSL_FAIL( "OGridColumnPropertyTranslator::addVetoableChangeListener: not implemented - this should not be needed!" );
    }

    void SAL_CALL OGridColumnPropertyTranslator::removeVetoableChangeListener( const OUString&, const Reference< XVetoableChangeListener >& ) throw (UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
    {
        OSL_FAIL( "OGridColumnPropertyTranslator::removeVetoableChangeListener: not implemented - this should not be needed!" );
    }

    void SAL_CALL OGridColumnPropertyTranslator::setPropertyValues( const Sequence< OUString >& aPropertyNames, const Sequence< Any >& aValues ) throw (PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
    {
        if ( !m_xGridColumn.is() )
            return;

        // if there's ever the need for more than one property being translated, then we should
        // certainly have a more clever implementation than this ...

        Sequence< OUString > aTranslatedNames( aPropertyNames );
        Sequence< Any >             aTranslatedValues( aValues );

        sal_Int32 nParaAlignPos = findStringElement( aTranslatedNames, getParaAlignProperty() );
        if ( nParaAlignPos != -1 )
        {
            aTranslatedNames[ nParaAlignPos ] = getAlignProperty();
            valueParaAdjustToAlign( aTranslatedValues[ nParaAlignPos ] );
        }

        m_xGridColumn->setPropertyValues( aTranslatedNames, aTranslatedValues );
    }

    Sequence< Any > SAL_CALL OGridColumnPropertyTranslator::getPropertyValues( const Sequence< OUString >& aPropertyNames ) throw (RuntimeException, std::exception)
    {
        Sequence< Any > aValues( aPropertyNames.getLength() );
        if ( !m_xGridColumn.is() )
            return aValues;

        Sequence< OUString > aTranslatedNames( aPropertyNames );
        sal_Int32 nAlignPos = findStringElement( aTranslatedNames, getParaAlignProperty() );
        if ( nAlignPos != -1 )
            aTranslatedNames[ nAlignPos ] = getAlignProperty();

        aValues = m_xGridColumn->getPropertyValues( aPropertyNames );
        if ( nAlignPos != -1 )
            valueAlignToParaAdjust( aValues[ nAlignPos ] );

        return aValues;
    }

    void SAL_CALL OGridColumnPropertyTranslator::addPropertiesChangeListener( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& ) throw (RuntimeException, std::exception)
    {
        OSL_FAIL( "OGridColumnPropertyTranslator::addPropertiesChangeListener: not implemented - this should not be needed!" );
    }

    void SAL_CALL OGridColumnPropertyTranslator::removePropertiesChangeListener( const Reference< XPropertiesChangeListener >& ) throw (RuntimeException, std::exception)
    {
        OSL_FAIL( "OGridColumnPropertyTranslator::removePropertiesChangeListener: not implemented - this should not be needed!" );
    }

    void SAL_CALL OGridColumnPropertyTranslator::firePropertiesChangeEvent( const Sequence< OUString >&, const Reference< XPropertiesChangeListener >& ) throw (RuntimeException, std::exception)
    {
        OSL_FAIL( "OGridColumnPropertyTranslator::firePropertiesChangeEvent: not implemented - this should not be needed!" );
    }

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
