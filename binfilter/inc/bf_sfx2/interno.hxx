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
#ifndef _SFX_INTERNO_HXX
#define _SFX_INTERNO_HXX

#include <bf_so3/ipobj.hxx>
#include "bf_sfx2/objsh.hxx"
#include "bf_sfx2/sfx.hrc"

namespace binfilter {

struct SfxInPlaceObject_Impl;
class SfxObjectShell;
class INote;


//=========================================================================

class SfxInPlaceObject : public SvInPlaceObject
{
    SfxObjectShell* 	pObjShell;
    INote*              pNote;
    BOOL                bTriggerLinkTimer : 1;
    BOOL				bDisableViewScaling : 1;

#if _SOLAR__PRIVATE
private:

public:
#endif
                        SO2_DECL_STANDARD_CLASS(SfxInPlaceObject)
public:
    SfxObjectShell*     GetObjectShell() const { return pObjShell; }

protected:
    void                SetShell( SfxObjectShell *pObjSh );

    virtual void        FillClass( SvGlobalName * pClassName,
                                   ULONG * pClipFormat,
                                   String * pAppName,
                                   String * pLongUserName,
                                   String * pUserName,
                                   long nVersion = SOFFICE_FILEFORMAT_CURRENT
    ) const;

    virtual BOOL        InitNew( SvStorage * );         // Rekursiv
    virtual BOOL        Load( SvStorage * );            // Rekursiv
    virtual BOOL        Save();                         // Rekursiv
    virtual BOOL        SaveAs( SvStorage * pNewStg );  // Rekursiv
    virtual void        HandsOff();                     // Rekursiv
    virtual BOOL        SaveCompleted( SvStorage * );   // Rekursiv

    virtual ErrCode		Verb( long nVerbPos,
                            SvEmbeddedClient *pCaller,
                            Window *pWin,
                            const Rectangle *pWorkAreaPixel );
    virtual BOOL		Verb( long nVerbPos, const Rectangle *pWorkAreaPixel );
    virtual void        Embed( BOOL bEmbedded );
    virtual void        Open( BOOL bOpen );
    virtual void        InPlaceActivate( BOOL );
    virtual void		DocumentNameChanged( const String & rDocName );

    virtual 			~SfxInPlaceObject();

public:
                        SfxInPlaceObject();

    void                SetTriggerLinkTimer( BOOL bSet )
                        { bTriggerLinkTimer = bSet; }
    void                UpdateLinks();

    virtual void        SetVisArea( const Rectangle & rVisArea );

    void				SetModified( BOOL bSet );
    void				DisableViewScaling( BOOL bSet )
                        { bDisableViewScaling = bSet; }
    BOOL				IsViewScalingDisabled()
                        { return bDisableViewScaling; }
};

#ifndef SFX_DECL_INTERNALOBJECT_DEFINED
#define SFX_DECL_INTERNALOBJECT_DEFINED
SO2_DECL_REF(SfxInPlaceObject)
#endif
SO2_IMPL_REF(SfxInPlaceObject)

#if _SOLAR__PRIVATE
class SfxForceLinkTimer_Impl
{
    SfxInPlaceObject *pInObj;

public:
    SfxForceLinkTimer_Impl( SfxObjectShell *pObj );
    ~SfxForceLinkTimer_Impl()
    { if( pInObj ) pInObj->SetTriggerLinkTimer( TRUE ); }
};
#endif


}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
