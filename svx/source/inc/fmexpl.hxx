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
#ifndef INCLUDED_SVX_SOURCE_INC_FMEXPL_HXX
#define INCLUDED_SVX_SOURCE_INC_FMEXPL_HXX

#include <svl/lstner.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <vcl/window.hxx>
#include <sfx2/childwin.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <vcl/image.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

#include <svtools/treelistbox.hxx>

#include <vcl/dialog.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/dockwin.hxx>
#include <svx/fmview.hxx>

#include "fmexch.hxx"
#include <vector>
#include <set>

class SdrObjListIter;
class FmFormShell;
class SdrObject;
class FmFormModel;
class FmFormView;
class SdrMarkList;


class FmEntryData;
class FmNavInsertedHint : public SfxHint
{
    FmEntryData* pEntryData;
    sal_uInt32 nPos;

public:
    FmNavInsertedHint( FmEntryData* pInsertedEntryData, sal_uInt32 nRelPos );
    virtual ~FmNavInsertedHint();

    FmEntryData* GetEntryData() const { return pEntryData; }
    sal_uInt32 GetRelPos() const { return nPos; }
};


class FmNavModelReplacedHint : public SfxHint
{
    FmEntryData* pEntryData;    // die Daten des Eintrages, der ein neues Model bekommen hat

public:
    FmNavModelReplacedHint( FmEntryData* pAffectedEntryData );
    virtual ~FmNavModelReplacedHint();

    FmEntryData* GetEntryData() const { return pEntryData; }
};


class FmNavRemovedHint : public SfxHint
{
    FmEntryData* pEntryData;

public:
    FmNavRemovedHint( FmEntryData* pInsertedEntryData );
    virtual ~FmNavRemovedHint();

    FmEntryData* GetEntryData() const { return pEntryData; }
};


class FmNavNameChangedHint : public SfxHint
{
    FmEntryData*    pEntryData;
    OUString          aNewName;

public:
    FmNavNameChangedHint( FmEntryData* pData, const OUString& rNewName );
    virtual ~FmNavNameChangedHint();

    FmEntryData*    GetEntryData() const { return pEntryData; }
    OUString          GetNewName() const { return aNewName; }
};


class FmNavClearedHint : public SfxHint
{
public:
    FmNavClearedHint();
    virtual ~FmNavClearedHint();
};


class FmNavViewMarksChanged : public SfxHint
{
    FmFormView* pView;
public:
    FmNavViewMarksChanged(FmFormView* pWhichView) { pView = pWhichView; }
    virtual ~FmNavViewMarksChanged() {}

    const FmFormView* GetAffectedView() const { return pView; }
};


class FmEntryDataList;
class FmEntryData
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >       m_xNormalizedIFace;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xProperties;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >     m_xChild;

protected:
    Image               m_aNormalImage;
    OUString     aText;

    FmEntryDataList*    pChildList;
    FmEntryData*        pParent;

protected:
    void    newObject( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace );

public:
    TYPEINFO();

    FmEntryData( FmEntryData* pParentData, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rIFace );
    FmEntryData( const FmEntryData& rEntryData );
    virtual ~FmEntryData();

    void    Clear();
    void    SetText( const OUString& rText ){ aText = rText; }
    void    SetParent( FmEntryData* pParentData ){ pParent = pParentData; }

    const Image&    GetNormalImage() const { return m_aNormalImage; }

    OUString          GetText() const { return aText; }
    FmEntryData*    GetParent() const { return pParent; }
    FmEntryDataList* GetChildList() const { return pChildList; }

    virtual bool IsEqualWithoutChildren( FmEntryData* pEntryData );
    virtual FmEntryData* Clone() = 0;

    // note that the interface returned is normalized, i.e. querying the given XInterface of the object
    // for XInterface must return the interface itself.
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& GetElement() const
    {
        return m_xNormalizedIFace;
    }

    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& GetPropertySet() const
    {
        return m_xProperties;
    }

    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >& GetChildIFace() const
    {
        return m_xChild;
    }
};


typedef ::std::vector< FmEntryData* > FmEntryDataBaseList;

class FmEntryDataList
{
private:
    FmEntryDataBaseList maEntryDataList;

public:
    FmEntryDataList();
    virtual ~FmEntryDataList();

    FmEntryData*    at( size_t Index )
        { return ( Index < maEntryDataList.size() ) ? maEntryDataList[ Index ] : NULL; }

    size_t          size() const { return maEntryDataList.size(); }
    FmEntryData*    remove( FmEntryData* pItem );
    void            insert( FmEntryData* pItem, size_t Index );
    void            clear();
};


// FmNavRequestSelectHint - jemand teilt dem NavigatorTree mit, dass er bestimmte Eintraege selektieren soll

typedef std::set<FmEntryData*> FmEntryDataArray;

class FmNavRequestSelectHint : public SfxHint
{
    FmEntryDataArray    m_arredToSelect;
    bool                m_bMixedSelection;
public:
    FmNavRequestSelectHint()
        : m_bMixedSelection(false)
    {
    }
    virtual ~FmNavRequestSelectHint() {}

    void SetMixedSelection(bool bMixedSelection) { m_bMixedSelection = bMixedSelection; }
    bool IsMixedSelection() const { return m_bMixedSelection; }
    void AddItem(FmEntryData* pEntry) { m_arredToSelect.insert(pEntry); }
    void ClearItems() { m_arredToSelect.clear(); }
    FmEntryDataArray& GetItems() { return m_arredToSelect; }
};


class FmFormData : public FmEntryData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >           m_xForm;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer > m_xContainer;

public:
    TYPEINFO_OVERRIDE();

    FmFormData(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
        const ImageList& _rNormalImages,
        FmFormData* _pParent = NULL
    );

    FmFormData( const FmFormData& rFormData );
    virtual ~FmFormData();

    void SetForm( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& xForm )
    {
        m_xForm = xForm;
        m_xContainer.set(m_xForm, css::uno::UNO_QUERY);
        newObject( m_xForm );
    }

    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& GetFormIface() const { return m_xForm; }
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainer >& GetContainer() const { return m_xContainer; }

    virtual bool IsEqualWithoutChildren( FmEntryData* pEntryData ) SAL_OVERRIDE;
    virtual FmEntryData* Clone() SAL_OVERRIDE;
};



class FmControlData : public FmEntryData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  m_xFormComponent;

    Image GetImage(const ImageList& ilNavigatorImages) const;

public:
    TYPEINFO_OVERRIDE();

    FmControlData(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& _rxComponent,
        const ImageList& _rNormalImages,
        FmFormData* _pParent
    );
    FmControlData( const FmControlData& rControlData );
    virtual ~FmControlData();

    const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& GetFormComponent() const { return m_xFormComponent; }
    virtual bool IsEqualWithoutChildren( FmEntryData* pEntryData ) SAL_OVERRIDE;
    virtual FmEntryData* Clone() SAL_OVERRIDE;

    void ModelReplaced(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& _rxNew,
        const ImageList& _rNormalImages
    );
};




namespace svxform
{


    class NavigatorTreeModel;

    class OFormComponentObserver
        :public ::cppu::WeakImplHelper2 <   ::com::sun::star::beans::XPropertyChangeListener
                                        ,   ::com::sun::star::container::XContainerListener
                                        >
    {
        ::svxform::NavigatorTreeModel*  m_pNavModel;
        sal_uInt32 m_nLocks;
        bool   m_bCanUndo;

    public:
        OFormComponentObserver( ::svxform::NavigatorTreeModel* pModel );

    // XEventListenerListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::container::XContainerListener

        virtual void SAL_CALL elementInserted(const  ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementReplaced(const  ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL elementRemoved(const  ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        void Lock() { m_nLocks++; }
        void UnLock() { m_nLocks--; }
        bool IsLocked() const { return m_nLocks != 0; }
        bool CanUndo() const { return m_bCanUndo; }
        void ReleaseModel() { m_pNavModel = NULL; }
    protected:
        void Insert(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xIface, sal_Int32 nIndex);
        void Remove( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );
    };

    class NavigatorTreeModel : public SfxBroadcaster
                           ,public SfxListener
    {
        friend class NavigatorTree;
        friend class OFormComponentObserver;

        FmEntryDataList*            m_pRootList;
        FmFormShell*                m_pFormShell;
        FmFormPage*                 m_pFormPage;
        FmFormModel*                m_pFormModel;
        OFormComponentObserver*     m_pPropChangeList;

        ImageList                   m_aNormalImages;

        void UpdateContent( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForms >& xForms );
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
        bool InsertFormComponent(FmNavRequestSelectHint& rHint, SdrObject* pObject);
            // ist ein Helper fuer vorherige, managet das Abteigen in SdrObjGroups
            // Rueckgabe sal_True, wenn das Objekt eine FormComponent ist (oder rekursiv nur aus solchen besteht)

    public:
        NavigatorTreeModel( const ImageList& _rNormalImages );
        virtual ~NavigatorTreeModel();

        void FillBranch( FmFormData* pParentData );
        void ClearBranch( FmFormData* pParentData );
        void UpdateContent( FmFormShell* pNewShell );

        void Insert( FmEntryData* pEntryData, sal_uLong nRelPos = CONTAINER_APPEND,
                                              bool bAlterModel = false );
        void Remove( FmEntryData* pEntryData, bool bAlterModel = false );

        bool Rename( FmEntryData* pEntryData, const OUString& rNewText );

        void Clear();
        void SetModified( bool bMod=true );

        ::com::sun::star::uno::Reference< ::com::sun::star::form::XForms >    GetForms() const;
        FmFormShell*        GetFormShell() const { return m_pFormShell; }
        FmFormPage*         GetFormPage() const { return m_pFormPage; }
        FmEntryData*        FindData( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xElement, FmEntryDataList* pDataList, bool bRecurs=true );
        FmEntryData*        FindData( const OUString& rText, FmFormData* pParentData, bool bRecurs=true );
        FmEntryDataList*    GetRootList() const { return m_pRootList; }
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >   GetFormComponents( FmFormData* pParentFormData );
        SdrObject*          Search(SdrObjListIter& rIter, const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >& xComp);

        virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
    };


    typedef std::set<SvTreeListEntry*> SvLBoxEntrySortedArray;

    class NavigatorTree : public SvTreeListBox, public SfxListener
    {
        enum DROP_ACTION        { DA_SCROLLUP, DA_SCROLLDOWN, DA_EXPANDNODE };
        enum SELDATA_ITEMS      { SDI_DIRTY, SDI_ALL, SDI_NORMALIZED, SDI_NORMALIZED_FORMARK };

        // beim Droppen will ich scrollen und Folder aufklappen koennen, dafuer :
        AutoTimer           m_aDropActionTimer;
        Timer               m_aSynchronizeTimer;
        // die Meta-Daten ueber meine aktuelle Selektion
        SvLBoxEntrySortedArray  m_arrCurrentSelection;
        // the entries which, in the view, are currently marked as "cut" (painted semi-transparent)
        ListBoxEntrySet         m_aCutEntries;
        // die Images, die ich brauche (und an FormDatas und EntryDatas weiterreiche)
        ImageList           m_aNavigatorImages;

        ::svxform::OControlExchangeHelper   m_aControlExchange;

        NavigatorTreeModel* m_pNavModel;
        SvTreeListEntry*        m_pRootEntry;
        SvTreeListEntry*        m_pEditEntry;

        ImplSVEvent *       nEditEvent;

        SELDATA_ITEMS       m_sdiState;
        Point               m_aTimerTriggered;      // die Position, an der der DropTimer angeschaltet wurde
        DROP_ACTION         m_aDropActionType;

        sal_uInt16          m_nSelectLock;
        sal_uInt16          m_nFormsSelected;
        sal_uInt16          m_nControlsSelected;
        sal_uInt16          m_nHiddenControls;      // (die Zahl geht in m_nControlsSelected mit ein)

        unsigned short      m_aTimerCounter;

        bool            m_bDragDataDirty        : 1;    // dito
        bool            m_bPrevSelectionMixed   : 1;
        bool            m_bMarkingObjects       : 1;    // wenn das sal_True ist, brauche ich auf die RequestSelectHints nicht reagieren
        bool            m_bRootSelected         : 1;
        bool            m_bInitialUpdate        : 1;   // bin ich das erste Mal im UpdateContent ?
        bool            m_bKeyboardCut          : 1;


        void            UpdateContent();
        FmControlData*  NewControl( const OUString& rServiceName, SvTreeListEntry* pParentEntry, bool bEditName = true );
        void            NewForm( SvTreeListEntry* pParentEntry );
        SvTreeListEntry*    Insert( FmEntryData* pEntryData, sal_uLong nRelPos=TREELIST_APPEND );
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
        void    ShowSelectionProperties(bool bForce = false);
        // alle selektierten Elemnte loeschen
        void    DeleteSelection();

        void SynchronizeSelection(FmEntryDataArray& arredToSelect);
            // nach dem Aufruf dieser Methode sind genau die Eintraege selektiert, die in dem Array bezeichnet sind
        void SynchronizeSelection();
            // macht das selbe, nimmt die MarkList der View
        void SynchronizeMarkList();
            // umgekehrte Richtung von SynchronizeMarkList : markiert in der View alle der aktuellen Selektion entsprechenden Controls

        void CollectObjects(FmFormData* pFormData, bool bDeep, ::std::set< ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > >& _rObjects);

        // im Select aktualisiere ich normalerweise die Marklist der zugehoerigen View, mit folgenden Funktionen
        // kann ich das Locking dieses Verhaltens steuern
        void LockSelectionHandling() { ++m_nSelectLock; }
        void UnlockSelectionHandling() { --m_nSelectLock; }
        bool IsSelectionHandlingLocked() const { return m_nSelectLock>0; }

        bool IsHiddenControl(FmEntryData* pEntryData);

        DECL_LINK( OnEdit, void* );
        DECL_LINK( OnDropActionTimer, void* );

        DECL_LINK(OnEntrySelDesel, NavigatorTree*);
        DECL_LINK(OnSynchronizeTimer, void*);

        DECL_LINK( OnClipboardAction, void* );

    protected:
        virtual void    Command( const CommandEvent& rEvt ) SAL_OVERRIDE;

        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

    public:
        NavigatorTree(vcl::Window* pParent );
        virtual ~NavigatorTree();

        void Clear();
        void UpdateContent( FmFormShell* pFormShell );
        void MarkViewObj( FmFormData* pFormData, bool bMark, bool bDeep = false );
        void MarkViewObj( FmControlData* pControlData, bool bMarkHandles, bool bMark );
        void UnmarkAllViewObj();

        bool IsFormEntry( SvTreeListEntry* pEntry );
        bool IsFormComponentEntry( SvTreeListEntry* pEntry );

        OUString GenerateName( FmEntryData* pEntryData );

        NavigatorTreeModel*    GetNavModel() const { return m_pNavModel; }
        SvTreeListEntry*        FindEntry( FmEntryData* pEntryData );

        virtual bool EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;
        virtual bool Select( SvTreeListEntry* pEntry, bool bSelect=true ) SAL_OVERRIDE;
        virtual bool EditingEntry( SvTreeListEntry* pEntry, Selection& ) SAL_OVERRIDE;
        virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
        virtual void KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

        virtual void ModelHasRemoved( SvTreeListEntry* _pEntry ) SAL_OVERRIDE;

        using SvTreeListBox::Insert;
        using SvTreeListBox::ExecuteDrop;
        using SvTreeListBox::Select;
        using SvTreeListBox::Notify;

    private:
        sal_Int8    implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, const Point& _rDropPos, bool _bDnD );
        sal_Int8    implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, bool _bDnD );

        sal_Int8    implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, const Point& _rDropPos, bool _bDnD );
        sal_Int8    implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, bool _bDnD );

        // check if a cut, copy, or drag operation can be started in the current situation
        bool        implAllowExchange( sal_Int8 _nAction, bool* _pHasNonHidden = NULL );
        // check if a paste with the current clipboard content can be accepted
        bool        implAcceptPaste( );

        // fills m_aControlExchange in preparation of a DnD or clipboard operation
        bool        implPrepareExchange( sal_Int8 _nAction );

        void        doPaste();
        void        doCopy();
        void        doCut();

        bool    doingKeyboardCut( ) const { return m_bKeyboardCut; }
    };


    class NavigatorFrame : public SfxDockingWindow, public SfxControllerItem
    {
    private:
        ::svxform::NavigatorTree* m_pNavigatorTree;

    protected:
        virtual void Resize() SAL_OVERRIDE;
        virtual bool Close() SAL_OVERRIDE;
        virtual void GetFocus() SAL_OVERRIDE;
        virtual Size CalcDockingSize( SfxChildAlignment ) SAL_OVERRIDE;
        virtual SfxChildAlignment CheckAlignment( SfxChildAlignment, SfxChildAlignment ) SAL_OVERRIDE;

        using SfxDockingWindow::StateChanged;

    public:
        NavigatorFrame( SfxBindings *pBindings, SfxChildWindow *pMgr,
                       vcl::Window* pParent );
        virtual ~NavigatorFrame();

        void UpdateContent( FmFormShell* pFormShell );
        void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) SAL_OVERRIDE;
        void FillInfo( SfxChildWinInfo& rInfo ) const SAL_OVERRIDE;
    };


    class SVX_DLLPUBLIC NavigatorFrameManager : public SfxChildWindow
    {
    public:
        SVX_DLLPRIVATE NavigatorFrameManager( vcl::Window *pParent, sal_uInt16 nId, SfxBindings *pBindings,
                          SfxChildWinInfo *pInfo );
        SFX_DECL_CHILDWINDOW( NavigatorFrameManager );
    };


}   // namespace svxform


#endif // INCLUDED_SVX_SOURCE_INC_FMEXPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
