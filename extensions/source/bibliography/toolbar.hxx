/*************************************************************************
 *
 *  $RCSfile: toolbar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-11-14 15:10:26 $
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

#ifndef _BIB_TOOLBAR_HXX
#define _BIB_TOOLBAR_HXX

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif



#ifndef _SV_TOOLBOX_HXX //autogen wg. ToolBox
#include <vcl/toolbox.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen wg. ::com::sun::star::form
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen wg. Edit
#include <vcl/edit.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen wg. FixedText
#include <vcl/fixed.hxx>
#endif

#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef _SV_TIMER_HXX //autogen wg. Timer
#include <vcl/timer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif


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
    virtual void    SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source)
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
SV_DECL_PTRARR_DEL( BibToolBarListenerArr, BibToolBarListenerPtr, 4, 4 );

class BibToolBar:   public ToolBox
{
    private:

        BibToolBarListenerArr   aListenerArr;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >            xController;
        Timer                   aTimer;
        Timer                   aMenuTimer;

        FixedText               aFtSource;
        ListBox                 aLBSource;
        FixedText               aFtQuery;
        Edit                    aEdQuery;
        PopupMenu               aPopupMenu;
        sal_uInt16              nMenuId;
        sal_uInt16              nSelMenuItem;
        rtl::OUString           aQueryField;
        DECL_LINK( SelHdl, ListBox* );
        DECL_LINK( SendSelHdl, Timer* );
        DECL_LINK( MenuHdl, Timer* );

    protected:

        void                    InitListener();
        virtual void            Select();
        virtual void            Click();
        void                    SendDispatch(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs);
        long                    PreNotify( NotifyEvent& rNEvt );


    public:

        BibToolBar(Window* pParent, WinBits nStyle = WB_3DLOOK );
        ~BibToolBar();

        void    SetXController(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > &);

        void    ClearSourceList();
        void    UpdateSourceList(sal_Bool bFlag=sal_True);
        void    EnableSourceList(sal_Bool bFlag=sal_True);
        void    InsertSourceEntry(const XubString&,sal_uInt16 nPos=LISTBOX_APPEND );
        void    SelectSourceEntry(const XubString& );

        void    EnableQuery(sal_Bool bFlag=sal_True);
        void    SetQueryString(const XubString& );

        void    ClearFilterMenu();
        sal_uInt16  InsertFilterItem(const XubString& );
        void    SelectFilterItem(sal_uInt16 nId);

        void    statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event)
                                            throw( ::com::sun::star::uno::RuntimeException );
};




#endif
