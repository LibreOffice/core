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


#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#define DBAUI_QUERYDESIGNUNDOACTION_HXX

#ifndef DBAUI_GENERALUNDO_HXX
#include "GeneralUndo.hxx"
#endif

namespace dbaui
{
    // ================================================================================================
    // OQueryDesignUndoAction - Undo-Basisklasse fuer Aktionen im graphischen Abfrageentwurf (ohne Feldliste)

    class OJoinTableView;
    class OQueryDesignUndoAction : public OCommentUndoAction
    {
    protected:
        OJoinTableView* m_pOwner;       // in diesem Container spielt sich alles ab

    public:
        OQueryDesignUndoAction(OJoinTableView* pOwner, sal_uInt16 nCommentID) : OCommentUndoAction(nCommentID), m_pOwner(pOwner) { }
    };
}
#endif // DBAUI_QUERYDESIGNUNDOACTION_HXX

