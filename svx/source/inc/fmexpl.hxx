/*************************************************************************
 *
 *  $RCSfile: fmexpl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:19 $
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
#ifndef _SVX_FMEXPL_HXX
#define _SVX_FMEXPL_HXX

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFXBRDCST_HXX //autogen
#include <svtools/brdcst.hxx>
#endif

#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif

#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif


#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_DOCKWIN_HXX //autogen
#include <vcl/dockwin.hxx>
#endif

#ifndef _SVX_FMVIEW_HXX
#include "fmview.hxx"
#endif

#ifndef _SVX_FMTOOLS_HXX
#include <fmtools.hxx>
#endif

class SdrObjListIter;
//========================================================================
class FmEntryData;
class FmExplInsertedHint : public SfxHint
{
    FmEntryData* pEntryData;
    sal_uInt32 nPos;

public:
    TYPEINFO();
    FmExplInsertedHint( FmEntryData* pInsertedEntryData, sal_uInt32 nRelPos );
    virtual ~FmExplInsertedHint();

    FmEntryData* GetEntryData() const { return pEntryData; }
    sal_uInt32 GetRelPos() const { return nPos; }
};

//========================================================================
class FmExplModelReplacedHint : public SfxHint
{
    FmEntryData* pEntryData;    // die Daten des Eintrages, der ein neues Model bekommen hat

public:
    TYPEINFO();
    FmExplModelReplacedHint( FmEntryData* pAffectedEntryData );
    virtual ~FmExplModelReplacedHint();

    FmEntryData* GetEntryData() const { return pEntryData; }
};

//========================================================================
class FmExplRemovedHint : public SfxHint
{
    FmEntryData* pEntryData;

public:
    TYPEINFO();
    FmExplRemovedHint( FmEntryData* pInsertedEntryData );
    virtual ~FmExplRemovedHint();

    FmEntryData* GetEntryData() const { return pEntryData; }
};

//========================================================================
class FmExplNameChangedHint : public SfxHint
{
    FmEntryData*    pEntryData;
    ::rtl::OUString          aNewName;

public:
    TYPEINFO();
    FmExplNameChangedHint( FmEntryData* pData, const ::rtl::OUString& rNewName );
    virtual ~FmExplNameChangedHint();

    FmEntryData*    GetEntryData() const { return pEntryData; }
    ::rtl::OUString          GetNewName() const { return aNewName; }
};

//========================================================================
class FmExplClearedHint : public SfxHint
{
public:
    TYPEINFO();
    FmExplClearedHint();
    virtual ~FmExplClearedHint();
};

//========================================================================
class FmExplViewMarksChanged : public SfxHint
{
    FmFormView* pView;
public:
    TYPEINFO();
    FmExplViewMarksChanged(FmFormView* pWhichView) { pView = pWhichView; }
    virtual ~FmExplViewMarksChanged() {}

    FmFormView* GetAffectedView() { return pView; }
};

//========================================================================
class FmEntryDataList;
class FmEntryData
{
protected:
    Image           aCollapsedImage;
    Image           aExpandedImage;
    ::rtl::OUString          aText;

    FmEntryDataList*    pChildList;
    FmEntryData*        pParent;

public:
    TYPEINFO();

    FmEntryData( FmEntryData* pParentData );
    FmEntryData( const FmEntryData& rEntryData );
    virtual ~FmEntryData();

    void    Clear();
    void    SetText( const ::rtl::OUString& rText ){ aText = rText; }
    void    SetParent( FmEntryData* pParentData ){ pParent = pParentData; }

    Image           GetCollapsedImage() const { return aCollapsedImage; }
    Image           GetExpandedImage() const { return aExpandedImage; }
    ::rtl::OUString          GetText() const { return aText; }
    FmEntryData*    GetParent() const { return pParent; }
    FmEntryDataList* GetChildList() const { return pChildList; }

    virtual sal_Bool IsEqualWithoutChilds( FmEntryData* pEntryData );
    virtual FmEntryData* Clone() = 0;

    virtual const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& GetElement() const = 0;
};

//========================================================================
DECLARE_LIST( FmEntryDataBaseList, FmEntryData* );

class FmEntryDataList : public FmEntryDataBaseList
{
public:
    FmEntryDataList();
    virtual ~FmEntryDataList();
};

//========================================================================
// FmExplRequestSelectHint - jemand teilt dem FmExplorer mit, dass er bestimmte Eintraege selektieren soll

typedef FmEntryData* FmEntryDataPtr;
SV_DECL_PTRARR_SORT( FmEntryDataArray, FmEntryDataPtr, 16, 16 )

class FmExplRequestSelectHint : public SfxHint
{
    FmEntryDataArray    m_arredToSelect;
    sal_Bool                m_bMixedSelection;
public:
    TYPEINFO();
    FmExplRequestSelectHint() { }
    virtual ~FmExplRequestSelectHint() {}

    void SetMixedSelection(sal_Bool bMixedSelection) { m_bMixedSelection = bMixedSelection; }
    sal_Bool IsMixedSelection() { return m_bMixedSelection; }
    void AddItem(FmEntryData* pEntry) { m_arredToSelect.Insert(pEntry); }
    void ClearItems() { m_arredToSelect.Remove(sal_uInt16(0), m_arredToSelect.Count()); }
    FmEntryDataArray& GetItems() { return m_arredToSelect; }
};

//========================================================================
class FmFormData : public FmEntryData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  m_xForm;

public:
    TYPEINFO();

    FmFormData( ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >  xForm, const ImageList& ilNavigatorImages, FmFormData* pParent=NULL );
    FmFormData( const FmFormData& rFormData );
    virtual ~FmFormData();

    void SetForm( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& xForm ){ m_xForm = xForm; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& GetFormIface() const { return m_xForm; }
    virtual sal_Bool IsEqualWithoutChilds( FmEntryData* pEntryData );
    virtual FmEntryData* Clone();

    virtual const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& GetElement() const { return m_xForm;}
};


//========================================================================
class FmControlData : public FmEntryData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  m_xFormComponent;

    Image GetImage(const ImageList& ilNavigatorImages) const;

public:
    TYPEINFO();

    FmControlData( ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xFormComponent, const ImageList& ilNavigatorImages, FmFormData* pParent );
    FmControlData( const FmControlData& rControlData );
    virtual ~FmControlData();

    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& GetFormComponent() const { return m_xFormComponent; }
    virtual sal_Bool IsEqualWithoutChilds( FmEntryData* pEntryData );
    virtual FmEntryData* Clone();
    virtual const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& GetElement() const { return m_xFormComponent;}

    void ModelReplaced(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& xNew, const ImageList& ilNavigatorImages);
};

//========================================================================
class FmExplorerModel;
class FmXExplPropertyChangeList : public ::cppu::WeakImplHelper2< ::com::sun::star::beans::XPropertyChangeListener,
                                  ::com::sun::star::container::XContainerListener>
{
    FmExplorerModel* m_pExplModel;
    sal_uInt32 m_nLocks;
    sal_Bool   m_bCanUndo;

public:
    FmXExplPropertyChangeList(FmExplorerModel* pModel);

    // UNO-Anbindung
    //  DECLARE_UNO3_AGG_DEFAULTS( FmXExplPropertyChangeList, UsrObject );
    //  virtual sal_Bool queryInterface(::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rOut);

// XEventListenerListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

// ::com::sun::star::container::XContainerListener

    virtual void SAL_CALL elementInserted(const  ::com::sun::star::container::ContainerEvent& rEvent);
    virtual void SAL_CALL elementReplaced(const  ::com::sun::star::container::ContainerEvent& rEvent);
    virtual void SAL_CALL elementRemoved(const  ::com::sun::star::container::ContainerEvent& rEvent);

    void Lock() { m_nLocks++; }
    void UnLock() { m_nLocks--; }
    sal_Bool IsLocked() const { return m_nLocks != 0; }
    sal_Bool CanUndo() const { return m_bCanUndo; }
    void ReleaseModel() { m_pExplModel = NULL; }
protected:
    void Insert(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xIface, sal_Int32 nIndex);
};

//========================================================================
class FmFormShell;
class SdrObject;
class FmFormModel;
class FmExplorerModel : public SfxBroadcaster
                       ,public SfxListener
{
    friend class FmExplorer;
    friend class FmXExplPropertyChangeList;

    FmEntryDataList*            m_pRootList;
    FmFormShell*                m_pFormShell;
    FmFormPage*                 m_pFormPage;
    FmFormModel*                m_pFormModel;
    FmXExplPropertyChangeList*  m_pPropChangeList;

    const ImageList             m_ilNavigatorImages;

    void Update( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xForms );
    FmControlData* CreateControlData( ::com::sun::star::form::XFormComponent* pFormComponent );

    void InsertForm(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& xForm, sal_uInt32 nRelPos);
    void RemoveForm(FmFormData* pFormData);

    void InsertFormComponent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& xComp, sal_uInt32 nRelPos);
    void RemoveFormComponent(FmControlData* pControlData);
    void InsertSdrObj(const SdrObject* pSdrObj);
    void RemoveSdrObj(const SdrObject* pSdrObj);

    void ReplaceFormComponent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& xOld, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& xNew);

    void BroadcastMarkedObjects(const SdrMarkList& mlMarked);
        // einen RequestSelectHint mit den aktuell markierten Objekten broadcasten
    sal_Bool InsertFormComponent(FmExplRequestSelectHint& rHint, SdrObject* pObject);
        // ist ein Helper fuer vorherige, managet das Abteigen in SdrObjGroups
        // Rueckgabe sal_True, wenn das Objekt eine FormComponent ist (oder rekursiv nur aus solchen besteht)

public:
    FmExplorerModel(const ImageList& ilNavigatorImages);
    virtual ~FmExplorerModel();

    void FillBranch( FmFormData* pParentData );
    void ClearBranch( FmFormData* pParentData );
    void Update( FmFormShell* pNewShell );

    void Insert( FmEntryData* pEntryData, sal_uInt32 nRelPos = LIST_APPEND,
                                          sal_Bool bAlterModel = sal_False );
    void Remove( FmEntryData* pEntryData, sal_Bool bAlterModel = sal_False );

    sal_Bool Rename( FmEntryData* pEntryData, const ::rtl::OUString& rNewText );
    sal_Bool IsNameAlreadyDefined( const ::rtl::OUString& rName, FmFormData* pParentData );
    void Clear();
    sal_Bool CheckEntry( FmEntryData* pEntryData );
    void SetModified( sal_Bool bMod=sal_True );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >    GetForms() const;
    FmFormShell*        GetFormShell() const { return m_pFormShell; }
    FmFormPage*         GetFormPage() const { return m_pFormPage; }
    FmEntryData*        FindData( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xElement, FmEntryDataList* pDataList, sal_Bool bRecurs=sal_True );
    FmEntryData*        FindData( const ::rtl::OUString& rText, FmFormData* pParentData, sal_Bool bRecurs=sal_True );
    FmEntryDataList*    GetRootList() const { return m_pRootList; }
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >   GetFormComponents( FmFormData* pParentFormData );
    SdrObject*          GetSdrObj( FmControlData* pControlData );
    SdrObject*          Search(SdrObjListIter& rIter, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& xComp);

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


//========================================================================
typedef SvLBoxEntry* SvLBoxEntryPtr;
SV_DECL_PTRARR_SORT( SvLBoxEntrySortedArray, SvLBoxEntryPtr, 16, 16 )

class FmExplorer : public SvTreeListBox, public SfxListener
{
    enum DROP_ACTION        { DA_SCROLLUP, DA_SCROLLDOWN, DA_EXPANDNODE };
    enum SELDATA_ITEMS      { SDI_DIRTY, SDI_ALL, SDI_NORMALIZED, SDI_NORMALIZED_FORMARK };

    // beim Droppen will ich scrollen und Folder aufklappen koennen, dafuer :
    AutoTimer           m_aDropActionTimer;
    Timer               m_aSynchronizeTimer;
    // die Meta-Daten ueber meine aktuelle Selektion
    SvLBoxEntrySortedArray  m_arrCurrentSelection;
    // die Images, die ich brauche (und an FormDatas und EntryDatas weiterreiche)
    ImageList           m_ilNavigatorImages;

    FmExplorerModel*    m_pExplModel;
    SvLBoxEntry*        m_pRootEntry;
    SvLBoxEntry*        m_pEditEntry;

    sal_uInt32              nEditEvent;

    Image               m_aCollapsedNodeImg;
    Image               m_aExpandedNodeImg;
    Image               m_aRootImg;

    SELDATA_ITEMS       m_sdiState;
    Point               m_aTimerTriggered;      // die Position, an der der DropTimer angeschaltet wurde
    DROP_ACTION         m_aDropActionType;

    sal_uInt16              m_nSelectLock;
    sal_uInt16              m_nFormsSelected;
    sal_uInt16              m_nControlsSelected;
    sal_uInt16              m_nHiddenControls;      // (die Zahl geht in m_nControlsSelected mit ein)

    unsigned short      m_aTimerCounter;

    sal_Bool                m_bShellOrPageChanged:1;    // wird in jedem Update(FmFormShell*) auf sal_True gesetzt
    sal_Bool                m_bDragDataDirty:1;     // dito
    sal_Bool                m_bPrevSelectionMixed:1;
    sal_Bool                m_bMarkingObjects:1;  // wenn das sal_True ist, brauche ich auf die RequestSelectHints nicht reagieren
    sal_Bool                m_bRootSelected:1;
    sal_Bool                m_bInitialUpdate:1;   // bin ich das erste Mal im Update ?


    void            Update();
    sal_Bool            IsDeleteAllowed();
    FmControlData*  NewControl( const ::rtl::OUString& rServiceName, SvLBoxEntry* pParentEntry, sal_Bool bEditName = sal_True );
    void            NewForm( SvLBoxEntry* pParentEntry );
    SvLBoxEntry*    Insert( FmEntryData* pEntryData, sal_uInt32 nRelPos=LIST_APPEND );
    void            Remove( FmEntryData* pEntryData );


    void CollectSelectionData(SELDATA_ITEMS sdiHow);
        // sammelt in m_arrCurrentSelection die aktuell selektierten Eintraege, normalisiert die Liste wenn verlangt
        // SDI_NORMALIZED bedeutet einfach, dass alle Eintraege, die schon einen selektierten Vorfahren haben, nicht mit gesammelt
        // werden.
        // SDI_NORMALIZED_FORMARK bedeutet, dass wie bei SDI_NORMALIZED verfahren wird, aber Eintraege, deren direktes Elter nicht
        // selektiert ist, aufgenommen werden (unabhaengig vom Status weiterer Vorfahren), desgleichen Formulare, die selektiert sind,
        // unabhaengig vom Status irgendwelcher Vorfahren
        // Bei beiden Normalized-Modi enthalten die m_nFormsSelected, ... die richtige Anzahl, auch wenn nicht alle dieser Eintraege
        // in m_arrCurrentSelection landen.
        // SDI_DIRTY ist natuerlich nicht erlaubt als Parameter

    // ein einziges Interface fuer alle selektierten Eintraege zusammensetzen
    void    ShowSelectionProperties(sal_Bool bForce = sal_False);
    // alle selektierten Elemnte loeschen
    void    DeleteSelection();

    void SynchronizeSelection(FmEntryDataArray& arredToSelect);
        // nach dem Aufruf dieser Methode sind genau die Eintraege selektiert, die in dem Array bezeichnet sind
    void SynchronizeSelection();
        // macht das selbe, nimmt die MarkList der ::com::sun::star::sdbcx::View
    void SynchronizeMarkList();
        // umgekehrte Richtung von SynchronizeMarkList : markiert in der ::com::sun::star::sdbcx::View alle der aktuellen Selektion entsprechenden Controls

    // im Select aktualisiere ich normalerweise die Marklist der zugehoerigen ::com::sun::star::sdbcx::View, mit folgenden Funktionen
    // kann ich das Locking dieses Verhaltens steuern
    void LockSelectionHandling() { ++m_nSelectLock; }
    void UnlockSelectionHandling() { --m_nSelectLock; }
    sal_Bool IsSelectionHandlingLocked() const { return m_nSelectLock>0; }

    sal_Bool IsHiddenControl(FmEntryData* pEntryData);

    DECL_LINK( OnEdit, void* );
    DECL_LINK( OnDropActionTimer, void* );

    DECL_LINK(OnEntrySelDesel, FmExplorer*);
    DECL_LINK(OnSynchronizeTimer, void*);

protected:
    virtual void Command( const CommandEvent& rEvt );

public:
    FmExplorer( Window* pParent );
    virtual ~FmExplorer();

    void Clear();
    void Update( FmFormShell* pFormShell );
    void MarkViewObj( FmFormData* pFormData, sal_Bool bMark, sal_Bool bDeep = sal_False );
    void MarkViewObj( FmControlData* pControlData, sal_Bool bMarkHandles, sal_Bool bMark );
    void UnmarkAllViewObj();

    sal_Bool IsFormEntry( SvLBoxEntry* pEntry );
    sal_Bool IsFormComponentEntry( SvLBoxEntry* pEntry );

    ::rtl::OUString GenerateName( FmEntryData* pEntryData );

    FmExplorerModel*    GetExplModel() const { return m_pExplModel; }
    SvLBoxEntry*        FindEntry( FmEntryData* pEntryData );

    virtual sal_Bool EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );
    virtual sal_Bool Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );
    virtual sal_Bool QueryDrop( DropEvent& rDEvt );
    virtual sal_Bool Drop( const DropEvent& rDEvt );
    virtual sal_Bool EditingEntry( SvLBoxEntry* pEntry, Selection& );
    virtual void MouseButtonUp( const MouseEvent& rMEvt );
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void KeyInput( const KeyEvent& rKEvt );
};


//========================================================================
class FmFormShell;
class FmExplorerWin : public SfxDockingWindow, public SfxControllerItem
{
private:
    FmExplorer* m_pFmExplorer;

protected:
    virtual void Resize();
    virtual sal_Bool Close();
    virtual Size CalcDockingSize( SfxChildAlignment );
    virtual SfxChildAlignment CheckAlignment( SfxChildAlignment, SfxChildAlignment );

public:
    FmExplorerWin( SfxBindings *pBindings, SfxChildWindow *pMgr,
                   Window* pParent );
    virtual ~FmExplorerWin();

    void Update( FmFormShell* pFormShell );
    void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
    void FillInfo( SfxChildWinInfo& rInfo ) const;
};

//========================================================================
class FmExplorerWinMgr : public SfxChildWindow
{
public:
    FmExplorerWinMgr( Window *pParent, sal_uInt16 nId, SfxBindings *pBindings,
                      SfxChildWinInfo *pInfo );
    SFX_DECL_CHILDWINDOW( FmExplorerWinMgr );
};

#endif // _SVX_FMEXPL_HXX

