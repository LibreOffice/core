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

#pragma once

#include <formula/opcode.hxx>
#include <formula/paramclass.hxx>

/** Activate parameter classification documentation.

    Building with DEBUG_SC_PARCLASSDOC 1 enables generation of parameter
    classification documentation when instantiating the first Calc document if
    the environment variable OOO_CALC_GENPARCLASSDOC is set and SAL_LOG
    contains +INFO.sc.core

    Generated output contains CALC_GENPARCLASSDOC that can be easily grep'ed for.

    Activation adds overhead to Calc initialization time to gather information
    that otherwise is not needed for anything else.
 */
#define DEBUG_SC_PARCLASSDOC 0

namespace formula
{
    class FormulaToken;
}

class ScParameterClassification
{
public:

                                /// MUST be called once before any other method.
    static  void                Init();

    static  void                Exit();

                                /** Get one parameter type for function eOp.
                                    @param nParameter
                                        Which parameter, 0-based.
                                        SAL_MAX_UINT16 for return type of eOp.
                                 */
    static  formula::ParamClass GetParameterType( const formula::FormulaToken* pToken,
                                        sal_uInt16 nParameter);

                                /** Whether OpCode has a parameter of type
                                    ForceArray or ReferenceOrForceArray. */
    static  bool        HasForceArray( OpCode eOp)
                                    {
                                        return 0 <= static_cast<short>(eOp) &&
                                            eOp <= SC_OPCODE_LAST_OPCODE_ID &&
                                            pData[eOp].bHasForceArray;
                                    }

private:

    struct CommonData
    {
        const static sal_Int32 nMaxParams = 7;

        formula::ParamClass nParam[nMaxParams];
        sal_uInt8           nRepeatLast;
        formula::ParamClass eReturn;
    };

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
    static  formula::ParamClass GetExternalParameterType(
                                    const formula::FormulaToken* pToken, sal_uInt16 nParameter);

#if DEBUG_SC_PARCLASSDOC
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
                                                == formula::ParamClass::Unknown ? 1 :
                                                pData[eOp].nMinParams;
                                        return 0;
                                    }

                                /** Whether last parameter types are repeated. */
    static  inline  bool        HasRepeatParameters( OpCode eOp)
                                    {
                                        return eOp <= SC_OPCODE_LAST_OPCODE_ID
                                            && pData[eOp].aData.nRepeatLast > 0;
                                    }
#endif // DEBUG_SC_PARCLASSDOC
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
