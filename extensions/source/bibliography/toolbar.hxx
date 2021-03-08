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

#ifndef INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_TOOLBAR_HXX
#define INCLUDED_EXTENSIONS_SOURCE_BIBLIOGRAPHY_TOOLBAR_HXX

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>

#include <vcl/InterimItemWindow.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/timer.hxx>
#include <cppuhelper/implbase.hxx>
#include <vector>

class BibDataManager;
class BibToolBar;

class BibToolBarListener: public cppu::WeakImplHelper < css::frame::XStatusListener>
{
private:

    ToolBoxItemId      nIndex;
    OUString           aCommand;

protected:

    VclPtr<BibToolBar> pToolBar;

public:

    BibToolBarListener(BibToolBar *pTB, const OUString& aStr, ToolBoxItemId nId);
    virtual ~BibToolBarListener() override;

    const OUString&          GetCommand() const { return aCommand;}

    // css::lang::XEventListener
    // we do not hold References to dispatches, so there is nothing to do on disposal
    virtual void    SAL_CALL disposing(const css::lang::EventObject& /*Source*/) override {};

    // css::frame::XStatusListener
    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event) override;

};

class BibTBListBoxListener: public BibToolBarListener
{
public:

    BibTBListBoxListener(BibToolBar *pTB, const OUString& aStr, ToolBoxItemId nId);
    virtual ~BibTBListBoxListener() override;

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event) override;

};

class BibTBEditListener: public BibToolBarListener
{
public:

    BibTBEditListener(BibToolBar *pTB, const OUString& aStr, ToolBoxItemId nId);
    virtual ~BibTBEditListener() override;

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event) override;

};

class BibTBQueryMenuListener:   public BibToolBarListener
{
public:

    BibTBQueryMenuListener(BibToolBar *pTB, const OUString& aStr, ToolBoxItemId nId);
    virtual ~BibTBQueryMenuListener() override;

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event) override;

};


typedef std::vector< css::uno::Reference< css::frame::XStatusListener> > BibToolBarListenerArr;

class ComboBoxControl final : public InterimItemWindow
{
public:
    ComboBoxControl(vcl::Window* pParent);
    virtual ~ComboBoxControl() override;
    virtual void dispose() override;

    weld::ComboBox* get_widget() { return m_xLBSource.get(); }

    void set_sensitive(bool bSensitive)
    {
        m_xFtSource->set_sensitive(bSensitive);
        m_xLBSource->set_sensitive(bSensitive);
        Enable(bSensitive);
    }

private:
    std::unique_ptr<weld::Label> m_xFtSource;
    std::unique_ptr<weld::ComboBox> m_xLBSource;
};

class EditControl final : public InterimItemWindow
{
public:
    EditControl(vcl::Window* pParent);
    virtual ~EditControl() override;
    virtual void dispose() override;

    weld::Entry* get_widget() { return m_xEdQuery.get(); }

    void set_sensitive(bool bSensitive)
    {
        m_xFtQuery->set_sensitive(bSensitive);
        m_xEdQuery->set_sensitive(bSensitive);
        Enable(bSensitive);
    }

private:
    std::unique_ptr<weld::Label> m_xFtQuery;
    std::unique_ptr<weld::Entry> m_xEdQuery;
};

class BibToolBar:   public ToolBox
{
    private:

        BibToolBarListenerArr   aListenerArr;
        css::uno::Reference< css::frame::XController >  xController;
        Idle                    aIdle;
        VclPtr<ComboBoxControl> xSource;
        weld::ComboBox*         pLbSource;
        VclPtr<EditControl>     xQuery;
        weld::Entry*            pEdQuery;
        std::unique_ptr<weld::Builder> xBuilder;
        std::unique_ptr<weld::Menu> xPopupMenu;
        sal_uInt16              nMenuId;
        OString                 sSelMenuItem;
        OUString                aQueryField;
        Link<void*,void>        aLayoutManager;
        sal_Int16               nSymbolsSize;

        ToolBoxItemId           nTBC_SOURCE;
        ToolBoxItemId           nTBC_QUERY;
        ToolBoxItemId           nTBC_BT_AUTOFILTER;
        ToolBoxItemId           nTBC_BT_COL_ASSIGN;
        ToolBoxItemId           nTBC_BT_CHANGESOURCE;
        ToolBoxItemId           nTBC_BT_FILTERCRIT;
        ToolBoxItemId           nTBC_BT_REMOVEFILTER;

        BibDataManager*         pDatMan;
        DECL_LINK( SelHdl, weld::ComboBox&, void );
        DECL_LINK( SendSelHdl, Timer*, void );
        DECL_LINK( MenuHdl, ToolBox*, void );
        DECL_LINK( OptionsChanged_Impl, LinkParamNone*, void );
        DECL_LINK( SettingsChanged_Impl, VclSimpleEvent&, void );

        void                    ApplyImageList();
        void                    RebuildToolbar();

    protected:

        void                    DataChanged( const DataChangedEvent& rDCEvt ) override;
        void                    InitListener();
        virtual void            Select() override;
        virtual void            Click() override;
        virtual bool            PreNotify( NotifyEvent& rNEvt ) override;


    public:

        BibToolBar(vcl::Window* pParent, Link<void*,void> aLink);
        virtual ~BibToolBar() override;
        virtual void dispose() override;

        ToolBoxItemId  GetChangeSourceId() const { return nTBC_BT_CHANGESOURCE; }

        void    SetXController(const css::uno::Reference< css::frame::XController > &);

        void    ClearSourceList();
        void    UpdateSourceList(bool bFlag);
        void    EnableSourceList(bool bFlag);
        void    InsertSourceEntry(const OUString& );
        void    SelectSourceEntry(const OUString& );

        void    EnableQuery(bool bFlag);
        void    SetQueryString(const OUString& );
        void    AdjustToolBox();

        void    ClearFilterMenu();
        sal_uInt16  InsertFilterItem(const OUString& );
        void    SelectFilterItem(sal_uInt16 nId);

        /// @throws css::uno::RuntimeException
        void    statusChanged(const css::frame::FeatureStateEvent& Event);

        void    SetDatMan(BibDataManager& rDatMan) {pDatMan = &rDatMan;}
        void    SendDispatch(ToolBoxItemId nId, const css::uno::Sequence< css::beans::PropertyValue >& rArgs);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
