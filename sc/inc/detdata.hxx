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

#ifndef INCLUDED_SC_INC_DETDATA_HXX
#define INCLUDED_SC_INC_DETDATA_HXX

#include "global.hxx"

#include <memory>
#include <vector>

enum ScDetOpType
{
    SCDETOP_ADDSUCC,
    SCDETOP_DELSUCC,
    SCDETOP_ADDPRED,
    SCDETOP_DELPRED,
    SCDETOP_ADDERROR
};

class ScDetOpData
{
    ScAddress       aPos;
    ScDetOpType const eOperation;

public:
                        ScDetOpData( const ScAddress& rP, ScDetOpType eOp ) :
                            aPos(rP), eOperation(eOp) {}

                        ScDetOpData( const ScDetOpData& rData ) :
                            aPos(rData.aPos), eOperation(rData.eOperation) {}

    const ScAddress&    GetPos() const          { return aPos; }
    ScDetOpType         GetOperation() const    { return eOperation; }

    // for UpdateRef:
    void                SetPos(const ScAddress& rNew)   { aPos=rNew; }

    bool operator==     ( const ScDetOpData& r ) const
                        { return eOperation == r.eOperation && aPos == r.aPos; }
};

//  list of operators

typedef std::vector<std::unique_ptr<ScDetOpData>> ScDetOpDataVector;

class ScDetOpList
{
    bool              bHasAddError;       // updated in append
    ScDetOpDataVector aDetOpDataVector;

public:
        ScDetOpList() : bHasAddError(false) {}
        ScDetOpList(const ScDetOpList& rList);

    void    DeleteOnTab( SCTAB nTab );
    void    UpdateReference( const ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                             const ScRange& rRange, SCCOL nDx, SCROW nDy, SCTAB nDz );

    bool        operator==( const ScDetOpList& r ) const;       // for ref-undo

    void        Append( ScDetOpData* pData );
    ScDetOpDataVector&  GetDataVector() { return aDetOpDataVector; }
    const ScDetOpData& GetObject( size_t nPos ) const;

    bool        HasAddError() const     { return bHasAddError; }
    size_t      Count() const { return aDetOpDataVector.size(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
