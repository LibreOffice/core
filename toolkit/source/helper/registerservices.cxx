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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
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
#include <toolkit/controls/formattedcontrol.hxx>
#include <toolkit/controls/roadmapcontrol.hxx>
#include <toolkit/controls/tkscrollbar.hxx>
#include "toolkit/controls/tkspinbutton.hxx"
#include <toolkit/controls/tksimpleanimation.hxx>
#include <toolkit/controls/tkthrobber.hxx>
#include <toolkit/controls/animatedimages.hxx>
#include <toolkit/controls/spinningprogress.hxx>
#include <toolkit/controls/dialogcontrol.hxx>
#include <toolkit/controls/tabpagemodel.hxx>
#include <toolkit/controls/tabpagecontainer.hxx>
#include "toolkit/dllapi.h"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace toolkit
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::registry;

    //.........................................................................
    Reference< XRegistryKey > registerServices( const Reference< XRegistryKey >& _rxParentKey,
        const sal_Char* _pAsciiImplName, const sal_Char* _pAsciiServiceName )
    {
        ::rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM( "/stardiv.Toolkit." ) );
        sImplName += ::rtl::OUString::createFromAscii( _pAsciiImplName );
        sImplName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES" ) );

        Reference< XRegistryKey > xNewKey = _rxParentKey->createKey( sImplName );
        xNewKey->createKey( ::rtl::OUString::createFromAscii( _pAsciiServiceName ) );

        return xNewKey;
    }

    //.........................................................................
    Reference< XRegistryKey > registerServices( const Reference< XRegistryKey >& _rxParentKey,
        const sal_Char* _pAsciiImplName, const sal_Char* _pAsciiServiceName1, const sal_Char* _pAsciiServiceName2 )
    {
        Reference< XRegistryKey > xComponentServicesKey = registerServices( _rxParentKey, _pAsciiImplName, _pAsciiServiceName1 );
        xComponentServicesKey->createKey( ::rtl::OUString::createFromAscii( _pAsciiServiceName2 ) );
        return xComponentServicesKey;
    }

    //.........................................................................
    void* tryCreateFactory( const sal_Char* _pRequiredImplName, const sal_Char* _pComponentImplName,
        const sal_Char* _pAsciiServiceName1, const sal_Char* _pAsciiServiceName2,
        ::cppu::ComponentInstantiation _pInstantiation, const Reference< XMultiServiceFactory >& _rxServiceFactory )
    {
        void* pReturn = NULL;

        if ( rtl_str_compare( _pRequiredImplName, _pComponentImplName ) == 0 )
        {
            Sequence< ::rtl::OUString > aServiceNames( _pAsciiServiceName2 ? 2 : 1 );
            aServiceNames.getArray()[ 0 ] = ::rtl::OUString::createFromAscii( _pAsciiServiceName1 );
            if ( _pAsciiServiceName2 )
                aServiceNames.getArray()[ 1 ] = ::rtl::OUString::createFromAscii( _pAsciiServiceName2 );
            Reference< XSingleServiceFactory > xFactory( ::cppu::createSingleFactory(
                _rxServiceFactory, ::rtl::OUString::createFromAscii( _pComponentImplName ),
                _pInstantiation, aServiceNames
            ) );

            if ( xFactory.is() )
            {
                xFactory->acquire();
                pReturn = xFactory.get();
            }
        }

        return pReturn;
    }


}

#define IMPL_CREATEINSTANCE( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ) \
    { return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ImplName ); }

#define IMPL_CREATEINSTANCE2( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rSMgr) \
    { return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ImplName( rSMgr ) ); }

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL UnoControlDialogModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& )
{
    return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new OGeometryControlModel<UnoControlDialogModel> );
}

#define CHECKANDCREATEFACTORY( ImplName, ServiceName1, ServiceName2 ) \
    pRet = tryCreateFactory( sImplementationName, "stardiv.Toolkit." #ImplName, \
                ServiceName1, ServiceName2, \
                ImplName##_CreateInstance, xServiceFactory \
            ); \
    if ( pRet ) \
        return pRet; \

#define TRY_OOO_FACTORY( ImplName, ServiceName ) \
    pRet = tryCreateFactory( sImplementationName, "org.openoffice.comp.toolkit." #ImplName, \
                ServiceName, NULL, \
                ImplName##_CreateInstance, xServiceFactory \
            ); \
    if ( pRet ) \
        return pRet; \

using namespace toolkit;

IMPL_CREATEINSTANCE2( VCLXToolkit )
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
IMPL_CREATEINSTANCE( UnoControlFixedHyperlinkModel )
IMPL_CREATEINSTANCE( UnoControlFixedTextModel )
IMPL_CREATEINSTANCE( UnoControlFormattedFieldModel )
IMPL_CREATEINSTANCE( UnoControlGroupBoxModel )
IMPL_CREATEINSTANCE( UnoControlImageControlModel )
IMPL_CREATEINSTANCE( UnoControlListBoxModel )
IMPL_CREATEINSTANCE( UnoControlNumericFieldModel )
IMPL_CREATEINSTANCE( UnoControlPatternFieldModel )
IMPL_CREATEINSTANCE( UnoControlRadioButtonModel )
IMPL_CREATEINSTANCE( UnoControlTimeFieldModel )
IMPL_CREATEINSTANCE( UnoControlProgressBarModel )
IMPL_CREATEINSTANCE( UnoControlScrollBarModel )
IMPL_CREATEINSTANCE( UnoSpinButtonModel )
IMPL_CREATEINSTANCE( UnoControlFixedLineModel )
IMPL_CREATEINSTANCE( UnoCurrencyFieldControl )
IMPL_CREATEINSTANCE( UnoDateFieldControl )
IMPL_CREATEINSTANCE( UnoDialogControl )
IMPL_CREATEINSTANCE( UnoEditControl )
IMPL_CREATEINSTANCE( UnoFileControl )
IMPL_CREATEINSTANCE( UnoFixedHyperlinkControl )
IMPL_CREATEINSTANCE( UnoFixedTextControl )
IMPL_CREATEINSTANCE( UnoFormattedFieldControl )
IMPL_CREATEINSTANCE( UnoGroupBoxControl )
IMPL_CREATEINSTANCE( UnoImageControlControl )
IMPL_CREATEINSTANCE( UnoListBoxControl )
IMPL_CREATEINSTANCE( UnoNumericFieldControl )
IMPL_CREATEINSTANCE( UnoPatternFieldControl )
IMPL_CREATEINSTANCE( UnoRadioButtonControl )
IMPL_CREATEINSTANCE( UnoTimeFieldControl )
IMPL_CREATEINSTANCE( UnoProgressBarControl )
IMPL_CREATEINSTANCE( UnoScrollBarControl )
IMPL_CREATEINSTANCE( UnoSpinButtonControl )
IMPL_CREATEINSTANCE( UnoFixedLineControl )
IMPL_CREATEINSTANCE( VCLXMenuBar )
IMPL_CREATEINSTANCE( VCLXPointer )
IMPL_CREATEINSTANCE( VCLXPopupMenu )
IMPL_CREATEINSTANCE( VCLXPrinterServer )
IMPL_CREATEINSTANCE( UnoRoadmapControl )
IMPL_CREATEINSTANCE( UnoControlRoadmapModel )
IMPL_CREATEINSTANCE( UnoSimpleAnimationControl )
IMPL_CREATEINSTANCE( UnoSimpleAnimationControlModel )
IMPL_CREATEINSTANCE( UnoThrobberControl )
IMPL_CREATEINSTANCE( UnoThrobberControlModel )
IMPL_CREATEINSTANCE( UnoControlTabPage )
//IMPL_CREATEINSTANCE( UnoControlTabPageModel )
//IMPL_CREATEINSTANCE( UnoControlTabPageContainer )
//IMPL_CREATEINSTANCE( UnoControlTabPageContainerModel )
IMPL_CREATEINSTANCE( AnimatedImagesControl )
IMPL_CREATEINSTANCE( AnimatedImagesControlModel )
IMPL_CREATEINSTANCE( SpinningProgressControlModel )

extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL TreeControl_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL TreeControlModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL MutableTreeDataModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridControl_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridControlModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DefaultGridDataModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DefaultGridColumnModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridColumn_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL UnoControlTabPageContainer_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL UnoControlTabPageContainerModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );

//extern sal_Bool SAL_CALL UnoControlTabPageModel_component_writeInfo(void * serviceManager, void * registryKey);
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL UnoControlTabPageModel_CreateInstance (const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );

extern void * SAL_CALL comp_AsyncCallback_component_getFactory( const char * implName, void * serviceManager, void * registryKey );

extern void * SAL_CALL comp_Layout_component_getFactory( const char * implName, void * serviceManager, void * registryKey );

extern "C"
{

TOOLKIT_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

TOOLKIT_DLLPUBLIC void* SAL_CALL component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, void* _pRegistryKey )
{
    void* pRet = NULL;

    if ( _pServiceManager )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory =
            static_cast< ::com::sun::star::lang::XMultiServiceFactory* >( _pServiceManager );

        CHECKANDCREATEFACTORY( VCLXToolkit, szServiceName_Toolkit, szServiceName2_Toolkit )
        CHECKANDCREATEFACTORY( VCLXPopupMenu, szServiceName_PopupMenu, szServiceName2_PopupMenu )
        CHECKANDCREATEFACTORY( VCLXMenuBar, szServiceName_MenuBar, szServiceName2_MenuBar )
        CHECKANDCREATEFACTORY( VCLXPointer, szServiceName_Pointer, szServiceName2_Pointer )
        CHECKANDCREATEFACTORY( UnoControlContainer, szServiceName_UnoControlContainer, szServiceName2_UnoControlContainer )
        CHECKANDCREATEFACTORY( UnoControlContainerModel, szServiceName_UnoControlContainerModel, szServiceName2_UnoControlContainerModel )
        CHECKANDCREATEFACTORY( StdTabController, szServiceName_TabController, szServiceName2_TabController )
        CHECKANDCREATEFACTORY( StdTabControllerModel, szServiceName_TabControllerModel, szServiceName2_TabControllerModel )
        CHECKANDCREATEFACTORY( UnoDialogControl, szServiceName_UnoControlDialog, szServiceName2_UnoControlDialog )
        CHECKANDCREATEFACTORY( UnoControlDialogModel, szServiceName_UnoControlDialogModel, szServiceName2_UnoControlDialogModel )
        CHECKANDCREATEFACTORY( UnoEditControl, szServiceName_UnoControlEdit, szServiceName2_UnoControlEdit )
        CHECKANDCREATEFACTORY( UnoControlEditModel, szServiceName_UnoControlEditModel, szServiceName2_UnoControlEditModel )
        CHECKANDCREATEFACTORY( UnoDateFieldControl, szServiceName_UnoControlDateField, szServiceName2_UnoControlDateField )
        CHECKANDCREATEFACTORY( UnoControlDateFieldModel, szServiceName_UnoControlDateFieldModel, szServiceName2_UnoControlDateFieldModel )
        CHECKANDCREATEFACTORY( UnoTimeFieldControl, szServiceName_UnoControlTimeField, szServiceName2_UnoControlTimeField )
        CHECKANDCREATEFACTORY( UnoControlTimeFieldModel, szServiceName_UnoControlTimeFieldModel, szServiceName2_UnoControlTimeFieldModel )
        CHECKANDCREATEFACTORY( UnoNumericFieldControl, szServiceName_UnoControlNumericField, szServiceName2_UnoControlNumericField )
        CHECKANDCREATEFACTORY( UnoControlNumericFieldModel, szServiceName_UnoControlNumericFieldModel, szServiceName2_UnoControlNumericFieldModel )
        CHECKANDCREATEFACTORY( UnoCurrencyFieldControl, szServiceName_UnoControlCurrencyField, szServiceName2_UnoControlCurrencyField )
        CHECKANDCREATEFACTORY( UnoControlCurrencyFieldModel, szServiceName_UnoControlCurrencyFieldModel, szServiceName2_UnoControlCurrencyFieldModel )
        CHECKANDCREATEFACTORY( UnoPatternFieldControl, szServiceName_UnoControlPatternField, szServiceName2_UnoControlPatternField )
        CHECKANDCREATEFACTORY( UnoControlPatternFieldModel, szServiceName_UnoControlPatternFieldModel, szServiceName2_UnoControlPatternFieldModel )
        CHECKANDCREATEFACTORY( UnoFormattedFieldControl, szServiceName_UnoControlFormattedField, szServiceName2_UnoControlFormattedField )
        CHECKANDCREATEFACTORY( UnoControlFormattedFieldModel, szServiceName_UnoControlFormattedFieldModel, szServiceName2_UnoControlFormattedFieldModel )
        CHECKANDCREATEFACTORY( UnoFileControl, szServiceName_UnoControlFileControl, szServiceName2_UnoControlFileControl )
        CHECKANDCREATEFACTORY( UnoControlFileControlModel, szServiceName_UnoControlFileControlModel, szServiceName2_UnoControlFileControlModel )
        CHECKANDCREATEFACTORY( UnoButtonControl, szServiceName_UnoControlButton, szServiceName2_UnoControlButton )
        CHECKANDCREATEFACTORY( UnoControlButtonModel, szServiceName_UnoControlButtonModel, szServiceName2_UnoControlButtonModel )
        CHECKANDCREATEFACTORY( UnoImageControlControl, szServiceName_UnoControlImageButton, szServiceName2_UnoControlImageButton )
        CHECKANDCREATEFACTORY( UnoControlImageControlModel, szServiceName_UnoControlImageButtonModel, szServiceName2_UnoControlImageButtonModel )
        CHECKANDCREATEFACTORY( UnoImageControlControl, szServiceName_UnoControlImageControl, szServiceName2_UnoControlImageControl )
        CHECKANDCREATEFACTORY( UnoControlImageControlModel, szServiceName_UnoControlImageControlModel, szServiceName2_UnoControlImageControlModel )
        CHECKANDCREATEFACTORY( UnoRadioButtonControl, szServiceName_UnoControlRadioButton, szServiceName2_UnoControlRadioButton )
        CHECKANDCREATEFACTORY( UnoControlRadioButtonModel, szServiceName_UnoControlRadioButtonModel, szServiceName2_UnoControlRadioButtonModel )
        CHECKANDCREATEFACTORY( UnoCheckBoxControl, szServiceName_UnoControlCheckBox, szServiceName2_UnoControlCheckBox )
        CHECKANDCREATEFACTORY( UnoControlCheckBoxModel, szServiceName_UnoControlCheckBoxModel, szServiceName2_UnoControlCheckBoxModel )
        CHECKANDCREATEFACTORY( UnoListBoxControl, szServiceName_UnoControlListBox, szServiceName2_UnoControlListBox )
        CHECKANDCREATEFACTORY( UnoControlListBoxModel, szServiceName_UnoControlListBoxModel, szServiceName2_UnoControlListBoxModel )
        CHECKANDCREATEFACTORY( UnoComboBoxControl, szServiceName_UnoControlComboBox, szServiceName2_UnoControlComboBox )
        CHECKANDCREATEFACTORY( UnoControlComboBoxModel, szServiceName_UnoControlComboBoxModel, szServiceName2_UnoControlComboBoxModel )
        CHECKANDCREATEFACTORY( UnoFixedTextControl, szServiceName_UnoControlFixedText, szServiceName2_UnoControlFixedText )
        CHECKANDCREATEFACTORY( UnoControlFixedTextModel, szServiceName_UnoControlFixedTextModel, szServiceName2_UnoControlFixedTextModel )
        CHECKANDCREATEFACTORY( UnoGroupBoxControl, szServiceName_UnoControlGroupBox, szServiceName2_UnoControlGroupBox )
        CHECKANDCREATEFACTORY( UnoControlGroupBoxModel, szServiceName_UnoControlGroupBoxModel, szServiceName2_UnoControlGroupBoxModel )
        CHECKANDCREATEFACTORY( UnoProgressBarControl, szServiceName_UnoControlProgressBar, szServiceName2_UnoControlProgressBar )
        CHECKANDCREATEFACTORY( UnoControlProgressBarModel, szServiceName_UnoControlProgressBarModel, szServiceName2_UnoControlProgressBarModel )
        CHECKANDCREATEFACTORY( UnoScrollBarControl, szServiceName_UnoControlScrollBar, szServiceName2_UnoControlScrollBar )
        CHECKANDCREATEFACTORY( UnoControlScrollBarModel, szServiceName_UnoControlScrollBarModel, szServiceName2_UnoControlScrollBarModel )
        CHECKANDCREATEFACTORY( UnoFixedLineControl, szServiceName_UnoControlFixedLine, szServiceName2_UnoControlFixedLine )
        CHECKANDCREATEFACTORY( UnoControlFixedLineModel, szServiceName_UnoControlFixedLineModel, szServiceName2_UnoControlFixedLineModel )
        CHECKANDCREATEFACTORY( VCLXPrinterServer, szServiceName_PrinterServer, szServiceName2_PrinterServer )
        CHECKANDCREATEFACTORY( UnoRoadmapControl, szServiceName_UnoControlRoadmap, szServiceName2_UnoControlRoadmap )
        CHECKANDCREATEFACTORY( UnoControlRoadmapModel, szServiceName_UnoControlRoadmapModel, szServiceName2_UnoControlRoadmapModel )
        CHECKANDCREATEFACTORY( UnoSpinButtonModel, szServiceName_UnoSpinButtonModel, NULL )
        CHECKANDCREATEFACTORY( UnoSpinButtonControl, szServiceName_UnoSpinButtonControl, NULL )
        CHECKANDCREATEFACTORY( TreeControl, szServiceName_TreeControl, NULL )
        CHECKANDCREATEFACTORY( TreeControlModel, szServiceName_TreeControlModel, NULL )
        CHECKANDCREATEFACTORY( MutableTreeDataModel, szServiceName_MutableTreeDataModel, NULL )
        CHECKANDCREATEFACTORY( UnoSimpleAnimationControlModel, szServiceName_UnoSimpleAnimationControlModel, szServiceName2_UnoSimpleAnimationControlModel )
        CHECKANDCREATEFACTORY( UnoSimpleAnimationControl, szServiceName_UnoSimpleAnimationControl, szServiceName2_UnoSimpleAnimationControl )
        CHECKANDCREATEFACTORY( UnoThrobberControlModel, szServiceName_UnoThrobberControlModel, szServiceName2_UnoThrobberControlModel )
        CHECKANDCREATEFACTORY( UnoThrobberControl, szServiceName_UnoThrobberControl, szServiceName2_UnoThrobberControl )
        TRY_OOO_FACTORY( AnimatedImagesControl, szServiceName_AnimatedImagesControl )
        TRY_OOO_FACTORY( AnimatedImagesControlModel, szServiceName_AnimatedImagesControlModel )
        TRY_OOO_FACTORY( SpinningProgressControlModel, szServiceName_SpinningProgressControlModel )
        CHECKANDCREATEFACTORY( UnoFixedHyperlinkControl, szServiceName_UnoControlFixedHyperlink, NULL )
        CHECKANDCREATEFACTORY( UnoControlFixedHyperlinkModel, szServiceName_UnoControlFixedHyperlinkModel, NULL )
        CHECKANDCREATEFACTORY( GridControl, szServiceName_GridControl, NULL )
         CHECKANDCREATEFACTORY( GridControlModel, szServiceName_GridControlModel, NULL )
        CHECKANDCREATEFACTORY( DefaultGridDataModel, szServiceName_DefaultGridDataModel, NULL )
        CHECKANDCREATEFACTORY( DefaultGridColumnModel, szServiceName_DefaultGridColumnModel, NULL )
        CHECKANDCREATEFACTORY( GridColumn, szServiceName_GridColumn, NULL )
        CHECKANDCREATEFACTORY( UnoControlTabPageModel, szServiceName_UnoControlTabPageModel, NULL )
        CHECKANDCREATEFACTORY( UnoControlTabPage, szServiceName_UnoControlTabPage, NULL )
        CHECKANDCREATEFACTORY( UnoControlTabPageContainerModel, szServiceName_UnoControlTabPageContainerModel, NULL )
        CHECKANDCREATEFACTORY( UnoControlTabPageContainer, szServiceName_UnoControlTabPageContainer, NULL )


        if ( rtl_str_compare( sImplementationName, "com.sun.star.awt.comp.AsyncCallback" ) == 0 )
            return comp_AsyncCallback_component_getFactory( sImplementationName, _pServiceManager, _pRegistryKey );
        if( pRet == 0 )
            pRet = comp_Layout_component_getFactory( sImplementationName, _pServiceManager, _pRegistryKey );
    }
    return pRet;
}
}



