/*************************************************************************
 *
 *  $RCSfile: fmservs.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-31 09:30:57 $
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

#pragma hdrstop

#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _FM_STATIC_HXX_
#include "fmstatic.hxx"
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

IMPLEMENT_CONSTASCII_USTRING(AWT_CONTROL_TEXTFIELD,"com.sun.star.awt.TextField");

IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_EDIT,"stardiv.vcl.control.Edit");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_LISTBOX,"stardiv.vcl.control.ListBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_COMBOBOX,"stardiv.vcl.control.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_RADIOBUTTON,"stardiv.vcl.control.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_GROUPBOX,"stardiv.vcl.control.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_FIXEDTEXT,"stardiv.vcl.control.FixedText");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_COMMANDBUTTON,"stardiv.vcl.control.Button");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_CHECKBOX,"stardiv.vcl.control.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_IMAGEBUTTON,"stardiv.vcl.control.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_FILECONTROL,"stardiv.vcl.control.FileControl");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_TIMEFIELD,"stardiv.vcl.control.TimeField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_DATEFIELD,"stardiv.vcl.control.DateField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_NUMERICFIELD,"stardiv.vcl.control.NumericField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_CURRENCYFIELD,"stardiv.vcl.control.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_PATTERNFIELD,"stardiv.vcl.control.PatternField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_FORMATTEDFIELD,"stardiv.vcl.control.FormattedField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROL_IMAGECONTROL,"stardiv.vcl.control.ImageControl");

IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_EDIT,"stardiv.vcl.controlmodel.Edit");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_LISTBOX,"stardiv.vcl.controlmodel.ListBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_COMBOBOX,"stardiv.vcl.controlmodel.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_RADIOBUTTON,"stardiv.vcl.controlmodel.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_GROUPBOX,"stardiv.vcl.controlmodel.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_FIXEDTEXT,"stardiv.vcl.controlmodel.FixedText");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_COMMANDBUTTON,"stardiv.vcl.controlmodel.Button");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_CHECKBOX,"stardiv.vcl.controlmodel.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_IMAGEBUTTON,"stardiv.vcl.controlmodel.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_FILECONTROL,"stardiv.vcl.controlmodel.FileControl");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_TIMEFIELD,"stardiv.vcl.controlmodel.TimeField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_DATEFIELD,"stardiv.vcl.controlmodel.DateField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_NUMERICFIELD,"stardiv.vcl.controlmodel.NumericField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_CURRENCYFIELD,"stardiv.vcl.controlmodel.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_PATTERNFIELD,"stardiv.vcl.controlmodel.PatternField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_FORMATTEDFIELD,"stardiv.vcl.controlmodel.FormattedField");
IMPLEMENT_CONSTASCII_USTRING(VCL_CONTROLMODEL_IMAGECONTROL,"stardiv.vcl.controlmodel.ImageControl");

// -----------------------
// service names for compatibility
// -----------------------
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_FORM,"stardiv.one.form.component.Form");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_EDIT,"stardiv.one.form.component.Edit");          // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_TEXTFIELD,"stardiv.one.form.component.TextField");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_LISTBOX,"stardiv.one.form.component.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_COMBOBOX,"stardiv.one.form.component.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_RADIOBUTTON,"stardiv.one.form.component.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_GROUPBOX,"stardiv.one.form.component.GroupBox");      // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_FIXEDTEXT,"stardiv.one.form.component.FixedText");        // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_COMMANDBUTTON,"stardiv.one.form.component.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_CHECKBOX,"stardiv.one.form.component.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_GRID,"stardiv.one.form.component.Grid");          // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_GRIDCONTROL,"stardiv.one.form.component.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_IMAGEBUTTON,"stardiv.one.form.component.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_FILECONTROL,"stardiv.one.form.component.FileControl");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_TIMEFIELD,"stardiv.one.form.component.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_DATEFIELD,"stardiv.one.form.component.DateField");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_NUMERICFIELD,"stardiv.one.form.component.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_CURRENCYFIELD,"stardiv.one.form.component.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_PATTERNFIELD,"stardiv.one.form.component.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_HIDDEN,"stardiv.one.form.component.Hidden");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_HIDDENCONTROL,"stardiv.one.form.component.HiddenControl");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_IMAGECONTROL,"stardiv.one.form.component.ImageControl");
IMPLEMENT_CONSTASCII_USTRING(FM_COMPONENT_FORMATTEDFIELD,"stardiv.one.form.component.FormattedField");

IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_EDIT,"stardiv.one.form.control.Edit");                  // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_TEXTFIELD,"stardiv.one.form.control.TextField");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_LISTBOX,"stardiv.one.form.control.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_COMBOBOX,"stardiv.one.form.control.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_RADIOBUTTON,"stardiv.one.form.control.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_GROUPBOX,"stardiv.one.form.control.GroupBox");              // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_FIXEDTEXT,"stardiv.one.form.control.FixedText");            // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_COMMANDBUTTON,"stardiv.one.form.control.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_CHECKBOX,"stardiv.one.form.control.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_GRID,"stardiv.one.form.control.Grid");                  // compatibility
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_GRIDCONTROL,"stardiv.one.form.control.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_IMAGEBUTTON,"stardiv.one.form.control.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_TIMEFIELD,"stardiv.one.form.control.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_DATEFIELD,"stardiv.one.form.control.DateField");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_NUMERICFIELD,"stardiv.one.form.control.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_CURRENCYFIELD,"stardiv.one.form.control.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_PATTERNFIELD,"stardiv.one.form.control.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_IMAGECONTROL,"stardiv.one.form.control.ImageControl");
IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_FORMATTEDFIELD,"stardiv.one.form.control.FormattedField");

// -----------------------
// new (sun) service names
// -----------------------
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_FORM,"com.sun.star.form.component.Form");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_TEXTFIELD,"com.sun.star.form.component.TextField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_LISTBOX,"com.sun.star.form.component.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_COMBOBOX,"com.sun.star.form.component.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_RADIOBUTTON,"com.sun.star.form.component.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_GROUPBOX,"com.sun.star.form.component.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_FIXEDTEXT,"com.sun.star.form.component.FixedText");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_COMMANDBUTTON,"com.sun.star.form.component.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_CHECKBOX,"com.sun.star.form.component.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_GRIDCONTROL,"com.sun.star.form.component.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_IMAGEBUTTON,"com.sun.star.form.component.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_FILECONTROL,"com.sun.star.form.component.FileControl");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_TIMEFIELD,"com.sun.star.form.component.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_DATEFIELD,"com.sun.star.form.component.DateField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_NUMERICFIELD,"com.sun.star.form.component.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_CURRENCYFIELD,"com.sun.star.form.component.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_PATTERNFIELD,"com.sun.star.form.component.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_HIDDENCONTROL,"com.sun.star.form.component.HiddenControl");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_IMAGECONTROL,"com.sun.star.form.component.ImageControl");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_FORMATTEDFIELD,"com.sun.star.form.component.FormattedField");

IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_TEXTFIELD,"com.sun.star.form.control.TextField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_LISTBOX,"com.sun.star.form.control.ListBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_COMBOBOX,"com.sun.star.form.control.ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_RADIOBUTTON,"com.sun.star.form.control.RadioButton");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_GROUPBOX,"com.sun.star.form.control.GroupBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_FIXEDTEXT,"com.sun.star.form.control.FixedText");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_COMMANDBUTTON,"com.sun.star.form.control.CommandButton");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_CHECKBOX,"com.sun.star.form.control.CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_GRIDCONTROL,"com.sun.star.form.control.GridControl");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_IMAGEBUTTON,"com.sun.star.form.control.ImageButton");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_TIMEFIELD,"com.sun.star.form.control.TimeField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_DATEFIELD,"com.sun.star.form.control.DateField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_NUMERICFIELD,"com.sun.star.form.control.NumericField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_CURRENCYFIELD,"com.sun.star.form.control.CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_PATTERNFIELD,"com.sun.star.form.control.PatternField");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_IMAGECONTROL,"com.sun.star.form.control.ImageControl");
IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_FORMATTEDFIELD,"com.sun.star.form.control.FormattedField");

IMPLEMENT_CONSTASCII_USTRING(FM_DATA_ENGINE,"com.sun.star.data.DatabaseEngine");
IMPLEMENT_CONSTASCII_USTRING(FM_NUMBER_FORMATTER,"com.sun.star.util.NumberFormatter");
IMPLEMENT_CONSTASCII_USTRING(FM_FORM_CONTROLLER,"com.sun.star.form.controller.FormController");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_ROWSET,"com.sun.star.sdb.RowSet");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_CONNECTION,"com.sun.star.sdb.Connection");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDBC_STATEMENT,"com.sun.star.sdbc.Statement");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_DATABASE_ENVIRONMENT,"com.sun.star.sdb.DatabaseEnvironment");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_DATABASE_ACCESS_CONNECTION,"com.sun.star.sdb.DatabaseAccessConnection");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_DATABASE_ACCESS_CONTEXT,"com.sun.star.sdb.DatabaseAccessContext");
IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_INTERACTION_HANDLER,"com.sun.star.sdb.InteractionHandler");


#define DECL_SERVICE(ImplName)                      \
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_NewInstance_Impl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &) throw( ::com::sun::star::uno::Exception );

#define REGISTER_SERVICE(ImplName, ServiceName)                         \
    sString = (ServiceName);                                        \
    xSingleFactory = ::cppu::createSingleFactory(xServiceFactory,               \
                        ::rtl::OUString(), ImplName##_NewInstance_Impl,             \
                        ::com::sun::star::uno::Sequence< ::rtl::OUString>(&sString, 1));    \
    if (xSingleFactory.is())                                            \
        xSet->insert(::com::sun::star::uno::makeAny(xSingleFactory));

// Deklaration der ServiceMethoden
// ------------------------------------------------------------------------
DECL_SERVICE(FmXGridControl);
DECL_SERVICE(FmXFormController);

// ------------------------------------------------------------------------
void ImplSmartRegisterUnoServices()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xServiceFactory(::comphelper::getProcessServiceFactory(), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XSet >  xSet(xServiceFactory, ::com::sun::star::uno::UNO_QUERY);
    if (!xSet.is())
        return;

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aServices;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >  xSingleFactory;

    ::rtl::OUString sString;

    REGISTER_SERVICE(FmXFormController, FM_FORM_CONTROLLER);

    // DBGridControl
    // ------------------------------------------------------------------------
    REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRID);  // compatibility
    REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRIDCONTROL);
    REGISTER_SERVICE(FmXGridControl, FM_SUN_CONTROL_GRIDCONTROL);


};


