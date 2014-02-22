/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "services.hxx"
#include "frm_module.hxx"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <uno/mapping.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;



#define DECLARE_SERVICE_INFO(classImplName) \
    namespace frm { \
        extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> SAL_CALL classImplName##_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory) throw (::com::sun::star::uno::RuntimeException); \
    }


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


DECLARE_SERVICE_INFO(Binding)
DECLARE_SERVICE_INFO(Model)
DECLARE_SERVICE_INFO(XForms)


DECLARE_SERVICE_INFO(OFormattedFieldWrapper)
    
    
    
    
namespace frm { \
    extern Reference< XInterface > SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException); \
}

DECLARE_SERVICE_INFO(OFormsCollection)
DECLARE_SERVICE_INFO(ImageProducer)



static Sequence< OUString >                      s_aClassImplementationNames;
static Sequence<Sequence< OUString > >   s_aClassServiceNames;
static Sequence<sal_Int64>                              s_aFactories;
    
    


void registerClassInfo(
        OUString _rClassImplName,                                
        const Sequence< OUString >& _rServiceNames,      
        ::cppu::ComponentInstantiation _pCreateFunction                 
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
        
        return;
    Sequence< OUString > aServices;

    
    
    
    
    REGISTER_CLASS2(OFixedTextModel, FRM_COMPONENT_FIXEDTEXT, FRM_SUN_COMPONENT_FIXEDTEXT);
    
    REGISTER_CLASS3(OHiddenModel, FRM_COMPONENT_HIDDENCONTROL, FRM_SUN_COMPONENT_HIDDENCONTROL, FRM_COMPONENT_HIDDEN);
    
    REGISTER_CLASS2(OFileControlModel, FRM_COMPONENT_FILECONTROL, FRM_SUN_COMPONENT_FILECONTROL);
    
    REGISTER_CLASS2(OImageButtonModel, FRM_COMPONENT_IMAGEBUTTON, FRM_SUN_COMPONENT_IMAGEBUTTON);
    
    REGISTER_CLASS3(OGridControlModel, FRM_COMPONENT_GRID /* compatibility */, FRM_COMPONENT_GRIDCONTROL, FRM_SUN_COMPONENT_GRIDCONTROL);
    
    REGISTER_CLASS2(OGroupBoxModel, FRM_COMPONENT_GROUPBOX, FRM_SUN_COMPONENT_GROUPBOX);

    
    REGISTER_CLASS4( ORadioButtonModel, FRM_COMPONENT_RADIOBUTTON, FRM_SUN_COMPONENT_RADIOBUTTON, FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON, BINDABLE_DATABASE_RADIO_BUTTON );
    
    REGISTER_CLASS4( OCheckBoxModel, FRM_COMPONENT_CHECKBOX, FRM_SUN_COMPONENT_CHECKBOX, FRM_SUN_COMPONENT_DATABASE_CHECKBOX, BINDABLE_DATABASE_CHECK_BOX );
    
    REGISTER_CLASS4( OListBoxModel, FRM_COMPONENT_LISTBOX, FRM_SUN_COMPONENT_LISTBOX, FRM_SUN_COMPONENT_DATABASE_LISTBOX, BINDABLE_DATABASE_LIST_BOX );
    
    REGISTER_CLASS4( OComboBoxModel, FRM_COMPONENT_COMBOBOX, FRM_SUN_COMPONENT_COMBOBOX, FRM_SUN_COMPONENT_DATABASE_COMBOBOX, BINDABLE_DATABASE_COMBO_BOX );
    
    REGISTER_CLASS4( OEditModel, FRM_COMPONENT_TEXTFIELD, FRM_SUN_COMPONENT_TEXTFIELD, FRM_SUN_COMPONENT_DATABASE_TEXTFIELD, BINDABLE_DATABASE_TEXT_FIELD );
    
    REGISTER_CLASS3( ODateModel, FRM_COMPONENT_DATEFIELD, FRM_SUN_COMPONENT_DATEFIELD, FRM_SUN_COMPONENT_DATABASE_DATEFIELD );
    
    REGISTER_CLASS3( OTimeModel, FRM_COMPONENT_TIMEFIELD, FRM_SUN_COMPONENT_TIMEFIELD, FRM_SUN_COMPONENT_DATABASE_TIMEFIELD );
    
    REGISTER_CLASS4( ONumericModel, FRM_COMPONENT_NUMERICFIELD, FRM_SUN_COMPONENT_NUMERICFIELD, FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD, BINDABLE_DATABASE_NUMERIC_FIELD );
    
    REGISTER_CLASS3( OCurrencyModel, FRM_COMPONENT_CURRENCYFIELD, FRM_SUN_COMPONENT_CURRENCYFIELD, FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD );
    
    REGISTER_CLASS3( OPatternModel, FRM_COMPONENT_PATTERNFIELD, FRM_SUN_COMPONENT_PATTERNFIELD, FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD );
    
    REGISTER_CLASS2( OButtonModel, FRM_COMPONENT_COMMANDBUTTON, FRM_SUN_COMPONENT_COMMANDBUTTON );
    
    REGISTER_CLASS2( OImageControlModel, FRM_COMPONENT_IMAGECONTROL, FRM_SUN_COMPONENT_IMAGECONTROL );

    
    REGISTER_CLASS1(OFormattedFieldWrapper, FRM_COMPONENT_EDIT);
        
        
    
    
    aServices.realloc(4);
    aServices.getArray()[0] = FRM_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[1] = FRM_SUN_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[2] = FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD;
    aServices.getArray()[3] = BINDABLE_DATABASE_FORMATTED_FIELD;

    registerClassInfo(OUString("com.sun.star.comp.forms.OFormattedFieldWrapper_ForcedFormatted"),
        aServices,
        frm::OFormattedFieldWrapper_CreateInstance_ForceFormatted);

    
    
    
    REGISTER_CLASS2(ORadioButtonControl, STARDIV_ONE_FORM_CONTROL_RADIOBUTTON, FRM_SUN_CONTROL_RADIOBUTTON);
    
    REGISTER_CLASS2(OCheckBoxControl, STARDIV_ONE_FORM_CONTROL_CHECKBOX, FRM_SUN_CONTROL_CHECKBOX);
    
    REGISTER_CLASS2(OGroupBoxControl, STARDIV_ONE_FORM_CONTROL_GROUPBOX, FRM_SUN_CONTROL_GROUPBOX);
    
    REGISTER_CLASS2(OListBoxControl, STARDIV_ONE_FORM_CONTROL_LISTBOX, FRM_SUN_CONTROL_LISTBOX);
    
    REGISTER_CLASS2(OComboBoxControl, STARDIV_ONE_FORM_CONTROL_COMBOBOX, FRM_SUN_CONTROL_COMBOBOX);
    
    REGISTER_CLASS3(OEditControl, STARDIV_ONE_FORM_CONTROL_TEXTFIELD, FRM_SUN_CONTROL_TEXTFIELD, STARDIV_ONE_FORM_CONTROL_EDIT);
    
    REGISTER_CLASS2(ODateControl, STARDIV_ONE_FORM_CONTROL_DATEFIELD, FRM_SUN_CONTROL_DATEFIELD);
    
    REGISTER_CLASS2(OTimeControl, STARDIV_ONE_FORM_CONTROL_TIMEFIELD, FRM_SUN_CONTROL_TIMEFIELD);
    
    REGISTER_CLASS2(ONumericControl, STARDIV_ONE_FORM_CONTROL_NUMERICFIELD, FRM_SUN_CONTROL_NUMERICFIELD);
    
    REGISTER_CLASS2(OCurrencyControl, STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD, FRM_SUN_CONTROL_CURRENCYFIELD);
    
    REGISTER_CLASS2(OPatternControl, STARDIV_ONE_FORM_CONTROL_PATTERNFIELD, FRM_SUN_CONTROL_PATTERNFIELD);
    
    REGISTER_CLASS2(OFormattedControl, STARDIV_ONE_FORM_CONTROL_FORMATTEDFIELD, FRM_SUN_CONTROL_FORMATTEDFIELD);
    
    REGISTER_CLASS2(OButtonControl, STARDIV_ONE_FORM_CONTROL_COMMANDBUTTON, FRM_SUN_CONTROL_COMMANDBUTTON);
    
    REGISTER_CLASS2(OImageButtonControl, STARDIV_ONE_FORM_CONTROL_IMAGEBUTTON, FRM_SUN_CONTROL_IMAGEBUTTON);
    
    REGISTER_CLASS2(OImageControlControl, STARDIV_ONE_FORM_CONTROL_IMAGECONTROL, FRM_SUN_CONTROL_IMAGECONTROL);


    
    
    aServices.realloc(1);
    aServices.getArray()[0] = "com.sun.star.form.Forms";
    REGISTER_CLASS_CORE(OFormsCollection);

    REGISTER_CLASS1(ImageProducer, SRV_AWT_IMAGEPRODUCER);

    
    
#define REGISTER_XFORMS_CLASS(name) \
    aServices.realloc(1); \
    aServices[0] = "com.sun.star.xforms." #name ; \
    REGISTER_CLASS_CORE(name)

    REGISTER_XFORMS_CLASS(Model);
    REGISTER_XFORMS_CLASS(XForms);

}


extern "C"
{


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


SAL_DLLPUBLIC_EXPORT void* SAL_CALL frm_component_getFactory(const sal_Char* _pImplName, XMultiServiceFactory* _pServiceManager, void* /*_pRegistryKey*/)
{
    if (!_pServiceManager || !_pImplName)
        return NULL;

    
    
    void* pRet = NULL;

    
    ensureClassInfos();

    
    sal_Int32 nClasses = s_aClassImplementationNames.getLength();
    OSL_ENSURE((s_aClassServiceNames.getLength() == nClasses) &&
        (s_aFactories.getLength() == nClasses),
        "forms::component_getFactory : invalid class infos !");

    
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

    
    
    if ( !pRet )
    {
        createRegistryInfo_FORMS();
        {
            
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
