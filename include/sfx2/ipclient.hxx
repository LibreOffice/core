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
#ifndef INCLUDED_SFX2_IPCLIENT_HXX
#define INCLUDED_SFX2_IPCLIENT_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <rtl/ref.hxx>

#include <tools/gen.hxx>

#include <vcl/window.hxx>
#include <vcl/errcode.hxx>

namespace com::sun::star::embed { class XEmbeddedObject; }
namespace com::sun::star::uno { template <typename > class Reference; }

class SfxInPlaceClient_Impl;
class SfxViewShell;
class SfxObjectShell;
class Fraction;


class SFX2_DLLPUBLIC SfxInPlaceClient
{
friend class SfxInPlaceClient_Impl;

    rtl::Reference<SfxInPlaceClient_Impl>  m_xImp;
    SfxViewShell*           m_pViewSh;
    VclPtr<vcl::Window>     m_pEditWin;

    // called after the requested new object area was negotiated
    SAL_DLLPRIVATE virtual void ObjectAreaChanged();

    // an active object was resized by the user and now asks for the new space
    SAL_DLLPRIVATE virtual void RequestNewObjectArea( tools::Rectangle& );

    // notify the client that an active object has changed its VisualAreaSize
    SAL_DLLPRIVATE virtual void ViewChanged();

public:
                        SfxInPlaceClient( SfxViewShell* pViewShell, vcl::Window* pDraw, sal_Int64 nAspect );
    virtual             ~SfxInPlaceClient();

    SfxInPlaceClient(const SfxInPlaceClient &) = delete;
    SfxInPlaceClient& operator=(const SfxInPlaceClient &) = delete;

    SfxViewShell*       GetViewShell() const { return m_pViewSh; }
    vcl::Window*        GetEditWin() const { return m_pEditWin; }
    const css::uno::Reference < css::embed::XEmbeddedObject >& GetObject() const;
    void                SetObject( const css::uno::Reference < css::embed::XEmbeddedObject >& rObject );
    void                SetObjectState( sal_Int32 );
    bool                IsObjectUIActive() const;
    bool                IsObjectInPlaceActive() const;
    void                DeactivateObject();
    bool                SetObjArea( const tools::Rectangle & );
    const tools::Rectangle&    GetObjArea() const;
    tools::Rectangle           GetScaledObjArea() const;
    void                SetSizeScale( const Fraction & rScaleWidth, const Fraction & rScaleHeight );
    void                SetObjAreaAndScale( const tools::Rectangle&, const Fraction&, const Fraction& );
    const Fraction&     GetScaleWidth() const;
    const Fraction&     GetScaleHeight() const;
    void                Invalidate();
    static SfxInPlaceClient* GetClient( SfxObjectShell const * pDoc, const css::uno::Reference < css::embed::XEmbeddedObject >& xObject );
    sal_Int64           GetAspect() const;
    sal_Int64           GetObjectMiscStatus() const;
    ErrCode             DoVerb(sal_Int32 nVerb);
    void                VisAreaChanged();
    void                ResetObject();
    bool                IsUIActive() const;

    virtual void        FormatChanged(); // object format was changed (used for StarMath formulas aligning)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
