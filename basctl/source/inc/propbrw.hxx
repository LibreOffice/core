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

#ifndef _BASCTL_PROPBRW_HXX
#define _BASCTL_PROPBRW_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/stl_types.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>
#include <svl/lstner.hxx>
#include <svx/svdmark.hxx>

//============================================================================
// PropBrwMgr
//============================================================================

class PropBrwMgr : public SfxChildWindow
{
public:
    PropBrwMgr(Window *pParent, sal_uInt16 nId, SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(PropBrwMgr);
};

//============================================================================
// PropBrw
//============================================================================

class SfxBindings;
class SdrView;

class PropBrw : public SfxFloatingWindow , public SfxListener, public SfxBroadcaster
{
private:
    bool        m_bInitialStateChange;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                    m_xMeAsFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xBrowserController;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                    m_xBrowserComponentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                    m_xContextDocument;

protected:
    SdrView*        pView;
    virtual void Resize();
    virtual void FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual sal_Bool Close();

    DECLARE_STL_VECTOR(::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>, InterfaceArray);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
        CreateMultiSelectionSequence( const SdrMarkList& _rMarkList );
    void implSetNewObjectSequence( const ::com::sun::star::uno::Sequence
        < ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rObjectSeq );

    void implSetNewObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

    ::rtl::OUString GetHeadlineName(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);

public:
    PropBrw( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB,
             SfxBindings *pBindings,
             PropBrwMgr* pMgr,
             Window* pParent,
             const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument
    );
    virtual ~PropBrw();
    using Window::Update;
    // note: changing the Context document to an instance other than the one given in the ctor is not supported
    // currently
    void    Update( const SfxViewShell* _pShell );
    SdrView*        GetCurView() const { return pView; }

private:
    void    ImplUpdate( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxContextDocument, SdrView* pView );
    void    ImplDestroyController();
    void    ImplReCreateController();
};

#endif // _BASCTL_PROPBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
