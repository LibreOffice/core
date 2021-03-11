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
#ifndef INCLUDED_SVX_SOURCE_INC_FMSHIMP_HXX
#define INCLUDED_SVX_SOURCE_INC_FMSHIMP_HXX

#include <config_options.h>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/form/runtime/FeatureState.hpp>
#include <tools/diagnose_ex.h>
#include <vcl/timer.hxx>
#include <sfx2/shell.hxx>
#include <svx/svdmark.hxx>
#include <svx/fmsearch.hxx>

#include <svx/fmtools.hxx>
#include <osl/mutex.hxx>
#include <comphelper/container.hxx>
#include <cppuhelper/compbase.hxx>
#include <unotools/configitem.hxx>
#include "formcontrolling.hxx"
#include "fmdocumentclassification.hxx"
#include <o3tl/typed_flags_set.hxx>

#include <queue>
#include <string_view>
#include <vector>
#include <memory>

struct ImplSVEvent;

typedef std::vector< css::uno::Reference< css::form::XForm > > FmFormArray;

// catch database exceptions if they occur
#define DO_SAFE(statement) try { statement; } catch( const Exception& ) { TOOLS_WARN_EXCEPTION("svx", "unhandled exception (I tried to move a cursor (or something like that).)"); }

enum class LoopGridsSync {
    DISABLE_SYNC     = 1,
    FORCE_SYNC       = 2,
    ENABLE_SYNC      = 3
};
enum class LoopGridsFlags
{
    NONE             = 0,
    DISABLE_ROCTRLR  = 4
};
namespace o3tl
{
    template<> struct typed_flags<LoopGridsFlags> : is_typed_flags<LoopGridsFlags, 0x04> {};
}


// flags for controlling the behaviour when calling loadForms
enum class LoadFormsFlags {
    Load          = 0x0000,      // default: simply load
    Sync          = 0x0000,      // default: do in synchronous
    Unload        = 0x0001,      // unload
    Async         = 0x0002      // do this async
};
namespace o3tl {
    template<> struct typed_flags<LoadFormsFlags> : is_typed_flags<LoadFormsFlags, 0x0003> {};
}

namespace weld {
    class Menu;
}

// a class iterating through all fields of a form which are bound to a field
// sub forms are ignored, grid columns (where the grid is a direct child of the form) are included
class FmXBoundFormFieldIterator : public ::comphelper::IndexAccessIterator
{
public:
    FmXBoundFormFieldIterator(const css::uno::Reference< css::uno::XInterface>& _rStartingPoint) : ::comphelper::IndexAccessIterator(_rStartingPoint) { }

protected:
    virtual bool ShouldHandleElement(const css::uno::Reference< css::uno::XInterface>& _rElement) override;
    virtual bool ShouldStepInto(const css::uno::Reference< css::uno::XInterface>& _rContainer) const override;
};

class FmFormPage;
class FmFormPageImpl;

struct FmLoadAction
{
    FmFormPage* pPage;
    ImplSVEvent * nEventId;
    LoadFormsFlags  nFlags;

    FmLoadAction( FmFormPage* _pPage, LoadFormsFlags _nFlags, ImplSVEvent * _nEventId )
        :pPage( _pPage ), nEventId( _nEventId ), nFlags( _nFlags )
    {
    }
};


class SfxViewFrame;
typedef ::cppu::WeakComponentImplHelper<   css::beans::XPropertyChangeListener
                                        ,   css::container::XContainerListener
                                        ,   css::view::XSelectionChangeListener
                                        ,   css::form::XFormControllerListener
                                        >   FmXFormShell_BD_BASE;


class FmXFormShell_Base_Disambiguation : public FmXFormShell_BD_BASE
{
    using css::beans::XPropertyChangeListener::disposing;
protected:
    FmXFormShell_Base_Disambiguation( ::osl::Mutex& _rMutex );
    using WeakComponentImplHelperBase::disposing;
};


namespace svx
{
    class FmTextControlShell;
}


typedef FmXFormShell_Base_Disambiguation    FmXFormShell_BASE;
typedef ::utl::ConfigItem                   FmXFormShell_CFGBASE;

struct SdrViewEvent;
class FmFormShell;
class FmFormView;
class FmFormObj;
class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) FmXFormShell final : public FmXFormShell_BASE
                                    ,public FmXFormShell_CFGBASE
                                    ,public svx::IControllerFeatureInvalidation
{
    friend class FmFormView;
    friend class FmXFormView;

    class SuspendPropertyTracking;
    friend class SuspendPropertyTracking;

    // timer for delayed mark
    Timer               m_aMarkTimer;
    std::vector<SdrObject*> m_arrSearchedControls;
        // We enable a permanent cursor for the grid we found a searched text, it's disabled in the next "found" event.
    FmFormArray         m_aSearchForms;

    struct SAL_DLLPRIVATE InvalidSlotInfo {
        sal_uInt16 id;
        sal_uInt8   flags;
        InvalidSlotInfo(sal_uInt16 slotId, sal_uInt8 flgs) : id(slotId), flags(flgs) {};
    };
    std::vector<InvalidSlotInfo> m_arrInvalidSlots;
        // we explicitly switch off the propbrw before leaving the design mode
        // this flag tells us if we have to switch it on again when reentering

    css::form::NavigationBarMode   m_eNavigate;                // kind of navigation

        // since I want to mark an SdrObject when searching for the treatment of the "found",
        // I get all relevant objects before yanking up of the search dialog
        // (the array is thus only valid during the search process)
    std::vector<tools::Long> m_arrRelativeGridColumn;

    ::osl::Mutex    m_aMutex;
    ImplSVEvent *   m_nInvalidationEvent;
    ImplSVEvent *   m_nActivationEvent;
    ::std::queue< FmLoadAction >
                    m_aLoadingPages;

    FmFormShell*                m_pShell;
    std::unique_ptr<svx::FmTextControlShell>  m_pTextShell;

    svx::ControllerFeatures   m_aActiveControllerFeatures;
    svx::ControllerFeatures   m_aNavControllerFeatures;

    // current form, controller
    // only available in the alive mode
    css::uno::Reference< css::form::runtime::XFormController >    m_xActiveController;
    css::uno::Reference< css::form::runtime::XFormController >    m_xNavigationController;
    css::uno::Reference< css::form::XForm >                       m_xActiveForm;

    // current container of a page
    // only available in the design mode
    css::uno::Reference< css::container::XIndexAccess>            m_xForms;

    // the currently selected objects, as to be displayed in the property browser
    InterfaceBag                                                  m_aCurrentSelection;
    /// the currently selected form, or the form which all currently selected controls belong to, or <NULL/>
    css::uno::Reference< css::form::XForm >                       m_xCurrentForm;
    /// the last selection/marking of controls only. Necessary to implement the "Control properties" slot
    InterfaceBag                                                  m_aLastKnownMarkedControls;


        // And this is also for the 'found': When I find in GridControls, I need the column,
        // but only get the number of the field corresponding to the number of the
        // column + <offset>, where the offset depends on the position of the GridControl
        // in the form. So here is a conversion.
    css::uno::Reference< css::awt::XControlModel>                 m_xLastGridFound;
     // the frame we live in
    css::uno::Reference< css::frame::XFrame>                      m_xAttachedFrame;
    // Administration of external form views (see the SID_FM_VIEW_AS_GRID-slot)
    css::uno::Reference< css::frame::XController >                m_xExternalViewController;      // the controller for the external form view
    css::uno::Reference< css::form::runtime::XFormController >    m_xExtViewTriggerController;    // the nav controller at the time the external display was triggered
    css::uno::Reference< css::sdbc::XResultSet >                  m_xExternalDisplayedForm;       // the form which the external view is based on

    mutable ::svxform::DocumentType
                    m_eDocumentType;        /// the type of document we're living in
    sal_Int16       m_nLockSlotInvalidation;
    bool        m_bHadPropertyBrowserInDesignMode : 1;

    bool        m_bTrackProperties  : 1;
        // should I (or the owner of this impl class) take car of the update of the css::beans::Property-Browser?

    bool        m_bUseWizards : 1;

    bool        m_bDatabaseBar      : 1;    // is there a database bar
    bool        m_bInActivate       : 1;    // is a controller activated
    bool        m_bSetFocus         : 1;    // may the focus be changed over
    bool        m_bFilterMode       : 1;    // is a filter currently set to the controls
    bool        m_bChangingDesignMode:1;    // sal_True within SetDesignMode
    bool        m_bPreparedClose    : 1;    // for the current modification state of the current form
                                                //  PrepareClose had been called and the user denied to save changes
    bool        m_bFirstActivation  : 1;    // has the shell ever been activated?

public:
    // attribute access
    SAL_DLLPRIVATE const css::uno::Reference< css::frame::XFrame >&
                getHostFrame_Lock() const { return m_xAttachedFrame; }
    SAL_DLLPRIVATE const css::uno::Reference< css::sdbc::XResultSet >&
                getExternallyDisplayedForm_Lock() const { return m_xExternalDisplayedForm; }

    SAL_DLLPRIVATE bool
                didPrepareClose_Lock() const { return m_bPreparedClose; }
    SAL_DLLPRIVATE void
                didPrepareClose_Lock(bool bDid) { m_bPreparedClose = bDid; }

    SAL_DLLPRIVATE FmXFormShell(FmFormShell& _rShell, SfxViewFrame* _pViewFrame);

private:
    SAL_DLLPRIVATE virtual ~FmXFormShell() override;

// EventListener
    SAL_DLLPRIVATE virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// css::container::XContainerListener
    SAL_DLLPRIVATE virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) override;
    SAL_DLLPRIVATE virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) override;
    SAL_DLLPRIVATE virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) override;

// XSelectionChangeListener
    SAL_DLLPRIVATE virtual void SAL_CALL selectionChanged(const css::lang::EventObject& rEvent) override;

// css::beans::XPropertyChangeListener
    SAL_DLLPRIVATE virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

// css::form::XFormControllerListener
    SAL_DLLPRIVATE virtual void SAL_CALL formActivated(const css::lang::EventObject& rEvent) override;
    SAL_DLLPRIVATE virtual void SAL_CALL formDeactivated(const css::lang::EventObject& rEvent) override;

// OComponentHelper
    SAL_DLLPRIVATE virtual void SAL_CALL disposing() override;

public:
    SAL_DLLPRIVATE void EnableTrackProperties_Lock(bool bEnable) { m_bTrackProperties = bEnable; }
    SAL_DLLPRIVATE bool IsTrackPropertiesEnabled_Lock() const { return m_bTrackProperties; }

    // activation handling
    SAL_DLLPRIVATE void        viewActivated_Lock(FmFormView& _rCurrentView, bool _bSyncAction = false);
    SAL_DLLPRIVATE void        viewDeactivated_Lock(FmFormView& _rCurrentView, bool _bDeactivateController = true);

    // IControllerFeatureInvalidation
    SAL_DLLPRIVATE virtual void invalidateFeatures/*_NoLock*/( const ::std::vector< sal_Int32 >& _rFeatures ) override;

    SAL_DLLPRIVATE void ExecuteTabOrderDialog_Lock( // execute SID_FM_TAB_DIALOG
        const css::uno::Reference< css::awt::XTabControllerModel >& _rxForForm
    );

    // stuff
    SAL_DLLPRIVATE void AddElement_Lock(const css::uno::Reference< css::uno::XInterface>& Element);
    SAL_DLLPRIVATE void RemoveElement_Lock(const css::uno::Reference< css::uno::XInterface>& Element);

    /** updates m_xForms, to be either <NULL/>, if we're in alive mode, or our current page's forms collection,
        if in design mode
    */
    SAL_DLLPRIVATE void UpdateForms_Lock(bool bInvalidate);

    SAL_DLLPRIVATE void ExecuteSearch_Lock();      // execute SID_FM_SEARCH
    SAL_DLLPRIVATE void CreateExternalView_Lock(); // execute SID_FM_VIEW_AS_GRID

    SAL_DLLPRIVATE bool GetY2KState_Lock(sal_uInt16 & n);
    SAL_DLLPRIVATE void SetY2KState_Lock(sal_uInt16 n);

private:
    // form handling
    /// load or unload the forms on a page
    SAL_DLLPRIVATE void loadForms_Lock( FmFormPage* _pPage, const LoadFormsFlags _nBehaviour );
    SAL_DLLPRIVATE void smartControlReset( const css::uno::Reference< css::container::XIndexAccess >& _rxModels );


    SAL_DLLPRIVATE void startListening_Lock();
    SAL_DLLPRIVATE void stopListening_Lock();

    SAL_DLLPRIVATE css::uno::Reference< css::awt::XControl >
        impl_getControl_Lock(
            const css::uno::Reference< css::awt::XControlModel>& i_rxModel,
            const FmFormObj& i_rKnownFormObj
        );

    // collects in strNames the names of all forms
    SAL_DLLPRIVATE static void impl_collectFormSearchContexts_nothrow_Lock(
        const css::uno::Reference< css::uno::XInterface>& _rxStartingPoint,
        const OUString& _rCurrentLevelPrefix,
        FmFormArray& _out_rForms,
        ::std::vector< OUString >& _out_rNames );

    /** checks whenever the instance is already disposed, if so, this is reported as assertion error (debug
        builds only) and <TRUE/> is returned.
    */
    SAL_DLLPRIVATE bool    impl_checkDisposed_Lock() const;

public:
    // method for non design mode (alive mode)
    SAL_DLLPRIVATE void setActiveController_Lock(const css::uno::Reference< css::form::runtime::XFormController>& _xController, bool _bNoSaveOldContent = false);
    SAL_DLLPRIVATE const css::uno::Reference< css::form::runtime::XFormController>& getActiveController_Lock() const { return m_xActiveController; }
    SAL_DLLPRIVATE const css::uno::Reference< css::form::runtime::XFormController>& getActiveInternalController_Lock() const { return m_xActiveController == m_xExternalViewController ? m_xExtViewTriggerController : m_xActiveController; }
    SAL_DLLPRIVATE const css::uno::Reference< css::form::XForm>& getActiveForm_Lock() const { return m_xActiveForm; }
    SAL_DLLPRIVATE const css::uno::Reference< css::form::runtime::XFormController>& getNavController_Lock() const { return m_xNavigationController; }

    SAL_DLLPRIVATE const svx::ControllerFeatures& getActiveControllerFeatures_Lock() const
        { return m_aActiveControllerFeatures; }
    SAL_DLLPRIVATE const svx::ControllerFeatures& getNavControllerFeatures_Lock() const
        { return m_aNavControllerFeatures.isAssigned() ? m_aNavControllerFeatures : m_aActiveControllerFeatures; }

    /** announces a new "current selection"
        @return
            <TRUE/> if and only if the to-bet-set selection was different from the previous selection
    */
    SAL_DLLPRIVATE bool setCurrentSelection_Lock(const InterfaceBag& rSelection);

    /** sets the new selection to the last known marked controls
    */
    SAL_DLLPRIVATE bool selectLastMarkedControls_Lock();

    /** retrieves the current selection
    */
    void    getCurrentSelection_Lock(InterfaceBag& /* [out] */ _rSelection) const;

    /** sets a new current selection as indicated by a mark list
        @return
            <TRUE/> if and only if the to-bet-set selection was different from the previous selection
    */
    SAL_DLLPRIVATE bool setCurrentSelectionFromMark_Lock(const SdrMarkList& rMarkList);

    /// returns the currently selected form, or the form which all currently selected controls belong to, or <NULL/>
    SAL_DLLPRIVATE const css::uno::Reference< css::form::XForm >&
                getCurrentForm_Lock() const { return m_xCurrentForm; }
    SAL_DLLPRIVATE void forgetCurrentForm_Lock();
    /// returns whether the last known marking contained only controls
    SAL_DLLPRIVATE bool onlyControlsAreMarked_Lock() const { return !m_aLastKnownMarkedControls.empty(); }

    /// determines whether the current selection consists of exactly the given object
    SAL_DLLPRIVATE bool isSolelySelected_Lock(
                const css::uno::Reference< css::uno::XInterface >& _rxObject
            );

    /// handles a MouseButtonDown event of the FmFormView
    SAL_DLLPRIVATE void handleMouseButtonDown_Lock( const SdrViewEvent& _rViewEvent );
    /// handles the request for showing the "Properties"
    SAL_DLLPRIVATE void handleShowPropertiesRequest_Lock();

    SAL_DLLPRIVATE bool hasForms_Lock() const { return m_xForms.is() && m_xForms->getCount() != 0; }
    SAL_DLLPRIVATE bool hasDatabaseBar_Lock() const { return m_bDatabaseBar; }

    SAL_DLLPRIVATE void ShowSelectionProperties_Lock(bool bShow);
    SAL_DLLPRIVATE bool IsPropBrwOpen_Lock() const;

    SAL_DLLPRIVATE void DetermineSelection_Lock(const SdrMarkList& rMarkList);
    SAL_DLLPRIVATE void SetSelection_Lock(const SdrMarkList& rMarkList);
    SAL_DLLPRIVATE void SetSelectionDelayed_Lock();

    SAL_DLLPRIVATE void SetDesignMode_Lock(bool bDesign);

    SAL_DLLPRIVATE bool GetWizardUsing_Lock() const { return m_bUseWizards; }
    SAL_DLLPRIVATE void SetWizardUsing_Lock(bool _bUseThem);

        // setting the filter mode
    SAL_DLLPRIVATE bool isInFilterMode_Lock() const { return m_bFilterMode; }
    SAL_DLLPRIVATE void startFiltering_Lock();
    SAL_DLLPRIVATE void stopFiltering_Lock(bool bSave);

        // fills rMenu to be a menu that contains all ControlConversion entries
    SAL_DLLPRIVATE static void GetConversionMenu_Lock(weld::Menu& rMenu);

    /// checks whether a given control conversion slot can be applied to the current selection
    SAL_DLLPRIVATE bool canConvertCurrentSelectionToControl_Lock(std::string_view rIdent);
    /// enables or disables all conversion slots in a menu, according to the current selection
    SAL_DLLPRIVATE void checkControlConversionSlotsForCurrentSelection_Lock(weld::Menu& rMenu);
    /// executes a control conversion slot for a given object
    SAL_DLLPRIVATE bool executeControlConversionSlot_Lock(const css::uno::Reference< css::form::XFormComponent >& _rxObject, std::string_view rIdent);
    /** executes a control conversion slot for the current selection
        @precond canConvertCurrentSelectionToControl( <arg>_nSlotId</arg> ) must return <TRUE/>
    */
    SAL_DLLPRIVATE void executeControlConversionSlot_Lock(std::string_view rIdent);
    /// checks whether the given slot id denotes a control conversion slot
    SAL_DLLPRIVATE static bool isControlConversionSlot(std::string_view rIdent);

    SAL_DLLPRIVATE void ExecuteTextAttribute_Lock(SfxRequest& _rReq);
    SAL_DLLPRIVATE void GetTextAttributeState_Lock(SfxItemSet& _rSet);
    SAL_DLLPRIVATE bool IsActiveControl_Lock(bool _bCountRichTextOnly) const;
    SAL_DLLPRIVATE void ForgetActiveControl_Lock();
    SAL_DLLPRIVATE void SetControlActivationHandler_Lock(const Link<LinkParamNone*,void>& _rHdl);

    /// classifies our host document
    SAL_DLLPRIVATE ::svxform::DocumentType getDocumentType_Lock() const;
    SAL_DLLPRIVATE bool isEnhancedForm_Lock() const;

    /// determines whether our host document is currently read-only
    SAL_DLLPRIVATE bool IsReadonlyDoc_Lock() const;

    // Setting the curObject/selObject/curForm is delayed (SetSelectionDelayed). With the
    // following functions this can be inquired/enforced.
    SAL_DLLPRIVATE inline bool IsSelectionUpdatePending_Lock() const;
    SAL_DLLPRIVATE void        ForceUpdateSelection_Lock();

    SAL_DLLPRIVATE css::uno::Reference< css::frame::XModel>          getContextDocument_Lock() const;
    SAL_DLLPRIVATE css::uno::Reference< css::form::XForm>            getInternalForm_Lock(const css::uno::Reference< css::form::XForm>& _xForm) const;
    SAL_DLLPRIVATE css::uno::Reference< css::sdbc::XResultSet>       getInternalForm_Lock(const css::uno::Reference< css::sdbc::XResultSet>& _xForm) const;
        // if the form belongs to the controller (extern) displaying a grid, the according internal form will
        // be displayed, _xForm else

    // check if the current control of the active controller has the focus
    SAL_DLLPRIVATE bool HasControlFocus_Lock() const;

private:
    DECL_DLLPRIVATE_LINK(OnFoundData_Lock, FmFoundRecordInformation&, void);
    DECL_DLLPRIVATE_LINK(OnCanceledNotFound_Lock, FmFoundRecordInformation&, void);
    DECL_DLLPRIVATE_LINK(OnSearchContextRequest_Lock, FmSearchContext&, sal_uInt32);
    DECL_DLLPRIVATE_LINK(OnTimeOut_Lock, Timer*, void);
    DECL_DLLPRIVATE_LINK(OnFirstTimeActivation_Lock, void*, void);
    DECL_DLLPRIVATE_LINK(OnFormsCreated_Lock, FmFormPageImpl&, void);

    SAL_DLLPRIVATE void LoopGrids_Lock(LoopGridsSync nSync, LoopGridsFlags nWhat = LoopGridsFlags::NONE);

    // invalidation of slots
    SAL_DLLPRIVATE void InvalidateSlot_Lock(sal_Int16 nId, bool bWithId);
    SAL_DLLPRIVATE void UpdateSlot_Lock(sal_Int16 nId);
    // locking the invalidation - if the internal locking counter goes to 0, all accumulated slots
    // are invalidated (asynchronously)
    SAL_DLLPRIVATE void LockSlotInvalidation_Lock(bool bLock);

    DECL_DLLPRIVATE_LINK(OnInvalidateSlots_Lock, void*, void);

    SAL_DLLPRIVATE void CloseExternalFormViewer_Lock();
        // closes the task-local beamer displaying a grid view for a form

    // ConfigItem related stuff
    SAL_DLLPRIVATE virtual void Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;
    SAL_DLLPRIVATE void implAdjustConfigCache_Lock();

    SAL_DLLPRIVATE css::uno::Reference< css::awt::XControlContainer >
            getControlContainerForView_Lock() const;

    /** finds and sets a default for m_xCurrentForm, if it is currently NULL
    */
    SAL_DLLPRIVATE void impl_defaultCurrentForm_nothrow_Lock();

    /** sets m_xCurrentForm to the provided form, and updates everything which
        depends on the current form
    */
    SAL_DLLPRIVATE void impl_updateCurrentForm_Lock( const css::uno::Reference< css::form::XForm >& _rxNewCurForm );

    /** adds or removes ourself as XEventListener at m_xActiveController
    */
    SAL_DLLPRIVATE void impl_switchActiveControllerListening_Lock(const bool _bListen);

    /** add an element
    */
    SAL_DLLPRIVATE void    impl_AddElement_nothrow(const css::uno::Reference< css::uno::XInterface>& Element);

    /** remove an element
    */
    SAL_DLLPRIVATE void    impl_RemoveElement_nothrow_Lock(const css::uno::Reference< css::uno::XInterface>& Element);

    SAL_DLLPRIVATE virtual void ImplCommit() override;

    // asynchronous cursor actions/navigation slot handling

public:
    /** execute the given form slot
        <p>Warning. Only a small set of slots implemented currently.</p>
        @param _nSlot
            the slot to execute
    */
    SAL_DLLPRIVATE void ExecuteFormSlot_Lock(sal_Int32 _nSlot);

    /** determines whether the current form slot is currently enabled
    */
    SAL_DLLPRIVATE bool    IsFormSlotEnabled( sal_Int32 _nSlot, css::form::runtime::FeatureState* _pCompleteState );

    SAL_DLLPRIVATE static OString SlotToIdent(sal_uInt16 nSlot);

private:
    DECL_DLLPRIVATE_LINK( OnLoadForms_Lock, void*, void );
};


inline bool FmXFormShell::IsSelectionUpdatePending_Lock() const
{
    return m_aMarkTimer.IsActive();
}


// = An iterator that, emanating from an interface, looks for an object whose
// = css::beans::Property-Set has a ControlSource and a BoundField property, the
// = latter having a non-NULL value. If the interface itself does not meet this
// = condition, it is tested whether it is a container (that is, has a
// = css::container::XIndexAccess), then it is descended there and the same tried
// = for each element of the container (again possibly with descent). If any
// = object thereby has the required property, the part with the container test
// = for that object is omitted.
// =

class SearchableControlIterator : public ::comphelper::IndexAccessIterator
{
    OUString         m_sCurrentValue;
        // the current value of the ControlSource css::beans::Property

public:
    const OUString& getCurrentValue() const { return m_sCurrentValue; }

public:
    SearchableControlIterator(css::uno::Reference< css::uno::XInterface> const & xStartingPoint);

    virtual bool ShouldHandleElement(const css::uno::Reference< css::uno::XInterface>& rElement) override;
    virtual bool ShouldStepInto(const css::uno::Reference< css::uno::XInterface>& xContainer) const override;
    virtual void Invalidate() override { IndexAccessIterator::Invalidate(); m_sCurrentValue.clear(); }
};

#endif // INCLUDED_SVX_SOURCE_INC_FMSHIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
