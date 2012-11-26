/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    SFX_DECL_INTERFACE(SD_IF_SDGRAPHICDOCSHELL)
    SFX_DECL_OBJECTFACTORY();

    using SotObject::GetInterface;

    GraphicDocShell (
        SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED,
        bool bSdDataObj=false,
        DocumentType=DOCUMENT_TYPE_DRAW);

    GraphicDocShell (
        const sal_uInt64 nModelCreationFlags,
        bool bSdDataObj=false,
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
