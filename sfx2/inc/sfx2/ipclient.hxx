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
#ifndef _SFX_IPCLIENT_HXX
#define _SFX_IPCLIENT_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <tools/ref.hxx>
#include <tools/gen.hxx>

#include <sfx2/objsh.hxx>

class SfxInPlaceClient_Impl;
class SfxViewShell;
class SfxObjectShell;
class Window;
class Fraction;

//=========================================================================

class SFX2_DLLPUBLIC SfxInPlaceClient
{
friend class SfxInPlaceClient_Impl;

    SfxInPlaceClient_Impl*  m_pImp;
    SfxViewShell*           m_pViewSh;
    Window*                 m_pEditWin;

    // called after the requested new object area was negotiated
    SAL_DLLPRIVATE virtual void ObjectAreaChanged();

    // an active object was resized by the user and now asks for the new space
    SAL_DLLPRIVATE virtual void RequestNewObjectArea( Rectangle& );

    // notify the client that an active object has changed its VisualAreaSize
    SAL_DLLPRIVATE virtual void ViewChanged();

    // an object wants to become visible
    SAL_DLLPRIVATE virtual void MakeVisible();

public:
                        SfxInPlaceClient( SfxViewShell* pViewShell, Window* pDraw, sal_Int64 nAspect = com::sun::star::embed::Aspects::MSOLE_CONTENT );
    virtual             ~SfxInPlaceClient();

    SfxViewShell*       GetViewShell() const { return m_pViewSh; }
    Window*             GetEditWin() const { return m_pEditWin; }
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetObject() const;
    void                SetObject( const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& rObject );
    void                SetObjectState( sal_Int32 );
    Size                GetObjectVisAreaSize() const;
    sal_Bool            IsObjectUIActive() const;
    sal_Bool            IsObjectInPlaceActive() const;
    sal_Bool            IsObjectActive() const;
    void                DeactivateObject();
    sal_Bool                SetObjArea( const Rectangle & );
    Rectangle           GetObjArea() const;
    Rectangle           GetScaledObjArea() const;
    void                SetSizeScale( const Fraction & rScaleWidth, const Fraction & rScaleHeight );
    sal_Bool            SetObjAreaAndScale( const Rectangle&, const Fraction&, const Fraction& );
    const Fraction&     GetScaleWidth() const;
    const Fraction&     GetScaleHeight() const;
    void                Invalidate();
    static Window*      GetActiveWindow( SfxObjectShell* pDoc, const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObject );
    static SfxInPlaceClient* GetClient( SfxObjectShell* pDoc, const com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject >& xObject );
    sal_Int64           GetAspect() const;
    sal_Int64           GetObjectMiscStatus() const;
    ErrCode             DoVerb( long nVerb );
    void                VisAreaChanged();
    void                ResetObject();
    sal_Bool                IsUIActive();

    virtual void        FormatChanged(); // object format was changed (used for StarMath formulas aligning)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
