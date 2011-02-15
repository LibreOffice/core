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


