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

#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#define SD_GRAPHIC_DOC_SHELL_HXX

#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include "DrawDocShell.hxx"
#include "glob.hxx"
#include "sdmod.hxx"
#include "pres.hxx"

namespace sd {

/*************************************************************************
|*
|* Document-Shell fuer Draw-Dokumente
|*
\************************************************************************/

class GraphicDocShell
    : public DrawDocShell
{
public:
    TYPEINFO();

    SFX_DECL_INTERFACE(SD_IF_SDGRAPHICDOCSHELL)
    SFX_DECL_OBJECTFACTORY();

    using SotObject::GetInterface;

    GraphicDocShell (
        SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
        sal_Bool bSdDataObj=sal_False,
        DocumentType=DOCUMENT_TYPE_DRAW);

    GraphicDocShell (
        const sal_uInt64 nModelCreationFlags,
        sal_Bool bSdDataObj=sal_False,
        DocumentType=DOCUMENT_TYPE_DRAW);

    virtual ~GraphicDocShell (void);
};

#ifndef SV_DECL_SD_GRAPHIC_DOC_SHELL_DEFINED
#define SV_DECL_SD_GRAPHIC_DOC_SHELL_DEFINED
SV_DECL_REF(GraphicDocShell)
#endif

SV_IMPL_REF (GraphicDocShell)

} // end of namespace sd

#endif
