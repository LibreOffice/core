/*************************************************************************
 *
 *  $RCSfile: xcl97esc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
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

#ifndef _XCL97ESC_HXX
#define _XCL97ESC_HXX


#ifndef _SVX_ESCHEREX_HXX
#include <svx/escherex.hxx>
#endif
#ifndef _TOOLS_TABLE_HXX //autogen wg. Table
#include <tools/table.hxx>
#endif
#ifndef _STACK_HXX //autogen wg. Stack
#include <tools/stack.hxx>
#endif


// --- class XclEscherEx ---------------------------------------------

struct RootData;
class SvStream;
class TempFile;
class SvFileStream;
class XclObj;
class XclEscherHostAppData;
class XclEscherClientData;
class XclEscherClientTextbox;

class XclEscherEx : public EscherEx
{
private:
        List                aOffsetMap;
        Stack               aStack;
        RootData&           rRootData;
        TempFile*           pPicTempFile;
        SvFileStream*       pPicStrm;
        XclObj*             pCurrXclObj;
        XclEscherHostAppData*   pCurrAppData;
        XclEscherClientData*    pTheClientData; // always the same
        XclEscherClientTextbox* pAdditionalText;
        USHORT                  nAdditionalText;

            void                DeleteCurrAppData();

public:
                                XclEscherEx( SvStream& rStrm, UINT32 nDrawings,
                                    RootData& rRoot );
    virtual                     ~XclEscherEx();

                                /// maintains OffsetMap
    virtual void                InsertAtCurrentPos( UINT32 nBytes, BOOL bCont = FALSE );

    virtual SvStream*           QueryPicStream();
    virtual EscherExHostAppData*    StartShape( const com::sun::star::uno::Reference<
                                        com::sun::star::drawing::XShape>& rShape );
    virtual void                EndShape( UINT16 nShapeType, UINT32 nShapeID );
    virtual EscherExHostAppData*    EnterAdditionalTextGroup();

                                /// appends stream offset to list and returns position in list
            ULONG               AddCurrentOffsetToMap();
                                /// replaces position in list with current stream offset
            void                ReplaceCurrentOffsetInMap( ULONG nPos );
                                /// returns stream offset for position in list
    inline  ULONG               GetOffsetFromMap( ULONG nPos ) const;
                                /// last position in list (count-1)
    inline  ULONG               GetLastOffsetMapPos() const;

                                /// Flush and merge PicStream into EscherStream
            void                EndDocument();
};


inline ULONG XclEscherEx::GetOffsetFromMap( ULONG nPos ) const
{
    return (ULONG) aOffsetMap.GetObject( nPos );
}


inline ULONG XclEscherEx::GetLastOffsetMapPos() const
{
    return aOffsetMap.Count() - 1;
}


// --- class XclEscher -----------------------------------------------

struct RootData;
class TempFile;
class SvFileStream;

class XclEscher
{
private:
        TempFile*           pTempFile;
        SvFileStream*       pStrm;
        XclEscherEx*        pEx;

public:
                                XclEscher( UINT32 nDrawings, RootData& rRoot );
                                ~XclEscher();

    inline  XclEscherEx*        GetEx() const       { return pEx; }
    inline  SvStream&           GetStrm() const     { return *pStrm; }
};


// --- class XclEscherHostAppData ------------------------------------

class XclEscherHostAppData : public EscherExHostAppData
{
private:
        BOOL                bStackedGroup;

public:
                                XclEscherHostAppData() : bStackedGroup( FALSE )
                                    {}
    inline  void                SetStackedGroup( BOOL b )   { bStackedGroup = b; }
    inline  BOOL                IsStackedGroup() const  { return bStackedGroup; }
};


// --- class XclEscherClientAnchor -----------------------------------

struct RootData;
class ScDocument;
class ScAddress;
class Rectangle;
class SdrObject;

class XclEscherClientAnchor : public EscherExClientAnchor_Base
{
private:
        RootData*           pRootData;
                                        // all undocumented
        UINT16              nFlag;      // 0: move and size with cell
                                        // 2: move with cell, don't size
                                        // 3: don't move, don't size
        UINT16              nCol1;      // UL column
        UINT16              nX1;        // UL offset (in fractions of a column, 1024)
        UINT16              nRow1;      // UL row
        UINT16              nY1;        // UL offset (in fractions of a row, 256)
        UINT16              nCol2;      // LR column
        UINT16              nX2;        // LR offset (in fractions of a column, 1024)
        UINT16              nRow2;      // LR row
        UINT16              nY2;        // LR offset (in fractions of a row, 256)

    static  BOOL                FindNextCol(
                                    USHORT& nCol,
                                    const ScDocument* pDoc,
                                    USHORT nTab,
                                    short nDir
                                    );
    static  BOOL                FindNextRow(
                                    USHORT& nRow,
                                    const ScDocument* pDoc,
                                    USHORT nTab,
                                    short nDir
                                    );
    static  void                ColX(
                                    UINT16& nCol,
                                    UINT16& nX,
                                    UINT16 nStart,
                                    long& nWidth,
                                    long nPosX,
                                    const ScDocument* pDoc,
                                    USHORT nTab
                                    );
    static  void                RowY(
                                    UINT16& nRow,
                                    UINT16& nY,
                                    UINT16 nStart,
                                    long& nWidth,
                                    long nPosY,
                                    const ScDocument* pDoc,
                                    USHORT nTab
                                    );
            void                Init( const Rectangle& rRect );

public:

    static  UINT16              GetMoveSizeFlag( const SdrObject& rObj );

                                XclEscherClientAnchor(
                                    RootData& rRoot,
                                    UINT16 nMoveSizeFlag
                                    );

                                /// used for Charts
                                XclEscherClientAnchor(
                                    RootData& rRoot,
                                    const SdrObject& rObj
                                    );

                                /// only used for notes without sizes
                                XclEscherClientAnchor(
                                    const ScDocument* pDoc,
                                    const ScAddress& rPos
                                    );

                                /// used by class XclObjDropDown
            void                SetDropDownPosition( const ScAddress& rAddr );

    virtual void                WriteData( EscherEx& rEx, const Rectangle& rRect );
            void                WriteData( EscherEx& rEx ) const;
};


// --- class XclEscherClientData -------------------------------------

class XclEscherClientData : public EscherExClientRecord_Base
{
public:
                                XclEscherClientData() {}
    virtual void                WriteData( EscherEx& rEx ) const;
};


// --- class XclEscherClientTextbox ----------------------------------

class SdrTextObj;

class XclEscherClientTextbox : public EscherExClientRecord_Base
{
private:
        RootData&               rRootData;
        const SdrTextObj&       rTextObj;
        XclObj*                 pXclObj;

public:
                                XclEscherClientTextbox(
                                    RootData& rRoot,
                                    const SdrTextObj& rObj,
                                    XclObj* pObj
                                    );

                                //! ONLY for the AdditionalText mimic
    inline  void                SetXclObj( XclObj* p )  { pXclObj = p; }

    virtual void                WriteData( EscherEx& rEx ) const;
};



#endif // _XCL97ESC_HXX
