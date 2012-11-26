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


#ifndef DBAUI_GENERALUNDO_HXX
#define DBAUI_GENERALUNDO_HXX

#ifndef _UNDO_HXX
#include <svl/undo.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

namespace dbaui
{
    // ================================================================================================
    // SbaCommentUndoAction - Undo-Basisklasse fuer Aktionen, deren GetComment einen aus einer Sba-Ressource
    // geladenen String liefert

    class OCommentUndoAction : public SfxUndoAction
    {
        OModuleClient    m_aModuleClient;

    protected:
        String                  m_strComment; // undo, redo comment

    public:
        OCommentUndoAction(sal_uInt16 nCommentID) { m_strComment = String(ModuleRes(nCommentID)); }

        virtual UniString   GetComment() const { return m_strComment; }
    };
}
#endif // DBAUI_GENERALUNDO_HXX


