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
#include <boost/ptr_container/ptr_vector.hpp>

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

    OUString           GetCommand() const { return aCommand;}

    // css::lang::XEventListener
    // we do not hold References to dispatches, so there is nothing to do on disposal
    virtual void    SAL_CALL disposing(const css::lang::EventObject& /*Source*/)
                                            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE {};

    // css::frame::XStatusListener
    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

};

class BibTBListBoxListener: public BibToolBarListener
{
public:

    BibTBListBoxListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId);
    virtual ~BibTBListBoxListener();

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

};

class BibTBEditListener: public BibToolBarListener
{
public:

    BibTBEditListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId);
    virtual ~BibTBEditListener();

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

};

class BibTBQueryMenuListener:   public BibToolBarListener
{
public:

    BibTBQueryMenuListener(BibToolBar *pTB, const OUString& aStr, sal_uInt16 nId);
    virtual ~BibTBQueryMenuListener();

    virtual void    SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event)
                                            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

};


typedef css::uno::Reference< css::frame::XStatusListener> BibToolBarListenerRef;
typedef boost::ptr_vector<BibToolBarListenerRef> BibToolBarListenerArr;

class BibToolBar:   public ToolBox
{
    private:

        BibToolBarListenerArr   aListenerArr;
        css::uno::Reference< css::frame::XController >  xController;
        Idle                    aIdle;
        ImageList               aImgLst;
        ImageList               aImgLstHC;
        ImageList               aBigImgLst;
        ImageList               aBigImgLstHC;
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

        BibDataManager*         pDatMan;
        DECL_LINK( SelHdl, ListBox* );
        DECL_LINK_TYPED( SendSelHdl, Idle*, void );
        DECL_LINK_TYPED( MenuHdl, ToolBox*, void );
        DECL_LINK_TYPED( OptionsChanged_Impl, LinkParamNone*, void );
        DECL_LINK_TYPED( SettingsChanged_Impl, VclSimpleEvent&, void );

        void                    ApplyImageList();
        void                    RebuildToolbar();

    protected:

        void                    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
        void                    InitListener();
        virtual void            Select() SAL_OVERRIDE;
        virtual void            Click() SAL_OVERRIDE;
        virtual bool            PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;


    public:

        BibToolBar(vcl::Window* pParent, Link<void*,void> aLink, WinBits nStyle = WB_3DLOOK );
        virtual ~BibToolBar();
        virtual void dispose() SAL_OVERRIDE;

        void    SetXController(const css::uno::Reference< css::frame::XController > &);

        void    ClearSourceList();
        void    UpdateSourceList(bool bFlag=true);
        void    EnableSourceList(bool bFlag=true);
        void    InsertSourceEntry(const OUString&,sal_Int32  nPos=LISTBOX_APPEND );
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
