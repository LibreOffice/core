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


#ifndef DBAUI_QUERYTABCONNUNDOACTION_HXX
#define DBAUI_QUERYTABCONNUNDOACTION_HXX

#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#include "QueryDesignUndoAction.hxx"
#endif

namespace dbaui
{
    class OQueryTableConnection;
    class OQueryTableView;
    class OQueryTabConnUndoAction : public OQueryDesignUndoAction
    {
    protected:
        OQueryTableConnection*  m_pConnection;
        sal_Bool                    m_bOwnerOfConn;
            // bin ich alleiniger Eigentuemer der Connection ? (aendert sich mit jedem Redo oder Undo)

    public:
        OQueryTabConnUndoAction(OQueryTableView* pOwner, sal_uInt16 nCommentID);
        virtual ~OQueryTabConnUndoAction();

        virtual void Undo() = 0;
        virtual void Redo() = 0;

        void SetConnection(OQueryTableConnection* pConn) { m_pConnection = pConn; }
            // anschliessend bitte SetOwnership
        void SetOwnership(sal_Bool bTakeIt) { m_bOwnerOfConn = bTakeIt; }
    };
}
#endif // DBAUI_QUERYTABCONNUNDOACTION_HXX


