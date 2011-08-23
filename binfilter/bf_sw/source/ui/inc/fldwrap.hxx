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
#ifndef _FLDWRAP_HXX
#define _FLDWRAP_HXX

#include "chldwrap.hxx"
namespace binfilter {

class SwFldDlgWrapper : public SwChildWinWrapper
{
public:
    SwFldDlgWrapper( Window* pParent, USHORT nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo ): SwChildWinWrapper( pParent, nId ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwFldDlgWrapper);

};
/* -----------------04.02.2003 14:14-----------------
 * field dialog only showing database page to support
 * mail merge 
 * --------------------------------------------------*/
class SwFldDataOnlyDlgWrapper : public SwChildWinWrapper
{
public:
    SwFldDataOnlyDlgWrapper( Window* pParent, USHORT nId,
                        SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SwFldDataOnlyDlgWrapper);

    virtual BOOL    ReInitDlg(SwDocShell *pDocSh);
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
