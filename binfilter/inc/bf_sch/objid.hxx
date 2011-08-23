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

#ifndef _SCH_OBJID_HXX
#define _SCH_OBJID_HXX

#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SVDITER_HXX //autogen
#include <bf_svx/svditer.hxx>
#endif
namespace binfilter {


/*************************************************************************
|*
|* Id-Objekt fuer Chart-Grafik-Objekte
|*
\************************************************************************/

class SchObjectId : public SdrObjUserData
{
    UINT16 nObjId;	// Id-Wert

public:
    SchObjectId();
    SchObjectId(UINT16 nId);

        virtual SdrObjUserData* Clone(SdrObject *pObj) const;

    virtual void WriteData(SvStream& rOut);
    virtual void ReadData(SvStream& rIn);

        void SetObjId(UINT16 nId) {  nObjId = nId; }
    UINT16 GetObjId() { return nObjId; }
};

/*************************************************************************
|*
|* Tool-Funktionen fuer Objekt-Ids
|*
\************************************************************************/

extern SchObjectId* GetObjectId(const SdrObject& rObj);

extern SdrObject* GetObjWithId(UINT16 nObjId, const SdrObjList& rObjList,
                                                           ULONG* pIndex = NULL,
                                                           SdrIterMode eMode = IM_FLAT);

//Die hier folgenden Funktionen dienen AUSSCHLIESSLICH zu DEBUG-Zwecken
#ifdef DBG_UTIL
extern char *GetCHOBJIDName(const long id);
#endif

} //namespace binfilter
#endif	// _SCH_OBJID_HXX


