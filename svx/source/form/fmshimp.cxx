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


#include <sal/macros.h>
#include "fmitems.hxx"
#include "fmobj.hxx"
#include "fmpgeimp.hxx"
#include "svx/fmtools.hxx"
#include "fmprop.hrc"
#include "svx/fmresids.hrc"
#include "fmservs.hxx"
#include "fmshimp.hxx"
#include "fmtextcontrolshell.hxx"
#include "fmundo.hxx"
#include "fmurl.hxx"
#include "fmvwimp.hxx"
#include "formtoolbars.hxx"
#include "gridcols.hxx"
#include "svx/svditer.hxx"
#include "svx/dialmgr.hxx"
#include "svx/dialogs.hrc"
#include "svx/fmglob.hxx"
#include "svx/fmmodel.hxx"
#include "svx/fmpage.hxx"
#include "svx/fmshell.hxx"
#include "svx/obj3d.hxx"
#include "svx/sdrpagewindow.hxx"
#include "svx/svdpagv.hxx"
#include "svx/svxdlg.hxx"
#include "svx/svxids.hrc"

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/form/TabOrderDialog.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/form/XBoundControl.hpp>
#include <com/sun/star/form/XGrid.hpp>
#include <com/sun/star/form/XGridPeer.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/util/XModeSelector.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/evtmethodhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/solarmutex.hxx>
#include <comphelper/string.hxx>
#include <connectivity/dbtools.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

// wird fuer Invalidate verwendet -> mitpflegen
static sal_uInt16 DatabaseSlotMap[] =
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
    SID_FM_REFRESH_FORM_CONTROL,
    SID_FM_SEARCH,
    SID_FM_FILTER_START,
    SID_FM_VIEW_AS_GRID,
    0
};

// wird fuer Invalidate verwendet -> mitpflegen
// aufsteigend sortieren !!!!!!
static sal_Int16 DlgSlotMap[] =    // slots des Controllers
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
    SID_FM_SHOW_DATANAVIGATOR,
    SID_FM_DATANAVIGATOR_CONTROL,
    0
};

static sal_Int16 SelObjectSlotMap[] =  // vom SelObject abhaengige Slots
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
    SID_FM_DATANAVIGATOR_CONTROL,

    0
};

// die folgenden Arrays muessen kosistent sein, also einander entsprechende Eintraege an der selben relativen Position
// innerhalb ihres jeweiligen Arrays stehen
static sal_Int16 nConvertSlots[] =
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

static sal_Int16 nCreateSlots[] =
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

static sal_Int16 nObjectTypes[] =
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::form::binding;
using namespace ::com::sun::star::form::runtime;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;
using namespace ::svxform;
using namespace ::svx;

//==============================================================================
//= helper
//==============================================================================
namespace
{
    //..........................................................................
    void collectInterfacesFromMarkList( const SdrMarkList& _rMarkList, InterfaceBag& /* [out] */ _rInterfaces )
    {
        _rInterfaces.clear();

        sal_uInt32 nMarkCount = _rMarkList.GetMarkCount();
        for ( sal_uInt32 i = 0; i < nMarkCount; ++i)
        {
            SdrObject* pCurrent = _rMarkList.GetMark( i )->GetMarkedSdrObj();

            SdrObjListIter* pGroupIterator = NULL;
            if ( pCurrent->IsGroupObject() )
            {
                pGroupIterator = new SdrObjListIter( *pCurrent->GetSubList() );
                pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
            }

            while ( pCurrent )
            {
                FmFormObj* pAsFormObject = FmFormObj::GetFormObject( pCurrent );
                    // note this will de-reference virtual objects, if necessary/possible
                if ( pAsFormObject )
                {
                    Reference< XInterface > xControlModel( pAsFormObject->GetUnoControlModel(), UNO_QUERY );
                        // the UNO_QUERY is important for normalization
                    if ( xControlModel.is() )
                        _rInterfaces.insert( xControlModel );
                }

                // next element
                pCurrent = pGroupIterator && pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
            }

            if ( pGroupIterator )
                delete pGroupIterator;
        }
    }

    //..........................................................................
    sal_Int16 GridView2ModelPos(const Reference< XIndexAccess>& rColumns, sal_Int16 nViewPos)
    {
        try
        {
            if (rColumns.is())
            {
                // loop through all columns
                sal_Int16 i;
                Reference< XPropertySet> xCur;
                for (i=0; i<rColumns->getCount(); ++i)
                {
                    rColumns->getByIndex(i) >>= xCur;
                    if (!::comphelper::getBOOL(xCur->getPropertyValue(FM_PROP_HIDDEN)))
                    {
                        // for every visible col : if nViewPos is greater zero, decrement it, else we
                        // have found the model position
                        if (!nViewPos)
                            break;
                        else
                            --nViewPos;
                    }
                }
                if (i<rColumns->getCount())
                    return i;
            }
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return (sal_Int16)-1;
    }

    //..........................................................................
    void TransferEventScripts(const Reference< XControlModel>& xModel, const Reference< XControl>& xControl,
        const Sequence< ScriptEventDescriptor>& rTransferIfAvailable)
    {
        // first check if we have a XEventAttacherManager for the model
        Reference< XChild> xModelChild(xModel, UNO_QUERY);
        if (!xModelChild.is())
            return; // nothing to do

        Reference< XEventAttacherManager> xEventManager(xModelChild->getParent(), UNO_QUERY);
        if (!xEventManager.is())
            return; // nothing to do

        if (!rTransferIfAvailable.getLength())
            return; // nothing to do

        // check for the index of the model within it's parent
        Reference< XIndexAccess> xParentIndex(xModelChild->getParent(), UNO_QUERY);
        if (!xParentIndex.is())
            return; // nothing to do
        sal_Int32 nIndex = getElementPos(xParentIndex, xModel);
        if (nIndex<0 || nIndex>=xParentIndex->getCount())
            return; // nothing to do

        // then we need information about the listeners supported by the control and the model
        Sequence< Type> aModelListeners;
        Sequence< Type> aControlListeners;

        Reference< XIntrospection> xModelIntrospection = Introspection::create(::comphelper::getProcessComponentContext());
        Reference< XIntrospection> xControlIntrospection = Introspection::create(::comphelper::getProcessComponentContext());

        if (xModel.is())
        {
            Any aModel(makeAny(xModel));
            aModelListeners = xModelIntrospection->inspect(aModel)->getSupportedListeners();
        }

        if (xControl.is())
        {
            Any aControl(makeAny(xControl));
            aControlListeners = xControlIntrospection->inspect(aControl)->getSupportedListeners();
        }

        sal_Int32 nMaxNewLen = aModelListeners.getLength() + aControlListeners.getLength();
        if (!nMaxNewLen)
            return; // the model and the listener don't support any listeners (or we were unable to retrieve these infos)

        Sequence< ScriptEventDescriptor>    aTransferable(nMaxNewLen);
        ScriptEventDescriptor* pTransferable = aTransferable.getArray();

        const ScriptEventDescriptor* pCurrent = rTransferIfAvailable.getConstArray();
        sal_Int32 i,j,k;
        for (i=0; i<rTransferIfAvailable.getLength(); ++i, ++pCurrent)
        {
            // search the model/control idl classes for the event described by pCurrent
            for (   Sequence< Type>* pCurrentArray = &aModelListeners;
                    pCurrentArray;
                    pCurrentArray = (pCurrentArray == &aModelListeners) ? &aControlListeners : NULL
                )
            {
                const Type* pCurrentListeners = pCurrentArray->getConstArray();
                for (j=0; j<pCurrentArray->getLength(); ++j, ++pCurrentListeners)
                {
                    OUString aListener = (*pCurrentListeners).getTypeName();
                    sal_Int32 nTokens = comphelper::string::getTokenCount(aListener, '.');
                    if (nTokens)
                        aListener = comphelper::string::getToken(aListener, nTokens - 1, '.');

                    if (aListener == pCurrent->ListenerType.getStr())
                        // the current ScriptEventDescriptor doesn't match the current listeners class
                        continue;

                    // now check the methods
                    Sequence< OUString> aMethodsNames = ::comphelper::getEventMethodsForType(*pCurrentListeners);

                    const OUString* pMethodsNames = aMethodsNames.getConstArray();
                    for (k=0; k<aMethodsNames.getLength(); ++k, ++pMethodsNames)
                    {
                        if ((*pMethodsNames).compareTo(pCurrent->EventMethod) != COMPARE_EQUAL)
                            // the current ScriptEventDescriptor doesn't match the current listeners current method
                            continue;

                        // we can transfer the script event : the model (control) supports it
                        *pTransferable = *pCurrent;
                        ++pTransferable;
                        break;
                    }
                    if (k<aMethodsNames.getLength())
                        break;
                }
            }
        }

        sal_Int32 nRealNewLen = pTransferable - aTransferable.getArray();
        aTransferable.realloc(nRealNewLen);

        xEventManager->registerScriptEvents(nIndex, aTransferable);
    }

    //------------------------------------------------------------------------------
    OUString getServiceNameByControlType(sal_Int16 nType)
    {
        switch (nType)
        {
            case OBJ_FM_EDIT            : return FM_COMPONENT_TEXTFIELD;
            case OBJ_FM_BUTTON          : return FM_COMPONENT_COMMANDBUTTON;
            case OBJ_FM_FIXEDTEXT       : return FM_COMPONENT_FIXEDTEXT;
            case OBJ_FM_LISTBOX         : return FM_COMPONENT_LISTBOX;
            case OBJ_FM_CHECKBOX        : return FM_COMPONENT_CHECKBOX;
            case OBJ_FM_RADIOBUTTON     : return FM_COMPONENT_RADIOBUTTON;
            case OBJ_FM_GROUPBOX        : return FM_COMPONENT_GROUPBOX;
            case OBJ_FM_COMBOBOX        : return FM_COMPONENT_COMBOBOX;
            case OBJ_FM_GRID            : return FM_COMPONENT_GRIDCONTROL;
            case OBJ_FM_IMAGEBUTTON     : return FM_COMPONENT_IMAGEBUTTON;
            case OBJ_FM_FILECONTROL     : return FM_COMPONENT_FILECONTROL;
            case OBJ_FM_DATEFIELD       : return FM_COMPONENT_DATEFIELD;
            case OBJ_FM_TIMEFIELD       : return FM_COMPONENT_TIMEFIELD;
            case OBJ_FM_NUMERICFIELD    : return FM_COMPONENT_NUMERICFIELD;
            case OBJ_FM_CURRENCYFIELD   : return FM_COMPONENT_CURRENCYFIELD;
            case OBJ_FM_PATTERNFIELD    : return FM_COMPONENT_PATTERNFIELD;
            case OBJ_FM_HIDDEN          : return FM_COMPONENT_HIDDENCONTROL;
            case OBJ_FM_IMAGECONTROL    : return FM_COMPONENT_IMAGECONTROL;
            case OBJ_FM_FORMATTEDFIELD  : return FM_COMPONENT_FORMATTEDFIELD;
            case OBJ_FM_SCROLLBAR       : return FM_SUN_COMPONENT_SCROLLBAR;
            case OBJ_FM_SPINBUTTON      : return FM_SUN_COMPONENT_SPINBUTTON;
            case OBJ_FM_NAVIGATIONBAR   : return FM_SUN_COMPONENT_NAVIGATIONBAR;
        }
        return OUString();
    }

}

//------------------------------------------------------------------------------
// check if the control has one of the interfaces we can use for searching
// *_pCurrentText will be filled with the current text of the control (as used when searching this control)
sal_Bool IsSearchableControl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxControl,
    OUString* _pCurrentText )
{
    if ( !_rxControl.is() )
        return sal_False;

    Reference< XTextComponent > xAsText( _rxControl, UNO_QUERY );
    if ( xAsText.is() )
    {
        if ( _pCurrentText )
            *_pCurrentText = xAsText->getText();
        return sal_True;
    }

    Reference< XListBox > xListBox( _rxControl, UNO_QUERY );
    if ( xListBox.is() )
    {
        if ( _pCurrentText )
            *_pCurrentText = xListBox->getSelectedItem();
        return sal_True;
    }

    Reference< XCheckBox > xCheckBox( _rxControl, UNO_QUERY );
    if ( xCheckBox.is() )
    {
        if ( _pCurrentText )
        {
            switch ( (::TriState)xCheckBox->getState() )
            {
                case STATE_NOCHECK: *_pCurrentText = OUString("0" ); break;
                case STATE_CHECK: *_pCurrentText = OUString("1" ); break;
                default: *_pCurrentText = OUString(); break;
            }
        }
        return sal_True;
    }

    return sal_False;
}

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

//------------------------------------------------------------------------------
sal_Bool isControlList(const SdrMarkList& rMarkList)
{
    // enthaelt die liste nur Controls und mindestens ein control
    sal_uInt32 nMarkCount = rMarkList.GetMarkCount();
    sal_Bool  bControlList = nMarkCount != 0;

    sal_Bool bHadAnyLeafs = sal_False;

    for (sal_uInt32 i = 0; i < nMarkCount && bControlList; i++)
    {
        SdrObject *pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
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
        {
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
    }

    return bControlList && bHadAnyLeafs;
}

//------------------------------------------------------------------------
Reference< XForm > GetForm(const Reference< XInterface>& _rxElement)
{
    Reference< XForm > xForm( _rxElement, UNO_QUERY );
    if ( xForm.is() )
        return xForm;

    Reference< XChild > xChild( _rxElement, UNO_QUERY );
    if ( xChild.is() )
        return GetForm( xChild->getParent() );

    return Reference< XForm >();
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
FmXFormShell::FmXFormShell( FmFormShell& _rShell, SfxViewFrame* _pViewFrame )
        :FmXFormShell_BASE(m_aMutex)
        ,FmXFormShell_CFGBASE(OUString("Office.Common/Misc"), CONFIG_MODE_DELAYED_UPDATE)
        ,m_eNavigate( NavigationBarMode_NONE )
        ,m_nInvalidationEvent( 0 )
        ,m_nActivationEvent( 0 )
        ,m_pShell( &_rShell )
        ,m_pTextShell( new ::svx::FmTextControlShell( _pViewFrame ) )
        ,m_aActiveControllerFeatures( this )
        ,m_aNavControllerFeatures( this )
        ,m_eDocumentType( eUnknownDocumentType )
        ,m_nLockSlotInvalidation( 0 )
        ,m_bHadPropertyBrowserInDesignMode( sal_False )
        ,m_bTrackProperties( sal_True )
        ,m_bUseWizards( sal_True )
        ,m_bDatabaseBar( sal_False )
        ,m_bInActivate( sal_False )
        ,m_bSetFocus( sal_False )
        ,m_bFilterMode( sal_False )
        ,m_bChangingDesignMode( sal_False )
        ,m_bPreparedClose( sal_False )
        ,m_bFirstActivation( sal_True )
{
    DBG_CTOR(FmXFormShell,NULL);
    m_aMarkTimer.SetTimeout(100);
    m_aMarkTimer.SetTimeoutHdl(LINK(this,FmXFormShell,OnTimeOut));

    if ( _pViewFrame )
        m_xAttachedFrame = _pViewFrame->GetFrame().GetFrameInterface();

    // to prevent deletion of this we acquire our refcounter once
    ::comphelper::increment(FmXFormShell_BASE::m_refCount);

    // correct the refcounter
    ::comphelper::decrement(FmXFormShell_BASE::m_refCount);

    // cache the current configuration settings we're interested in
    implAdjustConfigCache();
    // and register for changes on this settings
    Sequence< OUString > aNames(1);
    aNames[0] = OUString("FormControlPilotsEnabled");
    EnableNotification(aNames);
}

//------------------------------------------------------------------------
FmXFormShell::~FmXFormShell()
{
    delete m_pTextShell;
    DBG_DTOR(FmXFormShell,NULL);
}

//------------------------------------------------------------------
Reference< XModel > FmXFormShell::getContextDocument() const
{
    Reference< XModel > xModel;

    // determine the type of document we live in
    try
    {
        Reference< XController > xController;
        if ( m_xAttachedFrame.is() )
            xController = m_xAttachedFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return xModel;
}

//------------------------------------------------------------------
bool FmXFormShell::isEnhancedForm() const
{
    return getDocumentType() == eEnhancedForm;
}

//------------------------------------------------------------------
bool FmXFormShell::impl_checkDisposed() const
{
    if ( !m_pShell )
    {
        OSL_FAIL( "FmXFormShell::impl_checkDisposed: already disposed!" );
        return true;
    }
    return false;
}

//------------------------------------------------------------------
::svxform::DocumentType FmXFormShell::getDocumentType() const
{
    if ( m_eDocumentType != eUnknownDocumentType )
        return m_eDocumentType;

    // determine the type of document we live in
    Reference< XModel > xModel = getContextDocument();
    if ( xModel.is() )
        m_eDocumentType = DocumentClassification::classifyDocument( xModel );
    else
    {
        OSL_FAIL( "FmXFormShell::getDocumentType: can't determine the document type!" );
        m_eDocumentType = eTextDocument;
            // fallback, just to have a defined state
    }

    return m_eDocumentType;
}

//------------------------------------------------------------------
bool FmXFormShell::IsReadonlyDoc() const
{
    if ( impl_checkDisposed() )
        return true;

    FmFormModel* pModel = m_pShell->GetFormModel();
    if ( pModel && pModel->GetObjectShell() )
        return pModel->GetObjectShell()->IsReadOnly() || pModel->GetObjectShell()->IsReadOnlyUI();
    return true;
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
    impl_checkDisposed();

    if (m_xActiveController == e.Source)
    {
        // wird der Controller freigeben dann alles loslassen
        stopListening();
        m_xActiveForm = NULL;
        m_xActiveController = NULL;
        m_xNavigationController = NULL;

        m_aActiveControllerFeatures.dispose();
        m_aNavControllerFeatures.dispose();

        if ( m_pShell )
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
    }

    if (e.Source == m_xExternalViewController)
    {
        Reference< runtime::XFormController > xFormController( m_xExternalViewController, UNO_QUERY );
        OSL_ENSURE( xFormController.is(), "FmXFormShell::disposing: invalid external view controller!" );
        if (xFormController.is())
            xFormController->removeActivateListener((XFormControllerListener*)this);

        Reference< ::com::sun::star::lang::XComponent> xComp(m_xExternalViewController, UNO_QUERY);
        if (xComp.is())
            xComp->removeEventListener((XEventListener*)(XPropertyChangeListener*)this);

        m_xExternalViewController = NULL;
        m_xExternalDisplayedForm = NULL;
        m_xExtViewTriggerController = NULL;

        InvalidateSlot( SID_FM_VIEW_AS_GRID, sal_False );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::propertyChange(const PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    if ( impl_checkDisposed() )
        return;

    if (evt.PropertyName == FM_PROP_ROWCOUNT)
    {
        // Das gleich folgenden Update erzwingt ein Neu-Painten der entsprechenden Slots. Wenn ich mich aber hier nicht
        // in dem HauptThread der Applikation befinde (weil zum Beispiel ein Cursor gerade Datensaetze zaehlt und mir dabei
        // immer diese PropertyChanges beschert), kann sich das mit en normalen Paints im HauptThread der Applikation beissen.
        // (Solche Paints passieren zum Beispiel, wenn man einfach nur eine andere Applikation ueber das Office legt und wieder
        // zurueckschaltet).
        // Deshalb die Benutzung des SolarMutex, der sichert das ab.
        comphelper::SolarMutex& rSolarSafety = Application::GetSolarMutex();
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
            InvalidateSlot(SID_FM_RECORD_TOTAL, sal_False);
            LockSlotInvalidation(sal_False);
        }
    }

    // this may be called from a non-main-thread so invalidate the shell asynchronously
    LockSlotInvalidation(sal_True);
    InvalidateSlot(0, 0);       // special meaning : invalidate m_pShell
    LockSlotInvalidation(sal_False);
}

//------------------------------------------------------------------------------
void FmXFormShell::invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures )
{
    if ( impl_checkDisposed() )
        return;

    OSL_ENSURE( _rFeatures.size() > 0, "FmXFormShell::invalidateFeatures: invalid arguments!" );

    if ( m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame() )
    {
        // unfortunately, SFX requires sal_uInt16
        ::std::vector< sal_uInt16 > aSlotIds;
        aSlotIds.reserve( _rFeatures.size() );
        ::std::copy( _rFeatures.begin(),
            _rFeatures.end(),
            ::std::insert_iterator< ::std::vector< sal_uInt16 > >( aSlotIds, aSlotIds.begin() )
        );

        // furthermore, SFX wants a terminating 0
        aSlotIds.push_back( 0 );

        // and, last but not least, SFX wants the ids to be sorted
        ::std::sort( aSlotIds.begin(), aSlotIds.end() - 1 );

        sal_uInt16 *pSlotIds = &(aSlotIds[0]);
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( pSlotIds );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::formActivated(const EventObject& rEvent) throw( RuntimeException )
{
    if ( impl_checkDisposed() )
        return;

    Reference< runtime::XFormController > xController( rEvent.Source, UNO_QUERY_THROW );
    m_pTextShell->formActivated( xController );
    setActiveController( xController );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormShell::formDeactivated(const EventObject& rEvent) throw( RuntimeException )
{
    if ( impl_checkDisposed() )
        return;

    Reference< runtime::XFormController > xController( rEvent.Source, UNO_QUERY_THROW );
    m_pTextShell->formDeactivated( xController );
}

//------------------------------------------------------------------------------
void FmXFormShell::disposing()
{
    impl_checkDisposed();

    FmXFormShell_BASE::disposing();

    if ( m_pShell && !m_pShell->IsDesignMode() )
        setActiveController( NULL, sal_True );
        // do NOT save the content of the old form (the second parameter tells this)
        // if we're here, then we expect that PrepareClose has been called, and thus the user
        // got a chance to commit or reject any changes. So in case we're here and there
        // are still uncommitted changes, the user explicitly wanted this.

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
        if ( m_nActivationEvent )
        {
            Application::RemoveUserEvent( m_nActivationEvent );
            m_nActivationEvent = 0;
        }
    }

    {
        ::osl::ClearableMutexGuard aGuard(m_aAsyncSafety);
        aGuard.clear();

        DBG_ASSERT(!m_nInvalidationEvent, "FmXFormShell::~FmXFormShell : still have an invalidation event !");
            // should habe been deleted while beeing disposed

        m_aMarkTimer.Stop();
    }

    DisableNotification();

    RemoveElement( m_xForms );
    m_xForms.clear();

    impl_switchActiveControllerListening( false );
    m_xActiveController         = NULL;
    m_xActiveForm               = NULL;

    m_pShell                    = NULL;
    m_xNavigationController     = NULL;
    m_xCurrentForm              = NULL;
    m_xLastGridFound            = NULL;
    m_xAttachedFrame            = NULL;
    m_xExternalViewController   = NULL;
    m_xExtViewTriggerController = NULL;
    m_xExternalDisplayedForm    = NULL;
    m_xLastGridFound            = NULL;

    InterfaceBag aEmpty;
    m_aCurrentSelection.swap( aEmpty );

    m_aActiveControllerFeatures.dispose();
    m_aNavControllerFeatures.dispose();
}

//------------------------------------------------------------------------------
void FmXFormShell::UpdateSlot( sal_Int16 _nId )
{
    if ( impl_checkDisposed() )
        return;

    ::osl::MutexGuard aGuard(m_aInvalidationSafety);

    if ( m_nLockSlotInvalidation )
    {
        OSL_FAIL( "FmXFormShell::UpdateSlot: cannot update if invalidation is currently locked!" );
        InvalidateSlot( _nId, sal_False );
    }
    else
    {
        OSL_ENSURE( _nId, "FmXFormShell::UpdateSlot: can't update the complete shell!" );
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( _nId, sal_True, sal_True );
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Update( _nId );
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::InvalidateSlot( sal_Int16 nId, sal_Bool bWithId )
{
    if ( impl_checkDisposed() )
        return;

    ::osl::MutexGuard aGuard(m_aInvalidationSafety);
    if (m_nLockSlotInvalidation)
    {
        sal_uInt8 nFlags = ( bWithId ? 0x01 : 0 );
        m_arrInvalidSlots.push_back( InvalidSlotInfo(nId, nFlags) );
    }
    else
        if (nId)
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(nId, sal_True, bWithId);
        else
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}

//------------------------------------------------------------------------------
void FmXFormShell::LockSlotInvalidation(sal_Bool bLock)
{
    if ( impl_checkDisposed() )
        return;

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
IMPL_LINK_NOARG(FmXFormShell, OnInvalidateSlots)
{
    if ( impl_checkDisposed() )
        return 0L;

    ::osl::MutexGuard aGuard(m_aInvalidationSafety);
    m_nInvalidationEvent = 0;

    for (std::vector<InvalidSlotInfo>::const_iterator i = m_arrInvalidSlots.begin(); i < m_arrInvalidSlots.end(); ++i)
    {
        if (i->id)
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(i->id, sal_True, (i->flags & 0x01));
        else
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
    }
    m_arrInvalidSlots.clear();
    return 0L;
}

//------------------------------------------------------------------------------
void FmXFormShell::ForceUpdateSelection(sal_Bool bAllowInvalidation)
{
    if ( impl_checkDisposed() )
        return;

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

    PopupMenu* pNewMenu = new PopupMenu(SVX_RES( RID_FMSHELL_CONVERSIONMENU ));

    ImageList aImageList( SVX_RES( RID_SVXIMGLIST_FMEXPL) );
    for ( size_t i = 0; i < sizeof (nConvertSlots) / sizeof (nConvertSlots[0]); ++i )
    {
        // das entsprechende Image dran
        pNewMenu->SetItemImage(nConvertSlots[i], aImageList.GetImage(nCreateSlots[i]));
    }

    return pNewMenu;
}

//------------------------------------------------------------------------------
bool FmXFormShell::isControlConversionSlot( sal_uInt16 nSlotId )
{
    for ( size_t i = 0; i < sizeof (nConvertSlots) / sizeof (nConvertSlots[0]); ++i )
        if (nConvertSlots[i] == nSlotId)
            return true;
    return false;
}

//------------------------------------------------------------------------------
bool FmXFormShell::executeControlConversionSlot( sal_uInt16 _nSlotId )
{
    OSL_PRECOND( canConvertCurrentSelectionToControl( _nSlotId ), "FmXFormShell::executeControlConversionSlot: illegal call!" );
    InterfaceBag::const_iterator aSelectedElement = m_aCurrentSelection.begin();
    if ( aSelectedElement == m_aCurrentSelection.end() )
        return false;

    return executeControlConversionSlot( Reference< XFormComponent >( *aSelectedElement, UNO_QUERY ), _nSlotId );
}

//------------------------------------------------------------------------------
bool FmXFormShell::executeControlConversionSlot( const Reference< XFormComponent >& _rxObject, sal_uInt16 _nSlotId )
{
    if ( impl_checkDisposed() )
        return false;

    OSL_ENSURE( _rxObject.is(), "FmXFormShell::executeControlConversionSlot: invalid object!" );
    if ( !_rxObject.is() )
        return false;

    SdrPage* pPage = m_pShell->GetCurPage();
    FmFormPage* pFormPage = pPage ? dynamic_cast< FmFormPage* >( pPage ) : NULL;
    OSL_ENSURE( pFormPage, "FmXFormShell::executeControlConversionSlot: no current (form) page!" );
    if ( !pFormPage )
        return false;

    OSL_ENSURE( isSolelySelected( _rxObject ),
        "FmXFormShell::executeControlConversionSlot: hmm ... shouldn't this parameter be redundant?" );

    for ( size_t lookupSlot = 0; lookupSlot < sizeof( nConvertSlots ) / sizeof( nConvertSlots[0] ); ++lookupSlot )
    {
        if (nConvertSlots[lookupSlot] == _nSlotId)
        {
            Reference< XInterface > xNormalizedObject( _rxObject, UNO_QUERY );

            FmFormObj* pFormObject = NULL;
            SdrObjListIter aPageIter( *pFormPage );
            while ( aPageIter.IsMore() )
            {
                SdrObject* pCurrent = aPageIter.Next();
                pFormObject = FmFormObj::GetFormObject( pCurrent );
                if ( !pFormObject )
                    continue;

                Reference< XInterface > xCurrentNormalized( pFormObject->GetUnoControlModel(), UNO_QUERY );
                if ( xCurrentNormalized.get() == xNormalizedObject.get() )
                    break;

                pFormObject = NULL;
            }

            if ( !pFormObject )
                return false;

            OUString sNewName( getServiceNameByControlType( nObjectTypes[ lookupSlot ] ) );
            Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
            Reference< XControlModel> xNewModel( xContext->getServiceManager()->createInstanceWithContext(sNewName, xContext), UNO_QUERY );
            if (!xNewModel.is())
                return false;

            Reference< XControlModel> xOldModel( pFormObject->GetUnoControlModel() );
            Reference< XServiceInfo> xModelInfo(xOldModel, UNO_QUERY);

            // Properties uebertragen
            Reference< XPropertySet> xOldSet(xOldModel, UNO_QUERY);
            Reference< XPropertySet> xNewSet(xNewModel, UNO_QUERY);


            Locale aNewLanguage = Application::GetSettings().GetUILanguageTag().getLocale();
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
                Reference< XIndexContainer> xIndexParent(xChild->getParent(), UNO_QUERY);
                if (xIndexParent.is())
                {
                    // the form container works with FormComponents
                    Reference< XFormComponent> xComponent(xNewModel, UNO_QUERY);
                    DBG_ASSERT(xComponent.is(), "FmXFormShell::executeControlConversionSlot: the new model is no form component !");
                    Any aNewModel(makeAny(xComponent));
                    try
                    {

                        sal_Int32 nIndex = getElementPos(xParent, xOldModel);
                        if (nIndex>=0 && nIndex<xParent->getCount())
                            xIndexParent->replaceByIndex(nIndex, aNewModel);
                        else
                        {
                            OSL_FAIL("FmXFormShell::executeControlConversionSlot: could not replace the model !");
                            Reference< ::com::sun::star::lang::XComponent> xNewComponent(xNewModel, UNO_QUERY);
                            if (xNewComponent.is())
                                xNewComponent->dispose();
                            return false;
                        }
                    }
                    catch(Exception&)
                    {
                        OSL_FAIL("FmXFormShell::executeControlConversionSlot: could not replace the model !");
                        Reference< ::com::sun::star::lang::XComponent> xNewComponent(xNewModel, UNO_QUERY);
                        if (xNewComponent.is())
                            xNewComponent->dispose();
                        return false;
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
            pFormObject->SetChanged();
            pFormObject->SetUnoControlModel(xNewModel);

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
                if ( xOldBindable.is() )
                {
                    try
                    {
                        if ( xNewBindable.is() )
                            xNewBindable->setValueBinding( xOldBindable->getValueBinding() );
                        xOldBindable->setValueBinding( NULL );
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
            // same for list entry sources
            {
                Reference< XListEntrySink > xOldSink( xOldModel, UNO_QUERY );
                Reference< XListEntrySink > xNewSink( xNewModel, UNO_QUERY );
                if ( xOldSink.is() )
                {
                    try
                    {
                        if ( xNewSink.is() )
                            xNewSink->setListEntrySource( xOldSink->getListEntrySource() );
                        xOldSink->setListEntrySource( NULL );
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }

            // create an undo action
            FmFormModel* pModel = m_pShell->GetFormModel();
            DBG_ASSERT(pModel != NULL, "FmXFormShell::executeControlConversionSlot: my shell has no model !");
            if (pModel && pModel->IsUndoEnabled() )
            {
                pModel->AddUndo(new FmUndoModelReplaceAction(*pModel, pFormObject, xOldModel));
            }
            else
            {
                FmUndoModelReplaceAction::DisposeElement( xOldModel );
            }

            return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
bool FmXFormShell::canConvertCurrentSelectionToControl( sal_Int16 nConversionSlot )
{
    if ( m_aCurrentSelection.empty() )
        return false;

    InterfaceBag::const_iterator aCheck = m_aCurrentSelection.begin();
    Reference< XServiceInfo > xElementInfo( *aCheck, UNO_QUERY );
    if ( !xElementInfo.is() )
        // no service info -> cannot determine this
        return false;

    if (  ++aCheck != m_aCurrentSelection.end() )
        // more than one element
        return false;

    if ( Reference< XForm >::query( xElementInfo ).is() )
        // it's a form
        return false;

    sal_Int16 nObjectType = getControlTypeByObject( xElementInfo );

    if (  ( OBJ_FM_HIDDEN == nObjectType )
       || ( OBJ_FM_CONTROL == nObjectType )
       || ( OBJ_FM_GRID == nObjectType )
       )
        return false;   // those types cannot be converted

    DBG_ASSERT(sizeof(nConvertSlots)/sizeof(nConvertSlots[0]) == sizeof(nObjectTypes)/sizeof(nObjectTypes[0]),
        "FmXFormShell::canConvertCurrentSelectionToControl: nConvertSlots & nObjectTypes must have the same size !");

    for ( size_t i = 0; i < sizeof( nConvertSlots ) / sizeof( nConvertSlots[0] ); ++i )
        if (nConvertSlots[i] == nConversionSlot)
            return nObjectTypes[i] != nObjectType;

    return sal_True;    // all other slots: assume "yes"
}

//------------------------------------------------------------------------------
void FmXFormShell::checkControlConversionSlotsForCurrentSelection( Menu& rMenu )
{
    for (sal_Int16 i=0; i<rMenu.GetItemCount(); ++i)
        // der Context ist schon von einem Typ, der dem Eitnrag entspricht -> disable
        rMenu.EnableItem( rMenu.GetItemId(i), canConvertCurrentSelectionToControl( rMenu.GetItemId( i ) ) );
}

//------------------------------------------------------------------------------
void FmXFormShell::LoopGrids(sal_Int16 nWhat)
{
    if ( impl_checkDisposed() )
        return;

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
    if ( impl_checkDisposed() )
        return NULL;

    SdrPageView* pPageView = NULL;
    if ( m_pShell && m_pShell->GetFormView() )
        pPageView = m_pShell->GetFormView()->GetSdrPageView();

    Reference< XControlContainer> xControlContainer;
    if ( pPageView )
        xControlContainer = pPageView->GetPageWindow(0)->GetControlContainer();

    return xControlContainer;
}

//------------------------------------------------------------------------------
void FmXFormShell::ExecuteTabOrderDialog( const Reference< XTabControllerModel >& _rxForForm )
{
    if ( impl_checkDisposed() )
        return;

    OSL_PRECOND( _rxForForm.is(), "FmXFormShell::ExecuteTabOrderDialog: invalid tabbing model!" );
    if ( !_rxForForm.is() )
        return;

    try
    {
        Reference< XWindow > xParentWindow;
        if ( m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame() )
            xParentWindow = VCLUnoHelper::GetInterface ( &m_pShell->GetViewShell()->GetViewFrame()->GetWindow() );

        Reference< dialogs::XExecutableDialog > xDialog = form::TabOrderDialog::createWithModel(
                comphelper::getProcessComponentContext(),
                _rxForForm, getControlContainerForView(), xParentWindow
            );

        xDialog->execute();
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FmXFormShell::ExecuteTabOrderDialog: caught an exception!" );
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::ExecuteSearch()
{
    if ( impl_checkDisposed() )
        return;

    // eine Sammlung aller (logischen) Formulare
    FmFormArray aEmpty;
    m_aSearchForms.swap( aEmpty );
    ::std::vector< OUString > aContextNames;
    impl_collectFormSearchContexts_nothrow( m_pShell->GetCurPage()->GetForms(), OUString(), m_aSearchForms, aContextNames );
    OSL_POSTCOND( m_aSearchForms.size() == aContextNames.size(),
        "FmXFormShell::ExecuteSearch: nonsense!" );
    if ( m_aSearchForms.size() != aContextNames.size() )
        return;

    // filter out the forms which do not contain valid controls at all
    {
        FmFormArray aValidForms;
        ::std::vector< OUString > aValidContexts;
        FmFormArray::const_iterator form = m_aSearchForms.begin();
        ::std::vector< OUString >::const_iterator contextName = aContextNames.begin();
        for ( ; form != m_aSearchForms.end(); ++form, ++contextName )
        {
            FmSearchContext aTestContext;
            aTestContext.nContext = static_cast< sal_Int16 >( form - m_aSearchForms.begin() );
            sal_uInt32 nValidControls = OnSearchContextRequest( &aTestContext );
            if ( nValidControls > 0 )
            {
                aValidForms.push_back( *form );
                aValidContexts.push_back( *contextName );
            }
        }

        m_aSearchForms.swap( aValidForms );
        aContextNames.swap( aValidContexts );
    }

    if (m_aSearchForms.empty() )
    {   // es gibt keine Controls, die alle Bedingungen fuer eine Suche erfuellen
        ErrorBox(NULL, WB_OK, SVX_RESSTR(RID_STR_NODATACONTROLS)).Execute();
        return;
    }

    // jetzt brauche ich noch einen 'initial context'
    sal_Int16 nInitialContext = 0;
    Reference< XForm> xActiveForm( getActiveForm());
    for ( size_t i=0; i<m_aSearchForms.size(); ++i )
    {
        if (m_aSearchForms.at(i) == xActiveForm)
        {
            nInitialContext = (sal_Int16)i;
            break;
        }
    }

    // wenn der Dialog initial den Text des aktiven Controls anbieten soll, muss dieses ein XTextComponent-Interface habe,
    // ausserdem macht das nur Sinn, wenn das aktuelle Feld auch an ein Tabellen- (oder was-auch-immer-)Feld gebunden ist
    OUString strActiveField;
    OUString strInitialText;
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
                OUString sInitialText;
                if (IsSearchableControl(xCurControl, &sInitialText))
                    strInitialText = sInitialText.getStr();
            }
        }
    }

    // um eventuelle GridControls, die ich kenne, kuemmern
    LoopGrids(GA_DISABLE_SYNC /*| GA_ENABLE_ROCTRLR*/);

    // jetzt bin ich reif fuer den Dialog
    // wenn die potentiellen Deadlocks, die durch die Benutzung des Solar-Mutex in MTs VCLX...-Klasen entstehen, irgendwann mal
    // ausgeraeumt sind, sollte hier ein SM_USETHREAD rein, denn die Suche in einem eigenen Thread ist doch etwas fluessiger
    // sollte allerdings irgendwie von dem unterliegenden Cursor abhaengig gemacht werden, DAO zum Beispiel ist nicht thread-sicher
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    AbstractFmSearchDialog* pDialog = NULL;
    if ( pFact )
        pDialog = pFact->CreateFmSearchDialog( &m_pShell->GetViewShell()->GetViewFrame()->GetWindow(), strInitialText, aContextNames, nInitialContext, LINK( this, FmXFormShell, OnSearchContextRequest ) );
    DBG_ASSERT( pDialog, "FmXFormShell::ExecuteSearch: could not create the search dialog!" );
    if ( pDialog )
    {
        pDialog->SetActiveField( strActiveField );
        pDialog->SetFoundHandler( LINK( this, FmXFormShell, OnFoundData ) );
        pDialog->SetCanceledNotFoundHdl( LINK( this, FmXFormShell, OnCanceledNotFound ) );
        pDialog->Execute();
        delete pDialog;
    }

    // GridControls wieder restaurieren
    LoopGrids(GA_ENABLE_SYNC | GA_DISABLE_ROCTRLR);

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetSdrPageView());
        // da ich in OnFoundData (fals ich dort war) Controls markiert habe
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::GetY2KState(sal_uInt16& n)
{
    if ( impl_checkDisposed() )
        return sal_False;

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
                Any aVal( xSet->getPropertyValue("TwoDigitDateStart") );
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
    if ( impl_checkDisposed() )
        return;

    Reference< XForm > xActiveForm( getActiveForm());
    Reference< XRowSet > xActiveRowSet( xActiveForm, UNO_QUERY );
    if ( xActiveRowSet.is() )
    {
        Reference< XNumberFormatsSupplier > xSupplier( getNumberFormats( getRowSetConnection( xActiveRowSet ), sal_False ) );
        if (xSupplier.is())
        {
            Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    Any aVal;
                    aVal <<= n;
                    xSet->setPropertyValue("TwoDigitDateStart", aVal);
                }
                catch(Exception&)
                {
                    OSL_FAIL("FmXFormShell::SetY2KState: Exception occurred!");
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
            xCurrentForms = Reference< XIndexAccess>( m_pShell->GetCurPage()->GetForms( false ), UNO_QUERY );
    }
    if (!xCurrentForms.is())
        return;

    ::comphelper::IndexAccessIterator aIter(xCurrentForms);
    Reference< XInterface> xCurrentElement( aIter.Next());
    while (xCurrentElement.is())
    {
        // ist das aktuelle Element eine DatabaseForm ?
        Reference< XRowSet> xElementAsRowSet( xCurrentElement, UNO_QUERY );
        if ( xElementAsRowSet.is() )
        {
            Reference< XNumberFormatsSupplier > xSupplier( getNumberFormats( getRowSetConnection( xElementAsRowSet ), sal_False ) );
            if (!xSupplier.is())
                continue;

            Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    Any aVal;
                    aVal <<= n;
                    xSet->setPropertyValue("TwoDigitDateStart", aVal);
                }
                catch(Exception&)
                {
                    OSL_FAIL("FmXFormShell::SetY2KState: Exception occurred!");
                }

            }
        }
        xCurrentElement = aIter.Next();
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::CloseExternalFormViewer()
{
    if ( impl_checkDisposed() )
        return;

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
    if ( impl_checkDisposed() )
        return NULL;

    Reference< runtime::XFormController> xExternalCtrlr(m_xExternalViewController, UNO_QUERY);
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
    if ( impl_checkDisposed() )
        return NULL;

    Reference< runtime::XFormController > xExternalCtrlr(m_xExternalViewController, UNO_QUERY);
    if (xExternalCtrlr.is() && (_xForm == xExternalCtrlr->getModel()))
    {
        DBG_ASSERT(m_xExternalDisplayedForm.is(), "FmXFormShell::getInternalForm : invalid external form !");
        return Reference< XForm>(m_xExternalDisplayedForm, UNO_QUERY);
    }
    return _xForm;
}

//------------------------------------------------------------------------
namespace
{
    static bool lcl_isNavigationRelevant( sal_Int32 _nWhich )
    {
        return  ( _nWhich == SID_FM_RECORD_FIRST )
            ||  ( _nWhich == SID_FM_RECORD_PREV )
            ||  ( _nWhich == SID_FM_RECORD_NEXT )
            ||  ( _nWhich == SID_FM_RECORD_LAST )
            ||  ( _nWhich == SID_FM_RECORD_NEW );
    }
}

//------------------------------------------------------------------------------
bool FmXFormShell::IsFormSlotEnabled( sal_Int32 _nSlot, FeatureState* _pCompleteState )
{
    const ::svx::ControllerFeatures& rController =
            lcl_isNavigationRelevant( _nSlot )
        ?   getNavControllerFeatures()
        :   getActiveControllerFeatures();

    if ( !_pCompleteState )
        return rController->isEnabled( _nSlot );

    rController->getState( _nSlot, *_pCompleteState );
    return _pCompleteState->Enabled;
}

//------------------------------------------------------------------------------
void FmXFormShell::ExecuteFormSlot( sal_Int32 _nSlot )
{
    const ::svx::ControllerFeatures& rController =
            lcl_isNavigationRelevant( _nSlot )
        ?   getNavControllerFeatures()
        :   getActiveControllerFeatures();

    rController->execute( _nSlot );

    if ( _nSlot == SID_FM_RECORD_UNDO )
    {
        // if we're doing an UNDO, *and* if the affected form is the form which we also display
        // as external view, then we need to reset the controls of the external form, too
        if ( getInternalForm( getActiveForm() ) == m_xExternalDisplayedForm )
        {
            Reference< XIndexAccess > xContainer( m_xExternalDisplayedForm, UNO_QUERY );
            if ( xContainer.is() )
            {
                Reference< XReset > xReset;
                for ( sal_Int32 i = 0; i < xContainer->getCount(); ++i )
                {
                    if ( ( xContainer->getByIndex( i ) >>= xReset ) && xReset.is() )
                    {
                        // no resets on sub forms
                        Reference< XForm > xAsForm( xReset, UNO_QUERY );
                        if ( !xAsForm.is() )
                            xReset->reset();
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::impl_switchActiveControllerListening( const bool _bListen )
{
    Reference< XComponent> xComp( m_xActiveController, UNO_QUERY );
    if ( !xComp.is() )
        return;

    if ( _bListen )
        xComp->addEventListener( (XFormControllerListener*)this );
    else
        xComp->removeEventListener( (XFormControllerListener*)this );
}

//------------------------------------------------------------------------------
void FmXFormShell::setActiveController( const Reference< runtime::XFormController >& xController, sal_Bool _bNoSaveOldContent )
{
    if ( impl_checkDisposed() )
        return;

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

        bool bDifferentForm = ( xOldForm.get() != xNewForm.get() );
        bool bNeedSave = bDifferentForm && !_bNoSaveOldContent;
            // we save the content of the old form if we move to a new form, and saving old content is allowed

        if ( m_xActiveController.is() && bNeedSave )
        {
            // beim Wechsel des Controllers den Inhalt speichern, ein Commit
            // wurde bereits ausgefuehrt
            if ( m_aActiveControllerFeatures->commitCurrentControl() )
            {
                m_bSetFocus = sal_True;
                if ( m_aActiveControllerFeatures->isModifiedRow() )
                {
                    sal_Bool bIsNew = m_aActiveControllerFeatures->isInsertionRow();
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

        impl_switchActiveControllerListening( false );

        m_aActiveControllerFeatures.dispose();
        m_xActiveController = xController;
        if ( m_xActiveController.is() )
            m_aActiveControllerFeatures.assign( m_xActiveController );

        impl_switchActiveControllerListening( true );

        if ( m_xActiveController.is() )
            m_xActiveForm = getInternalForm( Reference< XForm >( m_xActiveController->getModel(), UNO_QUERY ) );
        else
            m_xActiveForm = NULL;

        startListening();

        // activate all dispatchers belonging to form of the new navigation controller
        xNavigationForm = NULL;
        if (m_xNavigationController.is())
            xNavigationForm = Reference< XResultSet>(m_xNavigationController->getModel(), UNO_QUERY);

        m_bInActivate = sal_False;

        m_pShell->UIFeatureChanged();
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);

        InvalidateSlot(SID_FM_FILTER_NAVIGATOR_CONTROL, sal_True);
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::getCurrentSelection( InterfaceBag& /* [out] */ _rSelection ) const
{
    _rSelection = m_aCurrentSelection;
}

//------------------------------------------------------------------------------
bool FmXFormShell::setCurrentSelectionFromMark( const SdrMarkList& _rMarkList )
{
    m_aLastKnownMarkedControls.clear();

    if ( ( _rMarkList.GetMarkCount() > 0 ) && isControlList( _rMarkList ) )
        collectInterfacesFromMarkList( _rMarkList, m_aLastKnownMarkedControls );

    return setCurrentSelection( m_aLastKnownMarkedControls );
}

//------------------------------------------------------------------------------
bool FmXFormShell::selectLastMarkedControls()
{
    return setCurrentSelection( m_aLastKnownMarkedControls );
}

//------------------------------------------------------------------------------
bool FmXFormShell::setCurrentSelection( const InterfaceBag& _rSelection )
{
    if ( impl_checkDisposed() )
        return false;

    DBG_ASSERT( m_pShell->IsDesignMode(), "FmXFormShell::setCurrentSelection: only to be used in design mode!" );

    if ( _rSelection.empty() && m_aCurrentSelection.empty() )
        // nothing to do
        return false;

    if ( _rSelection.size() == m_aCurrentSelection.size() )
    {
        InterfaceBag::const_iterator aNew = _rSelection.begin();
        InterfaceBag::const_iterator aOld = m_aCurrentSelection.begin();
        for ( ; aNew != _rSelection.end(); ++aNew, ++aOld )
        {
            OSL_ENSURE( Reference< XInterface >( *aNew, UNO_QUERY ).get() == aNew->get(), "FmXFormShell::setCurrentSelection: new interface not normalized!" );
            OSL_ENSURE( Reference< XInterface >( *aOld, UNO_QUERY ).get() == aOld->get(), "FmXFormShell::setCurrentSelection: old interface not normalized!" );

            if ( aNew->get() != aOld->get() )
                break;
        }

        if ( aNew == _rSelection.end() )
            // both bags equal
            return false;
    }

    // the following is some strange code to ensure that when you have two grid controls in a document,
    // only one of them can have a selected column.
    // TODO: this should happen elsewhere, but not here - shouldn't it?
    if ( !m_aCurrentSelection.empty() )
    {
        Reference< XChild > xCur; if ( m_aCurrentSelection.size() == 1 ) xCur = xCur.query( *m_aCurrentSelection.begin() );
        Reference< XChild > xNew; if ( _rSelection.size() == 1 ) xNew = xNew.query( *_rSelection.begin() );

        // is there nothing to be selected, or the parents differ, and the parent of the current object
        // is a selection supplier, then deselect
        if ( xCur.is() && ( !xNew.is() || ( xCur->getParent() != xNew->getParent() ) ) )
        {
            Reference< XSelectionSupplier > xSel( xCur->getParent(), UNO_QUERY );
            if ( xSel.is() )
                xSel->select( Any() );
        }
    }

    m_aCurrentSelection = _rSelection;

    // determine the form which all the selected obj�cts belong to, if any
    Reference< XForm > xNewCurrentForm;
    for ( InterfaceBag::const_iterator loop = m_aCurrentSelection.begin();
          loop != m_aCurrentSelection.end();
          ++loop
        )
    {
        Reference< XForm > xThisRoundsForm( GetForm( *loop ) );
        OSL_ENSURE( xThisRoundsForm.is(), "FmXFormShell::setCurrentSelection: *everything* should belong to a form!" );

        if ( !xNewCurrentForm.is() )
        {   // the first form we encounterd
            xNewCurrentForm = xThisRoundsForm;
        }
        else if ( xNewCurrentForm != xThisRoundsForm )
        {   // different forms -> no "current form" at all
            xNewCurrentForm.clear();
            break;
        }
    }

    if ( !m_aCurrentSelection.empty() )
        impl_updateCurrentForm( xNewCurrentForm );

    // ensure some slots are updated
    for ( size_t i = 0; i < sizeof( SelObjectSlotMap ) / sizeof( SelObjectSlotMap[0] ); ++i )
        InvalidateSlot( SelObjectSlotMap[i], sal_False);

    return true;
}

//------------------------------------------------------------------------------
bool FmXFormShell::isSolelySelected( const Reference< XInterface >& _rxObject )
{
    return ( m_aCurrentSelection.size() == 1 ) && ( *m_aCurrentSelection.begin() == _rxObject );
}

//------------------------------------------------------------------------------
void FmXFormShell::forgetCurrentForm()
{
    if ( !m_xCurrentForm.is() )
        return;

    // reset ...
    impl_updateCurrentForm( NULL );

    // ... and try finding a new current form
    // #i88186# / 2008-04-12 / frank.schoenheit@sun.com
    impl_defaultCurrentForm_nothrow();
}

//------------------------------------------------------------------------------
void FmXFormShell::impl_updateCurrentForm( const Reference< XForm >& _rxNewCurForm )
{
    if ( impl_checkDisposed() )
        return;

    m_xCurrentForm = _rxNewCurForm;

    // propagate to the FormPage(Impl)
    FmFormPage* pPage = m_pShell->GetCurPage();
    if ( pPage )
        pPage->GetImpl().setCurForm( m_xCurrentForm );

    // ensure the UI which depends on the current form is up-to-date
    for ( size_t i = 0; i < sizeof( DlgSlotMap ) / sizeof( DlgSlotMap[0] ); ++i )
        InvalidateSlot( DlgSlotMap[i], sal_False );
}

//------------------------------------------------------------------------------
void FmXFormShell::startListening()
{
    if ( impl_checkDisposed() )
        return;

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
            OUString aSource = ::comphelper::getString(xActiveFormSet->getPropertyValue(FM_PROP_COMMAND));
            if (!aSource.isEmpty())
            {
                m_bDatabaseBar = sal_True;

                xActiveFormSet->getPropertyValue(FM_PROP_NAVIGATION) >>= m_eNavigate;

                switch (m_eNavigate)
                {
                    case NavigationBarMode_PARENT:
                    {
                        // suchen des Controllers, ueber den eine Navigation moeglich ist
                        Reference< XChild> xChild(m_xActiveController, UNO_QUERY);
                        Reference< runtime::XFormController > xParent;
                        while (xChild.is())
                        {
                            xChild = Reference< XChild>(xChild->getParent(), UNO_QUERY);
                            xParent  = Reference< runtime::XFormController >(xChild, UNO_QUERY);
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
    if ( impl_checkDisposed() )
        return;

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
void FmXFormShell::ShowSelectionProperties( sal_Bool bShow )
{
    if ( impl_checkDisposed() )
        return;

    // if the window is already visible, only update the state
    sal_Bool bHasChild = m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_SHOW_PROPERTIES );
    if ( bHasChild && bShow )
        UpdateSlot( SID_FM_PROPERTY_CONTROL );

    // else toggle state
    else
        m_pShell->GetViewShell()->GetViewFrame()->ToggleChildWindow(SID_FM_SHOW_PROPERTIES);

    InvalidateSlot( SID_FM_PROPERTIES, sal_False );
    InvalidateSlot( SID_FM_CTL_PROPERTIES, sal_False );
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnFoundData, FmFoundRecordInformation*, pfriWhere)
{
    if ( impl_checkDisposed() )
        return 0;

    DBG_ASSERT((pfriWhere->nContext >= 0) && (pfriWhere->nContext < (sal_Int16)m_aSearchForms.size()),
        "FmXFormShell::OnFoundData : ungueltiger Kontext !");
    Reference< XForm> xForm( m_aSearchForms.at(pfriWhere->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnFoundData : ungueltige Form !");

    Reference< XRowLocate> xCursor(xForm, UNO_QUERY);
    if (!xCursor.is())
        return 0;       // was soll ich da machen ?

    // zum Datensatz
    try
    {
        xCursor->moveToBookmark(pfriWhere->aPosition);
    }
    catch(const SQLException&)
    {
        OSL_FAIL("Can position on bookmark!");
    }

    LoopGrids(GA_FORCE_SYNC);

    // und zum Feld (dazu habe ich vor dem Start des Suchens die XVclComponent-Interfaces eingesammelt)
    SAL_WARN_IF(static_cast<size_t>(pfriWhere->nFieldPos) >=
            m_arrSearchedControls.size(),
        "svx.form", "FmXFormShell::OnFoundData : invalid index!");
    SdrObject* pObject = m_arrSearchedControls.at(pfriWhere->nFieldPos);

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetSdrPageView());
    m_pShell->GetFormView()->MarkObj(pObject, m_pShell->GetFormView()->GetSdrPageView());

    FmFormObj* pFormObject = FmFormObj::GetFormObject( pObject );
    Reference< XControlModel > xControlModel( pFormObject ? pFormObject->GetUnoControlModel() : Reference< XControlModel >() );
    DBG_ASSERT( xControlModel.is(), "FmXFormShell::OnFoundData: invalid control!" );
    if ( !xControlModel.is() )
        return 0;

    // disable the permanent cursor for the last grid we found a record
    if (m_xLastGridFound.is() && (m_xLastGridFound != xControlModel))
    {
        Reference< XPropertySet> xOldSet(m_xLastGridFound, UNO_QUERY);
        xOldSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, makeAny( (sal_Bool)sal_False ) );
        Reference< XPropertyState> xOldSetState(xOldSet, UNO_QUERY);
        if (xOldSetState.is())
            xOldSetState->setPropertyToDefault(FM_PROP_CURSORCOLOR);
        else
            xOldSet->setPropertyValue(FM_PROP_CURSORCOLOR, Any());
    }

    // wenn das Feld sich in einem GridControl befindet, muss ich dort noch in die entsprechende Spalte gehen
    sal_Int32 nGridColumn = m_arrRelativeGridColumn[pfriWhere->nFieldPos];
    if (nGridColumn != -1)
    {   // dummer weise muss ich mir das Control erst wieder besorgen
        Reference< XControl> xControl( impl_getControl( xControlModel, *pFormObject ) );
        Reference< XGrid> xGrid(xControl, UNO_QUERY);
        DBG_ASSERT(xGrid.is(), "FmXFormShell::OnFoundData : ungueltiges Control !");
        // wenn eine der Asserts anschlaegt, habe ich beim Aufbauen von m_arrSearchedControls wohl was falsch gemacht

        // enable a permanent cursor for the grid so we can see the found text
        Reference< XPropertySet> xModelSet(xControlModel, UNO_QUERY);
        DBG_ASSERT(xModelSet.is(), "FmXFormShell::OnFoundData : invalid control model (no property set) !");
        xModelSet->setPropertyValue( FM_PROP_ALWAYSSHOWCURSOR, makeAny( (sal_Bool)sal_True ) );
        xModelSet->setPropertyValue( FM_PROP_CURSORCOLOR, makeAny( sal_Int32( COL_LIGHTRED ) ) );
        m_xLastGridFound = xControlModel;

        if ( xGrid.is() )
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
    if ( impl_checkDisposed() )
        return 0;

    DBG_ASSERT((pfriWhere->nContext >= 0) && (pfriWhere->nContext < (sal_Int16)m_aSearchForms.size()),
        "FmXFormShell::OnCanceledNotFound : ungueltiger Kontext !");
    Reference< XForm> xForm( m_aSearchForms.at(pfriWhere->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnCanceledNotFound : ungueltige Form !");

    Reference< XRowLocate> xCursor(xForm, UNO_QUERY);
    if (!xCursor.is())
        return 0;       // was soll ich da machen ?

    // zum Datensatz
    try
    {
        xCursor->moveToBookmark(pfriWhere->aPosition);
    }
    catch(const SQLException&)
    {
        OSL_FAIL("Can position on bookmark!");
    }


    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetSdrPageView());
    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnSearchContextRequest, FmSearchContext*, pfmscContextInfo)
{
    if ( impl_checkDisposed() )
        return 0;

    DBG_ASSERT(pfmscContextInfo->nContext < (sal_Int16)m_aSearchForms.size(), "FmXFormShell::OnSearchContextRequest : invalid parameter !");
    Reference< XForm> xForm( m_aSearchForms.at(pfmscContextInfo->nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnSearchContextRequest : unexpected : invalid context !");

    Reference< XResultSet> xIter(xForm, UNO_QUERY);
    DBG_ASSERT(xIter.is(), "FmXFormShell::OnSearchContextRequest : unexpected : context has no iterator !");

    // --------------------------------------------------------------------------------------------
    // assemble the list of fields to involve (that is, the ControlSources of all fields that have such a property)
    OUString strFieldList, sFieldDisplayNames;
    m_arrSearchedControls.clear();
    m_arrRelativeGridColumn.clear();

    // small problem: To mark found fields, I need SdrObjects. To determine which controls
    // to include in the search, I need Controls (that is, XControl interfaces). So I have
    // to iterate over one of them and get the other in some way. Unfortunately, there is
    // no direct connexion between the two worlds (except from a GetUnoControl to a
    // SdrUnoObject, but this requires an OutputDevice I can not do anything with.
    // However I can get to the Model from the Control and also from the SdrObject, and in
    // this way the assignment SdrObject<->Control is possible with a double loop.
    // The alternative to this (ugly but certainly not entirely fixable) solution would be
    // to renounce the caching of the SdrObjects, which would lead to significant extra
    // work in OnFoundData (since there I'd have to get the SdrObject first thing every
    // time). But since OnFoundData is usually called more often than ExecuteSeearch, I'll
    // do that here.

    Reference< XNameAccess> xValidFormFields;
    Reference< XColumnsSupplier> xSupplyCols(xIter, UNO_QUERY);
    DBG_ASSERT(xSupplyCols.is(), "FmXFormShell::OnSearchContextRequest : invalid cursor : no columns supplier !");
    if (xSupplyCols.is())
        xValidFormFields = xSupplyCols->getColumns();
    DBG_ASSERT(xValidFormFields.is(), "FmXFormShell::OnSearchContextRequest : form has no fields !");

    // current Page/Controller
    FmFormPage* pCurrentPage = m_pShell->GetCurPage();
    DBG_ASSERT(pCurrentPage!=NULL, "FmXFormShell::OnSearchContextRequest : no page !");
    // Search all SdrControls of this page...
    OUString sControlSource, aName;

    SdrObjListIter aPageIter( *pCurrentPage );
    while ( aPageIter.IsMore() )
    {
        SdrObject* pCurrent = aPageIter.Next();
        FmFormObj* pFormObject = FmFormObj::GetFormObject( pCurrent );
        // note that in case pCurrent is a virtual object, pFormObject points to the referenced object

        if ( !pFormObject )
            continue;

        // the current object's model, in different tastes
        Reference< XControlModel> xControlModel( pFormObject->GetUnoControlModel() );
        Reference< XFormComponent > xCurrentFormComponent( xControlModel, UNO_QUERY );
        DBG_ASSERT( xCurrentFormComponent.is(), "FmXFormShell::OnSearchContextRequest: invalid objects!" );
        if ( !xCurrentFormComponent.is() )
            continue;

        // does the component belong to the form which we're interested in?
        if ( xCurrentFormComponent->getParent() != xForm )
            continue;

        // ... ask for the ControlSource property
        SearchableControlIterator iter( xCurrentFormComponent );
        Reference< XControl> xControl;
        // the control that has model xControlModel
        // (the following while can be passed through several times, without the Control
        // being modified, so I don't have to search every time from scratch)

        Reference< XInterface > xSearchable( iter.Next() );
        while ( xSearchable.is() )
        {
            sControlSource = iter.getCurrentValue();
            if ( sControlSource.isEmpty() )
            {
                // the current element has no ControlSource, so it is a GridControl (that
                // is the only thing that still permits the SearchableControlIteratore)
                xControl = impl_getControl( xControlModel, *pFormObject );
                DBG_ASSERT(xControl.is(), "FmXFormShell::OnSearchContextRequest : didn't ::std::find a control with requested model !");

                Reference< XGridPeer> xGridPeer;
                if ( xControl.is() )
                    xGridPeer.set( xControl->getPeer(), UNO_QUERY );
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

                        // can we use this column control for searching ?
                        if (!IsSearchableControl(xCurrentColumn))
                            continue;

                        sal_Int16 nModelPos = GridView2ModelPos(xModelColumns, nViewPos);
                        Reference< XPropertySet> xCurrentColModel;
                        xModelColumns->getByIndex(nModelPos) >>= xCurrentColModel;
                        aName = ::comphelper::getString(xCurrentColModel->getPropertyValue(FM_PROP_CONTROLSOURCE));
                        // the cursor has a field matching the control source ?
                        if (xValidFormFields->hasByName(aName))
                        {
                            strFieldList = strFieldList + OUString(aName.getStr()) + ";";

                            sFieldDisplayNames = sFieldDisplayNames +
                                    OUString(::comphelper::getString(xCurrentColModel->getPropertyValue(FM_PROP_LABEL)).getStr()) +
                                    ";";

                            pfmscContextInfo->arrFields.push_back(xCurrentColumn);

                            // and the SdrOject to the Field
                            m_arrSearchedControls.push_back(pCurrent);
                            // the number of the column
                            m_arrRelativeGridColumn.push_back(nViewPos);
                        }
                    }
                } while (false);
            }
            else
            {
                if (!sControlSource.isEmpty() && xValidFormFields->hasByName(sControlSource))
                {
                    // now I need the Control to SdrObject
                    if (!xControl.is())
                    {
                        xControl = impl_getControl( xControlModel, *pFormObject );
                        DBG_ASSERT(xControl.is(), "FmXFormShell::OnSearchContextRequest : didn't ::std::find a control with requested model !");
                    }

                    if (IsSearchableControl(xControl))
                    {
                        // all tests passed -> take along in the list
                        strFieldList = strFieldList + OUString(sControlSource.getStr()) + ";";

                        // the label which should appear for the control :
                        sFieldDisplayNames = sFieldDisplayNames +
                                OUString(getLabelName(Reference< XPropertySet>(xControlModel, UNO_QUERY)).getStr()) +
                                ";";

                        // mark the SdrObject (accelerates the treatment in OnFoundData)
                        m_arrSearchedControls.push_back(pCurrent);

                        // the number of the colum (here a dummy, since it is only interesting for GridControls)
                        m_arrRelativeGridColumn.push_back(-1);

                        // and for the formatted search...
                        pfmscContextInfo->arrFields.push_back(xControl);
                    }
                }
            }

            xSearchable = iter.Next();
        }
    }

    strFieldList = comphelper::string::stripEnd(strFieldList, ';');
    sFieldDisplayNames = comphelper::string::stripEnd(sFieldDisplayNames, ';');

    if (pfmscContextInfo->arrFields.empty())
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
    // when the cursor is a non-STANDARD RecordMode, set it back
    Reference< XPropertySet> xCursorSet(pfmscContextInfo->xCursor, UNO_QUERY);
    Reference< XResultSetUpdate> xUpdateCursor(pfmscContextInfo->xCursor, UNO_QUERY);
    if (xUpdateCursor.is() && xCursorSet.is() && xCursorSet.is())
    {
        if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISNEW)))
            xUpdateCursor->moveToCurrentRow();
        else if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISMODIFIED)))
            xUpdateCursor->cancelRowUpdates();
    }

    return pfmscContextInfo->arrFields.size();
}

  // XContainerListener
//------------------------------------------------------------------------------
void FmXFormShell::elementInserted(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    if ( impl_checkDisposed() )
        return;

    // new object to listen to
    Reference< XInterface> xTemp;
    evt.Element >>= xTemp;
    AddElement(xTemp);
    m_pShell->DetermineForms(sal_True);
}

//------------------------------------------------------------------------------
void FmXFormShell::elementReplaced(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    if ( impl_checkDisposed() )
        return;

    Reference< XInterface> xTemp;
    evt.ReplacedElement >>= xTemp;
    RemoveElement(xTemp);
    evt.Element >>= xTemp;
    AddElement(xTemp);
}

//------------------------------------------------------------------------------
void FmXFormShell::elementRemoved(const ContainerEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    if ( impl_checkDisposed() )
        return;

    Reference< XInterface> xTemp;
    evt.Element >>= xTemp;
    RemoveElement(xTemp);
    m_pShell->DetermineForms(sal_True);
}

//------------------------------------------------------------------------------
void FmXFormShell::UpdateForms( sal_Bool _bInvalidate )
{
    if ( impl_checkDisposed() )
        return;

    Reference< XIndexAccess > xForms;

    FmFormPage* pPage = m_pShell->GetCurPage();
    if ( pPage )
    {
        if ( m_pShell->m_bDesignMode )
            xForms = xForms.query( pPage->GetForms( false ) );
    }

    if ( m_xForms != xForms )
    {
        RemoveElement( m_xForms );
        m_xForms = xForms;
        AddElement( m_xForms );
    }

    m_pShell->DetermineForms( _bInvalidate );
}

//------------------------------------------------------------------------------
void FmXFormShell::AddElement(const Reference< XInterface>& _xElement)
{
    if ( impl_checkDisposed() )
        return;
    impl_AddElement_nothrow(_xElement);
}
// -----------------------------------------------------------------------------
void FmXFormShell::impl_AddElement_nothrow(const Reference< XInterface>& Element)
{
    // am Container horchen
    const Reference< XIndexContainer> xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        const sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface> xElement;
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            xElement.set(xContainer->getByIndex(i),UNO_QUERY);
            impl_AddElement_nothrow(xElement);
        }

        const Reference< XContainer> xCont(Element, UNO_QUERY);
        if (xCont.is())
            xCont->addContainerListener(this);
    }

    const Reference< ::com::sun::star::view::XSelectionSupplier> xSelSupplier(Element, UNO_QUERY);
    if (xSelSupplier.is())
        xSelSupplier->addSelectionChangeListener(this);
}

//------------------------------------------------------------------------------
void FmXFormShell::RemoveElement(const Reference< XInterface>& Element)
{
    if ( impl_checkDisposed() )
        return;
    impl_RemoveElement_nothrow(Element);
}
//------------------------------------------------------------------------------
void FmXFormShell::impl_RemoveElement_nothrow(const Reference< XInterface>& Element)
{
    const Reference< ::com::sun::star::view::XSelectionSupplier> xSelSupplier(Element, UNO_QUERY);
    if (xSelSupplier.is())
        xSelSupplier->removeSelectionChangeListener(this);

    // remove connection to children
    const Reference< XIndexContainer> xContainer(Element, UNO_QUERY);
    if (xContainer.is())
    {
        const Reference< XContainer> xCont(Element, UNO_QUERY);
        if (xCont.is())
            xCont->removeContainerListener(this);

        const sal_uInt32 nCount = xContainer->getCount();
        Reference< XInterface> xElement;
        for (sal_uInt32 i = 0; i < nCount; i++)
        {
            xElement.set(xContainer->getByIndex(i),UNO_QUERY);
            impl_RemoveElement_nothrow(xElement);
        }
    }

    InterfaceBag::iterator wasSelectedPos = m_aCurrentSelection.find( Element );
    if ( wasSelectedPos != m_aCurrentSelection.end() )
        m_aCurrentSelection.erase( wasSelectedPos );
}

//------------------------------------------------------------------------------
void FmXFormShell::selectionChanged(const EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException)
{
    if ( impl_checkDisposed() )
        return;

    Reference< XSelectionSupplier > xSupplier( rEvent.Source, UNO_QUERY );
    Reference< XInterface > xSelObj( xSupplier->getSelection(), UNO_QUERY );
    // a selection was removed, this can only be done by the shell
    if ( !xSelObj.is() )
        return;

    EnableTrackProperties(sal_False);

    sal_Bool bMarkChanged = m_pShell->GetFormView()->checkUnMarkAll(rEvent.Source);
    Reference< XForm > xNewForm( GetForm( rEvent.Source ) );

    InterfaceBag aNewSelection;
    aNewSelection.insert( xSelObj );

    if ( setCurrentSelection( aNewSelection ) && IsPropBrwOpen() )
        ShowSelectionProperties( sal_True );

    EnableTrackProperties(sal_True);

    if ( bMarkChanged )
        m_pShell->NotifyMarkListChanged( m_pShell->GetFormView() );
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormShell, OnTimeOut, void*, /*EMPTYTAG*/)
{
    if ( impl_checkDisposed() )
        return 0;

    if (m_pShell->IsDesignMode() && m_pShell->GetFormView())
        SetSelection(m_pShell->GetFormView()->GetMarkedObjectList());

    return 0;
}

//------------------------------------------------------------------------
void FmXFormShell::SetSelectionDelayed()
{
    if ( impl_checkDisposed() )
        return;

    if (m_pShell->IsDesignMode() && IsTrackPropertiesEnabled() && !m_aMarkTimer.IsActive())
        m_aMarkTimer.Start();
}

//------------------------------------------------------------------------
void FmXFormShell::SetSelection(const SdrMarkList& rMarkList)
{
    if ( impl_checkDisposed() )
        return;

    DetermineSelection(rMarkList);
    m_pShell->NotifyMarkListChanged(m_pShell->GetFormView());
}

//------------------------------------------------------------------------
void FmXFormShell::DetermineSelection(const SdrMarkList& rMarkList)
{
    if ( setCurrentSelectionFromMark( rMarkList ) && IsPropBrwOpen() )
        ShowSelectionProperties( sal_True );
}

//------------------------------------------------------------------------------
sal_Bool FmXFormShell::IsPropBrwOpen() const
{
    if ( impl_checkDisposed() )
        return sal_False;

    return( ( m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame() ) ?
            m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow(SID_FM_SHOW_PROPERTIES) : sal_False );
}

//------------------------------------------------------------------------------
class FmXFormShell::SuspendPropertyTracking
{
private:
    FmXFormShell&   m_rShell;
    bool            m_bEnabled;

public:
    SuspendPropertyTracking( FmXFormShell& _rShell )
        :m_rShell( _rShell )
        ,m_bEnabled( false )
    {
        if ( m_rShell.IsTrackPropertiesEnabled() )
        {
            m_rShell.EnableTrackProperties( sal_False );
            m_bEnabled = true;
        }
    }

    ~SuspendPropertyTracking( )
    {
        if ( m_bEnabled )   // note that ( false != m_bEnabled ) implies ( NULL != m_pShell )
            m_rShell.EnableTrackProperties( sal_True );
    }
};

//------------------------------------------------------------------------------
void FmXFormShell::SetDesignMode(sal_Bool bDesign)
{
    if ( impl_checkDisposed() )
        return;

    DBG_ASSERT(m_pShell->GetFormView(), "FmXFormShell::SetDesignMode : invalid call (have no shell or no view) !");
    m_bChangingDesignMode = sal_True;

    // 67506 - 15.07.99 - FS
    // if we're switching off the design mode we have to force the property browser to be closed
    // so it can commit it's changes _before_ we load the forms
    if (!bDesign)
    {
        m_bHadPropertyBrowserInDesignMode = m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow(SID_FM_SHOW_PROPERTIES);
        if (m_bHadPropertyBrowserInDesignMode)
            m_pShell->GetViewShell()->GetViewFrame()->ToggleChildWindow(SID_FM_SHOW_PROPERTIES);
    }

    FmFormView* pFormView = m_pShell->GetFormView();
    if (bDesign)
    {
        // we are currently filtering, so stop filtering
        if (m_bFilterMode)
            stopFiltering(sal_False);

        // unsubscribe from the objects of my MarkList
        pFormView->GetImpl()->stopMarkListWatching();
    }
    else
    {
        m_aMarkTimer.Stop();

        SuspendPropertyTracking aSuspend( *this );
        pFormView->GetImpl()->saveMarkList( sal_True );
    }

    if (bDesign && m_xExternalViewController.is())
        CloseExternalFormViewer();

    pFormView->ChangeDesignMode(bDesign);

    // notify listensers
    FmDesignModeChangedHint aChangedHint( bDesign );
    m_pShell->Broadcast(aChangedHint);

    m_pShell->m_bDesignMode = bDesign;
    UpdateForms( sal_False );

    m_pTextShell->designModeChanged( m_pShell->m_bDesignMode );

    if (bDesign)
    {
        SdrMarkList aList;
        {
            // during changing the mark list, don't track the selected objects in the property browser
            SuspendPropertyTracking aSuspend( *this );
            // restore the marks
            pFormView->GetImpl()->restoreMarkList( aList );
        }

        // synchronize with the restored mark list
        if ( aList.GetMarkCount() )
            SetSelection( aList );
    }
    else
    {
        // subscribe to the model of the view (so that I'm informed when someone deletes
        // during the alive mode controls that I had saved in the saveMarklist (60343)
        pFormView->GetImpl()->startMarkListWatching();
    }

    m_pShell->UIFeatureChanged();

    // 67506 - 15.07.99 - FS
    if (bDesign && m_bHadPropertyBrowserInDesignMode)
    {
        // The UIFeatureChanged performes an update (a check of the available features) asynchronously.
        // So we can't call ShowSelectionProperties directly as the according feature isn't enabled yet.
        // That's why we use an asynchron execution on the dispatcher.
        // (And that's why this has to be done AFTER the UIFeatureChanged.)
        m_pShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON );
    }
    m_bChangingDesignMode = sal_False;
}

//------------------------------------------------------------------------------
Reference< XControl> FmXFormShell::impl_getControl( const Reference< XControlModel >& i_rxModel, const FmFormObj& i_rKnownFormObj )
{
    if ( impl_checkDisposed() )
        return NULL;

    Reference< XControl > xControl;
    try
    {
        Reference< XControlContainer> xControlContainer( getControlContainerForView(), UNO_SET_THROW );

        Sequence< Reference< XControl > > seqControls( xControlContainer->getControls() );
        const Reference< XControl >* pControls = seqControls.getArray();
        // ... that I can then search
        for (sal_Int32 i=0; i<seqControls.getLength(); ++i)
        {
            xControl.set( pControls[i], UNO_SET_THROW );
            Reference< XControlModel > xCurrentModel( xControl->getModel() );
            if ( xCurrentModel == i_rxModel )
                break;
            xControl.clear();
        }

        if ( !xControl.is() )
        {
            // fallabck (some controls might not have been created, yet, since they were never visible so far)
            Reference< XControl > xContainerControl( xControlContainer, UNO_QUERY_THROW );
            const Window* pContainerWindow = VCLUnoHelper::GetWindow( xContainerControl->getPeer() );
            ENSURE_OR_THROW( pContainerWindow, "unexpected control container implementation" );

            const SdrView* pSdrView = m_pShell ? m_pShell->GetFormView() : NULL;
            ENSURE_OR_THROW( pSdrView, "no current view" );

            xControl.set( i_rKnownFormObj.GetUnoControl( *pSdrView, *pContainerWindow ), UNO_QUERY_THROW );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    OSL_ENSURE( xControl.is(), "FmXFormShell::impl_getControl: no control found!" );
    return xControl;
}

//------------------------------------------------------------------------------
void FmXFormShell::impl_collectFormSearchContexts_nothrow( const Reference< XInterface>& _rxStartingPoint,
    const OUString& _rCurrentLevelPrefix, FmFormArray& _out_rForms, ::std::vector< OUString >& _out_rNames )
{
    try
    {
        Reference< XIndexAccess> xContainer( _rxStartingPoint, UNO_QUERY );
        if ( !xContainer.is() )
            return;

        sal_Int32 nCount( xContainer->getCount() );
        if ( nCount == 0 )
            return;

        OUString sCurrentFormName;
        OUStringBuffer aNextLevelPrefix;
        for ( sal_Int32 i=0; i<nCount; ++i )
        {
            // is the current child a form?
            Reference< XForm > xCurrentAsForm( xContainer->getByIndex(i), UNO_QUERY );
            if ( !xCurrentAsForm.is() )
                continue;

            Reference< XNamed > xNamed( xCurrentAsForm, UNO_QUERY_THROW );
            sCurrentFormName = xNamed->getName();

            // the name of the current form
            OUStringBuffer sCompleteCurrentName( sCurrentFormName );
            if ( !_rCurrentLevelPrefix.isEmpty() )
            {
                sCompleteCurrentName.appendAscii( " (" );
                sCompleteCurrentName.append     ( _rCurrentLevelPrefix );
                sCompleteCurrentName.appendAscii( ")" );
            }

            // the prefix for the next level
            aNextLevelPrefix = _rCurrentLevelPrefix;
            if ( !_rCurrentLevelPrefix.isEmpty() )
                aNextLevelPrefix.append( (sal_Unicode)'/' );
            aNextLevelPrefix.append( sCurrentFormName );

            // remember both the form and it's "display name"
            _out_rForms.push_back( xCurrentAsForm );
            _out_rNames.push_back( sCompleteCurrentName.makeStringAndClear() );

            // und absteigen
            impl_collectFormSearchContexts_nothrow( xCurrentAsForm, aNextLevelPrefix.makeStringAndClear(), _out_rForms, _out_rNames );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::startFiltering()
{
    if ( impl_checkDisposed() )
        return;

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

    PFormViewPageWindowAdapter pAdapter = pXView->findWindow( xContainer );
    if ( pAdapter.is() )
    {
        const ::std::vector< Reference< runtime::XFormController> >& rControllerList = pAdapter->GetList();
        for (   ::std::vector< Reference< runtime::XFormController> >::const_iterator j = rControllerList.begin();
                j != rControllerList.end();
                ++j
            )
        {
            Reference< XModeSelector> xModeSelector(*j, UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode( OUString( "FilterMode"  ) );
        }
    }

    m_bFilterMode = sal_True;

    m_pShell->UIFeatureChanged();
    SfxViewFrame* pViewFrame = m_pShell->GetViewShell()->GetViewFrame();
    pViewFrame->GetBindings().InvalidateShell( *m_pShell );

    if  (   pViewFrame->KnowsChildWindow( SID_FM_FILTER_NAVIGATOR )
        &&  !pViewFrame->HasChildWindow( SID_FM_FILTER_NAVIGATOR )
        )
    {
        pViewFrame->ToggleChildWindow( SID_FM_FILTER_NAVIGATOR );
    }
}

//------------------------------------------------------------------------------
void saveFilter(const Reference< runtime::XFormController >& _rxController)
{
    Reference< XPropertySet> xFormAsSet(_rxController->getModel(), UNO_QUERY);
    Reference< XPropertySet> xControllerAsSet(_rxController, UNO_QUERY);
    Reference< XIndexAccess> xControllerAsIndex(_rxController, UNO_QUERY);

    // call the subcontroller
    Reference< runtime::XFormController > xController;
    for (sal_Int32 i = 0, nCount = xControllerAsIndex->getCount(); i < nCount; ++i)
    {
        xControllerAsIndex->getByIndex(i) >>= xController;
        saveFilter(xController);
    }

    try
    {

        xFormAsSet->setPropertyValue(FM_PROP_FILTER, xControllerAsSet->getPropertyValue(FM_PROP_FILTER));
        xFormAsSet->setPropertyValue(FM_PROP_APPLYFILTER, makeAny( (sal_Bool)sal_True ) );
    }
    catch (const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

}

//------------------------------------------------------------------------------
void FmXFormShell::stopFiltering(sal_Bool bSave)
{
    if ( impl_checkDisposed() )
        return;

    m_bFilterMode = sal_False;

    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    Reference< XControlContainer> xContainer;
    if (getActiveController() == m_xExternalViewController)
    {
        DBG_ASSERT(m_xExtViewTriggerController.is(), "FmXFormShell::stopFiltering : inconsistent : active external controller, but noone triggered this !");
        xContainer = m_xExtViewTriggerController->getContainer();
    }
    else
        xContainer = getActiveController()->getContainer();

    PFormViewPageWindowAdapter pAdapter = pXView->findWindow(xContainer);
    if ( pAdapter.is() )
    {
        const ::std::vector< Reference< runtime::XFormController > >& rControllerList = pAdapter->GetList();
        ::std::vector < OUString >   aOriginalFilters;
        ::std::vector < sal_Bool >          aOriginalApplyFlags;

        if (bSave)
        {
            for (::std::vector< Reference< runtime::XFormController > > ::const_iterator j = rControllerList.begin();
                 j != rControllerList.end(); ++j)
            {
                if (bSave)
                {   // remember the current filter settings in case we're goin to reload the forms below (which may fail)
                    try
                    {
                        Reference< XPropertySet > xFormAsSet((*j)->getModel(), UNO_QUERY);
                        aOriginalFilters.push_back(::comphelper::getString(xFormAsSet->getPropertyValue(FM_PROP_FILTER)));
                        aOriginalApplyFlags.push_back(::comphelper::getBOOL(xFormAsSet->getPropertyValue(FM_PROP_APPLYFILTER)));
                    }
                    catch(Exception&)
                    {
                        OSL_FAIL("FmXFormShell::stopFiltering : could not get the original filter !");
                        // put dummies into the arrays so the they have the right size

                        if (aOriginalFilters.size() == aOriginalApplyFlags.size())
                            // the first getPropertyValue failed -> use two dummies
                            aOriginalFilters.push_back( OUString() );
                        aOriginalApplyFlags.push_back( sal_False );
                    }
                }
                saveFilter(*j);
            }
        }
        for (::std::vector< Reference< runtime::XFormController > > ::const_iterator j = rControllerList.begin();
             j != rControllerList.end(); ++j)
        {

            Reference< XModeSelector> xModeSelector(*j, UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode( OUString( "DataMode"  ) );
        }
        if (bSave)  // execute the filter
        {
            const ::std::vector< Reference< runtime::XFormController > > & rControllers = pAdapter->GetList();
            for (::std::vector< Reference< runtime::XFormController > > ::const_iterator j = rControllers.begin();
                 j != rControllers.end(); ++j)
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
                    OSL_FAIL("FmXFormShell::stopFiltering: Exception occurred!");
                }

                if (!isRowSetAlive(xFormSet))
                {   // something went wrong -> restore the original state
                    OUString sOriginalFilter = aOriginalFilters[ j - rControllers.begin() ];
                    sal_Bool bOriginalApplyFlag = aOriginalApplyFlags[ j - rControllers.begin() ];
                    try
                    {
                        xFormSet->setPropertyValue(FM_PROP_FILTER, makeAny(sOriginalFilter));
                        xFormSet->setPropertyValue(FM_PROP_APPLYFILTER, makeAny(bOriginalApplyFlag));
                        xReload->reload();
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }
    }

    m_pShell->UIFeatureChanged();
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}

//------------------------------------------------------------------------------
void FmXFormShell::CreateExternalView()
{
    if ( impl_checkDisposed() )
        return;

    DBG_ASSERT(m_xAttachedFrame.is(), "FmXFormShell::CreateExternalView : no frame !");

    // the frame the external view is displayed in
    sal_Bool bAlreadyExistent = m_xExternalViewController.is();
    Reference< ::com::sun::star::frame::XFrame> xExternalViewFrame;
    OUString sFrameName("_beamer");
    sal_Int32 nSearchFlags = ::com::sun::star::frame::FrameSearchFlag::CHILDREN | ::com::sun::star::frame::FrameSearchFlag::CREATE;

    Reference< runtime::XFormController > xCurrentNavController( getNavController());
        // the creation of the "partwindow" may cause a deactivate of the document which will result in our nav controller to be set to NULL

    // _first_ check if we have any valid fields we can use for the grid view
    // FS - 21.10.99 - 69219
    {
        FmXBoundFormFieldIterator aModelIterator(xCurrentNavController->getModel());
        Reference< XPropertySet> xCurrentModelSet;
        bool bHaveUsableControls = false;
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
            bHaveUsableControls = true;
            break;
        }

        if (!bHaveUsableControls)
        {
            ErrorBox(NULL, WB_OK, SVX_RESSTR(RID_STR_NOCONTROLS_FOR_EXTERNALDISPLAY)).Execute();
            return;
        }
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
                Reference< runtime::XFormController > xAsFormController( m_xExternalViewController, UNO_QUERY );
                ControllerFeatures aHelper( xAsFormController, NULL );
                aHelper->commitCurrentControl();
            }

            Reference< runtime::XFormController > xNewController(m_xExtViewTriggerController);
            CloseExternalFormViewer();
            setActiveController(xNewController);
            return;
        }

        URL aClearURL;
        aClearURL.Complete = FMURL_GRIDVIEW_CLEARVIEW;

        Reference< ::com::sun::star::frame::XDispatch> xClear( xCommLink->queryDispatch(aClearURL, OUString(), 0));
        if (xClear.is())
            xClear->dispatch(aClearURL, Sequence< PropertyValue>());
    }

    // TODO: We need an interceptor at the xSupplier, which forwards all queryDispatch requests to the FormController
    // instance for which this "external view" was triggered

    // get the dispatch interface of the frame so we can communicate (interceptable) with the controller
    Reference< ::com::sun::star::frame::XDispatchProvider> xCommLink(xExternalViewFrame, UNO_QUERY);

    if (m_xExternalViewController.is())
    {
        DBG_ASSERT(xCommLink.is(), "FmXFormShell::CreateExternalView : the component doesn't have the necessary interfaces !");
        // collect the dispatchers we will need
        URL aAddColumnURL;
        aAddColumnURL.Complete = FMURL_GRIDVIEW_ADDCOLUMN;
        Reference< ::com::sun::star::frame::XDispatch> xAddColumnDispatch( xCommLink->queryDispatch(aAddColumnURL, OUString(), 0));
        URL aAttachURL;
        aAttachURL.Complete = FMURL_GRIDVIEW_ATTACHTOFORM;
        Reference< ::com::sun::star::frame::XDispatch> xAttachDispatch( xCommLink->queryDispatch(aAttachURL, OUString(), 0));

        if (xAddColumnDispatch.is() && xAttachDispatch.is())
        {
            DBG_ASSERT(xCurrentNavController.is(), "FmXFormShell::CreateExternalView : invalid call : have no nav controller !");
            // first : dispatch the descriptions for the columns to add
            Sequence< Reference< XControl> > aCurrentControls(xCurrentNavController->getControls());

            sal_Int16 nAddedColumns = 0;

            // for radio buttons we need some special structures
            typedef std::map< OUString, Sequence< OUString> > MapUString2UstringSeq;
            typedef std::map< OUString, OUString > FmMapUString2UString;
            typedef std::map< OUString, sal_Int16 > FmMapUString2Int16;

            MapUString2UstringSeq   aRadioValueLists;
            MapUString2UstringSeq   aRadioListSources;
            FmMapUString2UString    aRadioControlSources;
            FmMapUString2Int16      aRadioPositions;

            FmXBoundFormFieldIterator aModelIterator(xCurrentNavController->getModel());
            Reference< XPropertySet> xCurrentModelSet;
            Any aCurrentBoundField;
            OUString sColumnType,aGroupName,sControlSource;
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
                        Sequence< OUString>& aThisGroupLabels = aRadioListSources[aGroupName];
                        sal_Int32 nNewSizeL = aThisGroupLabels.getLength() + 1;
                        aThisGroupLabels.realloc(nNewSizeL);
                        aThisGroupLabels.getArray()[nNewSizeL - 1] = ::comphelper::getString(xCurrentModelSet->getPropertyValue(FM_PROP_REFVALUE));

                        // add the label to the value list sequence
                        Sequence< OUString>& aThisGroupControlSources = aRadioValueLists[aGroupName];
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
                    }
                    continue;

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
                pDispatchArgs->Name = OUString("ColumnProperties"); // TODO : fmurl.*
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
            for (   FmMapUString2UString::const_iterator aCtrlSource = aRadioControlSources.begin();
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
                OUString fColName = FM_COL_LISTBOX;
                pDispatchArgs->Value <<= fColName;
//              pDispatchArgs->Value <<= (OUString)FM_COL_LISTBOX;
                ++pDispatchArgs;

                // column position
                pDispatchArgs->Name = FMARG_ADDCOL_COLUMNPOS;
                FmMapUString2Int16::const_iterator aOffset = aRadioPositions.find((*aCtrlSource).first);
                DBG_ASSERT(aOffset != aRadioPositions.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                sal_Int16 nPosition = (*aOffset).second;
                nPosition = nPosition + nOffset;
                    // we already inserted nOffset additional columns ....
                pDispatchArgs->Value <<= nPosition;
                ++pDispatchArgs;

                // the
                pDispatchArgs->Name = OUString("ColumnProperties"); // TODO : fmurl.*
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

            m_xExternalDisplayedForm = xForm;
                // do this before dispatching the "attach" command, as the atach may result in a call to our queryDispatch (for the FormSlots)
                // whichs needs the m_xExternalDisplayedForm

            xAttachDispatch->dispatch(aAttachURL, Sequence< PropertyValue>(&aArg, 1));

            m_xExtViewTriggerController = xCurrentNavController;

            // we want to know modifications done in the external view
            // if the external controller is a XFormController we can use all our default handlings for it
            Reference< runtime::XFormController > xFormController( m_xExternalViewController, UNO_QUERY );
            OSL_ENSURE( xFormController.is(), "FmXFormShell::CreateExternalView:: invalid external view controller!" );
            if (xFormController.is())
                xFormController->addActivateListener((XFormControllerListener*)this);
        }
    }
#ifdef DBG_UTIL
    else
    {
        OSL_FAIL("FmXFormShell::CreateExternalView : could not create the external form view !");
    }
#endif
    InvalidateSlot( SID_FM_VIEW_AS_GRID, sal_False );
}

//------------------------------------------------------------------------
void FmXFormShell::implAdjustConfigCache()
{
    // get (cache) the wizard usage flag
    Sequence< OUString > aNames(1);
    aNames[0] = OUString("FormControlPilotsEnabled");
    Sequence< Any > aFlags = GetProperties(aNames);
    if (1 == aFlags.getLength())
        m_bUseWizards = ::cppu::any2bool(aFlags[0]);
}

//------------------------------------------------------------------------
void FmXFormShell::Notify( const com::sun::star::uno::Sequence< OUString >& _rPropertyNames)
{
    if ( impl_checkDisposed() )
        return;

    const OUString* pSearch = _rPropertyNames.getConstArray();
    const OUString* pSearchTil = pSearch + _rPropertyNames.getLength();
    for (;pSearch < pSearchTil; ++pSearch)
        if (0 == pSearch->compareToAscii("FormControlPilotsEnabled"))
        {
            implAdjustConfigCache();
            InvalidateSlot( SID_FM_USE_WIZARDS, sal_True );
        }
}

void FmXFormShell::Commit()
{
}

//------------------------------------------------------------------------
void FmXFormShell::SetWizardUsing(sal_Bool _bUseThem)
{
    m_bUseWizards = _bUseThem;

    Sequence< OUString > aNames(1);
    aNames[0] = OUString("FormControlPilotsEnabled");
    Sequence< Any > aValues(1);
    aValues[0] = ::cppu::bool2any(m_bUseWizards);
    PutProperties(aNames, aValues);
}

//------------------------------------------------------------------------
void FmXFormShell::viewDeactivated( FmFormView& _rCurrentView, sal_Bool _bDeactivateController /* = sal_True */ )
{

    if ( _rCurrentView.GetImpl() && !_rCurrentView.IsDesignMode() )
    {
        _rCurrentView.GetImpl()->Deactivate( _bDeactivateController );
    }

    // if we have an async load operation pending for the 0-th page for this view,
    // we need to cancel this
    FmFormPage* pPage = _rCurrentView.GetCurPage();
    if ( pPage )
    {
        // move all events from our queue to a new one, omit the events for the deactivated
        // page
        ::std::queue< FmLoadAction > aNewEvents;
        while ( !m_aLoadingPages.empty() )
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

    // remove callbacks at the page
    if ( pPage )
    {
        pPage->GetImpl().SetFormsCreationHdl( Link() );
    }
    UpdateForms( sal_True );
}

//------------------------------------------------------------------------
IMPL_LINK( FmXFormShell, OnFirstTimeActivation, void*, /*NOTINTERESTEDIN*/ )
{
    if ( impl_checkDisposed() )
        return 0L;

    m_nActivationEvent = 0;
    SfxObjectShell* pDocument = m_pShell->GetObjectShell();

    if  ( pDocument && !pDocument->HasName() )
    {
        if ( isEnhancedForm() )
        {
            // show the data navigator
            if ( !m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_SHOW_DATANAVIGATOR ) )
                m_pShell->GetViewShell()->GetViewFrame()->ToggleChildWindow( SID_FM_SHOW_DATANAVIGATOR );
        }
    }

    return 0L;
}

//------------------------------------------------------------------------
IMPL_LINK( FmXFormShell, OnFormsCreated, FmFormPage*, /*_pPage*/ )
{
    UpdateForms( sal_True );
    return 0L;
}

//------------------------------------------------------------------------
void FmXFormShell::viewActivated( FmFormView& _rCurrentView, sal_Bool _bSyncAction /* = sal_False */ )
{

    FmFormPage* pPage = _rCurrentView.GetCurPage();

    // activate our view if we are activated ourself
    // FS - 30.06.99 - 67308
    if ( _rCurrentView.GetImpl() && !_rCurrentView.IsDesignMode() )
    {
        // load forms for the page the current view belongs to
        if ( pPage )
        {
            if ( !pPage->GetImpl().hasEverBeenActivated() )
                loadForms( pPage, FORMS_LOAD | ( _bSyncAction ? FORMS_SYNC : FORMS_ASYNC ) );
            pPage->GetImpl().setHasBeenActivated( );
        }

        // first-time initializations for the views
        if ( !_rCurrentView.GetImpl()->hasEverBeenActivated( ) )
        {
            _rCurrentView.GetImpl()->onFirstViewActivation( PTR_CAST( FmFormModel, _rCurrentView.GetModel() ) );
            _rCurrentView.GetImpl()->setHasBeenActivated( );
        }

        // activate the current view
        _rCurrentView.GetImpl()->Activate( _bSyncAction );
    }

    // set callbacks at the page
    if ( pPage )
    {
        pPage->GetImpl().SetFormsCreationHdl( LINK( this, FmXFormShell, OnFormsCreated ) );
    }

    UpdateForms( sal_True );

    if ( !hasEverBeenActivated() )
    {
        m_nActivationEvent = Application::PostUserEvent( LINK( this, FmXFormShell, OnFirstTimeActivation ) );
        setHasBeenActivated();
    }

    // find a default "current form", if there is none, yet
    // #i88186# / 2008-04-12 / frank.schoenheit@sun.com
    impl_defaultCurrentForm_nothrow();
}

//------------------------------------------------------------------------------
void FmXFormShell::impl_defaultCurrentForm_nothrow()
{
    if ( impl_checkDisposed() )
        return;

    if ( m_xCurrentForm.is() )
        // no action required
        return;

    FmFormView* pFormView = m_pShell->GetFormView();
    FmFormPage* pPage = pFormView ? pFormView->GetCurPage() : NULL;
    if ( !pPage )
        return;

    try
    {
        Reference< XIndexAccess > xForms( pPage->GetForms( false ), UNO_QUERY );
        if ( !xForms.is() || !xForms->hasElements() )
            return;

        Reference< XForm > xNewCurrentForm( xForms->getByIndex(0), UNO_QUERY_THROW );
        impl_updateCurrentForm( xNewCurrentForm );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//------------------------------------------------------------------------------
void FmXFormShell::smartControlReset( const Reference< XIndexAccess >& _rxModels )
{
    if (!_rxModels.is())
    {
        OSL_FAIL("FmXFormShell::smartControlReset: invalid container!");
        return;
    }

    static const OUString sClassIdPropertyName = FM_PROP_CLASSID;
    static const OUString sBoundFieldPropertyName = FM_PROP_BOUNDFIELD;
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
IMPL_LINK( FmXFormShell, OnLoadForms, FmFormPage*, /*_pPage*/ )
{
    FmLoadAction aAction = m_aLoadingPages.front();
    m_aLoadingPages.pop();

    loadForms( aAction.pPage, aAction.nFlags & ~FORMS_ASYNC );
    return 0L;
}

//------------------------------------------------------------------------------
namespace
{
    bool lcl_isLoadable( const Reference< XInterface >& _rxLoadable )
    {
        // determines whether a form should be loaded or not
        // if there is no datasource or connection there is no reason to load a form
        Reference< XPropertySet > xSet( _rxLoadable, UNO_QUERY );
        if ( !xSet.is() )
            return false;
        try
        {
            Reference< XConnection > xConn;
            if ( OStaticDataAccessTools().isEmbeddedInDatabase( _rxLoadable.get(), xConn ) )
                return true;

            // is there already a active connection
            xSet->getPropertyValue(FM_PROP_ACTIVE_CONNECTION) >>= xConn;
            if ( xConn.is() )
                return true;

            OUString sPropertyValue;
            OSL_VERIFY( xSet->getPropertyValue( FM_PROP_DATASOURCE ) >>= sPropertyValue );
            if ( !sPropertyValue.isEmpty() )
                return true;

            OSL_VERIFY( xSet->getPropertyValue( FM_PROP_URL ) >>= sPropertyValue );
            if ( !sPropertyValue.isEmpty() )
                return true;
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return false;
    }
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
        xForms = xForms.query( _pPage->GetForms( false ) );

        if ( xForms.is() )
        {
            Reference< XLoadable >  xForm;
            bool                    bFormWasLoaded = false;
            for ( sal_Int32 j = 0, nCount = xForms->getCount(); j < nCount; ++j )
            {
                xForms->getByIndex( j ) >>= xForm;
                bFormWasLoaded = false;
                // a database form must be loaded for
                try
                {
                    if ( 0 == ( _nBehaviour & FORMS_UNLOAD ) )
                    {
                        if ( lcl_isLoadable( xForm ) && !xForm->isLoaded() )
                            xForm->load();
                    }
                    else
                    {
                        if ( xForm->isLoaded() )
                        {
                            bFormWasLoaded = true;
                            xForm->unload();
                        }
                    }
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
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
//------------------------------------------------------------------------
void FmXFormShell::handleShowPropertiesRequest()
{
    if ( onlyControlsAreMarked() )
        ShowSelectionProperties( sal_True );
}

//------------------------------------------------------------------------
void FmXFormShell::handleMouseButtonDown( const SdrViewEvent& _rViewEvent )
{
    // catch simple double clicks
    if ( ( _rViewEvent.nMouseClicks == 2 ) && ( _rViewEvent.nMouseCode == MOUSE_LEFT ) )
    {
        if ( _rViewEvent.eHit == SDRHIT_MARKEDOBJECT )
        {
            if ( onlyControlsAreMarked() )
                ShowSelectionProperties( sal_True );
        }
    }
}

//------------------------------------------------------------------------------
bool FmXFormShell::HasControlFocus() const
{
    bool bHasControlFocus = false;

    try
    {
        Reference< XFormController > xController( getActiveController() );
        Reference< XControl > xCurrentControl;
        if ( xController.is() )
            xCurrentControl.set( xController->getCurrentControl() );
        if ( xCurrentControl.is() )
        {
            Reference< XWindow2 > xPeerWindow( xCurrentControl->getPeer(), UNO_QUERY_THROW );
            bHasControlFocus = xPeerWindow->hasFocus();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return bHasControlFocus;
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
            m_sCurrentValue = OUString();
            return sal_True;
        }
    }

    return sal_False;
}

//------------------------------------------------------------------------------
sal_Bool SearchableControlIterator::ShouldStepInto(const Reference< XInterface>& /*xContainer*/) const
{
    return sal_True;
}

//==============================================================================
//==============================================================================

SFX_IMPL_MENU_CONTROL(ControlConversionMenuController, SfxBoolItem);

//------------------------------------------------------------------------------
ControlConversionMenuController::ControlConversionMenuController( sal_uInt16 _nId, Menu& _rMenu, SfxBindings& _rBindings )
    :SfxMenuControl( _nId, _rBindings )
    ,m_pMainMenu( &_rMenu )
    ,m_pConversionMenu( NULL )
{
    if ( _nId == SID_FM_CHANGECONTROLTYPE )
    {
        m_pConversionMenu = FmXFormShell::GetConversionMenu();
        _rMenu.SetPopupMenu( _nId, m_pConversionMenu );

        for (sal_Int16 i=0; i<m_pConversionMenu->GetItemCount(); ++i)
        {
            _rBindings.Invalidate(m_pConversionMenu->GetItemId(i));
            SfxStatusForwarder* pForwarder = new SfxStatusForwarder(m_pConversionMenu->GetItemId(i), *this);
            m_aStatusForwarders.push_back(pForwarder);
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
    else if (FmXFormShell::isControlConversionSlot(nSID))
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
            sal_uInt16 nSourcePos = pSource->GetItemPos(nSID);
            DBG_ASSERT(nSourcePos != MENU_ITEM_NOTFOUND, "ControlConversionMenuController::StateChanged : FmXFormShell supplied an invalid menu !");
            sal_uInt16 nPrevInSource = nSourcePos;
            sal_uInt16 nPrevInConversion = MENU_ITEM_NOTFOUND;
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
                nPrevInConversion = sal::static_int_cast< sal_uInt16 >(-1); // put the item at the first position
            m_pConversionMenu->InsertItem(nSID, pSource->GetItemText(nSID),
                pSource->GetItemBits(nSID), OString(), ++nPrevInConversion);
            m_pConversionMenu->SetItemImage(nSID, pSource->GetItemImage(nSID));
            m_pConversionMenu->SetHelpId(nSID, pSource->GetHelpId(nSID));

            delete pSource;
        }
        m_pMainMenu->EnableItem(SID_FM_CHANGECONTROLTYPE, m_pConversionMenu->GetItemCount() > 0);
    }
    else
    {
        OSL_FAIL("ControlConversionMenuController::StateChanged : unknown id !");
    }
}

//==============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
