/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmservs.cxx,v $
 * $Revision: 1.17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <com/sun/star/container/XSet.hpp>
#include "fmstatic.hxx"
#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>

namespace svxform
{

    // -----------------------
    // service names for compatibility
    // -----------------------
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

    IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_GRID,"stardiv.one.form.control.Grid");                  // compatibility
    IMPLEMENT_CONSTASCII_USTRING(FM_CONTROL_GRIDCONTROL,"stardiv.one.form.control.GridControl");

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
    IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_IMAGECONTROL,"com.sun.star.form.component.DatabaseImageControl");
    IMPLEMENT_CONSTASCII_USTRING(FM_SUN_COMPONENT_FORMATTEDFIELD,"com.sun.star.form.component.FormattedField");
    IMPLEMENT_CONSTASCII_USTRING( FM_SUN_COMPONENT_SCROLLBAR,    "com.sun.star.form.component.ScrollBar"         );
    IMPLEMENT_CONSTASCII_USTRING( FM_SUN_COMPONENT_SPINBUTTON,   "com.sun.star.form.component.SpinButton"        );
    IMPLEMENT_CONSTASCII_USTRING( FM_SUN_COMPONENT_NAVIGATIONBAR,"com.sun.star.form.component.NavigationToolBar" );

    IMPLEMENT_CONSTASCII_USTRING(FM_SUN_CONTROL_GRIDCONTROL,"com.sun.star.form.control.GridControl");

    IMPLEMENT_CONSTASCII_USTRING(FM_NUMBER_FORMATTER,"com.sun.star.util.NumberFormatter");
    IMPLEMENT_CONSTASCII_USTRING(FM_FORM_CONTROLLER,"com.sun.star.form.FormController");
    IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_CONNECTION,"com.sun.star.sdb.Connection");
    IMPLEMENT_CONSTASCII_USTRING(SRV_SDB_INTERACTION_HANDLER,"com.sun.star.sdb.InteractionHandler");
}   // namespace svxform

// ------------------------------------------------------------------------
#define DECL_SERVICE(ImplName)                      \
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_NewInstance_Impl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &) throw( ::com::sun::star::uno::Exception );

#define REGISTER_SERVICE(ImplName, ServiceName)                         \
    sString = (ServiceName);                                        \
    xSingleFactory = ::cppu::createSingleFactory(xServiceFactory,               \
                        ::rtl::OUString(), ImplName##_NewInstance_Impl,             \
                        ::com::sun::star::uno::Sequence< ::rtl::OUString>(&sString, 1));    \
    if (xSingleFactory.is())                                            \
        xSet->insert(::com::sun::star::uno::makeAny(xSingleFactory));


    DECL_SERVICE( FmXGridControl )
    DECL_SERVICE( FmXFormController )


// ------------------------------------------------------------------------
namespace svxform
{

#define DECL_SELFAWARE_SERVICE( ClassName )                     \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ClassName##_Create(      \
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );  \
    ::rtl::OUString SAL_CALL ClassName##_GetImplementationName();                                           \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ClassName##_GetSupportedServiceNames();     \


#define REGISTER_SELFAWARE_SERVICE( ClassName )                     \
    xSingleFactory = ::cppu::createSingleFactory( xServiceFactory,  \
                        ClassName##_GetImplementationName(),        \
                        ClassName##_Create,                         \
                        ClassName##_GetSupportedServiceNames()      \
                     );                                             \
    if ( xSingleFactory.is() )                                      \
        xSet->insert( ::com::sun::star::uno::makeAny( xSingleFactory ) );


    // ------------------------------------------------------------------------
    DECL_SELFAWARE_SERVICE( OAddConditionDialog )

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

        // ------------------------------------------------------------------------
        // FormController
        REGISTER_SERVICE(FmXFormController, FM_FORM_CONTROLLER);

        // ------------------------------------------------------------------------
        // FormController
        REGISTER_SELFAWARE_SERVICE( OAddConditionDialog );

        // ------------------------------------------------------------------------
        // DBGridControl
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRID);  // compatibility
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRIDCONTROL);
        REGISTER_SERVICE(FmXGridControl, FM_SUN_CONTROL_GRIDCONTROL);


    };


}   // namespace svxform
