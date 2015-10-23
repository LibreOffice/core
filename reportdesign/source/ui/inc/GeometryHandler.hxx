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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_GEOMETRYHANDLER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_GEOMETRYHANDLER_HXX

#include <sal/config.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/XFunction.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <memory>
#include <comphelper/stl_types.hxx>
#include <comphelper/listenernotification.hxx>
#include "metadata.hxx"


namespace rptui
{


    struct DefaultFunction
    {
        css::beans::Optional< OUString>               m_sInitialFormula;
        OUString                                      m_sName;
        OUString                                      m_sSearchString;
        OUString                                      m_sFormula;
        bool                                          m_bPreEvaluated;
        bool                                          m_bDeepTraversing;

        inline OUString getName() const { return m_sName; }
    } ;

    class OPropertyInfoService;
    typedef ::std::pair< css::uno::Reference< css::report::XFunction>, css::uno::Reference< css::report::XFunctionsSupplier> > TFunctionPair;
    typedef ::std::multimap< OUString,TFunctionPair, ::comphelper::UStringMixLess > TFunctions;
    typedef ::comphelper::OSimpleListenerContainer  <   css::beans::XPropertyChangeListener
                                                    ,   css::beans::PropertyChangeEvent
                                                    >   PropertyChangeListeners;
    typedef ::cppu::WeakComponentImplHelper<   css::inspection::XPropertyHandler
                                            ,   css::beans::XPropertyChangeListener
                                            ,   css::lang::XServiceInfo> GeometryHandler_Base;

    class GeometryHandler:
        private ::cppu::BaseMutex,
        public GeometryHandler_Base
    {
        /** sets the counter function at the data field.
        *   If the counter function doesn't exist it will be created.
        */
        void impl_setCounterFunction_throw();

        /** executes a dialog for chosing a filter criterion for a database report
            @param _out_rSelectedClause
                the filter or order clause as chosen by the user
            @precond
                we're really inspecting a database form (well, a RowSet at least)
            @return
                <TRUE/> if and only if the user successfully chose a clause
        */
        bool impl_dialogFilter_nothrow( OUString& _out_rSelectedClause, ::osl::ClearableMutexGuard& _rClearBeforeDialog ) const;

        /** returns the data field type depending on the data field of the report control
        *
        * \param _sDataField if the data field is not empty it will be used as data field, otherwise the data field will be used.
        * \return the data field type
        */
        sal_uInt32 impl_getDataFieldType_throw(const OUString& _sDataField = OUString()) const;

        css::uno::Any getConstantValue(bool bToControlValue,sal_uInt16 nResId,const css::uno::Any& _aValue,const OUString& _sConstantName,const OUString & PropertyName );
        css::beans::Property getProperty(const OUString & PropertyName);
        static void implCreateListLikeControl(
                const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory
                ,css::inspection::LineDescriptor & out_Descriptor
                ,sal_uInt16 _nResId
                ,bool _bReadOnlyControl
                ,bool _bTrueIfListBoxFalseIfComboBox
            );
        static void implCreateListLikeControl(
                const css::uno::Reference< css::inspection::XPropertyControlFactory >& _rxControlFactory
                ,css::inspection::LineDescriptor & out_Descriptor
                ,const ::std::vector< OUString>& _aEntries
                ,bool _bReadOnlyControl
                ,bool _bTrueIfListBoxFalseIfComboBox
            );
        void checkPosAndSize(   const css::awt::Point& _aNewPos,
                                const css::awt::Size& _aSize);

        OUString impl_convertToFormula( const css::uno::Any& _rControlValue );

        void impl_initFieldList_nothrow( css::uno::Sequence< OUString >& _rFieldNames ) const;

        /** Creates the function defined by the function template
        *
        * \param _sFunctionName the function name
        * \param _sDataField the data field
        * \param _aFunction the function template
        */
        void impl_createFunction(const OUString& _sFunctionName,const OUString& _sDataField,const DefaultFunction& _aFunction);

        /** check whether the given function name is a counter function.
        *
        * \param _sQuotedFunctionName the quoted function name to check
        * \param _Out_sScope the scope of the function
        * \return When true it is a counter functions otherwise false.
        */
        bool impl_isCounterFunction_throw(const OUString& _sQuotedFunctionName,OUString& _Out_sScope) const;

        /** clear the own properties like function and scope and send a notification
        *
        * \param _aGuard
        * \param _sOldFunctionName
        * \param _sOldScope
        * \param _nOldDataFieldType
        */
       void resetOwnProperties(::osl::ResettableMutexGuard& _aGuard,const OUString& _sOldFunctionName,const OUString& _sOldScope,const sal_uInt32 _nOldDataFieldType);

        /** checks whether the name is a field or a parameter
        *
        * \param _sName the name to check
        * \return true when it is a field or parameter otherwise false
        */
        bool impl_isDataField(const OUString& _sName) const;

        /**return all formula in a semicolon separated list
        *
        * \param _rList the localized function names
        */
        void impl_fillFormulaList_nothrow(::std::vector< OUString >& _out_rList) const;

        /** return all group names in a semicolon separated list starting with the group where this control is contained in.
        *
        * \param _rList fills the list with all scope names.
        */
        void impl_fillScopeList_nothrow(::std::vector< OUString >& _out_rList) const;

        /** return all supported output formats of the report definition
        *
        * \param _rList fills the list with all mime types
        */
        void impl_fillMimeTypes_nothrow(::std::vector< OUString >& _out_rList) const;

        /** return the one supported output formats of the report definition
        *
        * \param _sMimetype the mimetype
        */
        OUString impl_ConvertMimeTypeToUI_nothrow(const OUString& _sMimetype) const;

        /** return the MimeType for the given UI Name
        *
        * \param _sUIName the doc ui name
        */
        OUString impl_ConvertUIToMimeType_nothrow(const OUString& _sUIName) const;

        /** get the functions supplier for the set scope, default is the surrounding group.
        *
        * \param _rsNamePostfix the name postfix which can be used when the scope as name part is needed
        * \return the function supplier
        */
        css::uno::Reference< css::report::XFunctionsSupplier> fillScope_throw(OUString& _rsNamePostfix);

        /** checks if the given function is a default function we know.
        *
        * \param _sQuotedFunction the quoted function name
        * \param _Out_rDataField the data field which is used in the function
        * \param _xFunctionsSupplier the function supplier to search or empty if not used
        * \param _bSet If set to sal_True than the m_sDefaultFunction and m_sScope vars will be set if successful.
        * \return sal_True with known otherwise sal_False
        */
        bool isDefaultFunction(const OUString& _sQuotedFunction
                                    ,OUString& _Out_rDataField
                                    ,const css::uno::Reference< css::report::XFunctionsSupplier>& _xFunctionsSupplier = css::uno::Reference< css::report::XFunctionsSupplier>()
                                    ,bool _bSet = false) const;

        /** checks if the given function is a default function we know.
        *
        * \param _xFunction
        * \param _rDataField
        * \param _rsDefaultFunctionName
        * \return
        */
        bool impl_isDefaultFunction_nothrow( const css::uno::Reference< css::report::XFunction>& _xFunction
                                            ,OUString& _rDataField
                                            ,OUString& _rsDefaultFunctionName) const;

        /** fills the member m_aDefaultFunctions
        *
        */
        void loadDefaultFunctions();

        /** creates a default functionof the _sFunction for the data field _sDataField
        *   The new function will only be created if it didn't exist.
        *
        * \param _aGuard        Will be cleared, when a new function was created.
        * \param _sFunction     The name of the function.
        * \param _sDataField    The name of the data field.
        */
        void createDefaultFunction(::osl::ResettableMutexGuard& _aGuard ,const OUString& _sFunction,const OUString& _sDataField);

        void removeFunction();

        class OBlocker
        {
            bool& m_bIn;
        public:
            OBlocker(bool& _bIn) : m_bIn(_bIn){ m_bIn = true; }
            ~OBlocker() { m_bIn = false; }
        };


        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) throw(css::uno::RuntimeException, std::exception) override;

    public:
        // XServiceInfo - static versions
        static OUString getImplementationName_Static(  ) throw(css::uno::RuntimeException);
        static css::uno::Sequence< OUString > getSupportedServiceNames_static(  ) throw(css::uno::RuntimeException);
        static css::uno::Reference< css::uno::XInterface > SAL_CALL
                        create(const css::uno::Reference< css::uno::XComponentContext >&);

    public:
        explicit GeometryHandler(css::uno::Reference< css::uno::XComponentContext > const & context);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

        // css::lang::XComponent:
        virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & xListener)   throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) throw (css::uno::RuntimeException, std::exception) override;

        // css::inspection::XPropertyHandler:
        virtual void SAL_CALL inspect(const css::uno::Reference< css::uno::XInterface > & Component) throw (css::uno::RuntimeException, css::lang::NullPointerException, std::exception) override;
        virtual css::uno::Any SAL_CALL getPropertyValue(const OUString & PropertyName) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override;
        virtual void SAL_CALL setPropertyValue(const OUString & PropertyName, const css::uno::Any & Value) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::beans::PropertyVetoException, std::exception) override;
        virtual css::beans::PropertyState SAL_CALL getPropertyState(const OUString & PropertyName) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override;
        virtual css::inspection::LineDescriptor SAL_CALL describePropertyLine(const OUString& PropertyName, const css::uno::Reference< css::inspection::XPropertyControlFactory >& ControlFactory ) throw (css::beans::UnknownPropertyException, css::lang::NullPointerException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Any SAL_CALL convertToPropertyValue(const OUString & PropertyName, const css::uno::Any & ControlValue) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override;
        virtual css::uno::Any SAL_CALL convertToControlValue(const OUString & PropertyName, const css::uno::Any & PropertyValue, const css::uno::Type & ControlValueType) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override;
        virtual void SAL_CALL addPropertyChangeListener(const css::uno::Reference< css::beans::XPropertyChangeListener > & Listener) throw (css::uno::RuntimeException, css::lang::NullPointerException, std::exception) override;
        virtual void SAL_CALL removePropertyChangeListener(const css::uno::Reference< css::beans::XPropertyChangeListener > & _rxListener) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::Property > SAL_CALL getSupportedProperties() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupersededProperties() throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getActuatingProperties() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL isComposable(const OUString & PropertyName) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override;
        virtual css::inspection::InteractiveSelectionResult SAL_CALL onInteractivePropertySelection(const OUString & PropertyName, sal_Bool Primary, css::uno::Any & out_Data, const css::uno::Reference< css::inspection::XObjectInspectorUI > & InspectorUI) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, css::lang::NullPointerException, std::exception) override;
        virtual void SAL_CALL actuatingPropertyChanged(const OUString & ActuatingPropertyName, const css::uno::Any & NewValue, const css::uno::Any & OldValue, const css::uno::Reference< css::inspection::XObjectInspectorUI > & InspectorUI, sal_Bool FirstTimeInit) throw (css::uno::RuntimeException, css::lang::NullPointerException, std::exception) override;
        virtual sal_Bool SAL_CALL suspend(sal_Bool Suspend) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        virtual ~GeometryHandler();
    private:
        GeometryHandler(GeometryHandler &) = delete;
        void operator =(GeometryHandler &) = delete;

        // override WeakComponentImplHelperBase::disposing()
        // This function is called upon disposing the component,
        // if your component needs special work when it becomes
        // disposed, do it here.
        virtual void SAL_CALL disposing() override;

        PropertyChangeListeners                                   m_aPropertyListeners;
        css::uno::Sequence< OUString >                            m_aFieldNames;
        css::uno::Sequence< OUString >                            m_aParamNames;
        TFunctions                                                m_aFunctionNames;
        ::std::vector< DefaultFunction >                          m_aDefaultFunctions;
        DefaultFunction                                           m_aCounterFunction;
        css::uno::Reference< css::uno::XComponentContext >        m_xContext;
        mutable css::uno::Reference< css::report::XFunction>      m_xFunction;
        css::uno::Reference< css::inspection::XPropertyHandler >  m_xFormComponentHandler; /// delegatee
        css::uno::Reference< css::beans::XPropertySet >           m_xReportComponent; /// inspectee
        mutable css::uno::Reference< css::sdbc::XRowSet >         m_xRowSet;
        /// type converter, needed on various occasions
        css::uno::Reference< css::script::XTypeConverter >        m_xTypeConverter;
        /// access to property meta data
        ::std::unique_ptr< OPropertyInfoService >                 m_pInfoService;
        mutable OUString                                          m_sDefaultFunction;
        mutable OUString                                          m_sScope;
        sal_uInt32                                                m_nDataFieldType;
        mutable bool                                              m_bNewFunction;
        bool                                                      m_bIn;
    };

} // namespace rptui


#endif // RPT_GeometryHandler_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
