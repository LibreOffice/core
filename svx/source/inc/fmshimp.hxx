/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmshimp.hxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-29 08:49:55 $
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
#ifndef _SVX_FMSHIMP_HXX
#define _SVX_FMSHIMP_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_CONTAINEREVENT_HPP_
#include <com/sun/star/container/ContainerEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_NAVIGATIONBARMODE_HPP_
#include <com/sun/star/form/NavigationBarMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif

#ifndef _SVDMARK_HXX
#include <svx/svdmark.hxx>
#endif
#ifndef _FMSEARCH_HXX
#include <svx/fmsearch.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_LONGS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#ifndef _SFXMNUITEM_HXX //autogen
#include <sfx2/mnuitem.hxx>
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _FMSRCCF_HXX_
#include "fmsrccfg.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_THREAD_HXX_ //autogen
#include <vos/thread.hxx>
#endif
#ifndef _SFXCANCEL_HXX //autogen
#include <svtools/cancel.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_WARNING
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE4_HXX_
#include <cppuhelper/compbase4.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE6_HXX_
#include <cppuhelper/compbase6.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif
#ifndef SVX_FORMCONTROLLING_HXX
#include "formcontrolling.hxx"
#endif
#ifndef SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX
#include "fmdocumentclassification.hxx"
#endif

#include <queue>
#include <set>
#include <vector>

SV_DECL_PTRARR(SdrObjArray, SdrObject*, 32, 16)
//  SV_DECL_OBJARR(FmFormArray, ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>, 32, 16);
DECLARE_STL_VECTOR( ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > ,FmFormArray);

// catch databse exceptions if occur
#define DO_SAFE(statement) try { statement; } catch( const Exception& ) { DBG_ERROR("unhandled exception (I tried to move a cursor (or something like that).)"); }

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

//========================================================================
// I would prefer this to be a struct local to FmXFormShell but unfortunately local structs/classes
// are somewhat difficult with some of our compilers
class FmCursorActionThread;
struct SAL_DLLPRIVATE CursorActionDescription
{
    FmCursorActionThread*   pThread;
    ULONG                       nFinishedEvent;
        // we want to do the cleanup of the thread in the main thread so we post an event to ourself
    sal_Bool                    bCanceling;
        // this thread is being canceled

    CursorActionDescription() : pThread(NULL), nFinishedEvent(0), bCanceling(sal_False) { }
};

class FmFormPage;
//========================================================================
struct SAL_DLLPRIVATE FmLoadAction
{
    FmFormPage* pPage;
    ULONG       nEventId;
    sal_uInt16  nFlags;

    FmLoadAction( ) : pPage( NULL ), nEventId( 0 ), nFlags( 0 ) { }
    FmLoadAction( FmFormPage* _pPage, sal_uInt16 _nFlags, ULONG _nEventId )
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
class SAL_DLLPRIVATE FmXFormShell   :public FmXFormShell_BASE
                                    ,public FmXFormShell_CFGBASE
                                    ,public ::svxform::OStaticDataAccessTools
                                    ,public ::svx::IControllerFeatureInvalidation
{
    friend class FmFormView;
    friend class FmXFormView;
    friend class WizardUsageConfigItem;

    class SuspendPropertyTracking;
    friend class SuspendPropertyTracking;

    // Timer um verzoegerte Markierung vorzunehmen
    Timer               m_aMarkTimer;
    SdrObjArray         m_arrSearchedControls;
        // We enable a permanent cursor for the grid we found a searched text, it's disabled in the next "found" event.
    FmFormArray         m_aSearchForms;

    SvUShorts   m_arrInvalidSlots;
    SvBytes     m_arrInvalidSlots_Flags;
        // we explicitly switch off the propbrw before leaving the design mode
        // this flag tells us if we have to switch it on again when reentering

    typedef ::std::map<
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >,
            CursorActionDescription,
            ::comphelper::OInterfaceCompare< ::com::sun::star::sdbc::XResultSet >
        >   CursorActions;
    CursorActions   m_aCursorActions;
        // all actions on async cursors

    ::std::vector< sal_Bool >
                    m_aControlLocks;
        // while doing a async cursor action we have to lock all controls of the active controller.
        // m_aControlLocks remembers the previous lock states to be restored afterwards.
    ::osl::Mutex    m_aAsyncSafety;
        // secure the access to our thread related members
    ::osl::Mutex    m_aInvalidationSafety;
        // secure the access to all our slot invalidation related members

    ::com::sun::star::form::NavigationBarMode   m_eNavigate;                // Art der Navigation

        // da ich beim Suchen fuer die Behandlung des "gefunden" ein SdrObject markieren will, besorge ich mir vor dem
        // Hochreissen des Suchen-Dialoges alle relevanten Objekte
        // (das Array ist damit auch nur waehrend des Suchvorganges gueltig)
    SvLongs             m_arrRelativeGridColumn;

    ::osl::Mutex    m_aMutex;
    ULONG           m_nInvalidationEvent;
    ULONG           m_nActivationEvent;
    ::std::queue< FmLoadAction >
                    m_aLoadingPages;

    FmFormShell*                m_pShell;
    ::svx::FmTextControlShell*  m_pTextShell;

    ::svx::ControllerFeatures   m_aActiveControllerFeatures;
    ::svx::ControllerFeatures   m_aNavControllerFeatures;

    // aktuelle Form, Controller
    // nur im alive mode verfuegbar
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>  m_xActiveController;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>  m_xNavigationController;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>            m_xActiveForm;

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
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>         m_xExternalViewController;      // the controller for the external form view
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>      m_xExtViewTriggerController;    // the nav controller at the time the external display was triggered
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>           m_xExternalDisplayedForm;       // the form which the external view is based on

    FmXDispatchInterceptorImpl*     m_pExternalViewInterceptor;

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
    FmXFormShell(FmFormShell* _pShell, SfxViewFrame* _pViewFrame);

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
            void        viewActivated( FmFormView* _pCurrentView, sal_Bool _bSyncAction = sal_False );
            void        viewDeactivated( FmFormView* _pCurrentView, sal_Bool _bDeactivateController = sal_True );

    // IControllerFeatureInvalidation
    virtual void invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures );

    void ExecuteTabOrderDialog(         // execute SID_FM_TAB_DIALOG
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabControllerModel >& _rxForForm
    );

    // stuff
    void ResetForms(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xForms = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>(), sal_Bool bInvalidate = sal_False);
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

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl>  GetControlFromModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel>& xModel);
        // liefert das Control, welches das angegebene Model hat

    // sammelt in strNames die Namen aller Formulare
    static void impl_collectFormSearchContexts_nothrow(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _rxStartingPoint,
        const ::rtl::OUString& _rCurrentLevelPrefix,
        FmFormArray& _out_rForms,
        ::std::vector< String >& _out_rNames );

public:
    // methode fuer nicht designmode (alive mode)
    void setActiveController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& _xController, sal_Bool _bNoSaveOldContent = sal_False );
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& getActiveController() const {return m_xActiveController;}
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& getActiveInternalController() const { return m_xActiveController == m_xExternalViewController ? m_xExtViewTriggerController : m_xActiveController; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm>& getActiveForm() const {return m_xActiveForm;}
    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& getNavController() const {return m_xNavigationController;}

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
    void    getCurrentSelection( InterfaceBag& /* [out] */ _rSelection ) const;

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
    void clearFilter();

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

private:
    DECL_LINK(OnFoundData, FmFoundRecordInformation*);
    DECL_LINK(OnCanceledNotFound, FmFoundRecordInformation*);
    DECL_LINK(OnSearchContextRequest, FmSearchContext*);
    DECL_LINK(OnTimeOut, void*);
    DECL_LINK(OnFirstTimeActivation, void*);

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
    void implAdjustConfigCache();

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
            getControlContainerForView();

    // ---------------------------------------------------
    // asyncronous cursor actions/navigation slot handling

    void setControlLocks();     // lock all controls of the active controller
    void restoreControlLocks(); // restore the lock state of all controls of the active controller

public:
    enum CURSOR_ACTION { CA_MOVE_TO_LAST, CA_MOVE_ABSOLUTE };
    void DoAsyncCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& _xController, CURSOR_ACTION _eWhat);
    void DoAsyncCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm, CURSOR_ACTION _eWhat);

    sal_Bool HasAnyPendingCursorAction() const;
    void CancelAnyPendingCursorAction();

    sal_Bool HasPendingCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController>& _xController) const;
    sal_Bool HasPendingCursorAction(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xForm) const;

    /** execute the given form slot
        <p>Warning. Only a small set of slots implemented currently.</p>
        @param _nSlot
            the slot to execute
        @param _rxForm
            the form for which the slot is to be executed
        @param _rxController
            the controller to use for committing modified controls. Will not
            be used if <NULL/>
    */
    void    ExecuteFormSlot(
                sal_Int32 _nSlot,
                const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
                const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >& _rxController
            );

protected:
    DECL_LINK(OnCursorActionDone, FmCursorActionThread*);
    DECL_LINK(OnCursorActionDoneMainThread, FmCursorActionThread*);

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
SV_DECL_PTRARR_DEL(StatusForwarderArray, SfxStatusForwarder*, 16, 0)
class SAL_DLLPRIVATE ControlConversionMenuController : public SfxMenuControl
{
protected:
    StatusForwarderArray    m_aStatusForwarders;
    Menu*                   m_pMainMenu;
    PopupMenu*              m_pConversionMenu;

public:
    ControlConversionMenuController(sal_uInt16 nId, Menu& rMenu, SfxBindings& rBindings);
    virtual ~ControlConversionMenuController();
    SFX_DECL_MENU_CONTROL();

    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState);
};

//==================================================================
// FmCursorActionThread
//==================================================================

class SAL_DLLPRIVATE FmCursorActionThread : public ::vos::OThread
{
    Link                    m_aTerminationHandler;      // the handler to be called upon termination
    ::com::sun::star::sdbc::SQLException            m_aRunException;            // the database exception thrown by RunImpl
    ::osl::Mutex    m_aAccessSafety;            // for securing the multi-thread access
    ::osl::Mutex    m_aFinalExitControl;        // see StopItWait

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>           m_xDataSource;              // the cursor which we work with

private:

    UniString                   m_sStopperCaption;          // the caption for the ThreadStopper
    sal_Bool                    m_bCanceled:1;              // StopIt has been called ?
    sal_Bool                    m_bDeleteMyself:1;          // delete the thread upon termination (defaults to sal_False) ?
    sal_Bool                    m_bDisposeCursor:1;         // dispose the cursor upon termination (defaults to sal_False) ?
    sal_Bool                    m_bTerminated:1;                // onTerminated already called ?
    sal_Bool                    m_bRunFailed:1;             // a database execption occured in RunImpl ?

    // a ThreadStopper will be instantiated so that the open can be canceled via the UI
    class ThreadStopper : protected SfxCancellable
    {
        FmCursorActionThread*   m_pOwner;

        virtual ~ThreadStopper() { }

    public:
        ThreadStopper(FmCursorActionThread* pOwner, const UniString& rTitle);

        virtual void    Cancel();

        virtual void OwnerTerminated();
        // Normally the Owner (a FmCursorActionThread) would delete the stopper when terminated.
        // Unfortunally the application doesn't remove the 'red light' when a SfxCancellable is deleted
        // if it (the app) can't acquire the solar mutex. The deletion is IGNORED then. So we have make
        // sure that a) the stopper is deleted from inside the main thread (where the solar mutex is locked)
        // and b) that in the time between the termination of the thread and the deletion of the stopper
        // the latter doesn't access the former.
        // The OwnerTerminated cares for both aspects.
        // SO DON'T DELETE THE STOPPER EXPLICITLY !

    protected:
        // HACK HACK HACK HACK HACK : this should be private, but MSVC doesn't accept the LINK-macro then ....
        DECL_LINK(OnDeleteInMainThread, ThreadStopper*);
    };
    friend class FmCursorActionThread::ThreadStopper;


public:
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> getDataSource() const { return m_xDataSource; }

private:
    sal_Bool Terminated() { ::osl::MutexGuard aGuard(m_aAccessSafety); return m_bTerminated; }

public:
    FmCursorActionThread(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDataSource, const UniString& _rStopperCaption);
    virtual ~FmCursorActionThread() {}

    // control of self-deletion
    sal_Bool                IsSelfDeleteEnabled()               { ::osl::MutexGuard aGuard(m_aAccessSafety); return m_bDeleteMyself; }
    void                EnableSelfDelete(sal_Bool bEnable)      { ::osl::MutexGuard aGuard(m_aAccessSafety); m_bDeleteMyself = bEnable; }

    // control of cursor-dipose
    sal_Bool                IsCursorDisposeEnabled()            { ::osl::MutexGuard aGuard(m_aAccessSafety); return m_bDisposeCursor; }
    void                EnableCursorDispose(sal_Bool bEnable)   { ::osl::MutexGuard aGuard(m_aAccessSafety); m_bDisposeCursor = bEnable; }

    // error-access
    sal_Bool                RunFailed()                         { ::osl::MutexGuard aGuard(m_aAccessSafety); return m_bRunFailed; }
    ::com::sun::star::sdbc::SQLException        GetRunException()                   { ::osl::MutexGuard aGuard(m_aAccessSafety); return m_aRunException; }

    /// the excution (within the method "run") was canceled ?
    sal_Bool                WasCanceled()                       { ::osl::MutexGuard aGuard(m_aAccessSafety); return m_bCanceled; }

    /// the handler will be called synchronously (the parameter is a pointer to the thread)
    void                SetTerminationHdl(const Link& aTermHdl) { ::osl::MutexGuard aGuard(m_aAccessSafety); m_aTerminationHandler = aTermHdl; }

    /// cancels the process. returns to the caller immediately. to be called from another thread (of course ;)
    void                StopIt();

    /// cancels the process. does not return to the caller until the thread is terminated.
    void                StopItWait();

protected:
    virtual void SAL_CALL run();
    virtual void SAL_CALL onTerminated();

    /// called from within run. run itself handles (de)initialisation of the cancel handling.
    virtual void RunImpl() = 0;
};

//------------------------------------------------------------------------------

#define DECL_CURSOR_ACTION_THREAD(classname)            \
                                                        \
class SAL_DLLPRIVATE classname : public FmCursorActionThread    \
{                                                       \
public:                                                 \
    classname(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDataSource); \
protected:                                              \
    virtual void RunImpl();                             \
};                                                      \


//------------------------------------------------------------------------------

#define IMPL_CURSOR_ACTION_THREAD(classname, caption, action)   \
                                                                \
classname::classname(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>& _xDataSource) \
    :FmCursorActionThread(_xDataSource, caption)                \
{                                                               \
}                                                               \
                                                                \
void classname::RunImpl()                                       \
{                                                               \
    m_xDataSource->action;                                      \
}                                                               \



#endif          // _SVX_FMSHIMP_HXX
