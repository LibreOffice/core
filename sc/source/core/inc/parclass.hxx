/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parclass.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 13:12:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_PARCLASS_HXX
#define SC_PARCLASS_HXX

#ifndef SC_OPCODE_HXX
#include "opcode.hxx"
#endif
#include <sys/types.h>  // size_t

class ScToken;

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
            svDoubleRef by means of DoubleRefToPosSingleRef() or
            PopDoubleRefOrSingleRef() or GetDouble() or GetString() should have
            this. */
        Value,

        /** In array formula: area reference must stay reference. Otherwise
            don't care. Functions handling a svDoubleRef by means of
            PopDoubleRefOrSingleRef() should not have this. */
        Reference,

        /** In array formula: convert area reference to array. Function will be
            called only once if no Value type is involved. Functions able to
            handle a svMatrix parameter but not a svDoubleRef parameter as area
            should have this. */
        Array,

        /** Area reference must be converted to array in any case, and must
            also be propagated to subsequent operators and functions being part
            of a parameter of this function. */
        ForceArray
    };

                                /// MUST be called once before any other method.
    static  void                Init();

    static  void                Exit();

                                /** Get one parameter type for function eOp.
                                    @param nParameter
                                        Which parameter, 0-based */
    static  Type                GetParameterType( const ScToken* pToken,
                                        USHORT nParameter);

                                /** Whether OpCode has a parameter of type
                                    ForceArray. */
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
        bool        bRepeatLast;
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
        BYTE        nMinParams;         // fix or minimum, or repeat start
        bool        bHasForceArray;
    };

    static  const RawData       pRawData[];
    static  RunData*            pData;

    // ocExternal AddIns
    static  Type                GetExternalParameterType(
                                    const ScToken* pToken, USHORT nParameter);

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
                                    returns FALSE. For opcodes not specified in
                                    the implementation a parameter count of 1
                                    is assumed, for opcodes out of range 0 is
                                    assumed. If HasRepeatParameters() returns
                                    TRUE, information is NOT related to whether
                                    any parameters are optional, only the type
                                    of parameters is significant. */
    static  inline  BYTE        GetMinimumParameters( OpCode eOp)
                                    {
                                        if ( eOp <= SC_OPCODE_LAST_OPCODE_ID )
                                            return pData[eOp].aData.nParam[0]
                                                == Unknown ? 1 :
                                                pData[eOp].nMinParams;
                                        return 0;
                                    }

                                /** Whether last parameter type is repeated. */
    static  inline  bool        HasRepeatParameters( OpCode eOp)
                                    {
                                        return eOp <= SC_OPCODE_LAST_OPCODE_ID
                                            && pData[eOp].aData.bRepeatLast;
                                    }
#endif // OSL_DEBUG_LEVEL
};

#endif // SC_PARCLASS_HXX

