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

#ifndef _SD_GRDOCSH_HXX
#define _SD_GRDOCSH_HXX

#include <bf_sfx2/docfac.hxx>
#include <bf_sfx2/interno.hxx>
#include <bf_sfx2/objsh.hxx>

#include "docshell.hxx"
#include "glob.hxx"
#include "sdmod.hxx"
#include "pres.hxx"
namespace binfilter {


/*************************************************************************
|*
|* Document-Shell fuer Draw-Dokumente
|*
\************************************************************************/

class SdGraphicDocShell : public SdDrawDocShell
{
 public:
    TYPEINFO();

    SFX_DECL_OBJECTFACTORY_DLL(SdGraphicDocShell, SD_MOD());

    SdGraphicDocShell(SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
                   BOOL bSdDataObj=FALSE, DocumentType=DOCUMENT_TYPE_DRAW);
    SdGraphicDocShell(SdDrawDocument* pDoc, SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
                   BOOL bSdDataObj=FALSE, DocumentType=DOCUMENT_TYPE_DRAW);
    ~SdGraphicDocShell();
};

#ifndef SV_DECL_SDGRAPHICDOCSHELL_DEFINED
#define SV_DECL_SDGRAPHICDOCSHELL_DEFINED
SV_DECL_REF(SdGraphicDocShell)
#endif

SV_IMPL_REF (SdGraphicDocShell)


} //namespace binfilter
#endif          // _SD_GRDOCSH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
