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

