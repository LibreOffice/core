/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

class ScAddInAsync : public SvtBroadcaster
{
private:
    union
    {
        double      nVal;               // current value
        OUString*   pStr;
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
    const OUString& GetString() const       { return *pStr; }
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
