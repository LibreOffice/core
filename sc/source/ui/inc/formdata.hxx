/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formdata.hxx,v $
 * $Revision: 1.3.32.1 $
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

#ifndef SC_FORMDATA_HXX
#define SC_FORMDATA_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>

class ScInputHandler;
class ScDocShell;

//============================================================================

class ScFormEditData
{
public:
                        ScFormEditData();
                        ~ScFormEditData();

    void                SaveValues();
    BOOL                HasParent() const   { return pParent != NULL; }

    USHORT              GetMode() const     { return nMode; }
    xub_StrLen          GetFStart() const   { return nFStart; }
    USHORT              GetCatSel() const   { return nCatSel; }
    USHORT              GetFuncSel() const  { return nFuncSel; }
    USHORT              GetOffset() const   { return nOffset; }
    USHORT              GetEdFocus() const  { return nEdFocus; }
    const String&       GetUndoStr() const  { return aUndoStr; }
    BOOL                GetMatrixFlag()const{ return bMatrix;}
    ULONG               GetUniqueId()const  { return nUniqueId;}
    const Selection&    GetSelection()const { return aSelection;}
    ScInputHandler*     GetInputHandler()   { return pInputHandler;}
    ScDocShell*         GetDocShell()       { return pScDocShell;}

    void                SetMode( USHORT nNew )                  { nMode = nNew; }
    void                SetFStart( xub_StrLen nNew )            { nFStart = nNew; }
    void                SetCatSel( USHORT nNew )                { nCatSel = nNew; }
    void                SetFuncSel( USHORT nNew )               { nFuncSel = nNew; }
    void                SetOffset( USHORT nNew )                { nOffset = nNew; }
    void                SetEdFocus( USHORT nNew )               { nEdFocus = nNew; }
    void                SetUndoStr( const String& rNew )        { aUndoStr = rNew; }
    void                SetMatrixFlag(BOOL bNew)                { bMatrix=bNew;}
    void                SetUniqueId(ULONG nNew)                 { nUniqueId=nNew;}
    void                SetSelection(const Selection& aSel)     { aSelection=aSel;}
    void                SetInputHandler(ScInputHandler* pHdl)   { pInputHandler=pHdl;}
    void                SetDocShell(ScDocShell* pSds)           { pScDocShell=pSds;}



private:
                        ScFormEditData( const ScFormEditData& );
    const ScFormEditData& operator=( const ScFormEditData& r );
    void                Reset();

    ScFormEditData*     pParent;            // fuer Verschachtelung
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
    ScInputHandler*     pInputHandler;
    ScDocShell*         pScDocShell;
};



#endif // SC_CRNRDLG_HXX

