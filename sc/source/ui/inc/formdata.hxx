/*************************************************************************
 *
 *  $RCSfile: formdata.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_FORMDATA_HXX
#define SC_FORMDATA_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

class ScInputHandler;
class ScDocShell;

//============================================================================

class ScFormEditData
{
public:
                        ScFormEditData();
                        ~ScFormEditData();

    void                SaveValues();
    void                RestoreValues();
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

