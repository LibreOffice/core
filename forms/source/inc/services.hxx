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

#ifndef _FRM_SERVICES_HXX_
#define _FRM_SERVICES_HXX_

#include <rtl/ustring.hxx>
#include "frm_strings.hxx"

//.........................................................................
namespace frm
{
//.........................................................................

    FORMS_CONSTASCII_STRING( AWT_CONTROL_TEXTFIELD, "com.sun.star.awt.TextField" );

    FORMS_CONSTASCII_STRING( VCL_CONTROL_EDIT, "stardiv.vcl.control.Edit" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_LISTBOX, "stardiv.vcl.control.ListBox" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_COMBOBOX, "stardiv.vcl.control.ComboBox" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_RADIOBUTTON, "stardiv.vcl.control.RadioButton" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_GROUPBOX, "stardiv.vcl.control.GroupBox" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_FIXEDTEXT, "stardiv.vcl.control.FixedText" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_COMMANDBUTTON, "stardiv.vcl.control.Button" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_CHECKBOX, "stardiv.vcl.control.CheckBox" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_IMAGEBUTTON, "stardiv.vcl.control.ImageButton" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_FILECONTROL, "stardiv.vcl.control.FileControl" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_TIMEFIELD, "stardiv.vcl.control.TimeField" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_DATEFIELD, "stardiv.vcl.control.DateField" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_NUMERICFIELD, "stardiv.vcl.control.NumericField" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_CURRENCYFIELD, "stardiv.vcl.control.CurrencyField" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_PATTERNFIELD, "stardiv.vcl.control.PatternField" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_FORMATTEDFIELD, "stardiv.vcl.control.FormattedField" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_IMAGECONTROL, "stardiv.vcl.control.ImageControl" );

    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_EDIT,             "stardiv.vcl.controlmodel.Edit"              );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_LISTBOX,          "stardiv.vcl.controlmodel.ListBox"           );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_COMBOBOX,         "stardiv.vcl.controlmodel.ComboBox"          );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_RADIOBUTTON,      "stardiv.vcl.controlmodel.RadioButton"       );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_GROUPBOX,         "stardiv.vcl.controlmodel.GroupBox"          );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_FIXEDTEXT,        "stardiv.vcl.controlmodel.FixedText"         );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_COMMANDBUTTON,    "stardiv.vcl.controlmodel.Button"            );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_CHECKBOX,         "stardiv.vcl.controlmodel.CheckBox"          );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_IMAGEBUTTON,      "stardiv.vcl.controlmodel.ImageButton"       );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_FILECONTROL,      "stardiv.vcl.controlmodel.FileControl"       );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_TIMEFIELD,        "stardiv.vcl.controlmodel.TimeField"         );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_DATEFIELD,        "stardiv.vcl.controlmodel.DateField"         );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_NUMERICFIELD,     "stardiv.vcl.controlmodel.NumericField"      );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_CURRENCYFIELD,    "stardiv.vcl.controlmodel.CurrencyField"     );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_PATTERNFIELD,     "stardiv.vcl.controlmodel.PatternField"      );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_FORMATTEDFIELD,   "stardiv.vcl.controlmodel.FormattedField"    );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_IMAGECONTROL,     "stardiv.vcl.controlmodel.ImageControl"      );

    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_SCROLLBAR,        "com.sun.star.awt.UnoControlScrollBarModel"  );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_SCROLLBAR,             "com.sun.star.awt.UnoControlScrollBar"       );
    FORMS_CONSTASCII_STRING( VCL_CONTROLMODEL_SPINBUTTON,       "com.sun.star.awt.UnoControlSpinButtonModel" );
    FORMS_CONSTASCII_STRING( VCL_CONTROL_SPINBUTTON,            "com.sun.star.awt.UnoControlSpinButton"      );

    // -----------------------
    // service names for compatibility
    // -----------------------
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_FORM, "stardiv.one.form.component.Form" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_EDIT, "stardiv.one.form.component.Edit" );           // compatibility
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_TEXTFIELD, "stardiv.one.form.component.TextField" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_LISTBOX, "stardiv.one.form.component.ListBox" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_COMBOBOX, "stardiv.one.form.component.ComboBox" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_RADIOBUTTON, "stardiv.one.form.component.RadioButton" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_GROUPBOX, "stardiv.one.form.component.GroupBox" );       // compatibility
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_FIXEDTEXT, "stardiv.one.form.component.FixedText" );     // compatibility
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_COMMANDBUTTON, "stardiv.one.form.component.CommandButton" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_CHECKBOX, "stardiv.one.form.component.CheckBox" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_GRID, "stardiv.one.form.component.Grid" );           // compatibility
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_GRIDCONTROL, "stardiv.one.form.component.GridControl" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_IMAGEBUTTON, "stardiv.one.form.component.ImageButton" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_FILECONTROL, "stardiv.one.form.component.FileControl" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_TIMEFIELD, "stardiv.one.form.component.TimeField" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_DATEFIELD, "stardiv.one.form.component.DateField" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_NUMERICFIELD, "stardiv.one.form.component.NumericField" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_CURRENCYFIELD, "stardiv.one.form.component.CurrencyField" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_PATTERNFIELD, "stardiv.one.form.component.PatternField" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_HIDDEN, "stardiv.one.form.component.Hidden" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_HIDDENCONTROL, "stardiv.one.form.component.HiddenControl" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_IMAGECONTROL, "stardiv.one.form.component.ImageControl" );
    FORMS_CONSTASCII_STRING( FRM_COMPONENT_FORMATTEDFIELD, "stardiv.one.form.component.FormattedField" );

    // <compatibility_I>
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_COMMANDBUTTON,   "stardiv.one.form.control.CommandButton" );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_RADIOBUTTON,     "stardiv.one.form.control.RadioButton"   );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_CHECKBOX,        "stardiv.one.form.control.CheckBox"      );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_EDIT,            "stardiv.one.form.control.Edit"          );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_LISTBOX,         "stardiv.one.form.control.ListBox"       );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_COMBOBOX,        "stardiv.one.form.control.ComboBox"      );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_GROUPBOX,        "stardiv.one.form.control.GroupBox"      );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_TEXTFIELD,       "stardiv.one.form.control.TextField"     );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_GRID,            "stardiv.one.form.control.Grid"          );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_IMAGEBUTTON,     "stardiv.one.form.control.ImageButton"   );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_TIMEFIELD,       "stardiv.one.form.control.TimeField"     );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_DATEFIELD,       "stardiv.one.form.control.DateField"     );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_NUMERICFIELD,    "stardiv.one.form.control.NumericField"  );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_CURRENCYFIELD,   "stardiv.one.form.control.CurrencyField" );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_PATTERNFIELD,    "stardiv.one.form.control.PatternField"  );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_IMAGECONTROL,    "stardiv.one.form.control.ImageControl"  );
    FORMS_CONSTASCII_STRING( STARDIV_ONE_FORM_CONTROL_FORMATTEDFIELD,  "stardiv.one.form.control.FormattedField");
    // </compatibility_I>

    // -----------------------
    // new (sun) service names
    // -----------------------
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_FORM,            "com.sun.star.form.component.Form"              );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_HTMLFORM,        "com.sun.star.form.component.HTMLForm"          );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATAFORM,        "com.sun.star.form.component.DataForm"          );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_TEXTFIELD,       "com.sun.star.form.component.TextField"         );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_LISTBOX,         "com.sun.star.form.component.ListBox"           );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_COMBOBOX,        "com.sun.star.form.component.ComboBox"          );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_RADIOBUTTON,     "com.sun.star.form.component.RadioButton"       );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_GROUPBOX,        "com.sun.star.form.component.GroupBox"          );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_FIXEDTEXT,       "com.sun.star.form.component.FixedText"         );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_COMMANDBUTTON,   "com.sun.star.form.component.CommandButton"     );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_CHECKBOX,        "com.sun.star.form.component.CheckBox"          );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_GRIDCONTROL,     "com.sun.star.form.component.GridControl"       );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_IMAGEBUTTON,     "com.sun.star.form.component.ImageButton"       );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_FILECONTROL,     "com.sun.star.form.component.FileControl"       );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_TIMEFIELD,       "com.sun.star.form.component.TimeField"         );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATEFIELD,       "com.sun.star.form.component.DateField"         );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_NUMERICFIELD,    "com.sun.star.form.component.NumericField"      );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_CURRENCYFIELD,   "com.sun.star.form.component.CurrencyField"     );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_PATTERNFIELD,    "com.sun.star.form.component.PatternField"      );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_HIDDENCONTROL,   "com.sun.star.form.component.HiddenControl"     );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_FORMATTEDFIELD,  "com.sun.star.form.component.FormattedField"    );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_SCROLLBAR,       "com.sun.star.form.component.ScrollBar"         );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_SPINBUTTON,      "com.sun.star.form.component.SpinButton"        );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_RICHTEXTCONTROL, "com.sun.star.form.component.RichTextControl"   );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_NAVTOOLBAR,      "com.sun.star.form.component.NavigationToolBar" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_SUBMITBUTTON,    "com.sun.star.form.component.SubmitButton"      );

    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_IMAGECONTROL, "com.sun.star.form.component.DatabaseImageControl" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_RADIOBUTTON, "com.sun.star.form.component.DatabaseRadioButton" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_CHECKBOX, "com.sun.star.form.component.DatabaseCheckBox" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_LISTBOX, "com.sun.star.form.component.DatabaseListBox" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_COMBOBOX, "com.sun.star.form.component.DatabaseComboBox" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_FORMATTEDFIELD, "com.sun.star.form.component.DatabaseFormattedField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_TEXTFIELD, "com.sun.star.form.component.DatabaseTextField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_DATEFIELD, "com.sun.star.form.component.DatabaseDateField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_TIMEFIELD, "com.sun.star.form.component.DatabaseTimeField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD, "com.sun.star.form.component.DatabaseNumericField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD, "com.sun.star.form.component.DatabaseCurrencyField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD, "com.sun.star.form.component.DatabasePatternField" );

    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_TEXTFIELD, "com.sun.star.form.control.TextField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_LISTBOX, "com.sun.star.form.control.ListBox" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_COMBOBOX, "com.sun.star.form.control.ComboBox" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_RADIOBUTTON, "com.sun.star.form.control.RadioButton" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_GROUPBOX, "com.sun.star.form.control.GroupBox" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_FIXEDTEXT, "com.sun.star.form.control.FixedText" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_COMMANDBUTTON, "com.sun.star.form.control.CommandButton" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_CHECKBOX, "com.sun.star.form.control.CheckBox" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_GRIDCONTROL, "com.sun.star.form.control.GridControl" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_IMAGEBUTTON, "com.sun.star.form.control.ImageButton" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_TIMEFIELD, "com.sun.star.form.control.TimeField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_DATEFIELD, "com.sun.star.form.control.DateField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_NUMERICFIELD, "com.sun.star.form.control.NumericField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_CURRENCYFIELD, "com.sun.star.form.control.CurrencyField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_PATTERNFIELD, "com.sun.star.form.control.PatternField" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_IMAGECONTROL, "com.sun.star.form.control.ImageControl" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_FORMATTEDFIELD, "com.sun.star.form.control.FormattedField"  );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_RICHTEXTCONTROL,"com.sun.star.form.control.RichTextControl" );
    FORMS_CONSTASCII_STRING( FRM_SUN_CONTROL_SUBMITBUTTON,   "com.sun.star.form.control.SubmitButton"    );

    FORMS_CONSTASCII_STRING( FRM_SUN_FORMS_COLLECTION, "com.sun.star.form.Forms" );

    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_CHECK_BOX,      "com.sun.star.form.binding.BindableDatabaseCheckBox" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_COMBO_BOX,      "com.sun.star.form.binding.BindableDatabaseComboBox" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_FORMATTED_FIELD,"com.sun.star.form.binding.BindableDatabaseFormattedField" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_LIST_BOX,       "com.sun.star.form.binding.BindableDatabaseListBox" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_NUMERIC_FIELD,  "com.sun.star.form.binding.BindableDatabaseNumericField" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_RADIO_BUTTON,   "com.sun.star.form.binding.BindableDatabaseRadioButton" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_TEXT_FIELD,     "com.sun.star.form.binding.BindableDatabaseTextField" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_DATE_FIELD,     "com.sun.star.form.binding.BindableDatabaseDateField" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATABASE_TIME_FIELD,     "com.sun.star.form.binding.BindableDatabaseTimeField" );

    FORMS_CONSTASCII_STRING( BINDABLE_CONTROL_MODEL,            "com.sun.star.form.binding.BindableControlModel" );
    FORMS_CONSTASCII_STRING( BINDABLE_INTEGER_VALUE_RANGE,      "com.sun.star.form.binding.BindableIntegerValueRange" );
    FORMS_CONSTASCII_STRING( BINDABLE_DATA_AWARE_CONTROL_MODEL, "com.sun.star.form.binding.BindableDataAwareControlModel" );
    FORMS_CONSTASCII_STRING( DATA_AWARE_CONTROL_MODEL,          "com.sun.star.form.binding.DataAwareControlModel" );
    FORMS_CONSTASCII_STRING( VALIDATABLE_CONTROL_MODEL,         "com.sun.star.form.binding.ValidatableControlModel" );
    FORMS_CONSTASCII_STRING( VALIDATABLE_BINDABLE_CONTROL_MODEL,"com.sun.star.form.binding.ValidatableBindableControlModel" );

    // -----------------------
    // common
    // -----------------------
    FORMS_CONSTASCII_STRING( FRM_SUN_FORMCOMPONENT, "com.sun.star.form.FormComponent" );

    // -----------------------
    // misc
    // -----------------------
    FORMS_CONSTASCII_STRING( SRV_AWT_POINTER, "com.sun.star.awt.Pointer" );
    FORMS_CONSTASCII_STRING( SRV_AWT_IMAGEPRODUCER, "com.sun.star.awt.ImageProducer" );
    FORMS_CONSTASCII_STRING( FRM_NUMBER_FORMATS_SUPPLIER, "com.sun.star.util.NumberFormatsSupplier" );

    FORMS_CONSTASCII_STRING( SRV_SDB_ROWSET, "com.sun.star.sdb.RowSet" );
    FORMS_CONSTASCII_STRING( SRV_SDB_CONNECTION, "com.sun.star.sdb.Connection" );


//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_SERVICES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
