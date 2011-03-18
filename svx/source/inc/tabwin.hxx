/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SVX_TABWIN_HXX
#define _SVX_TABWIN_HXX

#include <svtools/svtreebx.hxx>
#include <vcl/floatwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <com/sun/star/form/XForm.hpp>

#include <comphelper/propmultiplex.hxx>
#include <svtools/transfer.hxx>
#include "svx/dbtoolsclient.hxx"

//==================================================================
class FmFieldWin;
class SAL_DLLPRIVATE FmFieldWinListBox
                    :public SvTreeListBox
{
    FmFieldWin* pTabWin;

protected:
//  virtual void Command( const CommandEvent& rEvt );

public:
    FmFieldWinListBox( FmFieldWin* pParent );
    virtual ~FmFieldWinListBox();

    sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

protected:
    // DragSourceHelper
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // SvLBox
    virtual sal_Bool DoubleClickHdl();

    using SvLBox::ExecuteDrop;
};

//========================================================================
class FmFormShell;
class SAL_DLLPRIVATE FmFieldWinData
{
public:
    FmFieldWinData();
    ~FmFieldWinData();
};

//========================================================================
class SAL_DLLPRIVATE FmFieldWin :public SfxFloatingWindow
                    ,public SfxControllerItem
                    ,public ::comphelper::OPropertyChangeListener
                    ,public ::svxform::OStaticDataAccessTools
{
    ::osl::Mutex        m_aMutex;
    FmFieldWinListBox* pListBox;
    FmFieldWinData*    pData;
    ::svxform::SharedConnection
                       m_aConnection;
    ::rtl::OUString    m_aDatabaseName,
                       m_aObjectName;
    sal_Int32          m_nObjectType;

    ::comphelper::OPropertyChangeMultiplexer*   m_pChangeListener;

public:
    FmFieldWin(SfxBindings *pBindings,
               SfxChildWindow *pMgr, Window* pParent);

    virtual ~FmFieldWin();
    virtual void Resize();
    virtual sal_Bool Close();
    virtual void GetFocus();
    virtual long PreNotify( NotifyEvent& _rNEvt );
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState);

    FmFieldWinData* GetData()  const {return pData;}

    void UpdateContent(FmFormShell*);
    void UpdateContent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > &);
    void FillInfo( SfxChildWinInfo& rInfo ) const;

    const ::rtl::OUString&      GetDatabaseName() const { return m_aDatabaseName; }
    ::svxform::SharedConnection GetConnection() const { return m_aConnection; }
    const ::rtl::OUString&      GetObjectName() const { return m_aObjectName; }
    sal_Int32                   GetObjectType() const { return m_nObjectType; }

    sal_Bool    createSelectionControls( );

protected:
    // FmXChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException );

protected:
    inline          SfxBindings&    GetBindings()       { return SfxControllerItem::GetBindings(); }
    inline  const   SfxBindings&    GetBindings() const { return SfxControllerItem::GetBindings(); }

    using SfxFloatingWindow::StateChanged;
};

//========================================================================
class SAL_DLLPRIVATE FmFieldWinMgr : public SfxChildWindow
{
public:
    FmFieldWinMgr(Window *pParent, sal_uInt16 nId,
        SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(FmFieldWinMgr);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
