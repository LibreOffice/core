/*************************************************************************
 *
 *  $RCSfile: registerservices.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:09 $
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

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#include <cppuhelper/factory.hxx>

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/macros.hxx>

#include <toolkit/awt/vclxtoolkit.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/awt/vclxpointer.hxx>
#include <toolkit/awt/vclxprinter.hxx>

#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <toolkit/controls/unocontrolcontainermodel.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <toolkit/controls/stdtabcontrollermodel.hxx>


#define REGISTER_SERVICES( ImplName, ServiceName1, ServiceName2 ) \
    xNewKey = pRegistryKey->createKey( ::rtl::OUString::createFromAscii( "/stardiv.Toolkit." #ImplName "/UNO/SERVICES" ) ); \
    xNewKey->createKey( ServiceName1 ); \
    xNewKey->createKey( ServiceName2 );


#define IMPL_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ) \
    { return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ImplName ); }

#define CHECKANDCREATEFACTORY( ImplName, ServiceName1, ServiceName2 ) \
    if ( rtl_str_compare( sImplementationName, "stardiv.Toolkit." #ImplName ) == 0 ) \
    { \
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aServiceNames(2); \
        aServiceNames.getArray()[0] = ServiceName1; \
        aServiceNames.getArray()[1] = ServiceName2; \
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xFactory( ::cppu::createSingleFactory( pServiceManager, \
            ::rtl::OUString::createFromAscii( sImplementationName ), ImplName##_CreateInstance, aServiceNames ) ); \
        if ( xFactory.is() ) \
        { \
            xFactory->acquire(); \
            pRet = xFactory.get(); \
        } \
    }


IMPL_CREATEINSTANCE( VCLXToolkit )
IMPL_CREATEINSTANCE( StdTabController )
IMPL_CREATEINSTANCE( StdTabControllerModel )
IMPL_CREATEINSTANCE( UnoButtonControl )
IMPL_CREATEINSTANCE( UnoCheckBoxControl )
IMPL_CREATEINSTANCE( UnoComboBoxControl )
IMPL_CREATEINSTANCE( UnoControlButtonModel )
IMPL_CREATEINSTANCE( UnoControlCheckBoxModel )
IMPL_CREATEINSTANCE( UnoControlComboBoxModel )
IMPL_CREATEINSTANCE( UnoControlContainer )
IMPL_CREATEINSTANCE( UnoControlContainerModel )
IMPL_CREATEINSTANCE( UnoControlCurrencyFieldModel )
IMPL_CREATEINSTANCE( UnoControlDateFieldModel )
IMPL_CREATEINSTANCE( UnoControlEditModel )
IMPL_CREATEINSTANCE( UnoControlFileControlModel )
IMPL_CREATEINSTANCE( UnoControlFixedTextModel )
IMPL_CREATEINSTANCE( UnoControlFormattedFieldModel )
IMPL_CREATEINSTANCE( UnoControlGroupBoxModel )
IMPL_CREATEINSTANCE( UnoControlImageControlModel )
IMPL_CREATEINSTANCE( UnoControlListBoxModel )
IMPL_CREATEINSTANCE( UnoControlNumericFieldModel )
IMPL_CREATEINSTANCE( UnoControlPatternFieldModel )
IMPL_CREATEINSTANCE( UnoControlRadioButtonModel )
IMPL_CREATEINSTANCE( UnoControlTimeFieldModel )
IMPL_CREATEINSTANCE( UnoCurrencyFieldControl )
IMPL_CREATEINSTANCE( UnoDateFieldControl )
IMPL_CREATEINSTANCE( UnoEditControl )
IMPL_CREATEINSTANCE( UnoFileControl )
IMPL_CREATEINSTANCE( UnoFixedTextControl )
IMPL_CREATEINSTANCE( UnoFormattedFieldControl )
IMPL_CREATEINSTANCE( UnoGroupBoxControl )
IMPL_CREATEINSTANCE( UnoImageControlControl )
IMPL_CREATEINSTANCE( UnoListBoxControl )
IMPL_CREATEINSTANCE( UnoNumericFieldControl )
IMPL_CREATEINSTANCE( UnoPatternFieldControl )
IMPL_CREATEINSTANCE( UnoRadioButtonControl )
IMPL_CREATEINSTANCE( UnoTimeFieldControl )
IMPL_CREATEINSTANCE( VCLXMenuBar )
IMPL_CREATEINSTANCE( VCLXPointer )
IMPL_CREATEINSTANCE( VCLXPopupMenu )
IMPL_CREATEINSTANCE( VCLXPrinterServer )


extern "C"
{

void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

sal_Bool SAL_CALL component_writeInfo( void* _pServiceManager, void* _pRegistryKey )
{
    if (_pRegistryKey)
    {
        ::com::sun::star::registry::XRegistryKey * pRegistryKey = reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( _pRegistryKey );
        ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > xNewKey;

        REGISTER_SERVICES( VCLXToolkit, ::rtl::OUString::createFromAscii( szServiceName_Toolkit ), ::rtl::OUString::createFromAscii( szServiceName2_Toolkit ) );
        REGISTER_SERVICES( VCLXPopupMenu, ::rtl::OUString::createFromAscii( szServiceName_PopupMenu ), ::rtl::OUString::createFromAscii( szServiceName2_PopupMenu ) );
        REGISTER_SERVICES( VCLXMenuBar, ::rtl::OUString::createFromAscii( szServiceName_MenuBar ), ::rtl::OUString::createFromAscii( szServiceName2_MenuBar ) );
        REGISTER_SERVICES( VCLXPointer, ::rtl::OUString::createFromAscii( szServiceName_Pointer ), ::rtl::OUString::createFromAscii( szServiceName2_Pointer ) );
        REGISTER_SERVICES( UnoControlContainer, ::rtl::OUString::createFromAscii( szServiceName_UnoControlContainer ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlContainer ) );
        REGISTER_SERVICES( UnoControlContainerModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlContainerModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlContainerModel ) );
        REGISTER_SERVICES( StdTabController, ::rtl::OUString::createFromAscii( szServiceName_TabController ), ::rtl::OUString::createFromAscii( szServiceName2_TabController ) );
        REGISTER_SERVICES( StdTabControllerModel, ::rtl::OUString::createFromAscii( szServiceName_TabControllerModel ), ::rtl::OUString::createFromAscii( szServiceName2_TabControllerModel ) );
        REGISTER_SERVICES( UnoEditControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlEdit ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlEdit ) );
        REGISTER_SERVICES( UnoControlEditModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlEditModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlEditModel ) );
        REGISTER_SERVICES( UnoDateFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDateField ) );
        REGISTER_SERVICES( UnoControlDateFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDateFieldModel ) );
        REGISTER_SERVICES( UnoTimeFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlTimeField ) );
        REGISTER_SERVICES( UnoControlTimeFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlTimeFieldModel ) );
        REGISTER_SERVICES( UnoNumericFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlNumericField ) );
        REGISTER_SERVICES( UnoControlNumericFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlNumericFieldModel ) );
        REGISTER_SERVICES( UnoCurrencyFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCurrencyField ) );
        REGISTER_SERVICES( UnoControlCurrencyFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCurrencyFieldModel ) );
        REGISTER_SERVICES( UnoPatternFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlPatternField ) );
        REGISTER_SERVICES( UnoControlPatternFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlPatternFieldModel ) );
        REGISTER_SERVICES( UnoFormattedFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFormattedField ) );
        REGISTER_SERVICES( UnoControlFormattedFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFormattedFieldModel ) );
        REGISTER_SERVICES( UnoFileControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControl ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFileControl ) );
        REGISTER_SERVICES( UnoControlFileControlModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControlModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFileControlModel ) );
        REGISTER_SERVICES( UnoButtonControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlButton ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlButton ) );
        REGISTER_SERVICES( UnoControlButtonModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlButtonModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlButtonModel ) );
        REGISTER_SERVICES( UnoImageControlControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageButton ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageButton ) );
        REGISTER_SERVICES( UnoControlImageControlModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageButtonModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageButtonModel ) );
        REGISTER_SERVICES( UnoImageControlControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControl ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageControl ) );
        REGISTER_SERVICES( UnoControlImageControlModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControlModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageControlModel ) );
        REGISTER_SERVICES( UnoRadioButtonControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButton ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButton ) );
        REGISTER_SERVICES( UnoControlRadioButtonModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButtonModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButtonModel ) );
        REGISTER_SERVICES( UnoCheckBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCheckBox ) );
        REGISTER_SERVICES( UnoControlCheckBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCheckBoxModel ) );
        REGISTER_SERVICES( UnoListBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlListBox ) );
        REGISTER_SERVICES( UnoControlListBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlListBoxModel ) );
        REGISTER_SERVICES( UnoComboBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlComboBox ) );
        REGISTER_SERVICES( UnoControlComboBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlComboBoxModel ) );
        REGISTER_SERVICES( UnoFixedTextControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedText ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedText ) );
        REGISTER_SERVICES( UnoControlFixedTextModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedTextModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedTextModel ) );
        REGISTER_SERVICES( UnoGroupBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlGroupBox ) );
        REGISTER_SERVICES( UnoControlGroupBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlGroupBoxModel ) );
        REGISTER_SERVICES( VCLXPrinterServer, ::rtl::OUString::createFromAscii( szServiceName_PrinterServer ), ::rtl::OUString::createFromAscii( szServiceName2_PrinterServer ) );
        return sal_True;
    }
    return sal_False;
}

void* SAL_CALL component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, void* _pRegistryKey )
{
    void* pRet = 0;

    if ( _pServiceManager )
    {
        ::com::sun::star::lang::XMultiServiceFactory* pServiceManager = reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( _pServiceManager );

        CHECKANDCREATEFACTORY( VCLXToolkit, ::rtl::OUString::createFromAscii( szServiceName_Toolkit ), ::rtl::OUString::createFromAscii( szServiceName2_Toolkit ) )
        else CHECKANDCREATEFACTORY( VCLXPopupMenu, ::rtl::OUString::createFromAscii( szServiceName_PopupMenu ), ::rtl::OUString::createFromAscii( szServiceName2_PopupMenu ) )
        else CHECKANDCREATEFACTORY( VCLXMenuBar, ::rtl::OUString::createFromAscii( szServiceName_MenuBar ), ::rtl::OUString::createFromAscii( szServiceName2_MenuBar ) )
        else CHECKANDCREATEFACTORY( VCLXPointer, ::rtl::OUString::createFromAscii( szServiceName_Pointer ), ::rtl::OUString::createFromAscii( szServiceName2_Pointer ) )
        else CHECKANDCREATEFACTORY( UnoControlContainer, ::rtl::OUString::createFromAscii( szServiceName_UnoControlContainer ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlContainer ) )
        else CHECKANDCREATEFACTORY( UnoControlContainerModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlContainerModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlContainerModel ) )
        else CHECKANDCREATEFACTORY( StdTabController, ::rtl::OUString::createFromAscii( szServiceName_TabController ), ::rtl::OUString::createFromAscii( szServiceName2_TabController ) )
        else CHECKANDCREATEFACTORY( StdTabControllerModel, ::rtl::OUString::createFromAscii( szServiceName_TabControllerModel ), ::rtl::OUString::createFromAscii( szServiceName2_TabControllerModel ) )
        else CHECKANDCREATEFACTORY( UnoEditControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlEdit ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlEdit ) )
        else CHECKANDCREATEFACTORY( UnoControlEditModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlEditModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlEditModel ) )
        else CHECKANDCREATEFACTORY( UnoDateFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDateField ) )
        else CHECKANDCREATEFACTORY( UnoControlDateFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDateFieldModel ) )
        else CHECKANDCREATEFACTORY( UnoTimeFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlTimeField ) )
        else CHECKANDCREATEFACTORY( UnoControlTimeFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlTimeFieldModel ) )
        else CHECKANDCREATEFACTORY( UnoNumericFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlNumericField ) )
        else CHECKANDCREATEFACTORY( UnoControlNumericFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlNumericFieldModel ) )
        else CHECKANDCREATEFACTORY( UnoCurrencyFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCurrencyField ) )
        else CHECKANDCREATEFACTORY( UnoControlCurrencyFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCurrencyFieldModel ) )
        else CHECKANDCREATEFACTORY( UnoPatternFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlPatternField ) )
        else CHECKANDCREATEFACTORY( UnoControlPatternFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlPatternFieldModel ) )
        else CHECKANDCREATEFACTORY( UnoFormattedFieldControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedField ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFormattedField ) )
        else CHECKANDCREATEFACTORY( UnoControlFormattedFieldModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedFieldModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFormattedFieldModel ) )
        else CHECKANDCREATEFACTORY( UnoFileControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControl ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFileControl ) )
        else CHECKANDCREATEFACTORY( UnoControlFileControlModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControlModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFileControlModel ) )
        else CHECKANDCREATEFACTORY( UnoButtonControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlButton ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlButton ) )
        else CHECKANDCREATEFACTORY( UnoControlButtonModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlButtonModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlButtonModel ) )
        else CHECKANDCREATEFACTORY( UnoImageControlControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageButton ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageButton ) )
        else CHECKANDCREATEFACTORY( UnoControlImageControlModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageButtonModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageButtonModel ) )
        else CHECKANDCREATEFACTORY( UnoImageControlControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControl ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageControl ) )
        else CHECKANDCREATEFACTORY( UnoControlImageControlModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControlModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageControlModel ) )
        else CHECKANDCREATEFACTORY( UnoRadioButtonControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButton ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButton ) )
        else CHECKANDCREATEFACTORY( UnoControlRadioButtonModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButtonModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButtonModel ) )
        else CHECKANDCREATEFACTORY( UnoCheckBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCheckBox ) )
        else CHECKANDCREATEFACTORY( UnoControlCheckBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCheckBoxModel ) )
        else CHECKANDCREATEFACTORY( UnoListBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlListBox ) )
        else CHECKANDCREATEFACTORY( UnoControlListBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlListBoxModel ) )
        else CHECKANDCREATEFACTORY( UnoComboBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlComboBox ) )
        else CHECKANDCREATEFACTORY( UnoControlComboBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlComboBoxModel ) )
        else CHECKANDCREATEFACTORY( UnoFixedTextControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedText ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedText ) )
        else CHECKANDCREATEFACTORY( UnoControlFixedTextModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedTextModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedTextModel ) )
        else CHECKANDCREATEFACTORY( UnoGroupBoxControl, ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBox ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlGroupBox ) )
        else CHECKANDCREATEFACTORY( UnoControlGroupBoxModel, ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBoxModel ), ::rtl::OUString::createFromAscii( szServiceName2_UnoControlGroupBoxModel ) )
        else CHECKANDCREATEFACTORY( VCLXPrinterServer, ::rtl::OUString::createFromAscii( szServiceName_PrinterServer ), ::rtl::OUString::createFromAscii( szServiceName2_PrinterServer ) )
    }
    return pRet;
}
}



