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

#ifndef SC_XIFORMULA_HXX
#define SC_XIFORMULA_HXX

#include "xlformula.hxx"
#include "xiroot.hxx"

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
    typedef ScfRef< XclImpFmlaCompImpl > XclImpFmlaCompImplRef;
    XclImpFmlaCompImplRef mxImpl;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
