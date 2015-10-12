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

#ifndef INCLUDED_BASCTL_SOURCE_INC_PROPBRW_HXX
#define INCLUDED_BASCTL_SOURCE_INC_PROPBRW_HXX

#include <sal/config.h>

#include <vector>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <svl/lstner.hxx>
#include <svl/SfxBroadcaster.hxx>
#include <svx/svdmark.hxx>
#include "bastypes.hxx"

class SfxBindings;
class SdrView;
class SfxViewShell;

namespace basctl
{

class DialogWindowLayout;

class PropBrw : public DockingWindow, public SfxListener, public SfxBroadcaster
{
private:
    bool        m_bInitialStateChange;

    css::uno::Reference< css::frame::XFrame2 >
                    m_xMeAsFrame;
    css::uno::Reference< css::beans::XPropertySet >
                    m_xBrowserController;
    css::uno::Reference< css::awt::XWindow >
                    m_xBrowserComponentWindow;
    css::uno::Reference< css::frame::XModel >
                    m_xContextDocument;

protected:
    SdrView*        pView;
    virtual void Resize() override;
    virtual bool Close() override;

    typedef std::vector< css::uno::Reference< css::uno::XInterface> > InterfaceArray;

    static css::uno::Sequence< css::uno::Reference< css::uno::XInterface > >
        CreateMultiSelectionSequence( const SdrMarkList& _rMarkList );
    void implSetNewObjectSequence( const css::uno::Sequence
        < css::uno::Reference< css::uno::XInterface > >& _rObjectSeq );

    void implSetNewObject( const css::uno::Reference< css::beans::XPropertySet >& _rxObject);

    static OUString GetHeadlineName( const css::uno::Reference< css::beans::XPropertySet >& _rxObject);

public:
    explicit PropBrw (DialogWindowLayout&);
    virtual ~PropBrw();
    virtual void dispose() override;
    using Window::Update;
    // note: changing the Context document to an instance other than the one given in the ctor is not supported
    // currently
    void    Update( const SfxViewShell* pShell );

private:
    void    ImplUpdate( const css::uno::Reference< css::frame::XModel >& _rxContextDocument, SdrView* pView );
    void    ImplDestroyController();
    void    ImplReCreateController();
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_INC_PROPBRW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
