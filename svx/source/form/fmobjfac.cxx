/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmobjfac.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:07:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include "svdobj.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _FM_FMOBJFAC_HXX
#include "fmobjfac.hxx"
#endif

#ifndef _FM_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _FM_FMOBJ_HXX
#include "fmobj.hxx"
#endif

#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif

#ifndef _FM_FMSHELL_HXX
#include "fmshell.hxx"
#endif

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_TBXFORM_HXX
#include "tbxform.hxx"
#endif

#ifndef _TOOLS_RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _SVX_TABWIN_HXX
#include "tabwin.hxx"
#endif

#ifndef _SVX_FMEXPL_HXX
#include "fmexpl.hxx"
#endif

#ifndef _SVX_FILTNAV_HXX
#include "filtnav.hxx"
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef SVX_FMPROPBRW_HXX
#include "fmPropBrw.hxx"
#endif

#ifndef _SVX_DATANAVI_HXX
#include "datanavi.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::svxform;

static BOOL bInit = FALSE;

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
        bInit = TRUE;
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
    if (pObjFactory->nInventor == FmFormInventor)
    {
        ::rtl::OUString sServiceSpecifier;

        typedef ::std::vector< ::std::pair< ::rtl::OUString, Any > > PropertyValueArray;
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
        if ( sServiceSpecifier.getLength() )
            pObjFactory->pNewObj = new FmFormObj( sServiceSpecifier, pObjFactory->nIdentifier );
        else
            pObjFactory->pNewObj = new FmFormObj( pObjFactory->nIdentifier );

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



