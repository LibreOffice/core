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

#include "services.hxx"
#include "frm_module.hxx"
#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>
#include <osl/diagnose.h>
#include <uno/mapping.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//---------------------------------------------------------------------------------------
//.......................................................................................
#define DECLARE_SERVICE_INFO(classImplName) \
    namespace frm { \
        extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL classImplName##_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory) throw (::com::sun::star::uno::RuntimeException); \
    }

//---------------------------------------------------------------------------------------
DECLARE_SERVICE_INFO(OFixedTextModel)
DECLARE_SERVICE_INFO(ORadioButtonModel)
DECLARE_SERVICE_INFO(ORadioButtonControl)
DECLARE_SERVICE_INFO(OCheckBoxModel)
DECLARE_SERVICE_INFO(OCheckBoxControl)
DECLARE_SERVICE_INFO(OHiddenModel)
DECLARE_SERVICE_INFO(OGroupBoxModel)
DECLARE_SERVICE_INFO(OGroupBoxControl)
DECLARE_SERVICE_INFO(OListBoxControl)
DECLARE_SERVICE_INFO(OListBoxModel)
DECLARE_SERVICE_INFO(OComboBoxControl)
DECLARE_SERVICE_INFO(OComboBoxModel)
DECLARE_SERVICE_INFO(OEditControl)
DECLARE_SERVICE_INFO(OEditModel)
DECLARE_SERVICE_INFO(ONumericControl)
DECLARE_SERVICE_INFO(ONumericModel)
DECLARE_SERVICE_INFO(OPatternControl)
DECLARE_SERVICE_INFO(OPatternModel)
DECLARE_SERVICE_INFO(OCurrencyControl)
DECLARE_SERVICE_INFO(OCurrencyModel)
DECLARE_SERVICE_INFO(ODateControl)
DECLARE_SERVICE_INFO(ODateModel)
DECLARE_SERVICE_INFO(OTimeControl)
DECLARE_SERVICE_INFO(OTimeModel)
DECLARE_SERVICE_INFO(OFormattedControl)
DECLARE_SERVICE_INFO(OFormattedModel)
DECLARE_SERVICE_INFO(OFileControlModel)
DECLARE_SERVICE_INFO(OButtonControl)
DECLARE_SERVICE_INFO(OButtonModel)
DECLARE_SERVICE_INFO(OImageButtonControl)
DECLARE_SERVICE_INFO(OImageButtonModel)

DECLARE_SERVICE_INFO(OImageControlControl)
DECLARE_SERVICE_INFO(OImageControlModel)
DECLARE_SERVICE_INFO(OGridControlModel)

// XForms objects
DECLARE_SERVICE_INFO(Binding)
DECLARE_SERVICE_INFO(Model)
DECLARE_SERVICE_INFO(XForms)

// some special handling for the FormattedFieldWrapper which can act as FormattedModel or as EditModel
DECLARE_SERVICE_INFO(OFormattedFieldWrapper)
    // this is for a service, which is instantiated through the EditModel service name
    // and which acts mostly as Edit (mostly means : if somebody uses XPersistObject::read immediately after
    // the object was instantiated and the stream contains a FormattedModel, it switches permanently to
    // formatted.)
namespace frm { \
    extern Reference< XInterface > SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException); \
}

DECLARE_SERVICE_INFO(OFormsCollection)
DECLARE_SERVICE_INFO(ImageProducer)

//---------------------------------------------------------------------------------------

static Sequence< ::rtl::OUString >                      s_aClassImplementationNames;
static Sequence<Sequence< ::rtl::OUString > >   s_aClassServiceNames;
static Sequence<sal_Int64>                              s_aFactories;
    // need to use sal_Int64 instead of ComponentInstantiation, as ComponentInstantiation has no cppuType, so
    // it can't be used with sequences

//---------------------------------------------------------------------------------------
void registerClassInfo(
        ::rtl::OUString _rClassImplName,                                // the ImplName of the class
        const Sequence< ::rtl::OUString >& _rServiceNames,      // the services supported by this class
        ::cppu::ComponentInstantiation _pCreateFunction                 // the method for instantiating such a class
        )
{
    sal_Int32 nCurrentLength = s_aClassImplementationNames.getLength();
    OSL_ENSURE((nCurrentLength == s_aClassServiceNames.getLength())
        && (nCurrentLength == s_aFactories.getLength()),
        "forms::registerClassInfo : invalid class infos !");

    s_aClassImplementationNames.realloc(nCurrentLength + 1);
    s_aClassServiceNames.realloc(nCurrentLength + 1);
    s_aFactories.realloc(nCurrentLength + 1);

    s_aClassImplementationNames.getArray()[nCurrentLength] = _rClassImplName;
    s_aClassServiceNames.getArray()[nCurrentLength] = _rServiceNames;
    s_aFactories.getArray()[nCurrentLength] = reinterpret_cast<sal_Int64>(_pCreateFunction);
}

//---------------------------------------------------------------------------------------
//.......................................................................................
#define REGISTER_CLASS_CORE(classImplName) \
    registerClassInfo( \
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.") ) + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(#classImplName)), \
        aServices, \
        frm::classImplName##_CreateInstance)

//.......................................................................................
#define REGISTER_CLASS1(classImplName, service1) \
    aServices.realloc(1); \
    aServices.getArray()[0] = frm::service1; \
    REGISTER_CLASS_CORE(classImplName)

//.......................................................................................
#define REGISTER_CLASS2(classImplName, service1, service2) \
    aServices.realloc(2); \
    aServices.getArray()[0] = frm::service1; \
    aServices.getArray()[1] = frm::service2; \
    REGISTER_CLASS_CORE(classImplName)

//.......................................................................................
#define REGISTER_CLASS3(classImplName, service1, service2, service3) \
    aServices.realloc(3); \
    aServices.getArray()[0] = frm::service1; \
    aServices.getArray()[1] = frm::service2; \
    aServices.getArray()[2] = frm::service3; \
    REGISTER_CLASS_CORE(classImplName)

//.......................................................................................
#define REGISTER_CLASS4(classImplName, service1, service2, service3, service4) \
    aServices.realloc(4); \
    aServices.getArray()[0] = frm::service1; \
    aServices.getArray()[1] = frm::service2; \
    aServices.getArray()[2] = frm::service3; \
    aServices.getArray()[3] = frm::service4; \
    REGISTER_CLASS_CORE(classImplName)

//---------------------------------------------------------------------------------------
void ensureClassInfos()
{
    if (s_aClassImplementationNames.getLength())
        // nothing to do
        return;
    Sequence< ::rtl::OUString > aServices;

    // ========================================================================
    // = ControlModels
    // ------------------------------------------------------------------------
    // - FixedText
    REGISTER_CLASS2(OFixedTextModel, FRM_COMPONENT_FIXEDTEXT, FRM_SUN_COMPONENT_FIXEDTEXT);
    // - Hidden
    REGISTER_CLASS3(OHiddenModel, FRM_COMPONENT_HIDDENCONTROL, FRM_SUN_COMPONENT_HIDDENCONTROL, FRM_COMPONENT_HIDDEN);
    // - FileControl
    REGISTER_CLASS2(OFileControlModel, FRM_COMPONENT_FILECONTROL, FRM_SUN_COMPONENT_FILECONTROL);
    // - ImageButton
    REGISTER_CLASS2(OImageButtonModel, FRM_COMPONENT_IMAGEBUTTON, FRM_SUN_COMPONENT_IMAGEBUTTON);
    // - GridControl
    REGISTER_CLASS3(OGridControlModel, FRM_COMPONENT_GRID /* compatibility */, FRM_COMPONENT_GRIDCONTROL, FRM_SUN_COMPONENT_GRIDCONTROL);
    // - GroupBox
    REGISTER_CLASS2(OGroupBoxModel, FRM_COMPONENT_GROUPBOX, FRM_SUN_COMPONENT_GROUPBOX);

    // - RadioButton
    REGISTER_CLASS4( ORadioButtonModel, FRM_COMPONENT_RADIOBUTTON, FRM_SUN_COMPONENT_RADIOBUTTON, FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON, BINDABLE_DATABASE_RADIO_BUTTON );
    // - CheckBox
    REGISTER_CLASS4( OCheckBoxModel, FRM_COMPONENT_CHECKBOX, FRM_SUN_COMPONENT_CHECKBOX, FRM_SUN_COMPONENT_DATABASE_CHECKBOX, BINDABLE_DATABASE_CHECK_BOX );
    // - ListBox
    REGISTER_CLASS4( OListBoxModel, FRM_COMPONENT_LISTBOX, FRM_SUN_COMPONENT_LISTBOX, FRM_SUN_COMPONENT_DATABASE_LISTBOX, BINDABLE_DATABASE_LIST_BOX );
    // - ComboBox
    REGISTER_CLASS4( OComboBoxModel, FRM_COMPONENT_COMBOBOX, FRM_SUN_COMPONENT_COMBOBOX, FRM_SUN_COMPONENT_DATABASE_COMBOBOX, BINDABLE_DATABASE_COMBO_BOX );
    // - EditControl
    REGISTER_CLASS4( OEditModel, FRM_COMPONENT_TEXTFIELD, FRM_SUN_COMPONENT_TEXTFIELD, FRM_SUN_COMPONENT_DATABASE_TEXTFIELD, BINDABLE_DATABASE_TEXT_FIELD );
    // - DateControl
    REGISTER_CLASS3( ODateModel, FRM_COMPONENT_DATEFIELD, FRM_SUN_COMPONENT_DATEFIELD, FRM_SUN_COMPONENT_DATABASE_DATEFIELD );
    // - TimeControl
    REGISTER_CLASS3( OTimeModel, FRM_COMPONENT_TIMEFIELD, FRM_SUN_COMPONENT_TIMEFIELD, FRM_SUN_COMPONENT_DATABASE_TIMEFIELD );
    // - NumericField
    REGISTER_CLASS4( ONumericModel, FRM_COMPONENT_NUMERICFIELD, FRM_SUN_COMPONENT_NUMERICFIELD, FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD, BINDABLE_DATABASE_NUMERIC_FIELD );
    // - CurrencyField
    REGISTER_CLASS3( OCurrencyModel, FRM_COMPONENT_CURRENCYFIELD, FRM_SUN_COMPONENT_CURRENCYFIELD, FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD );
    // - PatternField
    REGISTER_CLASS3( OPatternModel, FRM_COMPONENT_PATTERNFIELD, FRM_SUN_COMPONENT_PATTERNFIELD, FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD );
    // - Button
    REGISTER_CLASS2( OButtonModel, FRM_COMPONENT_COMMANDBUTTON, FRM_SUN_COMPONENT_COMMANDBUTTON );
    // - ImageControl
    REGISTER_CLASS2( OImageControlModel, FRM_COMPONENT_IMAGECONTROL, FRM_SUN_COMPONENT_IMAGECONTROL );

    // - FormattedField
    REGISTER_CLASS1(OFormattedFieldWrapper, FRM_COMPONENT_EDIT);
        // since SRC568 both OFormattedModel and OEditModel use FRM_COMPONENT_EDIT for persistence,
        // and while reading a wrapper determines which kind of model it is
    // register the wrapper for the FormattedField, as it handles the XPersistObject::write
    // so that version <= 5.1 are able to read it
    aServices.realloc(4);
    aServices.getArray()[0] = frm::FRM_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[1] = frm::FRM_SUN_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[2] = frm::FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD;
    aServices.getArray()[3] = frm::BINDABLE_DATABASE_FORMATTED_FIELD;

    registerClassInfo(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.forms.OFormattedFieldWrapper_ForcedFormatted") ),
        aServices,
        frm::OFormattedFieldWrapper_CreateInstance_ForceFormatted);

    // ========================================================================
    // = Controls
    // - RadioButton
    REGISTER_CLASS2(ORadioButtonControl, STARDIV_ONE_FORM_CONTROL_RADIOBUTTON, FRM_SUN_CONTROL_RADIOBUTTON);
    // - CheckBox
    REGISTER_CLASS2(OCheckBoxControl, STARDIV_ONE_FORM_CONTROL_CHECKBOX, FRM_SUN_CONTROL_CHECKBOX);
    // - GroupBox
    REGISTER_CLASS2(OGroupBoxControl, STARDIV_ONE_FORM_CONTROL_GROUPBOX, FRM_SUN_CONTROL_GROUPBOX);
    // - ListBox
    REGISTER_CLASS2(OListBoxControl, STARDIV_ONE_FORM_CONTROL_LISTBOX, FRM_SUN_CONTROL_LISTBOX);
    // - ComboBox
    REGISTER_CLASS2(OComboBoxControl, STARDIV_ONE_FORM_CONTROL_COMBOBOX, FRM_SUN_CONTROL_COMBOBOX);
    // - EditControl
    REGISTER_CLASS3(OEditControl, STARDIV_ONE_FORM_CONTROL_TEXTFIELD, FRM_SUN_CONTROL_TEXTFIELD, STARDIV_ONE_FORM_CONTROL_EDIT);
    // - DateControl
    REGISTER_CLASS2(ODateControl, STARDIV_ONE_FORM_CONTROL_DATEFIELD, FRM_SUN_CONTROL_DATEFIELD);
    // - TimeControl
    REGISTER_CLASS2(OTimeControl, STARDIV_ONE_FORM_CONTROL_TIMEFIELD, FRM_SUN_CONTROL_TIMEFIELD);
    // - NumericField
    REGISTER_CLASS2(ONumericControl, STARDIV_ONE_FORM_CONTROL_NUMERICFIELD, FRM_SUN_CONTROL_NUMERICFIELD);
    // - CurrencyField
    REGISTER_CLASS2(OCurrencyControl, STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD, FRM_SUN_CONTROL_CURRENCYFIELD);
    // - PatternField
    REGISTER_CLASS2(OPatternControl, STARDIV_ONE_FORM_CONTROL_PATTERNFIELD, FRM_SUN_CONTROL_PATTERNFIELD);
    // - FormattedField
    REGISTER_CLASS2(OFormattedControl, STARDIV_ONE_FORM_CONTROL_FORMATTEDFIELD, FRM_SUN_CONTROL_FORMATTEDFIELD);
    // - Button
    REGISTER_CLASS2(OButtonControl, STARDIV_ONE_FORM_CONTROL_COMMANDBUTTON, FRM_SUN_CONTROL_COMMANDBUTTON);
    // - ImageButton
    REGISTER_CLASS2(OImageButtonControl, STARDIV_ONE_FORM_CONTROL_IMAGEBUTTON, FRM_SUN_CONTROL_IMAGEBUTTON);
    // - ImageControl
    REGISTER_CLASS2(OImageControlControl, STARDIV_ONE_FORM_CONTROL_IMAGECONTROL, FRM_SUN_CONTROL_IMAGECONTROL);


    // ========================================================================
    // = various
    REGISTER_CLASS1(OFormsCollection, FRM_SUN_FORMS_COLLECTION);
    REGISTER_CLASS1(ImageProducer, SRV_AWT_IMAGEPRODUCER);

    // ========================================================================
    // = XForms core
#define REGISTER_XFORMS_CLASS(name) \
    aServices.realloc(1); \
    aServices.getArray()[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xforms." #name )); \
    REGISTER_CLASS_CORE(name)

    REGISTER_XFORMS_CLASS(Model);
    REGISTER_XFORMS_CLASS(XForms);

}

//=======================================================================================
extern "C"
{

//---------------------------------------------------------------------------------------
void SAL_CALL createRegistryInfo_ODatabaseForm();
void SAL_CALL createRegistryInfo_OFilterControl();
void SAL_CALL createRegistryInfo_OScrollBarModel();
void SAL_CALL createRegistryInfo_OSpinButtonModel();
void SAL_CALL createRegistryInfo_ONavigationBarModel();
void SAL_CALL createRegistryInfo_ONavigationBarControl();
void SAL_CALL createRegistryInfo_ORichTextModel();
void SAL_CALL createRegistryInfo_ORichTextControl();
void SAL_CALL createRegistryInfo_CLibxml2XFormsExtension();
void SAL_CALL createRegistryInfo_FormOperations();

//---------------------------------------------------------------------------------------
void SAL_CALL createRegistryInfo_FORMS()
{
    static sal_Bool bInit = sal_False;
    if (!bInit)
    {
        createRegistryInfo_ODatabaseForm();
        createRegistryInfo_OFilterControl();
        createRegistryInfo_OScrollBarModel();
        createRegistryInfo_OSpinButtonModel();
        createRegistryInfo_ONavigationBarModel();
        createRegistryInfo_ONavigationBarControl();
        createRegistryInfo_ORichTextModel();
        createRegistryInfo_ORichTextControl();
        createRegistryInfo_CLibxml2XFormsExtension();
        createRegistryInfo_FormOperations();
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------
SAL_DLLPUBLIC_EXPORT void* SAL_CALL frm_component_getFactory(const sal_Char* _pImplName, XMultiServiceFactory* _pServiceManager, void* /*_pRegistryKey*/)
{
    if (!_pServiceManager || !_pImplName)
        return NULL;

    // ========================================================================
    // a lot of stuff which is implemented "manually" here in this file
    void* pRet = NULL;

    // collect the class infos
    ensureClassInfos();

    // both our static sequences should have the same length ...
    sal_Int32 nClasses = s_aClassImplementationNames.getLength();
    OSL_ENSURE((s_aClassServiceNames.getLength() == nClasses) &&
        (s_aFactories.getLength() == nClasses),
        "forms::component_getFactory : invalid class infos !");

    // loop through the sequences and register the service providers
    const ::rtl::OUString* pClasses = s_aClassImplementationNames.getConstArray();
    const Sequence< ::rtl::OUString >* pServices = s_aClassServiceNames.getConstArray();
    const sal_Int64* pFunctionsAsInts = s_aFactories.getConstArray();

    for (sal_Int32 i=0; i<nClasses; ++i, ++pClasses, ++pServices, ++pFunctionsAsInts)
    {
        if (rtl_ustr_ascii_compare(pClasses->getStr(), _pImplName) == 0)
        {
            ::cppu::ComponentInstantiation aCurrentCreateFunction =
                reinterpret_cast< ::cppu::ComponentInstantiation>(*pFunctionsAsInts);

            Reference<XSingleServiceFactory> xFactory(
                ::cppu::createSingleFactory(
                    _pServiceManager,
                    *pClasses,
                    aCurrentCreateFunction,
                    *pServices
                )
            );
            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
                break;
            }
        }
    }

    // ========================================================================
    // the real way - use the OModule
    if ( !pRet )
    {
        createRegistryInfo_FORMS();
        {
            // let the module look for the component
            Reference< XInterface > xRet;
            xRet = ::frm::OFormsModule::getComponentFactory(
                ::rtl::OUString::createFromAscii( _pImplName ),
                static_cast< XMultiServiceFactory* >( _pServiceManager ) );

            if ( xRet.is() )
                xRet->acquire();
            pRet = xRet.get();
        }
    }

    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
