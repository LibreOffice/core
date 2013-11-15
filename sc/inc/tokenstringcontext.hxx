/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_TOKENSTRINGCONTEXT_HXX
#define SC_TOKENSTRINGCONTEXT_HXX

#include "compiler.hxx"

#include <boost/unordered_map.hpp>

class ScDocument;

namespace sc {

/**
 * Context for creating string from an array of formula tokens, used in
 * ScTokenArray::CreateString().  You can re-use the same string context
 * between multiple CreateString() calls as long as the document content is
 * unmodified.
 */
struct SC_DLLPUBLIC TokenStringContext
{
    typedef boost::unordered_map<sal_uInt16, OUString> IndexNameMapType;

    formula::FormulaGrammar::Grammar meGram;
    formula::FormulaCompiler::OpCodeMapPtr mxOpCodeMap;
    const ScCompiler::Convention* mpRefConv;
    OUString maErrRef;

    std::vector<OUString> maTabNames;
    IndexNameMapType maGlobalRangeNames;
    IndexNameMapType maNamedDBs;

    TokenStringContext( const ScDocument* pDoc, formula::FormulaGrammar::Grammar eGram );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
