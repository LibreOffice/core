/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolbar.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _BIB_TOOLBAR_HXX
#define _BIB_TOOLBAR_HXX

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>


#include <vcl/toolbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <svtools/svarray.hxx>
#include <vcl/timer.hxx>
#include <cppuhelper/implbase1.hxx> // helper for implementations

class BibDataManager;
class BibToolBar;

class BibToolBarListener: public cppu::WeakImplHelper1 < ::com::sun::star::frame::XStatusListener>
{
private:

    sal_uInt16      nIndex;
    rtl::OUString           aCommand;

protected:

    BibToolBar      *pToolBar;

public:

    BibToolBarListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId);
    ~BibToolBarListener();

    rtl::OUString           GetCommand();
    void            SetCommand(const rtl::OUString& aStr);

    sal_uInt16          GetIndex();
    void            SetIndex(sal_uInt16 nIndex);

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

    BibTBListBoxListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId);
    ~BibTBListBoxListener();

    virtual void    SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

};

class BibTBEditListener: public BibToolBarListener
{
public:

    BibTBEditListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId);
    ~BibTBEditListener();

    virtual void    SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

};

class BibTBQueryMenuListener:   public BibToolBarListener
{
public:

    BibTBQueryMenuListener(BibToolBar *pTB,rtl::OUString aStr,sal_uInt16 nId);
    ~BibTBQueryMenuListener();

    virtual void    SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

};


typedef ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener>* BibToolBarListenerPtr;
SV_DECL_PTRARR_DEL( BibToolBarListenerArr, BibToolBarListenerPtr, 4, 4 )

class BibToolBar:   public ToolBox
{
    private:

        BibToolBarListenerArr   aListenerArr;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >            xController;
        Timer                   aTimer;
//      Timer                   aMenuTimer;
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
        rtl::OUString           aQueryField;
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
        void    InsertSourceEntry(const XubString&,sal_uInt16 nPos=LISTBOX_APPEND );
        void    SelectSourceEntry(const XubString& );

        void    EnableQuery(sal_Bool bFlag=sal_True);
        void    SetQueryString(const XubString& );
        void    AdjustToolBox();

        void    ClearFilterMenu();
        sal_uInt16  InsertFilterItem(const XubString& );
        void    SelectFilterItem(sal_uInt16 nId);

        void    statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );

        void    SetDatMan(BibDataManager& rDatMan) {pDatMan = &rDatMan;}
        void    SendDispatch(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs);
};




#endif
