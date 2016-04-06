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


#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>
#include <cppuhelper/implbase.hxx>
#include <vector>

class BibDataManager;
class BibToolBar;

class BibToolBarListener: public cppu::WeakImplHelper < css::frame::XStatusListener>
{
private:

    sal_uInt16         nIndex;
    OUString           aCommand;

protected:

    VclPtr<BibToolBar> pToolBar;

public:

    BibToolBarListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId);
    virtual ~BibToolBarListener();

    const OUString&          GetCommand() const { return aCommand;}

    // css::lang::XEventListener
    // we do not hold References to dispatches, so there is nothing to do on disposal
    virtual void    SAL_CALL disposing(const css::lang::EventObject& /*Source*/)
                                            throw( css::uno::RuntimeException, std::exception ) override {};

    // css::frame::XStatusListener
    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) override;

};

class BibTBListBoxListener: public BibToolBarListener
{
public:

    BibTBListBoxListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId);
    virtual ~BibTBListBoxListener();

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) override;

};

class BibTBEditListener: public BibToolBarListener
{
public:

    BibTBEditListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId);
    virtual ~BibTBEditListener();

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) override;

};

class BibTBQueryMenuListener:   public BibToolBarListener
{
public:

    BibTBQueryMenuListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId);
    virtual ~BibTBQueryMenuListener();

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) override;

};


typedef std::vector< css::uno::Reference< css::frame::XStatusListener> > BibToolBarListenerArr;

class BibToolBar:   public ToolBox
{
    private:

        BibToolBarListenerArr   aListenerArr;
        css::uno::Reference< css::frame::XController >  xController;
        Idle                    aIdle;
        ImageList               aImgLst;
        ImageList               aBigImgLst;
        VclPtr<FixedText>       aFtSource;
        VclPtr<ListBox>         aLBSource;
        VclPtr<FixedText>       aFtQuery;
        VclPtr<Edit>            aEdQuery;
        PopupMenu               aPopupMenu;
        sal_uInt16              nMenuId;
        sal_uInt16              nSelMenuItem;
        OUString                aQueryField;
        Link<void*,void>        aLayoutManager;
        sal_Int16               nSymbolsSize;
        sal_Int16               nOutStyle;

        sal_uInt16              nTBC_FT_SOURCE;
        sal_uInt16              nTBC_LB_SOURCE;
        sal_uInt16              nTBC_FT_QUERY;
        sal_uInt16              nTBC_ED_QUERY;
        sal_uInt16              nTBC_BT_AUTOFILTER;
        sal_uInt16              nTBC_BT_COL_ASSIGN;
        sal_uInt16              nTBC_BT_CHANGESOURCE;
        sal_uInt16              nTBC_BT_FILTERCRIT;
        sal_uInt16              nTBC_BT_REMOVEFILTER;

        BibDataManager*         pDatMan;
        DECL_LINK_TYPED( SelHdl, ListBox&, void );
        DECL_LINK_TYPED( SendSelHdl, Idle*, void );
        DECL_LINK_TYPED( MenuHdl, ToolBox*, void );
        DECL_LINK_TYPED( OptionsChanged_Impl, LinkParamNone*, void );
        DECL_LINK_TYPED( SettingsChanged_Impl, VclSimpleEvent&, void );

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
        virtual ~BibToolBar();
        virtual void dispose() override;

        sal_uInt16  GetChangeSourceId() const { return nTBC_BT_CHANGESOURCE; }

        void    SetXController(const css::uno::Reference< css::frame::XController > &);

        void    ClearSourceList();
        void    UpdateSourceList(bool bFlag=true);
        void    EnableSourceList(bool bFlag=true);
        void    InsertSourceEntry(const OUString& );
        void    SelectSourceEntry(const OUString& );

        void    EnableQuery(bool bFlag=true);
        void    SetQueryString(const OUString& );
        void    AdjustToolBox();

        void    ClearFilterMenu();
        sal_uInt16  InsertFilterItem(const OUString& );
        void    SelectFilterItem(sal_uInt16 nId);

        void    statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException );

        void    SetDatMan(BibDataManager& rDatMan) {pDatMan = &rDatMan;}
        void    SendDispatch(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue >& rArgs);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
