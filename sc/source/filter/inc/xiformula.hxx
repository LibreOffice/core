/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

