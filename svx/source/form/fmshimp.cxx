/*************************************************************************
 *
 *  $RCSfile: fmshimp.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:36:51 $
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

#ifndef SVX_SOURCE_INC_FMTEXTCONTROLSHELL_HXX
#include "fmtextcontrolshell.hxx"
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
#ifndef _COM_SUN_STAR_FORM_BINDING_XBINDABLEVALUE_HPP_
#include <com/sun/star/form/binding/XBindableValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XLISTENTRYSINK_HPP_
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif

#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
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
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
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
#ifndef SVX_FORMCONTROLLING_HXX
#include "formcontrolling.hxx"
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#include "svxdlg.hxx" //CHINA001
#include "dialogs.hrc" //CHINA001

#include <algorithm>

// wird fuer Invalidate verwendet -> mitpflegen
sal_uInt16 DatabaseSlotMap[] =
{
    SID_FM_RECORD_FIRST,
    SID_FM_RECORD_NEXT,
    SID_FM_RECORD_PREV,
    SID_FM_RECORD_LAST,
    SID_FM_RECORD_NEW,
    SID_FM_RECORD_DELETE,
    SID_FM_RECORD_ABSOLUTE,
    SID_FM_RECORD_TOTAL,
    SID_FM_RECORD_SAVE,
    SID_FM_RECORD_UNDO,
    SID_FM_REMOVE_FILTER_SORT,
    SID_FM_SORTUP,
    SID_FM_SORTDOWN,
    SID_FM_ORDERCRIT,
    SID_FM_AUTOFILTER,
    SID_FM_FORM_FILTERED,
    SID_FM_REFRESH,
    SID_FM_SEARCH,
    SID_FM_FILTER_START,
    SID_FM_VIEW_AS_GRID,
    0
};

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
    SID_FM_CONVERTTO_IMAGEBUTTON,
    SID_FM_CONVERTTO_FILECONTROL,
    SID_FM_CONVERTTO_DATE,
    SID_FM_CONVERTTO_TIME,
    SID_FM_CONVERTTO_NUMERIC,
    SID_FM_CONVERTTO_CURRENCY,
    SID_FM_CONVERTTO_PATTERN,
    SID_FM_CONVERTTO_IMAGECONTROL,
    SID_FM_CONVERTTO_FORMATTED,
    SID_FM_CONVERTTO_SCROLLBAR,
    SID_FM_CONVERTTO_SPINBUTTON,
    SID_FM_CONVERTTO_NAVIGATIONBAR,

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
    SID_FM_CONVERTTO_IMAGEBUTTON,
    SID_FM_CONVERTTO_FILECONTROL,
    SID_FM_CONVERTTO_DATE,
    SID_FM_CONVERTTO_TIME,
    SID_FM_CONVERTTO_NUMERIC,
    SID_FM_CONVERTTO_CURRENCY,
    SID_FM_CONVERTTO_PATTERN,
    SID_FM_CONVERTTO_IMAGECONTROL,
    SID_FM_CONVERTTO_FORMATTED,
    SID_FM_CONVERTTO_SCROLLBAR,
    SID_FM_CONVERTTO_SPINBUTTON,
    SID_FM_CONVERTTO_NAVIGATIONBAR
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
    SID_FM_IMAGEBUTTON,
    SID_FM_FILECONTROL,
    SID_FM_DATEFIELD,
    SID_FM_TIMEFIELD,
    SID_FM_NUMERICFIELD,
    SID_FM_CURRENCYFIELD,
    SID_FM_PATTERNFIELD,
    SID_FM_IMAGECONTROL,
    SID_FM_FORMATTEDFIELD,
    SID_FM_SCROLLBAR,
    SID_FM_SPINBUTTON,
    SID_FM_NAVIGATIONBAR
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
    OBJ_FM_IMAGEBUTTON,
    OBJ_FM_FILECONTROL,
    OBJ_FM_DATEFIELD,
    OBJ_FM_TIMEFIELD,
    OBJ_FM_NUMERICFIELD,
    OBJ_FM_CURRENCYFIELD,
    OBJ_FM_PATTERNFIELD,
    OBJ_FM_IMAGECONTROL,
    OBJ_FM_FORMATTEDFIELD,
    OBJ_FM_SCROLLBAR,
    OBJ_FM_SPINBUTTON,
    OBJ_FM_NAVIGATIONBAR
};

using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::form::binding;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::svxform;
using namespace ::svx;

//------------------------------------------------------------------------------
sal_Bool FmXBoundFormFieldIterator::ShouldStepInto(const Reference< XInterface>& _rContainer) const
{
    if (_rContainer == m_xStartingPoint)
        // would be quite stupid to step over the root ....
        return sal_True;

    return Reference< XControlModel>(_rContainer, UNO_QUERY).is();
}

//------------------------------------------------------------------------------
sal_Bool FmXBoundFormFieldIterator::ShouldHandleElement(const Reference< XInterface>& _rElement)
{
    if (!_rElement.is())
        // NULL element
        return sal_False;

    if (Reference< XForm>(_rElement, UNO_QUERY).is() || Reference< XGrid>(_rElement, UNO_QUERY).is())
        // a forms or a grid
        return sal_False;

    Reference< XPropertySet> xSet(_rElement, UNO_QUERY);
    if (!xSet.is() || !::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
        // no "BoundField" property
        return sal_False;

    Any aVal( xSet->getPropertyValue(FM_PROP_BOUNDFIELD) );
    if (aVal.getValueTypeClass() != TypeClass_INTERFACE)
        // void or invalid property value
        return sal_False;

    return aVal.hasValue();
}

//==============================================================================

DECL_CURSOR_ACTION_THREAD(FmMoveToLastThread);
IMPL_CURSOR_ACTION_THREAD(FmMoveToLastThread, SVX_RES(RID_STR_MOVING_CURSOR), last());

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
Reference< XForm> GetForm(const Reference< XInterface>& xElement)
{
    Reference< XChild> xChild(xElement, UNO_QUERY);
    Reference< XInterface> xParent;
    if (xChild.is())
        xParent = xChild->getParent();

    Reference< XForm> xForm(xParent, UNO_QUERY);
    if (!xForm.is() && xParent.is())
        xForm = GetForm(xParent);

    return xForm;
}

//------------------------------------------------------------------------
Reference< XForm> GetForm(const FmFormObj* pObj)
{
    Reference< XForm> xForm;
    if (pObj)
    {
        Reference< XFormComponent> xFormComponent(pObj->GetUnoControlModel(), UNO_QUERY);
        if (xFormComponent.is())
            xForm = Reference< XForm>(xFormComponent->getParent(), UNO_QUERY);
    }
    return xForm;
}

//------------------------------------------------------------------------
Reference< XForm> FmXFormShell::DetermineCurForm(const SdrMarkList& rMarkList, sal_Bool& bMixedWithFormControls)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    // gibt es eine aktuelle Form, wird ermittelt aus den selektierten Objekten
    Reference< XForm> xReferenceForm;
    bMixedWithFormControls = sal_False;
    sal_uInt32 nMarkCount = rMarkList.GetMarkCount();

    sal_Bool bHasFormControls = sal_False;
    sal_Bool bHasForeignControls = sal_False;

    Reference< XForm> xForm;
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
            else if (xForm.is() && (XForm*)xReferenceForm.get() != (XForm*)xForm.get())
            {
                // mehr als eine Form selektiert
                bMixedWithFormControls = sal_True;
                return Reference< XForm>();
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
                    else if (xForm.is() && (XForm*)xReferenceForm.get() != (XForm*)xForm.get())
                    {
                        // mehr als eine Form selektiert
                        bMixedWithFormControls = sal_True;
                        return Reference< XForm>();
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
// class FmXFormShell_Base_Disambiguation
//========================================================================
FmXFormShell_Base_Disambiguation::FmXFormShell_Base_Disambiguation( ::osl::Mutex& _rMutex )
    :FmXFormShell_BD_BASE( _rMutex )
{
}

void SAL_CALL FmXFormShell_Base_Disambiguation::disposing()
{
    WeakComponentImplHelperBase::disposing();
    // Note:
    // This is a HACK.
    // Normally it should be sufficient to call the "disposing" of our direct
    // base class, but SUN PRO 5 does not like this and claims there is a conflict
    // with the XEventListener::disposing(EventObject) of our various listener
    // base classes.
}

//========================================================================
// class FmXFormShell
//========================================================================
DBG_NAME(FmXFormShell);
//------------------------------------------------------------------------
FmXFormShell::FmXFormShell( FmFormShell* _pShell, SfxViewFrame* _pViewFrame )
        :FmXFormShell_BASE(m_aMutex)
        ,FmXFormShell_CFGBASE(::rtl::OUString::createFromAscii("Office.Common/Misc"), CONFIG_MODE_DELAYED_UPDATE)
        ,m_aActiveControllerFeatures( ::comphelper::getProcessServiceFactory(), this )
        ,m_aNavControllerFeatures( ::comphelper::getProcessServiceFactory(), this )
        ,m_pShell(_pShell)
        ,m_bDatabaseBar(sal_False)
        ,m_eNavigate(NavigationBarMode_NONE)
        ,m_bTrackProperties(sal_True)
        ,m_bInActivate(sal_False)
        ,m_bSetFocus(sal_False)
        ,m_nLockSlotInvalidation(0)
        ,m_nInvalidationEvent(0)
        ,m_bFilterMode(sal_False)
        ,m_bHadPropBrw(sal_False)
        ,m_pExternalViewInterceptor(NULL)
        ,m_bChangingDesignMode(sal_False)
        ,m_bUseWizards(sal_True)
        ,m_bPreparedClose( sal_False )
        ,m_pTextShell( new ::svx::FmTextControlShell( _pViewFrame ) )
{
    DBG_CTOR(FmXFormShell,NULL);
    m_aMarkTimer.SetTimeout(100);
    m_aMarkTimer.SetTimeoutHdl(LINK(this,FmXFormShell,OnTimeOut));

    // we are a DispatchInterceptor, so we want to be inserted into the frame's dispatch chain, thus having
    // a chance for frame-spanning communication (via UNO, not slots)
    SfxFrame* pFrame = _pViewFrame ? _pViewFrame->GetFrame() : NULL;
    Reference< ::com::sun::star::frame::XFrame> xUnoFrame;
    if( pFrame)
        xUnoFrame = pFrame->GetFrameInterface();
    else
        xUnoFrame = Reference< ::com::sun::star::frame::XFrame>(NULL);

    // to prevent deletion of this we acquire our refcounter once
    ::comphelper::increment(FmXFormShell_BASE::m_refCount);

    // dispatch interception for the frame
    Reference< ::com::sun::star::frame::XDispatchProviderInterception> xSupplier(xUnoFrame, UNO_QUERY);

    m_xAttachedFrame = xUnoFrame;

    // correct the refcounter
    ::comphelper::decrement(FmXFormShell_BASE::m_refCount);

    // cache the current configuration settings we're interested in
    implAdjustConfigCache();
    // and register for changes on this settings
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii("FormControlPilotsEnabled");
    EnableNotification(aNames);
}

//------------------------------------------------------------------------
FmXFormShell::~FmXFormShell()
{
    delete m_pTextShell;
    DBG_DTOR(FmXFormShell,NULL);
}

//------------------------------------------------------------------
Any SAL_CALL FmXFormShell::queryInterface( const Type& type) throw ( RuntimeException )
{
    return FmXFormShell_BASE::queryInterface(type);
}
//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL FmXFormShell::getTypes(  ) throw(RuntimeException)
{
    return FmXFormShell_BASE::getTypes();
}
//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL FmXFormShell::getImplementationId() throw(RuntimeException)
{
    static ::cppu::OImplementationId* pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
//  EventListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::disposing(const EventObject& e) throw( RuntimeException )
{
    if (m_xActiveController == e.Source)
    {
        // wird der Controller freigeben dann alles loslassen
        stopListening();
        m_xActiveForm = NULL;
        m_xActiveController = NULL;
        m_xNavigationController = NULL;

        m_aActiveControllerFeatures.dispose();
        m_aNavControllerFeatures.dispose();

        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
    }

    if (e.Source == m_xExternalViewController)
    {
        Reference< XFormController> xFormController(m_xExternalViewController, UNO_QUERY);
        if (xFormController.is())
            xFormController->removeActivateListener((XFormControllerListener*)this);

        Reference< ::com::sun::star::lang::XComponent> xComp(m_xExternalViewController, UNO_QUERY);
        if (xComp.is())
            xComp->removeEventListener((XEventListener*)(XPropertyChangeListener*)this);

        m_xExternalViewController = NULL;
        m_xExternalDisplayedForm = NULL;
        m_xExtViewTriggerController = NULL;

        InvalidateSlot(SID_FM_VIEW_AS_GRID, sal_True, sal_False);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::propertyChange(const PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (evt.PropertyName == FM_PROP_ROWCOUNT)
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

    // this may be called from a non-main-thread so invalidate the shell asynchronously
    LockSlotInvalidation(sal_True);
    InvalidateSlot(0, 0, 0);        // special meaning : invalidate m_pShell
    LockSlotInvalidation(sal_False);
}

//------------------------------------------------------------------------------
void FmXFormShell::invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures )
{
    OSL_ENSURE( _rFeatures.size() > 0, "FmXFormShell::invalidateFeatures: invalid arguments!" );

    if ( m_pShell && m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame() )
    {
        // unfortunately, SFX requires sal_uInt16
        ::std::vector< sal_uInt16 > aSlotIds;
        aSlotIds.reserve( _rFeatures.size() );
        ::std::copy( _rFeatures.begin(),
            _rFeatures.end(),
            insert_iterator< ::std::vector< sal_uInt16 > >( aSlotIds, aSlotIds.begin() )
        );

        // furthermore, SFX wants a terminating 0
        aSlotIds.push_back( 0 );

        // and, last but not least, SFX wants the ids to be sorted
        ::std::sort( aSlotIds.begin(), aSlotIds.end() - 1 );

        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( aSlotIds.begin() );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::formActivated(const EventObject& rEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");

    Reference< XFormController > xController( rEvent.Source, UNO_QUERY );
    OSL_ENSURE( xController.is(), "FmXFormShell::formActivated: invalid event source!" );

    setActiveController( xController );
    m_pTextShell->formActivated( xController );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::formDeactivated(const EventObject& rEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");

    Reference< XFormController > xController( rEvent.Source, UNO_QUERY );
    OSL_ENSURE( xController.is(), "FmXFormShell::formDeactivated: invalid event source!" );

    m_pTextShell->formDeactivated( xController );
}

//------------------------------------------------------------------------------
void FmXFormShell::disposing()
{
    FmXFormShell_BASE::disposing();

    if ( m_pShell && !m_pShell->IsDesignMode() )
        setActiveController( NULL, sal_True );
        // do NOT save the content of the old form (the second parameter tells this)
        // if we're here, then we expect that PrepareClose has been called, and thus the user
        // got a chance to commit or reject any changes. So in case we're here and there
        // are still uncommitted changes, the user explicitly wanted this.
        // 2002-11-11 - 104702 - fs@openoffice.org

    // dispose our interceptor helpers
    if (m_pExternalViewInterceptor)
    {
        m_pExternalViewInterceptor->dispose();
        m_pExternalViewInterceptor->release();
        m_pExternalViewInterceptor = NULL;
    }

    m_pTextShell->dispose();

    m_xAttachedFrame = NULL;

    CloseExternalFormViewer();

    while ( m_aLoadingPages.size() )
    {
        Application::RemoveUserEvent( m_aLoadingPages.front().nEventId );
        m_aLoadingPages.pop();
    }

    {
        ::osl::MutexGuard aGuard(m_aInvalidationSafety);
        if (m_nInvalidationEvent)
        {
            Application::RemoveUserEvent(m_nInvalidationEvent);
            m_nInvalidationEvent = 0;
        }
    }

    {
        ::osl::ClearableMutexGuard aGuard(m_aAsyncSafety);
        if (HasAnyPendingCursorAction())
            CancelAnyPendingCursorAction();
        aGuard.clear();

        DBG_ASSERT(!m_nInvalidationEvent, "FmXFormShell::~FmXFormShell : still have an invalidation event !");
            // should habe been deleted while beeing disposed

        m_aMarkTimer.Stop();
    }

    // we are disposed from within the destructor of our shell, so now the shell pointer is invalid ....
    m_pShell                    = NULL;
    m_xActiveController         = NULL;
    m_xNavigationController     = NULL;
    m_xActiveForm               = NULL;
    m_xForms                    = NULL;
    m_xSelObject                = NULL;
    m_xCurControl               = NULL;
    m_xCurForm                  = NULL;
    m_xLastGridFound            = NULL;
    m_xAttachedFrame            = NULL;
    m_xExternalViewController   = NULL;
    m_xExtViewTriggerController = NULL;
    m_xExternalDisplayedForm    = NULL;
    m_xLastGridFound            = NULL;

    m_aActiveControllerFeatures.dispose();
    m_aNavControllerFeatures.dispose();
}

//------------------------------------------------------------------------------
void FmXFormShell::InvalidateSlot(sal_Int16 nId, sal_Bool bWithItem, sal_Bool bWithId)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (IsSelectionUpdatePending())
    {
        m_aMarkTimer.Stop();

        // die Invalidierung der Slots, die implizit von SetSelection besorgt wird, eventuell abschalten
        if (!bAllowInvalidation)
            LockSlotInvalidation(sal_True);

        SetSelection(m_pShell->GetFormView()->GetMarkedObjectList());

        if (!bAllowInvalidation)
            LockSlotInvalidation(sal_False);
    }
}

//------------------------------------------------------------------------------
PopupMenu* FmXFormShell::GetConversionMenu()
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    BOOL bIsHiContrastMode  = rSettings.GetMenuColor().IsDark();

    PopupMenu* pNewMenu = new PopupMenu(SVX_RES( RID_FMSHELL_CONVERSIONMENU ));

    ImageList aImageList( SVX_RES( bIsHiContrastMode ? RID_SVXIMGLIST_FMEXPL_HC : RID_SVXIMGLIST_FMEXPL) );
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
sal_Bool FmXFormShell::ExecuteControlConversionSlot(const Reference< XFormComponent>& xContext, sal_uInt16 nSlotId)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    for (int i=0; i<sizeof(nConvertSlots)/sizeof(nConvertSlots[0]); ++i)
    {
        if (nConvertSlots[i] == nSlotId)
            return ConvertControlTo(xContext, nObjectTypes[i]);
    }
    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::IsConversionPossible(const Reference< XPropertySet>& xContext, sal_Int16 nConversionSlot)
{
    Reference< XServiceInfo> xInfo(xContext, UNO_QUERY);
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
void FmXFormShell::CheckControlConversionSlots(const Reference< XPropertySet>& xContext, Menu& rMenu)
{
    for (sal_Int16 i=0; i<rMenu.GetItemCount(); ++i)
        // der Context ist schon von einem Typ, der dem Eitnrag entspricht -> disable
        rMenu.EnableItem(rMenu.GetItemId(i), IsConversionPossible(xContext, rMenu.GetItemId(i)));
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::ConvertControlTo(const Reference< XFormComponent>& xModel, sal_uInt16 nTargetObjectId)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (!xModel.is())
        // das wuerde ausser einem Performance-Verlust nix machen, aber trotzdem ...
        return sal_False;

    FmFormPage* pCurrentPage = m_pShell->GetCurPage();

    SdrUnoObj* pFound = NULL;
    for (sal_uInt32 i=0; i<pCurrentPage->GetObjCount(); ++i)
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
                Reference< XFormComponent> xCurrent(((SdrUnoObj*)pCurrent)->GetUnoControlModel(), UNO_QUERY);
                if ((XFormComponent*)xCurrent.get() == (XFormComponent*)xModel.get())
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
    Reference< XControlModel> xNewModel(::comphelper::getProcessServiceFactory()->createInstance(uNewName), UNO_QUERY);
    if (!xNewModel.is())
        return sal_False;

    Reference< XControlModel> xOldModel(pFound->GetUnoControlModel());
    Reference< XServiceInfo> xModelInfo(xOldModel, UNO_QUERY);
    sal_Int16 nOldModelType = xModelInfo.is() ? getControlTypeByObject(xModelInfo) : OBJ_FM_CONTROL;

    // Properties uebertragen
    Reference< XPropertySet> xOldSet(xOldModel, UNO_QUERY);
    Reference< XPropertySet> xNewSet(xNewModel, UNO_QUERY);


    Locale aNewLanguage = Application::GetSettings().GetUILocale();
    TransferFormComponentProperties(xOldSet, xNewSet, aNewLanguage);

    Sequence< ::com::sun::star::script::ScriptEventDescriptor> aOldScripts;
    Reference< XChild> xChild(xOldModel, UNO_QUERY);
    if (xChild.is())
    {
        Reference< XIndexAccess> xParent(xChild->getParent(), UNO_QUERY);

        // remember old script events
        Reference< ::com::sun::star::script::XEventAttacherManager> xEvManager(xChild->getParent(), UNO_QUERY);
        if (xParent.is() && xEvManager.is())
        {
            sal_Int32 nIndex = getElementPos(xParent, xOldModel);
            if (nIndex>=0 && nIndex<xParent->getCount())
                aOldScripts = xEvManager->getScriptEvents(nIndex);
        }

        // replace the mdoel within the parent container
        Reference< XIndexContainer> xIndexParent(xChild->getParent(), UNO_QUERY);   //Modified by BerryJia for fixing Bug102516 Time(China):2002-9-5 16:00
        if (xIndexParent.is())
        {
            // the form container works with FormComponents
            Reference< XFormComponent> xComponent(xNewModel, UNO_QUERY);
            DBG_ASSERT(xComponent.is(), "FmXFormShell::ConvertControlTo : the new model is no form component !");
            Any aNewModel(makeAny(xComponent));
            try
            {
                //Modified by BerryJia for fixing Bug102516 Time(China):2002-9-5 16:00
                sal_Int32 nIndex = getElementPos(xParent, xOldModel);
                if (nIndex>=0 && nIndex<xParent->getCount())
                    xIndexParent->replaceByIndex(nIndex, aNewModel);
                else
                {
                    DBG_ERROR("FmXFormShell::ConvertControlTo : could not replace the model !");
                    Reference< ::com::sun::star::lang::XComponent> xNewComponent(xNewModel, UNO_QUERY);
                    if (xNewComponent.is())
                        xNewComponent->dispose();
                    return sal_False;
                }
            }
            catch(Exception&)
            {
                DBG_ERROR("FmXFormShell::ConvertControlTo : could not replace the model !");
                Reference< ::com::sun::star::lang::XComponent> xNewComponent(xNewModel, UNO_QUERY);
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
        catch(Exception&)
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
        Reference< XControlContainer > xControlContainer( getControlContainerForView() );

        Sequence< Reference< XControl> > aControls( xControlContainer->getControls() );
        const Reference< XControl>* pControls = aControls.getConstArray();

        sal_uInt32 nLen = aControls.getLength();
        Reference< XControl> xControl;
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

    // transfer value bindings, if possible
    {
        Reference< XBindableValue > xOldBindable( xOldModel, UNO_QUERY );
        Reference< XBindableValue > xNewBindable( xNewModel, UNO_QUERY );
        if ( xOldBindable.is() && xNewBindable.is() )
        {
            xNewBindable->setValueBinding( xOldBindable->getValueBinding() );
            xOldBindable->setValueBinding( NULL );
        }
    }
    // same for list entry sources
    {
        Reference< XListEntrySink > xOldSink( xOldModel, UNO_QUERY );
        Reference< XListEntrySink > xNewSink( xNewModel, UNO_QUERY );
        if ( xOldSink.is() && xNewSink.is() )
        {
            xNewSink->setListEntrySource( xOldSink->getListEntrySource() );
            xOldSink->setListEntrySource( NULL );
        }
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XIndexContainer> xControlModels(m_xActiveForm, UNO_QUERY);
    if (xControlModels.is())
    {
        for (sal_Int16 i=0; i<xControlModels->getCount(); ++i)
        {
            Reference< XPropertySet> xModelSet;
            xControlModels->getByIndex(i) >>= xModelSet;
            if (!xModelSet.is())
                continue;

            if (!::comphelper::hasProperty(FM_PROP_CLASSID, xModelSet))
                continue;
            sal_Int16 nClassId = ::comphelper::getINT16(xModelSet->getPropertyValue(FM_PROP_CLASSID));
            if (FormComponentType::GRIDCONTROL != nClassId)
                continue;

            if (!::comphelper::hasProperty(FM_PROP_CURSORCOLOR, xModelSet) || !::comphelper::hasProperty(FM_PROP_ALWAYSSHOWCURSOR, xModelSet) || !::comphelper::hasProperty(FM_PROP_DISPLAYSYNCHRON, xModelSet))
                continue;

            switch (nWhat & GA_SYNC_MASK)
            {
                case GA_DISABLE_SYNC:
                    {
                        sal_Bool bB(sal_False);
                        xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, Any(&bB,getBooleanCppuType()));
                    }
                    break;
                case GA_FORCE_SYNC:
                {
                    Any aOldVal( xModelSet->getPropertyValue(FM_PROP_DISPLAYSYNCHRON) );
                    sal_Bool bB(sal_True);
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, Any(&bB,getBooleanCppuType()));
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, aOldVal);
                }
                break;
                case GA_ENABLE_SYNC:
                    {
                        sal_Bool bB(sal_True);
                        xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, Any(&bB,getBooleanCppuType()));
                    }
                    break;
            }

            if (nWhat & GA_DISABLE_ROCTRLR)
            {
                sal_Bool bB(sal_False);
                xModelSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, Any(&bB,getBooleanCppuType()));
                Reference< XPropertyState> xModelPropState(xModelSet, UNO_QUERY);
                if (xModelPropState.is())
                    xModelPropState->setPropertyToDefault(FM_PROP_CURSORCOLOR);
                else
                    xModelSet->setPropertyValue(FM_PROP_CURSORCOLOR, Any());        // this should be the default
            }
            else if (nWhat & GA_ENABLE_ROCTRLR)
            {
                sal_Bool bB(sal_True);
                xModelSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, Any(&bB,getBooleanCppuType()));
                xModelSet->setPropertyValue(FM_PROP_CURSORCOLOR, makeAny(sal_Int32(COL_LIGHTRED)));
            }
        }
    }
}

//------------------------------------------------------------------------------
Reference< XControlContainer > FmXFormShell::getControlContainerForView()
{
    SdrPageView* pPageView = NULL;
    if ( m_pShell && m_pShell->GetFormView() )
        pPageView = m_pShell->GetFormView()->GetPageViewPvNum(0);

    Reference< XControlContainer> xControlContainer;
    if ( pPageView )
        xControlContainer = pPageView->GetWindow(0)->GetControlContainerRef();

    return xControlContainer;
}

//------------------------------------------------------------------------------
void FmXFormShell::ExecuteTabOrderDialog( const Reference< XTabControllerModel >& _rxForForm )
{
    OSL_PRECOND( _rxForForm.is(), "FmXFormShell::ExecuteTabOrderDialog: invalid tabbing model!" );
    if ( !_rxForForm.is() )
        return;

    try
    {
        Sequence< Any > aDialogArgs( 3 );
        aDialogArgs[0] <<= NamedValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TabbingModel" ) ),
            makeAny( _rxForForm )
        );
        aDialogArgs[1] <<= NamedValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ControlContext" ) ),
            makeAny( getControlContainerForView() )
        );

        Reference< XWindow > xParentWindow;
        if ( m_pShell && m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame() )
            xParentWindow = VCLUnoHelper::GetInterface ( &m_pShell->GetViewShell()->GetViewFrame()->GetWindow() );
        aDialogArgs[2] <<= NamedValue(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ) ),
            makeAny( xParentWindow )
        );

        Reference< dialogs::XExecutableDialog > xDialog(
            ::comphelper::getProcessServiceFactory()->createInstanceWithArguments(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.ui.TabOrderDialog" ) ),
                aDialogArgs
            ),
            UNO_QUERY
        );
        OSL_ENSURE( xDialog.is(), "FmXFormShell::ExecuteTabOrderDialog: could not create the dialog!" );

        if ( xDialog.is() )
            xDialog->execute();
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "FmXFormShell::ExecuteTabOrderDialog: caught an exception!" );
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::ExecuteSearch()
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    UniString sTestContexts;
    //  m_arrSearchContexts.Remove(0, m_arrSearchContexts.Count());
    m_arrSearchContexts.clear();

    // eine Sammlung aller (logischen) Formulare
    Reference< XInterface> xAllForms(m_pShell->GetCurPage()->GetForms(),UNO_QUERY);
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
    for (xub_StrLen j=0; j<sTestContexts.GetTokenCount(';'); ++j)
    {
        strCurrentToken = sTestContexts.GetToken(j);
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
    Reference< XForm> xActiveForm( getActiveForm());
    for (i=0; i<(sal_Int32)m_arrSearchContexts.size(); ++i)
    {
        if (m_arrSearchContexts.at(i) == xActiveForm)
        {
            nInitialContext = (sal_Int16)i;
            break;
        }
    }

    // wenn der Dialog initial den Text des aktiven Controls anbieten soll, muss dieses ein XTextComponent-Interface habe,
    // ausserdem macht das nur Sinn, wenn das aktuelle Feld auch an ein Tabellen- (oder was-auch-immer-)Feld gebunden ist
    UniString strActiveField;
    UniString strInitialText;
    // ... das bekomme ich von meinem FormController
    DBG_ASSERT(m_xActiveController.is(), "FmXFormShell::ExecuteSearch : no active controller !");
    Reference< XControl> xActiveControl( m_xActiveController->getCurrentControl());
    if (xActiveControl.is())
    {
        // das Control kann mir sein Model sagen ...
        Reference< XControlModel> xActiveModel( xActiveControl->getModel());
        DBG_ASSERT(xActiveModel.is(), "FmXFormShell::ExecuteSearch : active control has no model !");

        // das Model frage ich nach der ControlSource-Eigenschaft ...
        Reference< XPropertySet> xProperties(xActiveControl->getModel(), UNO_QUERY);
        if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xProperties) && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xProperties))
        {
            Reference< XPropertySet> xField;
            xProperties->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;
            if (xField.is())    // (nur wenn das Ding wirklich gebunden ist)
            {
                // und das Control selber nach einem TextComponent-Interface (damit ich mir dort den Text abholen kann)
                Reference< XTextComponent> xText(xActiveControl, UNO_QUERY);
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
            Reference< XGrid> xGrid(xActiveControl, UNO_QUERY);
            if (xGrid.is())
            {
                // fuer strActiveField brauche ich die die ControlSource der Column, dafuer den Columns-Container, dafuer die
                // GridPeer
                Reference< XGridPeer> xGridPeer(xActiveControl->getPeer(), UNO_QUERY);
                Reference< XIndexAccess> xColumns;
                if (xGridPeer.is())
                    xColumns = Reference< XIndexAccess>(xGridPeer->getColumns(),UNO_QUERY);

                sal_Int16 nViewCol = xGrid->getCurrentColumnPosition();
                sal_Int16 nModelCol = GridView2ModelPos(xColumns, nViewCol);
                Reference< XPropertySet> xCurrentCol;
                if(xColumns.is())
                    xColumns->getByIndex(nModelCol) >>= xCurrentCol;
                if (xCurrentCol.is())
                    strActiveField = ::comphelper::getString(xCurrentCol->getPropertyValue(FM_PROP_LABEL)).getStr();

                // the text fo the current column
                Reference< XIndexAccess> xColControls(xGridPeer, UNO_QUERY);
                Reference< XInterface> xCurControl;
                xColControls->getByIndex(nViewCol) >>= xCurControl;
                ::rtl::OUString sInitialText;
                if (IsSearchableControl(xCurControl, &sInitialText))
                    strInitialText = sInitialText.getStr();
            }
        }
    }

    // um eventuelle GridControls, die ich kenne, kuemmern
    LoopGrids(GA_DISABLE_SYNC /*| GA_ENABLE_ROCTRLR*/);

    // jetzt bin ich reif fuer den Dialog
    //CHINA001 FmSearchDialog dlg(&m_pShell->GetViewShell()->GetViewFrame()->GetWindow(), strInitialText, strRealContexts, nInitialContext, LINK(this, FmXFormShell, OnSearchContextRequest),
//CHINA001      SM_ALLOWSCHEDULE); //CHINA001 FmSearchDialog::SM_ALLOWSCHEDULE);
    // wenn die potentiellen Deadlocks, die durch die Benutzung des Solar-Mutex in MTs VCLX...-Klasen entstehen, irgendwann mal
    // ausgeraeumt sind, sollte hier ein SM_USETHREAD rein, denn die Suche in einem eigenen Thread ist doch etwas fluessiger
    // sollte allerdings irgendwie von dem unterliegenden Cursor abhaengig gemacht werden, DAO zum Beispiel ist nicht thread-sicher
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");//CHINA001
    AbstractFmSearchDialog* dlg = pFact->CreateFmSearchDialog(&m_pShell->GetViewShell()->GetViewFrame()->GetWindow(), strInitialText, strRealContexts, nInitialContext, LINK(this, FmXFormShell, OnSearchContextRequest),ResId(RID_SVXDLG_SEARCHFORM),SM_ALLOWSCHEDULE);
    DBG_ASSERT(dlg, "Dialogdiet fail!");//CHINA001
    dlg->SetActiveField(strActiveField);//CHINA001  dlg.SetActiveField(strActiveField);

    dlg->SetFoundHandler(LINK(this, FmXFormShell, OnFoundData));//CHINA001 dlg.SetFoundHandler(LINK(this, FmXFormShell, OnFoundData));
    dlg->SetCanceledNotFoundHdl(LINK(this, FmXFormShell, OnCanceledNotFound));//CHINA001 dlg.SetCanceledNotFoundHdl(LINK(this, FmXFormShell, OnCanceledNotFound));
    dlg->Execute();//CHINA001 dlg.Execute();
    delete dlg; //add CHINA001
    // GridControls wieder restaurieren
    LoopGrids(GA_ENABLE_SYNC | GA_DISABLE_ROCTRLR);

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetPageViewPvNum(0));
        // da ich in OnFoundData (fals ich dort war) Controls markiert habe
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::GetY2KState(sal_uInt16& n)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (m_pShell->IsDesignMode())
        // im Design-Modus (ohne aktive Controls) soll sich das Haupt-Dokument darum kuemmern
        return sal_False;

    Reference< XForm> xForm( getActiveForm());
    if (!xForm.is())
        // kein aktuelles Formular (also insbesondere kein aktuelles Control) -> das Haupt-Dokument soll sich kuemmern
        return sal_False;

    Reference< XRowSet> xDB(xForm, UNO_QUERY);
    DBG_ASSERT(xDB.is(), "FmXFormShell::GetY2KState : current form has no dbform-interface !");

    Reference< XNumberFormatsSupplier> xSupplier( getNumberFormats(OStaticDataAccessTools().getRowSetConnection(xDB), sal_False));
    if (xSupplier.is())
    {
        Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
        if (xSet.is())
        {
            try
            {
                Any aVal( xSet->getPropertyValue(::rtl::OUString::createFromAscii("TwoDigitDateStart")) );
                aVal >>= n;
                return sal_True;
            }
            catch(Exception&)
            {
            }

        }
    }
    return sal_False;
}

//------------------------------------------------------------------------------
void FmXFormShell::SetY2KState(sal_uInt16 n)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XForm> xActiveForm( getActiveForm());
    Reference< XRowSet> xDB(xActiveForm, UNO_QUERY);
    if (xDB.is())
    {
        Reference< XNumberFormatsSupplier> xSupplier( getNumberFormats(getRowSetConnection(xDB), sal_False));
        if (xSupplier.is())
        {
            Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    Any aVal;
                    aVal <<= n;
                    xSet->setPropertyValue(::rtl::OUString::createFromAscii("TwoDigitDateStart"), aVal);
                }
                catch(Exception&)
                {
                    DBG_ERROR("FmXFormShell::SetY2KState: Exception occured!");
                }

            }
            return;
        }
    }

    // kein aktives Formular gefunden -> alle aktuell vorhandenen Formulare durchiterieren
    Reference< XIndexAccess> xCurrentForms( m_xForms);
    if (!xCurrentForms.is())
    {   // im alive-Modus sind meine Forms nicht gesetzt, wohl aber die an der Page
        if (m_pShell->GetCurPage())
            xCurrentForms = Reference< XIndexAccess>(m_pShell->GetCurPage()->GetForms(), UNO_QUERY);
    }
    if (!xCurrentForms.is())
        return;

    ::comphelper::IndexAccessIterator aIter(xCurrentForms);
    Reference< XInterface> xCurrentElement( aIter.Next());
    while (xCurrentElement.is())
    {
        // ist das aktuelle Element eine DatabaseForm ?
        Reference< XRowSet> xDB(xCurrentElement, UNO_QUERY);
        if (xDB.is())
        {
            Reference< XNumberFormatsSupplier> xSupplier( getNumberFormats(getRowSetConnection(xDB), sal_False));
            if (!xSupplier.is())
                continue;

            Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    Any aVal;
                    aVal <<= n;
                    xSet->setPropertyValue(::rtl::OUString::createFromAscii("TwoDigitDateStart"), aVal);
                }
                catch(Exception&)
                {
                    DBG_ERROR("FmXFormShell::SetY2KState: Exception occured!");
                }

            }
        }
        xCurrentElement = aIter.Next();
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::CloseExternalFormViewer()
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (!m_xExternalViewController.is())
        return;

    Reference< ::com::sun::star::frame::XFrame> xExternalViewFrame( m_xExternalViewController->getFrame());
    Reference< ::com::sun::star::frame::XDispatchProvider> xCommLink(xExternalViewFrame, UNO_QUERY);
    if (!xCommLink.is())
        return;

    xExternalViewFrame->setComponent(NULL,NULL);
    ::comphelper::disposeComponent(xExternalViewFrame);

    m_xExternalViewController   = NULL;
    m_xExtViewTriggerController = NULL;
    m_xExternalDisplayedForm    = NULL;
}

//------------------------------------------------------------------------------
Reference< XResultSet> FmXFormShell::getInternalForm(const Reference< XResultSet>& _xForm) const
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XFormController> xExternalCtrlr(m_xExternalViewController, UNO_QUERY);
    if (xExternalCtrlr.is() && (_xForm == xExternalCtrlr->getModel()))
    {
        DBG_ASSERT(m_xExternalDisplayedForm.is(), "FmXFormShell::getInternalForm : invalid external form !");
        return m_xExternalDisplayedForm;
    }
    return _xForm;
}

//------------------------------------------------------------------------------
Reference< XForm> FmXFormShell::getInternalForm(const Reference< XForm>& _xForm) const
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XFormController> xExternalCtrlr(m_xExternalViewController, UNO_QUERY);
    if (xExternalCtrlr.is() && (_xForm == xExternalCtrlr->getModel()))
    {
        DBG_ASSERT(m_xExternalDisplayedForm.is(), "FmXFormShell::getInternalForm : invalid external form !");
        return Reference< XForm>(m_xExternalDisplayedForm, UNO_QUERY);
    }
    return _xForm;
}

//------------------------------------------------------------------------------
void FmXFormShell::ExecuteFormSlot( sal_Int32 _nSlot,
        const Reference< XForm >& _rxForm, const Reference< XFormController >& _rxController )
{
    DBG_ASSERT( _rxForm.is(), "FmXFormShell::ExecuteFormSlot: invalid form!" );
    if ( !_rxForm.is() )
        return;

    DBG_ASSERT( !_rxController.is() || ( _rxController->getModel() == _rxForm ),
        "FmXFormShell::ExecuteFormSlot: inconsistent arguments!" );

    // delegate the functionality to a FormControllerHelper
    ControllerFeatures aHelper( ::comphelper::getProcessServiceFactory(), this );
    if ( _rxController.is() )
        aHelper.assign( _rxController );
    else
        aHelper.assign( _rxForm );

    aHelper->execute( _nSlot );
    if ( _nSlot == SID_FM_RECORD_UNDO )
        {
        // if we're doing an UNDO, *and* if the affected form is the form which we also display
        // as external view, then we need to reset the controls of the external form, too
        if ( getInternalForm( _rxForm ) == m_xExternalDisplayedForm )
            {
            Reference< XFormController > xExternalFormController( m_xExternalViewController, UNO_QUERY );
            if ( xExternalFormController.is() )
                aHelper->resetAllControls( Reference< XForm >( xExternalFormController->getModel(), UNO_QUERY ) );
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::setActiveController( const Reference< XFormController>& xController, sal_Bool _bNoSaveOldContent )
{
    DBG_ASSERT(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
        if (HasPendingCursorAction(Reference< XResultSet>(m_xActiveForm, UNO_QUERY)))
            restoreControlLocks();

        ::osl::ClearableMutexGuard aGuard(m_aAsyncSafety);
        // switch all nav dispatchers belonging to the form of the current nav controller to 'non active'
        Reference< XResultSet> xNavigationForm;
        if (m_xNavigationController.is())
            xNavigationForm = Reference< XResultSet>(m_xNavigationController->getModel(), UNO_QUERY);
        aGuard.clear();

        m_bInActivate = sal_True;

        // check if the 2 controllers serve different forms
        Reference< XResultSet> xOldForm;
        if (m_xActiveController.is())
            xOldForm = Reference< XResultSet>(m_xActiveController->getModel(), UNO_QUERY);
        Reference< XResultSet> xNewForm;
        if (xController.is())
            xNewForm = Reference< XResultSet>(xController->getModel(), UNO_QUERY);
        xOldForm = getInternalForm(xOldForm);
        xNewForm = getInternalForm(xNewForm);

        sal_Bool bDifferentForm = ( xOldForm.get() != xNewForm.get() );
        sal_Bool bNeedSave = bDifferentForm && !_bNoSaveOldContent;
            // we save the content of the old form if we move to a new form, and saving old content is allowed

        if ( m_xActiveController.is() && bNeedSave )
        {
            // beim Wechsel des Controllers den Inhalt speichern, ein Commit
            // wurde bereits ausgefuehrt
            if ( m_aActiveControllerFeatures->commitCurrentControl() )
            {
                m_bSetFocus = sal_True;
                if ( m_aActiveControllerFeatures->isModifiedRecord() )
                    {
                    sal_Bool bIsNew = m_aActiveControllerFeatures->isNewRecord();
                    sal_Bool bResult = m_aActiveControllerFeatures->commitCurrentRecord();
                    if ( !bResult && m_bSetFocus )
                    {
                        // if we couldn't save the current record, set the focus back to the
                        // current control
                        Reference< XWindow > xWindow( m_xActiveController->getCurrentControl(), UNO_QUERY );
                        if ( xWindow.is() )
                        xWindow->setFocus();
                        m_bInActivate = sal_False;
                        return;
                    }
                    else if ( bResult && bIsNew )
                    {
                        Reference< XResultSet > xCursor( m_aActiveControllerFeatures->getCursor().get() );
                        if ( xCursor.is() )
                        {
                            DO_SAFE( xCursor->last(); );
                        }
                    }
                }
            }
        }

        stopListening();

        m_aActiveControllerFeatures.dispose();
        m_xActiveController = xController;
        if ( m_xActiveController.is() )
            m_aActiveControllerFeatures.assign( m_xActiveController );

        if ( m_xActiveController.is() )
        {
            // set eventlistener to know when it is disposed
            Reference< ::com::sun::star::lang::XComponent> xComp(m_xActiveController, UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener((XEventListener*)(XPropertyChangeListener*)this);

            m_xActiveForm = getInternalForm(Reference< XForm>(m_xActiveController->getModel(), UNO_QUERY));
        }
        else
            m_xActiveForm = NULL;

        startListening();

        // activate all dispatchers belonging to form of the new navigation controller
        xNavigationForm = NULL;
        if (m_xNavigationController.is())
            xNavigationForm = Reference< XResultSet>(m_xNavigationController->getModel(), UNO_QUERY);

        // if there is a async cursor action running we have to lock the controls of the new controller
        if (HasPendingCursorAction(Reference< XResultSet>(m_xActiveForm, UNO_QUERY)))
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
                Reference< XPropertyState> xMultiIFace(FmXMultiSet::Create( rMarkList ));
                setCurControl( xMultiIFace );
            }
            else
            {
                pSelFormObj = PTR_CAST(FmFormObj, rMarkList.GetMark(0)->GetObj());

                if ( pSelFormObj ) // Einfachselektion
                {
                    Reference< XFormComponent> xFormComponent(pSelFormObj->GetUnoControlModel(), UNO_QUERY);
                    if (xFormComponent.is())
                        setCurControl(xFormComponent);
                    else
                        setCurControl( Reference< XInterface>() );
                }
                else
                    setCurControl( Reference< XInterface>() );
            }
        }
        else if (nMarkCount > 1)            // Mehrfachselektion
        {
            Reference< XPropertyState> xMultiIFace(FmXMultiSet::Create( rMarkList ));
            setCurControl( xMultiIFace );
        }
    }
    else // Keine Selektion oder gemischt
        setCurControl( Reference< XInterface>() );
}

//------------------------------------------------------------------------------
void FmXFormShell::setCurControl(const Reference< XInterface>& xNewControl)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DBG_ASSERT(m_pShell->IsDesignMode(), "nur im designmode verwenden");

    if (m_xCurControl == xNewControl)
        return;

    // Stammen beide Objecte vom gleichen Container
    if (m_xCurControl.is())
    {
        Reference< XChild> xCur(m_xCurControl, UNO_QUERY);
        Reference< XChild> xNew(xNewControl, UNO_QUERY);
        // wenn nichts selektiert oder unterschiedliche Väter und der Vater von xCurControl ist ein XSelectionSupplier
        // dann muß dieser deselektieren.
        if (!xNew.is() || xCur->getParent() != xNew->getParent())
        {
            Reference< ::com::sun::star::view::XSelectionSupplier> xSel(xCur->getParent(), UNO_QUERY);
            if (xSel.is())
            {
                xSel->select(Any());
            }
        }
    }

    m_xCurControl = xNewControl;

    InvalidateSlot(SID_FM_CTL_PROPERTIES);
}

//------------------------------------------------------------------------------
void FmXFormShell::setSelObject(const Reference< XInterface>& xNewObj)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DBG_ASSERT(m_pShell->IsDesignMode(), "nur im designmode verwenden");

    if (m_xSelObject != xNewObj)
    {
        m_xSelObject = xNewObj;
        for (sal_Int16 i = 0; i<sizeof(SelObjectSlotMap)/sizeof(SelObjectSlotMap[0]); ++i)
            InvalidateSlot(SelObjectSlotMap[i]);
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::setCurForm(const Reference< XForm>& xF)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XRowSet> xDatabaseForm(m_xActiveForm, UNO_QUERY);
    if (xDatabaseForm.is() && getRowSetConnection(xDatabaseForm).is())
    {
        Reference< XPropertySet> xActiveFormSet(m_xActiveForm, UNO_QUERY);
        if (xActiveFormSet.is())
        {
            // wenn es eine Datenquelle gibt, dann den Listener aufbauen
            // TODO: this is strange - shouldn't this depend on a isLoaded instead of
            // a "has command value"? Finally, the command value only means that it was
            // intended to be loaded, not that it actually *is* loaded
            ::rtl::OUString aSource = ::comphelper::getString(xActiveFormSet->getPropertyValue(FM_PROP_COMMAND));
            if (aSource.getLength())
            {
                m_bDatabaseBar = sal_True;

                xActiveFormSet->getPropertyValue(FM_PROP_NAVIGATION) >>= m_eNavigate;

                switch (m_eNavigate)
                {
                    case NavigationBarMode_PARENT:
                    {
                        // suchen des Controllers, ueber den eine Navigation moeglich ist
                        Reference< XChild> xChild(m_xActiveController, UNO_QUERY);
                        Reference< XFormController> xParent;
                        while (xChild.is())
                        {
                            xChild = Reference< XChild>(xChild->getParent(), UNO_QUERY);
                            xParent  = Reference< XFormController>(xChild, UNO_QUERY);
                            Reference< XPropertySet> xParentSet;
                            if (xParent.is())
                                xParentSet = Reference< XPropertySet>(xParent->getModel(), UNO_QUERY);
                            if (xParentSet.is())
                            {
                                xParentSet->getPropertyValue(FM_PROP_NAVIGATION) >>= m_eNavigate;
                                if (m_eNavigate == NavigationBarMode_CURRENT)
                                    break;
                            }
                        }
                        m_xNavigationController = xParent;
                    }
                    break;

                    case NavigationBarMode_CURRENT:
                        m_xNavigationController = m_xActiveController;
                        break;

                    default:
                        m_xNavigationController = NULL;
                        m_bDatabaseBar = sal_False;
                }

                m_aNavControllerFeatures.dispose();
                if ( m_xNavigationController.is() && ( m_xNavigationController != m_xActiveController ) )
                    m_aNavControllerFeatures.assign( m_xNavigationController );

                // an dem Controller, der die Navigation regelt, wg. RecordCount lauschen
                Reference< XPropertySet> xNavigationSet;
                if (m_xNavigationController.is())
                {
                    xNavigationSet = Reference< XPropertySet>(m_xNavigationController->getModel(), UNO_QUERY);
                    if (xNavigationSet.is())
                        xNavigationSet->addPropertyChangeListener(FM_PROP_ROWCOUNT,this);
                }
                return;
            }
        }
    }

    m_eNavigate  = NavigationBarMode_NONE;
    m_bDatabaseBar = sal_False;
    m_xNavigationController = NULL;
}

//------------------------------------------------------------------------------
void FmXFormShell::stopListening()
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XRowSet> xDatabaseForm(m_xActiveForm, UNO_QUERY);
    if ( xDatabaseForm.is() )
    {
        if (m_xNavigationController.is())
        {
            Reference< XPropertySet> xSet(m_xNavigationController->getModel(), UNO_QUERY);
            if (xSet.is())
                xSet->removePropertyChangeListener(FM_PROP_ROWCOUNT, this);

        }
    }

    m_bDatabaseBar = sal_False;
    m_eNavigate  = NavigationBarMode_NONE;
    m_xNavigationController = NULL;
}

//------------------------------------------------------------------------------
void FmXFormShell::ShowProperties(const Reference< XInterface>& rxObject, sal_Bool bShow)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DBG_ASSERT((pfriWhere->nContext >= 0) && (pfriWhere->nContext < (sal_Int16)m_arrSearchContexts.size()),
        "FmXFormShell::OnFoundData : ungueltiger Kontext !");
    Reference< XForm> xForm( m_arrSearchContexts.at(pfriWhere->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnFoundData : ungueltige Form !");

    Reference< XRowLocate> xCursor(xForm, UNO_QUERY);
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
    Reference< XControlModel> xControlModel( ((SdrUnoObj*)pObject)->GetUnoControlModel());
    DBG_ASSERT(xControlModel.is(), "FmXFormShell::OnFoundData : ungueltiges Control !");

    // disable the permanent cursor for the last grid we found a record
    if (m_xLastGridFound.is() && (m_xLastGridFound != xControlModel))
    {
        Reference< XPropertySet> xOldSet(m_xLastGridFound, UNO_QUERY);
        sal_Bool bB(sal_False);
        xOldSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, makeAny( (sal_Bool)sal_False ) );
        Reference< XPropertyState> xOldSetState(xOldSet, UNO_QUERY);
        if (xOldSetState.is())
            xOldSetState->setPropertyToDefault(FM_PROP_CURSORCOLOR);
        else
            xOldSet->setPropertyValue(FM_PROP_CURSORCOLOR, Any());
    }

    // wenn das Feld sich in einem GridControl befindet, muss ich dort noch in die entsprechende Spalte gehen
    sal_Int32 nGridColumn = m_arrRelativeGridColumn.GetObject(pfriWhere->nFieldPos);
    if (nGridColumn != -1)
    {   // dummer weise muss ich mir das Control erst wieder besorgen
        Reference< XControl> xControl( GetControlFromModel(xControlModel));
        Reference< XGrid> xGrid(xControl, UNO_QUERY);
        DBG_ASSERT(xGrid.is(), "FmXFormShell::OnFoundData : ungueltiges Control !");
        // wenn eine der Asserts anschlaegt, habe ich beim Aufbauen von m_arrSearchedControls wohl was falsch gemacht

        // enable a permanent cursor for the grid so we can see the found text
        Reference< XPropertySet> xModelSet(xControlModel, UNO_QUERY);
        DBG_ASSERT(xModelSet.is(), "FmXFormShell::OnFoundData : invalid control model (no property set) !");
        sal_Bool bB(sal_True);
        xModelSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, Any(&bB,getBooleanCppuType()));
        xModelSet->setPropertyValue(FM_PROP_CURSORCOLOR, makeAny(sal_Int32(COL_LIGHTRED)));
        m_xLastGridFound = xControlModel;

        xGrid->setCurrentColumnPosition((sal_Int16)nGridColumn);
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DBG_ASSERT((pfriWhere->nContext >= 0) && (pfriWhere->nContext < (sal_Int16)m_arrSearchContexts.size()),
        "FmXFormShell::OnCanceledNotFound : ungueltiger Kontext !");
    Reference< XForm> xForm( m_arrSearchContexts.at(pfriWhere->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnCanceledNotFound : ungueltige Form !");

    Reference< XRowLocate> xCursor(xForm, UNO_QUERY);
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DBG_ASSERT(pfmscContextInfo->nContext < (sal_Int16)m_arrSearchContexts.size(), "FmXFormShell::OnSearchContextRequest : invalid parameter !");
    Reference< XForm> xForm( m_arrSearchContexts.at(pfmscContextInfo->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnSearchContextRequest : unexpected : invalid context !");

    Reference< XResultSet> xIter(xForm, UNO_QUERY);
    DBG_ASSERT(xIter.is(), "FmXFormShell::OnSearchContextRequest : unexpected : context has no iterator !");

    // --------------------------------------------------------------------------------------------
    // die Liste der zu involvierenden Felder zusammenstellen (sind die ControlSources aller Felder, die eine solche Eigenschaft habe)
    UniString strFieldList, sFieldDisplayNames;
    m_arrSearchedControls.Remove(0, m_arrSearchedControls.Count());
    m_arrRelativeGridColumn.Remove(0, m_arrRelativeGridColumn.Count());

    // folgendes kleines Problem : Ich brauche, um gefundene Felder zu markieren, SdrObjekte. Um hier festzustellen, welche Controls
    // ich in die Suche einbeziehen soll, brauche ich Controls (also XControl-Interfaces). Ich muss also ueber eines von beiden
    // iterieren und mir das jeweils andere besorgen. Dummerweise gibt es keine direkte Verbindung zwischen beiden Welten (abgesehen
    // von einem GetUnoControl an SdrUnoObject, das aber ein OutputDevice verlangt, womit ich nichts anfangen kann).
    // Allerdings komme ich sowohl von einem Control als auch von einem SdrObject zum Model, und damit ist mir mit einer doppelten
    // Schleife die Zuordnung SdrObject<->Control moeglich.
    // Die Alternative zu dieser (unschoenen und sicher auch nicht ganz fixen) Loesung waere, auf das Cachen der SdrObjects zu
    // verzichten, was dann aber in OnFoundData zu wesentlicher Mehrarbeit fuehren wuerde (da ich mir dort jedesmal das SdrObject
    // erst besorgen muesste). Da aber OnFoundData i.d.R. oefter aufgerufen wird als ExecuteSearch, erledige ich das hier.

    Reference< XNameAccess> xValidFormFields;
    Reference< XColumnsSupplier> xSupplyCols(xIter, UNO_QUERY);
    DBG_ASSERT(xSupplyCols.is(), "FmXFormShell::OnSearchContextRequest : invalid cursor : no columns supplier !");
    if (xSupplyCols.is())
        xValidFormFields = xSupplyCols->getColumns();
    DBG_ASSERT(xValidFormFields.is(), "FmXFormShell::OnSearchContextRequest : form has no fields !");

    // aktuelle(r) Page/Controller
    FmFormPage* pCurrentPage = m_pShell->GetCurPage();
    DBG_ASSERT(pCurrentPage!=NULL, "FmXFormShell::OnSearchContextRequest : no page !");
    // alle Sdr-Controls dieser Seite durchsuchen ...
    ::rtl::OUString sControlSource, sCompareString,aName;
    for (sal_Int32 i=0; i<(sal_Int32)pCurrentPage->GetObjCount(); ++i)
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
                Reference< XControlModel> xControlModel( ((SdrUnoObj*)pCurrent)->GetUnoControlModel());
                DBG_ASSERT(xControlModel.is(), "FmXFormShell::OnSearchContextRequest : invalid objects !");
                // ... als FormComponent
                Reference< XFormComponent> xCurrentAsFormComponent(xControlModel, UNO_QUERY);
                if (!xCurrentAsFormComponent.is())
                {
                    if (pGroupIterator)     // ich iteriere gerade durch eine Gruppe von Controls
                        pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
                    else
                        pCurrent = NULL;
                    continue;
                }

                // gehoert diese FormComponent ueberhaupt zu der Form, um die es geht ?
                Reference< XInterface> xParentOfCurrent( xCurrentAsFormComponent->getParent());
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
                Reference< XControl> xControlBehindModel;
                    // das Control, das als Model xControlModel hat
                    // (das folgende while kann mehrmals durchlaufen werden, ohne dass das Control sich aendert, dann muss
                    // ich nicht jedesmal neu suchen)

                Reference< XInterface> xCurrent( iter.Next());
                while (xCurrent.is())
                {
                    sControlSource = iter.getCurrentValue();
                    if (sControlSource == sCompareString)
                    {   // das aktuelle Element hat keine ControlSource, also ist es ein GridControl (das ist das einzige, was
                        // der SearchableControlIterator noch zulaesst)
                        xControlBehindModel = GetControlFromModel(xControlModel);
                        DBG_ASSERT(xControlBehindModel.is(), "FmXFormShell::OnSearchContextRequest : didn't ::std::find a control with requested model !");

                        Reference< XGridPeer> xGridPeer(xControlBehindModel->getPeer(), UNO_QUERY);
                        do
                        {
                            if (!xGridPeer.is())
                                break;

                            Reference< XIndexAccess> xPeerContainer(xGridPeer, UNO_QUERY);
                            if (!xPeerContainer.is())
                                break;

                            Reference< XIndexAccess> xModelColumns(xGridPeer->getColumns(), UNO_QUERY);
                            DBG_ASSERT(xModelColumns.is(), "FmXFormShell::OnSearchContextRequest : there is a grid control without columns !");
                                // the case 'no columns' should be indicated with an empty container, I think ...
                            DBG_ASSERT(xModelColumns->getCount() >= xPeerContainer->getCount(), "FmXFormShell::OnSearchContextRequest : impossible : have more view than model columns !");

                            Reference< XInterface> xCurrentColumn;
                            for (sal_Int16 nViewPos=0; nViewPos<xPeerContainer->getCount(); ++nViewPos)
                            {
                                xPeerContainer->getByIndex(nViewPos) >>= xCurrentColumn;
                                if (!xCurrentColumn.is())
                                    continue;

                                // can we use this column control fo searching ?
                                if (!IsSearchableControl(xCurrentColumn))
                                    continue;

                                sal_Int16 nModelPos = GridView2ModelPos(xModelColumns, nViewPos);
                                Reference< XPropertySet> xCurrentColModel;
                                xModelColumns->getByIndex(nModelPos) >>= xCurrentColModel;
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
                                sFieldDisplayNames += getLabelName(Reference< XPropertySet>(xControlModel, UNO_QUERY)).getStr();
                                sFieldDisplayNames += ';';

                                // das SdrObjekt merken (beschleunigt die Behandlung in OnFoundData)
                                m_arrSearchedControls.C40_INSERT(SdrObject, pCurrent, m_arrSearchedControls.Count());

                                // die Nummer der Spalte (hier ein Dummy, nur fuer GridControls interesant)
                                m_arrRelativeGridColumn.Insert(-1, m_arrRelativeGridColumn.Count());

                                // und fuer die formatierte Suche ...
                                pfmscContextInfo->arrFields.push_back(Reference< XInterface>(xControlBehindModel, UNO_QUERY));
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
    Reference< XPropertySet> xCursorSet(pfmscContextInfo->xCursor, UNO_QUERY);
    Reference< XResultSetUpdate> xUpdateCursor(pfmscContextInfo->xCursor, UNO_QUERY);
    if (xUpdateCursor.is() && xCursorSet.is() && xCursorSet.is())
        if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISNEW)))
            xUpdateCursor->moveToCurrentRow();
        else
            if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISMODIFIED)))
                xUpdateCursor->cancelRowUpdates();

    return pfmscContextInfo->arrFields.size();
}

  // XContainerListener
//------------------------------------------------------------------------------
void FmXFormShell::elementInserted(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    // neues Object zum lauschen
    Reference< XInterface> xTemp;
    evt.Element >>= xTemp;
    AddElement(xTemp);
    m_pShell->DetermineForms(sal_True);
}

//------------------------------------------------------------------------------
void FmXFormShell::elementReplaced(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XInterface> xTemp;
    evt.ReplacedElement >>= xTemp;
    RemoveElement(xTemp);
    evt.Element >>= xTemp;
    AddElement(xTemp);
}

//------------------------------------------------------------------------------
void FmXFormShell::elementRemoved(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XInterface> xTemp;
    evt.Element >>= xTemp;
    RemoveElement(xTemp);
    m_pShell->DetermineForms(sal_True);
}

//------------------------------------------------------------------------------
void FmXFormShell::ResetForms(const Reference< XIndexAccess>& _xForms, sal_Bool bInvalidate)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (m_xForms != _xForms)
    {
        RemoveElement(m_xForms);

        m_xForms = _xForms;

        AddElement(m_xForms);
    }
    m_pShell->DetermineForms(bInvalidate);
}

//------------------------------------------------------------------------------
void FmXFormShell::AddElement(const Reference< XInterface>& Element)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    // am Container horchen
    Reference< XIndexContainer> xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface> xElement;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            xContainer->getByIndex(i) >>= xElement;
            AddElement(xElement);
        }

        Reference< XContainer> xCont(Element, UNO_QUERY);
        if (xCont.is())
            xCont->addContainerListener(this);
    }

    Reference< ::com::sun::star::view::XSelectionSupplier> xSelSupplier(Element, UNO_QUERY);
    if (xSelSupplier.is())
        xSelSupplier->addSelectionChangeListener(this);
}

//------------------------------------------------------------------------------
void FmXFormShell::RemoveElement(const Reference< XInterface>& Element)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< ::com::sun::star::view::XSelectionSupplier> xSelSupplier(Element, UNO_QUERY);
    if (xSelSupplier.is())
        xSelSupplier->removeSelectionChangeListener(this);

    // Verbindung zu Kindern aufheben
    Reference< XIndexContainer> xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        Reference< XContainer> xCont(Element, UNO_QUERY);
        if (xCont.is())
            xCont->removeContainerListener(this);

        sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface> xElement;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            xContainer->getByIndex(i) >>= xElement;
            RemoveElement(xElement);
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::selectionChanged(const EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< ::com::sun::star::view::XSelectionSupplier> xSupplier(rEvent.Source, UNO_QUERY);
    Any aSelObj( xSupplier->getSelection() );
    if (aSelObj.getValueTypeClass() != TypeClass_INTERFACE)
        return;
    Reference< XInterface> xSelObj;
    aSelObj >>= xSelObj;

    // es wurde eine Selektion weggenommen, dieses kann nur durch die Shell vorgenommen werden
    if (!xSelObj.is())
        return;

    Reference< XInterface> xPreviousObject( getSelObject());
    EnableTrackProperties(sal_False);

    sal_Bool bMarkChanged = m_pShell->GetFormView()->checkUnMarkAll(rEvent.Source);
    Reference< XForm> xNewForm( GetForm(rEvent.Source));

    setCurControl(xSelObj);
    setCurForm(xNewForm);

    // wurde vorher Form angezeigt, dann wieder die Form anzeigen
    Reference< XForm> xOldForm(xPreviousObject, UNO_QUERY);
    if (xOldForm.is())
        setSelObject(m_xCurForm);
    else
        setSelObject(m_xCurControl);

    if (IsPropBrwOpen() && m_xSelObject != xPreviousObject)
        ShowProperties(m_xSelObject, sal_True);

    EnableTrackProperties(sal_True);
    if ( bMarkChanged )
        m_pShell->NotifyMarkListChanged(m_pShell->GetFormView());
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnTimeOut, void*, EMPTYTAG)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (m_pShell->IsDesignMode() && m_pShell->GetFormView())
        SetSelection(m_pShell->GetFormView()->GetMarkedObjectList());

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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DetermineSelection(rMarkList);
    m_pShell->NotifyMarkListChanged(m_pShell->GetFormView());
}

//------------------------------------------------------------------------
void FmXFormShell::DetermineSelection(const SdrMarkList& rMarkList)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    // Aktuelles Objekt setzen
    Reference< XInterface> xPreviousObject( m_xSelObject);
    sal_Bool bMixedWithFormControls = sal_True;
    Reference< XForm> xNewForm( DetermineCurForm(rMarkList, bMixedWithFormControls));

    // Aktuelle Form anpassen
    if (xNewForm.is() || bMixedWithFormControls)
    {
        // die Form wird nur angepasst, wenn eine eindeutige From gefunden wurde oder eine gemischte Selektion existiert,
        // die auch FormControls enthaelt (NICHT bei einer Selektion ohne solche oder bei leerer Selektion)
        if ((XForm*)xNewForm.get() != (XForm*)m_xCurForm.get())
            setCurForm( xNewForm );
    }

    setCurControl(rMarkList);

    // wurde vorher Form angezeigt, dann wieder die Form anzeigen
    Reference< XForm> xOldForm(m_xSelObject, UNO_QUERY);
    if (xOldForm.is())
        setSelObject(xNewForm);
    else
        setSelObject(m_xCurControl);

    // Falls ein Propertybrowser angezeigt wird, diesen invalidieren
    if (IsPropBrwOpen() && xPreviousObject != m_xSelObject)
        ShowProperties(m_xSelObject, sal_True);
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::IsPropBrwOpen() const
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    return( ( m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame() ) ?
            m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow(SID_FM_SHOW_PROPERTIES) : sal_False );
}

//------------------------------------------------------------------------------
class FmXFormShell::SuspendPropertyTracking
{
private:
    FmXFormShell*   m_pShell;
    sal_Bool        m_bEnabled;

public:
    SuspendPropertyTracking( FmXFormShell* _pShell )
        :m_pShell( _pShell )
        ,m_bEnabled( sal_False )
    {
        if ( m_pShell && m_pShell->IsTrackPropertiesEnabled() )
        {
            m_pShell->EnableTrackProperties( sal_False );
            m_bEnabled = sal_True;
        }
    }

    ~SuspendPropertyTracking( )
    {
        if ( m_bEnabled )   // note that ( sal_False != m_bEnabled ) implies ( NULL != m_pShell )
            m_pShell->EnableTrackProperties( sal_True );
    }
};

//------------------------------------------------------------------------------
void FmXFormShell::SetDesignMode(sal_Bool bDesign)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
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
        pFormView->GetImpl()->stopMarkListWatching();
    }
    else
    {
        m_aMarkTimer.Stop();

        SuspendPropertyTracking aSuspend( this );
        pFormView->GetImpl()->saveMarkList( sal_True );
    }

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
            Reference< XIndexAccess> xIndex(pPage->GetForms(), UNO_QUERY);
            ResetForms(xIndex, sal_False);
        }
        else
            ResetForms();
    }

    m_pShell->m_bDesignMode = bDesign;
    m_pTextShell->designModeChanged( m_pShell->m_bDesignMode );

    if (bDesign)
    {
        SdrMarkList aList;
        {
            // during changing the mark list, don't track the selected objects in the property browser
            SuspendPropertyTracking aSuspend( this );
            // restore the marks
            pFormView->GetImpl()->restoreMarkList( aList );
        }

        // synchronize with the restored mark list
        if ( aList.GetMarkCount() )
            SetSelection( aList );
    }
    else
    {
        // am Model der View als Listener anmelden (damit ich mitbekomme, wenn jemand waehrend des Alive-Modus
        // Controls loescht, die ich eigentlich mit saveMarkList gespeichert habe) (60343)
        pFormView->GetImpl()->startMarkListWatching();
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
Reference< XControl> FmXFormShell::GetControlFromModel(const Reference< XControlModel>& xModel)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XControlContainer> xControlContainer( getControlContainerForView() );

    Sequence< Reference< XControl> > seqControls( xControlContainer->getControls() );
    Reference< XControl>* pControls = seqControls.getArray();
    // ... die ich dann durchsuchen kann
    for (int i=0; i<seqControls.getLength(); ++i)
    {
        Reference< XControlModel> xSearchLoopModel( pControls[i]->getModel());
        if ((XControlModel*)xSearchLoopModel.get() == (XControlModel*)xModel.get())
            return pControls[i];
    }
    return Reference< XControl>(NULL);
}

//------------------------------------------------------------------------------
void FmXFormShell::CollectFormContexts(const Reference< XInterface>& xStartingPoint, const UniString& sCurrentLevelPrefix, UniString& sNames)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XIndexAccess> xAllComponentIndizies(xStartingPoint, UNO_QUERY);

    if (xAllComponentIndizies.is() && xAllComponentIndizies->getCount())
    {
        // work through all children
        UniString sCurrentFormName, sNextLevelPrefix;
        Reference< XForm> xCurrentAsForm;
        for (sal_Int32 i=0; i<xAllComponentIndizies->getCount(); ++i)
        {
            xAllComponentIndizies->getByIndex(i) >>= xCurrentAsForm;;
            // is the current child a form?
            if (xCurrentAsForm.is())
            {
                Reference< XPropertySet> xAskForName(xCurrentAsForm, UNO_QUERY);
                if (xAskForName.is())
                {
                    try { sCurrentFormName = ::comphelper::getString(xAskForName->getPropertyValue(FM_PROP_NAME)).getStr(); }
                    catch(Exception&)
                    {
                        DBG_ERROR("FmXFormShell::CollectFormContexts: Exception occured!");
                    }
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
Reference< XPropertySet> FmXFormShell::GetBoundField(const Reference< XControl>& _xControl, const Reference< XForm>& _xForm) const
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    // sonderbehandlung fuer databaseGrid
    Reference< XGrid> xGrid(_xControl, UNO_QUERY);
    Reference< XPropertySet> xModel, xField;

    if (xGrid.is())
    {
        sal_Int16 nCurrentPos = xGrid->getCurrentColumnPosition();
        Reference< XIndexAccess> xCols(_xControl->getModel(), UNO_QUERY);
        nCurrentPos = GridView2ModelPos(xCols, nCurrentPos);
        if (nCurrentPos != (sal_Int16)-1)
             xCols->getByIndex(nCurrentPos) >>= xModel;
    }
    else if (_xControl.is())
    {
        xModel = Reference< XPropertySet>(_xControl->getModel(), UNO_QUERY);
    }

    // Das gebundene Feld ist jetzt eine extra Eigenschaft, nicht mehr indirekt ueber die ControlSource zu besorgen,
    // da das Control selber durchaus entscheiden kann, dass es sich trotz gueltiger ControlSource nicht binden will.
    // (zum Beispiel ein TextControl an ein Bild-Feld)
    // (FS - 64265)
    if (xModel.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xModel))
        xModel->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

    return xField;
}

//------------------------------------------------------------------------------
//void FmXFormShell::SetWizardUsing(sal_Bool bUseThem)
//{
//  OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
//
//  m_bUseWizards = bUseThem;
//
//  // forward this to the configuration
//  Sequence< ::rtl::OUString > aNames(1);
//  aNames[0] = ::rtl::OUString::createFromAscii("FormControlPilotsEnabled");
//  Sequence< Any > aValues(1);
//  aValues[0] = bool2any(m_bUseWizards);
//  m_aWizardUsing.SetProperties(aNames, aValues);
//}

//------------------------------------------------------------------------------
void FmXFormShell::startFiltering()
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    // setting all forms in filter mode
    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    Reference< XControlContainer> xContainer;
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
        const ::std::vector< Reference< XFormController> >& rControllerList = (*i)->GetList();
        for (::std::vector< Reference< XFormController> >::const_iterator j = rControllerList.begin();
             j != rControllerList.end(); ++j)
        {
            Reference< XModeSelector> xModeSelector(*j, UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode(FILTER_MODE);
        }
    }

    m_bFilterMode = sal_True;

    m_pShell->UIFeatureChanged();
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}

//------------------------------------------------------------------------------
void saveFilter(const Reference< XFormController>& _rxController)
{
    Reference< XPropertySet> xFormAsSet(_rxController->getModel(), UNO_QUERY);
    Reference< XPropertySet> xControllerAsSet(_rxController, UNO_QUERY);
    Reference< XIndexAccess> xControllerAsIndex(_rxController, UNO_QUERY);

    // call the subcontroller
    Reference< XFormController> xController;
    for (sal_Int32 i = 0, nCount = xControllerAsIndex->getCount(); i < nCount; ++i)
    {
        xControllerAsIndex->getByIndex(i) >>= xController;
        saveFilter(xController);
    }

    try
    {

        xFormAsSet->setPropertyValue(FM_PROP_FILTER_CRITERIA, xControllerAsSet->getPropertyValue(FM_PROP_FILTER_CRITERIA));

        sal_Bool bB(sal_True);
        xFormAsSet->setPropertyValue(FM_PROP_APPLYFILTER, Any(&bB,getBooleanCppuType()));
    }
    catch(Exception&)
    {
        DBG_ERROR("saveFilter: Exception occured!");
    }

}

//------------------------------------------------------------------------------
void FmXFormShell::stopFiltering(sal_Bool bSave)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    m_bFilterMode = sal_False;

    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    Reference< XControlContainer> xContainer;
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
        const ::std::vector< Reference< XFormController> >& rControllerList = (*i)->GetList();
        ::std::vector < ::rtl::OUString >   aOriginalFilters;
        ::std::vector < sal_Bool >          aOriginalApplyFlags;

        if (bSave)
        {
            for (::std::vector< Reference< XFormController> > ::const_iterator j = rControllerList.begin();
                 j != rControllerList.end(); ++j)
            {
                if (bSave)
                {   // remember the current filter settings in case we're goin to reload the forms below (which may fail)
                    try
                    {
                        Reference< XPropertySet > xFormAsSet((*j)->getModel(), UNO_QUERY);
                        aOriginalFilters.push_back(::comphelper::getString(xFormAsSet->getPropertyValue(FM_PROP_FILTER_CRITERIA)));
                        aOriginalApplyFlags.push_back(::comphelper::getBOOL(xFormAsSet->getPropertyValue(FM_PROP_APPLYFILTER)));
                    }
                    catch(Exception&)
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
        for (::std::vector< Reference< XFormController> > ::const_iterator j = rControllerList.begin();
             j != rControllerList.end(); ++j)
        {

            Reference< XModeSelector> xModeSelector(*j, UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode(DATA_MODE);
        }
        if (bSave)  // execute the filter
        {
            const ::std::vector< Reference< XFormController> > & rControllerList = (*i)->GetList();
            for (::std::vector< Reference< XFormController> > ::const_iterator j = rControllerList.begin();
                 j != rControllerList.end(); ++j)
            {
                Reference< XLoadable> xReload((*j)->getModel(), UNO_QUERY);
                if (!xReload.is())
                    continue;
                Reference< XPropertySet > xFormSet(xReload, UNO_QUERY);

                try
                {
                    xReload->reload();
                }
                catch(Exception&)
                {
                    DBG_ERROR("FmXFormShell::stopFiltering: Exception occured!");
                }

                if (!isRowSetAlive(xFormSet))
                {   // something went wrong -> restore the original state
                    ::rtl::OUString sOriginalFilter = aOriginalFilters[ j - rControllerList.begin() ];
                    sal_Bool bOriginalApplyFlag = aOriginalApplyFlags[ j - rControllerList.begin() ];
                    try
                    {
                        xFormSet->setPropertyValue(FM_PROP_FILTER_CRITERIA, makeAny(sOriginalFilter));
                        xFormSet->setPropertyValue(FM_PROP_APPLYFILTER, makeAny(bOriginalApplyFlag));
                        xReload->reload();
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("FmXFormShell::stopFiltering: Exception occured!");
                    }
                }
            }
        }
    }

    m_pShell->UIFeatureChanged();
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}

//------------------------------------------------------------------------------
void clearFilter(const Reference< XFormController>& _rxController)
{
    Reference< XPropertySet> xControllerAsSet(_rxController, UNO_QUERY);
    Reference< XIndexAccess> xControllerAsIndex(_rxController, UNO_QUERY);

    // call the subcontroller
    Reference< XFormController> xController;
    for (sal_Int32 i = 0, nCount = xControllerAsIndex->getCount();
         i < nCount; i++)
    {
        xControllerAsIndex->getByIndex(i) >>= xController;
        clearFilter(xController);
    }

    // clear the filter
    Reference< XIndexContainer> xContainer;
    xControllerAsSet->getPropertyValue(FM_PROP_FILTERSUPPLIER) >>= xContainer;
    if (xContainer.is())
    {
        // clear the current filter
        Sequence< PropertyValue> aCondition;

        // as there is always an empty row, if we have a filter:
        if (xContainer->getCount())
        {
            xControllerAsSet->setPropertyValue(FM_PROP_CURRENTFILTER, makeAny(sal_Int32(xContainer->getCount() - 1)));
            while (xContainer->getCount() > 1)
                xContainer->removeByIndex(0);
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::clearFilter()
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    Reference< XControlContainer> xContainer;
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
        const ::std::vector< Reference< XFormController> > & rControllerList = (*i)->GetList();
        for (::std::vector< Reference< XFormController> > ::const_iterator j = rControllerList.begin();
             j != rControllerList.end(); ++j)
        {
            ::clearFilter(*j);
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::setControlLocks()
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XTabController> xCtrler(getActiveController(), UNO_QUERY);
    if (!xCtrler.is())
        return;

    Reference< XControlContainer> xControls(xCtrler->getContainer(), UNO_QUERY);
    if (!xControls.is())
        return;

    DBG_ASSERT(m_aControlLocks.Count() == 0, "FmXFormShell::setControlLocks : locking state array isn't empty (called me twice ?) !");

    Sequence< Reference< XControl> > aControls = xControls->getControls();
    const Reference< XControl>* pControls = aControls.getConstArray();

    // iterate through all bound controls, remember the old locking state, set the lock
    for (sal_Int32 i=0; i<aControls.getLength(); ++i)
    {
        Reference< XBoundControl> xCtrl(pControls[i], UNO_QUERY);
        if (!xCtrl.is())
        {
            // it may be a container of controls
            Reference< XIndexAccess> xContainer(pControls[i], UNO_QUERY);
            if (xContainer.is())
            {   // no recursion. we only know top level control containers (e.g. grid controls)
                for (sal_Int16 i=0; i<xContainer->getCount(); ++i)
                {
                    xContainer->getByIndex(i) >>= xCtrl;
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    Reference< XTabController> xCtrler(getActiveController(), UNO_QUERY);
    if (!xCtrler.is())
        return;

    Reference< XControlContainer> xControls(xCtrler->getContainer(), UNO_QUERY);
    if (!xControls.is())
        return;

    Sequence< Reference< XControl> > aControls(xControls->getControls());
    const Reference< XControl>* pControls = aControls.getConstArray();

    // iterate through all bound controls, restore the old locking state
    sal_Int32 nBoundControl = 0;
    for (sal_Int32 i=0; i<aControls.getLength(); ++i)
    {
        Reference< XBoundControl> xCtrl(pControls[i], UNO_QUERY);
        if (!xCtrl.is())
        {
            // it may be a container of controls
            Reference< XIndexAccess> xContainer(pControls[i], UNO_QUERY);
            if (xContainer.is())
            {   // no recursion. we only know top level control containers (e.g. grid controls)
                for (sal_Int16 i=0; i<xContainer->getCount(); ++i)
                {
                    xContainer->getByIndex(i) >>= xCtrl;
                    if (!xCtrl.is())
                        continue;

                    DBG_ASSERT(nBoundControl < m_aControlLocks.Count(), "FmXFormShell::restoreControlLocks : m_aControlLocks is invalid !");
                    xCtrl->setLock(m_aControlLocks.GetObject((sal_uInt16)nBoundControl));
                    ++nBoundControl;
                }
            }
            continue;
        }

        DBG_ASSERT(nBoundControl < m_aControlLocks.Count(), "FmXFormShell::restoreControlLocks : m_aControlLocks is invalid !");
            // a violation of this condition would mean a) setControlLocks hasn't been called or b) the ControlContainer
            // has changed since the last call to setControlLocks.
            // a) clearly is a fault of the programmer and b) shouldn't be possible (as we are in alive mode)
        xCtrl->setLock(m_aControlLocks.GetObject((sal_uInt16)nBoundControl));
        ++nBoundControl;
    }
    m_aControlLocks.Remove(0, m_aControlLocks.Count());
}

//------------------------------------------------------------------------------
void FmXFormShell::DoAsyncCursorAction(const Reference< XFormController>& _xController, CURSOR_ACTION _eWhat)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DBG_ASSERT(_xController.is(), "FmXFormShell::DoAsyncCursorAction : invalid argument !");
    DoAsyncCursorAction(Reference< XResultSet>(_xController->getModel(), UNO_QUERY), _eWhat);
}

//------------------------------------------------------------------------------
void FmXFormShell::DoAsyncCursorAction(const Reference< XResultSet>& _xForm, CURSOR_ACTION _eWhat)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    ::osl::MutexGuard aGuard(m_aAsyncSafety);
    DBG_ASSERT(_xForm.is(), "FmXFormShell::DoAsyncCursorAction : invalid argument !");

    // build the access path for the form
    if (HasPendingCursorAction(_xForm))
    {
        DBG_ERROR("FmXFormShell::DoAsyncCursorAction : invalid call !");
        return;
    }

    CursorActionDescription& rDesc = m_aCursorActions[ _xForm ];
        // [] will create a new one if non-existent
    DBG_ASSERT(rDesc.pThread == NULL, "FmXFormShell::DoAsyncCursorAction : the cursor action thread for this form is still alive !");

    Reference< XResultSet > xCursor( getInternalForm( _xForm ), UNO_QUERY );
    switch (_eWhat)
    {
        case CA_MOVE_TO_LAST :
            rDesc.pThread = new FmMoveToLastThread( xCursor );
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
sal_Bool FmXFormShell::HasPendingCursorAction(const Reference< XResultSet>& _xForm) const
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (!_xForm.is())
        return sal_False;

    // TODO: if we ever re-implement the asynchronous cursor actions, then this will happen
    // in the controller, and not in the form. In such a case, we here probably need to check
    // whether the controller for the form has this "pending cursor action"

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::HasPendingCursorAction(const Reference< XFormController>& xController) const
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    if (!xController.is())
        return sal_False;

    return HasPendingCursorAction(Reference< XResultSet>(xController->getModel(), UNO_QUERY));
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::HasAnyPendingCursorAction() const
{
    ::osl::MutexGuard aGuard( const_cast< FmXFormShell* >( this )->m_aAsyncSafety );

    for ( CursorActions::const_iterator aIter = m_aCursorActions.begin(); aIter != m_aCursorActions.end(); ++aIter )
    {
        if (((*aIter).second.pThread != NULL) || ((*aIter).second.nFinishedEvent != 0))
            return sal_True;
    }
    return sal_False;
}

//------------------------------------------------------------------------------
void FmXFormShell::CancelAnyPendingCursorAction()
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    ::comphelper::OReusableGuard< ::osl::Mutex> aGuard(m_aAsyncSafety);

    CursorActions::iterator aIter;
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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    ::osl::MutexGuard aGuard(m_aAsyncSafety);

    // search the pos of the thread within m_aCursorActions
    CursorActions::iterator aIter;
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
IMPL_LINK(FmXFormShell, OnCursorActionDoneMainThread, FmCursorActionThread*, pThread)
{
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    ::osl::MutexGuard aGuard(m_aAsyncSafety);

    // search the pos of the thread within m_aCursorActions
    CursorActions::iterator aIter = m_aCursorActions.find( pThread->getDataSource() );
    DBG_ASSERT(aIter != m_aCursorActions.end(), "FmXFormShell::OnCursorActionDoneMainThread : could not ::std::find the thread data !");
    DBG_ASSERT((*aIter).second.pThread == pThread, "FmXFormShell::OnCursorActionDoneMainThread : invalid thread data !");

    // remember some thread parameters
    Reference< XResultSet> xForm(pThread->getDataSource(), UNO_QUERY);

    // throw away the thread
    delete (*aIter).second.pThread;
    (*aIter).second.pThread = NULL;
    (*aIter).second.nFinishedEvent = 0;
    (*aIter).second.bCanceling = sal_False;
    // as we allow exactly one thread per form we may remove this ones data from m_aCursorActions
    m_aCursorActions.erase(aIter);

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
    OSL_ENSURE(!FmXFormShell_BASE::rBHelper.bDisposed,"FmXFormShell: Object already disposed!");
    DBG_ASSERT(m_xAttachedFrame.is(), "FmXFormShell::CreateExternalView : no frame !");

    // the frame the external view is displayed in
    sal_Bool bAlreadyExistent = m_xExternalViewController.is();
    Reference< ::com::sun::star::frame::XFrame> xExternalViewFrame;
    ::rtl::OUString sFrameName = ::rtl::OUString::createFromAscii("_beamer");
    sal_Int32 nSearchFlags = ::com::sun::star::frame::FrameSearchFlag::CHILDREN | ::com::sun::star::frame::FrameSearchFlag::CREATE;

    Reference< XFormController> xCurrentNavController( getNavController());
        // the creation of the "partwindow" may cause a deactivate of the document which will result in our nav controller to be set to NULL

    // _first_ check if we have any valid fields we can use for the grid view
    // FS - 21.10.99 - 69219
    FmXBoundFormFieldIterator aModelIterator(xCurrentNavController->getModel());
    Reference< XPropertySet> xCurrentModelSet;
    sal_Bool bHaveUsableControls = sal_False;
    while ((xCurrentModelSet = Reference< XPropertySet>(aModelIterator.Next(), UNO_QUERY)).is())
    {
        // the FmXBoundFormFieldIterator only supplies controls with a valid control source
        // so we just have to check the field type
        sal_Int16 nClassId = ::comphelper::getINT16(xCurrentModelSet->getPropertyValue(FM_PROP_CLASSID));
        switch (nClassId)
        {
            case FormComponentType::IMAGECONTROL:
            case FormComponentType::CONTROL:
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
        URL aWantToDispatch;
        aWantToDispatch.Complete = FMURL_COMPONENT_FORMGRIDVIEW;

        Reference< ::com::sun::star::frame::XDispatchProvider> xProv(m_xAttachedFrame, UNO_QUERY);
        Reference< ::com::sun::star::frame::XDispatch> xDisp;
        if (xProv.is())
            xDisp = xProv->queryDispatch(aWantToDispatch, sFrameName, nSearchFlags);
        if (xDisp.is())
        {
            xDisp->dispatch(aWantToDispatch, Sequence< PropertyValue>());
        }

        // with this the component should be loaded, now search the frame where it resides in
        xExternalViewFrame = m_xAttachedFrame->findFrame(sFrameName, ::com::sun::star::frame::FrameSearchFlag::CHILDREN);
        if (xExternalViewFrame.is())
        {
            m_xExternalViewController = xExternalViewFrame->getController();
            Reference< ::com::sun::star::lang::XComponent> xComp(m_xExternalViewController, UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener((XEventListener*)(XPropertyChangeListener*)this);
        }
    }
    else
    {
        xExternalViewFrame = m_xExternalViewController->getFrame();
        Reference< ::com::sun::star::frame::XDispatchProvider> xCommLink(xExternalViewFrame, UNO_QUERY);

        // if we display the active form we interpret the slot as "remove it"
        Reference< XForm> xCurrentModel(xCurrentNavController->getModel(), UNO_QUERY);
        if ((xCurrentModel == m_xExternalDisplayedForm) || (getInternalForm(xCurrentModel) == m_xExternalDisplayedForm))
        {
            if ( m_xExternalViewController == getActiveController() )
            {
                Reference< XFormController > xAsFormController( m_xExternalViewController, UNO_QUERY );
                ControllerFeatures aHelper( ::comphelper::getProcessServiceFactory(), xAsFormController, NULL );
                aHelper->commitCurrentControl();
            }

            Reference< XFormController> xNewController(m_xExtViewTriggerController);
            CloseExternalFormViewer();
            setActiveController(xNewController);
            return;
        }

        URL aClearURL;
        aClearURL.Complete = FMURL_GRIDVIEW_CLEARVIEW;

        Reference< ::com::sun::star::frame::XDispatch> xClear( xCommLink->queryDispatch(aClearURL, ::rtl::OUString::createFromAscii(""), 0));
        if (xClear.is())
            xClear->dispatch(aClearURL, Sequence< PropertyValue>());
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
        Reference< ::com::sun::star::frame::XDispatchProviderInterception> xSupplier(xExternalViewFrame, UNO_QUERY);
        ::rtl::OUString sInterceptorScheme = FMURL_FORMSLOTS_PREFIX;
        sInterceptorScheme += ::rtl::OUString::createFromAscii("*");
//      m_pExternalViewInterceptor = new FmXDispatchInterceptorImpl(xSupplier, this, 1, Sequence< ::rtl::OUString >(&sInterceptorScheme, 1));
//      m_pExternalViewInterceptor->acquire();
        // TODO: re-implement this in a easier way than before: We need an interceptor at the xSupplier, which
        // forwards all queryDispatch requests to the FormController instance for which this "external view"
        // was triggered
    }

    // get the dispatch interface of the frame so we can communicate (interceptable) with the controller
    Reference< ::com::sun::star::frame::XDispatchProvider> xCommLink(xExternalViewFrame, UNO_QUERY);

    if (m_xExternalViewController.is())
    {
        DBG_ASSERT(xCommLink.is(), "FmXFormShell::CreateExternalView : the component doesn't have the necessary interfaces !");
        // collect the dispatchers we will need
        URL aAddColumnURL;
        aAddColumnURL.Complete = FMURL_GRIDVIEW_ADDCOLUMN;
        Reference< ::com::sun::star::frame::XDispatch> xAddColumnDispatch( xCommLink->queryDispatch(aAddColumnURL, ::rtl::OUString::createFromAscii(""), 0));
        URL aAttachURL;
        aAttachURL.Complete = FMURL_GRIDVIEW_ATTACHTOFORM;
        Reference< ::com::sun::star::frame::XDispatch> xAttachDispatch( xCommLink->queryDispatch(aAttachURL, ::rtl::OUString::createFromAscii(""), 0));

        if (xAddColumnDispatch.is() && xAttachDispatch.is())
        {
            DBG_ASSERT(xCurrentNavController.is(), "FmXFormShell::CreateExternalView : invalid call : have no nav controller !");
            // first : dispatch the descriptions for the columns to add
            Sequence< Reference< XControl> > aCurrentControls(xCurrentNavController->getControls());
            const Reference< XControl>* pCurrentControl = aCurrentControls.getConstArray();

            sal_Int16 nAddedColumns = 0;

            // for radio buttons we need some special structures
            DECLARE_STL_USTRINGACCESS_MAP(Sequence< ::rtl::OUString>, MapUString2UstringSeq);
            DECLARE_STL_ITERATORS(MapUString2UstringSeq);
            DECLARE_STL_USTRINGACCESS_MAP(::rtl::OUString, FmMapUString2UString);
            DECLARE_STL_USTRINGACCESS_MAP(sal_Int16, FmMapUString2Int16);
            DECLARE_STL_ITERATORS(FmMapUString2Int16);

            MapUString2UstringSeq   aRadioValueLists;
            MapUString2UstringSeq   aRadioListSources;
            FmMapUString2UString    aRadioControlSources;
            FmMapUString2Int16      aRadioPositions;

            FmXBoundFormFieldIterator aModelIterator(xCurrentNavController->getModel());
            Reference< XPropertySet> xCurrentModelSet;
            Any aCurrentBoundField;
            ::rtl::OUString sColumnType,aGroupName,sControlSource;
            Sequence< Property> aProps;
            Reference< XPropertySet> xCurrentBoundField;
            while ((xCurrentModelSet = Reference< XPropertySet>(aModelIterator.Next(), UNO_QUERY)).is())
            {
                xCurrentModelSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xCurrentBoundField;
                OSL_ENSURE(xCurrentModelSet.is(),"xCurrentModelSet is null!");
                // create a description of the column to be created
                // first : determine it's type

                sal_Int16 nClassId = ::comphelper::getINT16(xCurrentModelSet->getPropertyValue(FM_PROP_CLASSID));
                switch (nClassId)
                {
                    case FormComponentType::RADIOBUTTON:
                    {
                        // get the label of the button (this is the access key for our structures)
                        aGroupName = getLabelName(xCurrentModelSet);

                        // add the reference value of the radio button to the list source sequence
                        Sequence< ::rtl::OUString>& aThisGroupLabels = aRadioListSources[aGroupName];
                        sal_Int32 nNewSizeL = aThisGroupLabels.getLength() + 1;
                        aThisGroupLabels.realloc(nNewSizeL);
                        aThisGroupLabels.getArray()[nNewSizeL - 1] = ::comphelper::getString(xCurrentModelSet->getPropertyValue(FM_PROP_REFVALUE));

                        // add the label to the value list sequence
                        Sequence< ::rtl::OUString>& aThisGroupControlSources = aRadioValueLists[aGroupName];
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

                    case FormComponentType::IMAGECONTROL:
                    case FormComponentType::CONTROL:
                        // no grid columns for these types (though they have a control source)
                        continue;
                    case FormComponentType::CHECKBOX:
                        sColumnType = FM_COL_CHECKBOX; break;
                    case FormComponentType::LISTBOX:
                        sColumnType = FM_COL_LISTBOX; break;
                    case FormComponentType::COMBOBOX:
                        sColumnType = FM_COL_COMBOBOX; break;
                    case FormComponentType::DATEFIELD:
                        sColumnType = FM_COL_DATEFIELD; break;
                    case FormComponentType::TIMEFIELD:
                        sColumnType = FM_COL_TIMEFIELD; break;
                    case FormComponentType::NUMERICFIELD:
                        sColumnType = FM_COL_NUMERICFIELD; break;
                    case FormComponentType::CURRENCYFIELD:
                        sColumnType = FM_COL_CURRENCYFIELD; break;
                    case FormComponentType::PATTERNFIELD:
                        sColumnType = FM_COL_PATTERNFIELD; break;

                    case FormComponentType::TEXTFIELD:
                        {
                            sColumnType = FM_COL_TEXTFIELD;
                            // we know at least two different controls which are TextFields : the basic edit field and the formatted
                            // field. we distinguish them by their service name
                            Reference< XServiceInfo> xInfo(xCurrentModelSet, UNO_QUERY);
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
                Sequence< PropertyValue> aDispatchArgs(nDispatchArgs);
                PropertyValue* pDispatchArgs = aDispatchArgs.getArray();

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
                Sequence< PropertyValue> aColumnProps(1);
                PropertyValue* pColumnProps = aColumnProps.getArray();

                // the label
                pColumnProps->Name = FM_PROP_LABEL;
                pColumnProps->Value <<= getLabelName(xCurrentModelSet);
                ++pColumnProps;

                // for all other props : transfer them
                Reference< XPropertySetInfo> xControlModelInfo( xCurrentModelSet->getPropertySetInfo());
                DBG_ASSERT(xControlModelInfo.is(), "FmXFormShell::CreateExternalView : the control model has no property info ! This will crash !");
                aProps = xControlModelInfo->getProperties();
                const Property* pProps = aProps.getConstArray();

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
                    if (pProps->Attributes & PropertyAttribute::READONLY)
                        // assume that properties which are readonly for the control are ro for the column to be created, too
                        continue;

                    pColumnProps->Name = pProps->Name;
                    pColumnProps->Value = xCurrentModelSet->getPropertyValue(pProps->Name);
                    ++pColumnProps;
                }
                aColumnProps.realloc(pColumnProps - aColumnProps.getArray());

                // columns props are a dispatch argument
                pDispatchArgs->Name = ::rtl::OUString::createFromAscii("ColumnProperties"); // TODO : fmurl.*
                pDispatchArgs->Value = makeAny(aColumnProps);
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
            Sequence< PropertyValue> aListBoxDescription(nListBoxDescription);
            for (   ConstFmMapUString2UStringIterator aCtrlSource = aRadioControlSources.begin();
                    aCtrlSource != aRadioControlSources.end();
                    ++aCtrlSource, ++nOffset
                )
            {

                PropertyValue* pListBoxDescription = aListBoxDescription.getArray();
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
                 ListSourceType eType = ListSourceType_VALUELIST;
                 pListBoxDescription->Value = makeAny(eType);
                ++pListBoxDescription;

                // list source
                MapUString2UstringSeq::const_iterator aCurrentListSource = aRadioListSources.find((*aCtrlSource).first);
                DBG_ASSERT(aCurrentListSource != aRadioListSources.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                pListBoxDescription->Name = FM_PROP_LISTSOURCE;
                pListBoxDescription->Value = makeAny((*aCurrentListSource).second);
                ++pListBoxDescription;

                // value list
                MapUString2UstringSeq::const_iterator aCurrentValueList = aRadioValueLists.find((*aCtrlSource).first);
                DBG_ASSERT(aCurrentValueList != aRadioValueLists.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                pListBoxDescription->Name = FM_PROP_STRINGITEMLIST;
                pListBoxDescription->Value = makeAny(((*aCurrentValueList).second));
                ++pListBoxDescription;

                DBG_ASSERT(nListBoxDescription == (pListBoxDescription - aListBoxDescription.getConstArray()),
                    "FmXFormShell::CreateExternalView : forgot to adjust nListBoxDescription ?");

                // properties describing the column "meta data"
                const sal_Int16 nDispatchArgs = 3;
                Sequence< PropertyValue> aDispatchArgs(nDispatchArgs);
                PropertyValue* pDispatchArgs = aDispatchArgs.getArray();

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
                pDispatchArgs->Value = makeAny(aListBoxDescription);
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
            PropertyValue aArg;
            aArg.Name = FMARG_ATTACHTO_MASTERFORM;
            Reference< XResultSet> xForm(xCurrentNavController->getModel(), UNO_QUERY);
            aArg.Value <<= xForm;

            m_xExternalDisplayedForm = Reference< XResultSet>(xForm, UNO_QUERY);
                // do this before dispatching the "attach" command, as the atach may result in a call to our queryDispatch (for the FormSlots)
                // whichs needs the m_xExternalDisplayedForm

            xAttachDispatch->dispatch(aAttachURL, Sequence< PropertyValue>(&aArg, 1));

            m_xExtViewTriggerController = xCurrentNavController;

            // we want to know modifications done in the external view
            // if the external controller is a XFormController we can use all our default handlings for it
            Reference< XFormController> xFormController(m_xExternalViewController, UNO_QUERY);
            if (xFormController.is())
                xFormController->addActivateListener((XFormControllerListener*)this);
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

//------------------------------------------------------------------------
void FmXFormShell::implAdjustConfigCache()
{
    // get (cache) the wizard usage flag
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii("FormControlPilotsEnabled");
    Sequence< Any > aFlags = GetProperties(aNames);
    if (1 == aFlags.getLength())
        m_bUseWizards = ::cppu::any2bool(aFlags[0]);
}

//------------------------------------------------------------------------
void FmXFormShell::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames)
{
    const ::rtl::OUString* pSearch = _rPropertyNames.getConstArray();
    const ::rtl::OUString* pSearchTil = pSearch + _rPropertyNames.getLength();
    for (;pSearch < pSearchTil; ++pSearch)
        if (0 == pSearch->compareToAscii("FormControlPilotsEnabled"))
        {
            implAdjustConfigCache();
            InvalidateSlot(SID_FM_USE_WIZARDS, sal_True, sal_True);
        }
}

//------------------------------------------------------------------------
void FmXFormShell::SetWizardUsing(sal_Bool _bUseThem)
{
    m_bUseWizards = _bUseThem;

    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii("FormControlPilotsEnabled");
    Sequence< Any > aValues(1);
    aValues[0] = ::cppu::bool2any(m_bUseWizards);
    PutProperties(aNames, aValues);
}

//------------------------------------------------------------------------
void FmXFormShell::viewDeactivated( FmFormView* _pCurrentView, sal_Bool _bDeactivateController /* = sal_True */ )
{
    // deactivate our view if we are deactivated ourself
    // FS - 30.06.99 - 67308
    if ( _pCurrentView )
    {
        // deactivate the controls on this page
        if ( _pCurrentView->GetImpl() && !_pCurrentView->IsDesignMode() )
        {
            _pCurrentView->GetImpl()->Deactivate( _bDeactivateController );
        }

        // if we have an async load operation pending for the 0-th page for this view,
        // we need to cancel this
        // 103727 - 2002-09-26 - fs@openoffice.org
        const SdrPageView* pCurPageView = _pCurrentView->GetPageViewPvNum( 0 );
        const FmFormPage* pPage = pCurPageView ? PTR_CAST( FmFormPage, pCurPageView->GetPage() ) : NULL;
        if ( pPage )
        {
            // move all events from our queue to a new one, omit the events for the deactivated
            // page
            ::std::queue< FmLoadAction > aNewEvents;
            while ( m_aLoadingPages.size() )
            {
                FmLoadAction aAction = m_aLoadingPages.front();
                m_aLoadingPages.pop();
                if ( pPage != aAction.pPage )
                {
                    aNewEvents.push( aAction );
                }
                else
                {
                    Application::RemoveUserEvent( aAction.nEventId );
                }
            }
            m_aLoadingPages = aNewEvents;
        }
    }
}

//------------------------------------------------------------------------
void FmXFormShell::viewActivated( FmFormView* _pCurrentView, sal_Bool _bSyncAction /* = sal_False */ )
{
    // activate our view if we are activated ourself
    // FS - 30.06.99 - 67308
    if ( _pCurrentView && _pCurrentView->GetImpl() && !_pCurrentView->IsDesignMode() )
    {
        // load forms for the page the current view belongs to
        SdrPageView* pCurPageView = _pCurrentView->GetPageViewPvNum( 0 );
        FmFormPage* pPage = pCurPageView ? PTR_CAST( FmFormPage, pCurPageView->GetPage() ) : NULL;
        if ( pPage )
        {
            if ( !pPage->GetImpl()->hasEverBeenActivated() )
                loadForms( pPage, FORMS_LOAD | ( _bSyncAction ? FORMS_SYNC : FORMS_ASYNC ) );
            pPage->GetImpl()->setHasBeenActivated( );
        }

        // first-time initializations for the views
        if ( !_pCurrentView->GetImpl()->hasEverBeenActivated( ) )
        {
            _pCurrentView->GetImpl()->onFirstViewActivation( PTR_CAST( FmFormModel, _pCurrentView->GetModel() ) );
            _pCurrentView->GetImpl()->setHasBeenActivated( );
        }

        // activate the current view
        _pCurrentView->GetImpl()->Activate( _bSyncAction );
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::smartControlReset( const Reference< XIndexAccess >& _rxModels )
{
    if (!_rxModels.is())
    {
        DBG_ERROR("FmXFormShell::smartControlReset: invalid container!");
        return;
    }

    static const ::rtl::OUString sClassIdPropertyName = FM_PROP_CLASSID;
    static const ::rtl::OUString sBoundFieldPropertyName = FM_PROP_BOUNDFIELD;
    sal_Int32 nCount = _rxModels->getCount();
    Reference< XPropertySet > xCurrent;
    Reference< XPropertySetInfo > xCurrentInfo;
    Reference< XPropertySet > xBoundField;

    for (sal_Int32 i=0; i<nCount; ++i)
    {
        _rxModels->getByIndex(i) >>= xCurrent;
        if (xCurrent.is())
            xCurrentInfo = xCurrent->getPropertySetInfo();
        else
            xCurrentInfo.clear();
        if (!xCurrentInfo.is())
            continue;

        if (xCurrentInfo->hasPropertyByName(sClassIdPropertyName))
        {   // it's a control model

            // check if this control is bound to a living database field
            if (xCurrentInfo->hasPropertyByName(sBoundFieldPropertyName))
                xCurrent->getPropertyValue(sBoundFieldPropertyName) >>= xBoundField;
            else
                xBoundField.clear();

            // reset only if it's *not* bound
            bool bReset = !xBoundField.is();

            // and additionally, check if it has an external value binding
            Reference< XBindableValue > xBindable( xCurrent, UNO_QUERY );
            if ( xBindable.is() && xBindable->getValueBinding().is() )
                bReset = false;

            if ( bReset )
            {
                Reference< XReset > xControlReset( xCurrent, UNO_QUERY );
                if ( xControlReset.is() )
                    xControlReset->reset();
            }
        }
        else
        {
            Reference< XIndexAccess > xContainer(xCurrent, UNO_QUERY);
            if (xContainer.is())
                smartControlReset(xContainer);
        }
    }
}

//------------------------------------------------------------------------
IMPL_LINK( FmXFormShell, OnLoadForms, FmFormPage*, _pPage )
{
    FmLoadAction aAction = m_aLoadingPages.front();
    m_aLoadingPages.pop();

    loadForms( aAction.pPage, aAction.nFlags & ~FORMS_ASYNC );
    return 0L;
}

//------------------------------------------------------------------------
void FmXFormShell::loadForms( FmFormPage* _pPage, const sal_uInt16 _nBehaviour /* FORMS_LOAD | FORMS_SYNC */ )
{
    DBG_ASSERT( ( _nBehaviour & ( FORMS_ASYNC | FORMS_UNLOAD ) )  != ( FORMS_ASYNC | FORMS_UNLOAD ),
        "FmXFormShell::loadForms: async loading not supported - this will heavily fail!" );

    if ( _nBehaviour & FORMS_ASYNC )
    {
        m_aLoadingPages.push( FmLoadAction(
            _pPage,
            _nBehaviour,
            Application::PostUserEvent( LINK( this, FmXFormShell, OnLoadForms ), _pPage )
        ) );
        return;
    }

    DBG_ASSERT( _pPage, "FmXFormShell::loadForms: invalid page!" );
    if ( _pPage )
    {
        // lock the undo env so the forms can change non-transient properties while loading
        // (without this my doc's modified flag would be set)
        FmFormModel* pModel = PTR_CAST( FmFormModel, _pPage->GetModel() );
        DBG_ASSERT( pModel, "FmXFormShell::loadForms: invalid model!" );
        if ( pModel )
            pModel->GetUndoEnv().Lock();

        // load all forms
        Reference< XIndexAccess >  xForms;
        xForms = xForms.query( _pPage->GetForms() );
        DBG_ASSERT( xForms.is(), "FmXFormShell::loadForms: invalid forms collection!" );

        if ( xForms.is() )
        {
            Reference< XLoadable >  xForm;
            sal_Bool                bFormWasLoaded = sal_False;
            for ( sal_Int32 j = 0, nCount = xForms->getCount(); j < nCount; ++j )
            {
                xForms->getByIndex( j ) >>= xForm;
                bFormWasLoaded = sal_False;
                // a database form must be loaded for
                if ( 0 == ( _nBehaviour & FORMS_UNLOAD ) )
                {
                    if ( ::isLoadable( xForm ) && !xForm->isLoaded() )
                        xForm->load();
                }
                else
                {
                    if ( xForm->isLoaded() )
                    {
                        bFormWasLoaded = sal_True;
                        xForm->unload();
                    }
                }

                // reset the form if it was loaded
                if ( bFormWasLoaded )
                {
                    Reference< XIndexAccess > xContainer( xForm, UNO_QUERY );
                    DBG_ASSERT( xContainer.is(), "FmXFormShell::loadForms: the form is no container!" );
                    if ( xContainer.is() )
                        smartControlReset( xContainer );
                }
            }
        }

        if ( pModel )
            // unlock the environment
            pModel->GetUndoEnv().UnLock();
    }
}

//------------------------------------------------------------------------
void FmXFormShell::ExecuteTextAttribute( SfxRequest& _rReq )
{
    m_pTextShell->ExecuteTextAttribute( _rReq );
}

//------------------------------------------------------------------------
void FmXFormShell::GetTextAttributeState( SfxItemSet& _rSet )
{
    m_pTextShell->GetTextAttributeState( _rSet );
}

//------------------------------------------------------------------------
bool FmXFormShell::IsActiveControl( bool _bCountRichTextOnly ) const
{
    return m_pTextShell->IsActiveControl( _bCountRichTextOnly );
}

//------------------------------------------------------------------------
void FmXFormShell::ForgetActiveControl()
{
    m_pTextShell->ForgetActiveControl();
}

//------------------------------------------------------------------------
void FmXFormShell::SetControlActivationHandler( const Link& _rHdl )
{
    m_pTextShell->SetControlActivationHandler( _rHdl );
}

//==============================================================================
//==============================================================================
SearchableControlIterator::SearchableControlIterator(Reference< XInterface> xStartingPoint)
    :IndexAccessIterator(xStartingPoint)
{
}

//------------------------------------------------------------------------------
sal_Bool SearchableControlIterator::ShouldHandleElement(const Reference< XInterface>& xElement)
{
    // wenn das Ding eine ControlSource und einen BoundField-Property hat
    Reference< XPropertySet> xProperties(xElement, UNO_QUERY);
    if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xProperties) && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xProperties))
    {
        // und das BoundField gueltig ist
        Reference< XPropertySet> xField;
        xProperties->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;
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
        Any aClassId( xProperties->getPropertyValue(FM_PROP_CLASSID) );
        if (::comphelper::getINT16(aClassId) == FormComponentType::GRIDCONTROL)
        {
            m_sCurrentValue = ::rtl::OUString();
            return sal_True;
        }
    }

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool SearchableControlIterator::ShouldStepInto(const Reference< XInterface>& xContainer) const
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

FmCursorActionThread::FmCursorActionThread(const Reference< XResultSet>& _xDataSource, const UniString& _rStopperCaption)
    :m_xDataSource(_xDataSource)
    ,m_sStopperCaption(_rStopperCaption)
    ,m_bCanceled(sal_False)
    ,m_bDeleteMyself(sal_False)
    ,m_bDisposeCursor(sal_False)
    ,m_bTerminated(sal_False)
    ,m_bRunFailed(sal_False)
{
    DBG_ASSERT(m_xDataSource.is() && Reference< XCancellable>(m_xDataSource, UNO_QUERY).is(),
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
    catch(SQLException e)
    {
        ::osl::MutexGuard aErrorGuard(m_aAccessSafety);
        m_bRunFailed = sal_True;
        m_aRunException = e;
    }
    catch(Exception&)
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
        Reference< ::com::sun::star::lang::XComponent> xDataSourceComponent(m_xDataSource, UNO_QUERY);
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

    Reference< XCancellable> xCancel(m_xDataSource, UNO_QUERY);
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
