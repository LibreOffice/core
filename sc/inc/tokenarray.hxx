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



#ifndef SC_TOKENARRAY_HXX
#define SC_TOKENARRAY_HXX

#include "formula/token.hxx"
#include <tools/solar.h>
#include "scdllapi.h"
#include <formula/tokenarray.hxx>

struct ScRawToken;
struct ScSingleRefData;
struct ScComplexRefData;
class ScMatrix;

class SC_DLLPUBLIC ScTokenArray : public formula::FormulaTokenArray
{
    friend class ScCompiler;
    sal_Bool                    ImplGetReference( ScRange& rRange, sal_Bool bValidOnly ) const;

public:
    ScTokenArray();
    /// Assignment with references to ScToken entries (not copied!)
    ScTokenArray( const ScTokenArray& );
   virtual ~ScTokenArray();
    ScTokenArray* Clone() const;    /// True copy!

    /// Exactly and only one range (valid or deleted)
    sal_Bool    IsReference( ScRange& rRange ) const;
    /// Exactly and only one valid range (no #REF!s)
    sal_Bool    IsValidReference( ScRange& rRange ) const;


                            /** Determines the extent of direct adjacent
                                references. Only use with real functions, e.g.
                                GetOuterFuncOpCode() == ocSum ! */
    sal_Bool                    GetAdjacentExtendOfOuterFuncRefs( SCCOLROW& nExtend,
                                const ScAddress& rPos, ScDirection );

    formula::FormulaToken* AddRawToken( const ScRawToken& );
    virtual bool AddFormulaToken(const com::sun::star::sheet::FormulaToken& _aToken,formula::ExternalReferenceHelper* _pRef);
    virtual formula::FormulaToken* AddOpCode( OpCode eCode );
    /** ScSingleRefToken with ocPush. */
    formula::FormulaToken* AddSingleReference( const ScSingleRefData& rRef );
    /** ScSingleRefOpToken with ocMatRef. */
    formula::FormulaToken* AddMatrixSingleReference( const ScSingleRefData& rRef );
    formula::FormulaToken* AddDoubleReference( const ScComplexRefData& rRef );
    formula::FormulaToken* AddExternalName( sal_uInt16 nFileId, const String& rName );
    formula::FormulaToken* AddExternalSingleReference( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef );
    formula::FormulaToken* AddExternalDoubleReference( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef );
    formula::FormulaToken* AddMatrix( ScMatrix* p );
    /** ScSingleRefOpToken with ocColRowName. */
    formula::FormulaToken* AddColRowName( const ScSingleRefData& rRef );
    virtual formula::FormulaToken* MergeArray( );

    /** Merge very last SingleRef+ocRange+SingleRef combination into DoubleRef
        and adjust pCode array, or do nothing if conditions not met.
        Unconditionally returns last token from the resulting pCode array, or
        NULL if there is no pCode (which actually would be caller's fault). */
    formula::FormulaToken* MergeRangeReference( const ScAddress & rPos );

    /// Assignment with references to ScToken entries (not copied!)
    ScTokenArray& operator=( const ScTokenArray& );

    /// Make 3D references point to old referenced position even if relative
    void            ReadjustRelative3DReferences(
                                const ScAddress& rOldPos,
                                const ScAddress& rNewPos );
};

#endif // SC_TOKENARRAY_HXX

