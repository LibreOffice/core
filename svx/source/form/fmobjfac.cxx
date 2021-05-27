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
#include <config_fuzzers.h>

#include <comphelper/diagnose_ex.hxx>
#include <svx/svdobj.hxx>
#include <svx/fmtools.hxx>
#include <fmservs.hxx>

#include <svx/fmobjfac.hxx>

#include <svx/svdobjkind.hxx>

#include <fmobj.hxx>

#include <svx/fmshell.hxx>

#include <svx/svxids.hrc>
#include <tbxform.hxx>

#include <tabwin.hxx>
#include <fmexpl.hxx>
#include <filtnav.hxx>

#include <fmprop.hxx>
#include <fmPropBrw.hxx>
#include <datanavi.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::svxform;

static bool bInit = false;

FmFormObjFactory::FmFormObjFactory()
{
    if ( bInit )
        return;

    SdrObjFactory::InsertMakeObjectHdl(LINK(this, FmFormObjFactory, MakeObject));


    // register the configuration css::frame::Controller and the NavigationBar
    SvxFmTbxCtlAbsRec::RegisterControl( SID_FM_RECORD_ABSOLUTE );
    SvxFmTbxCtlRecText::RegisterControl( SID_FM_RECORD_TEXT );
    SvxFmTbxCtlRecFromText::RegisterControl( SID_FM_RECORD_FROM_TEXT );
    SvxFmTbxCtlRecTotal::RegisterControl( SID_FM_RECORD_TOTAL );
    SvxFmTbxPrevRec::RegisterControl( SID_FM_RECORD_PREV );
    SvxFmTbxNextRec::RegisterControl( SID_FM_RECORD_NEXT );

    // registering global windows
    FmFieldWinMgr::RegisterChildWindow();
    FmPropBrwMgr::RegisterChildWindow();
    NavigatorFrameManager::RegisterChildWindow();
    DataNavigatorManager::RegisterChildWindow();
#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    FmFilterNavigatorWinMgr::RegisterChildWindow();
#endif

    // register the interface for the Formshell
    FmFormShell::RegisterInterface();

    ImplSmartRegisterUnoServices();
    bInit = true;
}

// create css::form::Form objects
namespace
{
    void    lcl_initProperty( FmFormObj const * _pObject, const OUString& _rPropName, const Any& _rValue )
    {
        try
        {
            Reference< XPropertySet >  xModelSet( _pObject->GetUnoControlModel(), UNO_QUERY );
            if ( xModelSet.is() )
                xModelSet->setPropertyValue( _rPropName, _rValue );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "svx", "lcl_initProperty" );
        }
    }
}

IMPL_STATIC_LINK(FmFormObjFactory, MakeObject, SdrObjCreatorParams, aParams, rtl::Reference<SdrObject>)
{
    rtl::Reference<SdrObject> pNewObj;

    if (aParams.nInventor == SdrInventor::FmForm)
    {
        OUString sServiceSpecifier;

        typedef ::std::vector< ::std::pair< OUString, Any > > PropertyValueArray;
        PropertyValueArray aInitialProperties;

        switch ( aParams.nObjIdentifier )
        {
            case SdrObjKind::FormEdit:
                sServiceSpecifier = FM_COMPONENT_EDIT;
                break;

            case SdrObjKind::FormButton:
                sServiceSpecifier = FM_COMPONENT_COMMANDBUTTON;
                break;

            case SdrObjKind::FormFixedText:
                sServiceSpecifier = FM_COMPONENT_FIXEDTEXT;
                break;

            case SdrObjKind::FormListbox:
                sServiceSpecifier = FM_COMPONENT_LISTBOX;
                break;

            case SdrObjKind::FormCheckbox:
                sServiceSpecifier = FM_COMPONENT_CHECKBOX;
                break;

            case SdrObjKind::FormRadioButton:
                sServiceSpecifier = FM_COMPONENT_RADIOBUTTON;
                break;

            case SdrObjKind::FormGroupBox:
                sServiceSpecifier = FM_COMPONENT_GROUPBOX;
                break;

            case SdrObjKind::FormCombobox:
                sServiceSpecifier = FM_COMPONENT_COMBOBOX;
                break;

            case SdrObjKind::FormGrid:
                sServiceSpecifier = FM_COMPONENT_GRID;
                break;

            case SdrObjKind::FormImageButton:
                sServiceSpecifier = FM_COMPONENT_IMAGEBUTTON;
                break;

            case SdrObjKind::FormFileControl:
                sServiceSpecifier = FM_COMPONENT_FILECONTROL;
                break;

            case SdrObjKind::FormDateField:
                sServiceSpecifier = FM_COMPONENT_DATEFIELD;
                break;

            case SdrObjKind::FormTimeField:
                sServiceSpecifier = FM_COMPONENT_TIMEFIELD;
                aInitialProperties.emplace_back( FM_PROP_TIMEMAX, Any( tools::Time( 23, 59, 59, 999999999 ).GetUNOTime() ) );
                break;

            case SdrObjKind::FormNumericField:
                sServiceSpecifier = FM_COMPONENT_NUMERICFIELD;
                break;

            case SdrObjKind::FormCurrencyField:
                sServiceSpecifier = FM_COMPONENT_CURRENCYFIELD;
                break;

            case SdrObjKind::FormPatternField:
                sServiceSpecifier = FM_COMPONENT_PATTERNFIELD;
                break;

            case SdrObjKind::FormHidden:
                sServiceSpecifier = FM_COMPONENT_HIDDEN;
                break;

            case SdrObjKind::FormImageControl:
                sServiceSpecifier = FM_COMPONENT_IMAGECONTROL;
                break;

            case SdrObjKind::FormFormattedField:
                sServiceSpecifier = FM_COMPONENT_FORMATTEDFIELD;
                break;

            case SdrObjKind::FormNavigationBar:
                sServiceSpecifier = FM_SUN_COMPONENT_NAVIGATIONBAR;
                break;

            case SdrObjKind::FormScrollbar:
                sServiceSpecifier = FM_SUN_COMPONENT_SCROLLBAR;
                aInitialProperties.emplace_back( FM_PROP_BORDER, Any( sal_Int16(0) ) );
                break;

            case SdrObjKind::FormSpinButton:
                sServiceSpecifier = FM_SUN_COMPONENT_SPINBUTTON;
                aInitialProperties.emplace_back( FM_PROP_BORDER, Any( sal_Int16(0) ) );
                break;

            default:
                break;
        }

        // create the actual object
        if ( !sServiceSpecifier.isEmpty() )
            pNewObj = new FmFormObj(aParams.rSdrModel, sServiceSpecifier);
        else
            pNewObj = new FmFormObj(aParams.rSdrModel);

        // initialize some properties which we want to differ from the defaults
        for (const auto& rInitProp : aInitialProperties)
        {
            lcl_initProperty(
                static_cast< FmFormObj* >( pNewObj.get() ),
                rInitProp.first,
                rInitProp.second
            );
        }
    }
    return pNewObj;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
