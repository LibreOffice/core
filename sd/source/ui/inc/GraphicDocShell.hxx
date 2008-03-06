/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GraphicDocShell.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:58:19 $
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

#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#define SD_GRAPHIC_DOC_SHELL_HXX

#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _SDMOD_HXX
#include "sdmod.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif

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
        BOOL bSdDataObj=FALSE,
        DocumentType=DOCUMENT_TYPE_DRAW,
        BOOL bScriptSupport=TRUE);

    virtual ~GraphicDocShell (void);
};

#ifndef SV_DECL_SD_GRAPHIC_DOC_SHELL_DEFINED
#define SV_DECL_SD_GRAPHIC_DOC_SHELL_DEFINED
SV_DECL_REF(GraphicDocShell)
#endif

SV_IMPL_REF (GraphicDocShell)

} // end of namespace sd

#endif
