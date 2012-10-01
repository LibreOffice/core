/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SC_ADIASYNC_HXX
#define SC_ADIASYNC_HXX

#include <svl/broadcast.hxx>
#include <set>

#include "callform.hxx"

extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData );
}

class ScDocument;
class ScAddInDocs : public std::set<ScDocument*> {};

class String;

class ScAddInAsync : public SvtBroadcaster
{
private:
    union
    {
        double      nVal;               // current value
        String*     pStr;
    };
    ScAddInDocs*    pDocs;              // List of using documents
    FuncData*       mpFuncData;         // Pointer to data in collection
    sal_uLong       nHandle;            // is casted from double to sal_uLong
    ParamType       meType;             // result of type PTR_DOUBLE or PTR_STRING
    bool            bValid;             // is value valid?

public:
    // cTor only if ScAddInAsync::Get fails.
    // nIndex: Index from FunctionCollection
    ScAddInAsync(sal_uLong nHandle, FuncData* pFuncData, ScDocument* pDoc);
                    // default-cTor only for that single, global aSeekObj!
                    ScAddInAsync();
    virtual         ~ScAddInAsync();
    static ScAddInAsync*    Get( sal_uLong nHandle );
    static void     CallBack( sal_uLong nHandle, void* pData );
    static void     RemoveDocument( ScDocument* pDocument );
    bool            IsValid() const         { return bValid; }
    ParamType       GetType() const         { return meType; }
    double          GetValue() const        { return nVal; }
    const String&   GetString() const       { return *pStr; }
    bool            HasDocument( ScDocument* pDoc ) const
                        { return pDocs->find( pDoc ) != pDocs->end(); }
    void            AddDocument( ScDocument* pDoc ) { pDocs->insert( pDoc ); }

    // Comparators for PtrArrSort
    bool operator< ( const ScAddInAsync& r ) const { return nHandle <  r.nHandle; }
    bool operator==( const ScAddInAsync& r ) const { return nHandle == r.nHandle; }
};

struct CompareScAddInAsync
{
  bool operator()( ScAddInAsync* const& lhs, ScAddInAsync* const& rhs ) const { return (*lhs)<(*rhs); }
};
class ScAddInAsyncs : public std::set<ScAddInAsync*, CompareScAddInAsync> {};

extern ScAddInAsyncs theAddInAsyncTbl;  // in adiasync.cxx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
