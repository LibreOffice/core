/*************************************************************************
 *
 *  $RCSfile: fmshimp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-20 14:18:56 $
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

#if STLPORT_VERSION>=321
#include <math.h>   // prevent conflict between exception and std::exception
#endif

#ifndef _SVX_GRIDCOLS_HXX
#include "gridcols.hxx"
#endif

#ifndef _E3D_OBJ3D_HXX
#include <obj3d.hxx>
#endif

#ifndef _SVX_FMVWIMP_HXX
#include "fmvwimp.hxx"
#endif

#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif

#ifndef _SVDPAGV_HXX //autogen
#include "svdpagv.hxx"
#endif

#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX //autogen
#include <dialmgr.hxx>
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif // _SVX_FMRESIDS_HRC

#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif

#ifndef _MULTIPRO_HXX
#include "multipro.hxx"
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XTEXTCOMPONENT_HPP_
#include <com/sun/star/awt/XTextComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCONTROL_HPP_
#include <com/sun/star/form/XBoundControl.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRID_HPP_
#include <com/sun/star/form/XGrid.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDPEER_HPP_
#include <com/sun/star/form/XGridPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODESELECTOR_HPP_
#include <com/sun/star/util/XModeSelector.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <fmglob.hxx>
#endif

#ifndef _SVDITER_HXX //autogen
#include <svditer.hxx>
#endif

#ifndef _SFX_INIMGR_HXX //autogen
#include <sfx2/inimgr.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _SFXVIEWSH_HXX //autogen wg. SfxViewShell
#include <sfx2/viewsh.hxx>
#endif

#ifndef _SFXVIEWFRM_HXX //autogen wg. SfxViewFrame
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SFXFRAME_HXX //autogen wg. SfxFrame
#include <sfx2/frame.hxx>
#endif

#ifndef _SVT_SDBPARSE_HXX
#include <svtools/sdbparse.hxx>
#endif

#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif // _SVX_FMSERVS_HXX

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif
#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif

#ifndef _SVX_FMURL_HXX
#include "fmurl.hxx"
#endif

#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_GUARDING_HXX_
#include <comphelper/guarding.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#if SUPD < 583

#ifndef _USR_SMARTCONV_HXX_
#include <usr/smartconv.hxx>
#endif

#include <smart/com/sun/star/frame/xframe.hxx>
#include <smart/com/sun/star/awt/XControlContainer.hxx>
#include <usr/xiface.hxx>

// this method is teporary ... as long as we use some implementations in foreign projects
// which do not work with UNO3
template <class SMARTTYPE, class UNO3TYPE>
void convertIFace(SMARTTYPE* _pIn, ::com::sun::star::uno::Reference<UNO3TYPE>& _rxOut)
{
    ::comphelper::InterfaceRef xUno3IFace;
    ::usr::convertUsr2UnoInterface(xUno3IFace, static_cast<XInterface*>(_pIn));
    _rxOut = ::com::sun::star::uno::Reference<UNO3TYPE>::query(xUno3IFace);
}

// this method is teporary ... as long as we use some implementations in foreign projects
// which do not work with UNO3
template <class SMARTREF>
void convertIFace(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxIn, SMARTREF& _rxOut)
{
    XInterfaceRef xUsr3IFace;
    ::usr::convertUno2UsrInterface(xUsr3IFace, _rxIn);
    _rxOut = SMARTREF(xUsr3IFace, USR_QUERY);
}

#endif

extern sal_Int16 ControllerSlotMap[];

extern sal_Int16 AutoSlotMap[];

// wird fuer Invalidate verwendet -> mitpflegen
extern sal_uInt16 DatabaseSlotMap[];

// wird fuer Invalidate verwendet -> mitpflegen
// aufsteigend sortieren !!!!!!
sal_Int16 DlgSlotMap[] =    // slots des Controllers
{
    SID_FM_CTL_PROPERTIES,
    SID_FM_PROPERTIES,
    SID_FM_TAB_DIALOG,
    SID_FM_ADD_FIELD,
    SID_FM_SHOW_FMEXPLORER,
    SID_FM_FIELDS_CONTROL,
    SID_FM_SHOW_PROPERTIES,
    SID_FM_PROPERTY_CONTROL,
    SID_FM_FMEXPLORER_CONTROL,
    0
};

sal_uInt16 ModifySlotMap[] =    // slots des Controllers
{
    SID_FM_RECORD_NEXT,
    SID_FM_RECORD_NEW,
    SID_FM_RECORD_SAVE,
    SID_FM_RECORD_UNDO,
    0
};

sal_Int16 SelObjectSlotMap[] =  // vom SelObject abhaengige Slots
{
    SID_FM_CONVERTTO_EDIT,
    SID_FM_CONVERTTO_BUTTON,
    SID_FM_CONVERTTO_FIXEDTEXT,
    SID_FM_CONVERTTO_LISTBOX,
    SID_FM_CONVERTTO_CHECKBOX,
    SID_FM_CONVERTTO_RADIOBUTTON,
    SID_FM_CONVERTTO_GROUPBOX,
    SID_FM_CONVERTTO_COMBOBOX,
    SID_FM_CONVERTTO_GRID,
    SID_FM_CONVERTTO_IMAGEBUTTON,
    SID_FM_CONVERTTO_FILECONTROL,
    SID_FM_CONVERTTO_DATE,
    SID_FM_CONVERTTO_TIME,
    SID_FM_CONVERTTO_NUMERIC,
    SID_FM_CONVERTTO_CURRENCY,
    SID_FM_CONVERTTO_PATTERN,
    SID_FM_CONVERTTO_IMAGECONTROL,
    SID_FM_CONVERTTO_FORMATTED,

    SID_FM_FMEXPLORER_CONTROL,

    0
};

// die folgenden Arrays muessen kosistent sein, also einander entsprechende Eintraege an der selben relativen Position
// innerhalb ihres jeweiligen Arrays stehen
sal_Int16 nConvertSlots[] =
{
    SID_FM_CONVERTTO_EDIT,
    SID_FM_CONVERTTO_BUTTON,
    SID_FM_CONVERTTO_FIXEDTEXT,
    SID_FM_CONVERTTO_LISTBOX,
    SID_FM_CONVERTTO_CHECKBOX,
    SID_FM_CONVERTTO_RADIOBUTTON,
    SID_FM_CONVERTTO_GROUPBOX,
    SID_FM_CONVERTTO_COMBOBOX,
    SID_FM_CONVERTTO_GRID,
    SID_FM_CONVERTTO_IMAGEBUTTON,
    SID_FM_CONVERTTO_FILECONTROL,
    SID_FM_CONVERTTO_DATE,
    SID_FM_CONVERTTO_TIME,
    SID_FM_CONVERTTO_NUMERIC,
    SID_FM_CONVERTTO_CURRENCY,
    SID_FM_CONVERTTO_PATTERN,
    SID_FM_CONVERTTO_IMAGECONTROL,
    SID_FM_CONVERTTO_FORMATTED
};

sal_Int16 nCreateSlots[] =
{
    SID_FM_EDIT,
    SID_FM_PUSHBUTTON,
    SID_FM_FIXEDTEXT,
    SID_FM_LISTBOX,
    SID_FM_CHECKBOX,
    SID_FM_RADIOBUTTON,
    SID_FM_GROUPBOX,
    SID_FM_COMBOBOX,
    SID_FM_DBGRID,
    SID_FM_IMAGEBUTTON,
    SID_FM_FILECONTROL,
    SID_FM_DATEFIELD,
    SID_FM_TIMEFIELD,
    SID_FM_NUMERICFIELD,
    SID_FM_CURRENCYFIELD,
    SID_FM_PATTERNFIELD,
    SID_FM_IMAGECONTROL,
    SID_FM_FORMATTEDFIELD
};

sal_Int16 nObjectTypes[] =
{
    OBJ_FM_EDIT,
    OBJ_FM_BUTTON,
    OBJ_FM_FIXEDTEXT,
    OBJ_FM_LISTBOX,
    OBJ_FM_CHECKBOX,
    OBJ_FM_RADIOBUTTON,
    OBJ_FM_GROUPBOX,
    OBJ_FM_COMBOBOX,
    OBJ_FM_GRID,
    OBJ_FM_IMAGEBUTTON,
    OBJ_FM_FILECONTROL,
    OBJ_FM_DATEFIELD,
    OBJ_FM_TIMEFIELD,
    OBJ_FM_NUMERICFIELD,
    OBJ_FM_CURRENCYFIELD,
    OBJ_FM_PATTERNFIELD,
    OBJ_FM_IMAGECONTROL,
    OBJ_FM_FORMATTEDFIELD
};

//------------------------------------------------------------------------------
sal_Bool FmXBoundFormFieldIterator::ShouldStepInto(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rContainer) const
{
    if (_rContainer == m_xStartingPoint)
        // would be quite stupid to step over the root ....
        return sal_True;

    return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>(_rContainer, ::com::sun::star::uno::UNO_QUERY).is();
}

//------------------------------------------------------------------------------
sal_Bool FmXBoundFormFieldIterator::ShouldHandleElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rElement)
{
    if (!_rElement.is())
        // NULL element
        return sal_False;

    if (::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>(_rElement, ::com::sun::star::uno::UNO_QUERY).is() || ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid>(_rElement, ::com::sun::star::uno::UNO_QUERY).is())
        // a forms or a grid
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(_rElement, ::com::sun::star::uno::UNO_QUERY);
    if (!xSet.is() || !::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
        // no "BoundField" property
        return sal_False;

    ::com::sun::star::uno::Any aVal( xSet->getPropertyValue(FM_PROP_BOUNDFIELD) );
    if (aVal.getValueTypeClass() != ::com::sun::star::uno::TypeClass_INTERFACE)
        // void or invalid property value
        return sal_False;

    if (!((::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aVal.getValue())->is())
        // empty ref
        return sal_False;

    return sal_True;
}

//==============================================================================

DECL_CURSOR_ACTION_THREAD(FmMoveToLastThread);
IMPL_CURSOR_ACTION_THREAD(FmMoveToLastThread, SVX_RES(RID_STR_MOVING_CURSOR), last());

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
sal_Bool hasObject(SdrObjListIter& rIter, SdrObject* pObj)
{
    sal_Bool bFound = sal_False;
    while (rIter.IsMore() && !bFound)
        bFound = pObj == rIter.Next();

    rIter.Reset();
    return bFound;
}

//------------------------------------------------------------------------------
sal_Bool isControlList(const SdrMarkList& rMarkList)
{
    // enthaelt die liste nur Controls und mindestens ein control
    sal_uInt32 nMarkCount = rMarkList.GetMarkCount();
    sal_Bool  bControlList = nMarkCount != 0;

    sal_Bool bHadAnyLeafs = sal_False;

    for (sal_uInt32 i = 0; i < nMarkCount && bControlList; i++)
    {
        SdrObject *pObj = rMarkList.GetMark(i)->GetObj();
        E3dObject* pAs3DObject = PTR_CAST(E3dObject, pObj);
        // E3dObject's do not contain any 2D-objects (by definition)
        // we need this extra check here : an E3dObject->IsGroupObject says "YES", but an SdrObjListIter working
        // with an E3dObject doesn't give me any Nodes (E3dObject has a sub list, but no members in that list,
        // cause there implementation differs from the one of "normal" SdrObject's. Unfortunally SdrObject::IsGroupObject
        // doesn't check the element count of the sub list, which is simply a bug in IsGroupObject we can't fix at the moment).
        // So at the end of this function bControlList would have the same value it was initialized with above : sal_True
        // And this would be wrong :)
        // 03.02.00 - 72529 - FS
        if (!pAs3DObject)
            if (pObj->IsGroupObject())
            {
                SdrObjListIter aIter(*pObj->GetSubList());
                while (aIter.IsMore() && bControlList)
                {
                    bControlList = FmFormInventor == aIter.Next()->GetObjInventor();
                    bHadAnyLeafs = sal_True;
                }
            }
            else
            {
                bHadAnyLeafs = sal_True;
                bControlList = FmFormInventor == pObj->GetObjInventor();
            }
    }

    return bControlList && bHadAnyLeafs;
}

//------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> GetForm(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xElement, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xParent;
    if (xChild.is())
        xParent = xChild->getParent();

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm(xParent, ::com::sun::star::uno::UNO_QUERY);
    if (!xForm.is() && xParent.is())
        xForm = GetForm(xParent);

    return xForm;
}

//------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> GetForm(const FmFormObj* pObj)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm;
    if (pObj)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent> xFormComponent(pObj->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xFormComponent.is())
            xForm = ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>(xFormComponent->getParent(), ::com::sun::star::uno::UNO_QUERY);
    }
    return xForm;
}

//------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> FmXFormShell::DetermineCurForm(const SdrMarkList& rMarkList, sal_Bool& bMixedWithFormControls)
{
    // gibt es eine aktuelle Form, wird ermittelt aus den selektierten Objekten
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xReferenceForm;
    bMixedWithFormControls = sal_False;
    sal_uInt32 nMarkCount = rMarkList.GetMarkCount();

    sal_Bool bHasFormControls = sal_False;
    sal_Bool bHasForeignControls = sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm;
    for (sal_uInt32 i=0; i<nMarkCount; i++)
    {
        SdrObject* pObj = rMarkList.GetMark(i)->GetObj();
        if (pObj->GetObjInventor() == FmFormInventor)
        {
            FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
            xForm = GetForm(pFormObj);

            if (!xReferenceForm.is())
            {
                xReferenceForm = xForm;
                bHasFormControls = sal_True;
            }
            else if (xForm.is() && (::com::sun::star::form::XForm*)xReferenceForm.get() != (::com::sun::star::form::XForm*)xForm.get())
            {
                // mehr als eine Form selektiert
                bMixedWithFormControls = sal_True;
                return ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>();
            }
        }
        else if (pObj->IsGroupObject())
        {
            SdrObjListIter aIter(*pObj->GetSubList());
            while (aIter.IsMore())
            {
                SdrObject* pObj = aIter.Next();
                if (pObj->GetObjInventor() == FmFormInventor)
                {
                    FmFormObj* pFormObj = PTR_CAST(FmFormObj, pObj);
                    xForm = GetForm(pFormObj);

                    if (!xReferenceForm.is())
                    {
                        xReferenceForm = xForm;
                        bHasFormControls = sal_True;
                    }
                    else if (xForm.is() && (::com::sun::star::form::XForm*)xReferenceForm.get() != (::com::sun::star::form::XForm*)xForm.get())
                    {
                        // mehr als eine Form selektiert
                        bMixedWithFormControls = sal_True;
                        return ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>();
                    }
                } else
                    // irgendein Nicht-Form-Control
                    bHasForeignControls = sal_True;
            }
        } else
            bHasForeignControls = sal_True;
    }

    // hierher komme ich nicht mehr, wenn Controls aus mindestens zwei Forms selektiert sind, also ermittelt sich
    // bMixedWithFormControls wie folgt
    bMixedWithFormControls = bHasForeignControls && bHasFormControls;
    return xReferenceForm;
}


//========================================================================
// class FmXFormShell
//========================================================================
//------------------------------------------------------------------------
FmXFormShell::FmXFormShell( FmFormShell* _pShell, SfxViewFrame* _pViewFrame )
        : ::cppu::OComponentHelper(m_aMutex)
        ,m_pShell(_pShell)
        ,m_bDatabaseBar(sal_False)
        ,m_eNavigate(::com::sun::star::form::NavigationBarMode_NONE)
        ,m_bActiveModified(sal_False)
        ,m_bTrackProperties(sal_True)
        ,m_pCheckForRemoval(NULL)
        ,m_bInActivate(sal_False)
        ,m_bSetFocus(sal_False)
        ,m_nLockSlotInvalidation(0)
        ,m_nInvalidationEvent(0)
        ,m_bFilterMode(sal_False)
        ,m_bHadPropBrw(sal_False)
        ,m_pMainFrameInterceptor(NULL)
        ,m_pExternalViewInterceptor(NULL)
        ,m_bChangingDesignMode(sal_False)
{
    m_aMarkTimer.SetTimeout(100);
    m_aMarkTimer.SetTimeoutHdl(LINK(this,FmXFormShell,OnTimeOut));

    String sUseWizards(SFX_INIMANAGER()->Get(SFX_GROUP_DATABASE, String::CreateFromAscii("UseWizards")));
    m_bUseWizards = (!sUseWizards.Len() || (sUseWizards.CompareToAscii("1") == COMPARE_EQUAL));
        // wenn kein Ini-Eintrag oder =1

    // we are a DispatchInterceptor, so we want to be inserted into the frame's dispatch chain, thus having
    // a chance for frame-spanning communication (via UNO, not slots)
    SfxFrame* pFrame = _pViewFrame ? _pViewFrame->GetFrame() : NULL;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame> xUnoFrame;
    if( pFrame)
#if SUPD > 582
        xUnoFrame = pFrame->GetFrameInterface();
#else
        convertIFace((XFrame*)pFrame->GetFrameInterface(),xUnoFrame);
#endif
    else
        xUnoFrame = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>(NULL);

    // to prevent deletion of this we acquire our refcounter once
    ::comphelper::increment(OComponentHelper::m_refCount);

    // dispatch interception for the frame
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception> xSupplier(xUnoFrame, ::com::sun::star::uno::UNO_QUERY);
    m_pMainFrameInterceptor = new FmXDispatchInterceptorImpl(xSupplier, this, 0);
    m_pMainFrameInterceptor->acquire();

    m_xAttachedFrame = xUnoFrame;

    // correct the refcounter
    ::comphelper::decrement(OComponentHelper::m_refCount);
}

//------------------------------------------------------------------------
FmXFormShell::~FmXFormShell()
{
    if (!rBHelper.bDisposed)
        dispose();

    ::osl::ClearableMutexGuard aGuard(m_aAsyncSafety);
    if (HasAnyPendingCursorAction())
        CancelAnyPendingCursorAction();
    aGuard.clear();

    if (m_pCheckForRemoval)
    {
        delete m_pCheckForRemoval;
        m_pCheckForRemoval = NULL;
    }

    DBG_ASSERT(!m_nInvalidationEvent, "FmXFormShell::~FmXFormShell : still have an invalidation event !");
        // should habe been deleted while beeing disposed

    m_aMarkTimer.Stop();
}

//------------------------------------------------------------------
::com::sun::star::uno::Any SAL_CALL FmXFormShell::queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException )
{

    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface(type,
        //  static_cast< ::com::sun::star::lang::XEventListener*>(this),
        static_cast< ::com::sun::star::container::XContainerListener*>(this),
        static_cast< ::com::sun::star::view::XSelectionChangeListener*>(this),
        static_cast< ::com::sun::star::sdbc::XRowSetListener*>(this),
        static_cast< ::com::sun::star::beans::XPropertyChangeListener*>(this),
        static_cast< ::com::sun::star::util::XModifyListener*>(this),
        //  static_cast< ::com::sun::star::frame::XDispatchProviderInterceptor*>(this),
        //  static_cast< ::com::sun::star::frame::XDispatchProvider*>(this),
        static_cast< ::com::sun::star::form::XFormControllerListener*>(this));
    if(aRet.hasValue())
        return aRet;
    return OComponentHelper::queryInterface(type);
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL FmXFormShell::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > aTypes(OComponentHelper::getTypes());
    aTypes.realloc(6);
    ::com::sun::star::uno::Type* pTypes = aTypes.getArray();

    pTypes[aTypes.getLength()-6] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener>*)0);
    pTypes[aTypes.getLength()-5] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener>*)0);
    pTypes[aTypes.getLength()-4] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSetListener>*)0);
    pTypes[aTypes.getLength()-3] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener>*)0);
    pTypes[aTypes.getLength()-2] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener>*)0);
    pTypes[aTypes.getLength()-1] = ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormControllerListener>*)0);

    return aTypes;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL FmXFormShell::getImplementationId() throw(::com::sun::star::uno::RuntimeException)
{
    return ::form::OImplementationIds::getImplementationId(getTypes());
}
//  EventListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::disposing(const ::com::sun::star::lang::EventObject& e) throw( ::com::sun::star::uno::RuntimeException )
{
    if (m_xActiveController == e.Source)
    {
        // wird der Controller freigeben dann alles loslassen
        stopListening();
        m_xActiveForm = NULL;
        m_xActiveController = NULL;
        m_xNavigationController = NULL;

        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
    }

    if (e.Source == m_xExternalViewController)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xFormController(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY);
        if (xFormController.is())
            xFormController->removeActivateListener((::com::sun::star::form::XFormControllerListener*)this);

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xComp(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY);
        if (xComp.is())
            xComp->removeEventListener((::com::sun::star::lang::XEventListener*)(::com::sun::star::beans::XPropertyChangeListener*)this);

        m_xExternalViewController = NULL;
        m_xExternalDisplayedForm = NULL;
        m_xExtViewTriggerController = NULL;

        InvalidateSlot(SID_FM_VIEW_AS_GRID, sal_True, sal_False);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
    if (evt.PropertyName == FM_PROP_ISMODIFIED)
    {
        if (!::comphelper::getBOOL(evt.NewValue))
            m_bActiveModified = sal_False;
    }
    else if (evt.PropertyName == FM_PROP_ISNEW)
    {
        if (!::comphelper::getBOOL(evt.NewValue))
            m_bActiveModified = sal_False;
    }
    else if (evt.PropertyName == FM_PROP_ROWCOUNT)
    {
        // Das gleich folgenden Update erzwingt ein Neu-Painten der entsprechenden Slots. Wenn ich mich aber hier nicht
        // in dem HauptThread der Applikation befinde (weil zum Beispiel ein Cursor gerade Datensaetze zaehlt und mir dabei
        // immer diese PropertyChanges beschert), kann sich das mit en normalen Paints im HauptThread der Applikation beissen.
        // (Solche Paints passieren zum Beispiel, wenn man einfach nur eine andere Applikation ueber das Office legt und wieder
        // zurueckschaltet).
        // Deshalb die Benutzung des SolarMutex, der sichert das ab.
        ::vos::IMutex& rSolarSafety = Application::GetSolarMutex();
        if (rSolarSafety.tryToAcquire())
        {
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_RECORD_TOTAL , sal_True, sal_False);
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Update(SID_FM_RECORD_TOTAL);
            rSolarSafety.release();
        }
        else
        {
            // with the following the slot is invalidated asynchron
            LockSlotInvalidation(sal_True);
            InvalidateSlot(SID_FM_RECORD_TOTAL , sal_True, sal_False);
            LockSlotInvalidation(sal_False);
        }
    }
    else if (m_xParser.is())
    {
        try
        {
            if (evt.PropertyName == FM_PROP_ACTIVECOMMAND)
                m_xParser->setQuery(::comphelper::getString(evt.NewValue));
            else if (evt.PropertyName == FM_PROP_FILTER_CRITERIA)
            {
                if (m_xParser->getFilter() != ::comphelper::getString(evt.NewValue))
                    m_xParser->setFilter(::comphelper::getString(evt.NewValue));
            }
            else if (evt.PropertyName == FM_PROP_SORT)
            {
                if (m_xParser->getOrder() != ::comphelper::getString(evt.NewValue))
                    m_xParser->setOrder(::comphelper::getString(evt.NewValue));
            }
        }
        catch(...)
        {
        }

    }

    // this may be called from a non-main-thread so invalidate the shell asynchronously
    LockSlotInvalidation(sal_True);
    InvalidateSlot(0, 0, 0);        // special meaning : invalidate m_pShell
    LockSlotInvalidation(sal_False);
}

// ::com::sun::star::util::XModifyListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::modified(const ::com::sun::star::lang::EventObject& rEvent)
{
    if (!m_bActiveModified)
    {
        m_bActiveModified = sal_True;
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(ModifySlotMap);
    }
}
// #endif

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> FmXFormShell::interceptedQueryDispatch(sal_uInt16 _nId, const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAsyncSafety);

    if (!m_pShell)
        return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>();
    // if we have no shell we are disposed, so we disposed all our dispatchers, too, so there is no need to supply a dispatcher
    // (and no possibility : without our shell, methods like GetPageForms won't work properly)

    // check if it is one of the form navigation urls
    static ::rtl::OUString sFormNavUrls[] = {
        FMURL_RECORD_MOVEFIRST,
        FMURL_RECORD_MOVEPREV,
        FMURL_RECORD_MOVENEXT,
        FMURL_RECORD_MOVELAST,
        FMURL_RECORD_MOVETONEW,
        FMURL_RECORD_UNDO
    };
    static sal_Int16 nAccordingSlots[] = {
        SID_FM_RECORD_FIRST,
        SID_FM_RECORD_PREV,
        SID_FM_RECORD_NEXT,
        SID_FM_RECORD_LAST,
        SID_FM_RECORD_NEW,
        SID_FM_RECORD_UNDO
    };
    ::rtl::OUString sMark;
    UniString sAccessPath,sExternalCheck,sPageId;
    for (sal_Int16 i=0; i<sizeof(sFormNavUrls)/sizeof(sFormNavUrls[0]); ++i)
    {
        if (aURL.Main.equals(sFormNavUrls[i]))
        {
            sMark = aURL.Mark;
            DBG_ASSERT(sMark.getLength(), "FmXFormShell::queryDispatch : invalid ::com::sun::star::util::URL !");
                // form navigation slots should always have a mark describing their form model's access paths (FmXFormController builds such a mark)

            sAccessPath = sMark.getStr();

            // check if it comes from our external form grid view
            UniString fComponentName = FMURL_COMPONENT_FORMGRIDVIEW;
            sExternalCheck = fComponentName;
//          sExternalCheck = (UniString)FMURL_COMPONENT_FORMGRIDVIEW;
            INetURLObject aExternalCheck(sExternalCheck);
            if (UniString(aExternalCheck.GetURLPath()) == sAccessPath)
            {   // it comes from the external dispatcher
                // -> correct the access path
                DBG_ASSERT(m_xExternalDisplayedForm.is() && m_xExternalViewController.is(),
                    "FmXFormShell::queryDispatch : where did this dispatch request come from ?");
                sAccessPath = GetAccessPathFromForm(m_xExternalDisplayedForm, GetPageId(m_xExternalDisplayedForm));
                sMark = sAccessPath;

                DBG_ASSERT(_nId == 1, "FmXFormShell::queryDispatch : where did this came from ?");
                // the FmXDispatchInterceptorImpl which forwarded this request should be m_pExternalViewInterceptor, and this interceptor
                // should have ID 1 ...
            }
#if DBG_UTIL
            else
            {
                DBG_ASSERT(_nId == 0, "FmXFormShell::queryDispatch : where did this came from ?");
                // the FmXDispatchInterceptorImpl which forwarded this request should be m_pMainFrameInterceptor, and this interceptor
                // should have ID 0 ...
            }
#endif

            // get the form the dispatcher is requested for
            // first the page id
            String sOriginalPathWithPagePrefix = sAccessPath;
            xub_StrLen nSepPos = sAccessPath.Search('\\');
            DBG_ASSERT(nSepPos != STRING_NOTFOUND, "FmXFormShell::queryDispatch : invalid ::com::sun::star::util::URL mark (no page prefix) !");
            sPageId = sAccessPath.Copy(0, nSepPos);
            sAccessPath = sAccessPath.Copy(nSepPos + 1, STRING_LEN);

            // from this id the forms collection and the form
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xPageForms(GetPageForms(sPageId), ::com::sun::star::uno::UNO_QUERY);
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xAffectedForm(getElementFromAccessPath(xPageForms, UniString(sAccessPath)), ::com::sun::star::uno::UNO_QUERY);
            DBG_ASSERT(xAffectedForm.is(), "FmXFormShell::queryDispatch : could not retrieve a form form the request !");


            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xFormProps(xAffectedForm, ::com::sun::star::uno::UNO_QUERY);
            ::com::sun::star::form::NavigationBarMode eMode = *(::com::sun::star::form::NavigationBarMode*)xFormProps->getPropertyValue(FM_PROP_NAVIGATION).getValue();
            if (eMode != ::com::sun::star::form::NavigationBarMode_CURRENT)
            {   // we can't supply a dispatcher for that : else we would have to listen to all operations on the parent
                // form and to all ops on the affected form itself, just to keep the state up-to-date. This would be too much to do ...
                // 73233 - 22.02.00 - FS
                return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>();
            }

            // get the dispatcher array for the form
            SingleFormDispatchers& aDispatchers = m_aNavigationDispatcher[sMark];
                // the [] operator will create a new one if it didn't exist before
            if (!aDispatchers.size())
            {   // it was a new one -> fill it with initial NULL values
                for (sal_Int16 j=0; j<sizeof(sFormNavUrls)/sizeof(sFormNavUrls[0]); ++j)
                    aDispatchers.insert(aDispatchers.begin(), (FmFormNavigationDispatcher*)NULL);
            }

            FmFormNavigationDispatcher*& pRequestedDispatcher = aDispatchers[i];
            if (!pRequestedDispatcher)
            {
                // nobody requested a dispatcher for this form and this slot before -> create a new one
                m_pShell->GetViewShell()->GetViewFrame()->GetBindings().DENTERREGISTRATIONS();
                pRequestedDispatcher = new FmFormNavigationDispatcher(aURL, nAccordingSlots[i], m_pShell->GetViewShell()->GetViewFrame()->GetBindings(), xAffectedForm, sOriginalPathWithPagePrefix);
                m_pShell->GetViewShell()->GetViewFrame()->GetBindings().DLEAVEREGISTRATIONS();
                DBG_ASSERT(((sPageId += '\\') += sAccessPath) == GetAccessPathFromForm(xAffectedForm, GetPageId(xAffectedForm)),
                    "FmXFormShell::queryDispatch : hmmm ... what does this access path mean ?");

                pRequestedDispatcher->acquire();
                pRequestedDispatcher->setExecutor(LINK(this, FmXFormShell, OnExecuteNavSlot));
            }

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xNavigationForm;
            if (m_xNavigationController.is())
                xNavigationForm = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(m_xNavigationController->getModel(), ::com::sun::star::uno::UNO_QUERY);
            // xNavigationForm is the form which our navigation bar belongs to
            // activate the new dispatcher if (and only if) it belongs to the same form
            if (xAffectedForm == xNavigationForm)
            {
                pRequestedDispatcher->SetActive(sal_True);
            }
            else
            {
                pRequestedDispatcher->SetActive(sal_False);
                UpdateFormDispatcher(pRequestedDispatcher);
            }

            return (::com::sun::star::frame::XDispatch*)pRequestedDispatcher;
        }
    }

    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch>();
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::formActivated(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    DBG_ASSERT(rEvent.Source == m_xExternalViewController, "FmXFormShell::formActivated : where did this come from ?");
    setActiveController(::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>(rEvent.Source, ::com::sun::star::uno::UNO_QUERY));
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::formDeactivated(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    DBG_ASSERT(rEvent.Source == m_xExternalViewController, "FmXFormShell::formDeactivated : where did this come from ?");
}

//------------------------------------------------------------------------------
void FmXFormShell::disposing()
{
    OComponentHelper::disposing();

    // dispose our interceptor helpers
    if (m_pMainFrameInterceptor)
    {
        m_pMainFrameInterceptor->dispose();
        m_pMainFrameInterceptor->release();
        m_pMainFrameInterceptor = NULL;
    }
    if (m_pExternalViewInterceptor)
    {
        m_pExternalViewInterceptor->dispose();
        m_pExternalViewInterceptor->release();
        m_pExternalViewInterceptor = NULL;
    }

    m_xAttachedFrame = NULL;

    CloseExternalFormViewer();

    // dispose all our navigation dispatchers
    for (   FormsDispatchersIterator aFormIter = m_aNavigationDispatcher.begin();
            aFormIter != m_aNavigationDispatcher.end();
            ++aFormIter
        )
    {
        for (   SingleFormDispatchersIterator aDispIter = (*aFormIter).second.begin();
                aDispIter < (*aFormIter).second.end();
                ++aDispIter
            )
        {
            FmFormNavigationDispatcher* pCurDispatcher = *aDispIter;
            if (pCurDispatcher)
            {
                pCurDispatcher->dispose();
                pCurDispatcher->release();
                *aDispIter = NULL;
            }
        }
    }
    m_aNavigationDispatcher.clear();

    {
        ::osl::MutexGuard aGuard(m_aInvalidationSafety);
        if (m_nInvalidationEvent)
        {
            Application::RemoveUserEvent(m_nInvalidationEvent);
            m_nInvalidationEvent = 0;
        }
    }

    // we are disposed from within the destructor of our shell, so now the shell pointer is invalid ....
    m_pShell = NULL;
}

//------------------------------------------------------------------------------
void FmXFormShell::InvalidateSlot(sal_Int16 nId, sal_Bool bWithItem, sal_Bool bWithId)
{
    ::osl::MutexGuard aGuard(m_aInvalidationSafety);
    if (m_nLockSlotInvalidation)
    {
        m_arrInvalidSlots.Insert(nId, m_arrInvalidSlots.Count());
        BYTE nFlags = (bWithItem ? 0x02 :0) + (bWithId ? 0x01 : 0);
        m_arrInvalidSlots_Flags.Insert(nFlags, m_arrInvalidSlots_Flags.Count());
    }
    else
        if (nId)
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(nId, bWithItem, bWithId);
        else
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}

//------------------------------------------------------------------------------
void FmXFormShell::LockSlotInvalidation(sal_Bool bLock)
{
    ::osl::MutexGuard aGuard(m_aInvalidationSafety);
    DBG_ASSERT(bLock || m_nLockSlotInvalidation>0, "FmXFormShell::LockSlotInvalidation : invalid call !");

    if (bLock)
        ++m_nLockSlotInvalidation;
    else if (!--m_nLockSlotInvalidation)
    {
        // alles, was sich waehrend der gelockten Phase angesammelt hat, (asynchron) invalidieren
        if (!m_nInvalidationEvent)
            m_nInvalidationEvent = Application::PostUserEvent(LINK(this, FmXFormShell, OnInvalidateSlots));
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnInvalidateSlots, void*, EMPTYARG)
{
    ::osl::MutexGuard aGuard(m_aInvalidationSafety);
    m_nInvalidationEvent = 0;

    DBG_ASSERT(m_arrInvalidSlots.Count() == m_arrInvalidSlots_Flags.Count(),
        "FmXFormShell::OnInvalidateSlots : inconsistent slot arrays !");
    BYTE nFlags;
    for (sal_Int16 i=0; i<m_arrInvalidSlots.Count(); ++i)
    {
        nFlags = m_arrInvalidSlots_Flags[i];

        if (m_arrInvalidSlots[i])
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(m_arrInvalidSlots[i], (nFlags & 0x02), (nFlags & 0x01));
        else
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
    }

    m_arrInvalidSlots.Remove(0, m_arrInvalidSlots.Count());
    m_arrInvalidSlots_Flags.Remove(0, m_arrInvalidSlots_Flags.Count());
    return 0L;
}

//------------------------------------------------------------------------------
void FmXFormShell::ForceUpdateSelection(sal_Bool bAllowInvalidation)
{
    if (IsSelectionUpdatePending())
    {
        m_aMarkTimer.Stop();

        // die Invalidierung der Slots, die implizit von SetSelection besorgt wird, eventuell abschalten
        if (!bAllowInvalidation)
            LockSlotInvalidation(sal_True);

        SetSelection(m_pShell->GetFormView()->GetMarkList());

        if (!bAllowInvalidation)
            LockSlotInvalidation(sal_False);
    }
}

//------------------------------------------------------------------------------
PopupMenu* FmXFormShell::GetConversionMenu()
{
    PopupMenu* pNewMenu = new PopupMenu(SVX_RES(RID_FMSHELL_CONVERSIONMENU));

    ImageList aImageList( SVX_RES(RID_SVXIMGLIST_FMEXPL) );
    for (int i=0; i<sizeof(nConvertSlots)/sizeof(nConvertSlots[0]); ++i)
    {
        // das entsprechende Image dran
        pNewMenu->SetItemImage(nConvertSlots[i], aImageList.GetImage(nCreateSlots[i]));
    }

    return pNewMenu;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::IsControlConversionSlot(sal_uInt16 nSlotId)
{
    for (int i=0; i<sizeof(nConvertSlots)/sizeof(nConvertSlots[0]); ++i)
        if (nConvertSlots[i] == nSlotId)
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::ExecuteControlConversionSlot(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent>& xContext, sal_uInt16 nSlotId)
{
    for (int i=0; i<sizeof(nConvertSlots)/sizeof(nConvertSlots[0]); ++i)
    {
        if (nConvertSlots[i] == nSlotId)
            return ConvertControlTo(xContext, nObjectTypes[i]);
    }
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::IsConversionPossible(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xContext, sal_Int16 nConversionSlot)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo> xInfo(xContext, ::com::sun::star::uno::UNO_QUERY);
    if (!xInfo.is())
        return sal_False;

    sal_Int16 nObjectType = getControlTypeByObject(xInfo);

    if ((OBJ_FM_HIDDEN == nObjectType) || (OBJ_FM_CONTROL == nObjectType) || (OBJ_FM_GRID == nObjectType))
        return sal_False;   // hiddens koennen nicht konvertiert werden

    DBG_ASSERT(sizeof(nConvertSlots)/sizeof(nConvertSlots[0]) == sizeof(nObjectTypes)/sizeof(nObjectTypes[0]),
        "FmXFormShell::IsConversionPossible : nConvertSlots & nObjectTypes must have the same size !");

    for (sal_Int16 i=0; i<sizeof(nConvertSlots)/sizeof(nConvertSlots[0]); ++i)
        if (nConvertSlots[i] == nConversionSlot)
            return nObjectTypes[i] != nObjectType;

    return sal_True;    // alle anderen Slots nehmen wir als gueltig an
}

//------------------------------------------------------------------------------
void FmXFormShell::CheckControlConversionSlots(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xContext, Menu& rMenu)
{
    for (sal_Int16 i=0; i<rMenu.GetItemCount(); ++i)
        // der Context ist schon von einem Typ, der dem Eitnrag entspricht -> disable
        rMenu.EnableItem(rMenu.GetItemId(i), IsConversionPossible(xContext, rMenu.GetItemId(i)));
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::ConvertControlTo(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent>& xModel, sal_uInt16 nTargetObjectId)
{
    if (!xModel.is())
        // das wuerde ausser einem Performance-Verlust nix machen, aber trotzdem ...
        return sal_False;

    FmFormPage* pCurrentPage = m_pShell->GetCurPage();

    SdrUnoObj* pFound = NULL;
    for (int i=0; i<pCurrentPage->GetObjCount(); ++i)
    {
        SdrObject* pCurrent = pCurrentPage->GetObj(i);

        // wenn das Ding eine Gruppierung ist -> durchiterieren
        SdrObjListIter* pGroupIterator = NULL;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator = new SdrObjListIter(*pCurrent->GetSubList());
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }

        while (pCurrent)
        {
            if (pCurrent->IsUnoObj())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent> xCurrent(((SdrUnoObj*)pCurrent)->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
                if ((::com::sun::star::form::XFormComponent*)xCurrent.get() == (::com::sun::star::form::XFormComponent*)xModel.get())
                    break;
            }

            if (pGroupIterator)     // ich iteriere gerade durch eine Gruppe von Controls
                pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
            else
                pCurrent = NULL;
        }
        delete pGroupIterator;

        if (pCurrent)
        {
            pFound = (SdrUnoObj*)pCurrent;
            break;
        }
    }

    if (!pFound)
        return sal_False;

    ::rtl::OUString uNewName(getServiceNameByControlType(nTargetObjectId));
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> xNewModel(::comphelper::getProcessServiceFactory()->createInstance(uNewName), ::com::sun::star::uno::UNO_QUERY);
    if (!xNewModel.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> xOldModel(pFound->GetUnoControlModel());
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo> xModelInfo(xOldModel, ::com::sun::star::uno::UNO_QUERY);
    sal_Int16 nOldModelType = xModelInfo.is() ? getControlTypeByObject(xModelInfo) : OBJ_FM_CONTROL;

    // Properties uebertragen
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xOldSet(xOldModel, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xNewSet(xNewModel, ::com::sun::star::uno::UNO_QUERY);

    String sLanguage, sCountry;
    ConvertLanguageToIsoNames(Application::GetAppInternational().GetLanguage(), sLanguage, sCountry);
    ::com::sun::star::lang::Locale aNewLanguage(sLanguage, sCountry, ::rtl::OUString());
    ::dbtools::TransferFormComponentProperties(xOldSet, xNewSet, aNewLanguage);

    ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor> aOldScripts;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xOldModel, ::com::sun::star::uno::UNO_QUERY);
    if (xChild.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xParent(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);

        // remember old script events
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager> xEvManager(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
        if (xParent.is() && xEvManager.is())
        {
            sal_Int32 nIndex = getElementPos(xParent, xOldModel);
            if (nIndex>=0 && nIndex<xParent->getCount())
                aOldScripts = xEvManager->getScriptEvents(nIndex);
        }

        // replace the mdoel within the parent container
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer> xNameParent(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
        if (xNameParent.is())
        {
            // the form container works with FormComponents
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent> xComponent(xNewModel, ::com::sun::star::uno::UNO_QUERY);
            DBG_ASSERT(xComponent.is(), "FmXFormShell::ConvertControlTo : the new model is no form component !");
            ::com::sun::star::uno::Any aNewModel(::com::sun::star::uno::makeAny(xComponent));
            try
            {
                DBG_ASSERT(::comphelper::hasProperty(FM_PROP_NAME, xOldSet),
                    "FmXFormShell::ConvertControlTo : one of the models is invalid !");
                xNameParent->replaceByName(::comphelper::getString(xOldSet->getPropertyValue(FM_PROP_NAME)), aNewModel);
            }
            catch(...)
            {
                DBG_ERROR("FmXFormShell::ConvertControlTo : could not replace the model !");
                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xNewComponent(xNewModel, ::com::sun::star::uno::UNO_QUERY);
                if (xNewComponent.is())
                    xNewComponent->dispose();
                return sal_False;
            }

        }
    }

    // special handling for the LabelControl-property : can only be set when the model is placed
    // within the forms hierarchy
    if (::comphelper::hasProperty(FM_PROP_CONTROLLABEL, xOldSet) && ::comphelper::hasProperty(FM_PROP_CONTROLLABEL, xNewSet))
    {
        try
        {
            xNewSet->setPropertyValue(FM_PROP_CONTROLLABEL, xOldSet->getPropertyValue(FM_PROP_CONTROLLABEL));
        }
        catch(...)
        {
        }

    }

    // neues Model setzen
    pFound->SetChanged();
    pFound->SetUnoControlModel(xNewModel);

    // transfer script events
    // (do this _after_ SetUnoControlModel as we need the new (implicitly created) control)
    if (aOldScripts.getLength())
    {
        // das Control zum Model suchen
        SdrPageView* pPageView = m_pShell->GetFormView()->GetPageViewPvNum(0);
        DBG_ASSERT(pPageView->GetWinList().GetCount() > 0, "FmXFormShell::ConvertControlTo : no SdrPageViewWinRecs");
        const SdrPageViewWinRec& rViewWinRec = pPageView->GetWinList()[0];
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xControlContainer(rViewWinRec.GetControlContainerRef());

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> > aControls( xControlContainer->getControls() );
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* pControls = aControls.getConstArray();

        sal_uInt32 nLen = aControls.getLength();
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> xControl;
        for (sal_uInt32 i=0 ; i<nLen; ++i)
        {
            if (pControls[i]->getModel() == xNewModel)
            {
                xControl = pControls[i];
                break;
            }
        }
        TransferEventScripts(xNewModel, xControl, aOldScripts);
    }

    // create an undo action
    FmFormModel* pModel = m_pShell->GetFormModel();
    DBG_ASSERT(pModel != NULL, "FmXFormShell::ConvertControlTo : my shell has no model !");
    if (pModel)
        pModel->AddUndo(new FmUndoModelReplaceAction(*pModel, pFound, xOldModel));

    return sal_True;
}

//------------------------------------------------------------------------------
void FmXFormShell::LoopGrids(sal_Int16 nWhat)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> xControlModels(m_xActiveForm, ::com::sun::star::uno::UNO_QUERY);
    if (xControlModels.is())
        for (sal_Int16 i=0; i<xControlModels->getCount(); ++i)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xModelSet(*(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)xControlModels->getByIndex(i).getValue(), ::com::sun::star::uno::UNO_QUERY);
            if (!xModelSet.is())
                continue;

            if (!::comphelper::hasProperty(FM_PROP_CLASSID, xModelSet))
                continue;
            sal_Int16 nClassId = ::comphelper::getINT16(xModelSet->getPropertyValue(FM_PROP_CLASSID));
            if (::com::sun::star::form::FormComponentType::GRIDCONTROL != nClassId)
                continue;

            if (!::comphelper::hasProperty(FM_PROP_CURSORCOLOR, xModelSet) || !::comphelper::hasProperty(FM_PROP_ALWAYSSHOWCURSOR, xModelSet) || !::comphelper::hasProperty(FM_PROP_DISPLAYSYNCHRON, xModelSet))
                continue;

            switch (nWhat & GA_SYNC_MASK)
            {
                case GA_DISABLE_SYNC:
                    {
                        sal_Bool bB(sal_False);
                        xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
                    }
                    break;
                case GA_FORCE_SYNC:
                {
                    ::com::sun::star::uno::Any aOldVal( xModelSet->getPropertyValue(FM_PROP_DISPLAYSYNCHRON) );
                    sal_Bool bB(sal_True);
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, aOldVal);
                }
                break;
                case GA_ENABLE_SYNC:
                    {
                        sal_Bool bB(sal_True);
                        xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
                    }
                    break;
            }

            if (nWhat & GA_DISABLE_ROCTRLR)
            {
                sal_Bool bB(sal_False);
                xModelSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState> xModelPropState(xModelSet, ::com::sun::star::uno::UNO_QUERY);
                if (xModelPropState.is())
                    xModelPropState->setPropertyToDefault(FM_PROP_CURSORCOLOR);
                else
                    xModelSet->setPropertyValue(FM_PROP_CURSORCOLOR, ::com::sun::star::uno::Any());     // this should be the default
            }
            else if (nWhat & GA_ENABLE_ROCTRLR)
            {
                sal_Bool bB(sal_True);
                xModelSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
                xModelSet->setPropertyValue(FM_PROP_CURSORCOLOR, ::com::sun::star::uno::makeAny(sal_Int32(COL_LIGHTRED)));
            }
        }

/*  // alle Controls der Page durchiterieren und nach GridControls suchen ...
    SdrPageView* pCurPageView = m_pShell->GetFormView()->GetPageViewPvNum(0);
    // deren ViewWinRec-Liste, daraus das erste Element
    DBG_ASSERT(pCurPageView->GetWinList().GetCount() > 0, "FmXFormShell::LoopGrids : unexpected : no SdrPageViewWinRecs");
    const SdrPageViewWinRec& rViewWinRec = pCurPageView->GetWinList()[0];
    // von dem bekomme ich alle Controls ...
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xControlContainer( rViewWinRec.GetControlContainerRef());
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> > seqControls = xControlContainer->getControls();
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* pControls = seqControls.getConstArray();
    // ... die ich dann durchsuchen kann
    for (int i=0; i<seqControls.getLength(); ++i)
    {
        ::com::sun::star::uno::Reference<XGridPeer> xGridPeer(pControls[i]->getPeer(), ::com::sun::star::uno::UNO_QUERY);
        if (!xGridPeer.is())
            continue;

        FmXGridPeer* pPeer = (FmXGridPeer*)xGridPeer->getImplementation(FmXGridPeer_getReflection());
        if (!pPeer)
            continue;

        FmGridControl* pGrid = (FmGridControl*)pPeer->GetWindow();
        // what to do ?
        // display synchronisation ?
        switch (nWhat & GA_SYNC_MASK)
        {
            case GA_DISABLE_SYNC:
                pGrid->setDisplaySynchron(sal_False); break;
            case GA_FORCE_SYNC:
                pGrid->forceSyncDisplay(); break;
            case GA_ENABLE_SYNC:
                pGrid->setDisplaySynchron(sal_True); break;
        }

        if (nWhat & GA_DISABLE_ROCTRLR)
            pGrid->forceROController(sal_False);
        else if (nWhat & GA_ENABLE_ROCTRLR)
            pGrid->forceROController(sal_True);
    }
*/
}

//------------------------------------------------------------------------------
void FmXFormShell::ExecuteSearch()
{
    UniString sTestContexts;
    //  m_arrSearchContexts.Remove(0, m_arrSearchContexts.Count());
    m_arrSearchContexts.clear();

    // eine Sammlung aller (logischen) Formulare
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xAllForms(m_pShell->GetCurPage()->GetForms(),::com::sun::star::uno::UNO_QUERY);
    //  ::fml::convertIFace((XInterface*),xAllForms);
    CollectFormContexts(xAllForms, String(), sTestContexts);
    sTestContexts.EraseLeadingChars(';');

    // hier sind jetzt dummerweise noch alle Forms drin, die ueberhaupt keine gueltigen Controls enthalten
    // um das auszufiltern, benutze ich einfach meinen OnSearchContextRequest-Handler, der genau die gueltigen Controls einer Form sammelt
    FmFormArray::reverse_iterator aIter = m_arrSearchContexts.rbegin();
    sal_Int32 i = m_arrSearchContexts.size();
    for (; aIter != m_arrSearchContexts.rend(); ++aIter,i--)
    {
        FmSearchContext fmscTest;
        fmscTest.nContext = i-1;
        sal_uInt32 lValidControls = LINK(this, FmXFormShell, OnSearchContextRequest).Call(&fmscTest);
        if (lValidControls == 0)
        {
            m_arrSearchContexts.erase(aIter.base()-1);
            sTestContexts.SetToken(i-1, ';', String());
        }
    }

    // jetzt enthaelt die Context-Liste noch ein paar Leer-Token
    UniString strRealContexts,strCurrentToken;
    for (i=0; i<sTestContexts.GetTokenCount(';'); ++i)
    {
        strCurrentToken = sTestContexts.GetToken(i);
        if (strCurrentToken.Len() != 0)
        {
            strRealContexts += ';';
            strRealContexts += strCurrentToken;
        }
    }
    strRealContexts.EraseLeadingChars(';');

    if (m_arrSearchContexts.size() == 0)
    {   // es gibt keine Controls, die alle Bedingungen fuer eine Suche erfuellen
        ErrorBox(NULL, WB_OK, SVX_RESSTR(RID_STR_NODATACONTROLS)).Execute();
        return;
    }

    // jetzt brauche ich noch einen 'initial context'
    sal_Int16 nInitialContext = 0;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xActiveForm( getActiveForm());
    for (i=0; i<m_arrSearchContexts.size(); ++i)
    {
        if (m_arrSearchContexts.at(i) == xActiveForm)
        {
            nInitialContext = i;
            break;
        }
    }

    // wenn der Dialog initial den Text des aktiven Controls anbieten soll, muss dieses ein XTextComponent-Interface habe,
    // ausserdem macht das nur Sinn, wenn das aktuelle Feld auch an ein Tabellen- (oder was-auch-immer-)Feld gebunden ist
    UniString strActiveField;
    UniString strInitialText;
    // ... das bekomme ich von meinem FormController
    DBG_ASSERT(m_xActiveController.is(), "FmXFormShell::ExecuteSearch : no active controller !");
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> xActiveControl( m_xActiveController->getCurrentControl());
    if (xActiveControl.is())
    {
        // das Control kann mir sein Model sagen ...
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> xActiveModel( xActiveControl->getModel());
        DBG_ASSERT(xActiveModel.is(), "FmXFormShell::ExecuteSearch : active control has no model !");

        // das Model frage ich nach der ControlSource-Eigenschaft ...
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xProperties(xActiveControl->getModel(), ::com::sun::star::uno::UNO_QUERY);
        if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xProperties) && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xProperties))
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xField( *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)(xProperties->getPropertyValue(FM_PROP_BOUNDFIELD).getValue()));
            if (xField.is())    // (nur wenn das Ding wirklich gebunden ist)
            {
                // und das Control selber nach einem TextComponent-Interface (damit ich mir dort den Text abholen kann)
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent> xText(xActiveControl, ::com::sun::star::uno::UNO_QUERY);
                if (xText.is())
                {
                    strActiveField = getLabelName(xProperties).getStr();
                    strInitialText = xText->getText().getStr();
                }
            }
        }
        else
        {
            // das Control selber hat keine ControlSource, aber vielleicht ist es ein GridControl
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid> xGrid(xActiveControl, ::com::sun::star::uno::UNO_QUERY);
            if (xGrid.is())
            {
                // fuer strActiveField brauche ich die die ControlSource der Column, dafuer den Columns-Container, dafuer die
                // GridPeer
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridPeer> xGridPeer(xActiveControl->getPeer(), ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xColumns;
                if (xGridPeer.is())
                    xColumns = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xGridPeer->getColumns(),::com::sun::star::uno::UNO_QUERY);

                sal_Int16 nViewCol = xGrid->getCurrentColumnPosition();
                sal_Int16 nModelCol = GridView2ModelPos(xColumns, nViewCol);
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCurrentCol( xColumns.is() ? *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)xColumns->getByIndex(nModelCol).getValue() : ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(NULL));
                if (xCurrentCol.is())
                    strActiveField = ::comphelper::getString(xCurrentCol->getPropertyValue(FM_PROP_LABEL)).getStr();

                // the text fo the current column
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xColControls(xGridPeer, ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xCurControl(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xColControls->getByIndex(nViewCol).getValue(), ::com::sun::star::uno::UNO_QUERY);
                ::rtl::OUString sInitialText;
                if (IsSearchableControl(xCurControl, &sInitialText))
                    strInitialText = sInitialText.getStr();
            }
        }
    }

    // um eventuelle GridControls, die ich kenne, kuemmern
    LoopGrids(GA_DISABLE_SYNC /*| GA_ENABLE_ROCTRLR*/);

    // jetzt bin ich reif fuer den Dialog
    FmSearchDialog dlg(NULL, strInitialText, strRealContexts, nInitialContext, LINK(this, FmXFormShell, OnSearchContextRequest),
        FmSearchDialog::SM_ALLOWSCHEDULE);
    // wenn die potentiellen Deadlocks, die durch die Benutzung des Solar-Mutex in MTs VCLX...-Klasen entstehen, irgendwann mal
    // ausgeraeumt sind, sollte hier ein SM_USETHREAD rein, denn die Suche in einem eigenen Thread ist doch etwas fluessiger
    // sollte allerdings irgendwie von dem unterliegenden Cursor abhaengig gemacht werden, DAO zum Beispiel ist nicht thread-sicher

    dlg.SetActiveField(strActiveField);

    dlg.SetFoundHandler(LINK(this, FmXFormShell, OnFoundData));
    dlg.SetCanceledNotFoundHdl(LINK(this, FmXFormShell, OnCanceledNotFound));
    dlg.Execute();

    // GridControls wieder restaurieren
    LoopGrids(GA_ENABLE_SYNC | GA_DISABLE_ROCTRLR);

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetPageViewPvNum(0));
        // da ich in OnFoundData (fals ich dort war) Controls markiert habe
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::GetY2KState(sal_uInt16& n)
{
    if (m_pShell->IsDesignMode())
        // im Design-Modus (ohne aktive Controls) soll sich das Haupt-Dokument darum kuemmern
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm( getActiveForm());
    if (!xForm.is())
        // kein aktuelles Formular (also insbesondere kein aktuelles Control) -> das Haupt-Dokument soll sich kuemmern
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xDB(xForm, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xDB.is(), "FmXFormShell::GetY2KState : current form has no dbform-interface !");

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> xSupplier( ::dbtools::getNumberFormats(::dbtools::getConnection(xDB), sal_False));
    if (xSupplier.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xSupplier->getNumberFormatSettings());
        if (xSet.is())
        {
            try
            {
                ::com::sun::star::uno::Any aVal( xSet->getPropertyValue(::rtl::OUString::createFromAscii("TwoDigitDateStart")) );
                aVal >>= n;
                return sal_True;
            }
            catch(...)
            {
            }

        }
    }
    return sal_False;
}

//------------------------------------------------------------------------------
void FmXFormShell::SetY2KState(sal_uInt16 n)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xActiveForm( getActiveForm());
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xDB(xActiveForm, ::com::sun::star::uno::UNO_QUERY);
    if (xDB.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> xSupplier( ::dbtools::getNumberFormats(::dbtools::getConnection(xDB), sal_False));
        if (xSupplier.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    ::com::sun::star::uno::Any aVal;
                    aVal <<= n;
                    xSet->setPropertyValue(::rtl::OUString::createFromAscii("TwoDigitDateStart"), aVal);
                }
                catch(...)
                {
                }

            }
            return;
        }
    }

    // kein aktives Formular gefunden -> alle aktuell vorhandenen Formulare durchiterieren
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xCurrentForms( m_xForms);
    if (!xCurrentForms.is())
    {   // im alive-Modus sind meine Forms nicht gesetzt, wohl aber die an der Page
        if (m_pShell->GetCurPage())
            xCurrentForms = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(m_pShell->GetCurPage()->GetForms(), ::com::sun::star::uno::UNO_QUERY);
    }
    if (!xCurrentForms.is())
        return;

    ::comphelper::IndexAccessIterator aIter(xCurrentForms);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xCurrentElement( aIter.Next());
    while (xCurrentElement.is())
    {
        // ist das aktuelle Element eine DatabaseForm ?
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xDB(xCurrentElement, ::com::sun::star::uno::UNO_QUERY);
        if (xDB.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier> xSupplier( ::dbtools::getNumberFormats(::dbtools::getConnection(xDB), sal_False));
            if (!xSupplier.is())
                continue;

            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    ::com::sun::star::uno::Any aVal;
                    aVal <<= n;
                    xSet->setPropertyValue(::rtl::OUString::createFromAscii("TwoDigitDateStart"), aVal);
                }
                catch(...)
                {
                }

            }
        }
        xCurrentElement = aIter.Next();
    }
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::CanMoveLeft(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    if (!xController.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xController->getModel(), ::com::sun::star::uno::UNO_QUERY);
    return CanMoveLeft(xSet);
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::CanMoveLeft(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xControllerModel)
{
    if (!_xControllerModel.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xCursor(_xControllerModel, ::com::sun::star::uno::UNO_QUERY);
    sal_Bool bIsNew = ::comphelper::getBOOL(_xControllerModel->getPropertyValue(FM_PROP_ISNEW));
    sal_Int32 nCount    = ::comphelper::getINT32(_xControllerModel->getPropertyValue(FM_PROP_ROWCOUNT));
    return nCount && (!xCursor->isFirst() || bIsNew);
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::CanMoveRight(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    if (!xController.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xController->getModel(), ::com::sun::star::uno::UNO_QUERY);
    return CanMoveRight(xSet);
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::CanMoveRight(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xControllerModel)
{
    if (!_xControllerModel.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xCursor(_xControllerModel, ::com::sun::star::uno::UNO_QUERY);
    sal_Int32 nCount        = ::comphelper::getINT32(_xControllerModel->getPropertyValue(FM_PROP_ROWCOUNT));
    sal_Bool  bIsModified   = ::comphelper::getBOOL(_xControllerModel->getPropertyValue(FM_PROP_ISMODIFIED));
    sal_Bool  bIsNew        = ::comphelper::getBOOL(_xControllerModel->getPropertyValue(FM_PROP_ISNEW));
    sal_Bool  bCanInsert    = ::dbtools::canInsert(_xControllerModel);

    return  (
                (   nCount
                &&  !xCursor->isLast()
                &&  !bIsNew
                )
            )
            ||
            (
                bCanInsert && (!bIsNew || bIsModified)
            );
}

//------------------------------------------------------------------------
sal_Bool FmXFormShell::CommitCurrent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& _xController)
{
    // muﬂ noch ein Control commitet werden
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> xActiveControl(_xController->getCurrentControl());
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundControl> xLockingTest(xActiveControl, ::com::sun::star::uno::UNO_QUERY);
    sal_Bool bControlIsLocked = xLockingTest.is() && xLockingTest->getLock();
    sal_Bool bResult = sal_True;
    if (xActiveControl.is() && !bControlIsLocked)
    {
        // zunaechst das Control fragen ob es das IFace unterstuetzt
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent> xBound(xActiveControl, ::com::sun::star::uno::UNO_QUERY);
        if (!xBound.is())
            xBound  = ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundComponent>(xActiveControl->getModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xBound.is() && !xBound->commit())
            bResult = sal_False;
    }
    return bResult;
}

//------------------------------------------------------------------------
void FmXFormShell::ResetCurrent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& _xController)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> xActiveControl( _xController->getCurrentControl());
    if (xActiveControl.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XReset> xReset(xActiveControl->getModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xReset.is())
            xReset->reset();
    }
}

//------------------------------------------------------------------------
void FmXFormShell::ResetAll(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& _xForm)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xContainer(_xForm, ::com::sun::star::uno::UNO_QUERY);
    for (sal_Int16 nBothViews=0; nBothViews<2; ++nBothViews)
    {
        if (xContainer.is())
        {
            for (sal_Int32 i=0; i<xContainer->getCount(); ++i)
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XReset> xReset(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xContainer->getByIndex(i).getValue(), ::com::sun::star::uno::UNO_QUERY);
                if (xReset.is())
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xAsForm(xReset, ::com::sun::star::uno::UNO_QUERY);
                    if (!xAsForm.is())  // no resets on the form
                        xReset->reset();
                }
            }
        }

        xContainer = NULL;
        if ((nBothViews == 0) && (getInternalForm(_xForm) == m_xExternalDisplayedForm))
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xExternalFormController(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY);
            if (xExternalFormController.is())
                xContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(xExternalFormController->getModel(), ::com::sun::star::uno::UNO_QUERY);
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::SaveModified(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate>& _xCursor, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xSet, sal_Bool& _rRecordInserted)
{
    _rRecordInserted = sal_False;
    if (!_xCursor.is())
        return sal_False;

    _xSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(_xCursor, ::com::sun::star::uno::UNO_QUERY);
    if (!_xSet.is())
        return sal_False;

    // muﬂ gespeichert werden ?
    sal_Bool  bIsNew        = ::comphelper::getBOOL(_xSet->getPropertyValue(FM_PROP_ISNEW));
    sal_Bool  bIsModified   = ::comphelper::getBOOL(_xSet->getPropertyValue(FM_PROP_ISMODIFIED));
    sal_Bool bResult = !bIsModified;
    if (bIsModified)
    {
        try
        {
            if (bIsNew)
                _xCursor->insertRow();
            else
                _xCursor->updateRow();
            bResult = sal_True;
        }
        catch(...)
        {
        }

        _rRecordInserted = bIsNew && bResult;
    }
    return bResult;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::SaveModified(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController,
                                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate>& xCursor,
                                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xSet,
                                sal_Bool& rRecordInserted)

{
    rRecordInserted = sal_False;

    if (!xController.is())
        return sal_False;

    xCursor = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate>(xController->getModel(), ::com::sun::star::uno::UNO_QUERY);
    return SaveModified(xCursor, xSet, rRecordInserted);
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::IsModified(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    if (!xController.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xController->getModel(), ::com::sun::star::uno::UNO_QUERY);
    if (!xSet.is())
        return sal_False;

    // Modifiziert
    if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED)))
        return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::SaveModified(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController, sal_Bool bCommit)
{
    if (!bCommit || CommitCurrent(xController))
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate> xCursor;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet;
        sal_Bool bInserted;
        return SaveModified(xController,
                            xCursor,
                            xSet,
                            bInserted);
    }
    else
        return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::MoveRight(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate>& _xCursor)
{
    if (!_xCursor.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xReadCursor(_xCursor, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xReadCursor.is(), "FmXFormShell::MoveRight : invalid cursor !");
    sal_Bool bInserted;

    sal_Bool bSuccess = SaveModified(_xCursor, xSet, bInserted);
    if (bSuccess)
    {
        try
        {
            if (bInserted)
            {
                // go to insert row
                _xCursor->moveToInsertRow();
            }
            else
            {
                if (xReadCursor->isLast())
                    _xCursor->moveToInsertRow();
                else
                    xReadCursor->next();
            }
        }
        catch(...)
        {
        }

    }
    return bSuccess;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::MoveRight(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    if (!xController.is())
        return sal_False;

    return MoveRight(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate>(xController->getModel(), ::com::sun::star::uno::UNO_QUERY));
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::MoveLeft(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate>& _xCursor)
{
    if (!_xCursor.is())
        return sal_False;

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xReadCursor(_xCursor, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xReadCursor.is(), "FmXFormShell::MoveLeft : invalid cursor !");
    sal_Bool bInserted;

    sal_Bool bSuccess = SaveModified(_xCursor, xSet, bInserted);

    if (bSuccess)
    {
        try
        {
            if (bInserted)
            {
                // retrieve the bookmark of the new record and move previous to that bookmark
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate> xLocate(xReadCursor, ::com::sun::star::uno::UNO_QUERY);
                xLocate->moveRelativeToBookmark(xLocate->getBookmark(), -1);
            }
            else if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW)))
            {
                // we assume that the inserted record is now the last record in the
                // result set
                xReadCursor->last();
            }
            else
                xReadCursor->previous();
        }
        catch(...)
        {
        }

    }
    return bSuccess;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::MoveLeft(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    if (!xController.is())
        return sal_False;

    return MoveLeft(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate>(xController->getModel(), ::com::sun::star::uno::UNO_QUERY));
}

// ::com::sun::star::sdbc::XRowSetListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::cursorMoved(const ::com::sun::star::lang::EventObject& event) throw( ::com::sun::star::uno::RuntimeException )
{
    m_bActiveModified = sal_False;
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(DatabaseSlotMap);
}

//------------------------------------------------------------------------------
void FmXFormShell::CloseExternalFormViewer()
{
    if (!m_xExternalViewController.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame> xExternalViewFrame( m_xExternalViewController->getFrame());
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider> xCommLink(xExternalViewFrame, ::com::sun::star::uno::UNO_QUERY);
    if (!xCommLink.is())
        return;

    ::com::sun::star::util::URL aCloseUrl;
        // tool windows (like the task local beamer used for the grid) are assumed to close when dispatching an empty ::com::sun::star::util::URL

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> xCloser( xCommLink->queryDispatch(aCloseUrl, ::rtl::OUString::createFromAscii(""), ::com::sun::star::frame::FrameSearchFlag::SELF | ::com::sun::star::frame::FrameSearchFlag::CHILDREN));
    DBG_ASSERT(xCloser.is(), "FmXFormShell::CloseExternalFormViewer : don't know how to close the tool frame !");
    if (xCloser.is())
        xCloser->dispatch(aCloseUrl, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>());

    m_xExternalViewController = NULL;
    m_xExtViewTriggerController = NULL;
    m_xExternalDisplayedForm = NULL;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> FmXFormShell::getInternalForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm) const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xExternalCtrlr(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY);
    if (xExternalCtrlr.is() && (_xForm == xExternalCtrlr->getModel()))
    {
        DBG_ASSERT(m_xExternalDisplayedForm.is(), "FmXFormShell::getInternalForm : invalid external form !");
        return m_xExternalDisplayedForm;
    }
    return _xForm;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> FmXFormShell::getInternalForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& _xForm) const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xExternalCtrlr(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY);
    if (xExternalCtrlr.is() && (_xForm == xExternalCtrlr->getModel()))
    {
        DBG_ASSERT(m_xExternalDisplayedForm.is(), "FmXFormShell::getInternalForm : invalid external form !");
        return ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>(m_xExternalDisplayedForm, ::com::sun::star::uno::UNO_QUERY);
    }
    return _xForm;
}

//------------------------------------------------------------------------------
void FmXFormShell::ActivateDispatchers(const UniString& _sNavFormAccess, sal_Bool _bActivate)
{
    for (   ConstFormsDispatchersIterator aFormIter = m_aNavigationDispatcher.begin();
            aFormIter != m_aNavigationDispatcher.end();
            ++aFormIter
        )
    {
        UniString sPath = aFormIter->first;

        if (sPath == _sNavFormAccess)
        {
            for (   ConstSingleFormDispatchersIterator aDispIter = (*aFormIter).second.begin();
                    aDispIter < (*aFormIter).second.end();
                    ++aDispIter
                )
            {
                FmFormNavigationDispatcher* pCurDispatcher = *aDispIter;
                if (pCurDispatcher)
                    pCurDispatcher->SetActive(_bActivate);
            }
        }
    }
}

//------------------------------------------------------------------------------
UniString FmXFormShell::GetAccessPathFromForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm, const UniString& rAssumedPagePrefix) const
{
    UniString sReturn;
    if (!_xForm.is())
        return sReturn;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xForm(getInternalForm(_xForm));

    sReturn = getFormComponentAccessPath(xForm);

    // prepend the page ident to the path
    UniString sPrefix( rAssumedPagePrefix );
    if (!sPrefix.Len())
    {
        FmFormPage* pPage = m_pShell->GetCurPage();
        DBG_ASSERT(pPage, "FmXFormShell::GetAccessPathFromForm : have no current page !");

        if (pPage && pPage->GetImpl())
            sPrefix = pPage->GetImpl()->GetPageId();
        else
            sPrefix.AssignAscii("no page");
    }

    sPrefix += '\\';
    sPrefix += sReturn;
    sReturn = sPrefix;

    return sReturn;
}

//------------------------------------------------------------------------------
UniString FmXFormShell::GetPageId(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm) const
{
    UniString sEmptyReturn;
    FmFormModel* pModel = m_pShell->GetFormModel();
    if (!pModel)
    {
        DBG_ERROR("FmXFormShell::GetPageId : have no form model !");
        return sEmptyReturn;
    }

    // search the top level forms collection
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xTopLevelContainer( getInternalForm(_xForm));
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xAsResultSet(xTopLevelContainer, ::com::sun::star::uno::UNO_QUERY);
    while (xAsResultSet.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(xTopLevelContainer, ::com::sun::star::uno::UNO_QUERY);
        xTopLevelContainer = xChild->getParent();
        xAsResultSet = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(xTopLevelContainer, ::com::sun::star::uno::UNO_QUERY);
    }

    // search all pages of my model
    for (sal_Int16 i=0; i<pModel->GetPageCount(); ++i)
    {
        FmFormPage* pCurrent = PTR_CAST(FmFormPage, pModel->GetPage(i));
        if (!pCurrent)
            continue;
        if (pCurrent->GetForms() == xTopLevelContainer)
            return pCurrent->GetImpl()->GetPageId();
    }

    return sEmptyReturn;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer> FmXFormShell::GetPageForms(const UniString sPageId) const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer> aEmptyReturn;
    FmFormModel* pModel = m_pShell->GetFormModel();
    if (!pModel)
    {
        DBG_ERROR("FmXFormShell::GetPageForms : have no form model !");
        return aEmptyReturn;
    }

    for (sal_uInt16 i=0; i<pModel->GetPageCount(); ++i)
    {
        FmFormPage* pCurrent = PTR_CAST(FmFormPage, pModel->GetPage(i));
        if (!pCurrent)
            continue;
        if (pCurrent->GetImpl()->GetPageId() == sPageId)
            return pCurrent->GetForms();
    }

    return aEmptyReturn;
}

//------------------------------------------------------------------------------
void FmXFormShell::UpdateAllFormDispatchers(const UniString& _rPath)
{
    for (   FormsDispatchersIterator aDispIter = m_aNavigationDispatcher.begin();
            aDispIter != m_aNavigationDispatcher.end();
            ++aDispIter
        )
    {
        UniString sThisRoundPath = aDispIter->first;
        if (PathsInterfere(sThisRoundPath, _rPath))
            // child or anchestor
        {
            SingleFormDispatchers& rDisp = (*aDispIter).second;
            for (   SingleFormDispatchersIterator aIter = rDisp.begin();
                    aIter != rDisp.end();
                    ++aIter
                )
            {
                FmFormNavigationDispatcher* pCurrent = *aIter;
                if (pCurrent)
                    UpdateFormDispatcher(pCurrent);
            }
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::UpdateFormDispatcher(FmFormNavigationDispatcher* _pDisp)
{
    sal_Bool bEnable = sal_False;
    if (!_pDisp)
        return;
    // all navigation slots are disabled if the form is blocked because of a pending move
    if (!HasPendingCursorAction(_pDisp->getForm()))
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(_pDisp->getForm(), ::com::sun::star::uno::UNO_QUERY);
        switch (_pDisp->getSlot())
        {
            case SID_FM_RECORD_UNDO:
                bEnable = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED));
                break;
            case SID_FM_RECORD_PREV:
            case SID_FM_RECORD_FIRST:
                bEnable = CanMoveLeft(xSet);
                break;
            case SID_FM_RECORD_NEXT:
                bEnable = CanMoveRight(xSet);
                break;
            case SID_FM_RECORD_LAST:
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xCursor(xSet, ::com::sun::star::uno::UNO_QUERY);
                sal_Int32 nCount = ::comphelper::getINT32(xSet->getPropertyValue(FM_PROP_ROWCOUNT));
                bEnable = nCount && (!xCursor->isLast() || ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW)));
            }
                break;
            case SID_FM_RECORD_NEW:
            {
                if (::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW)))
                    bEnable = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED));
                else
                    bEnable = ::dbtools::canInsert(xSet);
            }
            break;
        }
    }
    _pDisp->SetStatus(bEnable ? SFX_ITEM_AVAILABLE : SFX_ITEM_DISABLED, NULL);
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnExecuteNavSlot, FmFormNavigationDispatcher*, pDispatcher)
{
    DBG_ASSERT(pDispatcher, "FmXFormShell::OnExecuteNavSlot : invalid argument !");

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xCursor(pDispatcher->getForm());
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate> xUpdateCursor(xCursor, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCursorSet;
    sal_Bool bDoneSomething = sal_False;

    switch (pDispatcher->getSlot())
    {
        case SID_FM_RECORD_UNDO:
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(xCursor, ::com::sun::star::uno::UNO_QUERY);
            sal_Bool bInserting = xSet.is() && ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));
            if (!bInserting)
                xUpdateCursor->cancelRowUpdates();

            ResetAll(getActiveForm());

            if (bInserting)                 // wieder in den EinfuegeModus
                DO_SAFE( xUpdateCursor->moveToInsertRow(); );

            if (xCursor == getActiveForm())
            {
                m_bActiveModified = sal_False;
                m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(DatabaseSlotMap);
            }
        }
        break;
        case SID_FM_RECORD_FIRST:
            if (SaveModified(xUpdateCursor, xCursorSet, bDoneSomething))
            {
                DO_SAFE( xCursor->first(); );
            }
            break;
        case SID_FM_RECORD_PREV:
            MoveLeft(xUpdateCursor);
            break;
        case SID_FM_RECORD_NEXT:
            MoveRight(xUpdateCursor);
            break;
        case SID_FM_RECORD_LAST:
            if (SaveModified(xUpdateCursor, xCursorSet, bDoneSomething))
            {
                // run in an own thread if ...
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCursorProps(xCursor, ::com::sun::star::uno::UNO_QUERY);
                // ... the data source is thread safe ...
                sal_Bool bAllowOwnThread = ::comphelper::hasProperty(FM_PROP_THREADSAFE, xCursorProps) && ::comphelper::getBOOL(xCursorProps->getPropertyValue(FM_PROP_THREADSAFE));
                // ... the record count is unknown
                sal_Bool bNeedOwnThread = ::comphelper::hasProperty(FM_PROP_ROWCOUNTFINAL, xCursorProps) && !::comphelper::getBOOL(xCursorProps->getPropertyValue(FM_PROP_ROWCOUNTFINAL));


                if (bNeedOwnThread && bAllowOwnThread)
                    DoAsyncCursorAction(pDispatcher->getForm(), FmXFormShell::CA_MOVE_TO_LAST);
                else
                    DO_SAFE( xCursor->last(); );
            }
            break;
        case SID_FM_RECORD_NEW:
            if (SaveModified(xUpdateCursor, xCursorSet, bDoneSomething))
            {
                DO_SAFE( xUpdateCursor->moveToInsertRow(); );
            }
            break;
    }
    UpdateAllFormDispatchers(pDispatcher->getFormAccessPath());
    return 0L;
}

//------------------------------------------------------------------------------
void FmXFormShell::setActiveController(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    if (m_bChangingDesignMode)
        return;
    DBG_ASSERT(!m_pShell->IsDesignMode(), "nur im alive mode verwenden");

    // Ist die Routine ein zweites Mal gerufen worden,
    // dann sollte der Focus nicht mehr umgesetzt werden
    if (m_bInActivate)
    {
        m_bSetFocus = xController != m_xActiveController;
        return;
    }

    if (xController != m_xActiveController)
    {
        // if there is a async cursor action running we have to restore the locking state of the controls of the old controller
        if (HasPendingCursorAction(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(m_xActiveForm, ::com::sun::star::uno::UNO_QUERY)))
            restoreControlLocks();

        ::osl::ClearableMutexGuard aGuard(m_aAsyncSafety);
        // switch all nav dispatchers belonging to the form of the current nav controller to 'non active'
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xNavigationForm;
        if (m_xNavigationController.is())
            xNavigationForm = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(m_xNavigationController->getModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xNavigationForm.is())
            ActivateDispatchers(GetAccessPathFromForm(xNavigationForm), sal_False);
        aGuard.clear();

        m_bInActivate = sal_True;

        // check if the 2 controllers serve different forms
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xOldForm;
        if (m_xActiveController.is())
            xOldForm = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(m_xActiveController->getModel(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xNewForm;
        if (xController.is())
            xNewForm = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(xController->getModel(), ::com::sun::star::uno::UNO_QUERY);
        xOldForm = getInternalForm(xOldForm);
        xNewForm = getInternalForm(xNewForm);
        sal_Bool bNeedSave = (xOldForm != xNewForm);

        if (m_xActiveController.is() && bNeedSave)
        {
            // beim Wechsel des Controllers den Inhalt speichern, ein Commit
            // wurde bereits ausgefuehrt
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> xCurrentControl(m_xActiveController->getCurrentControl());
            if (xCurrentControl.is())
            {
                m_bSetFocus = sal_True;
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>    xSet(getActiveForm(), ::com::sun::star::uno::UNO_QUERY);
                if (IsModified(m_xActiveController))
                {
                    sal_Bool bIsNew = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));
                    sal_Bool bResult = sal_False;
                    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate> xCursor(getActiveForm(), ::com::sun::star::uno::UNO_QUERY);
                    try
                    {
                        if (bIsNew)
                            xCursor->insertRow();
                        else
                            xCursor->updateRow();
                        bResult = sal_True;
                    }
                    catch(...)
                    {
                    }


                    // Konnte nicht gespeichert werden, dann den Focus wieder zurueck setzen
                    if (!bResult && m_bSetFocus)
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow> xWindow(xCurrentControl, ::com::sun::star::uno::UNO_QUERY);
                        xWindow->setFocus();
                        m_bInActivate = sal_False;
                        return;
                    }

                    if (bResult && bIsNew)
                    {
                        DO_SAFE( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(xCursor, ::com::sun::star::uno::UNO_QUERY)->last(); );
                    }
                }
            }
        }

        stopListening();

        m_xActiveController = xController;
        if (m_xActiveController.is())
        {
            m_xActiveForm = getInternalForm(::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>(m_xActiveController->getModel(), ::com::sun::star::uno::UNO_QUERY));
        }
        else
            m_xActiveForm = NULL;

        startListening();

        // activate all dispatchers belonging to form of the new navigation controller
        xNavigationForm = NULL;
        if (m_xNavigationController.is())
            xNavigationForm = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(m_xNavigationController->getModel(), ::com::sun::star::uno::UNO_QUERY);
        if (xNavigationForm.is())
            ActivateDispatchers(GetAccessPathFromForm(xNavigationForm), sal_True);

        // if there is a async cursor action running we have to lock the controls of the new controller
        if (HasPendingCursorAction(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(m_xActiveForm, ::com::sun::star::uno::UNO_QUERY)))
            setControlLocks();

        m_bInActivate = sal_False;

        m_pShell->UIFeatureChanged();
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);

        InvalidateSlot(SID_FM_FILTER_NAVIGATOR_CONTROL, sal_True, sal_True);
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::setCurControl( const SdrMarkList& rMarkList )
{
    if (isControlList(rMarkList))
    {
        // Aktuell selektiertes Objekt setzen
        sal_uInt32 nMarkCount = rMarkList.GetMarkCount();
        FmFormObj* pSelFormObj = NULL;
        if (nMarkCount==1)
        {
            SdrObject *pObj=rMarkList.GetMark(0)->GetObj();

            if( pObj->IsGroupObject() ) // Mehrfachselektion
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState> xMultiIFace(new FmXMultiSet( rMarkList ));
                setCurControl( xMultiIFace );
            }
            else
            {
                pSelFormObj = PTR_CAST(FmFormObj, rMarkList.GetMark(0)->GetObj());

                if ( pSelFormObj ) // Einfachselektion
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent> xFormComponent(pSelFormObj->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
                    if (xFormComponent.is())
                        setCurControl(xFormComponent);
                    else
                        setCurControl( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>() );
                }
                else
                    setCurControl( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>() );
            }
        }
        else if (nMarkCount > 1)            // Mehrfachselektion
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState> xMultiIFace(new FmXMultiSet( rMarkList ));
            setCurControl( xMultiIFace );
        }
    }
    else // Keine Selektion oder gemischt
        setCurControl( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>() );
}

//------------------------------------------------------------------------------
void FmXFormShell::setCurControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xNewControl)
{
    DBG_ASSERT(m_pShell->IsDesignMode(), "nur im designmode verwenden");

    if (m_xCurControl == xNewControl)
        return;

    // Stammen beide Objecte vom gleichen Container
    if (m_xCurControl.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xCur(m_xCurControl, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xNew(xNewControl, ::com::sun::star::uno::UNO_QUERY);
        // wenn nichts selektiert oder unterschiedliche V‰ter und der Vater von xCurControl ist ein XSelectionSupplier
        // dann muﬂ dieser deselektieren.
        if (!xNew.is() || xCur->getParent() != xNew->getParent())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier> xSel(xCur->getParent(), ::com::sun::star::uno::UNO_QUERY);
            if (xSel.is())
            {
                xSel->select(::com::sun::star::uno::Any());
            }
        }
    }

    m_xCurControl = xNewControl;

    InvalidateSlot(SID_FM_CTL_PROPERTIES);
}

//------------------------------------------------------------------------------
void FmXFormShell::setSelObject(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xNewObj)
{
    DBG_ASSERT(m_pShell->IsDesignMode(), "nur im designmode verwenden");

    if (m_xSelObject != xNewObj)
    {
        m_xSelObject = xNewObj;
        for (sal_Int16 i = 0; i<sizeof(SelObjectSlotMap)/sizeof(SelObjectSlotMap[0]); ++i)
            InvalidateSlot(SelObjectSlotMap[i]);
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::setCurForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& xF)
{
    DBG_ASSERT(m_pShell->IsDesignMode(), "nur im designmode verwenden");
    if (xF != m_xCurForm)
    {
        m_xCurForm = xF;
        FmFormPage* pPage = m_pShell->GetCurPage();
        if (pPage)
            pPage->GetImpl()->setCurForm( xF );
        for (sal_Int16 i = 0; i<sizeof(DlgSlotMap)/sizeof(DlgSlotMap[0]); ++i)
            InvalidateSlot(DlgSlotMap[i]);
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::startListening()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xDatabaseForm(m_xActiveForm, ::com::sun::star::uno::UNO_QUERY);
    if (xDatabaseForm.is() && ::dbtools::getConnection(xDatabaseForm).is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xActiveFormSet(m_xActiveForm, ::com::sun::star::uno::UNO_QUERY);
        if (xActiveFormSet.is())
        {
            // wenn es eine Datenquelle gibt, dann den Listener aufbauen
            ::rtl::OUString aSource = ::comphelper::getString(xActiveFormSet->getPropertyValue(FM_PROP_COMMAND));
            if (aSource.len())
            {
                // datensatzwechsel mitbekommen
                xDatabaseForm->addRowSetListener(this);

                // Modify
                ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster> xBroadCaster(m_xActiveController, ::com::sun::star::uno::UNO_QUERY);
                if (xBroadCaster.is())
                    xBroadCaster->addModifyListener(this);

                sal_Bool bUseEscapeProcessing = ::comphelper::getBOOL(xActiveFormSet->getPropertyValue(FM_PROP_ESCAPE_PROCESSING));
                if (bUseEscapeProcessing)
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory> xFactory(::dbtools::getConnection(xDatabaseForm), ::com::sun::star::uno::UNO_QUERY);
                    if (xFactory.is())
                        m_xParser = xFactory->createQueryComposer();
                }

                m_bDatabaseBar = sal_True;

                ::com::sun::star::uno::Any aVal    = xActiveFormSet->getPropertyValue(FM_PROP_NAVIGATION);
                m_eNavigate    = *(::com::sun::star::form::NavigationBarMode*) aVal.getValue();

                xActiveFormSet->addPropertyChangeListener(FM_PROP_ISMODIFIED,this);
                xActiveFormSet->addPropertyChangeListener(FM_PROP_ISNEW, this);

                switch (m_eNavigate)
                {
                    case ::com::sun::star::form::NavigationBarMode_PARENT:
                    {
                        // suchen des Controllers, ueber den eine Navigation moeglich ist
                        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(m_xActiveController, ::com::sun::star::uno::UNO_QUERY);
                        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xParent;
                        while (xChild.is())
                        {
                            xChild = ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild>(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
                            xParent  = ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>(xChild, ::com::sun::star::uno::UNO_QUERY);
                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xParentSet;
                            if (xParent.is())
                                xParentSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(xParent->getModel(), ::com::sun::star::uno::UNO_QUERY);
                            if (xParentSet.is())
                            {
                                ::com::sun::star::uno::Any aVal( xParentSet->getPropertyValue(FM_PROP_NAVIGATION) );
                                m_eNavigate = *(::com::sun::star::form::NavigationBarMode*) aVal.getValue();

                                if (m_eNavigate == ::com::sun::star::form::NavigationBarMode_CURRENT)
                                    break;
                            }
                        }
                        m_xNavigationController = xParent;
                    }   break;
                    case ::com::sun::star::form::NavigationBarMode_CURRENT:
                        m_xNavigationController = m_xActiveController;
                        break;
                    default:
                        m_xNavigationController = NULL;
                        m_bDatabaseBar = sal_False;
                }

                // an dem Controller, der die Navigation regelt, wg. RecordCount lauschen
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xNavigationSet;
                if (m_xNavigationController.is())
                {
                    xNavigationSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(m_xNavigationController->getModel(), ::com::sun::star::uno::UNO_QUERY);
                    if (xNavigationSet.is())
                        xNavigationSet->addPropertyChangeListener(FM_PROP_ROWCOUNT,this);
                }

                // und dem Parser die Query-/Filter-/Sort-Einstellungen der aktiven Form
                if (m_xParser.is())
                {
                    ::rtl::OUString aStatement  = ::comphelper::getString(xActiveFormSet->getPropertyValue(FM_PROP_ACTIVECOMMAND));
                    ::rtl::OUString aFilter     = ::comphelper::getString(xActiveFormSet->getPropertyValue(FM_PROP_FILTER_CRITERIA));
                    ::rtl::OUString aSort       = ::comphelper::getString(xActiveFormSet->getPropertyValue(FM_PROP_SORT));

                    try
                    {
                        m_xParser->setQuery(aStatement);
                        m_xParser->setFilter(aFilter);
                        m_xParser->setOrder(aSort);
                    }
                    catch(...)
                    {
                    }


                    // nothing to do, change the parser on a reload
                    xActiveFormSet->addPropertyChangeListener(FM_PROP_ACTIVECOMMAND,this);
                    xActiveFormSet->addPropertyChangeListener(FM_PROP_FILTER_CRITERIA, this);
                    xActiveFormSet->addPropertyChangeListener(FM_PROP_SORT, this);
                }
                m_bActiveModified = sal_False;
                return;
            }
        }
    }

    m_eNavigate  = ::com::sun::star::form::NavigationBarMode_NONE;
    m_bDatabaseBar = sal_False;
    m_xNavigationController = NULL;
    m_bActiveModified = sal_False;
}

//------------------------------------------------------------------------------
void FmXFormShell::stopListening()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet> xDatabaseForm(m_xActiveForm, ::com::sun::star::uno::UNO_QUERY);
    if (xDatabaseForm.is() && m_bDatabaseBar)
    {
        // datensatzwechsel mitbekommen
        xDatabaseForm->removeRowSetListener(this);

        // Modify
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyBroadcaster> xBroadCaster(m_xActiveController, ::com::sun::star::uno::UNO_QUERY);
        if (xBroadCaster.is())
            xBroadCaster->removeModifyListener(this);

        // satzstatus
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xActiveFormSet(m_xActiveForm, ::com::sun::star::uno::UNO_QUERY);
        if (xActiveFormSet.is())
        {
            xActiveFormSet->removePropertyChangeListener(FM_PROP_ISMODIFIED,this);
            xActiveFormSet->removePropertyChangeListener(FM_PROP_ISNEW, this);

            if (m_xParser.is())
            {
                xActiveFormSet->removePropertyChangeListener(FM_PROP_FILTER_CRITERIA, this);
                xActiveFormSet->removePropertyChangeListener(FM_PROP_ACTIVECOMMAND,this);
                xActiveFormSet->removePropertyChangeListener(FM_PROP_SORT,this);
            }
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xComp(m_xParser, ::com::sun::star::uno::UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
        m_xParser = NULL;

        if (m_xNavigationController.is())
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSet(m_xNavigationController->getModel(), ::com::sun::star::uno::UNO_QUERY);
            if (xSet.is())
                xSet->removePropertyChangeListener(FM_PROP_ROWCOUNT, this);

        }
    }

    m_bDatabaseBar = sal_False;
    m_eNavigate  = ::com::sun::star::form::NavigationBarMode_NONE;
    m_xNavigationController = NULL;
    m_bActiveModified = sal_False;
}

//------------------------------------------------------------------------------
void FmXFormShell::ShowProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& rxObject, sal_Bool bShow)
{
    // Evtl. noch aktuelles Objekt umsetzen
    if (rxObject != m_xSelObject)
        setSelObject( rxObject );

    // Wenn Fenster bereits angezeigt wird, nur StatusUpdate
    sal_Bool bHasChild = m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow(SID_FM_SHOW_PROPERTIES);
    if (bHasChild && bShow)
        InvalidateSlot(SID_FM_PROPERTY_CONTROL, sal_True, sal_True);

    // Ansonsten Fenster ein-/ausblenden
    else
        m_pShell->GetViewShell()->GetViewFrame()->ToggleChildWindow(SID_FM_SHOW_PROPERTIES);

    InvalidateSlot(SID_FM_PROPERTIES);
    InvalidateSlot(SID_FM_CTL_PROPERTIES);
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnFoundData, FmFoundRecordInformation*, pfriWhere)
{
    DBG_ASSERT((pfriWhere->nContext >= 0) && (pfriWhere->nContext < m_arrSearchContexts.size()),
        "FmXFormShell::OnFoundData : ungueltiger Kontext !");
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm( m_arrSearchContexts.at(pfriWhere->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnFoundData : ungueltige Form !");

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate> xCursor(xForm, ::com::sun::star::uno::UNO_QUERY);
    if (!xCursor.is())
        return 0;       // was soll ich da machen ?

    // zum Datensatz
    xCursor->moveToBookmark(pfriWhere->aPosition);

    LoopGrids(GA_FORCE_SYNC);

    // und zum Feld (dazu habe ich vor dem Start des Suchens die XVclComponent-Interfaces eingesammelt)
    DBG_ASSERT(pfriWhere->nFieldPos < m_arrSearchedControls.Count(), "FmXFormShell::OnFoundData : ungueltige Daten uebergeben !");
    SdrObject* pObject = m_arrSearchedControls.GetObject(pfriWhere->nFieldPos);
    DBG_ASSERT(pObject != NULL, "FmXFormShell::OnFoundData : unerwartet : ungueltiges VclControl-Interface");

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetPageView(m_pShell->GetCurPage()));
    m_pShell->GetFormView()->MarkObj(pObject, m_pShell->GetFormView()->GetPageView(m_pShell->GetCurPage()));

    DBG_ASSERT(pObject->IsUnoObj(), "FmXFormShell::OnFoundData : ungueltiges Control !");
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> xControlModel( ((SdrUnoObj*)pObject)->GetUnoControlModel());
    DBG_ASSERT(xControlModel.is(), "FmXFormShell::OnFoundData : ungueltiges Control !");

    // disable the permanent cursor for the last grid we found a record
    if (m_aLastGridFound.is() && (m_aLastGridFound != xControlModel))
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xOldSet(m_aLastGridFound, ::com::sun::star::uno::UNO_QUERY);
        sal_Bool bB(sal_False);
        xOldSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState> xOldSetState(xOldSet, ::com::sun::star::uno::UNO_QUERY);
        if (xOldSetState.is())
            xOldSetState->setPropertyToDefault(FM_PROP_CURSORCOLOR);
        else
            xOldSet->setPropertyValue(FM_PROP_CURSORCOLOR, ::com::sun::star::uno::Any());
    }

    // wenn das Feld sich in einem GridControl befindet, muss ich dort noch in die entsprechende Spalte gehen
    sal_Int32 nGridColumn = m_arrRelativeGridColumn.GetObject(pfriWhere->nFieldPos);
    if (nGridColumn != -1)
    {   // dummer weise muss ich mir das Control erst wieder besorgen
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> xControl( GetControlFromModel(xControlModel));
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid> xGrid(xControl, ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xGrid.is(), "FmXFormShell::OnFoundData : ungueltiges Control !");
        // wenn eine der Asserts anschlaegt, habe ich beim Aufbauen von m_arrSearchedControls wohl was falsch gemacht

        // enable a permanent cursor for the grid so we can see the found text
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xModelSet(xControlModel, ::com::sun::star::uno::UNO_QUERY);
        DBG_ASSERT(xModelSet.is(), "FmXFormShell::OnFoundData : invalid control model (no property set) !");
        sal_Bool bB(sal_True);
        xModelSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
        xModelSet->setPropertyValue(FM_PROP_CURSORCOLOR, ::com::sun::star::uno::makeAny(sal_Int32(COL_LIGHTRED)));
        m_aLastGridFound = xControlModel;

        xGrid->setCurrentColumnPosition(nGridColumn);
    }

    // als der Cursor neu positioniert wurde, habe ich (in positioned) meine Formularleisten-Slots invalidiert, aber das greift
    // hier dummerweise nicht, da i.A. ja der (modale) Suchdialog oben ist ... also Gewalt ...
    sal_uInt16 nPos = 0;
    while (DatabaseSlotMap[nPos])
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Update(DatabaseSlotMap[nPos++]);
        // leider geht das Update im Gegensatz zum Invalidate nur mit einzelnen Slots)

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnCanceledNotFound, FmFoundRecordInformation*, pfriWhere)
{
    DBG_ASSERT((pfriWhere->nContext >= 0) && (pfriWhere->nContext < m_arrSearchContexts.size()),
        "FmXFormShell::OnCanceledNotFound : ungueltiger Kontext !");
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm( m_arrSearchContexts.at(pfriWhere->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnCanceledNotFound : ungueltige Form !");

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XRowLocate> xCursor(xForm, ::com::sun::star::uno::UNO_QUERY);
    if (!xCursor.is())
        return 0;       // was soll ich da machen ?

    // zum Datensatz
    xCursor->moveToBookmark(pfriWhere->aPosition);

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetPageViewPvNum(0));
    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnSearchContextRequest, FmSearchContext*, pfmscContextInfo)
{
    DBG_ASSERT(pfmscContextInfo->nContext < m_arrSearchContexts.size(), "FmXFormShell::OnSearchContextRequest : invalid parameter !");
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm( m_arrSearchContexts.at(pfmscContextInfo->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnSearchContextRequest : unexpected : invalid context !");

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xIter(xForm, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xIter.is(), "FmXFormShell::OnSearchContextRequest : unexpected : context has no iterator !");

    // --------------------------------------------------------------------------------------------
    // die Liste der zu involvierenden Felder zusammenstellen (sind die ControlSources aller Felder, die eine solche Eigenschaft habe)
    UniString strFieldList, sFieldDisplayNames;
    m_arrSearchedControls.Remove(0, m_arrSearchedControls.Count());
    m_arrRelativeGridColumn.Remove(0, m_arrRelativeGridColumn.Count());

    // folgendes kleines Problem : Ich brauche, um gefundene Felder zu markieren, SdrObjekte. Um hier festzustellen, welche Controls
    // ich in die Suche einbeziehen soll, brauche ich Controls (also ::com::sun::star::awt::XControl-Interfaces). Ich muss also ueber eines von beiden
    // iterieren und mir das jeweils andere besorgen. Dummerweise gibt es keine direkte Verbindung zwischen beiden Welten (abgesehen
    // von einem GetUnoControl an SdrUnoObject, das aber ein OutputDevice verlangt, womit ich nichts anfangen kann).
    // Allerdings komme ich sowohl von einem Control als auch von einem SdrObject zum Model, und damit ist mir mit einer doppelten
    // Schleife die Zuordnung SdrObject<->Control moeglich.
    // Die Alternative zu dieser (unschoenen und sicher auch nicht ganz fixen) Loesung waere, auf das Cachen der SdrObjects zu
    // verzichten, was dann aber in OnFoundData zu wesentlicher Mehrarbeit fuehren wuerde (da ich mir dort jedesmal das SdrObject
    // erst besorgen muesste). Da aber OnFoundData i.d.R. oefter aufgerufen wird als ExecuteSearch, erledige ich das hier.

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> xValidFormFields;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier> xSupplyCols(xIter, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xSupplyCols.is(), "FmXFormShell::OnSearchContextRequest : invalid cursor : no columns supplier !");
    if (xSupplyCols.is())
        xValidFormFields = xSupplyCols->getColumns();
    DBG_ASSERT(xValidFormFields.is(), "FmXFormShell::OnSearchContextRequest : form has no fields !");

    // aktuelle(r) Page/Controller
    FmFormPage* pCurrentPage = m_pShell->GetCurPage();
    DBG_ASSERT(pCurrentPage!=NULL, "FmXFormShell::OnSearchContextRequest : no page !");
    // alle Sdr-Controls dieser Seite durchsuchen ...
    ::rtl::OUString sControlSource, sCompareString,aName;
    for (int i=0; i<pCurrentPage->GetObjCount(); ++i)
    {
        SdrObject* pCurrent = pCurrentPage->GetObj(i);

        // wenn das Ding eine Gruppierung ist -> durchiterieren
        SdrObjListIter* pGroupIterator = NULL;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator = new SdrObjListIter(*pCurrent->GetSubList());
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }

        while (pCurrent)
        {
            if (pCurrent->IsUnoObj())
            {
                // das Model zum aktuellen Object ...
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> xControlModel( ((SdrUnoObj*)pCurrent)->GetUnoControlModel());
                DBG_ASSERT(xControlModel.is(), "FmXFormShell::OnSearchContextRequest : invalid objects !");
                // ... als FormComponent
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent> xCurrentAsFormComponent(xControlModel, ::com::sun::star::uno::UNO_QUERY);
                if (!xCurrentAsFormComponent.is())
                {
                    if (pGroupIterator)     // ich iteriere gerade durch eine Gruppe von Controls
                        pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
                    else
                        pCurrent = NULL;
                    continue;
                }

                // gehoert diese FormComponent ueberhaupt zu der Form, um die es geht ?
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xParentOfCurrent( xCurrentAsFormComponent->getParent());
                if (xParentOfCurrent != xForm) // vergleich fuehrt zu richtigem Ergebnis, seit TF_ONE
                {
                    if (pGroupIterator)     // ich iteriere gerade durch eine Gruppe von Controls
                        pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
                    else
                        pCurrent = NULL;
                    continue;
                }

                // ... nach der ControlSource-Eigenschaft fragen
                SearchableControlIterator iter(xCurrentAsFormComponent);
                ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> xControlBehindModel;
                    // das Control, das als Model xControlModel hat
                    // (das folgende while kann mehrmals durchlaufen werden, ohne dass das Control sich aendert, dann muss
                    // ich nicht jedesmal neu suchen)

                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xCurrent( iter.Next());
                while (xCurrent.is())
                {
                    sControlSource = iter.getCurrentValue();
                    if (sControlSource == sCompareString)
                    {   // das aktuelle Element hat keine ControlSource, also ist es ein GridControl (das ist das einzige, was
                        // der SearchableControlIterator noch zulaesst)
                        xControlBehindModel = GetControlFromModel(xControlModel);
                        DBG_ASSERT(xControlBehindModel.is(), "FmXFormShell::OnSearchContextRequest : didn't ::std::find a control with requested model !");

                        ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridPeer> xGridPeer(xControlBehindModel->getPeer(), ::com::sun::star::uno::UNO_QUERY);
                        do
                        {
                            if (!xGridPeer.is())
                                break;

                            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xPeerContainer(xGridPeer, ::com::sun::star::uno::UNO_QUERY);
                            if (!xPeerContainer.is())
                                break;

                            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xModelColumns(xGridPeer->getColumns(), ::com::sun::star::uno::UNO_QUERY);
                            DBG_ASSERT(xModelColumns.is(), "FmXFormShell::OnSearchContextRequest : there is a grid control without columns !");
                                // the case 'no columns' should be indicated with an empty container, I think ...
                            DBG_ASSERT(xModelColumns->getCount() >= xPeerContainer->getCount(), "FmXFormShell::OnSearchContextRequest : impossible : have more view than model columns !");

                            for (sal_Int16 nViewPos=0; nViewPos<xPeerContainer->getCount(); ++nViewPos)
                            {
                                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xCurrentColumn(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xPeerContainer->getByIndex(nViewPos).getValue(), ::com::sun::star::uno::UNO_QUERY);
                                if (!xCurrentColumn.is())
                                    continue;

                                // can we use this column control fo searching ?
                                if (!IsSearchableControl(xCurrentColumn))
                                    continue;

                                sal_Int16 nModelPos = GridView2ModelPos(xModelColumns, nViewPos);
                                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCurrentColModel( *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)xModelColumns->getByIndex(nModelPos).getValue());
                                aName = ::comphelper::getString(xCurrentColModel->getPropertyValue(FM_PROP_CONTROLSOURCE));
                                // the cursor has a field matching the control source ?
                                if (xValidFormFields->hasByName(aName))
                                {
                                    strFieldList += aName.getStr();
                                    strFieldList += ';';

                                    sFieldDisplayNames += ::comphelper::getString(xCurrentColModel->getPropertyValue(FM_PROP_LABEL)).getStr();
                                    sFieldDisplayNames += ';';

                                    pfmscContextInfo->arrFields.push_back(xCurrentColumn);

                                    // und das SdrObjekt zum Feld
                                    m_arrSearchedControls.C40_INSERT(SdrObject, pCurrent, m_arrSearchedControls.Count());
                                    // die Nummer der Spalte
                                    m_arrRelativeGridColumn.Insert(nViewPos, m_arrRelativeGridColumn.Count());
                                }
                            }
                        } while (sal_False);
                    }
                    else
                    {
                        if (sControlSource.getLength() && xValidFormFields->hasByName(sControlSource))
                        {
                            // jetzt brauche ich das Control zum SdrObject
                            if (!xControlBehindModel.is())
                            {
                                xControlBehindModel = GetControlFromModel(xControlModel);
                                DBG_ASSERT(xControlBehindModel.is(), "FmXFormShell::OnSearchContextRequest : didn't ::std::find a control with requested model !");
                            }

                            if (IsSearchableControl(xControlBehindModel))
                            {   // alle Tests ueberstanden -> in die Liste mit aufnehmen
                                strFieldList += sControlSource.getStr();
                                strFieldList += ';';

                                // the label which should appear for the control :
                                sFieldDisplayNames += getLabelName(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(xControlModel, ::com::sun::star::uno::UNO_QUERY)).getStr();
                                sFieldDisplayNames += ';';

                                // das SdrObjekt merken (beschleunigt die Behandlung in OnFoundData)
                                m_arrSearchedControls.C40_INSERT(SdrObject, pCurrent, m_arrSearchedControls.Count());

                                // die Nummer der Spalte (hier ein Dummy, nur fuer GridControls interesant)
                                m_arrRelativeGridColumn.Insert(-1, m_arrRelativeGridColumn.Count());

                                // und fuer die formatierte Suche ...
                                pfmscContextInfo->arrFields.push_back(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>(xControlBehindModel, ::com::sun::star::uno::UNO_QUERY));
                            }
                        }
                    }

                    xCurrent = iter.Next();
                }
            }

            if (pGroupIterator)     // ich iteriere gerade durch eine Gruppe von Controls
                pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
            else
                pCurrent = NULL;
        }
        delete pGroupIterator;
    }
    strFieldList.EraseTrailingChars(';');
    sFieldDisplayNames.EraseTrailingChars(';');

    if (!pfmscContextInfo->arrFields.size())
    {
        pfmscContextInfo->arrFields.clear();
        pfmscContextInfo->xCursor = NULL;
        pfmscContextInfo->strUsedFields.Erase();
        return 0L;
    }

    pfmscContextInfo->xCursor = xIter;
    pfmscContextInfo->strUsedFields = strFieldList;
    pfmscContextInfo->sFieldDisplayNames = sFieldDisplayNames;

    // 66463 - 31.05.99 - FS
    // wenn der Cursor sich in einem anderen RecordMode als STANDARD befindet, ruecksetzen
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCursorSet(pfmscContextInfo->xCursor, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate> xUpdateCursor(pfmscContextInfo->xCursor, ::com::sun::star::uno::UNO_QUERY);
    if (xUpdateCursor.is() && xCursorSet.is() && xCursorSet.is())
        if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISNEW)))
            xUpdateCursor->moveToCurrentRow();
        else
            if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISMODIFIED)))
                xUpdateCursor->cancelRowUpdates();

    return pfmscContextInfo->arrFields.size();
}

  // ::com::sun::star::container::XContainerListener
//------------------------------------------------------------------------------
void FmXFormShell::elementInserted(const ::com::sun::star::container::ContainerEvent& evt)
{
    // neues Object zum lauschen
    AddElement(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)evt.Element.getValue());
    m_pShell->DetermineForms(sal_True);
}

//------------------------------------------------------------------------------
void FmXFormShell::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt)
{
    RemoveElement(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)evt.ReplacedElement.getValue());
    AddElement(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)evt.Element.getValue());
}

//------------------------------------------------------------------------------
void FmXFormShell::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt)
{
    RemoveElement(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)evt.Element.getValue());
    m_pShell->DetermineForms(sal_True);
}

//------------------------------------------------------------------------------
void FmXFormShell::ResetForms(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xForms, sal_Bool bInvalidate)
{
    if (m_xForms != _xForms)
    {
        RemoveElement(m_xForms);

        m_xForms = _xForms;

        AddElement(m_xForms);
    }
    m_pShell->DetermineForms(bInvalidate);
}

//------------------------------------------------------------------------------
void FmXFormShell::AddElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element)
{
    // am Container horchen
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> xContainer(Element, ::com::sun::star::uno::UNO_QUERY);
    if (xContainer.is())
    {
        sal_uInt32 nCount = xContainer->getCount();
        ::com::sun::star::uno::Any aElement;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            aElement = xContainer->getByIndex(i);
            AddElement(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aElement.getValue());
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer> xCont(Element, ::com::sun::star::uno::UNO_QUERY);
        if (xCont.is())
            xCont->addContainerListener(this);
    }

    ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier> xSelSupplier(Element, ::com::sun::star::uno::UNO_QUERY);
    if (xSelSupplier.is())
        xSelSupplier->addSelectionChangeListener(this);
}

//------------------------------------------------------------------------------
void FmXFormShell::RemoveElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier> xSelSupplier(Element, ::com::sun::star::uno::UNO_QUERY);
    if (xSelSupplier.is())
        xSelSupplier->removeSelectionChangeListener(this);

    // Verbindung zu Kindern aufheben
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> xContainer(Element, ::com::sun::star::uno::UNO_QUERY);
    if (xContainer.is())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer> xCont(Element, ::com::sun::star::uno::UNO_QUERY);
        if (xCont.is())
            xCont->removeContainerListener(this);

        sal_uInt32 nCount = xContainer->getCount();
        ::com::sun::star::uno::Any aElement;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            aElement = xContainer->getByIndex(i);
            RemoveElement(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aElement.getValue());
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::selectionChanged(const ::com::sun::star::lang::EventObject& rEvent)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier> xSupplier(rEvent.Source, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Any aSelObj( xSupplier->getSelection() );
    if (aSelObj.getValueTypeClass() != ::com::sun::star::uno::TypeClass_INTERFACE)
        return;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xSelObj( *(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aSelObj.getValue());

    // es wurde eine Selektion weggenommen, dieses kann nur durch die Shell vorgenommen werden
    if (!xSelObj.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xPreviousObject( getSelObject());
    EnableTrackProperties(sal_False);

    m_pShell->GetFormView()->UnmarkAll();
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xNewForm( GetForm(rEvent.Source));

    setCurControl(xSelObj);
    setCurForm(xNewForm);

    // wurde vorher Form angezeigt, dann wieder die Form anzeigen
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xOldForm(xPreviousObject, ::com::sun::star::uno::UNO_QUERY);
    if (xOldForm.is())
        setSelObject(m_xCurForm);
    else
        setSelObject(m_xCurControl);

    if (IsPropBrwOpen() && m_xSelObject != xPreviousObject)
        ShowProperties(m_xSelObject, sal_True);

    EnableTrackProperties(sal_True);
    m_pShell->NotifyMarkListChanged(m_pShell->GetFormView());
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnTimeOut, void*, EMPTYTAG)
{
    if (m_pShell->IsDesignMode() && m_pShell->GetFormView())
        SetSelection(m_pShell->GetFormView()->GetMarkList());

    return 0;
}

//------------------------------------------------------------------------
void FmXFormShell::SetSelectionDelayed(FmFormView* pView)
{
    if (m_pShell->IsDesignMode() && IsTrackPropertiesEnabled() && !m_aMarkTimer.IsActive())
        m_aMarkTimer.Start();
}

//------------------------------------------------------------------------
void FmXFormShell::SetSelection(const SdrMarkList& rMarkList)
{
    DetermineSelection(rMarkList);
    m_pShell->NotifyMarkListChanged(m_pShell->GetFormView());
}

//------------------------------------------------------------------------
void FmXFormShell::DetermineSelection(const SdrMarkList& rMarkList)
{
    // Aktuelles Objekt setzen
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xPreviousObject( m_xSelObject);
    sal_Bool bMixedWithFormControls = sal_True;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xNewForm( DetermineCurForm(rMarkList, bMixedWithFormControls));

    // Aktuelle Form anpassen
    if (xNewForm.is() || bMixedWithFormControls)
    {
        // die Form wird nur angepasst, wenn eine eindeutige From gefunden wurde oder eine gemischte Selektion existiert,
        // die auch FormControls enthaelt (NICHT bei einer Selektion ohne solche oder bei leerer Selektion)
        if ((::com::sun::star::form::XForm*)xNewForm.get() != (::com::sun::star::form::XForm*)m_xCurForm.get())
            setCurForm( xNewForm );
    }

    setCurControl(rMarkList);

    // wurde vorher Form angezeigt, dann wieder die Form anzeigen
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xOldForm(m_xSelObject, ::com::sun::star::uno::UNO_QUERY);
    if (xOldForm.is())
        setSelObject(xNewForm);
    else
        setSelObject(m_xCurControl);

    // Falls ein Propertybrowser angezeigt wird, diesen invalidieren
    if (IsPropBrwOpen() && xPreviousObject != m_xSelObject)
        ShowProperties(m_xSelObject, sal_True);
}

//------------------------------------------------------------------------------
void FmXFormShell::SaveMarkList(const FmFormView* pView)
{
    m_aMark = pView->GetMarkList();
    m_aMarkTimer.Stop();
}

//------------------------------------------------------------------------------
void FmXFormShell::RestoreMarkList(FmFormView* pView)
{
    const SdrMarkList& rCurrentList = pView->GetMarkList();
    FmFormPage* pPage = m_pShell->GetCurPage();
    if (pPage)
    {
        // es gibt eine Markierung, dann diese beibehalten
        if (rCurrentList.GetMarkCount())
        {
            m_aMark.Clear();
            SetSelection(rCurrentList);
        }
        else
        {
            // wichtig ist das auf die Objecte der markliste nicht zugegriffen wird
            // da diese bereits zerstoert sein koennen
            SdrPageView* pCurPageView = pView->GetPageViewPvNum(0);
            SdrObjListIter aPageIter( *pPage );
            sal_Bool bFound = sal_True;

            // gibt es noch alle Objecte
            sal_uInt32 nCount = m_aMark.GetMarkCount();
            for (sal_uInt32 i = 0; i < nCount && bFound; i++)
            {
                SdrMark*   pMark = m_aMark.GetMark(i);
                SdrObject* pObj  = pMark->GetObj();
                if (pObj->IsGroupObject())
                {
                    SdrObjListIter aIter(*pObj->GetSubList());
                    while (aIter.IsMore() && bFound)
                        bFound = hasObject(aPageIter, aIter.Next());
                }
                else
                    bFound = hasObject(aPageIter, pObj);

                bFound = bFound && pCurPageView == pMark->GetPageView();
            }

            if (bFound)
            {
                // Das LastObject auswerten
                if (nCount) // Objecte jetzt Markieren
                {
                    sal_Bool bEnable = sal_False;
                    if (IsTrackPropertiesEnabled())
                    {
                        EnableTrackProperties(sal_False);
                        bEnable = sal_True;
                    }

                    for (sal_uInt32 i = 0; i < nCount; i++)
                    {
                        SdrMark* pMark = m_aMark.GetMark(i);
                        pView->MarkObj(pMark->GetObj(), pMark->GetPageView());
                    }

                    if (bEnable)
                        EnableTrackProperties(sal_True);

                    SetSelection(m_aMark);
                }
                else
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xCont(pPage->GetForms(), ::com::sun::star::uno::UNO_QUERY);

                    // Ist das aktuelle Object ein Element eines SelectionSuppliers?
                    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> xChild(m_xCurControl,::com::sun::star::uno::UNO_QUERY);
                    ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier> xSelectionSupplier;
                    if (xChild.is())
                        xSelectionSupplier = ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier>(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
                    if (xSelectionSupplier.is())
                    {
                        // suchen der Zugehoreigen Form
                        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm(GetForm(m_xCurControl));
                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xIface(xForm, ::com::sun::star::uno::UNO_QUERY);
                        if (xForm.is() && searchElement(xCont, xIface))
                        {
                            setCurForm(xForm);
                            setCurControl(m_xCurControl);

                            // jetzt noch die Selection vornehmen
                            xSelectionSupplier->select(::com::sun::star::uno::makeAny(m_xCurControl));
                        }
                    }
                    else
                    {
                        // Auswerten des letzen Objects
                        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xForm(m_xCurForm, ::com::sun::star::uno::UNO_QUERY);
                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xIface(xForm, ::com::sun::star::uno::UNO_QUERY);
                        if (xForm.is() && searchElement(xCont, xIface))
                        {
                            setCurForm(xForm);
                        }
                        else
                        {
                            if (pPage->GetImpl()->getCurForm().is())
                                xForm = pPage->GetImpl()->getCurForm();
                            else if (xCont.is() && xCont->getCount())
                                xForm = pPage->GetImpl()->getDefaultForm();

                            if (xForm.is())
                                setCurForm(xForm);
                        }
                    }

                    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xPreviousObject(getSelObject());

                    // wurde vorher Form angezeigt, dann wieder die Form anzeigen
                    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xOldForm(xPreviousObject, ::com::sun::star::uno::UNO_QUERY);
                    if (xOldForm.is())
                        setSelObject(m_xCurForm);
                    else
                        setSelObject(m_xCurControl);

                    if (IsPropBrwOpen() && m_xSelObject != xPreviousObject)
                        ShowProperties(m_xSelObject, sal_True);
                }
            }
            m_aMark.Clear();
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::IsPropBrwOpen() const
{
    return( ( m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame() ) ?
            m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow(SID_FM_SHOW_PROPERTIES) : sal_False );
}

//------------------------------------------------------------------------------
void FmXFormShell::SetDesignMode(sal_Bool bDesign)
{
    DBG_ASSERT(m_pShell && m_pShell->GetFormView(), "FmXFormShell::SetDesignMode : invalid call (have no shell or no view) !");
    m_bChangingDesignMode = sal_True;

    // 67506 - 15.07.99 - FS
    // if we're switching off the design mode we have to force the property browser to be closed
    // so it can commit it's changes _before_ we load the forms
    if (!bDesign)
    {
        m_bHadPropBrw = m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow(SID_FM_SHOW_PROPERTIES);
        if (m_bHadPropBrw)
            m_pShell->GetViewShell()->GetViewFrame()->ToggleChildWindow(SID_FM_SHOW_PROPERTIES);
    }

    FmFormView* pFormView = m_pShell->GetFormView();
    if (bDesign)
    {
        // we are currently filtering, so stop filtering
        if (m_bFilterMode)
            stopFiltering(sal_False);

        // an den Objekten meiner MarkList als Listener abmelden
        if (m_pCheckForRemoval)
        {
            m_pCheckForRemoval->EndListeningAll();
            delete m_pCheckForRemoval;
            m_pCheckForRemoval = NULL;
        }
    }
    else
        SaveMarkList(pFormView);

    if (bDesign && m_xExternalViewController.is())
        CloseExternalFormViewer();

    pFormView->ChangeDesignMode(bDesign);

    // Listener benachrichtigen
    FmDesignModeChangedHint aChangedHint( bDesign );
    m_pShell->Broadcast(aChangedHint);

    FmFormPage* pPage = m_pShell->GetCurPage();
    if (pPage)
    {
        if (bDesign)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xIndex(pPage->GetForms(), ::com::sun::star::uno::UNO_QUERY);
            ResetForms(xIndex, sal_False);
        }
        else
            ResetForms();
    }

    m_pShell->m_bDesignMode = bDesign;
    if (bDesign)
        RestoreMarkList(pFormView);
    else
    {
        // am Model der View als Listener anmelden (damit ich mitbekomme, wenn jemand waehrend des Alive-Modus
        // Controls loescht, die ich eigentlich mit SaveMarkList gespeichert habe) (60343)
        if (!m_pCheckForRemoval)
        {
            m_pCheckForRemoval = new ObjectRemoveListener(this);
            FmFormModel* pModel = m_pShell->GetFormModel();
            DBG_ASSERT(pModel != NULL, "FmXFormShell::SetDesignMode : shell has no model !");
            m_pCheckForRemoval->StartListening(*(SfxBroadcaster*)pModel);
        }
        else
            DBG_ERROR("FmXFormShell::SetDesignMode : already listening !");
    }

    m_pShell->UIFeatureChanged();

    // 67506 - 15.07.99 - FS
    if (bDesign && m_bHadPropBrw)
    {
        // The UIFeatureChanged performes an update (a check of the available features) asynchronously.
        // So we can't call ShowProperties directly as the according feature isn't enabled yet.
        // That's why we use an asynchron execution on the dispatcher.
        // (And that's why this has to be done AFTER the UIFeatureChanged.)
        FmInterfaceItem aInterfaceItem( SID_FM_SHOW_PROPERTY_BROWSER, getSelObject() );
        m_pShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON,
            &aInterfaceItem, 0L );
    }
    m_bChangingDesignMode = sal_False;
}

//------------------------------------------------------------------------------
void FmXFormShell::ObjectRemovedInAliveMode(const SdrObject* pObject)
{
    // wenn das entfernte Objekt in meiner MarkList, die ich mir beim Umschalten in den Alive-Mode gemerkt habe, steht,
    // muss ich es jetzt da rausnehmen, da ich sonst beim Zurueckschalten versuche, die Markierung wieder zu setzen
    // (interesanterweise geht das nur bei gruppierten Objekten schief (beim Zugriff auf deren ObjList GPF), nicht bei einzelnen)

    sal_uInt32 nCount = m_aMark.GetMarkCount();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        SdrMark* pMark = m_aMark.GetMark(i);
        SdrObject* pCurrent = pMark->GetObj();
        if (pObject == pCurrent)
        {
            m_aMark.DeleteMark(i);
            return;
        }
        // ich brauche nicht in GroupObjects absteigen : wenn dort unten ein Objekt geloescht wird, dann bleibt der
        // Zeiger auf das GroupObject, den ich habe, trotzdem weiter gueltig bleibt ...
    }
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> FmXFormShell::GetControlFromModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xModel)
{
    // die View ...
    SdrPageView* pCurPageView = m_pShell->GetFormView()->GetPageViewPvNum(0);
    // deren ViewWinRec-Liste, daraus das erste Element
    DBG_ASSERT(pCurPageView->GetWinList().GetCount() > 0, "FmXFormShell::GetControlFromModel : unexpected : no SdrPageViewWinRecs");
    const SdrPageViewWinRec& rViewWinRec = pCurPageView->GetWinList()[0];
    // von dem bekomme ich alle Controls ...
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xControlContainer(rViewWinRec.GetControlContainerRef());
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> > seqControls( xControlContainer->getControls() );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* pControls = seqControls.getArray();
    // ... die ich dann durchsuchen kann
    for (int i=0; i<seqControls.getLength(); ++i)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel> xSearchLoopModel( pControls[i]->getModel());
        if ((::com::sun::star::awt::XControlModel*)xSearchLoopModel.get() == (::com::sun::star::awt::XControlModel*)xModel.get())
            return pControls[i];
    }
    return ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>(NULL);
}

//------------------------------------------------------------------------------
void FmXFormShell::CollectFormContexts(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xStartingPoint, const UniString& sCurrentLevelPrefix, UniString& sNames)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xAllComponentIndizies(xStartingPoint, ::com::sun::star::uno::UNO_QUERY);

    if (xAllComponentIndizies.is() && xAllComponentIndizies->getCount())
    {
        // das Ding hat Kinder, also gehen wir die alle durch
        ::com::sun::star::uno::Any aElement;
        UniString sCurrentFormName, sNextLevelPrefix;
        for (sal_Int32 i=0; i<xAllComponentIndizies->getCount(); ++i)
        {
            aElement = xAllComponentIndizies->getByIndex(i);
            // ist das aktuelle Kind eine Form ?
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xCurrentAsForm(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aElement.getValue(), ::com::sun::star::uno::UNO_QUERY);
            if (xCurrentAsForm.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xAskForName(xCurrentAsForm, ::com::sun::star::uno::UNO_QUERY);
                if (xAskForName.is())
                {
                    try { sCurrentFormName = ::comphelper::getString(xAskForName->getPropertyValue(FM_PROP_NAME)).getStr(); }
                    catch(...) { }
                }
                // den Namen an die Aufzaehlung haengen
                sNames += ';';
                sNames += sCurrentFormName;
                if (sCurrentLevelPrefix.Len() != 0)
                {
                    sNames.AppendAscii(" (");
                    sNames += sCurrentLevelPrefix;
                    sNames += ')';
                }

                // den Prefix fuer den naechsten Level
                sNextLevelPrefix = sCurrentLevelPrefix;
                if (sCurrentLevelPrefix.Len() != 0)
                    sNextLevelPrefix += '/';
                sNextLevelPrefix += sCurrentFormName;

                // das Interface merken
                m_arrSearchContexts.push_back(xCurrentAsForm);

                // und absteigen
                CollectFormContexts(xCurrentAsForm, sNextLevelPrefix, sNames);
            }
        }
    }
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> FmXFormShell::GetBoundField(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>& _xControl, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& _xForm) const
{
    // sonderbehandlung fuer databaseGrid
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XGrid> xGrid(_xControl, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xModel, xField;

    if (xGrid.is())
    {
        sal_Int16 nCurrentPos = xGrid->getCurrentColumnPosition();
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xCols(_xControl->getModel(), ::com::sun::star::uno::UNO_QUERY);
        nCurrentPos = GridView2ModelPos(xCols, nCurrentPos);
        if (nCurrentPos != (sal_Int16)-1)
            xModel = *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)xCols->getByIndex(nCurrentPos).getValue();
    }
    else if (_xControl.is())
    {
        xModel = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(_xControl->getModel(), ::com::sun::star::uno::UNO_QUERY);
    }

    // Das gebundene Feld ist jetzt eine extra Eigenschaft, nicht mehr indirekt ueber die ControlSource zu besorgen,
    // da das Control selber durchaus entscheiden kann, dass es sich trotz gueltiger ControlSource nicht binden will.
    // (zum Beispiel ein TextControl an ein Bild-Feld)
    // (FS - 64265)
    if (xModel.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xModel))
    {
        ::com::sun::star::uno::Any aElement( xModel->getPropertyValue(FM_PROP_BOUNDFIELD) );
        xField = *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)aElement.getValue();
    }

    return xField;
}

//------------------------------------------------------------------------------
void FmXFormShell::SetWizardUsing(sal_Bool bUseThem)
{
    // gleich an die INI weiterreichen
    SFX_INIMANAGER()->Set( (bUseThem ? '1' : '0'), SFX_GROUP_DATABASE, String::CreateFromAscii("UseWizards"));
    m_bUseWizards = bUseThem;
}

//------------------------------------------------------------------------------
FmXFormShell::ObjectRemoveListener::ObjectRemoveListener(FmXFormShell* pParent)
    :SfxListener()
    ,m_pParent(pParent)
{
}

//------------------------------------------------------------------------------
void FmXFormShell::ObjectRemoveListener::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if (rHint.ISA(SdrHint) && (((SdrHint&)rHint).GetKind() == HINT_OBJREMOVED))
        m_pParent->ObjectRemovedInAliveMode(((SdrHint&)rHint).GetObject());
}

//------------------------------------------------------------------------------
void FmXFormShell::startFiltering()
{
    // setting all forms in filter mode
    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xContainer;
    if (getActiveController() == m_xExternalViewController)
    {
        DBG_ASSERT(m_xExtViewTriggerController.is(), "FmXFormShell::startFiltering : inconsistent : active external controller, but noone triggered this !");
        xContainer = m_xExtViewTriggerController->getContainer();
    }
    else
        xContainer = getActiveController()->getContainer();

    FmWinRecList::iterator i = pXView->findWindow(xContainer);
    if (i != pXView->getWindowList().end())
    {
        const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> >& rControllerList = (*i)->GetList();
        for (::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> >::const_iterator j = rControllerList.begin();
             j != rControllerList.end(); ++j)
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector> xModeSelector(*j, ::com::sun::star::uno::UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode(FILTER_MODE);
        }
    }

    m_bFilterMode = sal_True;

    m_pShell->UIFeatureChanged();
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}

//------------------------------------------------------------------------------
void saveFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xFormAsSet(xController->getModel(), ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xControllerAsSet(xController, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xControllerAsIndex(xController, ::com::sun::star::uno::UNO_QUERY);

    // call the subcontroller
    for (sal_Int32 i = 0, nCount = xControllerAsIndex->getCount();
         i < nCount; i++)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xController(*(::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>*)xControllerAsIndex->getByIndex(i).getValue());
        saveFilter(xController);
    }

    try
    {

        xFormAsSet->setPropertyValue(FM_PROP_FILTER_CRITERIA, xControllerAsSet->getPropertyValue(FM_PROP_FILTER_CRITERIA));

        sal_Bool bB(sal_True);
        xFormAsSet->setPropertyValue(FM_PROP_APPLYFILTER, ::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
    }
    catch(...)
    {
    }

}

//------------------------------------------------------------------------------
void FmXFormShell::stopFiltering(sal_Bool bSave)
{
    m_bFilterMode = sal_False;

    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xContainer;
    if (getActiveController() == m_xExternalViewController)
    {
        DBG_ASSERT(m_xExtViewTriggerController.is(), "FmXFormShell::startFiltering : inconsistent : active external controller, but noone triggered this !");
        xContainer = m_xExtViewTriggerController->getContainer();
    }
    else
        xContainer = getActiveController()->getContainer();

    FmWinRecList::iterator i = pXView->findWindow(xContainer);
    if (i != pXView->getWindowList().end())
    {
        const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> >& rControllerList = (*i)->GetList();
        ::std::vector < ::rtl::OUString >   aOriginalFilters;
        ::std::vector < sal_Bool >          aOriginalApplyFlags;

        if (bSave)
        {
            for (::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> > ::const_iterator j = rControllerList.begin();
                 j != rControllerList.end(); ++j)
            {
                if (bSave)
                {   // remember the current filter settings in case we're goin to reload the forms below (which may fail)
                    try
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xFormAsSet((*j)->getModel(), ::com::sun::star::uno::UNO_QUERY);
                        aOriginalFilters.push_back(::comphelper::getString(xFormAsSet->getPropertyValue(FM_PROP_FILTER_CRITERIA)));
                        aOriginalApplyFlags.push_back(::comphelper::getBOOL(xFormAsSet->getPropertyValue(FM_PROP_APPLYFILTER)));
                    }
                    catch(...)
                    {
                        DBG_ERROR("FmXFormShell::stopFiltering : could not get the original filter !");
                        // put dummies into the arrays so the they have the right size

                        if (aOriginalFilters.size() == aOriginalApplyFlags.size())
                            // the first getPropertyValue failed -> use two dummies
                            aOriginalFilters.push_back( ::rtl::OUString() );
                        aOriginalApplyFlags.push_back( sal_False );
                    }
                }
                saveFilter(*j);
            }
        }
        for (::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> > ::const_iterator j = rControllerList.begin();
             j != rControllerList.end(); ++j)
        {

            ::com::sun::star::uno::Reference< ::com::sun::star::util::XModeSelector> xModeSelector(*j, ::com::sun::star::uno::UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode(DATA_MODE);
        }
        if (bSave)  // execute the filter
        {
            const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> > & rControllerList = (*i)->GetList();
            for (::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> > ::const_iterator j = rControllerList.begin();
                 j != rControllerList.end(); ++j)
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable> xReload((*j)->getModel(), ::com::sun::star::uno::UNO_QUERY);
                if (!xReload.is())
                    continue;
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xFormSet(xReload, ::com::sun::star::uno::UNO_QUERY);

                try
                {
                    xReload->reload();
                }
                catch(...)
                {
                }

                if (!isRowSetAlive(xFormSet))
                {   // something went wrong -> restore the original state
                    ::rtl::OUString sOriginalFilter = aOriginalFilters[ j - rControllerList.begin() ];
                    sal_Bool bOriginalApplyFlag = aOriginalApplyFlags[ j - rControllerList.begin() ];
                    try
                    {
                        xFormSet->setPropertyValue(FM_PROP_FILTER_CRITERIA, ::com::sun::star::uno::makeAny(sOriginalFilter));
                        xFormSet->setPropertyValue(FM_PROP_APPLYFILTER, ::com::sun::star::uno::makeAny(bOriginalApplyFlag));
                        xReload->reload();
                    }
                    catch(...)
                    {
                    }
                }
            }
        }
    }

    m_pShell->UIFeatureChanged();
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}

//------------------------------------------------------------------------------
void clearFilter(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xControllerAsSet(xController, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xControllerAsIndex(xController, ::com::sun::star::uno::UNO_QUERY);

    // call the subcontroller
    for (sal_Int32 i = 0, nCount = xControllerAsIndex->getCount();
         i < nCount; i++)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xController(*(::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>*)xControllerAsIndex->getByIndex(i).getValue());
        clearFilter(xController);
    }

    // clear the filter
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer> xContainer(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xControllerAsSet->getPropertyValue(FM_PROP_FILTERSUPPLIER).getValue(), ::com::sun::star::uno::UNO_QUERY);
    if (xContainer.is())
    {
        // clear the current filter
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aCondition;

        // as there is always an empty row, if we have a filter:
        if (xContainer->getCount())
        {
            xControllerAsSet->setPropertyValue(FM_PROP_CURRENTFILTER, ::com::sun::star::uno::makeAny(sal_Int32(xContainer->getCount() - 1)));
            while (xContainer->getCount() > 1)
                xContainer->removeByIndex(0);
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::clearFilter()
{
    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xContainer;
    if (getActiveController() == m_xExternalViewController)
    {
        DBG_ASSERT(m_xExtViewTriggerController.is(), "FmXFormShell::startFiltering : inconsistent : active external controller, but noone triggered this !");
        xContainer = m_xExtViewTriggerController->getContainer();
    }
    else
        xContainer = getActiveController()->getContainer();

    FmWinRecList::iterator i = pXView->findWindow(xContainer);
    if (i != pXView->getWindowList().end())
    {
        const ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> > & rControllerList = (*i)->GetList();
        for (::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> > ::const_iterator j = rControllerList.begin();
             j != rControllerList.end(); ++j)
        {
            ::clearFilter(*j);
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::setControlLocks()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController> xCtrler(getActiveController(), ::com::sun::star::uno::UNO_QUERY);
    if (!xCtrler.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xControls(xCtrler->getContainer(), ::com::sun::star::uno::UNO_QUERY);
    if (!xControls.is())
        return;

    DBG_ASSERT(m_aControlLocks.Count() == 0, "FmXFormShell::setControlLocks : locking state array isn't empty (called me twice ?) !");

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> > aControls = xControls->getControls();
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* pControls = aControls.getConstArray();

    // iterate through all bound controls, remember the old locking state, set the lock
    for (sal_Int32 i=0; i<aControls.getLength(); ++i)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundControl> xCtrl(pControls[i], ::com::sun::star::uno::UNO_QUERY);
        if (!xCtrl.is())
        {
            // it may be a container of controls
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xContainer(pControls[i], ::com::sun::star::uno::UNO_QUERY);
            if (xContainer.is())
            {   // no recursion. we only know top level control containers (e.g. grid controls)
                for (sal_Int16 i=0; i<xContainer->getCount(); ++i)
                {
                    xCtrl = ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundControl>(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xContainer->getByIndex(i).getValue(), ::com::sun::star::uno::UNO_QUERY);
                    if (!xCtrl.is())
                        continue;

                    m_aControlLocks.Insert(xCtrl->getLock(), m_aControlLocks.Count());
                    xCtrl->setLock(sal_True);
                }
            }
            continue;
        }

        m_aControlLocks.Insert(xCtrl->getLock(), m_aControlLocks.Count());
        xCtrl->setLock(sal_True);
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::restoreControlLocks()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController> xCtrler(getActiveController(), ::com::sun::star::uno::UNO_QUERY);
    if (!xCtrler.is())
        return;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer> xControls(xCtrler->getContainer(), ::com::sun::star::uno::UNO_QUERY);
    if (!xControls.is())
        return;

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> > aControls(xControls->getControls());
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* pControls = aControls.getConstArray();

    // iterate through all bound controls, restore the old locking state
    sal_Int32 nBoundControl = 0;
    for (sal_Int32 i=0; i<aControls.getLength(); ++i)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundControl> xCtrl(pControls[i], ::com::sun::star::uno::UNO_QUERY);
        if (!xCtrl.is())
        {
            // it may be a container of controls
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> xContainer(pControls[i], ::com::sun::star::uno::UNO_QUERY);
            if (xContainer.is())
            {   // no recursion. we only know top level control containers (e.g. grid controls)
                for (sal_Int16 i=0; i<xContainer->getCount(); ++i)
                {
                    xCtrl = ::com::sun::star::uno::Reference< ::com::sun::star::form::XBoundControl>(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)xContainer->getByIndex(i).getValue(), ::com::sun::star::uno::UNO_QUERY);
                    if (!xCtrl.is())
                        continue;

                    DBG_ASSERT(nBoundControl < m_aControlLocks.Count(), "FmXFormShell::restoreControlLocks : m_aControlLocks is invalid !");
                    xCtrl->setLock(m_aControlLocks.GetObject(nBoundControl++));
                }
            }
            continue;
        }

        DBG_ASSERT(nBoundControl < m_aControlLocks.Count(), "FmXFormShell::restoreControlLocks : m_aControlLocks is invalid !");
            // a violation of this condition would mean a) setControlLocks hasn't been called or b) the ControlContainer
            // has changed since the last call to setControlLocks.
            // a) clearly is a fault of the programmer and b) shouldn't be possible (as we are in alive mode)
        xCtrl->setLock(m_aControlLocks.GetObject(nBoundControl++));
    }
    m_aControlLocks.Remove(0, m_aControlLocks.Count());
}

//------------------------------------------------------------------------------
void FmXFormShell::DoAsyncCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& _xController, CURSOR_ACTION _eWhat)
{
    DBG_ASSERT(_xController.is(), "FmXFormShell::DoAsyncCursorAction : invalid argument !");
    DoAsyncCursorAction(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(_xController->getModel(), ::com::sun::star::uno::UNO_QUERY), _eWhat);
}

//------------------------------------------------------------------------------
void FmXFormShell::DoAsyncCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm, CURSOR_ACTION _eWhat)
{
    ::osl::MutexGuard aGuard(m_aAsyncSafety);
    DBG_ASSERT(_xForm.is(), "FmXFormShell::DoAsyncCursorAction : invalid argument !");

    // get the id of the page the form belongs to
    UniString sFormPageId = GetPageId(_xForm);
    DBG_ASSERT(sFormPageId.Len(), "FmXFormShell::DoAsyncCursorAction : could not ::std::find the page the form belongs to !");

    // build the access path for the form
    if (HasPendingCursorAction(_xForm))
    {
        DBG_ERROR("FmXFormShell::DoAsyncCursorAction : invalid call !");
        return;
    }

    UniString sPath = GetAccessPathFromForm(_xForm, sFormPageId);
    DBG_ASSERT(sPath.Len(), "FmXFormShell::DoAsyncCursorAction : could not ::std::find a page for the form !");

    CursorActionDescription& rDesc = m_aCursorActions[sPath];
        // [] will create a new one if non-existent
    DBG_ASSERT(rDesc.pThread == NULL, "FmXFormShell::DoAsyncCursorAction : the cursor action thread for this form is still alive !");

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xCursor(getInternalForm(_xForm), ::com::sun::star::uno::UNO_QUERY);
    switch (_eWhat)
    {
        case CA_MOVE_TO_LAST :
            rDesc.pThread = new FmMoveToLastThread(xCursor, sPath);
            break;
        case CA_MOVE_ABSOLUTE:
            DBG_ERROR("FmXFormShell::DoAsyncCursorAction : CA_MOVE_ABSOLUTE not supported yet !");
            return;
    }
    rDesc.pThread->SetTerminationHdl(LINK(this, FmXFormShell, OnCursorActionDone));
    rDesc.pThread->EnableSelfDelete(sal_False);

    rDesc.bCanceling = sal_False;

    // set the control locks before starting the thread
    DBG_ASSERT(getInternalForm(m_xActiveForm) == m_xActiveForm, "FmXFormShell::DoAsyncCursorAction : the active form should always be a internal one !");
    if (getInternalForm(_xForm) == m_xActiveForm)
        setControlLocks();

    // go ...
    rDesc.pThread->create();

    // set a priority slightly below normal
    rDesc.pThread->setPriority(::vos::OThread::TPriority_BelowNormal);
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::HasPendingCursorAction(const UniString& _rAccessPath) const
{
    ::osl::MutexGuard aGuard(((FmXFormShell*)this)->m_aAsyncSafety);

    for (   ConstCursorActionsIterator aIter = m_aCursorActions.begin();
            aIter != m_aCursorActions.end();
            ++aIter
        )
    {
        if (!(*aIter).second.pThread && !(*aIter).second.nFinishedEvent)
            continue;
        UniString sThisThreadPath = (*aIter).second.pThread->getAccessPath();

        // we have found a running thread. it blocks the form given by the access path if
        // one of the two forms (the thread's or the given) is an anestor of the other
        // this is the case if and only if one path is a prefix of the other
        if (PathsInterfere(sThisThreadPath, _rAccessPath))
            return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::HasPendingCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm) const
{
    if (!_xForm.is())
        return sal_False;

    return HasPendingCursorAction(GetAccessPathFromForm(_xForm, GetPageId(_xForm)));
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::HasPendingCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& xController) const
{
    if (!xController.is())
        return sal_False;

    return HasPendingCursorAction(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(xController->getModel(), ::com::sun::star::uno::UNO_QUERY));
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::HasAnyPendingCursorAction() const
{
    ::osl::MutexGuard aGuard(((FmXFormShell*)this)->m_aAsyncSafety);

    for (ConstCursorActionsIterator aIter = m_aCursorActions.begin(); aIter != m_aCursorActions.end(); ++aIter)
    {
        if (((*aIter).second.pThread != NULL) || ((*aIter).second.nFinishedEvent != 0))
            return sal_True;
    }
    return sal_False;
}

//------------------------------------------------------------------------------
void FmXFormShell::CancelAnyPendingCursorAction()
{
    ::comphelper::OReusableGuard< ::osl::Mutex> aGuard(m_aAsyncSafety);

    CursorActionsIterator aIter;
    for (aIter = m_aCursorActions.begin(); aIter != m_aCursorActions.end(); ++aIter)
    {
        if (!(*aIter).second.nFinishedEvent && (*aIter).second.pThread)
        {   // the thread is really running
            (*aIter).second.bCanceling = sal_True;

            aGuard.clear();
            (*aIter).second.pThread->StopItWait();
                // StopItWait returns after the termination handler (our OnCursorActionDone) has been called
            aGuard.attach(m_aAsyncSafety);
        }
    }

    // all threads are finished (means canceled), now do the cleanup
    for (aIter = m_aCursorActions.begin(); aIter != m_aCursorActions.end(); ++aIter)
    {
        if ((*aIter).second.pThread)
        {
            LINK(this, FmXFormShell, OnCursorActionDoneMainThread).Call((*aIter).second.pThread);
            DBG_ASSERT((*aIter).second.pThread == NULL, "FmXFormShell::CancelAnyPendingCursorAction : strange behaviour of OnCursorActionDoneMainThread !");
        }
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnCursorActionDone, FmCursorActionThread*, pThread)
{
    ::osl::MutexGuard aGuard(m_aAsyncSafety);

    // search the pos of the thread within m_aCursorActions
    CursorActionsIterator aIter;
    for (aIter = m_aCursorActions.begin(); aIter != m_aCursorActions.end(); ++aIter)
    {
        if ((*aIter).second.pThread == pThread)
            break;
    }

    DBG_ASSERT(aIter != m_aCursorActions.end(), "FmXFormShell::OnCursorActionDone : could not ::std::find the thread !");
    DBG_ASSERT((*aIter).second.nFinishedEvent == 0, "FmXFormShell::OnCursorActionDone : another 'ActionDone' for this thread is pending !");
        // as we allow only one async action at a time (per form) this shouldn't happen

    if (!(*aIter).second.bCanceling)
        (*aIter).second.nFinishedEvent = Application::PostUserEvent(LINK(this, FmXFormShell, OnCursorActionDoneMainThread), pThread);
    // if bCanceling is sal_True an other thread's execution is in CancelAnyPendingCursorAction
    return 0L;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::PathsInterfere(const UniString& _rPathLeft, const UniString& _rPathRight) const
{
    return ((_rPathLeft.Search(_rPathRight) == 0) || (_rPathRight.Search(_rPathLeft) == 0));
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnCursorActionDoneMainThread, FmCursorActionThread*, pThread)
{
    ::osl::MutexGuard aGuard(m_aAsyncSafety);

    // search the pos of the thread within m_aCursorActions
    CursorActionsIterator aIter = m_aCursorActions.find(pThread->getAccessPath());
    DBG_ASSERT(aIter != m_aCursorActions.end(), "FmXFormShell::OnCursorActionDoneMainThread : could not ::std::find the thread data !");
    DBG_ASSERT((*aIter).second.pThread == pThread, "FmXFormShell::OnCursorActionDoneMainThread : invalid thread data !");

    // remember some thread parameters
    UniString sPath = pThread->getAccessPath();
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xForm(pThread->getDataSource(), ::com::sun::star::uno::UNO_QUERY);

    // throw away the thread
    delete (*aIter).second.pThread;
    (*aIter).second.pThread = NULL;
    (*aIter).second.nFinishedEvent = 0;
    (*aIter).second.bCanceling = sal_False;
    // as we allow exactly one thread per form we may remove this ones data from m_aCursorActions
    m_aCursorActions.erase(aIter);

    // there may be several dispatchers affected (one async move locks not only the form itself, but all forms
    // which are an ancestor or a child)
    // -> update their status
    UpdateAllFormDispatchers(sPath);

    DBG_ASSERT(getInternalForm(m_xActiveForm) == m_xActiveForm, "FmXFormShell::DoAsyncCursorAction : the active form should always be a internal one !");
    DBG_ASSERT(getInternalForm(xForm) == xForm, "FmXFormShell::DoAsyncCursorAction : the thread's form should always be a internal one !");
        // if one of the two asserts above fails the upcoming comparison is senseless
    if (xForm == m_xActiveForm)
    {
        // the active controller belongs to the form where the action is finished
        // -> re-enable the controls if they are not locked because of another running thread
        if (!HasPendingCursorAction(xForm))
            restoreControlLocks();
    }

    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(DatabaseSlotMap);
        // it may not be neccessary but me thinks it's faster without the check if we really have to do this

    return 0L;
}

//------------------------------------------------------------------------------
void FmXFormShell::CreateExternalView()
{
    DBG_ASSERT(m_xAttachedFrame.is(), "FmXFormShell::CreateExternalView : no frame !");

    // the frame the external view is displayed in
    sal_Bool bAlreadyExistent = m_xExternalViewController.is();
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame> xExternalViewFrame;
    ::rtl::OUString sFrameName = ::rtl::OUString::createFromAscii("_partwindow");
    sal_Int32 nSearchFlags = ::com::sun::star::frame::FrameSearchFlag::SELF | ::com::sun::star::frame::FrameSearchFlag::PARENT | ::com::sun::star::frame::FrameSearchFlag::CREATE;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xCurrentNavController( getNavController());
        // the creation of the "partwindow" may cause a deactivate of the document which will result in our nav controller to be set to NULL

    // _first_ check if we have any valid fields we can use for the grid view
    // FS - 21.10.99 - 69219
    FmXBoundFormFieldIterator aModelIterator(xCurrentNavController->getModel());
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCurrentModelSet;
    sal_Bool bHaveUsableControls = sal_False;
    while ((xCurrentModelSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(aModelIterator.Next(), ::com::sun::star::uno::UNO_QUERY)).is())
    {
        // the FmXBoundFormFieldIterator only supplies controls with a valid control source
        // so we just have to check the field type
        sal_Int16 nClassId = ::comphelper::getINT16(xCurrentModelSet->getPropertyValue(FM_PROP_CLASSID));
        switch (nClassId)
        {
            case ::com::sun::star::form::FormComponentType::IMAGECONTROL:
            case ::com::sun::star::form::FormComponentType::CONTROL:
                continue;
        }
        bHaveUsableControls = sal_True;
        break;
    }

    if (!bHaveUsableControls)
    {
        ErrorBox(NULL, WB_OK, SVX_RESSTR(RID_STR_NOCONTROLS_FOR_EXTERNALDISPLAY)).Execute();
        return;
    }

    // load the component for external form views
    if (!bAlreadyExistent)
    {
        ::com::sun::star::util::URL aWantToDispatch;
        aWantToDispatch.Complete = FMURL_COMPONENT_FORMGRIDVIEW;

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider> xProv(m_xAttachedFrame, ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> xDisp;
        if (xProv.is())
            xDisp = xProv->queryDispatch(aWantToDispatch, sFrameName, nSearchFlags);
        if (xDisp.is())
        {
            xDisp->dispatch(aWantToDispatch, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>());
        }

        // with this the component should be loaded, now search the frame where it resides in
        xExternalViewFrame = m_xAttachedFrame->findFrame(sFrameName, nSearchFlags & ~::com::sun::star::frame::FrameSearchFlag::CREATE);
        if (xExternalViewFrame.is())
        {
            m_xExternalViewController = xExternalViewFrame->getController();
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xComp(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener((::com::sun::star::lang::XEventListener*)(::com::sun::star::beans::XPropertyChangeListener*)this);
        }
    }
    else
    {
        xExternalViewFrame = m_xExternalViewController->getFrame();
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider> xCommLink(xExternalViewFrame, ::com::sun::star::uno::UNO_QUERY);

        // if we display the active form we interpret the slot as "remove it"
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm> xCurrentModel(xCurrentNavController->getModel(), ::com::sun::star::uno::UNO_QUERY);
        if ((xCurrentModel == m_xExternalDisplayedForm) || (getInternalForm(xCurrentModel) == m_xExternalDisplayedForm))
        {
            if (m_xExternalViewController == getActiveController())
                CommitCurrent(::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY));

            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xNewController(m_xExtViewTriggerController);
            CloseExternalFormViewer();
            setActiveController(xNewController);
            return;
        }

        ::com::sun::star::util::URL aClearURL;
        aClearURL.Complete = FMURL_GRIDVIEW_CLEARVIEW;

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> xClear( xCommLink->queryDispatch(aClearURL, ::rtl::OUString::createFromAscii(""), 0));
        if (xClear.is())
            xClear->dispatch(aClearURL, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>());
    }

    // interception of slots of the external view
    if (m_pExternalViewInterceptor)
    {   // already intercepting ...
        if (m_pExternalViewInterceptor->getIntercepted() != xExternalViewFrame)
        {   // ... but another frame -> create a new interceptor
            m_pExternalViewInterceptor->dispose();
            m_pExternalViewInterceptor->release();
            m_pExternalViewInterceptor = NULL;
        }
    }

    if (!m_pExternalViewInterceptor)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterception> xSupplier(xExternalViewFrame, ::com::sun::star::uno::UNO_QUERY);
        m_pExternalViewInterceptor = new FmXDispatchInterceptorImpl(xSupplier, this, 1);
        m_pExternalViewInterceptor->acquire();
    }

    // get the dispatch interface of the frame so we can communicate (interceptable) with the controller
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider> xCommLink(xExternalViewFrame, ::com::sun::star::uno::UNO_QUERY);

    if (m_xExternalViewController.is())
    {
        DBG_ASSERT(xCommLink.is(), "FmXFormShell::CreateExternalView : the component doesn't have the necessary interfaces !");
        // collect the dispatchers we will need
        ::com::sun::star::util::URL aAddColumnURL;
        aAddColumnURL.Complete = FMURL_GRIDVIEW_ADDCOLUMN;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> xAddColumnDispatch( xCommLink->queryDispatch(aAddColumnURL, ::rtl::OUString::createFromAscii(""), 0));
        ::com::sun::star::util::URL aAttachURL;
        aAttachURL.Complete = FMURL_GRIDVIEW_ATTACHTOFORM;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch> xAttachDispatch( xCommLink->queryDispatch(aAttachURL, ::rtl::OUString::createFromAscii(""), 0));

        if (xAddColumnDispatch.is() && xAttachDispatch.is())
        {
            DBG_ASSERT(xCurrentNavController.is(), "FmXFormShell::CreateExternalView : invalid call : have no nav controller !");
            // first : dispatch the descriptions for the columns to add
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl> > aCurrentControls(xCurrentNavController->getControls());
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>* pCurrentControl = aCurrentControls.getConstArray();

            sal_Int16 nAddedColumns = 0;

            // for radio buttons we need some special structures
            DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::Sequence< ::rtl::OUString>, MapUString2UstringSeq);
            DECLARE_STL_ITERATORS(MapUString2UstringSeq);
            DECLARE_STL_USTRINGACCESS_MAP(::rtl::OUString, FmMapUString2UString);
            DECLARE_STL_USTRINGACCESS_MAP(sal_Int16, FmMapUString2Int16);
            DECLARE_STL_ITERATORS(FmMapUString2Int16);

            MapUString2UstringSeq   aRadioValueLists;
            MapUString2UstringSeq   aRadioListSources;
            FmMapUString2UString    aRadioControlSources;
            FmMapUString2Int16      aRadioPositions;

            FmXBoundFormFieldIterator aModelIterator(xCurrentNavController->getModel());
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCurrentModelSet;
            ::com::sun::star::uno::Any aCurrentBoundField;
            ::rtl::OUString sColumnType,aGroupName,sControlSource;
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps;
            while ((xCurrentModelSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>(aModelIterator.Next(), ::com::sun::star::uno::UNO_QUERY)).is())
            {
                aCurrentBoundField = xCurrentModelSet->getPropertyValue(FM_PROP_BOUNDFIELD);
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCurrentBoundField(*(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>*)aCurrentBoundField.getValue(), ::com::sun::star::uno::UNO_QUERY);

                // create a description of the column to be created
                // first : determine it's type

                sal_Int16 nClassId = ::comphelper::getINT16(xCurrentModelSet->getPropertyValue(FM_PROP_CLASSID));
                switch (nClassId)
                {
                    case ::com::sun::star::form::FormComponentType::RADIOBUTTON:
                    {
                        // get the label of the button (this is the access key for our structures)
                        aGroupName = getLabelName(xCurrentModelSet);

                        // add the reference value of the radio button to the list source sequence
                        ::com::sun::star::uno::Sequence< ::rtl::OUString>& aThisGroupLabels = aRadioListSources[aGroupName];
                        sal_Int32 nNewSizeL = aThisGroupLabels.getLength() + 1;
                        aThisGroupLabels.realloc(nNewSizeL);
                        aThisGroupLabels.getArray()[nNewSizeL - 1] = ::comphelper::getString(xCurrentModelSet->getPropertyValue(FM_PROP_REFVALUE));

                        // add the label to the value list sequence
                        ::com::sun::star::uno::Sequence< ::rtl::OUString>& aThisGroupControlSources = aRadioValueLists[aGroupName];
                        sal_Int32 nNewSizeC = aThisGroupControlSources.getLength() + 1;
                        aThisGroupControlSources.realloc(nNewSizeC);
                        aThisGroupControlSources.getArray()[nNewSizeC - 1] = ::comphelper::getString(xCurrentModelSet->getPropertyValue(FM_PROP_LABEL));

                        // remember the controls source of the radio group
                        sControlSource = ::comphelper::getString(xCurrentModelSet->getPropertyValue(FM_PROP_CONTROLSOURCE));
                        if (aRadioControlSources.find(aGroupName) == aRadioControlSources.end())
                            aRadioControlSources[aGroupName] = sControlSource;
#ifdef DBG_UTIL
                        else
                            DBG_ASSERT(aRadioControlSources[aGroupName] == sControlSource,
                            "FmXFormShell::CreateExternalView : inconsistent radio buttons detected !");
                            // (radio buttons with the same name should have the same control source)
#endif
                        // remember the position within the columns
                        if (aRadioPositions.find(aGroupName) == aRadioPositions.end())
                            aRadioPositions[aGroupName] = (sal_Int16)nAddedColumns;

                        // any further handling is done below
                        continue;
                    }
                    break;

                    case ::com::sun::star::form::FormComponentType::IMAGECONTROL:
                    case ::com::sun::star::form::FormComponentType::CONTROL:
                        // no grid columns for these types (though they have a control source)
                        continue;
                    case ::com::sun::star::form::FormComponentType::CHECKBOX:
                        sColumnType = FM_COL_CHECKBOX; break;
                    case ::com::sun::star::form::FormComponentType::LISTBOX:
                        sColumnType = FM_COL_LISTBOX; break;
                    case ::com::sun::star::form::FormComponentType::COMBOBOX:
                        sColumnType = FM_COL_COMBOBOX; break;
                    case ::com::sun::star::form::FormComponentType::DATEFIELD:
                        sColumnType = FM_COL_DATEFIELD; break;
                    case ::com::sun::star::form::FormComponentType::TIMEFIELD:
                        sColumnType = FM_COL_TIMEFIELD; break;
                    case ::com::sun::star::form::FormComponentType::NUMERICFIELD:
                        sColumnType = FM_COL_NUMERICFIELD; break;
                    case ::com::sun::star::form::FormComponentType::CURRENCYFIELD:
                        sColumnType = FM_COL_CURRENCYFIELD; break;
                    case ::com::sun::star::form::FormComponentType::PATTERNFIELD:
                        sColumnType = FM_COL_PATTERNFIELD; break;

                    case ::com::sun::star::form::FormComponentType::TEXTFIELD:
                        {
                            sColumnType = FM_COL_TEXTFIELD;
                            // we know at least two different controls which are TextFields : the basic edit field and the formatted
                            // field. we distinguish them by their service name
                            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo> xInfo(xCurrentModelSet, ::com::sun::star::uno::UNO_QUERY);
                            if (xInfo.is())
                            {
                                sal_Int16 nObjectType = getControlTypeByObject(xInfo);
                                if (OBJ_FM_FORMATTEDFIELD == nObjectType)
                                    sColumnType = FM_COL_FORMATTEDFIELD;
                            }
                        }
                        break;
                    default:
                        sColumnType = FM_COL_TEXTFIELD; break;
                }

                const sal_Int16 nDispatchArgs = 3;
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aDispatchArgs(nDispatchArgs);
                ::com::sun::star::beans::PropertyValue* pDispatchArgs = aDispatchArgs.getArray();

                // properties describing "meta data" about the column
                // the type
                pDispatchArgs->Name = FMARG_ADDCOL_COLUMNTYPE;
                pDispatchArgs->Value <<= sColumnType;
                ++pDispatchArgs;

                // the pos : append the col
                pDispatchArgs->Name = FMARG_ADDCOL_COLUMNPOS;
                pDispatchArgs->Value <<= nAddedColumns;
                ++pDispatchArgs;

                // the properties to forward to the new column
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aColumnProps(1);
                ::com::sun::star::beans::PropertyValue* pColumnProps = aColumnProps.getArray();

                // the label
                pColumnProps->Name = FM_PROP_LABEL;
                pColumnProps->Value <<= getLabelName(xCurrentModelSet);
                ++pColumnProps;

                // for all other props : transfer them
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> xControlModelInfo( xCurrentModelSet->getPropertySetInfo());
                DBG_ASSERT(xControlModelInfo.is(), "FmXFormShell::CreateExternalView : the control model has no property info ! This will crash !");
                aProps = xControlModelInfo->getProperties();
                const ::com::sun::star::beans::Property* pProps = aProps.getConstArray();

                // realloc the control description sequence
                sal_Int32 nExistentDescs = pColumnProps - aColumnProps.getArray();
                aColumnProps.realloc(nExistentDescs + aProps.getLength());
                pColumnProps = aColumnProps.getArray() + nExistentDescs;

                for (sal_Int32 i=0; i<aProps.getLength(); ++i, ++pProps)
                {
                    if (pProps->Name.equals(FM_PROP_LABEL))
                        // already set
                        continue;
                    if (pProps->Name.equals(FM_PROP_DEFAULTCONTROL))
                        // allow the column's own "default control"
                        continue;
                    if (pProps->Attributes & ::com::sun::star::beans::PropertyAttribute::READONLY)
                        // assume that properties which are readonly for the control are ro for the column to be created, too
                        continue;

                    pColumnProps->Name = pProps->Name;
                    pColumnProps->Value = xCurrentModelSet->getPropertyValue(pProps->Name);
                    ++pColumnProps;
                }
                aColumnProps.realloc(pColumnProps - aColumnProps.getArray());

                // columns props are a dispatch argument
                pDispatchArgs->Name = ::rtl::OUString::createFromAscii("ColumnProperties"); // TODO : fmurl.*
                pDispatchArgs->Value = ::com::sun::star::uno::makeAny(aColumnProps);
                ++pDispatchArgs;
                DBG_ASSERT(nDispatchArgs == (pDispatchArgs - aDispatchArgs.getConstArray()),
                    "FmXFormShell::CreateExternalView : forgot to adjust nDispatchArgs ?");

                // dispatch the "add column"
                xAddColumnDispatch->dispatch(aAddColumnURL, aDispatchArgs);
                ++nAddedColumns;
            }

            // now for the radio button handling
            sal_Int16 nOffset(0);
            // properties describing the "direct" column properties
            const sal_Int16 nListBoxDescription = 6;
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aListBoxDescription(nListBoxDescription);
            for (   ConstFmMapUString2UStringIterator aCtrlSource = aRadioControlSources.begin();
                    aCtrlSource != aRadioControlSources.end();
                    ++aCtrlSource, ++nOffset
                )
            {

                ::com::sun::star::beans::PropertyValue* pListBoxDescription = aListBoxDescription.getArray();
                // label
                pListBoxDescription->Name = FM_PROP_LABEL;
                pListBoxDescription->Value <<= (*aCtrlSource).first;
                ++pListBoxDescription;

                // control source
                pListBoxDescription->Name = FM_PROP_CONTROLSOURCE;
                pListBoxDescription->Value <<= (*aCtrlSource).second;
                ++pListBoxDescription;

                // bound column
                pListBoxDescription->Name = FM_PROP_BOUNDCOLUMN;
                pListBoxDescription->Value <<= (sal_Int16)1;
                ++pListBoxDescription;

                // content type
                pListBoxDescription->Name = FM_PROP_LISTSOURCETYPE;
                 ::com::sun::star::form::ListSourceType eType = ::com::sun::star::form::ListSourceType_VALUELIST;
                 pListBoxDescription->Value = ::com::sun::star::uno::makeAny(eType);
                ++pListBoxDescription;

                // list source
                MapUString2UstringSeq::const_iterator aCurrentListSource = aRadioListSources.find((*aCtrlSource).first);
                DBG_ASSERT(aCurrentListSource != aRadioListSources.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                pListBoxDescription->Name = FM_PROP_LISTSOURCE;
                pListBoxDescription->Value = ::com::sun::star::uno::makeAny((*aCurrentListSource).second);
                ++pListBoxDescription;

                // value list
                MapUString2UstringSeq::const_iterator aCurrentValueList = aRadioValueLists.find((*aCtrlSource).first);
                DBG_ASSERT(aCurrentValueList != aRadioValueLists.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                pListBoxDescription->Name = FM_PROP_STRINGITEMLIST;
                pListBoxDescription->Value = ::com::sun::star::uno::makeAny(((*aCurrentValueList).second));
                ++pListBoxDescription;

                DBG_ASSERT(nListBoxDescription == (pListBoxDescription - aListBoxDescription.getConstArray()),
                    "FmXFormShell::CreateExternalView : forgot to adjust nListBoxDescription ?");

                // properties describing the column "meta data"
                const sal_Int16 nDispatchArgs = 3;
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aDispatchArgs(nDispatchArgs);
                ::com::sun::star::beans::PropertyValue* pDispatchArgs = aDispatchArgs.getArray();

                // column type : listbox
                pDispatchArgs->Name = FMARG_ADDCOL_COLUMNTYPE;
                ::rtl::OUString fColName = FM_COL_LISTBOX;
                pDispatchArgs->Value <<= fColName;
//              pDispatchArgs->Value <<= (::rtl::OUString)FM_COL_LISTBOX;
                ++pDispatchArgs;

                // column position
                pDispatchArgs->Name = FMARG_ADDCOL_COLUMNPOS;
                FmMapUString2Int16::const_iterator aOffset = aRadioPositions.find((*aCtrlSource).first);
                DBG_ASSERT(aOffset != aRadioPositions.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                sal_Int16 nPosition = (*aOffset).second;
                nPosition += nOffset;
                    // we alread inserted nOffset additinal columns ....
                pDispatchArgs->Value <<= nPosition;
                ++pDispatchArgs;

                // the
                pDispatchArgs->Name = ::rtl::OUString::createFromAscii("ColumnProperties"); // TODO : fmurl.*
                pDispatchArgs->Value = ::com::sun::star::uno::makeAny(aListBoxDescription);
                ++pDispatchArgs;
                DBG_ASSERT(nDispatchArgs == (pDispatchArgs - aDispatchArgs.getConstArray()),
                    "FmXFormShell::CreateExternalView : forgot to adjust nDispatchArgs ?");

                // dispatch the "add column"
                xAddColumnDispatch->dispatch(aAddColumnURL, aDispatchArgs);
                ++nAddedColumns;
            }


            DBG_ASSERT(nAddedColumns > 0, "FmXFormShell::CreateExternalView : no controls (inconsistent) !");
                // we should have checked if we have any usable controls (see above).

            // "load" the "form" of the external view
            ::com::sun::star::beans::PropertyValue aArg;
            aArg.Name = FMARG_ATTACHTO_MASTERFORM;
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> xForm(xCurrentNavController->getModel(), ::com::sun::star::uno::UNO_QUERY);
            aArg.Value <<= xForm;

            m_xExternalDisplayedForm = ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>(xForm, ::com::sun::star::uno::UNO_QUERY);
                // do this before dispatching the "attach" command, as the atach may result in a call to our queryDispatch (for the FormSlots)
                // whichs needs the m_xExternalDisplayedForm

            xAttachDispatch->dispatch(aAttachURL, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>(&aArg, 1));

            m_xExtViewTriggerController = xCurrentNavController;

            // we want to know modifications done in the external view
            // if the external controller is a ::com::sun::star::form::XFormController we can use all our default handlings for it
            ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController> xFormController(m_xExternalViewController, ::com::sun::star::uno::UNO_QUERY);
            if (xFormController.is())
                xFormController->addActivateListener((::com::sun::star::form::XFormControllerListener*)this);
        }
    }
#ifdef DBG_UTIL
    else
    {
        DBG_ERROR("FmXFormShell::CreateExternalView : could not create the external form view !");
    }
#endif
    InvalidateSlot(SID_FM_VIEW_AS_GRID, sal_True, sal_False);
}

//==============================================================================
//==============================================================================
SearchableControlIterator::SearchableControlIterator(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xStartingPoint)
    :IndexAccessIterator(xStartingPoint)
{
}

//------------------------------------------------------------------------------
sal_Bool SearchableControlIterator::ShouldHandleElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement)
{
    // wenn das Ding eine ControlSource und einen BoundField-::com::sun::star::beans::Property hat
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xProperties(xElement, ::com::sun::star::uno::UNO_QUERY);
    if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xProperties) && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xProperties))
    {
        // und das BoundField gueltig ist
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xField( *(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)(xProperties->getPropertyValue(FM_PROP_BOUNDFIELD).getValue()));
        if (xField.is())
        {
            // nehmen wir's
            m_sCurrentValue = ::comphelper::getString(xProperties->getPropertyValue(FM_PROP_CONTROLSOURCE));
            return sal_True;
        }
    }

    // wenn es ein Grid-Control ist
    if (::comphelper::hasProperty(FM_PROP_CLASSID, xProperties))
    {
        ::com::sun::star::uno::Any aClassId( xProperties->getPropertyValue(FM_PROP_CLASSID) );
        if (::comphelper::getINT16(aClassId) == ::com::sun::star::form::FormComponentType::GRIDCONTROL)
        {
            m_sCurrentValue = ::rtl::OUString();
            return sal_True;
        }
    }

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool SearchableControlIterator::ShouldStepInto(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xContainer) const
{
    return sal_True;
}

//==============================================================================
//==============================================================================
SV_IMPL_PTRARR(StatusForwarderArray, SfxStatusForwarder*)

SFX_IMPL_MENU_CONTROL(ControlConversionMenuController, SfxBoolItem);

//------------------------------------------------------------------------------
ControlConversionMenuController::ControlConversionMenuController(sal_uInt16 nId, Menu &rMenu, SfxBindings &rBindings)
    :SfxMenuControl(nId, rBindings)
    ,m_pMainMenu(&rMenu)
    ,m_pConversionMenu(NULL)
{
    if (nId == SID_FM_CHANGECONTROLTYPE)
    {
        m_pConversionMenu = FmXFormShell::GetConversionMenu();
        rMenu.SetPopupMenu(nId, m_pConversionMenu);

        for (sal_Int16 i=0; i<m_pConversionMenu->GetItemCount(); ++i)
        {
            rBindings.Invalidate(m_pConversionMenu->GetItemId(i));
            SfxStatusForwarder* pForwarder = new SfxStatusForwarder(m_pConversionMenu->GetItemId(i), *this);
            m_aStatusForwarders.C40_INSERT(SfxStatusForwarder, pForwarder, m_aStatusForwarders.Count());
        }
    }
}

//------------------------------------------------------------------------------
ControlConversionMenuController::~ControlConversionMenuController()
{
    m_pMainMenu->SetPopupMenu(SID_FM_CHANGECONTROLTYPE, NULL);
    delete m_pConversionMenu;
}

//------------------------------------------------------------------------------
void ControlConversionMenuController::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    if (nSID == GetId())
        SfxMenuControl::StateChanged(nSID, eState, pState);
    else if (FmXFormShell::IsControlConversionSlot(nSID))
    {
        if ((m_pConversionMenu->GetItemPos(nSID) != MENU_ITEM_NOTFOUND) && (eState == SFX_ITEM_DISABLED))
        {
            m_pConversionMenu->RemoveItem(m_pConversionMenu->GetItemPos(nSID));
        }
        else if ((m_pConversionMenu->GetItemPos(nSID) == MENU_ITEM_NOTFOUND) && (eState != SFX_ITEM_DISABLED))
        {
            // We can't simply re-insert the item because we have a clear order for all the our items.
            // So first we have to determine the position of the item to insert.
            PopupMenu* pSource = FmXFormShell::GetConversionMenu();
            sal_Int16 nSourcePos = pSource->GetItemPos(nSID);
            DBG_ASSERT(nSourcePos != MENU_ITEM_NOTFOUND, "ControlConversionMenuController::StateChanged : FmXFormShell supplied an invalid menu !");
            sal_Int16 nPrevInSource = nSourcePos;
            sal_Int16 nPrevInConversion = MENU_ITEM_NOTFOUND;
            while (nPrevInSource>0)
            {
                sal_Int16 nPrevId = pSource->GetItemId(--nPrevInSource);

                // do we have the source's predecessor in our conversion menu, too ?
                nPrevInConversion = m_pConversionMenu->GetItemPos(nPrevId);
                if (nPrevInConversion != MENU_ITEM_NOTFOUND)
                    break;
            }
            if (MENU_ITEM_NOTFOUND == nPrevInConversion)
                // none of the items which precede the nSID-slot in the source menu are present in our conversion menu
                nPrevInConversion = 0 - 1;  // put the item at the first position
            m_pConversionMenu->InsertItem(nSID, pSource->GetItemText(nSID), pSource->GetItemBits(nSID), ++nPrevInConversion);
            m_pConversionMenu->SetItemImage(nSID, pSource->GetItemImage(nSID));
            m_pConversionMenu->SetHelpId(nSID, pSource->GetHelpId(nSID));

            delete pSource;
        }
        m_pMainMenu->EnableItem(SID_FM_CHANGECONTROLTYPE, m_pConversionMenu->GetItemCount() > 0);
    }
    else
    {
        DBG_ERROR("ControlConversionMenuController::StateChanged : unknown id !");
    }
}

//==================================================================

FmCursorActionThread::FmCursorActionThread(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDataSource, const UniString& _rStopperCaption,
                                           const UniString& _rPath)
    :m_xDataSource(_xDataSource)
    ,m_sStopperCaption(_rStopperCaption)
    ,m_bCanceled(sal_False)
    ,m_bDeleteMyself(sal_False)
    ,m_bDisposeCursor(sal_False)
    ,m_bTerminated(sal_False)
    ,m_bRunFailed(sal_False)
    ,m_sAccessPath(_rPath)
{
    DBG_ASSERT(m_xDataSource.is() && ::com::sun::star::uno::Reference< ::com::sun::star::util::XCancellable>(m_xDataSource, ::com::sun::star::uno::UNO_QUERY).is(),
        "FmCursorActionThread::FmCursorActionThread : invalid cursor !");
}

//------------------------------------------------------------------------------
void FmCursorActionThread::run()
{
    // On instantiation of a SfxCancellable the application is notified and 'switches on' the red stop button.
    // Unfortunally this is conditioned with the acquirement of the solar mutex, and the application tries
    // only once and ignores the notification if it fails.
    // To prevent that we get the solar mutex and _block_ 'til we got it.
    // As we are in the 'top level execution' of this thread (with a rather small stack and no other mutexes locked)
    // we shouldn't experience problems with deadlocks ...
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());
    ThreadStopper* pStopper = new ThreadStopper(this, m_sStopperCaption);
    aSolarGuard.clear();

    // we're not canceled yet
    ::osl::ClearableMutexGuard aInitGuard(m_aAccessSafety);
    m_bCanceled = sal_False;
    m_bRunFailed = sal_False;
    aInitGuard.clear();

    // start it
    try
    {
        RunImpl();
    }
    catch(::com::sun::star::sdbc::SQLException e)
    {
        ::osl::MutexGuard aErrorGuard(m_aAccessSafety);
        m_bRunFailed = sal_True;
        m_aRunException = e;
    }
    catch(...)
    {
        DBG_ERROR("FmCursorActionThread::run : catched a non-database exception !");
    }


    sal_Bool bReallyCanceled;
    ::osl::ClearableMutexGuard aEvalGuard(m_aAccessSafety);
    bReallyCanceled = m_bCanceled;
    aEvalGuard.clear();

    pStopper->OwnerTerminated();
        // this will cause the stopper to delete itself (in the main thread) so we don't have to care of the
        // solar mutex
}

//------------------------------------------------------------------------------
void FmCursorActionThread::onTerminated()
{
    ::osl::ClearableMutexGuard aResetGuard(m_aAccessSafety);
    if (m_aTerminationHandler.IsSet())
        m_aTerminationHandler.Call(this);

    if (IsCursorDisposeEnabled())
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xDataSourceComponent(m_xDataSource, ::com::sun::star::uno::UNO_QUERY);
        if (xDataSourceComponent.is())
            xDataSourceComponent->dispose();
    }

    aResetGuard.clear();
        // with this StopItWait has a chance to do it's Terminated()

    ::osl::MutexGuard aGuard(m_aFinalExitControl);
        // Terminated() in StopItWait still returns sal_False
    m_bTerminated = sal_True;
        // Terminated() now returns sal_True, but StopItWait can't exit until this method exits (because of the guarded m_aFinalExitControl)

    if (IsSelfDeleteEnabled())
        delete this;
}

//------------------------------------------------------------------------------
void FmCursorActionThread::StopIt()
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    m_bCanceled = sal_True;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCancellable> xCancel(m_xDataSource, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xCancel.is(), "FmCursorActionThread::StopIt : invalid cursor !");
    xCancel->cancel();
}

//------------------------------------------------------------------------------
void FmCursorActionThread::StopItWait()
{
    StopIt();

    while (!Terminated())
        ;

    // OnTerminated has been called, but we can't simply exit : Suppose the caller want's to delete the thread object
    // immediately after returning from StopItWait. With the following guard we ensure that we exit this method
    // only after onTerminated has exited.
    ::osl::ClearableMutexGuard aGuard(m_aFinalExitControl);

    // now onTerminated has exited, so we can leave, too
}

//------------------------------------------------------------------------------
FmCursorActionThread::ThreadStopper::ThreadStopper(FmCursorActionThread* pOwner, const UniString& rTitle)
    :SfxCancellable(SFX_APP()->GetCancelManager(), rTitle)
    ,m_pOwner(pOwner)
{
}

//------------------------------------------------------------------------------
void FmCursorActionThread::ThreadStopper::Cancel()
{
    if (!m_pOwner)      // the owner is already terminated and we're waiting for the OnDeleteInMainThread event
        return;

    ::osl::MutexGuard aGuard(m_pOwner->m_aAccessSafety);
    if (IsCancelled())
        // we already did pass this to our owner
        return;

    SfxCancellable::Cancel();
    m_pOwner->StopIt();
}

//------------------------------------------------------------------------------
void FmCursorActionThread::ThreadStopper::OwnerTerminated()
{
    m_pOwner = NULL;
    Application::PostUserEvent(LINK(this, FmCursorActionThread::ThreadStopper, OnDeleteInMainThread), this);
}

//------------------------------------------------------------------------------
IMPL_LINK(FmCursorActionThread::ThreadStopper, OnDeleteInMainThread, FmCursorActionThread::ThreadStopper*, pThis)
{
    delete pThis;
    return 0L;
}

//==============================================================================
//==============================================================================

//------------------------------------------------------------------------------
FmFormNavigationDispatcher::FmFormNavigationDispatcher(const ::com::sun::star::util::URL& _rUrl, sal_Int16 _nSlotId, SfxBindings& _rBindings, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm, const UniString& _rAccessPath)
        :FmSlotDispatch(_rUrl, _nSlotId, _rBindings)
        ,m_bActive(sal_False)
        ,m_xCursor(_xForm)
        ,m_sAccessPath(_rAccessPath)
{
}

//------------------------------------------------------------------------------
void FmFormNavigationDispatcher::SetActive(sal_Bool bEnable)
{
    if (m_bActive == bEnable)
        return;

    m_bActive = bEnable;

    // broadcast the initial state
    if (bEnable)
    {
        SfxPoolItem* pState = NULL;
        SfxItemState eInitialState = GetBindings().QueryState(m_nSlot, pState);
        NotifyState(eInitialState, pState);
    }
}

//------------------------------------------------------------------------------
void FmFormNavigationDispatcher::SetStatus(SfxItemState eState, const SfxPoolItem* pState)
{
    m_aNonActiveState = BuildEvent(eState, pState);
    NOTIFY_LISTENERS(m_aStatusListeners, ::com::sun::star::frame::XStatusListener, statusChanged, m_aNonActiveState);
}

//------------------------------------------------------------------------------
void FmFormNavigationDispatcher::StateChanged(sal_Int16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    if (IsActive())
        FmSlotDispatch::StateChanged(nSID, eState, pState);
}

//------------------------------------------------------------------------------
void FmFormNavigationDispatcher::NotifyState(SfxItemState eState, const SfxPoolItem* pState, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener>& rListener)
{
    if (IsActive())
        FmSlotDispatch::NotifyState(eState, pState, rListener);
    else
        if (rListener.is())
            rListener->statusChanged(m_aNonActiveState);
        else
            NOTIFY_LISTENERS(m_aStatusListeners, ::com::sun::star::frame::XStatusListener, statusChanged, m_aNonActiveState);
}

//------------------------------------------------------------------------------
void FmFormNavigationDispatcher::dispose(void) throw( ::com::sun::star::uno::RuntimeException )
{
    m_aNonActiveState.Source = NULL;
        // the Source is a reference to myself, so we would never be deleted without this
    FmSlotDispatch::dispose();
}

//==============================================================================




