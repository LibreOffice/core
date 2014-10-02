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

#include <config_features.h>

#include "services.hxx"
#include "frm_module.hxx"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <uno/mapping.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

static Sequence< OUString >              s_aClassImplementationNames;
static Sequence<Sequence< OUString > >   s_aClassServiceNames;

// need to use sal_Int64 instead of ComponentInstantiation, as ComponentInstantiation has no cppuType, so
// it can't be used with sequences
static Sequence<sal_Int64>               s_aFactories;

void registerClassInfo(
        const OUString& _rClassImplName,                 // the ImplName of the class
        const Sequence< OUString >& _rServiceNames,      // the services supported by this class
        ::cppu::ComponentInstantiation _pCreateFunction  // the method for instantiating such a class
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



#define REGISTER_CLASS_CORE(classImplName) \
    registerClassInfo( \
        OUString("com.sun.star.form.") + OUString(#classImplName), \
        aServices, \
        frm::classImplName##_CreateInstance)


#define REGISTER_CLASS1(classImplName, service1) \
    aServices.realloc(1); \
    aServices.getArray()[0] = service1; \
    REGISTER_CLASS_CORE(classImplName)


#define REGISTER_CLASS2(classImplName, service1, service2) \
    aServices.realloc(2); \
    aServices.getArray()[0] = service1; \
    aServices.getArray()[1] = service2; \
    REGISTER_CLASS_CORE(classImplName)


#define REGISTER_CLASS3(classImplName, service1, service2, service3) \
    aServices.realloc(3); \
    aServices.getArray()[0] = service1; \
    aServices.getArray()[1] = service2; \
    aServices.getArray()[2] = service3; \
    REGISTER_CLASS_CORE(classImplName)


#define REGISTER_CLASS4(classImplName, service1, service2, service3, service4) \
    aServices.realloc(4); \
    aServices.getArray()[0] = service1; \
    aServices.getArray()[1] = service2; \
    aServices.getArray()[2] = service3; \
    aServices.getArray()[3] = service4; \
    REGISTER_CLASS_CORE(classImplName)


void ensureClassInfos()
{
    if (s_aClassImplementationNames.getLength())
        // nothing to do
        return;
    Sequence< OUString > aServices;


    // = ControlModels

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
    aServices.getArray()[0] = FRM_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[1] = FRM_SUN_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[2] = FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD;
    aServices.getArray()[3] = BINDABLE_DATABASE_FORMATTED_FIELD;

    registerClassInfo(OUString("com.sun.star.comp.forms.OFormattedFieldWrapper_ForcedFormatted"),
        aServices,
        frm::OFormattedFieldWrapper_CreateInstance_ForceFormatted);


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



    // = various
    aServices.realloc(1);
    aServices.getArray()[0] = "com.sun.star.form.Forms";
    REGISTER_CLASS_CORE(OFormsCollection);

    REGISTER_CLASS1(ImageProducer, SRV_AWT_IMAGEPRODUCER);


    // = XForms core
#define REGISTER_XFORMS_CLASS(name) \
    aServices.realloc(1); \
    aServices[0] = "com.sun.star.xforms." #name ; \
    REGISTER_CLASS_CORE(name)

    REGISTER_XFORMS_CLASS(Model);
    REGISTER_XFORMS_CLASS(XForms);

}


extern "C"
{

void SAL_CALL createRegistryInfo_FORMS()
{
    static bool bInit = false;
    if (!bInit)
    {
#if HAVE_FEATURE_DBCONNECTIVITY
        createRegistryInfo_ODatabaseForm();
#endif
        createRegistryInfo_OFilterControl();
        createRegistryInfo_OScrollBarModel();
        createRegistryInfo_OSpinButtonModel();
        createRegistryInfo_ONavigationBarModel();
        createRegistryInfo_ONavigationBarControl();
        createRegistryInfo_ORichTextModel();
        createRegistryInfo_ORichTextControl();
        createRegistryInfo_CLibxml2XFormsExtension();
#if HAVE_FEATURE_DBCONNECTIVITY
        createRegistryInfo_FormOperations();
#endif
        bInit = true;
    }
}


SAL_DLLPUBLIC_EXPORT void* SAL_CALL frm_component_getFactory(const sal_Char* _pImplName, void* _pServiceManager, void* /*_pRegistryKey*/)
{
    if (!_pServiceManager || !_pImplName)
        return NULL;


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
    const OUString* pClasses = s_aClassImplementationNames.getConstArray();
    const Sequence< OUString >* pServices = s_aClassServiceNames.getConstArray();
    const sal_Int64* pFunctionsAsInts = s_aFactories.getConstArray();

    for (sal_Int32 i=0; i<nClasses; ++i, ++pClasses, ++pServices, ++pFunctionsAsInts)
    {
        if (rtl_ustr_ascii_compare(pClasses->getStr(), _pImplName) == 0)
        {
            ::cppu::ComponentInstantiation aCurrentCreateFunction =
                reinterpret_cast< ::cppu::ComponentInstantiation>(*pFunctionsAsInts);

            Reference<XSingleServiceFactory> xFactory(
                ::cppu::createSingleFactory(
                    static_cast<css::lang::XMultiServiceFactory*>(
                        _pServiceManager),
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


    // the real way - use the OModule
    if ( !pRet )
    {
        createRegistryInfo_FORMS();
        {
            // let the module look for the component
            Reference< XInterface > xRet;
            xRet = ::frm::OFormsModule::getComponentFactory(
                OUString::createFromAscii( _pImplName ),
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
