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

#ifndef SC_DETDATA_HXX
#define SC_DETDATA_HXX

#include <boost/ptr_container/ptr_vector.hpp>
#include "global.hxx"

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
    ScDetOpType     eOperation;

public:
                        ScDetOpData( const ScAddress& rP, ScDetOpType eOp ) :
                            aPos(rP), eOperation(eOp) {}

                        ScDetOpData( const ScDetOpData& rData ) :
                            aPos(rData.aPos), eOperation(rData.eOperation) {}

    const ScAddress&    GetPos() const          { return aPos; }
    ScDetOpType         GetOperation() const    { return eOperation; }

    // for UpdateRef:
    void                SetPos(const ScAddress& rNew)   { aPos=rNew; }

    int operator==      ( const ScDetOpData& r ) const
                            { return eOperation == r.eOperation && aPos == r.aPos; }
};

//
//  list of operators
//

typedef boost::ptr_vector<ScDetOpData> ScDetOpDataVector;

class ScDetOpList
{
    sal_Bool    bHasAddError;       // updated in append
    ScDetOpDataVector aDetOpDataVector;

public:
        ScDetOpList() : bHasAddError(false) {}
        ScDetOpList(const ScDetOpList& rList);
        ~ScDetOpList() {}

    void    DeleteOnTab( SCTAB nTab );
    void    UpdateReference( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    sal_Bool    operator==( const ScDetOpList& r ) const;       // for ref-undo

    void         Append( ScDetOpData* pData );
    ScDetOpDataVector&  GetDataVector() { return aDetOpDataVector; }
    const ScDetOpData* GetObject( size_t nPos ) const;

    sal_Bool    HasAddError() const     { return bHasAddError; }
    size_t      Count() const { return aDetOpDataVector.size(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
