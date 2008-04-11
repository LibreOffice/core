/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basdoc.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _BASDOC_HXX
#define _BASDOC_HXX

#include <svx/ifaceids.hxx>
#include <iderid.hxx>
#include <sfx2/objsh.hxx>

class SfxPrinter;

class BasicDocShell: public SfxObjectShell
{
    SfxPrinter*         pPrinter;

protected:
    virtual void        FillStatusBar( StatusBar& rBar);
    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                          USHORT nAspect = ASPECT_CONTENT );
    virtual void    FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               sal_Int32 nVersion ) const;

public:
                        TYPEINFO();

                        using SotObject::GetInterface;
                        SFX_DECL_OBJECTFACTORY();
                        SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_DOCSH )
                        BasicDocShell( SfxObjectCreateMode eMode = SFX_CREATE_MODE_STANDARD );
                        ~BasicDocShell();

    void                Execute( SfxRequest& rReq );
    void                GetState( SfxItemSet& rSet );

    SfxPrinter*         GetPrinter( BOOL bCreate );
    void                SetPrinter( SfxPrinter* pPrinter );
};

#endif  // _BASDOC_HXX
