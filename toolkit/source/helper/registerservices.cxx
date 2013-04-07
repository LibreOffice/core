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

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <comphelper/processfactory.hxx>
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
#include <toolkit/controls/dialogcontrol.hxx>
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
    void* tryCreateFactory( const sal_Char* _pRequiredImplName, const sal_Char* _pComponentImplName,
        const sal_Char* _pAsciiServiceName1, const sal_Char* _pAsciiServiceName2,
        ::cppu::ComponentInstantiation _pInstantiation, const Reference< XMultiServiceFactory >& _rxServiceFactory )
    {
        void* pReturn = NULL;

        if ( rtl_str_compare( _pRequiredImplName, _pComponentImplName ) == 0 )
        {
            Sequence< OUString > aServiceNames( _pAsciiServiceName2 ? 2 : 1 );
            aServiceNames.getArray()[ 0 ] = OUString::createFromAscii( _pAsciiServiceName1 );
            if ( _pAsciiServiceName2 )
                aServiceNames.getArray()[ 1 ] = OUString::createFromAscii( _pAsciiServiceName2 );
            Reference< XSingleServiceFactory > xFactory( ::cppu::createSingleFactory(
                _rxServiceFactory, OUString::createFromAscii( _pComponentImplName ),
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

#define IMPL_CREATEINSTANCE_CTX( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory ) \
    { return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new ImplName( comphelper::getComponentContext(i_factory) ) ); }

#define IMPL_CREATE_INSTANCE_WITH_GEOMETRY( ImplName ) \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory ) \
{ \
    return ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface >( ( ::cppu::OWeakObject* ) new OGeometryControlModel< ImplName >( comphelper::getComponentContext( i_factory ) ) ); \
}

#define GET_FACTORY_WITH_IMPL_PREFIX( ClassName, ImplNamePrefix, ServiceName1, ServiceName2 ) \
    pRet = tryCreateFactory( sImplementationName, ImplNamePrefix "." #ClassName, \
                ServiceName1, ServiceName2, \
                ClassName##_CreateInstance, xServiceFactory \
            ); \
    if ( pRet ) \
        return pRet; \

#define GET_FACTORY( ImplName, ServiceName1, ServiceName2 ) \
    GET_FACTORY_WITH_IMPL_PREFIX( ImplName, "stardiv.Toolkit", ServiceName1, ServiceName2 )

using namespace toolkit;

IMPL_CREATEINSTANCE( VCLXToolkit )
IMPL_CREATEINSTANCE( StdTabController )
IMPL_CREATEINSTANCE( StdTabControllerModel )
IMPL_CREATEINSTANCE( UnoButtonControl )
IMPL_CREATEINSTANCE( UnoCheckBoxControl )
IMPL_CREATEINSTANCE( UnoComboBoxControl )
IMPL_CREATEINSTANCE_CTX( UnoControlButtonModel )
IMPL_CREATEINSTANCE_CTX( UnoControlCheckBoxModel )
IMPL_CREATEINSTANCE_CTX( UnoControlComboBoxModel )
IMPL_CREATEINSTANCE( UnoControlContainer )
IMPL_CREATEINSTANCE_CTX( UnoControlContainerModel )
IMPL_CREATEINSTANCE_CTX( UnoControlCurrencyFieldModel )
IMPL_CREATEINSTANCE_CTX( UnoControlDateFieldModel )
IMPL_CREATEINSTANCE_CTX( UnoControlEditModel )
IMPL_CREATEINSTANCE_CTX( UnoControlFileControlModel )
IMPL_CREATEINSTANCE_CTX( UnoControlFixedHyperlinkModel )
IMPL_CREATEINSTANCE_CTX( UnoControlFixedTextModel )
IMPL_CREATEINSTANCE_CTX( UnoControlFormattedFieldModel )
IMPL_CREATEINSTANCE_CTX( UnoControlGroupBoxModel )
IMPL_CREATEINSTANCE_CTX( UnoControlImageControlModel )
IMPL_CREATEINSTANCE_CTX( UnoControlListBoxModel )
IMPL_CREATEINSTANCE_CTX( UnoControlNumericFieldModel )
IMPL_CREATEINSTANCE_CTX( UnoControlPatternFieldModel )
IMPL_CREATEINSTANCE_CTX( UnoControlRadioButtonModel )
IMPL_CREATEINSTANCE_CTX( UnoControlTimeFieldModel )
IMPL_CREATEINSTANCE_CTX( UnoControlProgressBarModel )
IMPL_CREATEINSTANCE_CTX( UnoControlScrollBarModel )
IMPL_CREATEINSTANCE_CTX( UnoSpinButtonModel )
IMPL_CREATEINSTANCE_CTX( UnoMultiPageModel )
IMPL_CREATEINSTANCE_CTX( UnoPageModel )
IMPL_CREATEINSTANCE_CTX( UnoFrameModel )
IMPL_CREATEINSTANCE_CTX( UnoControlFixedLineModel )
IMPL_CREATEINSTANCE( UnoCurrencyFieldControl )
IMPL_CREATEINSTANCE( UnoDateFieldControl )
IMPL_CREATEINSTANCE_CTX( UnoDialogControl )
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
IMPL_CREATEINSTANCE_CTX( UnoMultiPageControl )
IMPL_CREATEINSTANCE_CTX( UnoPageControl )
IMPL_CREATEINSTANCE_CTX( UnoFrameControl )
IMPL_CREATEINSTANCE( VCLXMenuBar )
IMPL_CREATEINSTANCE( VCLXPointer )
IMPL_CREATEINSTANCE( VCLXPopupMenu )
IMPL_CREATEINSTANCE( VCLXPrinterServer )
IMPL_CREATEINSTANCE( UnoRoadmapControl )
IMPL_CREATEINSTANCE_CTX( UnoControlRoadmapModel )
IMPL_CREATEINSTANCE( UnoSimpleAnimationControl )
IMPL_CREATEINSTANCE_CTX( UnoSimpleAnimationControlModel )
IMPL_CREATEINSTANCE( UnoThrobberControl )
IMPL_CREATEINSTANCE_CTX( UnoThrobberControlModel )
IMPL_CREATEINSTANCE_CTX( UnoControlTabPage )
IMPL_CREATEINSTANCE_CTX( UnoControlTabPageModel )
IMPL_CREATEINSTANCE_CTX( UnoControlTabPageContainer )
IMPL_CREATEINSTANCE_CTX( UnoControlTabPageContainerModel )
IMPL_CREATEINSTANCE( AnimatedImagesControl )
IMPL_CREATEINSTANCE_CTX( AnimatedImagesControlModel )
IMPL_CREATEINSTANCE_CTX( SpinningProgressControlModel )

IMPL_CREATE_INSTANCE_WITH_GEOMETRY( UnoControlDialogModel )

extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL TreeControl_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL TreeControlModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL MutableTreeDataModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridControl_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridControlModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DefaultGridDataModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DefaultGridColumnModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GridColumn_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SortableGridDataModel_CreateInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );

extern void * SAL_CALL comp_AsyncCallback_component_getFactory( const char * implName, void * serviceManager, void * registryKey );

extern "C"
{

TOOLKIT_DLLPUBLIC void* SAL_CALL tk_component_getFactory( const sal_Char* sImplementationName, void* _pServiceManager, void* _pRegistryKey )
{

    void* pRet = NULL;

    if ( _pServiceManager )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory =
            static_cast< ::com::sun::star::lang::XMultiServiceFactory* >( _pServiceManager );
        GET_FACTORY( VCLXToolkit, szServiceName_Toolkit, szServiceName2_Toolkit )
        GET_FACTORY( VCLXPopupMenu, szServiceName_PopupMenu, szServiceName2_PopupMenu )
        GET_FACTORY( VCLXMenuBar, szServiceName_MenuBar, szServiceName2_MenuBar )
        GET_FACTORY( VCLXPointer, szServiceName_Pointer, szServiceName2_Pointer )
        GET_FACTORY( UnoControlContainer, szServiceName_UnoControlContainer, szServiceName2_UnoControlContainer )
        GET_FACTORY( UnoControlContainerModel, szServiceName_UnoControlContainerModel, szServiceName2_UnoControlContainerModel )
        GET_FACTORY( StdTabController, szServiceName_TabController, szServiceName2_TabController )
        GET_FACTORY( StdTabControllerModel, szServiceName_TabControllerModel, szServiceName2_TabControllerModel )
        GET_FACTORY( UnoDialogControl, szServiceName_UnoControlDialog, szServiceName2_UnoControlDialog )
        GET_FACTORY( UnoControlDialogModel, szServiceName_UnoControlDialogModel, szServiceName2_UnoControlDialogModel )
        GET_FACTORY( UnoEditControl, szServiceName_UnoControlEdit, szServiceName2_UnoControlEdit )
        GET_FACTORY( UnoControlEditModel, szServiceName_UnoControlEditModel, szServiceName2_UnoControlEditModel )
        GET_FACTORY( UnoDateFieldControl, szServiceName_UnoControlDateField, szServiceName2_UnoControlDateField )
        GET_FACTORY( UnoControlDateFieldModel, szServiceName_UnoControlDateFieldModel, szServiceName2_UnoControlDateFieldModel )
        GET_FACTORY( UnoTimeFieldControl, szServiceName_UnoControlTimeField, szServiceName2_UnoControlTimeField )
        GET_FACTORY( UnoControlTimeFieldModel, szServiceName_UnoControlTimeFieldModel, szServiceName2_UnoControlTimeFieldModel )
        GET_FACTORY( UnoNumericFieldControl, szServiceName_UnoControlNumericField, szServiceName2_UnoControlNumericField )
        GET_FACTORY( UnoControlNumericFieldModel, szServiceName_UnoControlNumericFieldModel, szServiceName2_UnoControlNumericFieldModel )
        GET_FACTORY( UnoCurrencyFieldControl, szServiceName_UnoControlCurrencyField, szServiceName2_UnoControlCurrencyField )
        GET_FACTORY( UnoControlCurrencyFieldModel, szServiceName_UnoControlCurrencyFieldModel, szServiceName2_UnoControlCurrencyFieldModel )
        GET_FACTORY( UnoPatternFieldControl, szServiceName_UnoControlPatternField, szServiceName2_UnoControlPatternField )
        GET_FACTORY( UnoControlPatternFieldModel, szServiceName_UnoControlPatternFieldModel, szServiceName2_UnoControlPatternFieldModel )
        GET_FACTORY( UnoFormattedFieldControl, szServiceName_UnoControlFormattedField, szServiceName2_UnoControlFormattedField )
        GET_FACTORY( UnoControlFormattedFieldModel, szServiceName_UnoControlFormattedFieldModel, szServiceName2_UnoControlFormattedFieldModel )
        GET_FACTORY( UnoFileControl, szServiceName_UnoControlFileControl, szServiceName2_UnoControlFileControl )
        GET_FACTORY( UnoControlFileControlModel, szServiceName_UnoControlFileControlModel, szServiceName2_UnoControlFileControlModel )
        GET_FACTORY( UnoButtonControl, szServiceName_UnoControlButton, szServiceName2_UnoControlButton )
        GET_FACTORY( UnoControlButtonModel, szServiceName_UnoControlButtonModel, szServiceName2_UnoControlButtonModel )
        GET_FACTORY( UnoImageControlControl, szServiceName_UnoControlImageButton, szServiceName2_UnoControlImageButton )
        GET_FACTORY( UnoControlImageControlModel, szServiceName_UnoControlImageButtonModel, szServiceName2_UnoControlImageButtonModel )
        GET_FACTORY( UnoImageControlControl, szServiceName_UnoControlImageControl, szServiceName2_UnoControlImageControl )
        GET_FACTORY( UnoControlImageControlModel, szServiceName_UnoControlImageControlModel, szServiceName2_UnoControlImageControlModel )
        GET_FACTORY( UnoRadioButtonControl, szServiceName_UnoControlRadioButton, szServiceName2_UnoControlRadioButton )
        GET_FACTORY( UnoControlRadioButtonModel, szServiceName_UnoControlRadioButtonModel, szServiceName2_UnoControlRadioButtonModel )
        GET_FACTORY( UnoCheckBoxControl, szServiceName_UnoControlCheckBox, szServiceName2_UnoControlCheckBox )
        GET_FACTORY( UnoControlCheckBoxModel, szServiceName_UnoControlCheckBoxModel, szServiceName2_UnoControlCheckBoxModel )
        GET_FACTORY( UnoListBoxControl, szServiceName_UnoControlListBox, szServiceName2_UnoControlListBox )
        GET_FACTORY( UnoControlListBoxModel, szServiceName_UnoControlListBoxModel, szServiceName2_UnoControlListBoxModel )
        GET_FACTORY( UnoComboBoxControl, szServiceName_UnoControlComboBox, szServiceName2_UnoControlComboBox )
        GET_FACTORY( UnoControlComboBoxModel, szServiceName_UnoControlComboBoxModel, szServiceName2_UnoControlComboBoxModel )
        GET_FACTORY( UnoFixedTextControl, szServiceName_UnoControlFixedText, szServiceName2_UnoControlFixedText )
        GET_FACTORY( UnoControlFixedTextModel, szServiceName_UnoControlFixedTextModel, szServiceName2_UnoControlFixedTextModel )
        GET_FACTORY( UnoGroupBoxControl, szServiceName_UnoControlGroupBox, szServiceName2_UnoControlGroupBox )
        GET_FACTORY( UnoControlGroupBoxModel, szServiceName_UnoControlGroupBoxModel, szServiceName2_UnoControlGroupBoxModel )
        GET_FACTORY( UnoProgressBarControl, szServiceName_UnoControlProgressBar, szServiceName2_UnoControlProgressBar )
        GET_FACTORY( UnoControlProgressBarModel, szServiceName_UnoControlProgressBarModel, szServiceName2_UnoControlProgressBarModel )
        GET_FACTORY( UnoScrollBarControl, szServiceName_UnoControlScrollBar, szServiceName2_UnoControlScrollBar )
        GET_FACTORY( UnoControlScrollBarModel, szServiceName_UnoControlScrollBarModel, szServiceName2_UnoControlScrollBarModel )
        GET_FACTORY( UnoFixedLineControl, szServiceName_UnoControlFixedLine, szServiceName2_UnoControlFixedLine )
        GET_FACTORY( UnoControlFixedLineModel, szServiceName_UnoControlFixedLineModel, szServiceName2_UnoControlFixedLineModel )
        GET_FACTORY( VCLXPrinterServer, szServiceName_PrinterServer, szServiceName2_PrinterServer )
        GET_FACTORY( UnoRoadmapControl, szServiceName_UnoControlRoadmap, szServiceName2_UnoControlRoadmap )
        GET_FACTORY( UnoControlRoadmapModel, szServiceName_UnoControlRoadmapModel, szServiceName2_UnoControlRoadmapModel )
        GET_FACTORY( UnoMultiPageModel, szServiceName_UnoMultiPageModel, NULL )
        GET_FACTORY( UnoMultiPageControl, szServiceName_UnoMultiPageControl, NULL )
        GET_FACTORY( UnoPageModel, szServiceName_UnoPageModel, NULL )
        GET_FACTORY( UnoPageControl, szServiceName_UnoPageControl, NULL )
        GET_FACTORY( UnoFrameModel, szServiceName_UnoFrameModel, NULL )
        GET_FACTORY( UnoFrameControl, szServiceName_UnoFrameControl, NULL )
        GET_FACTORY( UnoSpinButtonModel, szServiceName_UnoSpinButtonModel, NULL )
        GET_FACTORY( UnoSpinButtonControl, szServiceName_UnoSpinButtonControl, NULL )
        GET_FACTORY( TreeControl, szServiceName_TreeControl, NULL )
        GET_FACTORY( TreeControlModel, szServiceName_TreeControlModel, NULL )
        GET_FACTORY( MutableTreeDataModel, szServiceName_MutableTreeDataModel, NULL )
        GET_FACTORY( UnoSimpleAnimationControlModel, szServiceName_UnoSimpleAnimationControlModel, szServiceName2_UnoSimpleAnimationControlModel )
        GET_FACTORY( UnoSimpleAnimationControl, szServiceName_UnoSimpleAnimationControl, szServiceName2_UnoSimpleAnimationControl )
        GET_FACTORY( UnoThrobberControlModel, szServiceName_UnoThrobberControlModel, szServiceName2_UnoThrobberControlModel )
        GET_FACTORY( UnoThrobberControl, szServiceName_UnoThrobberControl, szServiceName2_UnoThrobberControl )
        GET_FACTORY( UnoFixedHyperlinkControl, szServiceName_UnoControlFixedHyperlink, NULL )
        GET_FACTORY( UnoControlFixedHyperlinkModel, szServiceName_UnoControlFixedHyperlinkModel, NULL )
        GET_FACTORY( GridControl, szServiceName_GridControl, NULL );
         GET_FACTORY( GridControlModel, szServiceName_GridControlModel, NULL );
        GET_FACTORY( DefaultGridDataModel, szServiceName_DefaultGridDataModel, NULL );
        GET_FACTORY( DefaultGridColumnModel, szServiceName_DefaultGridColumnModel, NULL );
        GET_FACTORY_WITH_IMPL_PREFIX( GridColumn, "org.openoffice.comp.toolkit", szServiceName_GridColumn, NULL );
        GET_FACTORY_WITH_IMPL_PREFIX( SortableGridDataModel, "org.openoffice.comp.toolkit", szServiceName_SortableGridDataModel, NULL );
        GET_FACTORY( UnoControlTabPageModel, szServiceName_UnoControlTabPageModel, NULL )
        GET_FACTORY( UnoControlTabPage, szServiceName_UnoControlTabPage, NULL )
        GET_FACTORY( UnoControlTabPageContainerModel, szServiceName_UnoControlTabPageContainerModel, NULL )
        GET_FACTORY( UnoControlTabPageContainer, szServiceName_UnoControlTabPageContainer, NULL )
        GET_FACTORY_WITH_IMPL_PREFIX( AnimatedImagesControl, "org.openoffice.comp.toolkit", szServiceName_AnimatedImagesControl, NULL )
        GET_FACTORY_WITH_IMPL_PREFIX( AnimatedImagesControlModel, "org.openoffice.comp.toolkit", szServiceName_AnimatedImagesControlModel, NULL )
        GET_FACTORY_WITH_IMPL_PREFIX( SpinningProgressControlModel, "org.openoffice.comp.toolkit", szServiceName_SpinningProgressControlModel, NULL )

        if ( rtl_str_compare( sImplementationName, "com.sun.star.awt.comp.AsyncCallback" ) == 0 )
            return comp_AsyncCallback_component_getFactory( sImplementationName, _pServiceManager, _pRegistryKey );
    }
    return pRet;
}
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
