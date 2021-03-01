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
#ifndef INCLUDED_SVX_SOURCE_INC_FILTNAV_HXX
#define INCLUDED_SVX_SOURCE_INC_FILTNAV_HXX

#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/form/runtime/XFilterController.hpp>
#include <svl/lstner.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <vcl/window.hxx>
#include <sfx2/childwin.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/ctrlitem.hxx>

#include "fmexch.hxx"
#include "sqlparserclient.hxx"

class FmFormShell;

namespace svxform
{

class FmFilterItem;
class FmFilterItems;
class FmParentData;
class FmFilterAdapter;

// data structure for the filter model
class FmFilterData
{
    FmParentData*    m_pParent;
    OUString         m_aText;

public:
    FmFilterData(FmParentData* pParent, const OUString& rText)
        :m_pParent( pParent )
        ,m_aText( rText )
    {}
    virtual ~FmFilterData(){}

    void    SetText( const OUString& rText ){ m_aText = rText; }
    const OUString& GetText() const { return m_aText; }
    FmParentData* GetParent() const {return m_pParent;}

    virtual OUString GetImage() const;
};

class FmParentData : public FmFilterData
{
protected:
    ::std::vector< std::unique_ptr<FmFilterData> >  m_aChildren;

public:
    FmParentData(FmParentData* pParent, const OUString& rText)
        : FmFilterData(pParent, rText)
    {}
    virtual ~FmParentData() override;

    ::std::vector< std::unique_ptr<FmFilterData> >& GetChildren() { return m_aChildren; }
};

// Item representing the forms and subforms
class FmFormItem : public FmParentData
{
    css::uno::Reference< css::form::runtime::XFormController >    m_xController;
    css::uno::Reference< css::form::runtime::XFilterController >  m_xFilterController;

public:

    FmFormItem(  FmParentData* _pParent,
                 const css::uno::Reference< css::form::runtime::XFormController > & _xController,
                 const OUString& _rText)
        :FmParentData( _pParent, _rText )
        ,m_xController( _xController )
        ,m_xFilterController( _xController, css::uno::UNO_QUERY_THROW )
    {
    }

    const css::uno::Reference< css::form::runtime::XFormController >&
        GetController() const { return m_xController; }

    const css::uno::Reference< css::form::runtime::XFilterController >&
        GetFilterController() const { return m_xFilterController; }

    virtual OUString GetImage() const override;
};

class FmFilterItems : public FmParentData
{
public:
    FmFilterItems(FmFormItem* pParent, const OUString& rText ) : FmParentData(pParent, rText) {}

    FmFilterItem* Find( const ::sal_Int32 _nFilterComponentIndex ) const;
    virtual OUString GetImage() const override;
};

class FmFilterItem : public FmFilterData
{
    OUString      m_aFieldName;
    sal_Int32     m_nComponentIndex;

public:
    FmFilterItem(
        FmFilterItems* pParent,
        const OUString& aFieldName,
        const OUString& aCondition,
        const sal_Int32 _nComponentIndex
    );

    const OUString& GetFieldName() const {return m_aFieldName;}
    sal_Int32 GetComponentIndex() const { return m_nComponentIndex; }

    virtual OUString GetImage() const override;
};

class FmFilterModel final : public FmParentData
                     ,public SfxBroadcaster
                     ,public ::svxform::OSQLParserClient
{
    friend class FmFilterAdapter;

    css::uno::Reference< css::container::XIndexAccess >           m_xControllers;
    css::uno::Reference< css::form::runtime::XFormController >    m_xController;
    rtl::Reference<FmFilterAdapter>                               m_pAdapter;
    FmFilterItems*          m_pCurrentItems;

public:
    FmFilterModel();
    virtual ~FmFilterModel() override;

    void Update(const css::uno::Reference< css::container::XIndexAccess > & xControllers, const css::uno::Reference< css::form::runtime::XFormController > & xCurrent);
    void Clear();
    bool ValidateText(FmFilterItem const * pItem, OUString& rText, OUString& rErrorMsg) const;
    void Append(FmFilterItems* pItems, std::unique_ptr<FmFilterItem> pFilterItem);
    void SetTextForItem(FmFilterItem* pItem, const OUString& rText);

    FmFormItem* GetCurrentForm() const {return m_pCurrentItems ? static_cast<FmFormItem*>(m_pCurrentItems->GetParent()) : nullptr;}
    FmFilterItems* GetCurrentItems() const {return m_pCurrentItems;}
    void SetCurrentItems(FmFilterItems* pCurrent);

    const css::uno::Reference< css::form::runtime::XFormController > & GetCurrentController() const {return m_xController;}
    void SetCurrentController(const css::uno::Reference< css::form::runtime::XFormController > & xController);

    void Remove(FmFilterData* pFilterItem);
    static void AppendFilterItems( FmFormItem& _rItem );
    void EnsureEmptyFilterRows( FmParentData& _rItem );

private:
    void Insert(const ::std::vector<std::unique_ptr<FmFilterData>>::iterator& rPos, std::unique_ptr<FmFilterData> pFilterItem);
    void Remove( const ::std::vector<std::unique_ptr<FmFilterData>>::iterator& rPos );
    FmFormItem* Find(const ::std::vector<std::unique_ptr<FmFilterData>>& rItems, const css::uno::Reference< css::form::runtime::XFormController > & xController) const;
    FmFormItem* Find(const ::std::vector<std::unique_ptr<FmFilterData>>& rItems, const css::uno::Reference< css::form::XForm >& xForm) const;
    void Update(const css::uno::Reference< css::container::XIndexAccess > & xControllers, FmParentData* pParent);
};


class OFilterItemExchange : public OLocalExchange
{
    ::std::vector<FmFilterItem*>    m_aDraggedEntries;
    FmFormItem*             m_pFormItem;        // ensure that we drop on the same form

public:
    OFilterItemExchange();

    static SotClipboardFormatId getFormatId( );
    inline static bool  hasFormat( const DataFlavorExVector& _rFormats );

    const ::std::vector<FmFilterItem*>& getDraggedEntries() const { return m_aDraggedEntries; }
    void setDraggedEntries(const ::std::vector<FmFilterItem*>& _rList) { m_aDraggedEntries = _rList; }
    FmFormItem* getFormItem() const { return m_pFormItem; }

    void setFormItem( FmFormItem* _pItem ) { m_pFormItem = _pItem; }

protected:
    virtual void AddSupportedFormats() override;
};

inline bool OFilterItemExchange::hasFormat( const DataFlavorExVector& _rFormats )
{
    return OLocalExchange::hasFormat( _rFormats, getFormatId() );
}

class OFilterExchangeHelper : public OLocalExchangeHelper
{
public:
    OFilterExchangeHelper() : OLocalExchangeHelper() { }

    OFilterItemExchange* operator->() const { return static_cast<OFilterItemExchange*>(m_xTransferable.get()); }
    OFilterItemExchange& operator*() const { return *static_cast<OFilterItemExchange*>(m_xTransferable.get()); }

protected:
    virtual rtl::Reference<OLocalExchange> createExchange() const override;
};

class FmFilterNavigator;

class FmFilterNavigatorDropTarget : public DropTargetHelper
{
private:
    FmFilterNavigator& m_rTreeView;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    FmFilterNavigatorDropTarget(FmFilterNavigator& rTreeView);
};

class FmFilterNavigator final : public SfxListener
{
    VclPtr<vcl::Window> m_xTopLevel;
    std::unique_ptr<weld::TreeView> m_xTreeView;
    FmFilterNavigatorDropTarget m_aDropTargetHelper;

    std::unique_ptr<FmFilterModel> m_pModel;
    std::unique_ptr<weld::TreeIter> m_xEditingCurrently;
    OFilterExchangeHelper   m_aControlExchange;

    ImplSVEvent* m_nAsyncRemoveEvent;

public:
    FmFilterNavigator(vcl::Window* pTopLevel, std::unique_ptr<weld::TreeView> xTreeView);
    virtual ~FmFilterNavigator() override;

    void GrabFocus() { m_xTreeView->grab_focus(); }

    void EndEditing();

    void UpdateContent(
            const css::uno::Reference< css::container::XIndexAccess > & xControllers,
            const css::uno::Reference< css::form::runtime::XFormController > & xCurrent
        );

    weld::TreeView& get_widget() { return *m_xTreeView; }

    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt);
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

private:
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(PopupMenuHdl, const CommandEvent&, bool);
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    DECL_STATIC_LINK(FmFilterNavigator, CustomGetSizeHdl, weld::TreeView::get_size_args, Size);
    DECL_STATIC_LINK(FmFilterNavigator, CustomRenderHdl, weld::TreeView::render_args, void);

    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(EditingEntryHdl, const weld::TreeIter&, bool);
    typedef std::pair<const weld::TreeIter&, OUString> IterString;
    DECL_LINK(EditedEntryHdl, const IterString&, bool);

    DECL_LINK(DragBeginHdl, bool&, bool);

    void DeleteSelection();
    std::unique_ptr<weld::TreeIter> FindEntry(const FmFilterData* pItem) const;
    void Insert(FmFilterData* pItem, int nPos);
    void Remove(FmFilterData const * pItem);

    DECL_LINK(OnRemove, void*, void);

    /** returns the first form item and the selected FilterItems in the vector
        @param  _rItemList
            Is filled inside. <OUT/>
        @return
            The first form item.
    */
    FmFormItem* getSelectedFilterItems(::std::vector<FmFilterItem*>& _rItemList);

    /**
     * inserts the filter items into the tree model and creates new FilterItems if needed.
     *    @param  _rFilterList
     *        The items which should be inserted.
     *    @param  _pTargetItems
     *        The target where to insert the items.
     *    @param  _bCopy
     *        If <TRUE/> the items will not be removed from the model, otherwise they will.
     */
    void insertFilterItem(const ::std::vector<FmFilterItem*>& _rFilterList,FmFilterItems* _pTargetItems, bool _bCopy);

    bool getPrevEntry(weld::TreeIter& rEntry);
    bool getNextEntry(weld::TreeIter& rEntry);
};

class FmFilterNavigatorWin : public SfxDockingWindow, public SfxControllerItem
{
private:
    std::unique_ptr<FmFilterNavigator> m_xNavigatorTree;

protected:
    virtual bool Close() override;
    virtual void GetFocus() override;
    virtual Size CalcDockingSize( SfxChildAlignment ) override;
    virtual SfxChildAlignment CheckAlignment( SfxChildAlignment, SfxChildAlignment ) override;

    using SfxDockingWindow::StateChanged;

public:
    FmFilterNavigatorWin( SfxBindings *pBindings, SfxChildWindow *pMgr,
                   vcl::Window* pParent );
    virtual ~FmFilterNavigatorWin() override;
    virtual void dispose() override;

    void UpdateContent( FmFormShell const * pFormShell );
    void StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState ) override;
    void FillInfo( SfxChildWinInfo& rInfo ) const override;
};

class FmFilterNavigatorWinMgr : public SfxChildWindow
{
public:
    FmFilterNavigatorWinMgr( vcl::Window *pParent, sal_uInt16 nId, SfxBindings *pBindings,
                      SfxChildWinInfo *pInfo );
    SFX_DECL_CHILDWINDOW( FmFilterNavigatorWinMgr );
};

}

#endif // INCLUDED_SVX_SOURCE_INC_FILTNAV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
