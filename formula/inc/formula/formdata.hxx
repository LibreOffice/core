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



#ifndef FORMULA_FORMDATA_HXX
#define FORMULA_FORMDATA_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>
#include "formula/formuladllapi.h"

//============================================================================
namespace formula
{
class FORMULA_DLLPUBLIC FormEditData
{
public:
                        FormEditData();
                        virtual ~FormEditData();

    virtual void            SaveValues();
    void                    RestoreValues();
    sal_Bool                    HasParent() const   { return pParent != NULL; }

    inline sal_uInt16           GetMode() const     { return nMode; }
    inline xub_StrLen       GetFStart() const   { return nFStart; }
    inline sal_uInt16           GetCatSel() const   { return nCatSel; }
    inline sal_uInt16           GetFuncSel() const  { return nFuncSel; }
    inline sal_uInt16           GetOffset() const   { return nOffset; }
    inline sal_uInt16           GetEdFocus() const  { return nEdFocus; }
    inline const String&    GetUndoStr() const  { return aUndoStr; }
    inline sal_Bool             GetMatrixFlag()const{ return bMatrix;}
    inline rtl::OString     GetUniqueId()const  { return aUniqueId;}
    inline const Selection& GetSelection()const { return aSelection;}

    inline void             SetMode( sal_uInt16 nNew )                  { nMode = nNew; }
    inline void             SetFStart( xub_StrLen nNew )            { nFStart = nNew; }
    inline void             SetCatSel( sal_uInt16 nNew )                { nCatSel = nNew; }
    inline void             SetFuncSel( sal_uInt16 nNew )               { nFuncSel = nNew; }
    inline void             SetOffset( sal_uInt16 nNew )                { nOffset = nNew; }
    inline void             SetEdFocus( sal_uInt16 nNew )               { nEdFocus = nNew; }
    inline void             SetUndoStr( const String& rNew )        { aUndoStr = rNew; }
    inline void             SetMatrixFlag(sal_Bool bNew)                { bMatrix=bNew;}
    inline void             SetUniqueId(const rtl::OString nNew)    { aUniqueId=nNew;}
    inline void             SetSelection(const Selection& aSel)     { aSelection=aSel;}
protected:
    void                Reset();
    FormEditData( const FormEditData& );
    const FormEditData& operator=( const FormEditData& r );

    FormEditData*   pParent;            // fuer Verschachtelung
private:
    sal_uInt16              nMode;              // enum ScFormulaDlgMode
    xub_StrLen          nFStart;
    sal_uInt16              nCatSel;
    sal_uInt16              nFuncSel;
    sal_uInt16              nOffset;
    sal_uInt16              nEdFocus;
    String              aUndoStr;
    sal_Bool                bMatrix;
    rtl::OString        aUniqueId;
    Selection           aSelection;
};


} // formula
#endif // FORMULA_FORMDATA_HXX

