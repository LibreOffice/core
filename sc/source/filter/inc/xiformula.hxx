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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XIFORMULA_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XIFORMULA_HXX

#include "xlformula.hxx"
#include "xiroot.hxx"
#include <boost/shared_ptr.hpp>

// Formula compiler ===========================================================

class ScRangeList;
class XclImpFmlaCompImpl;

/** The formula compiler to create Calc token arrays from Excel token arrays. */
class XclImpFormulaCompiler : protected XclImpRoot
{
public:
    explicit            XclImpFormulaCompiler( const XclImpRoot& rRoot );
    virtual             ~XclImpFormulaCompiler();

    /** Creates a range list from the passed Excel token array.
        @param rStrm  Stream pointing to additional formula data (e.g. constant array data). */
    void                CreateRangeList(
                            ScRangeList& rScRanges, XclFormulaType eType,
                            const XclTokenArray& rXclTokArr, XclImpStream& rStrm );

    /**
     * Creates a formula token array from the Excel token array.  Note that
     * the caller must create a copy of the token array instance returend by
     * this function if the caller needs to persistently store the array,
     * because the pointer points to an array instance on the stack.
     */
    const ScTokenArray* CreateFormula( XclFormulaType eType, const XclTokenArray& rXclTokArr );

private:
    typedef boost::shared_ptr< XclImpFmlaCompImpl > XclImpFmlaCompImplRef;
    XclImpFmlaCompImplRef mxImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
