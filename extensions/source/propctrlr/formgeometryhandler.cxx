/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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


#include "propertyhandler.hxx"
#include "formmetadata.hxx"
#include "formstrings.hxx"
#include "handlerhelper.hxx"
#include "cellbindinghelper.hxx"

#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/container/XMap.hpp>
#include <com/sun/star/inspection/XNumericControl.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>

#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/componentbase.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace pcr
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::UnknownPropertyException;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::awt::XControlModel;
    using ::com::sun::star::drawing::XControlShape;
    using ::com::sun::star::container::XMap;
    using ::com::sun::star::inspection::LineDescriptor;
    using ::com::sun::star::inspection::XPropertyControlFactory;
    using ::com::sun::star::lang::NullPointerException;
    using ::com::sun::star::beans::Optional;
    using ::com::sun::star::inspection::XNumericControl;
    using ::com::sun::star::drawing::XShape;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::text::TextContentAnchorType;
    using ::com::sun::star::text::TextContentAnchorType_AT_PARAGRAPH;
    using ::com::sun::star::text::TextContentAnchorType_AS_CHARACTER;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::inspection::XObjectInspectorUI;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::sheet::XSpreadsheet;
    using ::com::sun::star::table::XColumnRowRange;
    using ::com::sun::star::table::XTableColumns;
    using ::com::sun::star::table::XTableRows;
    using ::com::sun::star::table::XCellRange;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::form::XGridColumnFactory;
    /** === end UNO using === **/
    namespace MeasureUnit = ::com::sun::star::util::MeasureUnit;

    typedef ::com::sun::star::awt::Point    AwtPoint;
    typedef ::com::sun::star::awt::Size     AwtSize;

    #define ANCHOR_TO_SHEET 0
    #define ANCHOR_TO_CELL  1

    //====================================================================
    //= BroadcastHelperBase
    //====================================================================
    class BroadcastHelperBase
    {
    protected:
        BroadcastHelperBase( ::osl::Mutex& _rMutex )
            :maBHelper( _rMutex )
        {
        }

    protected:
        ::cppu::OBroadcastHelper& getBroadcastHelper() { return maBHelper; }

    private:
        ::cppu::OBroadcastHelper    maBHelper;
    };

    //====================================================================
    //= ShapeGeometryChangeNotifier - declaration
    //====================================================================
    /** helper class to work around the ...unfortunate implementation of property change broadcasts
        in the XShape implementation, which broadcasts way too generous and unspecified
    */
    typedef ::comphelper::ComponentBase ShapeGeometryChangeNotifier_CBase;
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::beans::XPropertyChangeListener
                                    >   ShapeGeometryChangeNotifier_IBase;

    class ShapeGeometryChangeNotifier   :public BroadcastHelperBase
                                        ,public ShapeGeometryChangeNotifier_CBase
                                        ,public ShapeGeometryChangeNotifier_IBase
    {
    public:
        ShapeGeometryChangeNotifier( ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rParentMutex, const Reference< XShape >& _shape )
            :BroadcastHelperBase( _rParentMutex )
            ,ShapeGeometryChangeNotifier_CBase( BroadcastHelperBase::getBroadcastHelper(), ::comphelper::ComponentBase::NoInitializationNeeded() )
            ,ShapeGeometryChangeNotifier_IBase()
            ,m_rParent( _rParent )
            ,m_aPropertyChangeListeners( _rParentMutex )
            ,m_xShape( _shape )
        {
            ENSURE_OR_THROW( m_xShape.is(), "illegal shape!" );
            impl_init_nothrow();
        }

        // property change broadcasting
        void addPropertyChangeListener( const Reference< XPropertyChangeListener >& _listener )
        {
            m_aPropertyChangeListeners.addInterface( _listener );
        }
        void removePropertyChangeListener( const Reference< XPropertyChangeListener >& _listener )
        {
            m_aPropertyChangeListeners.removeInterface( _listener );
        }

        // XComponent equivalent
        void dispose()
        {
            ::osl::MutexGuard aGuard( getMutex() );
            impl_dispose_nothrow();
        }

        // XInterface
        virtual void SAL_CALL acquire(  ) throw ()
        {
            m_rParent.acquire();
        }

        virtual void SAL_CALL release(  ) throw ()
        {
            m_rParent.release();
        }

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const PropertyChangeEvent& _event ) throw (RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const EventObject& _event ) throw (RuntimeException);

    protected:
        virtual ~ShapeGeometryChangeNotifier()
        {
            if ( !getBroadcastHelper().bDisposed )
            {
                acquire();
                dispose();
            }
        }

    protected:
        ::cppu::OBroadcastHelper& getBroadcastHelper() { return BroadcastHelperBase::getBroadcastHelper(); }

    private:
        void    impl_init_nothrow();
        void    impl_dispose_nothrow();

    private:
        ::cppu::OWeakObject&                m_rParent;
        ::cppu::OInterfaceContainerHelper   m_aPropertyChangeListeners;
        Reference< XShape >                 m_xShape;
    };

    //====================================================================
    //= FormGeometryHandler - declaration
    //====================================================================
    class FormGeometryHandler;
    typedef HandlerComponentBase< FormGeometryHandler > FormGeometryHandler_Base;
    /** a property handler for any virtual string properties
    */
    class FormGeometryHandler : public FormGeometryHandler_Base
    {
    public:
        FormGeometryHandler(
            const Reference< XComponentContext >& _rxContext
        );

        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw (RuntimeException);
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw (RuntimeException);

    protected:
        ~FormGeometryHandler();

    protected:
        // XPropertyHandler overriables
        virtual Any                         SAL_CALL getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException);
        virtual void                        SAL_CALL setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException);
        virtual LineDescriptor              SAL_CALL describePropertyLine( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControlFactory >& _rxControlFactory ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::NullPointerException, ::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void                        SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getActuatingProperties( ) throw (RuntimeException);
        virtual void                        SAL_CALL actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& _rOldValue, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (NullPointerException, RuntimeException);

        // OComponentHandler overridables
        virtual void SAL_CALL disposing();

        // PropertyHandler overridables
        virtual Sequence< Property >        SAL_CALL doDescribeSupportedProperties() const;

    protected:
        virtual void onNewComponent();

    private:
        bool    impl_haveTextAnchorType_nothrow() const;
        bool    impl_haveSheetAnchorType_nothrow() const;
        void    impl_setSheetAnchorType_nothrow( const sal_Int32 _nAnchorType ) const;

    private:
        Reference< XControlShape >                      m_xAssociatedShape;
        Reference< XPropertySet >                       m_xShapeProperties;
        ::rtl::Reference< ShapeGeometryChangeNotifier > m_xChangeNotifier;
    };

    //====================================================================
    //= FormGeometryHandler - implementation
    //====================================================================
    DBG_NAME( FormGeometryHandler )
    //--------------------------------------------------------------------
    FormGeometryHandler::FormGeometryHandler( const Reference< XComponentContext >& _rxContext )
        :FormGeometryHandler_Base( _rxContext )
    {
        DBG_CTOR( FormGeometryHandler, NULL );
    }

    //--------------------------------------------------------------------
    FormGeometryHandler::~FormGeometryHandler( )
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();
            dispose();
        }

        DBG_DTOR( FormGeometryHandler, NULL );
    }

    //--------------------------------------------------------------------
    void FormGeometryHandler::onNewComponent()
    {
        if ( m_xChangeNotifier.is() )
        {
            m_xChangeNotifier->dispose();
            m_xChangeNotifier.clear();
        }
        m_xAssociatedShape.clear();
        m_xShapeProperties.clear();

        FormGeometryHandler_Base::onNewComponent();

        try
        {
            Reference< XControlModel > xControlModel( m_xComponent, UNO_QUERY );
            if ( xControlModel.is() )
            {
                // do not ask the map for shapes for grid control columns ....
                Reference< XChild > xCompChild( m_xComponent, UNO_QUERY_THROW );
                Reference< XGridColumnFactory > xCheckGrid( xCompChild->getParent(), UNO_QUERY );
                if ( !xCheckGrid.is() )
                {
                    Reference< XMap > xControlMap( m_aContext.getContextValueByAsciiName( "ControlShapeAccess" ), UNO_QUERY_THROW );
                    m_xAssociatedShape.set( xControlMap->get( makeAny( xControlModel ) ), UNO_QUERY_THROW );
                    m_xShapeProperties.set( m_xAssociatedShape, UNO_QUERY_THROW );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( m_xAssociatedShape.is() )
            m_xChangeNotifier = new ShapeGeometryChangeNotifier( *this, m_aMutex, m_xAssociatedShape.get() );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL FormGeometryHandler::getImplementationName_static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.extensions.FormGeometryHandler" ) );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL FormGeometryHandler::getSupportedServiceNames_static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( 1 );
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.inspection.FormGeometryHandler" ) );
        return aSupported;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL FormGeometryHandler::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        ENSURE_OR_THROW2( m_xAssociatedShape.is(), "internal error: properties, but no shape!", *this );
        ENSURE_OR_THROW2( m_xShapeProperties.is(), "internal error: no shape properties!", *this );

        Any aReturn;
        try
        {
            switch ( nPropId )
            {
            case PROPERTY_ID_POSITIONX:
                aReturn <<= m_xAssociatedShape->getPosition().X;
                break;
            case PROPERTY_ID_POSITIONY:
                aReturn <<= m_xAssociatedShape->getPosition().Y;
                break;
            case PROPERTY_ID_WIDTH:
                aReturn <<= m_xAssociatedShape->getSize().Width;
                break;
            case PROPERTY_ID_HEIGHT:
                aReturn <<= m_xAssociatedShape->getSize().Height;
                break;
            case PROPERTY_ID_TEXT_ANCHOR_TYPE:
                aReturn = m_xShapeProperties->getPropertyValue( PROPERTY_ANCHOR_TYPE );
                OSL_ENSURE( aReturn.hasValue(), "FormGeometryHandler::getPropertyValue: illegal anchor type!" );
                break;
            case PROPERTY_ID_SHEET_ANCHOR_TYPE:
            {
                Reference< XSpreadsheet > xAnchorSheet( m_xShapeProperties->getPropertyValue( PROPERTY_ANCHOR ), UNO_QUERY );
                aReturn <<= sal_Int32( xAnchorSheet.is() ? ANCHOR_TO_SHEET : ANCHOR_TO_CELL );
            }
            break;

            default:
                OSL_FAIL( "FormGeometryHandler::getPropertyValue: huh?" );
                break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormGeometryHandler::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        ENSURE_OR_THROW2( m_xAssociatedShape.is(), "internal error: properties, but no shape!", *this );
        ENSURE_OR_THROW2( m_xShapeProperties.is(), "internal error: properties, but no shape!", *this );

        try
        {
            switch ( nPropId )
            {
            case PROPERTY_ID_POSITIONX:
            case PROPERTY_ID_POSITIONY:
            {
                sal_Int32 nPosition(0);
                OSL_VERIFY( _rValue >>= nPosition );

                AwtPoint aPos( m_xAssociatedShape->getPosition() );
                if ( nPropId == PROPERTY_ID_POSITIONX )
                    aPos.X = nPosition;
                else
                    aPos.Y = nPosition;
                m_xAssociatedShape->setPosition( aPos );
            }
            break;

            case PROPERTY_ID_WIDTH:
            case PROPERTY_ID_HEIGHT:
            {
                sal_Int32 nSize(0);
                OSL_VERIFY( _rValue >>= nSize );

                AwtSize aSize( m_xAssociatedShape->getSize() );
                if ( nPropId == PROPERTY_ID_WIDTH )
                    aSize.Width = nSize;
                else
                    aSize.Height = nSize;
                m_xAssociatedShape->setSize( aSize );
            }
            break;

            case PROPERTY_ID_TEXT_ANCHOR_TYPE:
                m_xShapeProperties->setPropertyValue( PROPERTY_ANCHOR_TYPE, _rValue );
                break;

            case PROPERTY_ID_SHEET_ANCHOR_TYPE:
            {
                sal_Int32 nSheetAnchorType = 0;
                OSL_VERIFY( _rValue >>= nSheetAnchorType );
                impl_setSheetAnchorType_nothrow( nSheetAnchorType );
            }
            break;

            default:
                OSL_FAIL( "FormGeometryHandler::getPropertyValue: huh?" );
                break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    LineDescriptor SAL_CALL FormGeometryHandler::describePropertyLine( const ::rtl::OUString& _rPropertyName,
            const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nPropId( impl_getPropertyId_throw( _rPropertyName ) );

        LineDescriptor aLineDesc( PropertyHandler::describePropertyLine( _rPropertyName, _rxControlFactory ) );
        try
        {
            bool bIsSize = false;
            switch ( nPropId )
            {
            case PROPERTY_ID_WIDTH:
            case PROPERTY_ID_HEIGHT:
                bIsSize = true;
                // NO break!
            case PROPERTY_ID_POSITIONX:
            case PROPERTY_ID_POSITIONY:
            {
                Optional< double > aZero( sal_True, 0 );
                Optional< double > aValueNotPresent( sal_False, 0 );
                aLineDesc.Control = PropertyHandlerHelper::createNumericControl(
                    _rxControlFactory, 2, bIsSize ? aZero : aValueNotPresent, aValueNotPresent, sal_False );

                Reference< XNumericControl > xNumericControl( aLineDesc.Control, UNO_QUERY_THROW );
                xNumericControl->setValueUnit( MeasureUnit::MM_100TH );
                xNumericControl->setDisplayUnit( impl_getDocumentMeasurementUnit_throw() );
            }
            break;

            case PROPERTY_ID_TEXT_ANCHOR_TYPE:
            case PROPERTY_ID_SHEET_ANCHOR_TYPE:
                // default handling from PropertyHandler is sufficient
                break;

            default:
                OSL_FAIL( "FormGeometryHandler::describePropertyLine: huh?" );
                break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return aLineDesc;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormGeometryHandler::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _listener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xChangeNotifier.is(), "FormGeometryHandler::addPropertyChangeListener: no notified, implies no shape!?" );
        if ( m_xChangeNotifier.is() )
            m_xChangeNotifier->addPropertyChangeListener( _listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormGeometryHandler::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _listener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_PRECOND( m_xChangeNotifier.is(), "FormGeometryHandler::removePropertyChangeListener: no notified, implies no shape!?" );
        if ( m_xChangeNotifier.is() )
            m_xChangeNotifier->removePropertyChangeListener( _listener );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL FormGeometryHandler::getActuatingProperties( ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aInterestedIn(1);
        aInterestedIn[0] = PROPERTY_TEXT_ANCHOR_TYPE;
        return aInterestedIn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormGeometryHandler::actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& /*_rOldValue*/, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool /*_bFirstTimeInit*/ ) throw (NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        ::osl::MutexGuard aGuard( m_aMutex );
        PropertyId nActuatingPropId( impl_getPropertyId_throw( _rActuatingPropertyName ) );

        switch ( nActuatingPropId )
        {
        case PROPERTY_ID_TEXT_ANCHOR_TYPE:
        {
            TextContentAnchorType eAnchorType( TextContentAnchorType_AT_PARAGRAPH );
            OSL_VERIFY( _rNewValue >>= eAnchorType );
            _rxInspectorUI->enablePropertyUI( PROPERTY_POSITIONX, eAnchorType != TextContentAnchorType_AS_CHARACTER );
        }
        break;
        default:
            OSL_FAIL( "FormGeometryHandler::actuatingPropertyChanged: not registered for this property!" );
            break;
        }
    }

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL FormGeometryHandler::doDescribeSupportedProperties() const
    {
        if ( !m_xAssociatedShape.is() )
            return Sequence< Property >();

        ::std::vector< Property > aProperties;

        addInt32PropertyDescription( aProperties, PROPERTY_POSITIONX );
        addInt32PropertyDescription( aProperties, PROPERTY_POSITIONY );
        addInt32PropertyDescription( aProperties, PROPERTY_WIDTH );
        addInt32PropertyDescription( aProperties, PROPERTY_HEIGHT );

        if ( impl_haveTextAnchorType_nothrow() )
            implAddPropertyDescription( aProperties, PROPERTY_TEXT_ANCHOR_TYPE, ::cppu::UnoType< TextContentAnchorType >::get() );

        if ( impl_haveSheetAnchorType_nothrow() )
            addInt32PropertyDescription( aProperties, PROPERTY_SHEET_ANCHOR_TYPE );

        return Sequence< Property >( &(*aProperties.begin()), aProperties.size() );
    }

    //--------------------------------------------------------------------
    void SAL_CALL FormGeometryHandler::disposing()
    {
        FormGeometryHandler_Base::disposing();

        if ( m_xChangeNotifier.is() )
        {
            m_xChangeNotifier->dispose();
            m_xChangeNotifier.clear();
        }
    }

    //--------------------------------------------------------------------
    bool FormGeometryHandler::impl_haveTextAnchorType_nothrow() const
    {
        ENSURE_OR_THROW( m_xShapeProperties.is(), "not to be called without shape properties" );
        try
        {
            Reference< XPropertySetInfo > xPSI( m_xShapeProperties->getPropertySetInfo(), UNO_SET_THROW );
            if ( xPSI->hasPropertyByName( PROPERTY_ANCHOR_TYPE ) )
                return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool FormGeometryHandler::impl_haveSheetAnchorType_nothrow() const
    {
        ENSURE_OR_THROW( m_xShapeProperties.is(), "not to be called without shape properties" );
        try
        {
            Reference< XPropertySetInfo > xPSI( m_xShapeProperties->getPropertySetInfo(), UNO_SET_THROW );
            if ( !xPSI->hasPropertyByName( PROPERTY_ANCHOR ) )
                return false;
            Reference< XServiceInfo > xSI( m_xAssociatedShape, UNO_QUERY_THROW );
            if ( xSI->supportsService( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.Shape" ) ) ) )
                return true;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }

    //--------------------------------------------------------------------
    namespace
    {
        static sal_Int32 lcl_getLowerBoundRowOrColumn( const Reference< XIndexAccess >& _rxRowsOrColumns, const bool _bRows,
            const AwtPoint& _rRelativePosition )
        {
            sal_Int32 nAccumulated = 0;

            const sal_Int32& rRelativePos = _bRows ? _rRelativePosition.Y : _rRelativePosition.X;

            sal_Int32 nElements = _rxRowsOrColumns->getCount();
            sal_Int32 currentPos = 0;
            for ( currentPos=0; currentPos<nElements; ++currentPos )
            {
                Reference< XPropertySet > xRowOrColumn( _rxRowsOrColumns->getByIndex( currentPos ), UNO_QUERY_THROW );

                sal_Bool bIsVisible = sal_True;
                OSL_VERIFY( xRowOrColumn->getPropertyValue( PROPERTY_IS_VISIBLE ) >>= bIsVisible );
                if ( !bIsVisible )
                    continue;

                sal_Int32 nHeightOrWidth( 0 );
                OSL_VERIFY( xRowOrColumn->getPropertyValue( _bRows ? PROPERTY_HEIGHT : PROPERTY_WIDTH ) >>= nHeightOrWidth );

                if ( nAccumulated + nHeightOrWidth > rRelativePos )
                    break;

                nAccumulated += nHeightOrWidth;
            }

            return currentPos;
        }
    }

    //--------------------------------------------------------------------
    void FormGeometryHandler::impl_setSheetAnchorType_nothrow( const sal_Int32 _nAnchorType ) const
    {
        ENSURE_OR_THROW( m_xShapeProperties.is(), "illegal to be called without shape properties." );
        try
        {
            CellBindingHelper aHelper( m_xComponent, impl_getContextDocument_nothrow() );
            // find the sheet which the control belongs to
            Reference< XSpreadsheet > xSheet;
            aHelper.getControlSheetIndex( xSheet );

            switch ( _nAnchorType )
            {
            case ANCHOR_TO_SHEET:
                OSL_ENSURE( xSheet.is(),
                    "FormGeometryHandler::impl_setSheetAnchorType_nothrow: sheet not found!" );
                if ( xSheet.is() )
                {
                    AwtPoint aPreservePosition( m_xAssociatedShape->getPosition() );
                    m_xShapeProperties->setPropertyValue( PROPERTY_ANCHOR, makeAny( xSheet ) );
                    m_xAssociatedShape->setPosition( aPreservePosition );
                }
                break;

            case ANCHOR_TO_CELL:
            {
                Reference< XColumnRowRange > xColsRows( xSheet, UNO_QUERY_THROW );

                // get the current anchor
                Reference< XSpreadsheet > xCurrentAnchor;
                OSL_VERIFY( m_xShapeProperties->getPropertyValue( PROPERTY_ANCHOR ) >>= xCurrentAnchor );
                OSL_ENSURE( xCurrentAnchor.is(), "FormGeometryHandler::impl_setSheetAnchorType_nothrow: only to be called when currently anchored to a sheet!" );

                // get the current position
                AwtPoint aRelativePosition( m_xAssociatedShape->getPosition() );

                Reference< XTableColumns > xCols( xColsRows->getColumns(), UNO_SET_THROW );
                sal_Int32 nNewAnchorCol = lcl_getLowerBoundRowOrColumn( xCols.get(), false, aRelativePosition );

                Reference< XTableRows > xRows( xColsRows->getRows(), UNO_SET_THROW );
                sal_Int32 nNewAnchorRow = lcl_getLowerBoundRowOrColumn( xRows.get(), true, aRelativePosition );

                Reference< XCellRange > xSheetCellRange( xSheet, UNO_QUERY_THROW );
                Any aNewAnchorCell( xSheetCellRange->getCellByPosition( nNewAnchorCol, nNewAnchorRow ) );
                m_xShapeProperties->setPropertyValue( PROPERTY_ANCHOR, aNewAnchorCell );
            }
            break;

            default:
                OSL_FAIL( "FormGeometryHandler::impl_setSheetAnchorType_nothrow: illegal anchor type!" );
                break;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //====================================================================
    //= ShapeGeometryChangeNotifier - implementation
    //====================================================================
    namespace
    {
        struct EventTranslation
        {
            ::rtl::OUString sPropertyName;
            Any             aNewPropertyValue;

            EventTranslation( const ::rtl::OUString& _propertyName, const Any& _newPropertyValue )
                :sPropertyName( _propertyName )
                ,aNewPropertyValue( _newPropertyValue )
            {
            }
        };
    }

    //--------------------------------------------------------------------
    void SAL_CALL ShapeGeometryChangeNotifier::propertyChange( const PropertyChangeEvent& _event ) throw (RuntimeException)
    {
        ::comphelper::ComponentMethodGuard aGuard( *this );

        ::std::vector< EventTranslation > aEventTranslations;
        aEventTranslations.reserve(2);

        if ( _event.PropertyName == "Position" )
        {
            AwtPoint aPos = m_xShape->getPosition();
            aEventTranslations.push_back( EventTranslation( PROPERTY_POSITIONX, makeAny( aPos.X ) ) );
            aEventTranslations.push_back( EventTranslation( PROPERTY_POSITIONY, makeAny( aPos.Y ) ) );
        }
        else if ( _event.PropertyName == "Size" )
        {
            AwtSize aSize = m_xShape->getSize();
            aEventTranslations.push_back( EventTranslation( PROPERTY_WIDTH, makeAny( aSize.Width ) ) );
            aEventTranslations.push_back( EventTranslation( PROPERTY_HEIGHT, makeAny( aSize.Height ) ) );
        }
        else if ( _event.PropertyName == PROPERTY_ANCHOR_TYPE )
        {
            aEventTranslations.push_back( EventTranslation( PROPERTY_TEXT_ANCHOR_TYPE, makeAny( _event.NewValue ) ) );
        }
        else if ( _event.PropertyName == PROPERTY_ANCHOR )
        {
            aEventTranslations.push_back( EventTranslation( PROPERTY_SHEET_ANCHOR_TYPE, makeAny( _event.NewValue ) ) );
        }

        PropertyChangeEvent aTranslatedEvent( _event );
        aTranslatedEvent.Source = m_rParent;

        aGuard.clear();
        for ( ::std::vector< EventTranslation >::const_iterator t = aEventTranslations.begin();
              t != aEventTranslations.end();
              ++t
            )
        {
            aTranslatedEvent.PropertyName = t->sPropertyName;
            aTranslatedEvent.NewValue = t->aNewPropertyValue;
            m_aPropertyChangeListeners.notifyEach( &XPropertyChangeListener::propertyChange, aTranslatedEvent );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL ShapeGeometryChangeNotifier::disposing( const EventObject& /*_event*/ ) throw (RuntimeException)
    {
        ::comphelper::ComponentMethodGuard aGuard( *this );
        impl_dispose_nothrow();
    }

    //--------------------------------------------------------------------
    void ShapeGeometryChangeNotifier::impl_init_nothrow()
    {
        osl_atomic_increment( &m_refCount );
        try
        {
            Reference< XPropertySet > xShapeProperties( m_xShape, UNO_QUERY_THROW );
            xShapeProperties->addPropertyChangeListener( ::rtl::OUString(), this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    void ShapeGeometryChangeNotifier::impl_dispose_nothrow()
    {
        try
        {
            Reference< XPropertySet > xShapeProperties( m_xShape, UNO_QUERY_THROW );
            xShapeProperties->removePropertyChangeListener( ::rtl::OUString(), this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        getBroadcastHelper().bDisposed = true;
    }

//........................................................................
} // namespace pcr
//........................................................................

extern "C" void SAL_CALL createRegistryInfo_FormGeometryHandler()
{
    ::pcr::FormGeometryHandler::registerImplementation();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
