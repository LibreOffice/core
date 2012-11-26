/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <comphelper/stl_types.hxx>
#include <svx/svdobj.hxx>
#include "svx/fmtools.hxx"
#include "fmservs.hxx"

#ifndef _FM_FMOBJFAC_HXX
#include "svx/fmobjfac.hxx"
#endif

#ifndef _FM_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif

#ifndef _FM_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#include "fmshimp.hxx"

#ifndef _FM_FMSHELL_HXX
#include <svx/fmshell.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include "tbxform.hxx"
#include <tools/resid.hxx>

#ifndef _SVX_FMRESIDS_HRC
#include "svx/fmresids.hrc"
#endif
#include <tools/shl.hxx>
#include <svx/dialmgr.hxx>
#include "fmservs.hxx"
#include "tabwin.hxx"
#include "fmexpl.hxx"
#include "filtnav.hxx"

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
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
    void    lcl_initProperty( FmFormObj* _pObject, const ::rtl::OUString& _rPropName, const Any& _rValue )
    {
        try
        {
            Reference< XPropertySet >  xModelSet( _pObject->GetUnoControlModel(), UNO_QUERY );
            if ( xModelSet.is() )
                xModelSet->setPropertyValue( _rPropName, _rValue );
        }
        catch( const Exception& )
        {
            DBG_ERROR( "lcl_initProperty: caught an exception!" );
        }
    }
}

IMPL_LINK(FmFormObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
{
    if(FmFormInventor == pObjFactory->getSdrObjectCreationInfo().getInvent())
    {
        ::rtl::OUString sServiceSpecifier;

        typedef ::std::vector< ::std::pair< ::rtl::OUString, Any > > PropertyValueArray;
        PropertyValueArray aInitialProperties;

        switch(pObjFactory->getSdrObjectCreationInfo().getIdent())
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
                aInitialProperties.push_back( PropertyValueArray::value_type( FM_PROP_TIMEMAX, makeAny( (sal_Int32)( Time( 23, 59, 59, 99 ).GetTime() ) ) ) );
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
        FmFormObj* pFmFormObj = new FmFormObj(
            pObjFactory->getTargetModel(),
            sServiceSpecifier,
            pObjFactory->getSdrObjectCreationInfo().getIdent());

        pObjFactory->setNewSdrObject(pFmFormObj);

        // initialize some properties which we want to differ from the defaults
        for (   PropertyValueArray::const_iterator aInitProp = aInitialProperties.begin();
                aInitProp != aInitialProperties.end();
                ++aInitProp
            )
        {
            lcl_initProperty(
                pFmFormObj,
                aInitProp->first,
                aInitProp->second
            );
        }
    }

    return 0;
}



