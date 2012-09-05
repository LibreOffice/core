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



#ifndef SC_PARCLASS_HXX
#define SC_PARCLASS_HXX

#include "formula/opcode.hxx"
#include <sys/types.h>  // size_t

namespace formula
{
    class FormulaToken;
}

class ScParameterClassification
{
public:

    enum Type
    {
        Unknown = 0,    // MUST be zero for initialization mechanism!

        /** Out of bounds, function doesn't expect that many parameters.
            However, not necessarily returned. */
        Bounds,

        /** In array formula: single value to be passed. Results in JumpMatrix
            being created and multiple calls to function. Functions handling a
            formula::svDoubleRef by means of DoubleRefToPosSingleRef() or
            PopDoubleRefOrSingleRef() or GetDouble() or GetString() should have
            this. */
        Value,

        /** In array formula: area reference must stay reference. Otherwise
            don't care. Functions handling a formula::svDoubleRef by means of
            PopDoubleRefOrSingleRef() should not have this. */
        Reference,

        /** In array formula: convert area reference to array. Function will be
            called only once if no Value type is involved. Functions able to
            handle a svMatrix parameter but not a formula::svDoubleRef parameter as area
            should have this. */
        Array,

        /** Area reference must be converted to array in any case, and must
            also be propagated to subsequent operators and functions being part
            of a parameter of this function. */
        ForceArray,

        /** Area reference is not converted to array, but ForceArray must be
            propagated to subsequent operators and functions being part of a
            parameter of this function. Used with functions that treat
            references separately from arrays, but need the forced array
            calculation of parameters that are not references.*/
        ReferenceOrForceArray
    };

                                /// MUST be called once before any other method.
    static  void                Init();

    static  void                Exit();

                                /** Get one parameter type for function eOp.
                                    @param nParameter
                                        Which parameter, 0-based */
    static  Type                GetParameterType( const formula::FormulaToken* pToken,
                                        sal_uInt16 nParameter);

                                /** Whether OpCode has a parameter of type
                                    ForceArray or ReferenceOrForceArray. */
    static  inline  bool        HasForceArray( OpCode eOp)
                                    {
                                        return 0 <= (short)eOp &&
                                            eOp <= SC_OPCODE_LAST_OPCODE_ID &&
                                            pData[eOp].bHasForceArray;
                                    }

private:

    struct CommonData
    {
        const static size_t nMaxParams = 7;

        Type        nParam[nMaxParams];
        sal_uInt8   nRepeatLast;
    };

    // SUNWS7 needs a forward declared friend, otherwise members of the outer
    // class are not accessible (in this case CommonData).
    struct RawData;
    friend struct ScParameterClassification::RawData;
    struct RawData
    {
        OpCode      eOp;
        CommonData  aData;
    };

    struct RunData;
    friend struct ScParameterClassification::RunData;
    struct RunData
    {
        CommonData  aData;
        sal_uInt8        nMinParams;         // fix or minimum, or repeat start
        bool        bHasForceArray;
    };

    static  const RawData       pRawData[];
    static  RunData*            pData;

    // ocExternal AddIns
    static  Type                GetExternalParameterType(
                                    const formula::FormulaToken* pToken, sal_uInt16 nParameter);

#if OSL_DEBUG_LEVEL > 1
    // Generate documentation to stdout if environment variable
    // OOO_CALC_GENPARCLASSDOC is set.
    static  void                GenerateDocumentation();

    /* OpCodes not specified in the implementation are taken from the global
     * function list and all parameters, if any, are assumed to be of type
     * Value. This could also be done in the product version if needed, but we
     * don't want to spoil startup time. However, doing so could propagate the
     * minimum parameter count to the formula compiler, which, together with
     * additional information about optional parameters, could react on missing
     * parameters then. */
    static  void                MergeArgumentsFromFunctionResource();

                                /** Minimum number of parameters, or fix number
                                    of parameters if HasRepeatParameters()
                                    returns sal_False. For opcodes not specified in
                                    the implementation a parameter count of 1
                                    is assumed, for opcodes out of range 0 is
                                    assumed. If HasRepeatParameters() returns
                                    sal_True, information is NOT related to whether
                                    any parameters are optional, only the type
                                    of parameters is significant. */
    static  inline  sal_uInt8        GetMinimumParameters( OpCode eOp)
                                    {
                                        if ( eOp <= SC_OPCODE_LAST_OPCODE_ID )
                                            return pData[eOp].aData.nParam[0]
                                                == Unknown ? 1 :
                                                pData[eOp].nMinParams;
                                        return 0;
                                    }

                                /** Whether last parameter types are repeated. */
    static  inline  bool        HasRepeatParameters( OpCode eOp)
                                    {
                                        return eOp <= SC_OPCODE_LAST_OPCODE_ID
                                            && pData[eOp].aData.nRepeatLast > 0;
                                    }
#endif // OSL_DEBUG_LEVEL
};

#endif // SC_PARCLASS_HXX

