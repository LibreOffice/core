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
#ifndef _SWDRWBASSH_HXX
#define _SWDRWBASSH_HXX
#include "basesh.hxx"

class SwView;
class SfxItemSet;
class SwDrawBase;
class AbstractSvxNameDialog;
struct SvxSwFrameValidation;

class SwDrawBaseShell: public SwBaseShell
{
    SwDrawBase* pDrawActual;

    sal_uInt16      eDrawMode;
    sal_Bool        bRotate : 1;
    sal_Bool        bSelMove: 1;

    DECL_LINK( CheckGroupShapeNameHdl, AbstractSvxNameDialog* );
    DECL_LINK(ValidatePosition, SvxSwFrameValidation* );
public:
                SwDrawBaseShell(SwView &rShell);
    virtual     ~SwDrawBaseShell();

    SFX_DECL_INTERFACE(SW_DRAWBASESHELL)
    TYPEINFO();

    void        Execute(SfxRequest &);
    void        GetState(SfxItemSet &);
    void        DisableState(SfxItemSet &rSet)               { Disable(rSet);}
    sal_Bool        Disable(SfxItemSet& rSet, sal_uInt16 nWhich = 0);

    void        StateStatusline(SfxItemSet &rSet);

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
