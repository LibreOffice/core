/*************************************************************************
 *
 *  $RCSfile: fmobjfac.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:16 $
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

#ifndef _UTL_STLTYPES_HXX_
#include <unotools/stl_types.hxx>
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

#ifndef __VC_VCSBXDEF_HXX   // object id's
//#include "vcsbxdef.hxx"
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

#ifndef _SVX_PROPBRW_HXX
#include "propbrw.hxx"
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

#ifndef ENABLEUNICODE
#define WS(a) ::rtl::OUString(UniString(a)
#else
#define WS(a) a
#endif

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
FmFormObjFactory::FmFormObjFactory()
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
    FmExplorerWinMgr::RegisterChildWindow();
    FmFilterNavigatorWinMgr::RegisterChildWindow();

    //////////////////////////////////////////////////////////////////////
    // Interface fuer die Formshell registrieren
    FmFormShell::RegisterInterface(0);

    ImplSmartRegisterUnoServices();

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
IMPL_LINK(FmFormObjFactory, MakeObject, SdrObjFactory*, pObjFactory)
{
    if (pObjFactory->nInventor == FmFormInventor)
    {
        switch (pObjFactory->nIdentifier)
        {
            case OBJ_FM_CONTROL:    // allgemeines Object
            {
                pObjFactory->pNewObj = new FmFormObj();
            }   break;
            case OBJ_FM_EDIT:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_EDIT);
            }   break;
            case OBJ_FM_BUTTON:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_COMMANDBUTTON);
            }   break;
            case OBJ_FM_FIXEDTEXT:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_FIXEDTEXT);
            }   break;
            case OBJ_FM_LISTBOX:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_LISTBOX);
            }   break;
            case OBJ_FM_CHECKBOX:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_CHECKBOX);
            }   break;
            case OBJ_FM_RADIOBUTTON:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_RADIOBUTTON);
            }   break;
            case OBJ_FM_GROUPBOX:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_GROUPBOX);
            }   break;
            case OBJ_FM_COMBOBOX:
            {
                FmFormObj* pNew = new FmFormObj(FM_COMPONENT_COMBOBOX);
                pObjFactory->pNewObj = pNew;

                try
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xModelSet(pNew->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
                    if (xModelSet.is())
                    {
                        sal_Bool bB = sal_True;
                        xModelSet->setPropertyValue(FM_PROP_DROPDOWN, ::com::sun::star::uno::Any(&bB,::getBooleanCppuType()));
                    }
                }
                catch(...)
                {
                }

            }   break;
            case OBJ_FM_GRID:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_GRID);
            }   break;
            case OBJ_FM_IMAGEBUTTON:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_IMAGEBUTTON);
            }   break;
            case OBJ_FM_FILECONTROL:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_FILECONTROL);
            }   break;
            case OBJ_FM_DATEFIELD:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_DATEFIELD);
            }   break;
            case OBJ_FM_TIMEFIELD:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_TIMEFIELD);
            }   break;
            case OBJ_FM_NUMERICFIELD:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_NUMERICFIELD);
            }   break;
            case OBJ_FM_CURRENCYFIELD:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_CURRENCYFIELD);
            }   break;
            case OBJ_FM_PATTERNFIELD:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_PATTERNFIELD);
            }   break;
            case OBJ_FM_HIDDEN:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_HIDDEN);
            }   break;
            case OBJ_FM_IMAGECONTROL:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_IMAGECONTROL);
            }   break;
            case OBJ_FM_FORMATTEDFIELD:
            {
                pObjFactory->pNewObj = new FmFormObj(FM_COMPONENT_FORMATTEDFIELD);
            }   break;
            default:
                return 0;
        }
    }

    return 0;
}



