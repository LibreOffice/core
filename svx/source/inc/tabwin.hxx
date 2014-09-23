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
#ifndef INCLUDED_SVX_SOURCE_INC_TABWIN_HXX
#define INCLUDED_SVX_SOURCE_INC_TABWIN_HXX

#include <svtools/treelistbox.hxx>
#include <vcl/floatwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <com/sun/star/form/XForm.hpp>

#include <comphelper/propmultiplex.hxx>
#include <svtools/transfer.hxx>
#include "svx/dbtoolsclient.hxx"


class FmFieldWin;
class FmFieldWinListBox
                    :public SvTreeListBox
{
    FmFieldWin* pTabWin;

protected:
//  virtual void Command( const CommandEvent& rEvt );

public:
    FmFieldWinListBox( FmFieldWin* pParent );
    virtual ~FmFieldWinListBox();

    sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;
    sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

protected:
    // DragSourceHelper
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;

    // SvTreeListBox
    virtual bool DoubleClickHdl() SAL_OVERRIDE;

    using SvTreeListBox::ExecuteDrop;
};


class FmFormShell;
class FmFieldWinData
{
public:
    FmFieldWinData();
    ~FmFieldWinData();
};


class FmFieldWin :public SfxFloatingWindow
                    ,public SfxControllerItem
                    ,public ::comphelper::OPropertyChangeListener
                    ,public ::svxform::OStaticDataAccessTools
{
    ::osl::Mutex        m_aMutex;
    FmFieldWinListBox* pListBox;
    FmFieldWinData*    pData;
    ::svxform::SharedConnection
                       m_aConnection;
    OUString    m_aDatabaseName,
                       m_aObjectName;
    sal_Int32          m_nObjectType;

    ::comphelper::OPropertyChangeMultiplexer*   m_pChangeListener;

public:
    FmFieldWin(SfxBindings *pBindings,
               SfxChildWindow *pMgr, vcl::Window* pParent);

    virtual ~FmFieldWin();
    virtual void Resize() SAL_OVERRIDE;
    virtual bool Close() SAL_OVERRIDE;
    virtual void GetFocus() SAL_OVERRIDE;
    virtual bool PreNotify( NotifyEvent& _rNEvt ) SAL_OVERRIDE;
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState) SAL_OVERRIDE;

    FmFieldWinData* GetData()  const {return pData;}

    void UpdateContent(FmFormShell*);
    void UpdateContent(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > &);
    void FillInfo( SfxChildWinInfo& rInfo ) const SAL_OVERRIDE;

    const OUString&      GetDatabaseName() const { return m_aDatabaseName; }
    ::svxform::SharedConnection GetConnection() const { return m_aConnection; }
    const OUString&      GetObjectName() const { return m_aObjectName; }
    sal_Int32                   GetObjectType() const { return m_nObjectType; }

    bool    createSelectionControls( );

protected:
    // FmXChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException ) SAL_OVERRIDE;

protected:
    inline          SfxBindings&    GetBindings()       { return SfxControllerItem::GetBindings(); }
    inline  const   SfxBindings&    GetBindings() const { return SfxControllerItem::GetBindings(); }

    using SfxFloatingWindow::StateChanged;
};


class FmFieldWinMgr : public SfxChildWindow
{
public:
    FmFieldWinMgr(vcl::Window *pParent, sal_uInt16 nId,
        SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(FmFieldWinMgr);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
