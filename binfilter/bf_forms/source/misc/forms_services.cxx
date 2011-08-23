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

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef FORMS_MODULE_HXX
#include "formsmodule.hxx"
#endif
namespace binfilter {

//... namespace frm .......................................................
namespace frm
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

IMPLEMENT_CONSTASCII_USTRING(AWT_CONTROL_TEXTFIELD, "com.sun.star.awt.TextField");

IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_EDIT, "stardiv.vcl.control.Edit");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_LISTBOX, "stardiv.vcl.control.ListBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_COMBOBOX, "stardiv.vcl.control.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_RADIOBUTTON, "stardiv.vcl.control.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_GROUPBOX, "stardiv.vcl.control.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_FIXEDTEXT, "stardiv.vcl.control.FixedText");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_COMMANDBUTTON, "stardiv.vcl.control.Button");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_CHECKBOX, "stardiv.vcl.control.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_IMAGEBUTTON, "stardiv.vcl.control.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_FILECONTROL, "stardiv.vcl.control.FileControl");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_TIMEFIELD, "stardiv.vcl.control.TimeField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_DATEFIELD, "stardiv.vcl.control.DateField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_NUMERICFIELD, "stardiv.vcl.control.NumericField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_CURRENCYFIELD, "stardiv.vcl.control.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_PATTERNFIELD, "stardiv.vcl.control.PatternField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_FORMATTEDFIELD, "stardiv.vcl.control.FormattedField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_IMAGECONTROL, "stardiv.vcl.control.ImageControl");

IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_EDIT, "stardiv.vcl.controlmodel.Edit");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_LISTBOX, "stardiv.vcl.controlmodel.ListBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_COMBOBOX, "stardiv.vcl.controlmodel.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_RADIOBUTTON, "stardiv.vcl.controlmodel.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_GROUPBOX, "stardiv.vcl.controlmodel.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_FIXEDTEXT, "stardiv.vcl.controlmodel.FixedText");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_COMMANDBUTTON, "stardiv.vcl.controlmodel.Button");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_CHECKBOX, "stardiv.vcl.controlmodel.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_IMAGEBUTTON, "stardiv.vcl.controlmodel.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_FILECONTROL, "stardiv.vcl.controlmodel.FileControl");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_TIMEFIELD, "stardiv.vcl.controlmodel.TimeField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_DATEFIELD, "stardiv.vcl.controlmodel.DateField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_NUMERICFIELD, "stardiv.vcl.controlmodel.NumericField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_CURRENCYFIELD, "stardiv.vcl.controlmodel.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_PATTERNFIELD, "stardiv.vcl.controlmodel.PatternField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_FORMATTEDFIELD, "stardiv.vcl.controlmodel.FormattedField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_IMAGECONTROL, "stardiv.vcl.controlmodel.ImageControl");

// -----------------------
// service names for compatibility
// -----------------------
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_FORM, "stardiv.one.form.component.Form");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_EDIT, "stardiv.one.form.component.Edit");			// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_TEXTFIELD, "stardiv.one.form.component.TextField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_LISTBOX, "stardiv.one.form.component.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_COMBOBOX, "stardiv.one.form.component.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_RADIOBUTTON, "stardiv.one.form.component.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_GROUPBOX, "stardiv.one.form.component.GroupBox");		// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_FIXEDTEXT, "stardiv.one.form.component.FixedText");		// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_COMMANDBUTTON, "stardiv.one.form.component.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_CHECKBOX, "stardiv.one.form.component.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_GRID, "stardiv.one.form.component.Grid");			// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_GRIDCONTROL, "stardiv.one.form.component.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_IMAGEBUTTON, "stardiv.one.form.component.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_FILECONTROL, "stardiv.one.form.component.FileControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_TIMEFIELD, "stardiv.one.form.component.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_DATEFIELD, "stardiv.one.form.component.DateField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_NUMERICFIELD, "stardiv.one.form.component.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_CURRENCYFIELD, "stardiv.one.form.component.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_PATTERNFIELD, "stardiv.one.form.component.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_HIDDEN, "stardiv.one.form.component.Hidden");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_HIDDENCONTROL, "stardiv.one.form.component.HiddenControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_IMAGECONTROL, "stardiv.one.form.component.ImageControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_COMPONENT_FORMATTEDFIELD, "stardiv.one.form.component.FormattedField");

IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_EDIT, "stardiv.one.form.control.Edit");					// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_TEXTFIELD, "stardiv.one.form.control.TextField");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_LISTBOX, "stardiv.one.form.control.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_COMBOBOX, "stardiv.one.form.control.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_RADIOBUTTON, "stardiv.one.form.control.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_GROUPBOX, "stardiv.one.form.control.GroupBox");				// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_FIXEDTEXT, "stardiv.one.form.control.FixedText");			// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_COMMANDBUTTON, "stardiv.one.form.control.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_CHECKBOX, "stardiv.one.form.control.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_GRID, "stardiv.one.form.control.Grid");					// compatibility
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_GRIDCONTROL, "stardiv.one.form.control.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_IMAGEBUTTON, "stardiv.one.form.control.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_TIMEFIELD, "stardiv.one.form.control.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_DATEFIELD, "stardiv.one.form.control.DateField");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_NUMERICFIELD, "stardiv.one.form.control.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_CURRENCYFIELD, "stardiv.one.form.control.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_PATTERNFIELD, "stardiv.one.form.control.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_IMAGECONTROL, "stardiv.one.form.control.ImageControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_CONTROL_FORMATTEDFIELD, "stardiv.one.form.control.FormattedField");

// -----------------------
// new (sun) service names
// -----------------------
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_FORM, "com.sun.star.form.component.Form");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_HTMLFORM, "com.sun.star.form.component.HTMLForm");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_DATAFORM, "com.sun.star.form.component.DataForm");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_TEXTFIELD, "com.sun.star.form.component.TextField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_LISTBOX, "com.sun.star.form.component.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_COMBOBOX, "com.sun.star.form.component.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_RADIOBUTTON, "com.sun.star.form.component.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_GROUPBOX, "com.sun.star.form.component.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_FIXEDTEXT, "com.sun.star.form.component.FixedText");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_COMMANDBUTTON, "com.sun.star.form.component.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_CHECKBOX, "com.sun.star.form.component.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_GRIDCONTROL, "com.sun.star.form.component.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_IMAGEBUTTON, "com.sun.star.form.component.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_FILECONTROL, "com.sun.star.form.component.FileControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_TIMEFIELD, "com.sun.star.form.component.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_DATEFIELD, "com.sun.star.form.component.DateField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_NUMERICFIELD, "com.sun.star.form.component.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_CURRENCYFIELD, "com.sun.star.form.component.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_PATTERNFIELD, "com.sun.star.form.component.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_HIDDENCONTROL, "com.sun.star.form.component.HiddenControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_COMPONENT_FORMATTEDFIELD, "com.sun.star.form.component.FormattedField");

IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_IMAGECONTROL, "com.sun.star.form.component.DatabaseImageControl" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON, "com.sun.star.form.component.DatabaseRadioButton" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_CHECKBOX, "com.sun.star.form.component.DatabaseCheckBox" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_LISTBOX, "com.sun.star.form.component.DatabaseListBox" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_COMBOBOX, "com.sun.star.form.component.DatabaseComboBox" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_TEXTFIELD, "com.sun.star.form.component.DatabaseTextField" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_DATEFIELD, "com.sun.star.form.component.DatabaseDateField" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_TIMEFIELD, "com.sun.star.form.component.DatabaseTimeField" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD, "com.sun.star.form.component.DatabaseNumericField" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD, "com.sun.star.form.component.DatabaseCurrencyField" );
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD, "com.sun.star.form.component.DatabasePatternField" );

IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_TEXTFIELD, "com.sun.star.form.control.TextField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_LISTBOX, "com.sun.star.form.control.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_COMBOBOX, "com.sun.star.form.control.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_RADIOBUTTON, "com.sun.star.form.control.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_GROUPBOX, "com.sun.star.form.control.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_FIXEDTEXT, "com.sun.star.form.control.FixedText");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_COMMANDBUTTON, "com.sun.star.form.control.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_CHECKBOX, "com.sun.star.form.control.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_GRIDCONTROL, "com.sun.star.form.control.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_IMAGEBUTTON, "com.sun.star.form.control.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_TIMEFIELD, "com.sun.star.form.control.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_DATEFIELD, "com.sun.star.form.control.DateField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_NUMERICFIELD, "com.sun.star.form.control.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_CURRENCYFIELD, "com.sun.star.form.control.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_PATTERNFIELD, "com.sun.star.form.control.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_IMAGECONTROL, "com.sun.star.form.control.ImageControl");
IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_CONTROL_FORMATTEDFIELD, "com.sun.star.form.control.FormattedField");

IMPLEMENT_CONSTASCII_USTRING(FRM_SUN_FORMS_COLLECTION, "com.sun.star.form.Forms");

IMPLEMENT_CONSTASCII_USTRING(FRM_NUMBER_FORMATTER, "com.sun.star.util.NumberFormatter");
IMPLEMENT_CONSTASCII_USTRING(FRM_NUMBER_FORMATS_SUPPLIER, "com.sun.star.util.NumberFormatsSupplier");

IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_ROWSET, "com.sun.star.sdb.RowSet");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_CONNECTION, "com.sun.star.sdb.Connection");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDBC_STATEMENT, "com.sun.star.sdbc.Statement");

IMPLEMENT_CONSTASCII_USTRING(SRV_AWT_POINTER, "com.sun.star.awt.Pointer");
IMPLEMENT_CONSTASCII_USTRING(SRV_AWT_IMAGEPRODUCER, "com.sun.star.awt.ImageProducer");

// -----------------------
// common
// -----------------------
IMPLEMENT_CONSTASCII_USTRING( FRM_SUN_FORMCOMPONENT, "com.sun.star.form.FormComponent" );

//.........................................................................
}
//... namespace frm .......................................................
}//namespace bifilter
#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace binfilter {//STRIP009

namespace starregistry	= ::com::sun::star::registry;
namespace staruno		= ::com::sun::star::uno;
namespace starlang		= ::com::sun::star::lang;

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
DECLARE_SERVICE_INFO(OFormattedFieldWrapper)//STRIP008 ;
    // this is for a service, which is instantiated through the EditModel service name
    // and which acts mostly as Edit (mostly means : if somebody uses XPersistObject::read immediately after
    // the object was instantiated and the stream contains a FormattedModel, it switches permanently to
    // formatted.)
namespace frm { \
    extern Reference<XInterface> SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException); \
}

DECLARE_SERVICE_INFO(OFormsCollection)
DECLARE_SERVICE_INFO(ImageProducer)

//---------------------------------------------------------------------------------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

static Sequence< ::rtl::OUString >						s_aClassImplementationNames;
static Sequence<Sequence< ::rtl::OUString > >	s_aClassServiceNames;
static Sequence<sal_Int64>								s_aFactories;
    // need to use sal_Int64 instead of ComponentInstantiation, as ComponentInstantiation has no cppuType, so
    // it can't be used with sequences

//---------------------------------------------------------------------------------------
void registerClassInfo(
        ::rtl::OUString _rClassImplName,								// the ImplName of the class
        const Sequence< ::rtl::OUString >& _rServiceNames,		// the services supported by this class
        ::cppu::ComponentInstantiation _pCreateFunction					// the method for instantiating such a class
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
    REGISTER_CLASS3( ORadioButtonModel, FRM_COMPONENT_RADIOBUTTON, FRM_SUN_COMPONENT_RADIOBUTTON, FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON );
    // - CheckBox
    REGISTER_CLASS3( OCheckBoxModel, FRM_COMPONENT_CHECKBOX, FRM_SUN_COMPONENT_CHECKBOX, FRM_SUN_COMPONENT_DATABASE_CHECKBOX );
    // - ListBox
    REGISTER_CLASS3( OListBoxModel, FRM_COMPONENT_LISTBOX, FRM_SUN_COMPONENT_LISTBOX, FRM_SUN_COMPONENT_DATABASE_LISTBOX );
    // - ComboBox
    REGISTER_CLASS3( OComboBoxModel, FRM_COMPONENT_COMBOBOX, FRM_SUN_COMPONENT_COMBOBOX, FRM_SUN_COMPONENT_DATABASE_COMBOBOX );
    // - EditControl
    REGISTER_CLASS3( OEditModel, FRM_COMPONENT_TEXTFIELD, FRM_SUN_COMPONENT_TEXTFIELD, FRM_SUN_COMPONENT_DATABASE_TEXTFIELD );
    // - DateControl
    REGISTER_CLASS3( ODateModel, FRM_COMPONENT_DATEFIELD, FRM_SUN_COMPONENT_DATEFIELD, FRM_SUN_COMPONENT_DATABASE_DATEFIELD );
    // - TimeControl
    REGISTER_CLASS3( OTimeModel, FRM_COMPONENT_TIMEFIELD, FRM_SUN_COMPONENT_TIMEFIELD, FRM_SUN_COMPONENT_DATABASE_TIMEFIELD );
    // - NumericField
    REGISTER_CLASS3( ONumericModel, FRM_COMPONENT_NUMERICFIELD, FRM_SUN_COMPONENT_NUMERICFIELD, FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD );
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
    aServices.realloc(3);
    aServices.getArray()[0] = frm::FRM_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[1] = frm::FRM_SUN_COMPONENT_FORMATTEDFIELD;
    aServices.getArray()[2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseFormattedField");
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
    REGISTER_CLASS1(ImageProducer, SRV_AWT_IMAGEPRODUCER);
}

//---------------------------------------------------------------------------------------
void registerServiceProvider(const ::rtl::OUString& _rServiceImplName, const Sequence< ::rtl::OUString >& _rServices, starregistry::XRegistryKey* _pKey)
{
    ::rtl::OUString sMainKeyName = ::rtl::OUString::createFromAscii("/");
    sMainKeyName += _rServiceImplName;
    sMainKeyName += ::rtl::OUString::createFromAscii("/UNO/SERVICES");
    Reference<starregistry::XRegistryKey> xNewKey = _pKey->createKey(sMainKeyName);
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

//---------------------------------------------------------------------------------------
void SAL_CALL createRegistryInfo_FORMS()
{
    static sal_Bool bInit = sal_False;
    if (!bInit)
    {
        createRegistryInfo_ODatabaseForm();
        createRegistryInfo_OFilterControl();
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------
void SAL_CALL component_getImplementationEnvironment(const sal_Char** _ppEnvTypeName, uno_Environment** _ppEnv)
{
    *_ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
sal_Bool SAL_CALL component_writeInfo(void* _pServiceManager, starregistry::XRegistryKey* _pRegistryKey)
{
    if (_pRegistryKey)
    {
        try
        {
            // the real way - use the OModule
            createRegistryInfo_FORMS();
            if ( !::binfilter::frm::OFormsModule::writeComponentInfos(//STRIP008 			if ( !::frm::OFormsModule::writeComponentInfos(
                    static_cast<XMultiServiceFactory*>( _pServiceManager ),
                    static_cast<XRegistryKey*>( _pRegistryKey ) )
                )
                return sal_False;

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
        catch (starregistry::InvalidRegistryException &)
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

    // the real way - use the OModule
    if ( !pRet )
    {
        createRegistryInfo_FORMS();
        {
            // let the module look for the component
            Reference< XInterface > xRet;
            xRet = ::binfilter::frm::OFormsModule::getComponentFactory(//STRIP008 			xRet = ::frm::OFormsModule::getComponentFactory(
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
}
