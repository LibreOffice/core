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

#ifndef _BIB_TOOLBAR_HXX
#define _BIB_TOOLBAR_HXX

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>


#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/timer.hxx>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <boost/ptr_container/ptr_vector.hpp>

class BibDataManager;
class BibToolBar;

class BibToolBarListener: public cppu::WeakImplHelper1 < ::com::sun::star::frame::XStatusListener>
{
private:

    sal_uInt16      nIndex;
    OUString           aCommand;

protected:

    BibToolBar      *pToolBar;

public:

    BibToolBarListener(BibToolBar *pTB,OUString aStr,sal_uInt16 nId);
    ~BibToolBarListener();

    OUString           GetCommand() const;

    // ::com::sun::star::lang::XEventListener
    // we do not hold References to dispatches, so there is nothing to do on disposal
    virtual void    SAL_CALL disposing(const ::com::sun::star::lang::EventObject& /*Source*/)
                                            throw( ::com::sun::star::uno::RuntimeException ){};

    // ::com::sun::star::frame::XStatusListener
    virtual void    SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

};

class BibTBListBoxListener: public BibToolBarListener
{
public:

    BibTBListBoxListener(BibToolBar *pTB,OUString aStr,sal_uInt16 nId);
    ~BibTBListBoxListener();

    virtual void    SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

};

class BibTBEditListener: public BibToolBarListener
{
public:

    BibTBEditListener(BibToolBar *pTB,OUString aStr,sal_uInt16 nId);
    ~BibTBEditListener();

    virtual void    SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

};

class BibTBQueryMenuListener:   public BibToolBarListener
{
public:

    BibTBQueryMenuListener(BibToolBar *pTB,OUString aStr,sal_uInt16 nId);
    ~BibTBQueryMenuListener();

    virtual void    SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

};


typedef ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener> BibToolBarListenerRef;
typedef boost::ptr_vector<BibToolBarListenerRef> BibToolBarListenerArr;

class BibToolBar:   public ToolBox
{
    private:

        BibToolBarListenerArr   aListenerArr;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >            xController;
        Timer                   aTimer;
        ImageList               aImgLst;
        ImageList               aImgLstHC;
        ImageList               aBigImgLst;
        ImageList               aBigImgLstHC;
        FixedText               aFtSource;
        ListBox                 aLBSource;
        FixedText               aFtQuery;
        Edit                    aEdQuery;
        PopupMenu               aPopupMenu;
        sal_uInt16              nMenuId;
        sal_uInt16              nSelMenuItem;
        OUString           aQueryField;
        Link                    aLayoutManager;
        sal_Int16               nSymbolsSize;
        sal_Int16               nOutStyle;

        BibDataManager*         pDatMan;
        DECL_LINK( SelHdl, ListBox* );
        DECL_LINK( SendSelHdl, Timer* );
        DECL_LINK( MenuHdl, ToolBox* );
        DECL_LINK( OptionsChanged_Impl, void* );
        DECL_LINK( SettingsChanged_Impl, void* );

        void                    ApplyImageList();
        void                    RebuildToolbar();

    protected:

        void                    DataChanged( const DataChangedEvent& rDCEvt );
        void                    InitListener();
        virtual void            Select();
        virtual void            Click();
        long                    PreNotify( NotifyEvent& rNEvt );


    public:

        BibToolBar(Window* pParent, Link aLink, WinBits nStyle = WB_3DLOOK );
        ~BibToolBar();

        void    SetXController(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > &);

        void    ClearSourceList();
        void    UpdateSourceList(sal_Bool bFlag=sal_True);
        void    EnableSourceList(sal_Bool bFlag=sal_True);
        void    InsertSourceEntry(const OUString&,sal_uInt16 nPos=LISTBOX_APPEND );
        void    SelectSourceEntry(const OUString& );

        void    EnableQuery(sal_Bool bFlag=sal_True);
        void    SetQueryString(const OUString& );
        void    AdjustToolBox();

        void    ClearFilterMenu();
        sal_uInt16  InsertFilterItem(const OUString& );
        void    SelectFilterItem(sal_uInt16 nId);

        void    statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

        void    SetDatMan(BibDataManager& rDatMan) {pDatMan = &rDatMan;}
        void    SendDispatch(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs);
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
