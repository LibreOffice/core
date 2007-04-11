/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ipclient.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:21:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFX_IPCLIENT_HXX
#define _SFX_IPCLIENT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

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
    BOOL                SetObjArea( const Rectangle & );
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
    BOOL                IsUIActive();

    // used in Writer
    // Rectangle           PixelObjVisAreaToLogic( const Rectangle & rObjRect ) const;
    // Rectangle           LogicObjAreaToPixel( const Rectangle & rRect ) const;
};

#endif

