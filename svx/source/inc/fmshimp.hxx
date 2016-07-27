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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/form/runtime/FeatureState.hpp>
#include <vcl/timer.hxx>
#include <sfx2/app.hxx>
#include <svx/svdmark.hxx>
#include <svx/fmsearch.hxx>
#include <svx/svxids.hrc>
#include <svl/lstner.hxx>

#include "svx/fmtools.hxx"
#include "svx/fmsrccfg.hxx"
#include <osl/mutex.hxx>
#include <cppuhelper/component.hxx>
#include <comphelper/container.hxx>
#include <cppuhelper/compbase.hxx>
#include <unotools/configitem.hxx>
#include "formcontrolling.hxx"
#include "fmdocumentclassification.hxx"
#include <o3tl/typed_flags_set.hxx>

#include <queue>
#include <set>
#include <vector>
#include <memory>

typedef std::vector< css::uno::Reference< css::form::XForm > > FmFormArray;

// catch database exceptions if they occur
#define DO_SAFE(statement) try { statement; } catch( const Exception& ) { OSL_FAIL("unhandled exception (I tried to move a cursor (or something like that).)"); }

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
class SVX_DLLPUBLIC FmXFormShell   : public FmXFormShell_BASE
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
        inline InvalidSlotInfo(sal_uInt16 slotId, sal_uInt8 flgs) : id(slotId), flags(flgs) {};
    };
    std::vector<InvalidSlotInfo> m_arrInvalidSlots;
        // we explicitly switch off the propbrw before leaving the design mode
        // this flag tells us if we have to switch it on again when reentering

    ::osl::Mutex    m_aAsyncSafety;
        // secure the access to our thread related members
    ::osl::Mutex    m_aInvalidationSafety;
        // secure the access to all our slot invalidation related members

    css::form::NavigationBarMode   m_eNavigate;                // Art der Navigation

        // da ich beim Suchen fuer die Behandlung des "gefunden" ein SdrObject markieren will, besorge ich mir vor dem
        // Hochreissen des Suchen-Dialoges alle relevanten Objekte
        // (das Array ist damit auch nur waehrend des Suchvorganges gueltig)
    std::vector<long> m_arrRelativeGridColumn;

    ::osl::Mutex    m_aMutex;
    ImplSVEvent *   m_nInvalidationEvent;
    ImplSVEvent *   m_nActivationEvent;
    ::std::queue< FmLoadAction >
                    m_aLoadingPages;

    FmFormShell*                m_pShell;
    svx::FmTextControlShell*  m_pTextShell;

    svx::ControllerFeatures   m_aActiveControllerFeatures;
    svx::ControllerFeatures   m_aNavControllerFeatures;

    // aktuelle Form, Controller
    // nur im alive mode verfuegbar
    css::uno::Reference< css::form::runtime::XFormController >    m_xActiveController;
    css::uno::Reference< css::form::runtime::XFormController >    m_xNavigationController;
    css::uno::Reference< css::form::XForm >                       m_xActiveForm;

    // Aktueller container einer Page
    // nur im designmode verfuegbar
    css::uno::Reference< css::container::XIndexAccess>            m_xForms;

    // the currently selected objects, as to be displayed in the property browser
    InterfaceBag                                                  m_aCurrentSelection;
    /// the currently selected form, or the form which all currently selected controls belong to, or <NULL/>
    css::uno::Reference< css::form::XForm >                       m_xCurrentForm;
    /// the last selection/marking of controls only. Necessary to implement the "Control properties" slot
    InterfaceBag                                                  m_aLastKnownMarkedControls;


        // und das ist ebenfalls fuer's 'gefunden' : Beim Finden in GridControls brauche ich die Spalte, bekomme aber
        // nur die Nummer des Feldes, die entspricht der Nummer der Spalte + <offset>, wobei der Offset von der Position
        // des GridControls im Formular abhaengt. Also hier eine Umrechnung.
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
        // soll ich (bzw. der Owner diese Impl-Klasse) mich um die Aktualisierung des css::beans::Property-Browsers kuemmern ?

    bool        m_bUseWizards : 1;

    bool        m_bDatabaseBar      : 1;    // Gibt es eine Datenbankleiste
    bool        m_bInActivate       : 1;    // Wird ein Controller aktiviert
    bool        m_bSetFocus         : 1;    // Darf der Focus umgesetzt werden
    bool        m_bFilterMode       : 1;    // Wird gerade ein Filter auf die Controls angesetzt
    bool        m_bChangingDesignMode:1;    // sal_True within SetDesignMode
    bool        m_bPreparedClose    : 1;    // for the current modification state of the current form
                                                //  PrepareClose had been called and the user denied to save changes
    bool        m_bFirstActivation  : 1;    // has the shell ever been activated?

public:
    // attribute access
    SAL_DLLPRIVATE inline const css::uno::Reference< css::frame::XFrame >&
                getHostFrame() const { return m_xAttachedFrame; }
    SAL_DLLPRIVATE inline const css::uno::Reference< css::sdbc::XResultSet >&
                getExternallyDisplayedForm() const { return m_xExternalDisplayedForm; }

    SAL_DLLPRIVATE inline bool
                didPrepareClose() const { return m_bPreparedClose; }
    SAL_DLLPRIVATE inline void
                didPrepareClose( bool _bDid ) { m_bPreparedClose = _bDid; }

public:
    SAL_DLLPRIVATE FmXFormShell(FmFormShell& _rShell, SfxViewFrame* _pViewFrame);

protected:
    SAL_DLLPRIVATE virtual ~FmXFormShell();

// EventListener
    SAL_DLLPRIVATE virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw( css::uno::RuntimeException, std::exception ) override;

// css::container::XContainerListener
    SAL_DLLPRIVATE virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
    SAL_DLLPRIVATE virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
    SAL_DLLPRIVATE virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

// XSelectionChangeListener
    SAL_DLLPRIVATE virtual void SAL_CALL selectionChanged(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

// css::beans::XPropertyChangeListener
    SAL_DLLPRIVATE virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) throw( css::uno::RuntimeException, std::exception ) override;

// css::form::XFormControllerListener
    SAL_DLLPRIVATE virtual void SAL_CALL formActivated(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;
    SAL_DLLPRIVATE virtual void SAL_CALL formDeactivated(const css::lang::EventObject& rEvent) throw( css::uno::RuntimeException, std::exception ) override;

// OComponentHelper
    SAL_DLLPRIVATE virtual void SAL_CALL disposing() override;

public:
    SAL_DLLPRIVATE void EnableTrackProperties( bool bEnable) { m_bTrackProperties = bEnable; }
    SAL_DLLPRIVATE bool IsTrackPropertiesEnabled() {return m_bTrackProperties;}

    // activation handling
    SAL_DLLPRIVATE void        viewActivated( FmFormView& _rCurrentView, bool _bSyncAction = false );
    SAL_DLLPRIVATE void        viewDeactivated( FmFormView& _rCurrentView, bool _bDeactivateController = true );

    // IControllerFeatureInvalidation
    SAL_DLLPRIVATE virtual void invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures ) override;

    SAL_DLLPRIVATE void ExecuteTabOrderDialog(         // execute SID_FM_TAB_DIALOG
        const css::uno::Reference< css::awt::XTabControllerModel >& _rxForForm
    );

    // stuff
    SAL_DLLPRIVATE void AddElement(const css::uno::Reference< css::uno::XInterface>& Element);
    SAL_DLLPRIVATE void RemoveElement(const css::uno::Reference< css::uno::XInterface>& Element);

    /** updates m_xForms, to be either <NULL/>, if we're in alive mode, or our current page's forms collection,
        if in design mode
    */
    SAL_DLLPRIVATE void UpdateForms( bool _bInvalidate );

    SAL_DLLPRIVATE void ExecuteSearch();               // execute SID_FM_SEARCH
    SAL_DLLPRIVATE void CreateExternalView();          // execute SID_FM_VIEW_AS_GRID

    SAL_DLLPRIVATE bool        GetY2KState(sal_uInt16& n);
    SAL_DLLPRIVATE void        SetY2KState(sal_uInt16 n);

protected:
    // activation handling
    SAL_DLLPRIVATE inline  bool    hasEverBeenActivated( ) const { return !m_bFirstActivation; }
    SAL_DLLPRIVATE inline  void        setHasBeenActivated( ) { m_bFirstActivation = false; }

    // form handling
    /// load or unload the forms on a page
    SAL_DLLPRIVATE         void        loadForms( FmFormPage* _pPage, const LoadFormsFlags _nBehaviour = LoadFormsFlags::Load | LoadFormsFlags::Sync );
    SAL_DLLPRIVATE         void        smartControlReset( const css::uno::Reference< css::container::XIndexAccess >& _rxModels );


    SAL_DLLPRIVATE void startListening();
    SAL_DLLPRIVATE void stopListening();

    SAL_DLLPRIVATE css::uno::Reference< css::awt::XControl >
        impl_getControl(
            const css::uno::Reference< css::awt::XControlModel>& i_rxModel,
            const FmFormObj& i_rKnownFormObj
        );

    // sammelt in strNames die Namen aller Formulare
    SAL_DLLPRIVATE static void impl_collectFormSearchContexts_nothrow(
        const css::uno::Reference< css::uno::XInterface>& _rxStartingPoint,
        const OUString& _rCurrentLevelPrefix,
        FmFormArray& _out_rForms,
        ::std::vector< OUString >& _out_rNames );

    /** checks whenever the instance is already disposed, if so, this is reported as assertion error (debug
        builds only) and <TRUE/> is returned.
    */
    SAL_DLLPRIVATE bool    impl_checkDisposed() const;

public:
    // methode fuer nicht designmode (alive mode)
    SAL_DLLPRIVATE void setActiveController( const css::uno::Reference< css::form::runtime::XFormController>& _xController, bool _bNoSaveOldContent = false );
    SAL_DLLPRIVATE const css::uno::Reference< css::form::runtime::XFormController>& getActiveController() const {return m_xActiveController;}
    SAL_DLLPRIVATE const css::uno::Reference< css::form::runtime::XFormController>& getActiveInternalController() const { return m_xActiveController == m_xExternalViewController ? m_xExtViewTriggerController : m_xActiveController; }
    SAL_DLLPRIVATE const css::uno::Reference< css::form::XForm>& getActiveForm() const {return m_xActiveForm;}
    SAL_DLLPRIVATE const css::uno::Reference< css::form::runtime::XFormController>& getNavController() const {return m_xNavigationController;}

    SAL_DLLPRIVATE inline const svx::ControllerFeatures& getActiveControllerFeatures() const
        { return m_aActiveControllerFeatures; }
    SAL_DLLPRIVATE inline const svx::ControllerFeatures& getNavControllerFeatures() const
        { return m_aNavControllerFeatures.isAssigned() ? m_aNavControllerFeatures : m_aActiveControllerFeatures; }

    /** announces a new "current selection"
        @return
            <TRUE/> if and only if the to-bet-set selection was different from the previous selection
    */
    SAL_DLLPRIVATE bool    setCurrentSelection( const InterfaceBag& _rSelection );

    /** sets the new selection to the last known marked controls
    */
    SAL_DLLPRIVATE bool    selectLastMarkedControls();

    /** retrieves the current selection
    */
    void    getCurrentSelection( InterfaceBag& /* [out] */ _rSelection ) const;

    /** sets a new current selection as indicated by a mark list
        @return
            <TRUE/> if and only if the to-bet-set selection was different from the previous selection
    */
    SAL_DLLPRIVATE bool    setCurrentSelectionFromMark(const SdrMarkList& rMarkList);

    /// returns the currently selected form, or the form which all currently selected controls belong to, or <NULL/>
    SAL_DLLPRIVATE const css::uno::Reference< css::form::XForm >&
                getCurrentForm() const { return m_xCurrentForm; }
    SAL_DLLPRIVATE void        forgetCurrentForm();
    /// returns whether the last known marking contained only controls
    SAL_DLLPRIVATE bool    onlyControlsAreMarked() const { return !m_aLastKnownMarkedControls.empty(); }

    /// determines whether the current selection consists of exactly the given object
    SAL_DLLPRIVATE bool    isSolelySelected(
                const css::uno::Reference< css::uno::XInterface >& _rxObject
            );

    /// handles a MouseButtonDown event of the FmFormView
    SAL_DLLPRIVATE void handleMouseButtonDown( const SdrViewEvent& _rViewEvent );
    /// handles the request for showing the "Properties"
    SAL_DLLPRIVATE void handleShowPropertiesRequest();

    SAL_DLLPRIVATE bool hasForms() const {return m_xForms.is() && m_xForms->getCount() != 0;}
    SAL_DLLPRIVATE bool hasDatabaseBar() const {return m_bDatabaseBar;}

    SAL_DLLPRIVATE void ShowSelectionProperties( bool bShow );
    SAL_DLLPRIVATE bool IsPropBrwOpen() const;

    SAL_DLLPRIVATE void DetermineSelection(const SdrMarkList& rMarkList);
    SAL_DLLPRIVATE void SetSelection(const SdrMarkList& rMarkList);
    SAL_DLLPRIVATE void SetSelectionDelayed();

    SAL_DLLPRIVATE void SetDesignMode(bool bDesign);

    SAL_DLLPRIVATE bool    GetWizardUsing() const { return m_bUseWizards; }
    SAL_DLLPRIVATE void    SetWizardUsing(bool _bUseThem);

        // Setzen des Filtermodus
    SAL_DLLPRIVATE bool isInFilterMode() const {return m_bFilterMode;}
    SAL_DLLPRIVATE void startFiltering();
    SAL_DLLPRIVATE void stopFiltering(bool bSave);

    SAL_DLLPRIVATE static VclPtr<PopupMenu> GetConversionMenu();
        // ein Menue, das alle ControlConversion-Eintraege enthaelt

    /// checks whether a given control conversion slot can be applied to the current selection
    SAL_DLLPRIVATE        bool canConvertCurrentSelectionToControl( sal_Int16 nConversionSlot );
    /// enables or disables all conversion slots in a menu, according to the current selection
    SAL_DLLPRIVATE        void checkControlConversionSlotsForCurrentSelection( Menu& rMenu );
    /// executes a control conversion slot for a given object
    SAL_DLLPRIVATE        bool executeControlConversionSlot( const css::uno::Reference< css::form::XFormComponent >& _rxObject, sal_uInt16 _nSlotId );
    /** executes a control conversion slot for the current selection
        @precond canConvertCurrentSelectionToControl( <arg>_nSlotId</arg> ) must return <TRUE/>
    */
    SAL_DLLPRIVATE        void executeControlConversionSlot( sal_uInt16 _nSlotId );
    /// checks whether the given slot id denotes a control conversion slot
    SAL_DLLPRIVATE static bool isControlConversionSlot( sal_uInt16 _nSlotId );

    SAL_DLLPRIVATE void    ExecuteTextAttribute( SfxRequest& _rReq );
    SAL_DLLPRIVATE void    GetTextAttributeState( SfxItemSet& _rSet );
    SAL_DLLPRIVATE bool    IsActiveControl( bool _bCountRichTextOnly ) const;
    SAL_DLLPRIVATE void    ForgetActiveControl();
    SAL_DLLPRIVATE void    SetControlActivationHandler( const Link<LinkParamNone*,void>& _rHdl );

    /// classifies our host document
    SAL_DLLPRIVATE ::svxform::DocumentType
            getDocumentType() const;
    SAL_DLLPRIVATE bool    isEnhancedForm() const;

    /// determines whether our host document is currently read-only
    SAL_DLLPRIVATE bool    IsReadonlyDoc() const;

    // das Setzen des curObject/selObject/curForm erfolgt verzoegert (SetSelectionDelayed), mit den folgenden
    // Funktionen laesst sich das abfragen/erzwingen
    SAL_DLLPRIVATE inline bool IsSelectionUpdatePending();
    SAL_DLLPRIVATE void        ForceUpdateSelection();

    SAL_DLLPRIVATE css::uno::Reference< css::frame::XModel>          getContextDocument() const;
    SAL_DLLPRIVATE css::uno::Reference< css::form::XForm>            getInternalForm(const css::uno::Reference< css::form::XForm>& _xForm) const;
    SAL_DLLPRIVATE css::uno::Reference< css::sdbc::XResultSet>       getInternalForm(const css::uno::Reference< css::sdbc::XResultSet>& _xForm) const;
        // if the form belongs to the controller (extern) displaying a grid, the according internal form will
        // be displayed, _xForm else

    // check if the current control of the active controller has the focus
    SAL_DLLPRIVATE bool    HasControlFocus() const;

private:
    DECL_DLLPRIVATE_LINK_TYPED(OnFoundData, FmFoundRecordInformation&, void);
    DECL_DLLPRIVATE_LINK_TYPED(OnCanceledNotFound, FmFoundRecordInformation&, void);
    DECL_DLLPRIVATE_LINK_TYPED(OnSearchContextRequest, FmSearchContext&, sal_uInt32);
    DECL_DLLPRIVATE_LINK_TYPED(OnTimeOut, Timer*, void);
    DECL_DLLPRIVATE_LINK_TYPED(OnFirstTimeActivation, void*, void);
    DECL_DLLPRIVATE_LINK_TYPED(OnFormsCreated, FmFormPageImpl&, void);

    SAL_DLLPRIVATE void LoopGrids(LoopGridsSync nSync, LoopGridsFlags nWhat = LoopGridsFlags::NONE);

    // Invalidierung von Slots
    SAL_DLLPRIVATE void    InvalidateSlot( sal_Int16 nId, bool bWithId );
    SAL_DLLPRIVATE void    UpdateSlot( sal_Int16 nId );
    // Locking der Invalidierung - wenn der interne Locking-Counter auf 0 geht, werden alle aufgelaufenen Slots
    // (asynchron) invalidiert
    SAL_DLLPRIVATE void    LockSlotInvalidation(bool bLock);

    DECL_DLLPRIVATE_LINK_TYPED(OnInvalidateSlots, void*, void);

    SAL_DLLPRIVATE void    CloseExternalFormViewer();
        // closes the task-local beamer displaying a grid view for a form

    // ConfigItem related stuff
    SAL_DLLPRIVATE virtual void Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;
    SAL_DLLPRIVATE void implAdjustConfigCache();

    SAL_DLLPRIVATE css::uno::Reference< css::awt::XControlContainer >
            getControlContainerForView();

    /** finds and sets a default for m_xCurrentForm, if it is currently NULL
    */
    SAL_DLLPRIVATE void    impl_defaultCurrentForm_nothrow();

    /** sets m_xCurrentForm to the provided form, and updates everything which
        depends on the current form
    */
    SAL_DLLPRIVATE void    impl_updateCurrentForm( const css::uno::Reference< css::form::XForm >& _rxNewCurForm );

    /** adds or removes ourself as XEventListener at m_xActiveController
    */
    SAL_DLLPRIVATE void    impl_switchActiveControllerListening( const bool _bListen );

    /** add an element
    */
    SAL_DLLPRIVATE void    impl_AddElement_nothrow(const css::uno::Reference< css::uno::XInterface>& Element);

    /** remove an element
    */
    SAL_DLLPRIVATE void    impl_RemoveElement_nothrow(const css::uno::Reference< css::uno::XInterface>& Element);

    SAL_DLLPRIVATE virtual void ImplCommit() override;

    // asynchronous cursor actions/navigation slot handling

public:
    /** execute the given form slot
        <p>Warning. Only a small set of slots implemented currently.</p>
        @param _nSlot
            the slot to execute
    */
    SAL_DLLPRIVATE void    ExecuteFormSlot( sal_Int32 _nSlot );

    /** determines whether the current form slot is currently enabled
    */
    SAL_DLLPRIVATE bool    IsFormSlotEnabled( sal_Int32 _nSlot, css::form::runtime::FeatureState* _pCompleteState );

protected:
    DECL_DLLPRIVATE_LINK_TYPED( OnLoadForms, void*, void );
};


inline bool FmXFormShell::IsSelectionUpdatePending()
{
    return m_aMarkTimer.IsActive();
}


// = ein Iterator, der ausgehend von einem Interface ein Objekt sucht, dessen
// = css::beans::Property-Set eine ControlSource- sowie eine BoundField-Eigenschaft hat,
// = wobei letztere einen Wert ungleich NULL haben muss.
// = Wenn das Interface selber diese Bedingung nicht erfuellt, wird getestet,
// = ob es ein Container ist (also ueber eine css::container::XIndexAccess verfuegt), dann
// = wird dort abgestiegen und fuer jedes Element des Containers das selbe
// = versucht (wiederum eventuell mit Abstieg).
// = Wenn irgendein Objekt dabei die geforderte Eigenschaft hat, entfaellt
// = der Teil mit dem Container-Test fuer dieses Objekt.
// =

class SearchableControlIterator : public ::comphelper::IndexAccessIterator
{
    OUString         m_sCurrentValue;
        // der aktuelle Wert der ControlSource-css::beans::Property

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
