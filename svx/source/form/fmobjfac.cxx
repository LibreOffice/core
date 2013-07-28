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

#include <tools/diagnose_ex.h>
#include <comphelper/stl_types.hxx>
#include <svx/svdobj.hxx>
#include "svx/fmtools.hxx"
#include "fmservs.hxx"

#include "svx/fmobjfac.hxx"

#include <svx/fmglob.hxx>

#include "fmobj.hxx"
#include "fmshimp.hxx"

#include <svx/fmshell.hxx>

#include <svx/svxids.hrc>
#include "tbxform.hxx"
#include <tools/resid.hxx>

#include "svx/fmresids.hrc"
#include <tools/shl.hxx>
#include <svx/dialmgr.hxx>
#include "tabwin.hxx"
#include "fmexpl.hxx"
#include "filtnav.hxx"

#include "fmprop.hrc"
#include "fmPropBrw.hxx"
#include "datanavi.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::svxform;

static sal_Bool bInit = sal_False;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormObjFactory::FmFormObjFactory()
{
    if ( !bInit )
    {
        SdrObjFactory::InsertMakeObjectHdl(LINK(this, FmFormObjFactory, MakeObject));

        //////////////////////////////////////////////////////////////////////
        // Konfigurations-::com::sun::star::frame::Controller und NavigationBar registrieren
        SvxFmTbxCtlConfig::RegisterControl( SID_FM_CONFIG );
        SvxFmTbxCtlAbsRec::RegisterControl( SID_FM_RECORD_ABSOLUTE );
        SvxFmTbxCtlRecText::RegisterControl( SID_FM_RECORD_TEXT );
        SvxFmTbxCtlRecFromText::RegisterControl( SID_FM_RECORD_FROM_TEXT );
        SvxFmTbxCtlRecTotal::RegisterControl( SID_FM_RECORD_TOTAL );
        SvxFmTbxPrevRec::RegisterControl( SID_FM_RECORD_PREV );
        SvxFmTbxNextRec::RegisterControl( SID_FM_RECORD_NEXT );
        ControlConversionMenuController::RegisterControl(SID_FM_CHANGECONTROLTYPE);

        // Registrieung von globalen fenstern
        FmFieldWinMgr::RegisterChildWindow();
        FmPropBrwMgr::RegisterChildWindow();
        NavigatorFrameManager::RegisterChildWindow();
        DataNavigatorManager::RegisterChildWindow();
        FmFilterNavigatorWinMgr::RegisterChildWindow();

        //////////////////////////////////////////////////////////////////////
        // Interface fuer die Formshell registrieren
        FmFormShell::RegisterInterface(0);

        ImplSmartRegisterUnoServices();
        bInit = sal_True;
    }
}


/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
FmFormObjFactory::~FmFormObjFactory()
{
}


/*************************************************************************
|*
|* ::com::sun::star::form::Form-Objekte erzeugen
|*
\************************************************************************/
namespace
{
    void    lcl_initProperty( FmFormObj* _pObject, const OUString& _rPropName, const Any& _rValue )
    {
        try
        {
            Reference< XPropertySet >  xModelSet( _pObject->GetUnoControlModel(), UNO_QUERY );
            if ( xModelSet.is() )
                xModelSet->setPropertyValue( _rPropName, _rValue );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "lcl_initProperty: caught an exception!" );
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

IMPL_LINK(FmFormObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
{
    if (pObjFactory->nInventor == FmFormInventor)
    {
        OUString sServiceSpecifier;

        typedef ::std::vector< ::std::pair< OUString, Any > > PropertyValueArray;
        PropertyValueArray aInitialProperties;

        switch ( pObjFactory->nIdentifier )
        {
            case OBJ_FM_EDIT:
                sServiceSpecifier = FM_COMPONENT_EDIT;
                break;

            case OBJ_FM_BUTTON:
                sServiceSpecifier = FM_COMPONENT_COMMANDBUTTON;
                break;

            case OBJ_FM_FIXEDTEXT:
                sServiceSpecifier = FM_COMPONENT_FIXEDTEXT;
                break;

            case OBJ_FM_LISTBOX:
                sServiceSpecifier = FM_COMPONENT_LISTBOX;
                break;

            case OBJ_FM_CHECKBOX:
                sServiceSpecifier = FM_COMPONENT_CHECKBOX;
                break;

            case OBJ_FM_RADIOBUTTON:
                sServiceSpecifier = FM_COMPONENT_RADIOBUTTON;
                break;

            case OBJ_FM_GROUPBOX:
                sServiceSpecifier = FM_COMPONENT_GROUPBOX;
                break;

            case OBJ_FM_COMBOBOX:
                sServiceSpecifier = FM_COMPONENT_COMBOBOX;
                break;

            case OBJ_FM_GRID:
                sServiceSpecifier = FM_COMPONENT_GRID;
                break;

            case OBJ_FM_IMAGEBUTTON:
                sServiceSpecifier = FM_COMPONENT_IMAGEBUTTON;
                break;

            case OBJ_FM_FILECONTROL:
                sServiceSpecifier = FM_COMPONENT_FILECONTROL;
                break;

            case OBJ_FM_DATEFIELD:
                sServiceSpecifier = FM_COMPONENT_DATEFIELD;
                break;

            case OBJ_FM_TIMEFIELD:
                sServiceSpecifier = FM_COMPONENT_TIMEFIELD;
                aInitialProperties.push_back( PropertyValueArray::value_type( FM_PROP_TIMEMAX, makeAny( (sal_Int64)( Time( 23, 59, 59, 999999999 ).GetTime() ) ) ) );
                break;

            case OBJ_FM_NUMERICFIELD:
                sServiceSpecifier = FM_COMPONENT_NUMERICFIELD;
                break;

            case OBJ_FM_CURRENCYFIELD:
                sServiceSpecifier = FM_COMPONENT_CURRENCYFIELD;
                break;

            case OBJ_FM_PATTERNFIELD:
                sServiceSpecifier = FM_COMPONENT_PATTERNFIELD;
                break;

            case OBJ_FM_HIDDEN:
                sServiceSpecifier = FM_COMPONENT_HIDDEN;
                break;

            case OBJ_FM_IMAGECONTROL:
                sServiceSpecifier = FM_COMPONENT_IMAGECONTROL;
                break;

            case OBJ_FM_FORMATTEDFIELD:
                sServiceSpecifier = FM_COMPONENT_FORMATTEDFIELD;
                break;

            case OBJ_FM_NAVIGATIONBAR:
                sServiceSpecifier = FM_SUN_COMPONENT_NAVIGATIONBAR;
                break;

            case OBJ_FM_SCROLLBAR:
                sServiceSpecifier = FM_SUN_COMPONENT_SCROLLBAR;
                aInitialProperties.push_back( PropertyValueArray::value_type( FM_PROP_BORDER, makeAny( (sal_Int16)0 ) ) );
                break;

            case OBJ_FM_SPINBUTTON:
                sServiceSpecifier = FM_SUN_COMPONENT_SPINBUTTON;
                aInitialProperties.push_back( PropertyValueArray::value_type( FM_PROP_BORDER, makeAny( (sal_Int16)0 ) ) );
                break;
        }

        // create the actual object
        if ( !sServiceSpecifier.isEmpty() )
            pObjFactory->pNewObj = new FmFormObj(sServiceSpecifier);
        else
            pObjFactory->pNewObj = new FmFormObj();

        // initialize some properties which we want to differ from the defaults
        for (   PropertyValueArray::const_iterator aInitProp = aInitialProperties.begin();
                aInitProp != aInitialProperties.end();
                ++aInitProp
            )
        {
            lcl_initProperty(
                static_cast< FmFormObj* >( pObjFactory->pNewObj ),
                aInitProp->first,
                aInitProp->second
            );
        }
    }

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
