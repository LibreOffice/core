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

#include <config_options.h>
#include <svl/lstner.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <vcl/window.hxx>
#include <sfx2/childwin.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/ctrlitem.hxx>

#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>

#include <svx/fmview.hxx>

#include "fmexch.hxx"
#include <vector>
#include <set>
#include <cppuhelper/implbase.hxx>

class SdrObjListIter;
class FmFormShell;
class SdrObject;
class FmFormModel;
class FmFormView;
class SdrMarkList;


class FmEntryData;
class FmNavInsertedHint final : public SfxHint
{
    FmEntryData* pEntryData;
    sal_uInt32 nPos;

public:
    FmNavInsertedHint( FmEntryData* pInsertedEntryData, sal_uInt32 nRelPos );
    virtual ~FmNavInsertedHint() override;

    FmEntryData* GetEntryData() const { return pEntryData; }
    sal_uInt32 GetRelPos() const { return nPos; }
};


class FmNavModelReplacedHint final : public SfxHint
{
    FmEntryData* pEntryData;    // the data of the entry that has got a new model

public:
    FmNavModelReplacedHint( FmEntryData* pAffectedEntryData );
    virtual ~FmNavModelReplacedHint() override;

    FmEntryData* GetEntryData() const { return pEntryData; }
};


class FmNavRemovedHint final : public SfxHint
{
    FmEntryData* pEntryData;

public:
    FmNavRemovedHint( FmEntryData* pInsertedEntryData );
    virtual ~FmNavRemovedHint() override;

    FmEntryData* GetEntryData() const { return pEntryData; }
};


class FmNavNameChangedHint final : public SfxHint
{
    FmEntryData*    pEntryData;
    OUString          aNewName;

public:
    FmNavNameChangedHint( FmEntryData* pData, OUString aNewName );
    virtual ~FmNavNameChangedHint() override;

    FmEntryData*    GetEntryData() const { return pEntryData; }
    const OUString& GetNewName() const { return aNewName; }
};


class FmNavClearedHint final : public SfxHint
{
public:
    FmNavClearedHint();
    virtual ~FmNavClearedHint() override;
};


class FmNavViewMarksChanged final : public SfxHint
{
    FmFormView* pView;
public:
    FmNavViewMarksChanged(FmFormView* pWhichView) { pView = pWhichView; }

    const FmFormView* GetAffectedView() const { return pView; }
};


class FmEntryDataList;
class FmEntryData
{
private:
    css::uno::Reference< css::uno::XInterface >       m_xNormalizedIFace;
    css::uno::Reference< css::beans::XPropertySet >   m_xProperties;
    css::uno::Reference< css::container::XChild >     m_xChild;

protected:
    OUString            m_aNormalImage;
    OUString            aText;

    std::unique_ptr<FmEntryDataList>
                        pChildList;
    FmEntryData*        pParent;

protected:
    void    newObject( const css::uno::Reference< css::uno::XInterface >& _rxIFace );

public:

    FmEntryData( FmEntryData* pParentData, const css::uno::Reference< css::uno::XInterface >& _rIFace );
    FmEntryData( const FmEntryData& rEntryData );
    virtual ~FmEntryData();

    void    SetText( const OUString& rText ){ aText = rText; }
    void    SetParent( FmEntryData* pParentData ){ pParent = pParentData; }

    const OUString& GetNormalImage() const { return m_aNormalImage; }

    const OUString& GetText() const { return aText; }
    FmEntryData*    GetParent() const { return pParent; }
    FmEntryDataList* GetChildList() const { return pChildList.get(); }

    virtual bool IsEqualWithoutChildren( FmEntryData* pEntryData );
    virtual std::unique_ptr<FmEntryData> Clone() = 0;

    // note that the interface returned is normalized, i.e. querying the given XInterface of the object
    // for XInterface must return the interface itself.
    const css::uno::Reference< css::uno::XInterface >& GetElement() const
    {
        return m_xNormalizedIFace;
    }

    const css::uno::Reference< css::beans::XPropertySet >& GetPropertySet() const
    {
        return m_xProperties;
    }

    const css::uno::Reference< css::container::XChild >& GetChildIFace() const
    {
        return m_xChild;
    }
};


class FmEntryDataList final
{
private:
    std::vector< std::unique_ptr<FmEntryData> > maEntryDataList;

public:
    FmEntryDataList();
    ~FmEntryDataList();

    FmEntryData*    at( size_t Index )
        { return maEntryDataList.at(Index).get(); }

    size_t          size() const { return maEntryDataList.size(); }
    void            removeNoDelete( FmEntryData* pItem );
    void            insert( std::unique_ptr<FmEntryData> pItem, size_t Index );
    void            clear();
};


// FmNavRequestSelectHint - someone tells the NavigatorTree to select certain entries

typedef std::set<FmEntryData*> FmEntryDataArray;

class FmNavRequestSelectHint final : public SfxHint
{
    FmEntryDataArray    m_arredToSelect;
    bool                m_bMixedSelection;
public:
    FmNavRequestSelectHint()
        : m_bMixedSelection(false)
    {
    }

    void SetMixedSelection(bool bMixedSelection) { m_bMixedSelection = bMixedSelection; }
    bool IsMixedSelection() const { return m_bMixedSelection; }
    void AddItem(FmEntryData* pEntry) { m_arredToSelect.insert(pEntry); }
    void ClearItems() { m_arredToSelect.clear(); }
    FmEntryDataArray& GetItems() { return m_arredToSelect; }
};


class FmFormData final : public FmEntryData
{
    css::uno::Reference< css::form::XForm >           m_xForm;

public:
    FmFormData(const css::uno::Reference< css::form::XForm >& _rxForm, FmFormData* _pParent);
    FmFormData( const FmFormData& rFormData );
    virtual ~FmFormData() override;

    const css::uno::Reference< css::form::XForm >& GetFormIface() const { return m_xForm; }

    virtual bool IsEqualWithoutChildren( FmEntryData* pEntryData ) override;
    virtual std::unique_ptr<FmEntryData> Clone() override;
};


class FmControlData final : public FmEntryData
{
    css::uno::Reference< css::form::XFormComponent >  m_xFormComponent;

    OUString GetImage() const;

public:

    FmControlData(
        const css::uno::Reference< css::form::XFormComponent >& _rxComponent,
        FmFormData* _pParent
    );
    FmControlData( const FmControlData& rControlData );
    virtual ~FmControlData() override;

    const css::uno::Reference< css::form::XFormComponent >& GetFormComponent() const { return m_xFormComponent; }
    virtual bool IsEqualWithoutChildren( FmEntryData* pEntryData ) override;
    virtual std::unique_ptr<FmEntryData> Clone() override;

    void ModelReplaced(const css::uno::Reference< css::form::XFormComponent >& _rxNew);
};


namespace svxform
{


    class NavigatorTreeModel;

    class OFormComponentObserver final
        :public ::cppu::WeakImplHelper <   css::beans::XPropertyChangeListener
                                        ,   css::container::XContainerListener
                                        >
    {
        ::svxform::NavigatorTreeModel*  m_pNavModel;
        sal_uInt32 m_nLocks;
        bool   m_bCanUndo;

    public:
        OFormComponentObserver( ::svxform::NavigatorTreeModel* pModel );

    // XEventListenerListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    // css::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

    // css::container::XContainerListener

        virtual void SAL_CALL elementInserted(const  css::container::ContainerEvent& rEvent) override;
        virtual void SAL_CALL elementReplaced(const  css::container::ContainerEvent& rEvent) override;
        virtual void SAL_CALL elementRemoved(const  css::container::ContainerEvent& rEvent) override;

        void Lock() { m_nLocks++; }
        void UnLock() { m_nLocks--; }
        bool IsLocked() const { return m_nLocks != 0; }
        bool CanUndo() const { return m_bCanUndo; }
        void ReleaseModel() { m_pNavModel = nullptr; }
    private:
        void Insert(const css::uno::Reference< css::uno::XInterface >& xIface, sal_Int32 nIndex);
        void Remove( const css::uno::Reference< css::uno::XInterface >& _rxElement );
    };

    class NavigatorTreeModel final : public SfxBroadcaster
                           ,public SfxListener
    {
        friend class NavigatorTree;
        friend class OFormComponentObserver;

        std::unique_ptr<FmEntryDataList>
                                    m_pRootList;
        FmFormShell*                m_pFormShell;
        FmFormPage*                 m_pFormPage;
        FmFormModel*                m_pFormModel;
        rtl::Reference<OFormComponentObserver> m_pPropChangeList;

        void UpdateContent( const css::uno::Reference< css::form::XForms >& xForms );

        void InsertForm(const css::uno::Reference< css::form::XForm >& xForm, sal_uInt32 nRelPos);
        void RemoveForm(FmFormData const * pFormData);

        void InsertFormComponent(const css::uno::Reference< css::form::XFormComponent >& xComp, sal_uInt32 nRelPos);
        void RemoveFormComponent(FmControlData const * pControlData);
        void InsertSdrObj(const SdrObject* pSdrObj);
        void RemoveSdrObj(const SdrObject* pSdrObj);

        void ReplaceFormComponent(const css::uno::Reference< css::form::XFormComponent >& xOld, const css::uno::Reference< css::form::XFormComponent >& xNew);

        void BroadcastMarkedObjects(const SdrMarkList& mlMarked);
            // send a RequestSelectHint with the currently selected objects
        bool InsertFormComponent(FmNavRequestSelectHint& rHint, SdrObject* pObject);
            // is a helper for previous, manages the ... in SdrObjGroups;
            // returns sal_True if the object is a FormComponent (or recursively consists only of such)

    public:
        NavigatorTreeModel();
        virtual ~NavigatorTreeModel() override;

        void FillBranch( FmFormData* pParentData );
        void UpdateContent( FmFormShell* pNewShell );

        void Insert(FmEntryData* pEntryData, sal_uInt32 nRelPos = SAL_MAX_UINT32,
                                             bool bAlterModel = false);
        void Remove(FmEntryData* pEntryData, bool bAlterModel = false);

        static bool Rename( FmEntryData* pEntryData, const OUString& rNewText );

        void Clear();
        void SetModified();

        css::uno::Reference< css::form::XForms >    GetForms() const;
        FmFormShell*        GetFormShell() const { return m_pFormShell; }
        FmFormPage*         GetFormPage() const { return m_pFormPage; }
        FmEntryData*        FindData( const css::uno::Reference< css::uno::XInterface >& xElement, FmEntryDataList* pDataList, bool bRecurs=true );
        FmEntryData*        FindData( const OUString& rText, FmFormData const * pParentData, bool bRecurs );
        FmEntryDataList*    GetRootList() const { return m_pRootList.get(); }
        static css::uno::Reference< css::container::XIndexContainer >   GetFormComponents( FmFormData const * pParentFormData );

        virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    };

    class NavigatorTree;

    class NavigatorTreeDropTarget final : public DropTargetHelper
    {
    private:
        NavigatorTree& m_rTreeView;

        virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
        virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    public:
        NavigatorTreeDropTarget(NavigatorTree& rTreeView);
    };

    typedef std::set<std::unique_ptr<weld::TreeIter>> SvLBoxEntrySortedArray;

    class NavigatorTree final : public SfxListener
    {
        std::unique_ptr<weld::TreeView> m_xTreeView;
        NavigatorTreeDropTarget m_aDropTargetHelper;

        enum SELDATA_ITEMS      { SDI_DIRTY, SDI_ALL, SDI_NORMALIZED, SDI_NORMALIZED_FORMARK };

        Timer               m_aSynchronizeTimer;
        // the meta-data about my current selection
        SvLBoxEntrySortedArray  m_arrCurrentSelection;
        // the entries which, in the view, are currently marked as "cut" (painted semi-transparent)
        ListBoxEntrySet         m_aCutEntries;

        ::svxform::OControlExchangeHelper m_aControlExchange;

        std::unique_ptr<NavigatorTreeModel> m_pNavModel;
        std::unique_ptr<weld::TreeIter> m_xRootEntry;
        std::unique_ptr<weld::TreeIter> m_xEditEntry;

        ImplSVEvent *       nEditEvent;

        SELDATA_ITEMS       m_sdiState;

        sal_uInt16          m_nSelectLock;
        sal_uInt16          m_nFormsSelected;
        sal_uInt16          m_nControlsSelected;
        sal_uInt16          m_nHiddenControls;      // (the number is included in m_nControlsSelected)

        bool            m_bDragDataDirty        : 1;    // ditto
        bool            m_bPrevSelectionMixed   : 1;
        bool            m_bRootSelected         : 1;
        bool            m_bInitialUpdate        : 1;    // am I the first time in the UpdateContent?
        bool            m_bKeyboardCut          : 1;
        bool            m_bEditing              : 1;

        FmControlData*  NewControl(const OUString& rServiceName, const weld::TreeIter& rParentEntry, bool bEditName);
        void            NewForm(const weld::TreeIter& rParentEntry);
        std::unique_ptr<weld::TreeIter> Insert(const FmEntryData* pEntryData, int nRelPos);
        void            Remove( FmEntryData* pEntryData );


        void CollectSelectionData(SELDATA_ITEMS sdiHow);
            // Collects the currently selected entries in m_arrCurrentSelection, normalizes the list if requested.
            // - SDI_NORMALIZED simply means that all entries that already have a selected ancestor are not collected.
            // - SDI_NORMALIZED_FORMARK means that the procedure is the same as for SDI_NORMALIZED,
            //   but entries whose direct parent is not selected are collected (independent of the
            //   status of further ancestors). The same applies for forms that are selected,
            //   regardless of the status of any ancestors.
            // For both normalized modes, the m_nFormsSelected, ... contain the correct number,
            // even if not all of these entries end up in m_arrCurrentSelection.
            // SDI_DIRTY is of course not allowed as a parameter.

        // a single interface for all selected entries
        void    ShowSelectionProperties(bool bForce = false);
        // delete all selected elements
        void    DeleteSelection();

        void SynchronizeSelection(FmEntryDataArray& arredToSelect);
            // after calling this method, exactly the entries marked in the array are selected
        void SynchronizeSelection();
            // makes the same, takes the MarkList of the View
        void SynchronizeMarkList();
            // reverse direction of SynchronizeMarkList: selects in the view all controls corresponding to the current selection

        void CollectObjects(FmFormData const * pFormData, bool bDeep, ::std::set< css::uno::Reference< css::form::XFormComponent > >& _rObjects);

        // in the Select I usually update the Marklist of the corresponding view,
        // with the following functions I can control the locking of this behavior
        void LockSelectionHandling() { ++m_nSelectLock; }
        void UnlockSelectionHandling() { --m_nSelectLock; }
        bool IsSelectionHandlingLocked() const { return m_nSelectLock>0; }

        bool IsEditingActive() const { return m_bEditing; }

        static bool IsHiddenControl(FmEntryData const * pEntryData);

        DECL_LINK( KeyInputHdl, const KeyEvent&, bool );
        DECL_LINK( PopupMenuHdl, const CommandEvent&, bool );

        DECL_LINK(EditingEntryHdl, const weld::TreeIter&, bool);
        typedef std::pair<const weld::TreeIter&, OUString> IterString;
        DECL_LINK(EditedEntryHdl, const IterString&, bool);

        DECL_LINK( OnEdit, void*, void );

        DECL_LINK( OnEntrySelDesel, weld::TreeView&, void );
        DECL_LINK( OnSynchronizeTimer, Timer*, void );

        DECL_LINK( OnClipboardAction, OLocalExchange&, void );

        DECL_LINK( DragBeginHdl, bool&, bool );

    public:
        NavigatorTree(std::unique_ptr<weld::TreeView> xTreeView);
        virtual ~NavigatorTree() override;

        void Clear();
        void UpdateContent( FmFormShell* pFormShell );
        void MarkViewObj( FmFormData const * pFormData, bool bDeep );
        void MarkViewObj( FmControlData const * pControlData );
        void UnmarkAllViewObj();

        void GrabFocus() { m_xTreeView->grab_focus(); }

        bool IsFormEntry(const weld::TreeIter& rEntry);
        bool IsFormComponentEntry(const weld::TreeIter& rEntry);

        OUString GenerateName(const FmEntryData& rEntryData);

        NavigatorTreeModel*    GetNavModel() const { return m_pNavModel.get(); }
        std::unique_ptr<weld::TreeIter> FindEntry(FmEntryData* pEntryData);

        virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

        weld::TreeView& get_widget() { return *m_xTreeView; }

        sal_Int8    AcceptDrop(const AcceptDropEvent& rEvt);
        sal_Int8    ExecuteDrop(const ExecuteDropEvent& rEvt);

    private:
        sal_Int8    implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, const weld::TreeIter* _pTargetEntry, bool _bDnD );

        sal_Int8    implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, const Point& _rDropPos, bool _bDnD );
        sal_Int8    implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, const weld::TreeIter* _pTargetEntry, bool _bDnD );

        // check if a cut, copy, or drag operation can be started in the current situation
        bool        implAllowExchange( sal_Int8 _nAction, bool* _pHasNonHidden = nullptr );
        // check if a paste with the current clipboard content can be accepted
        bool        implAcceptPaste( );

        // fills m_aControlExchange in preparation of a DnD or clipboard operation
        bool        implPrepareExchange( sal_Int8 _nAction );

        void        ModelHasRemoved(const weld::TreeIter* _pEntry);

        void        doPaste();
        void        doCopy();
        void        doCut();

        bool    doingKeyboardCut( ) const { return m_bKeyboardCut; }
    };

    class NavigatorFrame final : public SfxDockingWindow, public SfxControllerItem
    {
    private:
        std::unique_ptr<NavigatorTree> m_xNavigatorTree;

        virtual bool Close() override;
        virtual void GetFocus() override;
        virtual Size CalcDockingSize( SfxChildAlignment ) override;
        virtual SfxChildAlignment CheckAlignment( SfxChildAlignment, SfxChildAlignment ) override;

        using SfxDockingWindow::StateChanged;

    public:
        NavigatorFrame( SfxBindings *pBindings, SfxChildWindow *pMgr,
                       vcl::Window* pParent );
        virtual ~NavigatorFrame() override;
        virtual void dispose() override;

        void UpdateContent( FmFormShell* pFormShell );
        void StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
        void FillInfo( SfxChildWinInfo& rInfo ) const override;
    };

    class UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) NavigatorFrameManager final : public SfxChildWindow
    {
    public:
        SVX_DLLPRIVATE NavigatorFrameManager( vcl::Window *pParent, sal_uInt16 nId, SfxBindings *pBindings,
                          SfxChildWinInfo *pInfo );
        SFX_DECL_CHILDWINDOW( NavigatorFrameManager );
    };
}

#endif // INCLUDED_SVX_SOURCE_INC_FMEXPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
