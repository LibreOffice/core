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
    BOOL                    HasParent() const   { return pParent != NULL; }

    inline USHORT           GetMode() const     { return nMode; }
    inline xub_StrLen       GetFStart() const   { return nFStart; }
    inline USHORT           GetCatSel() const   { return nCatSel; }
    inline USHORT           GetFuncSel() const  { return nFuncSel; }
    inline USHORT           GetOffset() const   { return nOffset; }
    inline USHORT           GetEdFocus() const  { return nEdFocus; }
    inline const String&    GetUndoStr() const  { return aUndoStr; }
    inline BOOL             GetMatrixFlag()const{ return bMatrix;}
    inline ULONG            GetUniqueId()const  { return nUniqueId;}
    inline const Selection& GetSelection()const { return aSelection;}

    inline void             SetMode( USHORT nNew )                  { nMode = nNew; }
    inline void             SetFStart( xub_StrLen nNew )            { nFStart = nNew; }
    inline void             SetCatSel( USHORT nNew )                { nCatSel = nNew; }
    inline void             SetFuncSel( USHORT nNew )               { nFuncSel = nNew; }
    inline void             SetOffset( USHORT nNew )                { nOffset = nNew; }
    inline void             SetEdFocus( USHORT nNew )               { nEdFocus = nNew; }
    inline void             SetUndoStr( const String& rNew )        { aUndoStr = rNew; }
    inline void             SetMatrixFlag(BOOL bNew)                { bMatrix=bNew;}
    inline void             SetUniqueId(ULONG nNew)                 { nUniqueId=nNew;}
    inline void             SetSelection(const Selection& aSel)     { aSelection=aSel;}
protected:
    void                Reset();
    FormEditData( const FormEditData& );
    const FormEditData& operator=( const FormEditData& r );

    FormEditData*   pParent;            // fuer Verschachtelung
private:
    USHORT              nMode;              // enum ScFormulaDlgMode
    xub_StrLen          nFStart;
    USHORT              nCatSel;
    USHORT              nFuncSel;
    USHORT              nOffset;
    USHORT              nEdFocus;
    String              aUndoStr;
    BOOL                bMatrix;
    ULONG               nUniqueId;
    Selection           aSelection;
};


} // formula
#endif // FORMULA_FORMDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
