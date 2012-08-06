/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_FMSHIMP_HXX
#define _SVX_FMSHIMP_HXX

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

#include <sfx2/mnuitem.hxx>
#include "svx/fmtools.hxx"
#include "svx/fmsrccfg.hxx"
#include <osl/mutex.hxx>
#include <cppuhelper/component.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/container.hxx>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/compbase6.hxx>
#include <unotools/configitem.hxx>
#include "svx/dbtoolsclient.hxx"
#include "formcontrolling.hxx"
#include "fmdocumentclassification.hxx"

#include <queue>
#include <set>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

typedef std::vector<SdrObject*> SdrObjArray;
DECLARE_STL_VECTOR( ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > ,FmFormArray);

// catch database exceptions if they occur
#define DO_SAFE(statement) try { statement; } catch( const Exception& ) { OSL_FAIL("unhandled exception (I tried to move a cursor (or something like that).)"); }

#define GA_DISABLE_SYNC     1
#define GA_FORCE_SYNC       2
#define GA_ENABLE_SYNC      3
#define GA_SYNC_MASK        3
#define GA_DISABLE_ROCTRLR  4
#define GA_ENABLE_ROCTRLR   8


// flags for controlling the behaviour when calling loadForms
#define FORMS_LOAD          0x0000      // default: simply load
#define FORMS_SYNC          0x0000      // default: do in synchronous

#define FORMS_UNLOAD        0x0001      // unload
#define FORMS_ASYNC         0x0002      // do this async

//==============================================================================
// a class iterating through all fields of a form which are bound to a field
// sub forms are ignored, grid columns (where the grid is a direct child of the form) are included
class SAL_DLLPRIVATE FmXBoundFormFieldIterator : public ::comphelper::IndexAccessIterator
{
public:
    FmXBoundFormFieldIterator(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rStartingPoint) : ::comphelper::IndexAccessIterator(_rStartingPoint) { }

protected:
    virtual sal_Bool ShouldHandleElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rElement);
    virtual sal_Bool ShouldStepInto(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rContainer) const;
};

class FmFormPage;
//========================================================================
struct SAL_DLLPRIVATE FmLoadAction
{
    FmFormPage* pPage;
    sal_uLong       nEventId;
    sal_uInt16  nFlags;

    FmLoadAction( ) : pPage( NULL ), nEventId( 0 ), nFlags( 0 ) { }
    FmLoadAction( FmFormPage* _pPage, sal_uInt16 _nFlags, sal_uLong _nEventId )
        :pPage( _pPage ), nEventId( _nEventId ), nFlags( _nFlags )
    {
    }
};

//========================================================================
class SfxViewFrame;
typedef ::cppu::WeakComponentImplHelper4<   ::com::sun::star::beans::XPropertyChangeListener
                                        ,   ::com::sun::star::container::XContainerListener
                                        ,   ::com::sun::star::view::XSelectionChangeListener
                                        ,   ::com::sun::star::form::XFormControllerListener
                                        >   FmXFormShell_BD_BASE;

//========================================================================
class SAL_DLLPRIVATE FmXFormShell_Base_Disambiguation : public FmXFormShell_BD_BASE
{
    using ::com::sun::star::beans::XPropertyChangeListener::disposing;
protected:
    FmXFormShell_Base_Disambiguation( ::osl::Mutex& _rMutex );
    virtual void SAL_CALL disposing();
};

//========================================================================
namespace svx
{
    class FmTextControlShell;
}

//========================================================================
typedef FmXFormShell_Base_Disambiguation    FmXFormShell_BASE;
typedef ::utl::ConfigItem                   FmXFormShell_CFGBASE;

struct SdrViewEvent;
class FmFormShell;
class FmFormView;
class FmFormObj;
class SAL_DLLPRIVATE FmXFormShell   :public FmXFormShell_BASE
                                    ,public FmXFormShell_CFGBASE
                                    ,public ::svxform::OStaticDataAccessTools
                                    ,public ::svx::IControllerFeatureInvalidation
{
    friend class FmFormView;
    friend class FmXFormView;

    class SuspendPropertyTracking;
    friend class SuspendPropertyTracking;

    // Timer um verzoegerte Markierung vorzunehmen
    Timer               m_aMarkTimer;
    SdrObjArray         m_arrSearchedControls;
        // We enable a permanent cursor for the grid we found a searched text, it's disabled in the next "found" event.
    FmFormArray         m_aSearchForms;

    struct InvalidSlotInfo {
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

    ::com::sun::star::form::NavigationBarMode   m_eNavigate;                // Art der Navigation

        // da ich beim Suchen fuer die Behandlung des "gefunden" ein SdrObject markieren will, besorge ich mir vor dem
        // Hochreissen des Suchen-Dialoges alle relevanten Objekte
        // (das Array ist damit auch nur waehrend des Suchvorganges gueltig)
    std::vector<long> m_arrRelativeGridColumn;

    ::osl::Mutex    m_aMutex;
    sal_uLong           m_nInvalidationEvent;
    sal_uLong           m_nActivationEvent;
    ::std::queue< FmLoadAction >
                    m_aLoadingPages;

    FmFormShell*                m_pShell;
    ::svx::FmTextControlShell*  m_pTextShell;

    ::svx::ControllerFeatures   m_aActiveControllerFeatures;
    ::svx::ControllerFeatures   m_aNavControllerFeatures;

    // aktuelle Form, Controller
    // nur im alive mode verfuegbar
    ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >    m_xActiveController;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >    m_xNavigationController;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >                       m_xActiveForm;

    // Aktueller container einer Page
    // nur im designmode verfuegbar
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess> m_xForms;

    // the currently selected objects, as to be displayed in the property browser
    InterfaceBag                                                                m_aCurrentSelection;
    /// the currently selected form, or the form which all currently selected controls belong to, or <NULL/>
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >           m_xCurrentForm;
    /// the last selection/marking of controls only. Necessary to implement the "Control properties" slot
    InterfaceBag                                                                m_aLastKnownMarkedControls;


        // und das ist ebenfalls fuer's 'gefunden' : Beim Finden in GridControls brauche ich die Spalte, bekomme aber
        // nur die Nummer des Feldes, die entspricht der Nummer der Spalte + <offset>, wobei der Offset von der Position
        // des GridControls im Formular abhaengt. Also hier eine Umrechnung.
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>         m_xLastGridFound;
     // the frame we live in
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>              m_xAttachedFrame;
    // Administration of external form views (see the SID_FM_VIEW_AS_GRID-slot)
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >                m_xExternalViewController;      // the controller for the external form view
    ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >    m_xExtViewTriggerController;    // the nav controller at the time the external display was triggered
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >                  m_xExternalDisplayedForm;       // the form which the external view is based on

    mutable ::svxform::DocumentType
                    m_eDocumentType;        /// the type of document we're living in
    sal_Int16       m_nLockSlotInvalidation;
    sal_Bool        m_bHadPropertyBrowserInDesignMode : 1;

    sal_Bool        m_bTrackProperties  : 1;
        // soll ich (bzw. der Owner diese Impl-Klasse) mich um die Aktualisierung des ::com::sun::star::beans::Property-Browsers kuemmern ?

    sal_Bool        m_bUseWizards : 1;

    sal_Bool        m_bDatabaseBar      : 1;    // Gibt es eine Datenbankleiste
    sal_Bool        m_bInActivate       : 1;    // Wird ein Controller aktiviert
    sal_Bool        m_bSetFocus         : 1;    // Darf der Focus umgesetzt werden
    sal_Bool        m_bFilterMode       : 1;    // Wird gerade ein Filter auf die Controls angesetzt
    sal_Bool        m_bChangingDesignMode:1;    // sal_True within SetDesignMode
    sal_Bool        m_bPreparedClose    : 1;    // for the current modification state of the current form
                                                //  PrepareClose had been called and the user denied to save changes
    sal_Bool        m_bFirstActivation  : 1;    // has the shell ever been activated?

public:
    // attribute access
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&
                getHostFrame() const { return m_xAttachedFrame; }
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >&
                getExternallyDisplayedForm() const { return m_xExternalDisplayedForm; }

    inline sal_Bool
                didPrepareClose() const { return m_bPreparedClose; }
    inline void
                didPrepareClose( sal_Bool _bDid ) { m_bPreparedClose = _bDid; }

public:
    FmXFormShell(FmFormShell& _rShell, SfxViewFrame* _pViewFrame);

    // UNO Anbindung
    DECLARE_UNO3_DEFAULTS(FmXFormShell, FmXFormShell_BASE);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& type) throw ( ::com::sun::star::uno::RuntimeException );

protected:
    ~FmXFormShell();

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

// EventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// XSelectionChangeListener
    virtual void SAL_CALL selectionChanged(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException );

// ::com::sun::star::form::XFormControllerListener
    virtual void SAL_CALL formActivated(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL formDeactivated(const ::com::sun::star::lang::EventObject& rEvent) throw( ::com::sun::star::uno::RuntimeException );

// OComponentHelper
    virtual void SAL_CALL disposing();

public:
    void EnableTrackProperties( sal_Bool bEnable) { m_bTrackProperties = bEnable; }
    sal_Bool IsTrackPropertiesEnabled() {return m_bTrackProperties;}

    // activation handling
            void        viewActivated( FmFormView& _rCurrentView, sal_Bool _bSyncAction = sal_False );
            void        viewDeactivated( FmFormView& _rCurrentView, sal_Bool _bDeactivateController = sal_True );

    // IControllerFeatureInvalidation
    virtual void invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures );

    void ExecuteTabOrderDialog(         // execute SID_FM_TAB_DIALOG
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >& _rxForForm
    );

    // stuff
    void AddElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
    void RemoveElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

    /** updates m_xForms, to be either <NULL/>, if we're in alive mode, or our current page's forms collection,
        if in design mode
    */
    void UpdateForms( sal_Bool _bInvalidate );

    void ExecuteSearch();               // execute SID_FM_SEARCH
    void CreateExternalView();          // execute SID_FM_VIEW_AS_GRID

    sal_Bool    GetY2KState(sal_uInt16& n);
    void        SetY2KState(sal_uInt16 n);

protected:
    // activation handling
    inline  sal_Bool    hasEverBeenActivated( ) const { return !m_bFirstActivation; }
    inline  void        setHasBeenActivated( ) { m_bFirstActivation = sal_False; }

    // form handling
    /// load or unload the forms on a page
            void        loadForms( FmFormPage* _pPage, const sal_uInt16 _nBehaviour = FORMS_LOAD | FORMS_SYNC );
            void        smartControlReset( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxModels );


    void startListening();
    void stopListening();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
        impl_getControl(
            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& i_rxModel,
            const FmFormObj& i_rKnownFormObj
        );

    // sammelt in strNames die Namen aller Formulare
    static void impl_collectFormSearchContexts_nothrow(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxStartingPoint,
        const ::rtl::OUString& _rCurrentLevelPrefix,
        FmFormArray& _out_rForms,
        ::std::vector< String >& _out_rNames );

    /** checks whether the instance is already disposed, if so, this is reported as assertion error (debug
        builds only) and <TRUE/> is returned.
    */
    bool    impl_checkDisposed() const;

public:
    // methode fuer nicht designmode (alive mode)
    void setActiveController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController>& _xController, sal_Bool _bNoSaveOldContent = sal_False );
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController>& getActiveController() const {return m_xActiveController;}
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController>& getActiveInternalController() const { return m_xActiveController == m_xExternalViewController ? m_xExtViewTriggerController : m_xActiveController; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& getActiveForm() const {return m_xActiveForm;}
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController>& getNavController() const {return m_xNavigationController;}

    inline const ::svx::ControllerFeatures& getActiveControllerFeatures() const
        { return m_aActiveControllerFeatures; }
    inline const ::svx::ControllerFeatures& getNavControllerFeatures() const
        { return m_aNavControllerFeatures.isAssigned() ? m_aNavControllerFeatures : m_aActiveControllerFeatures; }

    /** announces a new "current selection"
        @return
            <TRUE/> if and only if the to-bet-set selection was different from the previous selection
    */
    bool    setCurrentSelection( const InterfaceBag& _rSelection );

    /** sets the new selection to the last known marked controls
    */
    bool    selectLastMarkedControls();

    /** retrieves the current selection
    */
    SVX_DLLPUBLIC void    getCurrentSelection( InterfaceBag& /* [out] */ _rSelection ) const;

    /** sets a new current selection as indicated by a mark list
        @return
            <TRUE/> if and only if the to-bet-set selection was different from the previous selection
    */
    bool    setCurrentSelectionFromMark(const SdrMarkList& rMarkList);

    /// returns the currently selected form, or the form which all currently selected controls belong to, or <NULL/>
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >
                getCurrentForm() const { return m_xCurrentForm; }
    void        forgetCurrentForm();
    /// returns whether the last known marking contained only controls
    sal_Bool    onlyControlsAreMarked() const { return !m_aLastKnownMarkedControls.empty(); }

    /// determines whether the current selection consists of exactly the given object
    bool    isSolelySelected(
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject
            );

    /// handles a MouseButtonDown event of the FmFormView
    void handleMouseButtonDown( const SdrViewEvent& _rViewEvent );
    /// handles the request for showing the "Properties"
    void handleShowPropertiesRequest();

    sal_Bool hasForms() const {return m_xForms.is() && m_xForms->getCount() != 0;}
    sal_Bool hasDatabaseBar() const {return m_bDatabaseBar;}
    sal_Bool canNavigate() const    {return m_xNavigationController.is();}

    void ShowSelectionProperties( sal_Bool bShow );
    sal_Bool IsPropBrwOpen() const;

    void DetermineSelection(const SdrMarkList& rMarkList);
    void SetSelection(const SdrMarkList& rMarkList);
    void SetSelectionDelayed();

    void SetDesignMode(sal_Bool bDesign);

    sal_Bool    GetWizardUsing() const { return m_bUseWizards; }
    void        SetWizardUsing(sal_Bool _bUseThem);

        // Setzen des Filtermodus
    sal_Bool isInFilterMode() const {return m_bFilterMode;}
    void startFiltering();
    void stopFiltering(sal_Bool bSave);

    static PopupMenu* GetConversionMenu();
        // ein Menue, das alle ControlConversion-Eintraege enthaelt

    /// checks whethere a given control conversion slot can be applied to the current selection
           bool canConvertCurrentSelectionToControl( sal_Int16 nConversionSlot );
    /// enables or disables all conversion slots in a menu, according to the current selection
           void checkControlConversionSlotsForCurrentSelection( Menu& rMenu );
    /// executes a control conversion slot for a given object
           bool executeControlConversionSlot( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& _rxObject, sal_uInt16 _nSlotId );
    /** executes a control conversion slot for the current selection
        @precond canConvertCurrentSelectionToControl( <arg>_nSlotId</arg> ) must return <TRUE/>
    */
           bool executeControlConversionSlot( sal_uInt16 _nSlotId );
    /// checks whether the given slot id denotes a control conversion slot
    static bool isControlConversionSlot( sal_uInt16 _nSlotId );

    void    ExecuteTextAttribute( SfxRequest& _rReq );
    void    GetTextAttributeState( SfxItemSet& _rSet );
    bool    IsActiveControl( bool _bCountRichTextOnly = false ) const;
    void    ForgetActiveControl();
    void    SetControlActivationHandler( const Link& _rHdl );

    /// classifies our host document
    ::svxform::DocumentType
            getDocumentType() const;
    bool    isEnhancedForm() const;

    /// determines whether our host document is currently read-only
    bool    IsReadonlyDoc() const;

    // das Setzen des curObject/selObject/curForm erfolgt verzoegert (SetSelectionDelayed), mit den folgenden
    // Funktionen laesst sich das abfragen/erzwingen
    inline sal_Bool IsSelectionUpdatePending();
    void            ForceUpdateSelection(sal_Bool bLockInvalidation);

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>          getContextDocument() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>            getInternalForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& _xForm) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>       getInternalForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm) const;
        // if the form belongs to the controller (extern) displaying a grid, the according internal form will
        // be displayed, _xForm else

    // check if the current control of the active controler has the focus
    bool    HasControlFocus() const;

private:
    DECL_LINK(OnFoundData, FmFoundRecordInformation*);
    DECL_LINK(OnCanceledNotFound, FmFoundRecordInformation*);
    DECL_LINK(OnSearchContextRequest, FmSearchContext*);
    DECL_LINK(OnTimeOut, void*);
    DECL_LINK(OnFirstTimeActivation, void*);
    DECL_LINK(OnFormsCreated, FmFormPage*);

    void LoopGrids(sal_Int16 nWhat);

    // Invalidierung von Slots
    void    InvalidateSlot( sal_Int16 nId, sal_Bool bWithId );
    void    UpdateSlot( sal_Int16 nId );
    // Locking der Invalidierung - wenn der interne Locking-Counter auf 0 geht, werden alle aufgelaufenen Slots
    // (asynchron) invalidiert
    void    LockSlotInvalidation(sal_Bool bLock);

    DECL_LINK(OnInvalidateSlots, void*);

    void    CloseExternalFormViewer();
        // closes the task-local beamer displaying a grid view for a form

    // ConfigItem related stuff
    virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames);
    virtual void Commit();
    void implAdjustConfigCache();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
            getControlContainerForView();

    /** finds and sets a default for m_xCurrentForm, if it is currently NULL
    */
    void    impl_defaultCurrentForm_nothrow();

    /** sets m_xCurrentForm to the provided form, and udpates everything which
        depends on the current form
    */
    void    impl_updateCurrentForm( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxNewCurForm );

    /** adds or removes ourself as XEventListener at m_xActiveController
    */
    void    impl_switchActiveControllerListening( const bool _bListen );

    /** add an element
    */
    void    impl_AddElement_nothrow(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

    /** remove an element
    */
    void    impl_RemoveElement_nothrow(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

    // ---------------------------------------------------
    // asyncronous cursor actions/navigation slot handling

public:
    /** execute the given form slot
        <p>Warning. Only a small set of slots implemented currently.</p>
        @param _nSlot
            the slot to execute
    */
    void    ExecuteFormSlot( sal_Int32 _nSlot );

    /** determines whether the current form slot is currently enabled
    */
    bool    IsFormSlotEnabled( sal_Int32 _nSlot, ::com::sun::star::form::runtime::FeatureState* _pCompleteState = NULL );

protected:
    DECL_LINK( OnLoadForms, FmFormPage* );
};

//------------------------------------------------------------------------------
inline sal_Bool FmXFormShell::IsSelectionUpdatePending()
{
    return m_aMarkTimer.IsActive();
}

// ========================================================================
// = ein Iterator, der ausgehend von einem Interface ein Objekt sucht, dessen
// = ::com::sun::star::beans::Property-Set eine ControlSource- sowie eine BoundField-Eigenschaft hat,
// = wobei letztere einen Wert ungleich NULL haben muss.
// = Wenn das Interface selber diese Bedingung nicht erfuellt, wird getestet,
// = ob es ein Container ist (also ueber eine ::com::sun::star::container::XIndexAccess verfuegt), dann
// = wird dort abgestiegen und fuer jedes Element des Containers das selbe
// = versucht (wiederum eventuell mit Abstieg).
// = Wenn irgendein Objekt dabei die geforderte Eigenschaft hat, entfaellt
// = der Teil mit dem Container-Test fuer dieses Objekt.
// =

class SAL_DLLPRIVATE SearchableControlIterator : public ::comphelper::IndexAccessIterator
{
    ::rtl::OUString         m_sCurrentValue;
        // der aktuelle Wert der ControlSource-::com::sun::star::beans::Property

public:
    ::rtl::OUString     getCurrentValue() const { return m_sCurrentValue; }

public:
    SearchableControlIterator(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface> xStartingPoint);

    virtual sal_Bool ShouldHandleElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& rElement);
    virtual sal_Bool ShouldStepInto(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xContainer) const;
    virtual void Invalidate() { IndexAccessIterator::Invalidate(); m_sCurrentValue = ::rtl::OUString(); }
};

// ========================================================================
typedef boost::ptr_vector<SfxStatusForwarder> StatusForwarderArray;
class SVX_DLLPUBLIC ControlConversionMenuController : public SfxMenuControl
{
protected:
    StatusForwarderArray    m_aStatusForwarders;
    Menu*                   m_pMainMenu;
    PopupMenu*              m_pConversionMenu;

public:
    SVX_DLLPRIVATE ControlConversionMenuController(sal_uInt16 nId, Menu& rMenu, SfxBindings& rBindings);
    SVX_DLLPRIVATE virtual ~ControlConversionMenuController();
    SFX_DECL_MENU_CONTROL();

    SVX_DLLPRIVATE virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
};

#endif          // _SVX_FMSHIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
