/*************************************************************************
 *
 *  $RCSfile: xcl97esc.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:58:05 $
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

#ifndef SC_XLESCHER_HXX
#include "xlescher.hxx"
#endif
#ifndef SC_XLOCX_HXX
#include "xlocx.hxx"
#endif

namespace utl { class TempFile; }

// --- class XclEscherEx ---------------------------------------------

struct RootData;
class SvStream;
class XclObj;
class XclEscherHostAppData;
class XclEscherClientData;
class XclEscherClientTextbox;

class XclEscherEx : public EscherEx
{
private:
        List                aOffsetMap;
        Stack               aStack;
        XclExpOcxConverter  aOcxConverter;      /// Export of form controls.
        RootData&           rRootData;
        utl::TempFile*      pPicTempFile;
        SvStream*           pPicStrm;
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

class XclEscher
{
private:
        utl::TempFile*      pTempFile;
        SvStream*           pStrm;
        XclEscherEx*        pEx;

public:
                                XclEscher( UINT32 nDrawings, RootData& rRoot );
                                ~XclEscher();

    inline  XclEscherEx*        GetEx() const       { return pEx; }
    inline  SvStream&           GetStrm() const     { return *pStrm; }

    void                        AddSdrPage( RootData& rRootData );
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


// Escher client anchor =======================================================

class Rectangle;
class SdrObject;
class ScAddress;

/** Represents the position (anchor) of an Escher object in a Calc document. */
class XclExpEscherAnchor : public EscherExClientAnchor_Base, protected XclExpRoot
{
public:
    /** Constructs a dummy Escher client anchor. */
    explicit                    XclExpEscherAnchor( const XclExpRoot& rRoot, sal_uInt16 nFlags = 0 );
    /** Constructs an Escher client anchor directly from an SdrObject. */
    explicit                    XclExpEscherAnchor( const XclExpRoot& rRoot, const SdrObject& rSdrObj );

    /** Sets the flags according to the passed SdrObject. */
    void                        SetFlags( const SdrObject& rSdrObj );

    /** Called from SVX Escher exporter.
        @param rRect  The object anchor rectangle to be exported (in twips). */
    virtual void                WriteData( EscherEx& rEx, const Rectangle& rRect );

    /** Writes the Escher anchor structure with the current anchor position. */
    void                        WriteData( EscherEx& rEx ) const;

protected:
    BOOL                        FindNextCol( sal_uInt16& nCol, short nDir );
    BOOL                        FindNextRow( sal_uInt16& nRow, short nDir );

protected:  // for access in derived classes
    XclEscherAnchor             maAnchor;       /// The client anchor data.
    sal_uInt16                  mnFlags;        /// Flags for Escher stream export.
};

// ----------------------------------------------------------------------------

/** Represents the position (anchor) of an Escher note object. */
class XclExpEscherNoteAnchor : public XclExpEscherAnchor
{
public:
    explicit                    XclExpEscherNoteAnchor( const XclExpRoot& rRoot, const ScAddress& rPos );
};


// ----------------------------------------------------------------------------

/** Represents the position (anchor) of an Escher cell dropdown object. */
class XclExpEscherDropDownAnchor : public XclExpEscherAnchor
{
public:
    explicit                    XclExpEscherDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rPos );
};


// ============================================================================

// --- class XclEscherClientData -------------------------------------

class XclEscherClientData : public EscherExClientRecord_Base
{
public:
                                XclEscherClientData() {}
    virtual void                WriteData( EscherEx& rEx ) const;
};


// --- class XclEscherClientTextbox ----------------------------------

class SdrTextObj;
struct RootData;

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
