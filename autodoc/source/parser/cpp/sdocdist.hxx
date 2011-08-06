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

#ifndef ADC_CPP_SDOCDIST_HXX
#define ADC_CPP_SDOCDIST_HXX



// USED SERVICES
    // BASE CLASSES
#include "cxt2ary.hxx"
#include <ary/info/docstore.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{

using ary::Documentation;

/** Implementation struct for cpp::ContextForAry.
*/

struct ContextForAry::S_DocuDistributor : public ary::info::DocuStore
{
  public:
                        S_DocuDistributor()     :   pCurRe(0) {}
                        ~S_DocuDistributor()    {}

    void                Reset()                 { pCurRe = 0; pLastStoredDocu = 0; }

    void                SetCurrentlyStoredRe(
                            ary::cpp::CppEntity &
                                                io_rRe );
    void                Event_LineBreak();

  private:
    // Interface ary::info::DocuStore
    virtual void        do_Store2CurFile(
                            DYN ary::doc::Node& let_drDocu );
    virtual void        do_Store2CurNamespace(
                            DYN ary::doc::Node& let_drDocu );

    virtual void        do_Store2ConnectedDeclaration(
                            DYN ary::doc::Node& let_drDocu );

    virtual void        do_Store2Glossary(
                            DYN ary::doc::Node& let_drDocu,
                            const String  &     i_sExplainedTerm );
    virtual void        do_Store2GlobalTexts(
                            DYN ary::doc::Node& let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId );
    // DATA
    ary::cpp::CppEntity *
                        pCurRe;
    Dyn<ary::doc::Node> pLastStoredDocu;
};


// IMPLEMENTATION

/*  The implementation is in header, though not all inline, because this file
    is included in cxt2ary.cxx only!
*/


void
ContextForAry::
S_DocuDistributor::SetCurrentlyStoredRe( ary::cpp::CppEntity & io_rRe )
{
    pCurRe = &io_rRe;
    if ( pLastStoredDocu )
        pCurRe->Set_Docu( *pLastStoredDocu.Release() );
}

inline void
ContextForAry::
S_DocuDistributor::Event_LineBreak()
{
    pCurRe = 0;
}

void
ContextForAry::
S_DocuDistributor::do_Store2CurFile( DYN ary::doc::Node & let_drDocu )
{
    // KORR_FUTURE
    delete &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2CurNamespace( DYN ary::doc::Node & let_drDocu )
{
    // KORR_FUTURE
    delete &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2ConnectedDeclaration( DYN ary::doc::Node & let_drDocu )
{
    if ( pCurRe != 0 )
        pCurRe->Set_Docu(let_drDocu);
    else
        pLastStoredDocu = &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2Glossary( DYN ary::doc::Node & let_drDocu,
                                      const String &           // i_sExplainedTerm
                                      )
{
    // KORR_FUTURE
    delete &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2GlobalTexts( DYN ary::doc::Node &       let_drDocu,
                                         ary::info::GlobalTextId    // i_nId
                                       )
{
    // KORR_FUTURE
    delete &let_drDocu;
}




}   // namespace cpp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
