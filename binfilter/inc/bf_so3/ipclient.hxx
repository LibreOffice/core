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

#ifndef _IPCLIENT_HXX
#define _IPCLIENT_HXX

#include <bf_so3/client.hxx>

#include "bf_so3/so3dllapi.h"

/*************************************************************************
*************************************************************************/
class  MenuBar;
class  Accelerator;
struct IOleInPlaceSite;
struct IOleInPlaceFrame;
struct IOleInPlaceUIWindow;

namespace binfilter {
class  SvInPlaceEnvironment;
class SvContainerEnvironment;
class  SvInPlaceClientList;
class  SvInPlaceClient;
class  SvInPlaceClient;
class  SvInPlaceObject;
class  ImpInPlaceSite;
class  ImpInPlaceFrame;
class  ImpInPlaceUIWin;

    //=========================================================================
class SO3_DLLPUBLIC SvInPlaceClient : public SvEmbeddedClient
{
friend class ImpInPlaceSite;
friend class ImpInPlaceFrame;
friend class ImpInPlaceUIWin;
friend class SvInPlaceObject;
friend class SvEditObjectProtocol;
friend class ImplSvEditObjectProtocol;
friend class SvContainerEnvironment;

    IOleInPlaceSite *       pObjI;

    SO3_DLLPRIVATE 	BOOL            	CanInPlaceActivate() const;
protected:
                             ~SvInPlaceClient();

    virtual void    	Opened( BOOL bOpen );
    virtual void    	InPlaceActivate( BOOL bActivate );
    virtual void    	UIActivate( BOOL bActivate );

    virtual void		MakeViewData();
public:
                        SvInPlaceClient();
                        SO2_DECL_STANDARD_CLASS_DLL(SvInPlaceClient,SOAPP)

    IOleInPlaceSite *   		GetInPlaceSite() const;

    static SvInPlaceClientList & GetIPActiveClientList();

    SvContainerEnvironment * 	GetEnv();
    virtual void    			MakeVisible();

    BOOL            			IsInPlaceActive() const
                                { return aProt.IsInPlaceActive(); }
    SvInPlaceObject * 			GetIPObj() const
                                { return aProt.GetIPObj(); }

    virtual void 				DeactivateAndUndo();
    virtual void 				DiscardUndoState();
};
#ifndef SO2_DECL_SVINPLACECLIENT_DEFINED
#define SO2_DECL_SVINPLACECLIENT_DEFINED
SO2_DECL_REF(SvInPlaceClient)
#endif
SO2_IMPL_REF(SvInPlaceClient)

SV_DECL_REF_LIST(SvInPlaceClient,SvInPlaceClient*)
SV_IMPL_REF_LIST(SvInPlaceClient,SvInPlaceClient*)

}

#endif // _IPCLIENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
