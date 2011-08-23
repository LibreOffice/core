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

#ifndef _OUTPLACE_HXX
#define _OUTPLACE_HXX

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#include <bf_so3/ipobj.hxx>

#ifndef INCLUDED_SO3DLLAPI_H
#include "bf_so3/so3dllapi.h"
#endif

//=========================================================================
namespace binfilter
{
    struct SvOutPlace_Impl;
    class SvObjectServer;

class SO3_DLLPUBLIC SvOutPlaceObject : public SvInPlaceObject
/*	[Beschreibung]

    Die Klasse SvOutPlaceObject ist ein Wrapper fuer alle Windows Ole-Objekte

    [Probleme]
*/
{
private:
    SvOutPlace_Impl *	pImpl;

    SO3_DLLPRIVATE void			DataChanged_Impl( BOOL bOnlyEmbedSource );
    SO3_DLLPRIVATE void			LoadSO_Cont();
    SO3_DLLPRIVATE BOOL			MakeWorkStorageWrap_Impl( SvStorage * pStor );

protected:
    SO3_DLLPRIVATE virtual void    FillClass( SvGlobalName * pClassName,
                               ULONG * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               long nFileFormat = SOFFICE_FILEFORMAT_CURRENT ) const;
                    // Protokoll
    SO3_DLLPRIVATE virtual void    Open( BOOL bOpen );
    SO3_DLLPRIVATE virtual void    Embed( BOOL bEmbedded );
    SO3_DLLPRIVATE virtual ErrCode Verb( long, SvEmbeddedClient * pCallerClient,
                        Window * pWin, const Rectangle * pWorkRectPixel );

                    // Datenaustausch
    SO3_DLLPRIVATE virtual void    Draw( OutputDevice *,
                          const JobSetup & rSetup,
                          USHORT nAspect = ASPECT_CONTENT );

                    // Laden speichern
    SO3_DLLPRIVATE virtual BOOL    InitNew( SvStorage * );
    SO3_DLLPRIVATE virtual BOOL    Load( SvStorage * );
    SO3_DLLPRIVATE virtual BOOL    Save();
    SO3_DLLPRIVATE virtual BOOL    SaveAs( SvStorage * );
    SO3_DLLPRIVATE virtual void    HandsOff();
    SO3_DLLPRIVATE virtual BOOL    SaveCompleted( SvStorage * );

    SO3_DLLPRIVATE 				~SvOutPlaceObject();
public:
                    SO2_DECL_BASIC_CLASS(SvOutPlaceObject)
                    SvOutPlaceObject();
    static SvInPlaceObjectRef	InsertObject( Window *, SvStorage * pIStorage,
                                            BOOL & bOut, const SvGlobalName & rName, String & rFileName );
#ifdef WNT
    static SvInPlaceObjectRef   CreateFromData( const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::datatransfer::XTransferable>&,
                                                SvStorage*);
    static SvGlobalName			GetCLSID( const String & rFileName );
#endif
    static const ::binfilter::SvObjectServer* GetInternalServer_Impl( const SvGlobalName& aGlobName );

    void			ClearCache();
    SotStorage *	GetWorkingStorage();
    virtual void    SetVisArea( const Rectangle & rVisArea );
    using SvEmbeddedObject::GetVisArea;
    Rectangle		GetVisArea( USHORT dwAspect ) const;
    void			ViewChanged( USHORT nAspects );
    virtual const SvVerbList & GetVerbList() const;

    virtual UINT32		GetViewAspect() const;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent > GetUnoComponent() const;
    SvGlobalName		GetObjectCLSID();
    void    DrawObject( OutputDevice *,
                          const JobSetup & rSetup,
                          const Size& rSize,
                          USHORT nAspect = ASPECT_CONTENT );

};

SO2_DECL_IMPL_REF(SvOutPlaceObject)

}

#endif // _OUTPLACE_HXX

