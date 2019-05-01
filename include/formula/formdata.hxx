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

#ifndef INCLUDED_FORMULA_FORMDATA_HXX
#define INCLUDED_FORMULA_FORMDATA_HXX

#include <formula/formuladllapi.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/gen.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

namespace formula
{
enum class FormulaDlgMode;

class FORMULA_DLLPUBLIC FormEditData
{
public:
                        FormEditData();
                        virtual ~FormEditData();

    virtual void            SaveValues();

    FormulaDlgMode   GetMode() const     { return nMode; }
    sal_Int32        GetFStart() const   { return nFStart; }
    sal_uInt16       GetOffset() const   { return nOffset; }
    const OUString&  GetUndoStr() const  { return aUndoStr; }
    bool             GetMatrixFlag()const{ return bMatrix;}
    const Selection& GetSelection()const { return aSelection;}

    void             SetMode( FormulaDlgMode nNew )              { nMode = nNew; }
    void             SetFStart( sal_Int32 nNew )                 { nFStart = nNew; }
    void             SetOffset( sal_uInt16 nNew )                { nOffset = nNew; }
    void             SetUndoStr( const OUString& rNew )          { aUndoStr = rNew; }
    void             SetMatrixFlag(bool bNew)                    { bMatrix=bNew;}
    void             SetSelection(const Selection& aSel)         { aSelection=aSel;}
protected:
    void                Reset();
    FormEditData( const FormEditData& );
    FormEditData& operator=( const FormEditData& r );

private:
    FormulaDlgMode      nMode;
    sal_Int32           nFStart;
    sal_uInt16          nOffset;
    OUString            aUndoStr;
    bool                bMatrix;
    Selection           aSelection;
};


} // formula
#endif // INCLUDED_FORMULA_FORMDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
