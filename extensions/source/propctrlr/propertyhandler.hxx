/*************************************************************************
 *
 *  $RCSfile: propertyhandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:11:43 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <memory>
#include <vector>

//........................................................................
namespace pcr
{
//........................................................................

    typedef sal_Int32   PropertyId;

    struct PropertyUIDescriptor;
    class IPropertyBrowserUI;

    //====================================================================
    //= IPropertyHandler
    //====================================================================
    /** an instance does the complete property handling necessary for the property
        controller
    */
    class IPropertyHandler : public ::rtl::IReference
    {
    public:
        // ------------ meta data --------------------
        // TODO:
        // when we have migrated all the possible code in formcontroller.cxx to usage of this
        // interface (i.e. when even the generic properties are handled by an instance implementing
        // this interface), then these meta data are not necessary anymore (since all methods should
        // return <TRUE/> then).
        // Until this migration is completed, a property handler is allowed to implement
        // only parts of the functionality, and leave the rest up to the OPropertyBrowserController.

        /** determine whether <member>describePropertyUI</member>, <member>initializePropertyUI</member>,
            <member>requestUserInputOnButtonClick</member> and <member>executeButtonClick</member> are supported
        */
        virtual bool SAL_CALL
            supportsUIDescriptor( PropertyId _nPropId ) const = 0;

        /** retrieves the current value of a property given by id
            @param _nId
                the id of the property whose value is to be retrieved
            @param _bLazy
                If <TRUE/>, the callee must not check whether this property really exists.
                With passing <FALSE/>, the caller indicates that it's not really sure whether
                the property actually exists.
        */
        virtual ::com::sun::star::uno::Any SAL_CALL
            getPropertyValue( PropertyId _nPropId, bool _bLazy = true ) const = 0;

        /** sets the value of a property given by id
        */
        virtual void SAL_CALL
            setPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue ) = 0;

        /** translates a string representation of a property value
        */
        virtual ::com::sun::star::uno::Any SAL_CALL
            getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const = 0;

        /** translates a property value into a string representation
        */
        virtual ::rtl::OUString SAL_CALL
            getStringRepFromPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue ) const = 0;

        /// returns the state of a property given by id
        virtual ::com::sun::star::beans::PropertyState SAL_CALL
            getPropertyState( PropertyId _nPropId ) const = 0;

        /// registers a listener for notification about property value changes
        virtual void SAL_CALL
            startAllPropertyChangeListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener ) = 0;

        /// revokes a listener for notification about property value changes
        virtual void SAL_CALL
            stopAllPropertyChangeListening( ) = 0;

        /** returns the properties which the  handler can handle
        */
        virtual ::std::vector< ::com::sun::star::beans::Property > SAL_CALL
            getSupportedProperties() const = 0;

        /** returns the properties which are to be superseded by this handler

            Besides defining an own set of propertes (see <member>getSupportedProperties</member>),
            a property handler can also declare that foreign properties (which it is
            <em>not</em> responsible for) are superseded by its own properties.

            In this case, simply return those properties here.

            The handling is as follows: First, all property handlers are asked for the set of
            their supported properties. Then all handlers are asked for superseded
            properties. Only those properties which have at least once been reported as
            "supported", but <em>not</em> been reported as superseded, are visible in the user
            interface.

            Note that this requires corporative behavior of all property handlers: If one handler
            is responsible for property Foo, and supersedes property Bar, and another handler
            does exactly the opposite, then the result is that none of these properties will be
            present at the UI.
        */
        virtual ::std::vector< ::rtl::OUString > SAL_CALL
            getSupersededProperties( ) const = 0;

        /** retrieve the actuating properties which this handler is interested in

            In general, properties can be declared as "actuating", that is, when their value
            changes, the UI for other properties needs to be updated (e.g. enabled or disabled).

            With this method, a handler can declare that it feels responsible for some/all
            of the depending properties of certain actuating properties.

            Whenever the value of an actuating property changes, all handlers which expressed
            their interest in this particular actuating properties are called with their
            <member>updateDependentProperties</member> method.
        */
        virtual ::std::vector< ::rtl::OUString > SAL_CALL
            getActuatingProperties( ) const = 0;

        /** describes the UI to be used to represent the property
        */
        virtual void SAL_CALL
            describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const = 0;

        /** initializes the UI for a property

            This is called when the UI for a property has been newly inserted into the
            property browser, or when it has been rebuilt.

            @see rebuildPropertyUI::rebuildPropertyUI
        */
        virtual void SAL_CALL
            initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater ) = 0;

        /** called when a browse button belonging to a property UI represenation has been clicked

            When a browse button is clicked, two things happen: First, <member>requestUserInputOnButtonClick</member>
            is called. The property handler should obtain any data from the user which is needed to execute
            whatever action is associated with the button This may, for exmample, be a confirmation.
            However, the action must <em>not</em> be executed.<br/>
            Second, the data obtained from <member>requestUserInputOnButtonClick</member> is forwarded to
            <member>executeButtonClick</member>, which then must actually execute the action.

            If a particular button does not require input, handlers may simply return <TRUE/>.

            @param _nPropId
                The id of the property whose browse button has been clicked
            @param _bPrimary
                <TRUE/> if and only if the primary button has been clicked, <FALSE/> otherwise
            @param _rData
                The data which later on shall be passed to <member>executeButtonClick</member>.

            @return
                <TRUE/> if and only if <member>executeButtonClick</member> should be called with the
                data in <arg>_rData</arg>

            @see describePropertyUI
            @see executeButtonClick
        */
        virtual bool SAL_CALL
            requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, ::com::sun::star::uno::Any& _rData ) = 0;

        /** exeute an action associated with a browse button of a particular property

            Handlers are not allowed to raise UI in this method. If UI is needed for executing the action associated
            with a browse button, use <member>requestUserInputOnButtonClick</member>.

            @see describePropertyUI
            @see requestUserInputOnButtonClick
        */
        virtual void SAL_CALL
            executeButtonClick( PropertyId _nPropId, bool _bPrimary, const ::com::sun::star::uno::Any& _rData, IPropertyBrowserUI* _pUpdater ) = 0;

        /** updates the UI of dependent properties when the value of a certain actuating property changed
        */
        virtual void SAL_CALL
            updateDependentProperties( PropertyId _nActuatingPropId, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, IPropertyBrowserUI* _pUpdater ) = 0;

    public:
        virtual ~IPropertyHandler() { };
    };

    //====================================================================
    //= HandlerFactory
    //====================================================================
    typedef IPropertyHandler* (*HandlerFactory)
    (
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument,
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
    );

    //====================================================================
    //= PropertyHandler
    //====================================================================
    class OPropertyInfoService;
    /** the base class for property handlers
    */
    class PropertyHandler : public IPropertyHandler
    {
    private:
        /// cache for getSupportedProperties
        mutable ::std::vector< ::com::sun::star::beans::Property >
                                    m_aSupportedProperties;
        mutable bool                m_bSupportedPropertiesAreKnown;

    protected:
        oslInterlockedCount         m_refCount;
        /// our introspectee
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xIntrospectee;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >
                                    m_xTheListener;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >
                                    m_xTypeConverter;
        ::std::auto_ptr< OPropertyInfoService >
                                    m_pInfoService;

    protected:
        PropertyHandler(
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxIntrospectee,
                const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >& _rxTypeConverter
        );
        ~PropertyHandler();

        // default implementations for IPropertyHandler
        /// always returns <FALSE/>, delegating to the caller's default implementation this way
        virtual bool SAL_CALL supportsUIDescriptor( PropertyId _nPropId ) const;
        virtual ::std::vector< ::com::sun::star::beans::Property > SAL_CALL getSupportedProperties() const;
        virtual ::std::vector< ::rtl::OUString > SAL_CALL getSupersededProperties( ) const;
        virtual ::std::vector< ::rtl::OUString > SAL_CALL getActuatingProperties( ) const;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const;
        virtual ::rtl::OUString SAL_CALL getStringRepFromPropertyValue( PropertyId _nPropId, const ::com::sun::star::uno::Any& _rValue ) const;
        virtual ::com::sun::star::beans::PropertyState  SAL_CALL getPropertyState( PropertyId _nPropId ) const;
        virtual void SAL_CALL describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const;
        virtual void SAL_CALL initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater );
        virtual bool SAL_CALL requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, ::com::sun::star::uno::Any& _rData );
        virtual void SAL_CALL executeButtonClick( PropertyId _nPropId, bool _bPrimary, const ::com::sun::star::uno::Any& _rData, IPropertyBrowserUI* _pUpdater );
        virtual void SAL_CALL updateDependentProperties( PropertyId _nActuatingPropId, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, IPropertyBrowserUI* _pUpdater );
        virtual void SAL_CALL startAllPropertyChangeListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& _rxListener );
        virtual void SAL_CALL stopAllPropertyChangeListening( );

        // own overridables
        virtual ::std::vector< ::com::sun::star::beans::Property >
                                            SAL_CALL implDescribeSupportedProperties() const = 0;

    protected:
        /** fires the change in a property value to our listener (if any)
            @see startAllPropertyChangeListening
        */
        void    firePropertyChange( const ::rtl::OUString& _rPropName, PropertyId _nPropId,
                    const ::com::sun::star::uno::Any& _rOldValue, const ::com::sun::star::uno::Any& _rNewValue ) SAL_THROW(());

        //-------------------------------------------------------------------------------
        // helper for implementing implDescribeSupportedProperties
        /** adds a description for the given string property to the given property vector
            Most probably to be called from within getSupportedProperties
        */
        inline void addStringPropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const ::rtl::OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given int32 property to the given property vector
        */
        inline void addInt32PropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const ::rtl::OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given int16 property to the given property vector
        */
        inline void addInt16PropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const ::rtl::OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /** adds a description for the given double property to the given property vector
        */
        inline void addDoublePropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const ::rtl::OUString& _rPropertyName,
                    sal_Int16 _nAttribs = 0
                ) const;

        /// adds a Property, given by name only, to a given vector of Properties
        void implAddPropertyDescription(
                    ::std::vector< ::com::sun::star::beans::Property >& _rProperties,
                    const ::rtl::OUString& _rPropertyName,
                    const ::com::sun::star::uno::Type& _rType,
                    sal_Int16 _nAttribs = 0
                ) const;

        //-------------------------------------------------------------------------------
        // helper for accessing and maintaining meta data about our supported properties

        /** retrieves a property given by handle

            Note that the given property must be returned in getSupportedProperties
        */
        const ::com::sun::star::beans::Property*
                    getPropertyFromId( PropertyId _nPropId ) const;

        /** get the name of a property given by handle
        */
        inline ::rtl::OUString
                    getPropertyNameFromId( PropertyId _nPropId ) const;

        /** declares a given property to have a new type

            There might be properties whose type changes during the life time of the
            property handler. For example, imagine a handler which exposes "introspectee properties"
            which internally are properties of another component only <em>referenced</em> by
            the introspectee. If the reference changes, then then referenced component may change,
            and thus the type of some properties.

            (Note that if even the availability of such an indirect property changes, then
            this is not reflected in changing our "supported properties" set, but in simply hiding
            or showing the UI for the respective property.)

            Note that changing the type of a property is the only allowed modification you
            can do to your supported properties, after you have them described the first time
            (i.e. after your <member>implDescribeSupportedProperties</member> has been called).
            The reason is that this type is not evaluated externally to this handler,
            only some methods of this base class use it (e.g. the string conversion routines),
            and your derived class may use it.

            @see <member>IPropertyBrowserUI::showPropertyUI</member>
            @see <member>IPropertyBrowserUI::hidePropertyUI</member>
        */
        void        changeTypeOfSupportedProperty(
                        const sal_Int32 _nPropId,
                        const ::com::sun::star::uno::Type& _rNewType
                    );

    protected:
        // IReference implementqation
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    private:
        PropertyHandler();                                     // never implemented
        PropertyHandler( const PropertyHandler& );            // never implemented
        PropertyHandler& operator=( const PropertyHandler& ); // never implemented
    };

    //--------------------------------------------------------------------
    inline void PropertyHandler::addStringPropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const ::rtl::OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addInt32PropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const ::rtl::OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< sal_Int32* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addInt16PropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const ::rtl::OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< sal_Int16* >( NULL ) ), _nAttribs );
    }

    inline void PropertyHandler::addDoublePropertyDescription( ::std::vector< ::com::sun::star::beans::Property >& _rProperties, const ::rtl::OUString& _rPropertyName, sal_Int16 _nAttribs ) const
    {
        implAddPropertyDescription( _rProperties, _rPropertyName, ::getCppuType( static_cast< double* >( NULL ) ), _nAttribs );
    }

    inline ::rtl::OUString PropertyHandler::getPropertyNameFromId( PropertyId _nPropId ) const
    {
        const ::com::sun::star::beans::Property* pProp = getPropertyFromId( _nPropId );
        return pProp ? pProp->Name : ::rtl::OUString();
    }

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYHANDLER_HXX

