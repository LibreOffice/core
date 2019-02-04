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
#include <sal/log.hxx>
#include <fmobj.hxx>
#include <fmpgeimp.hxx>
#include <svx/fmtools.hxx>
#include <fmprop.hxx>
#include <fmservs.hxx>
#include <fmshimp.hxx>
#include <fmtextcontrolshell.hxx>
#include <fmundo.hxx>
#include <fmurl.hxx>
#include <fmvwimp.hxx>
#include <formtoolbars.hxx>
#include <gridcols.hxx>
#include <svx/svditer.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/fmglob.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/fmshell.hxx>
#include <svx/obj3d.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <bitmaps.hlst>

#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
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

#include <comphelper/evtmethodhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/solarmutex.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>

// is used for Invalidate -> maintain it as well
static const sal_uInt16 DatabaseSlotMap[] =
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

// is used for Invalidate -> maintain it as well
// sort ascending !!!!!!
static const sal_Int16 DlgSlotMap[] =    // slots of the controller
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

static const sal_Int16 SelObjectSlotMap[] =  // slots depending on the SelObject
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

// the following arrays must be consistent, i.e., corresponding entries should
// be at the same relative position within their respective arrays
static const char* aConvertSlots[] =
{
    "ConvertToEdit",
    "ConvertToButton",
    "ConvertToFixed",
    "ConvertToList",
    "ConvertToCheckBox",
    "ConvertToRadio",
    "ConvertToGroup",
    "ConvertToCombo",
    "ConvertToImageBtn",
    "ConvertToFileControl",
    "ConvertToDate",
    "ConvertToTime",
    "ConvertToNumeric",
    "ConvertToCurrency",
    "ConvertToPattern",
    "ConvertToImageControl",
    "ConvertToFormatted",
    "ConvertToScrollBar",
    "ConvertToSpinButton",
    "ConvertToNavigationBar"
};

static const OUStringLiteral aImgIds[] =
{
    RID_SVXBMP_EDITBOX,
    RID_SVXBMP_BUTTON,
    RID_SVXBMP_FIXEDTEXT,
    RID_SVXBMP_LISTBOX,
    RID_SVXBMP_CHECKBOX,
    RID_SVXBMP_RADIOBUTTON,
    RID_SVXBMP_GROUPBOX,
    RID_SVXBMP_COMBOBOX,
    RID_SVXBMP_IMAGEBUTTON,
    RID_SVXBMP_FILECONTROL,
    RID_SVXBMP_DATEFIELD,
    RID_SVXBMP_TIMEFIELD,
    RID_SVXBMP_NUMERICFIELD,
    RID_SVXBMP_CURRENCYFIELD,
    RID_SVXBMP_PATTERNFIELD,
    RID_SVXBMP_IMAGECONTROL,
    RID_SVXBMP_FORMATTEDFIELD,
    RID_SVXBMP_SCROLLBAR,
    RID_SVXBMP_SPINBUTTON,
    RID_SVXBMP_NAVIGATIONBAR
};

static const sal_Int16 nObjectTypes[] =
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
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::script;
using namespace ::svxform;
using namespace ::svx;
using namespace ::dbtools;


//= helper

namespace
{

    void collectInterfacesFromMarkList( const SdrMarkList& _rMarkList, InterfaceBag& /* [out] */ _rInterfaces )
    {
        _rInterfaces.clear();

        const size_t nMarkCount = _rMarkList.GetMarkCount();
        for ( size_t i = 0; i < nMarkCount; ++i)
        {
            SdrObject* pCurrent = _rMarkList.GetMark( i )->GetMarkedSdrObj();

            std::unique_ptr<SdrObjListIter> pGroupIterator;
            if ( pCurrent->IsGroupObject() )
            {
                pGroupIterator.reset(new SdrObjListIter( pCurrent->GetSubList() ));
                pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : nullptr;
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
                pCurrent = pGroupIterator && pGroupIterator->IsMore() ? pGroupIterator->Next() : nullptr;
            }
        }
    }


    sal_Int32 GridView2ModelPos(const Reference< XIndexAccess>& rColumns, sal_Int16 nViewPos)
    {
        try
        {
            if (rColumns.is())
            {
                // loop through all columns
                sal_Int32 i;
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
            DBG_UNHANDLED_EXCEPTION("svx");
        }
        return -1;
    }


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

        // check for the index of the model within its parent
        Reference< XIndexAccess> xParentIndex(xModelChild->getParent(), UNO_QUERY);
        if (!xParentIndex.is())
            return; // nothing to do
        sal_Int32 nIndex = getElementPos(xParentIndex, xModel);
        if (nIndex<0 || nIndex>=xParentIndex->getCount())
            return; // nothing to do

        // then we need information about the listeners supported by the control and the model
        Sequence< Type> aModelListeners;
        Sequence< Type> aControlListeners;

        Reference< XIntrospection> xIntrospection = theIntrospection::get(::comphelper::getProcessComponentContext());

        if (xModel.is())
        {
            Any aModel(makeAny(xModel));
            aModelListeners = xIntrospection->inspect(aModel)->getSupportedListeners();
        }

        if (xControl.is())
        {
            Any aControl(makeAny(xControl));
            aControlListeners = xIntrospection->inspect(aControl)->getSupportedListeners();
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
                    pCurrentArray = (pCurrentArray == &aModelListeners) ? &aControlListeners : nullptr
                )
            {
                const Type* pCurrentListeners = pCurrentArray->getConstArray();
                for (j=0; j<pCurrentArray->getLength(); ++j, ++pCurrentListeners)
                {
                    OUString aListener = (*pCurrentListeners).getTypeName();
                    if (!aListener.isEmpty())
                        aListener = aListener.copy(aListener.lastIndexOf('.')+1);

                    if (aListener == pCurrent->ListenerType)
                        // the current ScriptEventDescriptor doesn't match the current listeners class
                        continue;

                    // now check the methods
                    Sequence< OUString> aMethodsNames = ::comphelper::getEventMethodsForType(*pCurrentListeners);

                    const OUString* pMethodsNames = aMethodsNames.getConstArray();
                    for (k=0; k<aMethodsNames.getLength(); ++k, ++pMethodsNames)
                    {
                        if ((*pMethodsNames) != pCurrent->EventMethod)
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


    OUString getServiceNameByControlType(sal_Int16 nType)
    {
        switch (nType)
        {
            case OBJ_FM_EDIT            : return OUString(FM_COMPONENT_TEXTFIELD);
            case OBJ_FM_BUTTON          : return OUString(FM_COMPONENT_COMMANDBUTTON);
            case OBJ_FM_FIXEDTEXT       : return OUString(FM_COMPONENT_FIXEDTEXT);
            case OBJ_FM_LISTBOX         : return OUString(FM_COMPONENT_LISTBOX);
            case OBJ_FM_CHECKBOX        : return OUString(FM_COMPONENT_CHECKBOX);
            case OBJ_FM_RADIOBUTTON     : return OUString(FM_COMPONENT_RADIOBUTTON);
            case OBJ_FM_GROUPBOX        : return OUString(FM_COMPONENT_GROUPBOX);
            case OBJ_FM_COMBOBOX        : return OUString(FM_COMPONENT_COMBOBOX);
            case OBJ_FM_GRID            : return OUString(FM_COMPONENT_GRIDCONTROL);
            case OBJ_FM_IMAGEBUTTON     : return OUString(FM_COMPONENT_IMAGEBUTTON);
            case OBJ_FM_FILECONTROL     : return OUString(FM_COMPONENT_FILECONTROL);
            case OBJ_FM_DATEFIELD       : return OUString(FM_COMPONENT_DATEFIELD);
            case OBJ_FM_TIMEFIELD       : return OUString(FM_COMPONENT_TIMEFIELD);
            case OBJ_FM_NUMERICFIELD    : return OUString(FM_COMPONENT_NUMERICFIELD);
            case OBJ_FM_CURRENCYFIELD   : return OUString(FM_COMPONENT_CURRENCYFIELD);
            case OBJ_FM_PATTERNFIELD    : return OUString(FM_COMPONENT_PATTERNFIELD);
            case OBJ_FM_HIDDEN          : return OUString(FM_COMPONENT_HIDDENCONTROL);
            case OBJ_FM_IMAGECONTROL    : return OUString(FM_COMPONENT_IMAGECONTROL);
            case OBJ_FM_FORMATTEDFIELD  : return OUString(FM_COMPONENT_FORMATTEDFIELD);
            case OBJ_FM_SCROLLBAR       : return OUString(FM_SUN_COMPONENT_SCROLLBAR);
            case OBJ_FM_SPINBUTTON      : return OUString(FM_SUN_COMPONENT_SPINBUTTON);
            case OBJ_FM_NAVIGATIONBAR   : return OUString(FM_SUN_COMPONENT_NAVIGATIONBAR);
        }
        return OUString();
    }

}


// check if the control has one of the interfaces we can use for searching
// *_pCurrentText will be filled with the current text of the control (as used when searching this control)
bool IsSearchableControl( const css::uno::Reference< css::uno::XInterface>& _rxControl,
    OUString* _pCurrentText )
{
    if ( !_rxControl.is() )
        return false;

    Reference< XTextComponent > xAsText( _rxControl, UNO_QUERY );
    if ( xAsText.is() )
    {
        if ( _pCurrentText )
            *_pCurrentText = xAsText->getText();
        return true;
    }

    Reference< XListBox > xListBox( _rxControl, UNO_QUERY );
    if ( xListBox.is() )
    {
        if ( _pCurrentText )
            *_pCurrentText = xListBox->getSelectedItem();
        return true;
    }

    Reference< XCheckBox > xCheckBox( _rxControl, UNO_QUERY );
    if ( xCheckBox.is() )
    {
        if ( _pCurrentText )
        {
            switch ( static_cast<::TriState>(xCheckBox->getState()) )
            {
                case TRISTATE_FALSE: *_pCurrentText = "0"; break;
                case TRISTATE_TRUE: *_pCurrentText = "1"; break;
                default: _pCurrentText->clear(); break;
            }
        }
        return true;
    }

    return false;
}


bool FmXBoundFormFieldIterator::ShouldStepInto(const Reference< XInterface>& _rContainer) const
{
    if (_rContainer == m_xStartingPoint)
        // would be quite stupid to step over the root ....
        return true;

    return Reference< XControlModel>(_rContainer, UNO_QUERY).is();
}


bool FmXBoundFormFieldIterator::ShouldHandleElement(const Reference< XInterface>& _rElement)
{
    if (!_rElement.is())
        // NULL element
        return false;

    if (Reference< XForm>(_rElement, UNO_QUERY).is() || Reference< XGrid>(_rElement, UNO_QUERY).is())
        // a forms or a grid
        return false;

    Reference< XPropertySet> xSet(_rElement, UNO_QUERY);
    if (!xSet.is() || !::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
        // no "BoundField" property
        return false;

    Any aVal( xSet->getPropertyValue(FM_PROP_BOUNDFIELD) );
    if (aVal.getValueTypeClass() != TypeClass_INTERFACE)
        // void or invalid property value
        return false;

    return aVal.hasValue();
}


static bool isControlList(const SdrMarkList& rMarkList)
{
    // the list contains only controls and at least one control
    const size_t nMarkCount = rMarkList.GetMarkCount();
    bool  bControlList = nMarkCount != 0;

    bool bHadAnyLeafs = false;

    for (size_t i = 0; i < nMarkCount && bControlList; ++i)
    {
        SdrObject *pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
        E3dObject* pAs3DObject = dynamic_cast< E3dObject* >( pObj);
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
                SdrObjListIter aIter(pObj->GetSubList());
                while (aIter.IsMore() && bControlList)
                {
                    bControlList = SdrInventor::FmForm == aIter.Next()->GetObjInventor();
                    bHadAnyLeafs = true;
                }
            }
            else
            {
                bHadAnyLeafs = true;
                bControlList = SdrInventor::FmForm == pObj->GetObjInventor();
            }
        }
    }

    return bControlList && bHadAnyLeafs;
}


static Reference< XForm > GetForm(const Reference< XInterface>& _rxElement)
{
    Reference< XForm > xForm( _rxElement, UNO_QUERY );
    if ( xForm.is() )
        return xForm;

    Reference< XChild > xChild( _rxElement, UNO_QUERY );
    if ( xChild.is() )
        return GetForm( xChild->getParent() );

    return Reference< XForm >();
}

FmXFormShell_Base_Disambiguation::FmXFormShell_Base_Disambiguation( ::osl::Mutex& _rMutex )
    :FmXFormShell_BD_BASE( _rMutex )
{
}

FmXFormShell::FmXFormShell( FmFormShell& _rShell, SfxViewFrame* _pViewFrame )
        :FmXFormShell_BASE(m_aMutex)
        ,FmXFormShell_CFGBASE("Office.Common/Misc", ConfigItemMode::NONE)
        ,m_eNavigate( NavigationBarMode_NONE )
        ,m_nInvalidationEvent( nullptr )
        ,m_nActivationEvent( nullptr )
        ,m_pShell( &_rShell )
        ,m_pTextShell( new svx::FmTextControlShell( _pViewFrame ) )
        ,m_aActiveControllerFeatures( this )
        ,m_aNavControllerFeatures( this )
        ,m_eDocumentType( eUnknownDocumentType )
        ,m_nLockSlotInvalidation( 0 )
        ,m_bHadPropertyBrowserInDesignMode( false )
        ,m_bTrackProperties( true )
        ,m_bUseWizards( true )
        ,m_bDatabaseBar( false )
        ,m_bInActivate( false )
        ,m_bSetFocus( false )
        ,m_bFilterMode( false )
        ,m_bChangingDesignMode( false )
        ,m_bPreparedClose( false )
        ,m_bFirstActivation( true )
{
    m_aMarkTimer.SetTimeout(100);
    m_aMarkTimer.SetInvokeHandler(LINK(this, FmXFormShell, OnTimeOut_Lock));
    m_aMarkTimer.SetDebugName("svx::FmXFormShell m_aMarkTimer");

    m_xAttachedFrame = _pViewFrame->GetFrame().GetFrameInterface();

    // to prevent deletion of this we acquire our refcounter once
    osl_atomic_increment(&m_refCount);

    // correct the refcounter
    osl_atomic_decrement(&m_refCount);

    // cache the current configuration settings we're interested in
    implAdjustConfigCache_Lock();
    // and register for changes on this settings
    Sequence< OUString > aNames { "FormControlPilotsEnabled" };
    EnableNotification(aNames);
}


FmXFormShell::~FmXFormShell()
{
}


Reference< css::frame::XModel > FmXFormShell::getContextDocument_Lock() const
{
    Reference< css::frame::XModel > xModel;

    // determine the type of document we live in
    try
    {
        Reference< css::frame::XController > xController;
        if ( m_xAttachedFrame.is() )
            xController = m_xAttachedFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    return xModel;
}


bool FmXFormShell::isEnhancedForm_Lock() const
{
    return getDocumentType_Lock() == eEnhancedForm;
}


bool FmXFormShell::impl_checkDisposed_Lock() const
{
    DBG_TESTSOLARMUTEX();
    if ( !m_pShell )
    {
        OSL_FAIL( "FmXFormShell::impl_checkDisposed: already disposed!" );
        return true;
    }
    return false;
}


::svxform::DocumentType FmXFormShell::getDocumentType_Lock() const
{
    if ( m_eDocumentType != eUnknownDocumentType )
        return m_eDocumentType;

    // determine the type of document we live in
    Reference<css::frame::XModel> xModel = getContextDocument_Lock();
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


bool FmXFormShell::IsReadonlyDoc_Lock() const
{
    if (impl_checkDisposed_Lock())
        return true;

    FmFormModel* pModel = m_pShell->GetFormModel();
    if ( pModel && pModel->GetObjectShell() )
        return pModel->GetObjectShell()->IsReadOnly() || pModel->GetObjectShell()->IsReadOnlyUI();
    return true;
}

//  EventListener

void SAL_CALL FmXFormShell::disposing(const lang::EventObject& e)
{
    SolarMutexGuard g;

    if (m_xActiveController == e.Source)
    {
        // the controller will release, then release everything
        stopListening_Lock();
        m_xActiveForm = nullptr;
        m_xActiveController = nullptr;
        m_xNavigationController = nullptr;

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
            xFormController->removeActivateListener(static_cast<XFormControllerListener*>(this));

        Reference< css::lang::XComponent> xComp(m_xExternalViewController, UNO_QUERY);
        if (xComp.is())
            xComp->removeEventListener(static_cast<XEventListener*>(static_cast<XPropertyChangeListener*>(this)));

        m_xExternalViewController = nullptr;
        m_xExternalDisplayedForm = nullptr;
        m_xExtViewTriggerController = nullptr;

        InvalidateSlot_Lock( SID_FM_VIEW_AS_GRID, false );
    }
}


void SAL_CALL FmXFormShell::propertyChange(const PropertyChangeEvent& evt)
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock())
        return;

    if (evt.PropertyName == FM_PROP_ROWCOUNT)
    {
        // The update following this forces a re-painting of the corresponding
        // slots. But if I am not in the MainThread of the application (because,
        // for example, a cursor is counting data sets at the moment and always
        // gives me this PropertyChanges), this can clash with normal paints in
        // the MainThread of the application. (Such paints happen, for example,
        // if one simply places another application over the office and switches
        // back again).
        // Therefore the use of the SolarMutex, which safeguards that.
        comphelper::SolarMutex& rSolarSafety = Application::GetSolarMutex();
        if (rSolarSafety.tryToAcquire())
        {
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_RECORD_TOTAL, true);
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Update(SID_FM_RECORD_TOTAL);
            rSolarSafety.release();
        }
        else
        {
            // with the following the slot is invalidated asynchron
            LockSlotInvalidation_Lock(true);
            InvalidateSlot_Lock(SID_FM_RECORD_TOTAL, false);
            LockSlotInvalidation_Lock(false);
        }
    }

    // this may be called from a non-main-thread so invalidate the shell asynchronously
    LockSlotInvalidation_Lock(true);
    InvalidateSlot_Lock(0, false); // special meaning : invalidate m_pShell
    LockSlotInvalidation_Lock(false);
}


void FmXFormShell::invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures )
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock())
        return;

    OSL_ENSURE( !_rFeatures.empty(), "FmXFormShell::invalidateFeatures: invalid arguments!" );

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


void SAL_CALL FmXFormShell::formActivated(const lang::EventObject& rEvent)
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock())
        return;

    Reference< runtime::XFormController > xController( rEvent.Source, UNO_QUERY_THROW );
    m_pTextShell->formActivated( xController );
    setActiveController_Lock(xController);
}


void SAL_CALL FmXFormShell::formDeactivated(const lang::EventObject& rEvent)
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock())
        return;

    Reference< runtime::XFormController > xController( rEvent.Source, UNO_QUERY_THROW );
    m_pTextShell->formDeactivated( xController );
}


void FmXFormShell::disposing()
{
    SolarMutexGuard g;

    FmXFormShell_BASE::disposing();

    if ( m_pShell && !m_pShell->IsDesignMode() )
        setActiveController_Lock(nullptr, true);
        // do NOT save the content of the old form (the second parameter tells this)
        // if we're here, then we expect that PrepareClose has been called, and thus the user
        // got a chance to commit or reject any changes. So in case we're here and there
        // are still uncommitted changes, the user explicitly wanted this.

    m_pTextShell->dispose();

    m_xAttachedFrame = nullptr;

    CloseExternalFormViewer_Lock();

    while ( !m_aLoadingPages.empty() )
    {
        Application::RemoveUserEvent( m_aLoadingPages.front().nEventId );
        m_aLoadingPages.pop();
    }

    {
        if (m_nInvalidationEvent)
        {
            Application::RemoveUserEvent(m_nInvalidationEvent);
            m_nInvalidationEvent = nullptr;
        }
        if ( m_nActivationEvent )
        {
            Application::RemoveUserEvent( m_nActivationEvent );
            m_nActivationEvent = nullptr;
        }
    }

    {
        DBG_ASSERT(!m_nInvalidationEvent, "FmXFormShell::~FmXFormShell : still have an invalidation event !");
            // should have been deleted while being disposed

        m_aMarkTimer.Stop();
    }

    DisableNotification();

    RemoveElement_Lock(m_xForms);
    m_xForms.clear();

    impl_switchActiveControllerListening_Lock(false);
    m_xActiveController         = nullptr;
    m_xActiveForm               = nullptr;

    m_pShell                    = nullptr;
    m_xNavigationController     = nullptr;
    m_xCurrentForm              = nullptr;
    m_xLastGridFound            = nullptr;
    m_xAttachedFrame            = nullptr;
    m_xExternalViewController   = nullptr;
    m_xExtViewTriggerController = nullptr;
    m_xExternalDisplayedForm    = nullptr;

    InterfaceBag aEmpty;
    m_aCurrentSelection.swap( aEmpty );

    m_aActiveControllerFeatures.dispose();
    m_aNavControllerFeatures.dispose();
}


void FmXFormShell::UpdateSlot_Lock(sal_Int16 _nId)
{
    if (impl_checkDisposed_Lock())
        return;

    if ( m_nLockSlotInvalidation )
    {
        OSL_FAIL( "FmXFormShell::UpdateSlot: cannot update if invalidation is currently locked!" );
        InvalidateSlot_Lock(_nId, false);
    }
    else
    {
        OSL_ENSURE( _nId, "FmXFormShell::UpdateSlot: can't update the complete shell!" );
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate( _nId, true, true );
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Update( _nId );
    }
}


void FmXFormShell::InvalidateSlot_Lock(sal_Int16 nId, bool bWithId)
{
    if (impl_checkDisposed_Lock())
        return;

    if (m_nLockSlotInvalidation)
    {
        sal_uInt8 nFlags = ( bWithId ? 0x01 : 0 );
        m_arrInvalidSlots.emplace_back(nId, nFlags );
    }
    else
        if (nId)
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(nId, true, bWithId);
        else
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}


void FmXFormShell::LockSlotInvalidation_Lock(bool bLock)
{
    if (impl_checkDisposed_Lock())
        return;

    DBG_ASSERT(bLock || m_nLockSlotInvalidation>0, "FmXFormShell::LockSlotInvalidation : invalid call !");

    if (bLock)
        ++m_nLockSlotInvalidation;
    else if (!--m_nLockSlotInvalidation)
    {
        // (asynchronously) invalidate everything accumulated during the locked phase
        if (!m_nInvalidationEvent)
            m_nInvalidationEvent = Application::PostUserEvent(LINK(this, FmXFormShell, OnInvalidateSlots_Lock));
    }
}


IMPL_LINK_NOARG(FmXFormShell, OnInvalidateSlots_Lock, void*,void)
{
    if (impl_checkDisposed_Lock())
        return;

    m_nInvalidationEvent = nullptr;

    for (const auto& rInvalidSlot : m_arrInvalidSlots)
    {
        if (rInvalidSlot.id)
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(rInvalidSlot.id, true, (rInvalidSlot.flags & 0x01));
        else
            m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
    }
    m_arrInvalidSlots.clear();
}


void FmXFormShell::ForceUpdateSelection_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    if (IsSelectionUpdatePending_Lock())
    {
        m_aMarkTimer.Stop();

        // optionally turn off the invalidation of slots which is implicitly done by SetSelection
        LockSlotInvalidation_Lock(true);

        SetSelection_Lock(m_pShell->GetFormView()->GetMarkedObjectList());

        LockSlotInvalidation_Lock(false);
    }
}

VclBuilder* FmXFormShell::GetConversionMenu_Lock()
{
    VclBuilder* pBuilder = new VclBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/convertmenu.ui", "");
    VclPtr<PopupMenu> pNewMenu(pBuilder->get_menu("menu"));
    for (size_t i = 0; i < SAL_N_ELEMENTS(aConvertSlots); ++i)
    {
        // the corresponding image at it
        pNewMenu->SetItemImage(pNewMenu->GetItemId(aConvertSlots[i]), Image(StockImage::Yes, aImgIds[i]));
    }
    return pBuilder;
}

OString FmXFormShell::SlotToIdent(sal_uInt16 nSlot)
{
    assert(SAL_N_ELEMENTS(SelObjectSlotMap) >= SAL_N_ELEMENTS(aConvertSlots));

    for (size_t i = 0; i < SAL_N_ELEMENTS(aConvertSlots); ++i)
    {
        if (nSlot == SelObjectSlotMap[i])
            return aConvertSlots[i];
    }

    return OString();
}

bool FmXFormShell::isControlConversionSlot(const OString& rIdent)
{
    for (const auto& rConvertSlot : aConvertSlots)
        if (rIdent == rConvertSlot)
            return true;
    return false;
}

void FmXFormShell::executeControlConversionSlot_Lock(const OString &rIdent)
{
    OSL_PRECOND( canConvertCurrentSelectionToControl_Lock(rIdent), "FmXFormShell::executeControlConversionSlot: illegal call!" );
    InterfaceBag::const_iterator aSelectedElement = m_aCurrentSelection.begin();
    if ( aSelectedElement == m_aCurrentSelection.end() )
        return;

    executeControlConversionSlot_Lock(Reference<XFormComponent>(*aSelectedElement, UNO_QUERY), rIdent);
}

bool FmXFormShell::executeControlConversionSlot_Lock(const Reference<XFormComponent>& _rxObject, const OString& rIdent)
{
    if (impl_checkDisposed_Lock())
        return false;

    OSL_ENSURE( _rxObject.is(), "FmXFormShell::executeControlConversionSlot: invalid object!" );
    if ( !_rxObject.is() )
        return false;

    SdrPage* pPage = m_pShell->GetCurPage();
    FmFormPage* pFormPage = dynamic_cast< FmFormPage* >( pPage );
    OSL_ENSURE( pFormPage, "FmXFormShell::executeControlConversionSlot: no current (form) page!" );
    if ( !pFormPage )
        return false;

    OSL_ENSURE( isSolelySelected_Lock(_rxObject),
        "FmXFormShell::executeControlConversionSlot: hmm ... shouldn't this parameter be redundant?" );

    for (size_t lookupSlot = 0; lookupSlot < SAL_N_ELEMENTS(aConvertSlots); ++lookupSlot)
    {
        if (rIdent == aConvertSlots[lookupSlot])
        {
            Reference< XInterface > xNormalizedObject( _rxObject, UNO_QUERY );

            FmFormObj* pFormObject = nullptr;
            SdrObjListIter aPageIter( pFormPage );
            while ( aPageIter.IsMore() )
            {
                SdrObject* pCurrent = aPageIter.Next();
                pFormObject = FmFormObj::GetFormObject( pCurrent );
                if ( !pFormObject )
                    continue;

                Reference< XInterface > xCurrentNormalized( pFormObject->GetUnoControlModel(), UNO_QUERY );
                if ( xCurrentNormalized.get() == xNormalizedObject.get() )
                    break;

                pFormObject = nullptr;
            }

            if ( !pFormObject )
                return false;

            OUString sNewName( getServiceNameByControlType( nObjectTypes[ lookupSlot ] ) );
            Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
            Reference< XControlModel> xNewModel( xContext->getServiceManager()->createInstanceWithContext(sNewName, xContext), UNO_QUERY );
            if (!xNewModel.is())
                return false;

            Reference< XControlModel> xOldModel( pFormObject->GetUnoControlModel() );

            // transfer properties
            Reference< XPropertySet> xOldSet(xOldModel, UNO_QUERY);
            Reference< XPropertySet> xNewSet(xNewModel, UNO_QUERY);


            lang::Locale aNewLanguage = Application::GetSettings().GetUILanguageTag().getLocale();
            TransferFormComponentProperties(xOldSet, xNewSet, aNewLanguage);

            Sequence< css::script::ScriptEventDescriptor> aOldScripts;
            Reference< XChild> xChild(xOldModel, UNO_QUERY);
            if (xChild.is())
            {
                Reference< XIndexAccess> xParent(xChild->getParent(), UNO_QUERY);

                // remember old script events
                Reference< css::script::XEventAttacherManager> xEvManager(xChild->getParent(), UNO_QUERY);
                if (xParent.is() && xEvManager.is())
                {
                    sal_Int32 nIndex = getElementPos(xParent, xOldModel);
                    if (nIndex>=0 && nIndex<xParent->getCount())
                        aOldScripts = xEvManager->getScriptEvents(nIndex);
                }

                // replace the model within the parent container
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
                            Reference< css::lang::XComponent> xNewComponent(xNewModel, UNO_QUERY);
                            if (xNewComponent.is())
                                xNewComponent->dispose();
                            return false;
                        }
                    }
                    catch(Exception&)
                    {
                        OSL_FAIL("FmXFormShell::executeControlConversionSlot: could not replace the model !");
                        Reference< css::lang::XComponent> xNewComponent(xNewModel, UNO_QUERY);
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

            // set new model
            pFormObject->SetChanged();
            pFormObject->SetUnoControlModel(xNewModel);

            // transfer script events
            // (do this _after_ SetUnoControlModel as we need the new (implicitly created) control)
            if (aOldScripts.getLength())
            {
                // find the control for the model
                Reference<XControlContainer> xControlContainer(getControlContainerForView_Lock());

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
                        xOldBindable->setValueBinding( nullptr );
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("svx");
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
                        xOldSink->setListEntrySource( nullptr );
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("svx");
                    }
                }
            }

            // create an undo action
            FmFormModel* pModel = m_pShell->GetFormModel();
            DBG_ASSERT(pModel != nullptr, "FmXFormShell::executeControlConversionSlot: my shell has no model !");
            if (pModel && pModel->IsUndoEnabled() )
            {
                pModel->AddUndo(std::make_unique<FmUndoModelReplaceAction>(*pModel, pFormObject, xOldModel));
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

bool FmXFormShell::canConvertCurrentSelectionToControl_Lock(const OString& rIdent)
{
    if ( m_aCurrentSelection.empty() )
        return false;

    InterfaceBag::const_iterator aCheck = m_aCurrentSelection.begin();
    Reference< lang::XServiceInfo > xElementInfo( *aCheck, UNO_QUERY );
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

    DBG_ASSERT(SAL_N_ELEMENTS(aConvertSlots) == SAL_N_ELEMENTS(nObjectTypes),
        "FmXFormShell::canConvertCurrentSelectionToControl: aConvertSlots & nObjectTypes must have the same size !");

    for (size_t i = 0; i < SAL_N_ELEMENTS(aConvertSlots); ++i)
        if (rIdent == aConvertSlots[i])
            return nObjectTypes[i] != nObjectType;

    return true;    // all other slots: assume "yes"
}

void FmXFormShell::checkControlConversionSlotsForCurrentSelection_Lock(Menu& rMenu)
{
    for (sal_uInt16 i = 0; i < rMenu.GetItemCount(); ++i)
    {
        // the context is already of a type that corresponds to the entry -> disable
        const sal_uInt16 nId = rMenu.GetItemId(i);
        rMenu.EnableItem(nId, canConvertCurrentSelectionToControl_Lock(rMenu.GetItemIdent(nId)));
    }
}

void FmXFormShell::LoopGrids_Lock(LoopGridsSync nSync, LoopGridsFlags nFlags)
{
    if (impl_checkDisposed_Lock())
        return;

    Reference< XIndexContainer> xControlModels(m_xActiveForm, UNO_QUERY);
    if (xControlModels.is())
    {
        for (sal_Int32 i=0; i<xControlModels->getCount(); ++i)
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

            switch (nSync)
            {
                case LoopGridsSync::DISABLE_SYNC:
                {
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, Any(false));
                }
                break;
                case LoopGridsSync::FORCE_SYNC:
                {
                    Any aOldVal( xModelSet->getPropertyValue(FM_PROP_DISPLAYSYNCHRON) );
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, Any(true));
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, aOldVal);
                }
                break;
                case LoopGridsSync::ENABLE_SYNC:
                {
                    xModelSet->setPropertyValue(FM_PROP_DISPLAYSYNCHRON, Any(true));
                }
                break;
            }

            if (nFlags & LoopGridsFlags::DISABLE_ROCTRLR)
            {
                xModelSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, Any(false));
                Reference< XPropertyState> xModelPropState(xModelSet, UNO_QUERY);
                if (xModelPropState.is())
                    xModelPropState->setPropertyToDefault(FM_PROP_CURSORCOLOR);
                else
                    xModelSet->setPropertyValue(FM_PROP_CURSORCOLOR, Any());        // this should be the default
            }
        }
    }
}


Reference< XControlContainer > FmXFormShell::getControlContainerForView_Lock()
{
    if (impl_checkDisposed_Lock())
        return nullptr;

    SdrPageView* pPageView = nullptr;
    if ( m_pShell && m_pShell->GetFormView() )
        pPageView = m_pShell->GetFormView()->GetSdrPageView();

    Reference< XControlContainer> xControlContainer;
    if ( pPageView )
        xControlContainer = pPageView->GetPageWindow(0)->GetControlContainer();

    return xControlContainer;
}


void FmXFormShell::ExecuteTabOrderDialog_Lock(const Reference<XTabControllerModel>& _rxForForm)
{
    if (impl_checkDisposed_Lock())
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
                _rxForForm, getControlContainerForView_Lock(), xParentWindow
            );

        xDialog->execute();
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FmXFormShell::ExecuteTabOrderDialog: caught an exception!" );
    }
}


void FmXFormShell::ExecuteSearch_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    // a collection of all (logical) forms
    FmFormArray aEmpty;
    m_aSearchForms.swap( aEmpty );
    ::std::vector< OUString > aContextNames;
    impl_collectFormSearchContexts_nothrow_Lock(
        m_pShell->GetCurPage()->GetForms(), OUString(),
        m_aSearchForms, aContextNames);

    if ( m_aSearchForms.size() != aContextNames.size() )
    {
        SAL_WARN ( "svx.form", "FmXFormShell::ExecuteSearch: nonsense!" );
        return;
    }

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
            sal_uInt32 nValidControls = OnSearchContextRequest_Lock(aTestContext);
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
    {
        // there are no controls that meet all the conditions for a search
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                 SvxResId(RID_STR_NODATACONTROLS)));
        xBox->run();
        return;
    }

    // now I need another 'initial context'
    sal_Int16 nInitialContext = 0;
    Reference<XForm> xActiveForm(getActiveForm_Lock());
    for ( size_t i=0; i<m_aSearchForms.size(); ++i )
    {
        if (m_aSearchForms.at(i) == xActiveForm)
        {
            nInitialContext = static_cast<sal_Int16>(i);
            break;
        }
    }

    // If the dialog should initially offer the text of the active control,
    // this must have an XTextComponent interface. An addition, this makes
    // sense only if the current field is also bound to a table (or whatever) field.
    OUString strActiveField;
    OUString strInitialText;
    // ... this I get from my FormController
    DBG_ASSERT(m_xActiveController.is(), "FmXFormShell::ExecuteSearch : no active controller !");
    Reference< XControl> xActiveControl( m_xActiveController->getCurrentControl());
    if (xActiveControl.is())
    {
        // the control can tell me its model ...
        Reference< XControlModel> xActiveModel( xActiveControl->getModel());
        DBG_ASSERT(xActiveModel.is(), "FmXFormShell::ExecuteSearch : active control has no model !");

        // I ask the model for the ControlSource property ...
        Reference< XPropertySet> xProperties(xActiveControl->getModel(), UNO_QUERY);
        if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xProperties) && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xProperties))
        {
            Reference< XPropertySet> xField;
            xProperties->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;
            if (xField.is())    // (only when the thing is really bound)
            {
                // and the control itself for a TextComponent interface (so that I can pick up the text there)
                Reference< XTextComponent> xText(xActiveControl, UNO_QUERY);
                if (xText.is())
                {
                    strActiveField = getLabelName(xProperties);
                    strInitialText = xText->getText();
                }
            }
        }
        else
        {
            // the control itself has no ControlSource, but maybe it is a GridControl
            Reference< XGrid> xGrid(xActiveControl, UNO_QUERY);
            if (xGrid.is())
            {
                // for strActiveField I need the ControlSource of the column,
                // for that the columns container, for that the GridPeer
                Reference< XGridPeer> xGridPeer(xActiveControl->getPeer(), UNO_QUERY);
                Reference< XIndexAccess> xColumns;
                if (xGridPeer.is())
                    xColumns.set(xGridPeer->getColumns(),UNO_QUERY);

                sal_Int16 nViewCol = xGrid->getCurrentColumnPosition();
                sal_Int32 nModelCol = GridView2ModelPos(xColumns, nViewCol);
                Reference< XPropertySet> xCurrentCol;
                if(xColumns.is())
                    xColumns->getByIndex(nModelCol) >>= xCurrentCol;
                if (xCurrentCol.is())
                    strActiveField = ::comphelper::getString(xCurrentCol->getPropertyValue(FM_PROP_LABEL));

                // the text of the current column
                Reference< XIndexAccess> xColControls(xGridPeer, UNO_QUERY);
                Reference< XInterface> xCurControl;
                xColControls->getByIndex(nViewCol) >>= xCurControl;
                OUString sInitialText;
                if (IsSearchableControl(xCurControl, &sInitialText))
                    strInitialText = sInitialText;
            }
        }
    }

    // taking care of possible GridControls that I know
    LoopGrids_Lock(LoopGridsSync::DISABLE_SYNC);

    // Now I am ready for the dialogue.
    // When the potential deadlocks caused by the use of the solar mutex in
    // MTs VCLX... classes are eventually cleared, an SM_USETHREAD should be
    // placed here, because the search in a separate thread is nevertheless
    // somewhat more fluid. Should be, however, somehow made dependent of the
    // underlying cursor. DAO for example is not thread-safe.
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractFmSearchDialog> pDialog(
            pFact->CreateFmSearchDialog(
                m_pShell->GetViewShell()->GetViewFrame()->GetWindow().GetFrameWeld(),
                strInitialText, aContextNames, nInitialContext,
                LINK(this, FmXFormShell, OnSearchContextRequest_Lock) ));
    pDialog->SetActiveField( strActiveField );
    pDialog->SetFoundHandler(LINK(this, FmXFormShell, OnFoundData_Lock));
    pDialog->SetCanceledNotFoundHdl(LINK(this, FmXFormShell, OnCanceledNotFound_Lock));
    pDialog->Execute();
    pDialog.disposeAndClear();

    // restore GridControls again
    LoopGrids_Lock(LoopGridsSync::ENABLE_SYNC, LoopGridsFlags::DISABLE_ROCTRLR);

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetSdrPageView());
        // because I marked controls in OnFoundData (if I was there)
}


bool FmXFormShell::GetY2KState_Lock(sal_uInt16& n)
{
    if (impl_checkDisposed_Lock())
        return false;

    if (m_pShell->IsDesignMode())
        // in the design mode (without active controls) the main document is to take care of it
        return false;

    Reference<XForm> xForm(getActiveForm_Lock());
    if (!xForm.is())
        // no current form (in particular no current control) -> the main document is to take care
        return false;

    Reference< XRowSet> xDB(xForm, UNO_QUERY);
    DBG_ASSERT(xDB.is(), "FmXFormShell::GetY2KState : current form has no dbform-interface !");

    Reference< XNumberFormatsSupplier> xSupplier( getNumberFormats(getConnection(xDB)));
    if (xSupplier.is())
    {
        Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
        if (xSet.is())
        {
            try
            {
                Any aVal( xSet->getPropertyValue("TwoDigitDateStart") );
                aVal >>= n;
                return true;
            }
            catch(Exception&)
            {
            }

        }
    }
    return false;
}


void FmXFormShell::SetY2KState_Lock(sal_uInt16 n)
{
    if (impl_checkDisposed_Lock())
        return;

    Reference<XForm> xActiveForm(getActiveForm_Lock());
    Reference< XRowSet > xActiveRowSet( xActiveForm, UNO_QUERY );
    if ( xActiveRowSet.is() )
    {
        Reference< XNumberFormatsSupplier > xSupplier( getNumberFormats( getConnection( xActiveRowSet ) ) );
        if (xSupplier.is())
        {
            Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    xSet->setPropertyValue("TwoDigitDateStart", makeAny<sal_uInt16>(n));
                }
                catch(Exception&)
                {
                    OSL_FAIL("FmXFormShell::SetY2KState: Exception occurred!");
                }

            }
            return;
        }
    }

    // no active form found -> iterate through all current forms
    Reference< XIndexAccess> xCurrentForms( m_xForms);
    if (!xCurrentForms.is())
    {   // in the alive mode, my forms are not set, but the ones on the page are
        if (m_pShell->GetCurPage())
            xCurrentForms.set( m_pShell->GetCurPage()->GetForms( false ), UNO_QUERY );
    }
    if (!xCurrentForms.is())
        return;

    ::comphelper::IndexAccessIterator aIter(xCurrentForms);
    Reference< XInterface> xCurrentElement( aIter.Next());
    while (xCurrentElement.is())
    {
        // is the current element a DatabaseForm?
        Reference< XRowSet> xElementAsRowSet( xCurrentElement, UNO_QUERY );
        if ( xElementAsRowSet.is() )
        {
            Reference< XNumberFormatsSupplier > xSupplier( getNumberFormats( getConnection( xElementAsRowSet ) ) );
            if (!xSupplier.is())
                continue;

            Reference< XPropertySet> xSet(xSupplier->getNumberFormatSettings());
            if (xSet.is())
            {
                try
                {
                    xSet->setPropertyValue("TwoDigitDateStart", makeAny<sal_uInt16>(n));
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


void FmXFormShell::CloseExternalFormViewer_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    if (!m_xExternalViewController.is())
        return;

    Reference< css::frame::XFrame> xExternalViewFrame( m_xExternalViewController->getFrame());
    Reference< css::frame::XDispatchProvider> xCommLink(xExternalViewFrame, UNO_QUERY);
    if (!xCommLink.is())
        return;

    xExternalViewFrame->setComponent(nullptr,nullptr);
    ::comphelper::disposeComponent(xExternalViewFrame);
    m_xExternalViewController   = nullptr;
    m_xExtViewTriggerController = nullptr;
    m_xExternalDisplayedForm    = nullptr;
}


Reference<XResultSet> FmXFormShell::getInternalForm_Lock(const Reference<XResultSet>& _xForm) const
{
    if (impl_checkDisposed_Lock())
        return nullptr;

    Reference< runtime::XFormController> xExternalCtrlr(m_xExternalViewController, UNO_QUERY);
    if (xExternalCtrlr.is() && (_xForm == xExternalCtrlr->getModel()))
    {
        DBG_ASSERT(m_xExternalDisplayedForm.is(), "FmXFormShell::getInternalForm : invalid external form !");
        return m_xExternalDisplayedForm;
    }
    return _xForm;
}


Reference<XForm> FmXFormShell::getInternalForm_Lock(const Reference<XForm>& _xForm) const
{
    if (impl_checkDisposed_Lock())
        return nullptr;

    Reference< runtime::XFormController > xExternalCtrlr(m_xExternalViewController, UNO_QUERY);
    if (xExternalCtrlr.is() && (_xForm == xExternalCtrlr->getModel()))
    {
        DBG_ASSERT(m_xExternalDisplayedForm.is(), "FmXFormShell::getInternalForm : invalid external form !");
        return Reference< XForm>(m_xExternalDisplayedForm, UNO_QUERY);
    }
    return _xForm;
}


namespace
{
    bool lcl_isNavigationRelevant( sal_Int32 _nWhich )
    {
        return  ( _nWhich == SID_FM_RECORD_FIRST )
            ||  ( _nWhich == SID_FM_RECORD_PREV )
            ||  ( _nWhich == SID_FM_RECORD_NEXT )
            ||  ( _nWhich == SID_FM_RECORD_LAST )
            ||  ( _nWhich == SID_FM_RECORD_NEW );
    }
}


bool FmXFormShell::IsFormSlotEnabled( sal_Int32 _nSlot, FeatureState* _pCompleteState )
{
    const svx::ControllerFeatures& rController =
            lcl_isNavigationRelevant( _nSlot )
        ?   getNavControllerFeatures_Lock()
        :   getActiveControllerFeatures_Lock();

    if ( !_pCompleteState )
        return rController->isEnabled( _nSlot );

    rController->getState( _nSlot, *_pCompleteState );
    return _pCompleteState->Enabled;
}


void FmXFormShell::ExecuteFormSlot_Lock( sal_Int32 _nSlot )
{
    const svx::ControllerFeatures& rController =
            lcl_isNavigationRelevant( _nSlot )
        ?   getNavControllerFeatures_Lock()
        :   getActiveControllerFeatures_Lock();

    rController->execute( _nSlot );

    if ( _nSlot == SID_FM_RECORD_UNDO )
    {
        // if we're doing an UNDO, *and* if the affected form is the form which we also display
        // as external view, then we need to reset the controls of the external form, too
        if (getInternalForm_Lock(getActiveForm_Lock()) == m_xExternalDisplayedForm)
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


void FmXFormShell::impl_switchActiveControllerListening_Lock(const bool _bListen)
{
    Reference< XComponent> xComp( m_xActiveController, UNO_QUERY );
    if ( !xComp.is() )
        return;

    if ( _bListen )
        xComp->addEventListener( static_cast<XFormControllerListener*>(this) );
    else
        xComp->removeEventListener( static_cast<XFormControllerListener*>(this) );
}


void FmXFormShell::setActiveController_Lock(const Reference<runtime::XFormController>& xController, bool _bNoSaveOldContent)
{
    if (impl_checkDisposed_Lock())
        return;

    if (m_bChangingDesignMode)
        return;
    DBG_ASSERT(!m_pShell->IsDesignMode(), "only to be used in alive mode");

    // if the routine has been called a second time,
    // the focus should no longer be transferred
    if (m_bInActivate)
    {
        m_bSetFocus = xController != m_xActiveController;
        return;
    }

    if (xController == m_xActiveController)
        return;

    // switch all nav dispatchers belonging to the form of the current nav controller to 'non active'
    Reference< XResultSet> xNavigationForm;
    if (m_xNavigationController.is())
        xNavigationForm.set(m_xNavigationController->getModel(), UNO_QUERY);

    m_bInActivate = true;

    // check if the 2 controllers serve different forms
    Reference< XResultSet> xOldForm;
    if (m_xActiveController.is())
        xOldForm.set(m_xActiveController->getModel(), UNO_QUERY);
    Reference< XResultSet> xNewForm;
    if (xController.is())
        xNewForm = Reference< XResultSet>(xController->getModel(), UNO_QUERY);
    xOldForm = getInternalForm_Lock(xOldForm);
    xNewForm = getInternalForm_Lock(xNewForm);

    bool bDifferentForm = ( xOldForm.get() != xNewForm.get() );
    bool bNeedSave = bDifferentForm && !_bNoSaveOldContent;
        // we save the content of the old form if we move to a new form, and saving old content is allowed

    if ( m_xActiveController.is() && bNeedSave )
    {
        // save content on change of the controller; a commit has already been executed
        if ( m_aActiveControllerFeatures->commitCurrentControl() )
        {
            m_bSetFocus = true;
            if ( m_aActiveControllerFeatures->isModifiedRow() )
            {
                bool bIsNew = m_aActiveControllerFeatures->isInsertionRow();
                bool bResult = m_aActiveControllerFeatures->commitCurrentRecord();
                if ( !bResult && m_bSetFocus )
                {
                    // if we couldn't save the current record, set the focus back to the
                    // current control
                    Reference< XWindow > xWindow( m_xActiveController->getCurrentControl(), UNO_QUERY );
                    if ( xWindow.is() )
                        xWindow->setFocus();
                    m_bInActivate = false;
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

    stopListening_Lock();

    impl_switchActiveControllerListening_Lock(false);

    m_aActiveControllerFeatures.dispose();
    m_xActiveController = xController;
    if ( m_xActiveController.is() )
        m_aActiveControllerFeatures.assign( m_xActiveController );

    impl_switchActiveControllerListening_Lock(true);

    if ( m_xActiveController.is() )
        m_xActiveForm = getInternalForm_Lock(Reference<XForm>(m_xActiveController->getModel(), UNO_QUERY));
    else
        m_xActiveForm = nullptr;

    startListening_Lock();

    // activate all dispatchers belonging to form of the new navigation controller
    xNavigationForm = nullptr;
    if (m_xNavigationController.is())
        xNavigationForm.set(m_xNavigationController->getModel(), UNO_QUERY);

    m_bInActivate = false;

    m_pShell->UIFeatureChanged();
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);

    InvalidateSlot_Lock(SID_FM_FILTER_NAVIGATOR_CONTROL, true);
}


void FmXFormShell::getCurrentSelection_Lock(InterfaceBag& /* [out] */ _rSelection) const
{
    _rSelection = m_aCurrentSelection;
}


bool FmXFormShell::setCurrentSelectionFromMark_Lock(const SdrMarkList& _rMarkList)
{
    m_aLastKnownMarkedControls.clear();

    if ( ( _rMarkList.GetMarkCount() > 0 ) && isControlList( _rMarkList ) )
        collectInterfacesFromMarkList( _rMarkList, m_aLastKnownMarkedControls );

    return setCurrentSelection_Lock(m_aLastKnownMarkedControls);
}


bool FmXFormShell::selectLastMarkedControls_Lock()
{
    return setCurrentSelection_Lock(m_aLastKnownMarkedControls);
}


bool FmXFormShell::setCurrentSelection_Lock( const InterfaceBag& _rSelection )
{
    if (impl_checkDisposed_Lock())
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
        Reference< XChild > xCur; if ( m_aCurrentSelection.size() == 1 ) xCur.set(*m_aCurrentSelection.begin(), css::uno::UNO_QUERY);
        Reference< XChild > xNew; if ( _rSelection.size() == 1 ) xNew.set(*_rSelection.begin(), css::uno::UNO_QUERY);

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

    // determine the form which all the selected objects belong to, if any
    Reference< XForm > xNewCurrentForm;
    for (const auto& rpSelection : m_aCurrentSelection)
    {
        Reference< XForm > xThisRoundsForm( GetForm( rpSelection ) );
        OSL_ENSURE( xThisRoundsForm.is(), "FmXFormShell::setCurrentSelection: *everything* should belong to a form!" );

        if ( !xNewCurrentForm.is() )
        {   // the first form we encountered
            xNewCurrentForm = xThisRoundsForm;
        }
        else if ( xNewCurrentForm != xThisRoundsForm )
        {   // different forms -> no "current form" at all
            xNewCurrentForm.clear();
            break;
        }
    }

    if ( !m_aCurrentSelection.empty() )
        impl_updateCurrentForm_Lock(xNewCurrentForm);

    // ensure some slots are updated
    for (sal_Int16 i : SelObjectSlotMap)
        InvalidateSlot_Lock(i, false);

    return true;
}


bool FmXFormShell::isSolelySelected_Lock(const Reference<XInterface>& _rxObject)
{
    return ( m_aCurrentSelection.size() == 1 ) && ( *m_aCurrentSelection.begin() == _rxObject );
}


void FmXFormShell::forgetCurrentForm_Lock()
{
    if ( !m_xCurrentForm.is() )
        return;

    // reset ...
    impl_updateCurrentForm_Lock(nullptr);

    // ... and try finding a new current form
    // #i88186# / 2008-04-12 / frank.schoenheit@sun.com
    impl_defaultCurrentForm_nothrow_Lock();
}


void FmXFormShell::impl_updateCurrentForm_Lock(const Reference<XForm>& _rxNewCurForm)
{
    if (impl_checkDisposed_Lock())
        return;

    m_xCurrentForm = _rxNewCurForm;

    // propagate to the FormPage(Impl)
    FmFormPage* pPage = m_pShell->GetCurPage();
    if ( pPage )
        pPage->GetImpl().setCurForm( m_xCurrentForm );

    // ensure the UI which depends on the current form is up-to-date
    for (sal_Int16 i : DlgSlotMap)
        InvalidateSlot_Lock(i, false);
}


void FmXFormShell::startListening_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    Reference< XRowSet> xDatabaseForm(m_xActiveForm, UNO_QUERY);
    if (xDatabaseForm.is() && getConnection(xDatabaseForm).is())
    {
        Reference< XPropertySet> xActiveFormSet(m_xActiveForm, UNO_QUERY);
        if (xActiveFormSet.is())
        {
            // if there is a data source, then build the listener
            // TODO: this is strange - shouldn't this depend on a isLoaded instead of
            // a "has command value"? Finally, the command value only means that it was
            // intended to be loaded, not that it actually *is* loaded
            OUString aSource = ::comphelper::getString(xActiveFormSet->getPropertyValue(FM_PROP_COMMAND));
            if (!aSource.isEmpty())
            {
                m_bDatabaseBar = true;

                xActiveFormSet->getPropertyValue(FM_PROP_NAVIGATION) >>= m_eNavigate;

                switch (m_eNavigate)
                {
                    case NavigationBarMode_PARENT:
                    {
                        // search for the controller via which navigation is possible
                        Reference< XChild> xChild(m_xActiveController, UNO_QUERY);
                        Reference< runtime::XFormController > xParent;
                        while (xChild.is())
                        {
                            xChild.set(xChild->getParent(), UNO_QUERY);
                            xParent.set(xChild, UNO_QUERY);
                            Reference< XPropertySet> xParentSet;
                            if (xParent.is())
                                xParentSet.set(xParent->getModel(), UNO_QUERY);
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
                        m_xNavigationController = nullptr;
                        m_bDatabaseBar = false;
                }

                m_aNavControllerFeatures.dispose();
                if ( m_xNavigationController.is() && ( m_xNavigationController != m_xActiveController ) )
                    m_aNavControllerFeatures.assign( m_xNavigationController );

                // because of RecordCount, listen at the controller which controls the navigation
                Reference< XPropertySet> xNavigationSet;
                if (m_xNavigationController.is())
                {
                    xNavigationSet.set(m_xNavigationController->getModel(), UNO_QUERY);
                    if (xNavigationSet.is())
                        xNavigationSet->addPropertyChangeListener(FM_PROP_ROWCOUNT,this);
                }
                return;
            }
        }
    }

    m_eNavigate  = NavigationBarMode_NONE;
    m_bDatabaseBar = false;
    m_xNavigationController = nullptr;
}


void FmXFormShell::stopListening_Lock()
{
    if (impl_checkDisposed_Lock())
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

    m_bDatabaseBar = false;
    m_eNavigate  = NavigationBarMode_NONE;
    m_xNavigationController = nullptr;
}


void FmXFormShell::ShowSelectionProperties_Lock(bool bShow)
{
    if (impl_checkDisposed_Lock())
        return;

    // if the window is already visible, only update the state
    bool bHasChild = m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_SHOW_PROPERTIES );
    if ( bHasChild && bShow )
        UpdateSlot_Lock(SID_FM_PROPERTY_CONTROL);

    // else toggle state
    else
        m_pShell->GetViewShell()->GetViewFrame()->ToggleChildWindow(SID_FM_SHOW_PROPERTIES);

    InvalidateSlot_Lock(SID_FM_PROPERTIES, false);
    InvalidateSlot_Lock(SID_FM_CTL_PROPERTIES, false);
}


IMPL_LINK(FmXFormShell, OnFoundData_Lock, FmFoundRecordInformation&, rfriWhere, void)
{
    if (impl_checkDisposed_Lock())
        return;

    DBG_ASSERT((rfriWhere.nContext >= 0) && (rfriWhere.nContext < static_cast<sal_Int16>(m_aSearchForms.size())),
        "FmXFormShell::OnFoundData : invalid context!");
    Reference< XForm> xForm( m_aSearchForms.at(rfriWhere.nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnFoundData : invalid form!");

    Reference< XRowLocate> xCursor(xForm, UNO_QUERY);
    if (!xCursor.is())
        return;       // what should I do there?

    // to the record
    try
    {
        xCursor->moveToBookmark(rfriWhere.aPosition);
    }
    catch(const SQLException&)
    {
        OSL_FAIL("Can position on bookmark!");
    }

    LoopGrids_Lock(LoopGridsSync::FORCE_SYNC);

    // and to the field (for that, I collected the XVclComponent interfaces before the start of the search)
    SAL_WARN_IF(static_cast<size_t>(rfriWhere.nFieldPos) >=
            m_arrSearchedControls.size(),
        "svx.form", "FmXFormShell::OnFoundData : invalid index!");
    SdrObject* pObject = m_arrSearchedControls.at(rfriWhere.nFieldPos);

    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetSdrPageView());
    m_pShell->GetFormView()->MarkObj(pObject, m_pShell->GetFormView()->GetSdrPageView());

    FmFormObj* pFormObject = FmFormObj::GetFormObject( pObject );
    Reference< XControlModel > xControlModel( pFormObject ? pFormObject->GetUnoControlModel() : Reference< XControlModel >() );
    DBG_ASSERT( xControlModel.is(), "FmXFormShell::OnFoundData: invalid control!" );
    if ( !xControlModel.is() )
        return;

    // disable the permanent cursor for the last grid we found a record
    if (m_xLastGridFound.is() && (m_xLastGridFound != xControlModel))
    {
        Reference< XPropertySet> xOldSet(m_xLastGridFound, UNO_QUERY);
        xOldSet->setPropertyValue(FM_PROP_ALWAYSSHOWCURSOR, makeAny( false ) );
        Reference< XPropertyState> xOldSetState(xOldSet, UNO_QUERY);
        if (xOldSetState.is())
            xOldSetState->setPropertyToDefault(FM_PROP_CURSORCOLOR);
        else
            xOldSet->setPropertyValue(FM_PROP_CURSORCOLOR, Any());
    }

    // if the field is in a GridControl, I have to additionally go into the corresponding column there
    sal_Int32 nGridColumn = m_arrRelativeGridColumn[rfriWhere.nFieldPos];
    if (nGridColumn != -1)
    {   // unfortunately, I have to first get the control again
        Reference<XControl> xControl(pFormObject ? impl_getControl_Lock(xControlModel, *pFormObject) : Reference<XControl>());
        Reference< XGrid> xGrid(xControl, UNO_QUERY);
        DBG_ASSERT(xGrid.is(), "FmXFormShell::OnFoundData : invalid control!");
        // if one of the asserts fires, I probably did something wrong on building of m_arrSearchedControls

        // enable a permanent cursor for the grid so we can see the found text
        Reference< XPropertySet> xModelSet(xControlModel, UNO_QUERY);
        DBG_ASSERT(xModelSet.is(), "FmXFormShell::OnFoundData : invalid control model (no property set) !");
        xModelSet->setPropertyValue( FM_PROP_ALWAYSSHOWCURSOR, makeAny( true ) );
        xModelSet->setPropertyValue( FM_PROP_CURSORCOLOR, makeAny( COL_LIGHTRED ) );
        m_xLastGridFound = xControlModel;

        if ( xGrid.is() )
            xGrid->setCurrentColumnPosition(static_cast<sal_Int16>(nGridColumn));
    }

    // As the cursor has been repositioned, I have (in positioned) invalidated
    // my form bar slots. But that does not take effect here unfortunately, as
    // generally the (modal) search dialog is of course at the top ... So, force ...
    sal_uInt16 nPos = 0;
    while (DatabaseSlotMap[nPos])
        m_pShell->GetViewShell()->GetViewFrame()->GetBindings().Update(DatabaseSlotMap[nPos++]);
        // unfortunately the update goes against the invalidate with only individual slots
}


IMPL_LINK(FmXFormShell, OnCanceledNotFound_Lock, FmFoundRecordInformation&, rfriWhere, void)
{
    if (impl_checkDisposed_Lock())
        return;

    DBG_ASSERT((rfriWhere.nContext >= 0) && (rfriWhere.nContext < static_cast<sal_Int16>(m_aSearchForms.size())),
        "FmXFormShell::OnCanceledNotFound : invalid context!");
    Reference< XForm> xForm( m_aSearchForms.at(rfriWhere.nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnCanceledNotFound : invalid form!");

    Reference< XRowLocate> xCursor(xForm, UNO_QUERY);
    if (!xCursor.is())
        return;       // what should I do there?

    // to the record
    try
    {
        xCursor->moveToBookmark(rfriWhere.aPosition);
    }
    catch(const SQLException&)
    {
        OSL_FAIL("Can position on bookmark!");
    }


    m_pShell->GetFormView()->UnMarkAll(m_pShell->GetFormView()->GetSdrPageView());
}


IMPL_LINK(FmXFormShell, OnSearchContextRequest_Lock, FmSearchContext&, rfmscContextInfo, sal_uInt32)
{
    if (impl_checkDisposed_Lock())
        return 0;

    DBG_ASSERT(rfmscContextInfo.nContext < static_cast<sal_Int16>(m_aSearchForms.size()), "FmXFormShell::OnSearchContextRequest : invalid parameter !");
    Reference< XForm> xForm( m_aSearchForms.at(rfmscContextInfo.nContext));
    DBG_ASSERT(xForm.is(), "FmXFormShell::OnSearchContextRequest : unexpected : invalid context !");

    Reference< XResultSet> xIter(xForm, UNO_QUERY);
    DBG_ASSERT(xIter.is(), "FmXFormShell::OnSearchContextRequest : unexpected : context has no iterator !");


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
    // time). But since OnFoundData is usually called more often than ExecuteSearch, I'll
    // do that here.

    Reference< XNameAccess> xValidFormFields;
    Reference< XColumnsSupplier> xSupplyCols(xIter, UNO_QUERY);
    DBG_ASSERT(xSupplyCols.is(), "FmXFormShell::OnSearchContextRequest : invalid cursor : no columns supplier !");
    if (xSupplyCols.is())
        xValidFormFields = xSupplyCols->getColumns();
    DBG_ASSERT(xValidFormFields.is(), "FmXFormShell::OnSearchContextRequest : form has no fields !");

    // current Page/Controller
    FmFormPage* pCurrentPage = m_pShell->GetCurPage();
    assert(pCurrentPage && "FmXFormShell::OnSearchContextRequest : no page !");
    // Search all SdrControls of this page...
    OUString sControlSource, aName;

    SdrObjListIter aPageIter( pCurrentPage );
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
                xControl = impl_getControl_Lock(xControlModel, *pFormObject);
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
                    for (sal_Int32 nViewPos=0; nViewPos<xPeerContainer->getCount(); ++nViewPos)
                    {
                        xPeerContainer->getByIndex(nViewPos) >>= xCurrentColumn;
                        if (!xCurrentColumn.is())
                            continue;

                        // can we use this column control for searching ?
                        if (!IsSearchableControl(xCurrentColumn))
                            continue;

                        sal_Int32 nModelPos = GridView2ModelPos(xModelColumns, nViewPos);
                        Reference< XPropertySet> xCurrentColModel;
                        xModelColumns->getByIndex(nModelPos) >>= xCurrentColModel;
                        aName = ::comphelper::getString(xCurrentColModel->getPropertyValue(FM_PROP_CONTROLSOURCE));
                        // the cursor has a field matching the control source ?
                        if (xValidFormFields->hasByName(aName))
                        {
                            strFieldList = strFieldList + aName + ";";

                            sFieldDisplayNames = sFieldDisplayNames +
                                    ::comphelper::getString(xCurrentColModel->getPropertyValue(FM_PROP_LABEL)) +
                                    ";";

                            rfmscContextInfo.arrFields.push_back(xCurrentColumn);

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
                        xControl = impl_getControl_Lock(xControlModel, *pFormObject);
                        DBG_ASSERT(xControl.is(), "FmXFormShell::OnSearchContextRequest : didn't ::std::find a control with requested model !");
                    }

                    if (IsSearchableControl(xControl))
                    {
                        // all tests passed -> take along in the list
                        strFieldList = strFieldList + sControlSource + ";";

                        // the label which should appear for the control :
                        sFieldDisplayNames = sFieldDisplayNames +
                                getLabelName(Reference< XPropertySet>(xControlModel, UNO_QUERY)) +
                                ";";

                        // mark the SdrObject (accelerates the treatment in OnFoundData)
                        m_arrSearchedControls.push_back(pCurrent);

                        // the number of the column (here a dummy, since it is only interesting for GridControls)
                        m_arrRelativeGridColumn.push_back(-1);

                        // and for the formatted search...
                        rfmscContextInfo.arrFields.emplace_back( xControl, UNO_QUERY );
                    }
                }
            }

            xSearchable = iter.Next();
        }
    }

    strFieldList = comphelper::string::stripEnd(strFieldList, ';');
    sFieldDisplayNames = comphelper::string::stripEnd(sFieldDisplayNames, ';');

    if (rfmscContextInfo.arrFields.empty())
    {
        rfmscContextInfo.arrFields.clear();
        rfmscContextInfo.xCursor = nullptr;
        rfmscContextInfo.strUsedFields.clear();
        return 0L;
    }

    rfmscContextInfo.xCursor = xIter;
    rfmscContextInfo.strUsedFields = strFieldList;
    rfmscContextInfo.sFieldDisplayNames = sFieldDisplayNames;

    // 66463 - 31.05.99 - FS
    // when the cursor is a non-STANDARD RecordMode, set it back
    Reference< XPropertySet> xCursorSet(rfmscContextInfo.xCursor, UNO_QUERY);
    Reference< XResultSetUpdate> xUpdateCursor(rfmscContextInfo.xCursor, UNO_QUERY);
    if (xUpdateCursor.is() && xCursorSet.is())
    {
        if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISNEW)))
            xUpdateCursor->moveToCurrentRow();
        else if (::comphelper::getBOOL(xCursorSet->getPropertyValue(FM_PROP_ISMODIFIED)))
            xUpdateCursor->cancelRowUpdates();
    }

    return rfmscContextInfo.arrFields.size();
}

  // XContainerListener

void SAL_CALL FmXFormShell::elementInserted(const ContainerEvent& evt)
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock())
        return;

    // new object to listen to
    Reference< XInterface> xTemp;
    evt.Element >>= xTemp;
    AddElement_Lock(xTemp);

    m_pShell->DetermineForms(true);
}


void SAL_CALL FmXFormShell::elementReplaced(const ContainerEvent& evt)
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock() )
        return;

    Reference< XInterface> xTemp;
    evt.ReplacedElement >>= xTemp;
    RemoveElement_Lock(xTemp);
    evt.Element >>= xTemp;
    AddElement_Lock(xTemp);
}


void SAL_CALL FmXFormShell::elementRemoved(const ContainerEvent& evt)
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock())
        return;

    Reference< XInterface> xTemp;
    evt.Element >>= xTemp;
    RemoveElement_Lock(xTemp);

    m_pShell->DetermineForms(true);
}


void FmXFormShell::UpdateForms_Lock(bool _bInvalidate)
{
    if (impl_checkDisposed_Lock())
        return;

    Reference< XIndexAccess > xForms;

    FmFormPage* pPage = m_pShell->GetCurPage();
    if ( pPage && m_pShell->m_bDesignMode )
        xForms.set(pPage->GetForms( false ), css::uno::UNO_QUERY);

    if ( m_xForms != xForms )
    {
        RemoveElement_Lock( m_xForms );
        m_xForms = xForms;
        AddElement_Lock(m_xForms);
    }

    SolarMutexGuard g;
    m_pShell->DetermineForms( _bInvalidate );
}


void FmXFormShell::AddElement_Lock(const Reference<XInterface>& _xElement)
{
    if (impl_checkDisposed_Lock())
        return;
    impl_AddElement_nothrow(_xElement);
}

void FmXFormShell::impl_AddElement_nothrow(const Reference< XInterface>& Element)
{
    // listen at the container
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

    const Reference< css::view::XSelectionSupplier> xSelSupplier(Element, UNO_QUERY);
    if (xSelSupplier.is())
        xSelSupplier->addSelectionChangeListener(this);
}


void FmXFormShell::RemoveElement_Lock(const Reference<XInterface>& Element)
{
    if (impl_checkDisposed_Lock())
        return;
    impl_RemoveElement_nothrow_Lock(Element);
}

void FmXFormShell::impl_RemoveElement_nothrow_Lock(const Reference<XInterface>& Element)
{
    const Reference< css::view::XSelectionSupplier> xSelSupplier(Element, UNO_QUERY);
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
            impl_RemoveElement_nothrow_Lock(xElement);
        }
    }

    InterfaceBag::iterator wasSelectedPos = m_aCurrentSelection.find( Element );
    if ( wasSelectedPos != m_aCurrentSelection.end() )
        m_aCurrentSelection.erase( wasSelectedPos );
}


void SAL_CALL FmXFormShell::selectionChanged(const lang::EventObject& rEvent)
{
    SolarMutexGuard g;

    if (impl_checkDisposed_Lock())
        return;

    Reference< XSelectionSupplier > xSupplier( rEvent.Source, UNO_QUERY );
    Reference< XInterface > xSelObj( xSupplier->getSelection(), UNO_QUERY );
    // a selection was removed, this can only be done by the shell
    if ( !xSelObj.is() )
        return;

    EnableTrackProperties_Lock(false);

    bool bMarkChanged = m_pShell->GetFormView()->checkUnMarkAll(rEvent.Source);
    Reference< XForm > xNewForm( GetForm( rEvent.Source ) );

    InterfaceBag aNewSelection;
    aNewSelection.insert( Reference<XInterface>( xSelObj, UNO_QUERY ) );

    if (setCurrentSelection_Lock(aNewSelection) && IsPropBrwOpen_Lock())
        ShowSelectionProperties_Lock(true);

    EnableTrackProperties_Lock(true);

    if ( bMarkChanged )
        m_pShell->NotifyMarkListChanged( m_pShell->GetFormView() );
}


IMPL_LINK_NOARG(FmXFormShell, OnTimeOut_Lock, Timer*, void)
{
    if (impl_checkDisposed_Lock())
        return;

    if (m_pShell->IsDesignMode() && m_pShell->GetFormView())
        SetSelection_Lock(m_pShell->GetFormView()->GetMarkedObjectList());
}


void FmXFormShell::SetSelectionDelayed_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    if (m_pShell->IsDesignMode() && IsTrackPropertiesEnabled_Lock() && !m_aMarkTimer.IsActive())
        m_aMarkTimer.Start();
}


void FmXFormShell::SetSelection_Lock(const SdrMarkList& rMarkList)
{
    if (impl_checkDisposed_Lock())
        return;

    DetermineSelection_Lock(rMarkList);
    m_pShell->NotifyMarkListChanged(m_pShell->GetFormView());
}


void FmXFormShell::DetermineSelection_Lock(const SdrMarkList& rMarkList)
{
    if (setCurrentSelectionFromMark_Lock(rMarkList) && IsPropBrwOpen_Lock())
        ShowSelectionProperties_Lock(true);
}


bool FmXFormShell::IsPropBrwOpen_Lock() const
{
    if (impl_checkDisposed_Lock())
        return false;

    return m_pShell->GetViewShell() && m_pShell->GetViewShell()->GetViewFrame()
        && m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow(SID_FM_SHOW_PROPERTIES);
}


class FmXFormShell::SuspendPropertyTracking
{
private:
    FmXFormShell&   m_rShell;
    bool            m_bEnabled;

public:
    explicit SuspendPropertyTracking( FmXFormShell& _rShell )
        :m_rShell( _rShell )
        ,m_bEnabled( false )
    {
        if (m_rShell.IsTrackPropertiesEnabled_Lock())
        {
            m_rShell.EnableTrackProperties_Lock(false);
            m_bEnabled = true;
        }
    }

    ~SuspendPropertyTracking( )
    {
        if ( m_bEnabled )   // note that ( false != m_bEnabled ) implies ( NULL != m_pShell )
            m_rShell.EnableTrackProperties_Lock(true);
    }
};


void FmXFormShell::SetDesignMode_Lock(bool bDesign)
{
    if (impl_checkDisposed_Lock())
        return;

    DBG_ASSERT(m_pShell->GetFormView(), "FmXFormShell::SetDesignMode : invalid call (have no shell or no view) !");
    m_bChangingDesignMode = true;

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
            stopFiltering_Lock(false);

        // unsubscribe from the objects of my MarkList
        pFormView->GetImpl()->stopMarkListWatching();
    }
    else
    {
        m_aMarkTimer.Stop();

        SuspendPropertyTracking aSuspend( *this );
        pFormView->GetImpl()->saveMarkList();
    }

    if (bDesign && m_xExternalViewController.is())
        CloseExternalFormViewer_Lock();

    pFormView->ChangeDesignMode(bDesign);

    // notify listeners
    FmDesignModeChangedHint aChangedHint( bDesign );
    m_pShell->Broadcast(aChangedHint);

    m_pShell->m_bDesignMode = bDesign;
    UpdateForms_Lock(false);

    m_pTextShell->designModeChanged();

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
            SetSelection_Lock(aList);
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
        // The UIFeatureChanged performs an update (a check of the available features) asynchronously.
        // So we can't call ShowSelectionProperties directly as the according feature isn't enabled yet.
        // That's why we use an asynchron execution on the dispatcher.
        // (And that's why this has to be done AFTER the UIFeatureChanged.)
        m_pShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SfxCallMode::ASYNCHRON );
    }
    m_bChangingDesignMode = false;
}


Reference< XControl> FmXFormShell::impl_getControl_Lock(const Reference<XControlModel>& i_rxModel, const FmFormObj& i_rKnownFormObj)
{
    if (impl_checkDisposed_Lock())
        return nullptr;

    Reference< XControl > xControl;
    try
    {
        Reference< XControlContainer> xControlContainer(getControlContainerForView_Lock(), UNO_SET_THROW);

        Sequence< Reference< XControl > > seqControls( xControlContainer->getControls() );
        // ... that I can then search
        for (Reference< XControl > const & control : seqControls)
        {
            xControl.set( control, UNO_SET_THROW );
            Reference< XControlModel > xCurrentModel( xControl->getModel() );
            if ( xCurrentModel == i_rxModel )
                break;
            xControl.clear();
        }

        if ( !xControl.is() )
        {
            // fallback (some controls might not have been created, yet, since they were never visible so far)
            Reference< XControl > xContainerControl( xControlContainer, UNO_QUERY_THROW );
            const vcl::Window* pContainerWindow = VCLUnoHelper::GetWindow( xContainerControl->getPeer() );
            ENSURE_OR_THROW( pContainerWindow, "unexpected control container implementation" );

            const SdrView* pSdrView = m_pShell ? m_pShell->GetFormView() : nullptr;
            ENSURE_OR_THROW( pSdrView, "no current view" );

            xControl.set( i_rKnownFormObj.GetUnoControl( *pSdrView, *pContainerWindow ), UNO_QUERY_THROW );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    OSL_ENSURE( xControl.is(), "FmXFormShell::impl_getControl: no control found!" );
    return xControl;
}

// note: _out_rForms is a member so needs lock
void FmXFormShell::impl_collectFormSearchContexts_nothrow_Lock( const Reference<XInterface>& _rxStartingPoint,
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
                sCompleteCurrentName.append( " (" );
                sCompleteCurrentName.append     ( _rCurrentLevelPrefix );
                sCompleteCurrentName.append( ")" );
            }

            // the prefix for the next level
            aNextLevelPrefix = _rCurrentLevelPrefix;
            if ( !_rCurrentLevelPrefix.isEmpty() )
                aNextLevelPrefix.append( '/' );
            aNextLevelPrefix.append( sCurrentFormName );

            // remember both the form and it's "display name"
            _out_rForms.push_back( xCurrentAsForm );
            _out_rNames.push_back( sCompleteCurrentName.makeStringAndClear() );

            // and descend
            impl_collectFormSearchContexts_nothrow_Lock(
                xCurrentAsForm, aNextLevelPrefix.makeStringAndClear(),
                _out_rForms, _out_rNames);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


void FmXFormShell::startFiltering_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    // setting all forms in filter mode
    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    Reference< XControlContainer> xContainer;
    if (getActiveController_Lock() == m_xExternalViewController)
    {
        DBG_ASSERT(m_xExtViewTriggerController.is(), "FmXFormShell::startFiltering : inconsistent : active external controller, but no one triggered this !");
        xContainer = m_xExtViewTriggerController->getContainer();
    }
    else
        xContainer = getActiveController_Lock()->getContainer();

    PFormViewPageWindowAdapter pAdapter = pXView->findWindow( xContainer );
    if ( pAdapter.is() )
    {
        const ::std::vector< Reference< runtime::XFormController> >& rControllerList = pAdapter->GetList();
        for (const auto& rpController : rControllerList)
        {
            Reference< XModeSelector> xModeSelector(rpController, UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode( "FilterMode" );
        }
    }

    m_bFilterMode = true;

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


static void saveFilter(const Reference< runtime::XFormController >& _rxController)
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
        xFormAsSet->setPropertyValue(FM_PROP_APPLYFILTER, makeAny( true ) );
    }
    catch (const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

}


void FmXFormShell::stopFiltering_Lock(bool bSave)
{
    if (impl_checkDisposed_Lock())
        return;

    m_bFilterMode = false;

    FmXFormView* pXView = m_pShell->GetFormView()->GetImpl();

    // if the active controller is our external one we have to use the trigger controller
    Reference< XControlContainer> xContainer;
    if (getActiveController_Lock() == m_xExternalViewController)
    {
        DBG_ASSERT(m_xExtViewTriggerController.is(), "FmXFormShell::stopFiltering : inconsistent : active external controller, but no one triggered this !");
        xContainer = m_xExtViewTriggerController->getContainer();
    }
    else
        xContainer = getActiveController_Lock()->getContainer();

    PFormViewPageWindowAdapter pAdapter = pXView->findWindow(xContainer);
    if ( pAdapter.is() )
    {
        const ::std::vector< Reference< runtime::XFormController > >& rControllerList = pAdapter->GetList();
        ::std::vector < OUString >   aOriginalFilters;
        ::std::vector < bool >       aOriginalApplyFlags;

        if (bSave)
        {
            for (const auto& rpController : rControllerList)
            {
                // remember the current filter settings in case we're going to reload the forms below (which may fail)
                try
                {
                    Reference< XPropertySet > xFormAsSet(rpController->getModel(), UNO_QUERY);
                    aOriginalFilters.push_back(::comphelper::getString(xFormAsSet->getPropertyValue(FM_PROP_FILTER)));
                    aOriginalApplyFlags.push_back(::comphelper::getBOOL(xFormAsSet->getPropertyValue(FM_PROP_APPLYFILTER)));
                }
                catch(Exception&)
                {
                    OSL_FAIL("FmXFormShell::stopFiltering : could not get the original filter !");
                    // put dummies into the arrays so the they have the right size

                    if (aOriginalFilters.size() == aOriginalApplyFlags.size())
                        // the first getPropertyValue failed -> use two dummies
                        aOriginalFilters.emplace_back( );
                    aOriginalApplyFlags.push_back( false );
                }
                saveFilter(rpController);
            }
        }
        for (const auto& rController : rControllerList)
        {

            Reference< XModeSelector> xModeSelector(rController, UNO_QUERY);
            if (xModeSelector.is())
                xModeSelector->setMode( "DataMode" );
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
                    bool bOriginalApplyFlag = aOriginalApplyFlags[ j - rControllers.begin() ];
                    try
                    {
                        xFormSet->setPropertyValue(FM_PROP_FILTER, makeAny(sOriginalFilter));
                        xFormSet->setPropertyValue(FM_PROP_APPLYFILTER, makeAny(bOriginalApplyFlag));
                        xReload->reload();
                    }
                    catch(const Exception&)
                    {
                        DBG_UNHANDLED_EXCEPTION("svx");
                    }
                }
            }
        }
    }

    m_pShell->UIFeatureChanged();
    m_pShell->GetViewShell()->GetViewFrame()->GetBindings().InvalidateShell(*m_pShell);
}


void FmXFormShell::CreateExternalView_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    DBG_ASSERT(m_xAttachedFrame.is(), "FmXFormShell::CreateExternalView : no frame !");

    // the frame the external view is displayed in
    bool bAlreadyExistent = m_xExternalViewController.is();
    Reference< css::frame::XFrame> xExternalViewFrame;
    OUString sFrameName("_beamer");

    Reference<runtime::XFormController> xCurrentNavController(getNavController_Lock());
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
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(nullptr,
                                                                     VclMessageType::Warning, VclButtonsType::Ok,
                                                                     SvxResId(RID_STR_NOCONTROLS_FOR_EXTERNALDISPLAY)));
            xBox->run();
            return;
        }
    }

    // load the component for external form views
    if (!bAlreadyExistent)
    {
        URL aWantToDispatch;
        aWantToDispatch.Complete = FMURL_COMPONENT_FORMGRIDVIEW;

        Reference< css::frame::XDispatchProvider> xProv(m_xAttachedFrame, UNO_QUERY);
        Reference< css::frame::XDispatch> xDisp;
        if (xProv.is())
            xDisp = xProv->queryDispatch(aWantToDispatch, sFrameName,
                                         css::frame::FrameSearchFlag::CHILDREN | css::frame::FrameSearchFlag::CREATE);
        if (xDisp.is())
        {
            xDisp->dispatch(aWantToDispatch, Sequence< PropertyValue>());
        }

        // with this the component should be loaded, now search the frame where it resides in
        xExternalViewFrame = m_xAttachedFrame->findFrame(sFrameName, css::frame::FrameSearchFlag::CHILDREN);
        if (xExternalViewFrame.is())
        {
            m_xExternalViewController = xExternalViewFrame->getController();
            Reference< css::lang::XComponent> xComp(m_xExternalViewController, UNO_QUERY);
            if (xComp.is())
                xComp->addEventListener(static_cast<XEventListener*>(static_cast<XPropertyChangeListener*>(this)));
        }
    }
    else
    {
        xExternalViewFrame = m_xExternalViewController->getFrame();
        Reference< css::frame::XDispatchProvider> xCommLink(xExternalViewFrame, UNO_QUERY);

        // if we display the active form we interpret the slot as "remove it"
        Reference< XForm> xCurrentModel(xCurrentNavController->getModel(), UNO_QUERY);
        if ((xCurrentModel == m_xExternalDisplayedForm) || (getInternalForm_Lock(xCurrentModel) == m_xExternalDisplayedForm))
        {
            if (m_xExternalViewController == getActiveController_Lock())
            {
                Reference< runtime::XFormController > xAsFormController( m_xExternalViewController, UNO_QUERY );
                ControllerFeatures aHelper( xAsFormController );
                (void)aHelper->commitCurrentControl();
            }

            Reference< runtime::XFormController > xNewController(m_xExtViewTriggerController);
            CloseExternalFormViewer_Lock();
            setActiveController_Lock(xNewController);
            return;
        }

        URL aClearURL;
        aClearURL.Complete = FMURL_GRIDVIEW_CLEARVIEW;

        Reference< css::frame::XDispatch> xClear( xCommLink->queryDispatch(aClearURL, OUString(), 0));
        if (xClear.is())
            xClear->dispatch(aClearURL, Sequence< PropertyValue>());
    }

    // TODO: We need an interceptor at the xSupplier, which forwards all queryDispatch requests to the FormController
    // instance for which this "external view" was triggered

    // get the dispatch interface of the frame so we can communicate (interceptable) with the controller
    Reference< css::frame::XDispatchProvider> xCommLink(xExternalViewFrame, UNO_QUERY);

    if (m_xExternalViewController.is())
    {
        DBG_ASSERT(xCommLink.is(), "FmXFormShell::CreateExternalView : the component doesn't have the necessary interfaces !");
        // collect the dispatchers we will need
        URL aAddColumnURL;
        aAddColumnURL.Complete = FMURL_GRIDVIEW_ADDCOLUMN;
        Reference< css::frame::XDispatch> xAddColumnDispatch( xCommLink->queryDispatch(aAddColumnURL, OUString(), 0));
        URL aAttachURL;
        aAttachURL.Complete = FMURL_GRIDVIEW_ATTACHTOFORM;
        Reference< css::frame::XDispatch> xAttachDispatch( xCommLink->queryDispatch(aAttachURL, OUString(), 0));

        if (xAddColumnDispatch.is() && xAttachDispatch.is())
        {
            DBG_ASSERT(xCurrentNavController.is(), "FmXFormShell::CreateExternalView : invalid call : have no nav controller !");
            // first : dispatch the descriptions for the columns to add
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
            OUString sColumnType,aGroupName,sControlSource;
            Sequence< Property> aProps;
            while ((xCurrentModelSet = Reference< XPropertySet>(aModelIterator.Next(), UNO_QUERY)).is())
            {
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
                            aRadioPositions[aGroupName] = nAddedColumns;

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
                            Reference< lang::XServiceInfo> xInfo(xCurrentModelSet, UNO_QUERY);
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
                    if (pProps->Name == FM_PROP_LABEL)
                        // already set
                        continue;
                    if (pProps->Name == FM_PROP_DEFAULTCONTROL)
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
                pDispatchArgs->Name = "ColumnProperties"; // TODO : fmurl.*
                pDispatchArgs->Value <<= aColumnProps;
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
            for (const auto& rCtrlSource : aRadioControlSources)
            {
                PropertyValue* pListBoxDescription = aListBoxDescription.getArray();
                // label
                pListBoxDescription->Name = FM_PROP_LABEL;
                pListBoxDescription->Value <<= rCtrlSource.first;
                ++pListBoxDescription;

                // control source
                pListBoxDescription->Name = FM_PROP_CONTROLSOURCE;
                pListBoxDescription->Value <<= rCtrlSource.second;
                ++pListBoxDescription;

                // bound column
                pListBoxDescription->Name = FM_PROP_BOUNDCOLUMN;
                pListBoxDescription->Value <<= sal_Int16(1);
                ++pListBoxDescription;

                // content type
                pListBoxDescription->Name = FM_PROP_LISTSOURCETYPE;
                pListBoxDescription->Value <<= ListSourceType_VALUELIST;
                ++pListBoxDescription;

                // list source
                MapUString2UstringSeq::const_iterator aCurrentListSource = aRadioListSources.find(rCtrlSource.first);
                DBG_ASSERT(aCurrentListSource != aRadioListSources.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                pListBoxDescription->Name = FM_PROP_LISTSOURCE;
                pListBoxDescription->Value <<= (*aCurrentListSource).second;
                ++pListBoxDescription;

                // value list
                MapUString2UstringSeq::const_iterator aCurrentValueList = aRadioValueLists.find(rCtrlSource.first);
                DBG_ASSERT(aCurrentValueList != aRadioValueLists.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                pListBoxDescription->Name = FM_PROP_STRINGITEMLIST;
                pListBoxDescription->Value <<= (*aCurrentValueList).second;
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
                FmMapUString2Int16::const_iterator aOffset = aRadioPositions.find(rCtrlSource.first);
                DBG_ASSERT(aOffset != aRadioPositions.end(),
                    "FmXFormShell::CreateExternalView : inconsistent radio descriptions !");
                sal_Int16 nPosition = (*aOffset).second;
                nPosition = nPosition + nOffset;
                    // we already inserted nOffset additional columns ....
                pDispatchArgs->Value <<= nPosition;
                ++pDispatchArgs;

                // the
                pDispatchArgs->Name = "ColumnProperties"; // TODO : fmurl.*
                pDispatchArgs->Value <<= aListBoxDescription;
                ++pDispatchArgs;
                DBG_ASSERT(nDispatchArgs == (pDispatchArgs - aDispatchArgs.getConstArray()),
                    "FmXFormShell::CreateExternalView : forgot to adjust nDispatchArgs ?");

                // dispatch the "add column"
                xAddColumnDispatch->dispatch(aAddColumnURL, aDispatchArgs);
                ++nAddedColumns;
                ++nOffset;
            }


            DBG_ASSERT(nAddedColumns > 0, "FmXFormShell::CreateExternalView : no controls (inconsistent) !");
                // we should have checked if we have any usable controls (see above).

            // "load" the "form" of the external view
            PropertyValue aArg;
            aArg.Name = FMARG_ATTACHTO_MASTERFORM;
            Reference< XResultSet> xForm(xCurrentNavController->getModel(), UNO_QUERY);
            aArg.Value <<= xForm;

            m_xExternalDisplayedForm = xForm;
                // do this before dispatching the "attach" command, as the attach may result in a call to our queryDispatch (for the FormSlots)
                // which needs the m_xExternalDisplayedForm

            xAttachDispatch->dispatch(aAttachURL, Sequence< PropertyValue>(&aArg, 1));

            m_xExtViewTriggerController = xCurrentNavController;

            // we want to know modifications done in the external view
            // if the external controller is a XFormController we can use all our default handlings for it
            Reference< runtime::XFormController > xFormController( m_xExternalViewController, UNO_QUERY );
            OSL_ENSURE( xFormController.is(), "FmXFormShell::CreateExternalView:: invalid external view controller!" );
            if (xFormController.is())
                xFormController->addActivateListener(static_cast<XFormControllerListener*>(this));
        }
    }
#ifdef DBG_UTIL
    else
    {
        OSL_FAIL("FmXFormShell::CreateExternalView : could not create the external form view !");
    }
#endif
    InvalidateSlot_Lock(SID_FM_VIEW_AS_GRID, false);
}


void FmXFormShell::implAdjustConfigCache_Lock()
{
    // get (cache) the wizard usage flag
    Sequence< OUString > aNames { "FormControlPilotsEnabled" };
    Sequence< Any > aFlags = GetProperties(aNames);
    if (1 == aFlags.getLength())
        m_bUseWizards = ::cppu::any2bool(aFlags[0]);
}


void FmXFormShell::Notify( const css::uno::Sequence< OUString >& _rPropertyNames)
{
    DBG_TESTSOLARMUTEX();
    if (impl_checkDisposed_Lock())
        return;

    const OUString* pSearch = _rPropertyNames.getConstArray();
    const OUString* pSearchTil = pSearch + _rPropertyNames.getLength();
    for (;pSearch < pSearchTil; ++pSearch)
        if (*pSearch == "FormControlPilotsEnabled")
        {
            implAdjustConfigCache_Lock();
            InvalidateSlot_Lock(SID_FM_USE_WIZARDS, true);
        }
}

void FmXFormShell::ImplCommit()
{
}


void FmXFormShell::SetWizardUsing_Lock(bool _bUseThem)
{
    m_bUseWizards = _bUseThem;

    Sequence< OUString > aNames { "FormControlPilotsEnabled" };
    Sequence< Any > aValues(1);
    aValues[0] <<= m_bUseWizards;
    PutProperties(aNames, aValues);
}


void FmXFormShell::viewDeactivated_Lock(FmFormView& _rCurrentView, bool _bDeactivateController)
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
        pPage->GetImpl().SetFormsCreationHdl( Link<FmFormPageImpl&,void>() );
    }
    UpdateForms_Lock(true);
}


IMPL_LINK_NOARG( FmXFormShell, OnFirstTimeActivation_Lock, void*, void )
{
    if (impl_checkDisposed_Lock())
        return;

    m_nActivationEvent = nullptr;
    SfxObjectShell* pDocument = m_pShell->GetObjectShell();

    if  ( pDocument && !pDocument->HasName() )
    {
        if (isEnhancedForm_Lock())
        {
            // show the data navigator
            if ( !m_pShell->GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_SHOW_DATANAVIGATOR ) )
                m_pShell->GetViewShell()->GetViewFrame()->ToggleChildWindow( SID_FM_SHOW_DATANAVIGATOR );
        }
    }
}


IMPL_LINK_NOARG( FmXFormShell, OnFormsCreated_Lock, FmFormPageImpl&, void )
{
    UpdateForms_Lock(true);
}


void FmXFormShell::viewActivated_Lock(FmFormView& _rCurrentView, bool _bSyncAction)
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
                loadForms_Lock(pPage, LoadFormsFlags::Load
                                      | (_bSyncAction ? LoadFormsFlags::Sync
                                                      : LoadFormsFlags::Async));
            pPage->GetImpl().setHasBeenActivated( );
        }

        // first-time initializations for the views
        if ( !_rCurrentView.GetImpl()->hasEverBeenActivated( ) )
        {
            _rCurrentView.GetImpl()->onFirstViewActivation( dynamic_cast<FmFormModel*>( _rCurrentView.GetModel() )  );
            _rCurrentView.GetImpl()->setHasBeenActivated( );
        }

        // activate the current view
        _rCurrentView.GetImpl()->Activate( _bSyncAction );
    }

    // set callbacks at the page
    if ( pPage )
    {
        pPage->GetImpl().SetFormsCreationHdl(LINK(this, FmXFormShell, OnFormsCreated_Lock));
    }

    UpdateForms_Lock(true);

    if ( m_bFirstActivation )
    {
        m_nActivationEvent = Application::PostUserEvent(LINK(this, FmXFormShell, OnFirstTimeActivation_Lock));
        m_bFirstActivation = false;
    }

    // find a default "current form", if there is none, yet
    // #i88186# / 2008-04-12 / frank.schoenheit@sun.com
    impl_defaultCurrentForm_nothrow_Lock();
}


void FmXFormShell::impl_defaultCurrentForm_nothrow_Lock()
{
    if (impl_checkDisposed_Lock())
        return;

    if ( m_xCurrentForm.is() )
        // no action required
        return;

    FmFormView* pFormView = m_pShell->GetFormView();
    FmFormPage* pPage = pFormView ? pFormView->GetCurPage() : nullptr;
    if ( !pPage )
        return;

    try
    {
        Reference< XIndexAccess > xForms( pPage->GetForms( false ), UNO_QUERY );
        if ( !xForms.is() || !xForms->hasElements() )
            return;

        Reference< XForm > xNewCurrentForm( xForms->getByIndex(0), UNO_QUERY_THROW );
        impl_updateCurrentForm_Lock(xNewCurrentForm);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


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


IMPL_LINK_NOARG( FmXFormShell, OnLoadForms_Lock, void*, void )
{
    FmLoadAction aAction = m_aLoadingPages.front();
    m_aLoadingPages.pop();

    loadForms_Lock(aAction.pPage, aAction.nFlags & ~LoadFormsFlags::Async);
}


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
            if ( isEmbeddedInDatabase( _rxLoadable.get(), xConn ) )
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
            DBG_UNHANDLED_EXCEPTION("svx");
        }
        return false;
    }
}


void FmXFormShell::loadForms_Lock(FmFormPage* _pPage, const LoadFormsFlags _nBehaviour /* LoadFormsFlags::Load | LoadFormsFlags::Sync */)
{
    DBG_ASSERT( ( _nBehaviour & ( LoadFormsFlags::Async | LoadFormsFlags::Unload ) )  != ( LoadFormsFlags::Async | LoadFormsFlags::Unload ),
        "FmXFormShell::loadForms: async loading not supported - this will heavily fail!" );

    if ( _nBehaviour & LoadFormsFlags::Async )
    {
        m_aLoadingPages.push( FmLoadAction(
            _pPage,
            _nBehaviour,
            Application::PostUserEvent(LINK(this, FmXFormShell, OnLoadForms_Lock), _pPage)
        ) );
        return;
    }

    DBG_ASSERT( _pPage, "FmXFormShell::loadForms: invalid page!" );
    if ( _pPage )
    {
        // lock the undo env so the forms can change non-transient properties while loading
        // (without this my doc's modified flag would be set)
        FmFormModel& rFmFormModel(dynamic_cast< FmFormModel& >(_pPage->getSdrModelFromSdrPage()));
        rFmFormModel.GetUndoEnv().Lock();

        // load all forms
        Reference< XIndexAccess >  xForms;
        xForms.set(_pPage->GetForms( false ), css::uno::UNO_QUERY);

        if ( xForms.is() )
        {
            Reference< XLoadable >  xForm;
            for ( sal_Int32 j = 0, nCount = xForms->getCount(); j < nCount; ++j )
            {
                xForms->getByIndex( j ) >>= xForm;
                bool bFormWasLoaded = false;
                // a database form must be loaded for
                try
                {
                    if ( !( _nBehaviour & LoadFormsFlags::Unload ) )
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
                    DBG_UNHANDLED_EXCEPTION("svx");
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

        // unlock the environment
        rFmFormModel.GetUndoEnv().UnLock();
    }
}


void FmXFormShell::ExecuteTextAttribute_Lock(SfxRequest& _rReq)
{
    DBG_TESTSOLARMUTEX();
    m_pTextShell->ExecuteTextAttribute( _rReq );
}


void FmXFormShell::GetTextAttributeState_Lock(SfxItemSet& _rSet)
{
    DBG_TESTSOLARMUTEX();
    m_pTextShell->GetTextAttributeState( _rSet );
}


bool FmXFormShell::IsActiveControl_Lock(bool _bCountRichTextOnly ) const
{
    DBG_TESTSOLARMUTEX();
    return m_pTextShell->IsActiveControl( _bCountRichTextOnly );
}


void FmXFormShell::ForgetActiveControl_Lock()
{
    DBG_TESTSOLARMUTEX();
    m_pTextShell->ForgetActiveControl();
}


void FmXFormShell::SetControlActivationHandler_Lock(const Link<LinkParamNone*,void>& _rHdl)
{
    DBG_TESTSOLARMUTEX();
    m_pTextShell->SetControlActivationHandler( _rHdl );
}

void FmXFormShell::handleShowPropertiesRequest_Lock()
{
    if (onlyControlsAreMarked_Lock())
        ShowSelectionProperties_Lock( true );
}


void FmXFormShell::handleMouseButtonDown_Lock(const SdrViewEvent& _rViewEvent)
{
    // catch simple double clicks
    if ( ( _rViewEvent.nMouseClicks == 2 ) && ( _rViewEvent.nMouseCode == MOUSE_LEFT ) )
    {
        if ( _rViewEvent.eHit == SdrHitKind::MarkedObject )
        {
            if (onlyControlsAreMarked_Lock())
                ShowSelectionProperties_Lock( true );
        }
    }
}


bool FmXFormShell::HasControlFocus_Lock() const
{
    bool bHasControlFocus = false;

    try
    {
        Reference<runtime::XFormController> xController(getActiveController_Lock());
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
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    return bHasControlFocus;
}


SearchableControlIterator::SearchableControlIterator(Reference< XInterface> const & xStartingPoint)
    :IndexAccessIterator(xStartingPoint)
{
}


bool SearchableControlIterator::ShouldHandleElement(const Reference< XInterface>& xElement)
{
    // if the thing has a ControlSource and a BoundField property
    Reference< XPropertySet> xProperties(xElement, UNO_QUERY);
    if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xProperties) && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xProperties))
    {
        // and the BoundField is valid
        Reference< XPropertySet> xField;
        xProperties->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;
        if (xField.is())
        {
            // we take it
            m_sCurrentValue = ::comphelper::getString(xProperties->getPropertyValue(FM_PROP_CONTROLSOURCE));
            return true;
        }
    }

    // if it is a grid control
    if (::comphelper::hasProperty(FM_PROP_CLASSID, xProperties))
    {
        Any aClassId( xProperties->getPropertyValue(FM_PROP_CLASSID) );
        if (::comphelper::getINT16(aClassId) == FormComponentType::GRIDCONTROL)
        {
            m_sCurrentValue.clear();
            return true;
        }
    }

    return false;
}


bool SearchableControlIterator::ShouldStepInto(const Reference< XInterface>& /*xContainer*/) const
{
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
