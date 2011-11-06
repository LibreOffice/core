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


#ifndef DBAUI_UNDOSQLEDIT_HXX
#define DBAUI_UNDOSQLEDIT_HXX

#ifndef DBAUI_GENERALUNDO_HXX
#include "GeneralUndo.hxx"
#endif
#ifndef _DBU_CONTROL_HRC_
#include "dbu_control.hrc"
#endif

namespace dbaui
{
    class OSqlEdit;
    // ================================================================================================
    // OSqlEditUndoAct - Undo-class for changing sql text
    //------------------------------------------------------------------------
    class OSqlEditUndoAct : public OCommentUndoAction
    {
    protected:
        OSqlEdit*   m_pOwner;
        String      m_strNextText;

        virtual void    Undo() { ToggleText(); }
        virtual void    Redo() { ToggleText(); }

        void ToggleText();
    public:
        OSqlEditUndoAct(OSqlEdit* pEdit) : OCommentUndoAction(STR_QUERY_UNDO_MODIFYSQLEDIT), m_pOwner(pEdit) { }

        void SetOriginalText(const String& strText) { m_strNextText =strText; }
    };
}
#endif // DBAUI_UNDOSQLEDIT_HXX


