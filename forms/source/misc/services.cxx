/*************************************************************************
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:10:29 $
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

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef FRM_MODULE_HXX
#include "frm_module.hxx"
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

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

// some special handling for the FormattedFieldWrapper which can act as FormattedModel or as EditModel
DECLARE_SERVICE_INFO(OFormattedFieldWrapper);
    // this is for a service, which is instantiated through the EditModel service name
    // and which acts mostly as Edit (mostly means : if somebody uses XPersistObject::read immediately after
    // the object was instantiated and the stream contains a FormattedModel, it switches permanently to
    // formatted.)
namespace frm { \
    extern Reference< XInterface > SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException); \
}

DECLARE_SERVICE_INFO(OFormsCollection)

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
        ::rtl::OUString::createFromAscii("com.sun.star.form.") + ::rtl::OUString::createFromAscii(#classImplName), \
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
        // since SUPD568 both OFormattedModel and OEditModel use FRM_COMPONENT_EDIT for persistence,
        // and while reading a wrapper determines which kind of model it is
    // register the wrapper for the FormattedField, as it handles the XPersistObject::write
    // so that version <= 5.1 are able to read it
    aServices.realloc(4);
    aServices.getArray()[0] = frm::FRM_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[1] = frm::FRM_SUN_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[2] = frm::FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD;
    aServices.getArray()[3] = frm::BINDABLE_DATABASE_FORMATTED_FIELD;

    registerClassInfo(::rtl::OUString::createFromAscii("com.sun.star.comp.forms.OFormattedFieldWrapper_ForcedFormatted"),
        aServices,
        frm::OFormattedFieldWrapper_CreateInstance_ForceFormatted);

    // ========================================================================
    // = Controls
    // - RadioButton
    REGISTER_CLASS2(ORadioButtonControl, FRM_CONTROL_RADIOBUTTON, FRM_SUN_CONTROL_RADIOBUTTON);
    // - CheckBox
    REGISTER_CLASS2(OCheckBoxControl, FRM_CONTROL_CHECKBOX, FRM_SUN_CONTROL_CHECKBOX);
    // - GroupBox
    REGISTER_CLASS2(OGroupBoxControl, FRM_CONTROL_GROUPBOX, FRM_SUN_CONTROL_GROUPBOX);
    // - ListBox
    REGISTER_CLASS2(OListBoxControl, FRM_CONTROL_LISTBOX, FRM_SUN_CONTROL_LISTBOX);
    // - ComboBox
    REGISTER_CLASS2(OComboBoxControl, FRM_CONTROL_COMBOBOX, FRM_SUN_CONTROL_COMBOBOX);
    // - EditControl
    REGISTER_CLASS3(OEditControl, FRM_CONTROL_TEXTFIELD, FRM_SUN_CONTROL_TEXTFIELD, FRM_CONTROL_EDIT);
    // - DateControl
    REGISTER_CLASS2(ODateControl, FRM_CONTROL_DATEFIELD, FRM_SUN_CONTROL_DATEFIELD);
    // - TimeControl
    REGISTER_CLASS2(OTimeControl, FRM_CONTROL_TIMEFIELD, FRM_SUN_CONTROL_TIMEFIELD);
    // - NumericField
    REGISTER_CLASS2(ONumericControl, FRM_CONTROL_NUMERICFIELD, FRM_SUN_CONTROL_NUMERICFIELD);
    // - CurrencyField
    REGISTER_CLASS2(OCurrencyControl, FRM_CONTROL_CURRENCYFIELD, FRM_SUN_CONTROL_CURRENCYFIELD);
    // - PatternField
    REGISTER_CLASS2(OPatternControl, FRM_CONTROL_PATTERNFIELD, FRM_SUN_CONTROL_PATTERNFIELD);
    // - FormattedField
    REGISTER_CLASS2(OFormattedControl, FRM_CONTROL_FORMATTEDFIELD, FRM_SUN_CONTROL_FORMATTEDFIELD);
    // - Button
    REGISTER_CLASS2(OButtonControl, FRM_CONTROL_COMMANDBUTTON, FRM_SUN_CONTROL_COMMANDBUTTON);
    // - ImageButton
    REGISTER_CLASS2(OImageButtonControl, FRM_CONTROL_IMAGEBUTTON, FRM_SUN_CONTROL_IMAGEBUTTON);
    // - ImageControl
    REGISTER_CLASS2(OImageControlControl, FRM_CONTROL_IMAGECONTROL, FRM_SUN_CONTROL_IMAGECONTROL);


    // ========================================================================
    // = various
    REGISTER_CLASS1(OFormsCollection, FRM_SUN_FORMS_COLLECTION);
}

//---------------------------------------------------------------------------------------
void registerServiceProvider(const ::rtl::OUString& _rServiceImplName, const Sequence< ::rtl::OUString >& _rServices, XRegistryKey* _pKey)
{
    ::rtl::OUString sMainKeyName = ::rtl::OUString::createFromAscii("/");
    sMainKeyName += _rServiceImplName;
    sMainKeyName += ::rtl::OUString::createFromAscii("/UNO/SERVICES");
    Reference< XRegistryKey > xNewKey = _pKey->createKey(sMainKeyName);
    OSL_ENSURE(xNewKey.is(), "forms::registerProvider : could not create a registry key !");
    if (!xNewKey.is())
        return;

    const ::rtl::OUString* pSupportedServices = _rServices.getConstArray();
    for (sal_Int32 i=0; i<_rServices.getLength(); ++i, ++pSupportedServices)
        xNewKey->createKey(*pSupportedServices);
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
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------
void SAL_CALL component_getImplementationEnvironment(const sal_Char** _ppEnvTypeName, uno_Environment** _ppEnv)
{
    *_ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
sal_Bool SAL_CALL component_writeInfo(void* _pServiceManager, XRegistryKey* _pRegistryKey)
{
    if (_pRegistryKey)
    {
        try
        {
            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
            // the real way - use the OModule
            createRegistryInfo_FORMS();
            if ( !::frm::OFormsModule::writeComponentInfos(
                    static_cast<XMultiServiceFactory*>( _pServiceManager ),
                    static_cast<XRegistryKey*>( _pRegistryKey ) )
                )
                return sal_False;

            // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
            // a lot of stuff which is implemented "manually" here in this file

            // collect the class infos
            ensureClassInfos();

            // both our static sequences should have the same length ...
            sal_Int32 nClasses = s_aClassImplementationNames.getLength();
            OSL_ENSURE(s_aClassServiceNames.getLength() == nClasses,
                "forms::component_writeInfo : invalid class infos !");

            // loop through the sequences and register the service providers
            const ::rtl::OUString* pClasses = s_aClassImplementationNames.getConstArray();
            const Sequence< ::rtl::OUString >* pServices = s_aClassServiceNames.getConstArray();

            for (sal_Int32 i=0; i<nClasses; ++i, ++pClasses, ++pServices)
                registerServiceProvider(*pClasses, *pServices, _pRegistryKey);

            s_aClassImplementationNames.realloc(0);
            s_aClassServiceNames.realloc(0);
            s_aFactories.realloc(0);

            return sal_True;
        }
        catch ( InvalidRegistryException& )
        {
            OSL_ENSURE(sal_False, "forms::component_writeInfo : InvalidRegistryException !");
        }
    }
    s_aClassImplementationNames.realloc(0);
    s_aClassServiceNames.realloc(0);
    s_aFactories.realloc(0);
    return sal_False;
}

//---------------------------------------------------------------------------------------
void* SAL_CALL component_getFactory(const sal_Char* _pImplName, XMultiServiceFactory* _pServiceManager, void* /*_pRegistryKey*/)
{
    if (!_pServiceManager || !_pImplName)
        return NULL;

    // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
    // a lot of stuff which is implemented "manually" here in this file
    void* pRet = NULL;

    // collect the class infos
    ensureClassInfos();

    // both our static sequences should have the same length ...
    sal_Int32 nClasses = s_aClassImplementationNames.getLength();
    OSL_ENSURE((s_aClassServiceNames.getLength() == nClasses) &&
        (s_aFactories.getLength() == nClasses),
        "forms::component_writeInfo : invalid class infos !");

    // loop through the sequences and register the service providers
    const ::rtl::OUString* pClasses = s_aClassImplementationNames.getConstArray();
    const Sequence< ::rtl::OUString >* pServices = s_aClassServiceNames.getConstArray();
    const sal_Int64* pFunctionsAsInts = s_aFactories.getConstArray();

    for (sal_Int32 i=0; i<nClasses; ++i, ++pClasses, ++pServices, ++pFunctionsAsInts)
    {
        if (rtl_ustr_ascii_compare(*pClasses, _pImplName) == 0)
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

    // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
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
