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


#ifndef DBAUI_OQUERYMOVETABWINUNDOACT_HXX
#define DBAUI_OQUERYMOVETABWINUNDOACT_HXX

#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#include "QueryDesignUndoAction.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

namespace dbaui
{

    // ================================================================================================
    // OQueryMoveTabWinUndoAct - Undo-Klasse fuer Verschieben eines TabWins
    class OQueryTableWindow;
    class OTableWindow;
    class OJoinMoveTabWinUndoAct : public OQueryDesignUndoAction
    {
        Point           m_ptNextPosition;
        OTableWindow*   m_pTabWin;

    protected:
        void TogglePosition();

    public:
        OJoinMoveTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPosition, OTableWindow* pTabWin);

        virtual void    Undo() { TogglePosition(); }
        virtual void    Redo() { TogglePosition(); }
    };

    // ------------------------------------------------------------------------------------------------
    inline OJoinMoveTabWinUndoAct::OJoinMoveTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPosition, OTableWindow* pTabWin)
        :OQueryDesignUndoAction(pOwner, STR_QUERY_UNDO_MOVETABWIN)
        ,m_ptNextPosition(ptOriginalPosition)
        ,m_pTabWin(pTabWin)
    {
    }
}
#endif // DBAUI_OQUERYMOVETABWINUNDOACT_HXX


