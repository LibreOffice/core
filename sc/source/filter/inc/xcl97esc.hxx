/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xcl97esc.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _XCL97ESC_HXX
#define _XCL97ESC_HXX


#include <svx/escherex.hxx>
#include <tools/table.hxx>
#include <tools/stack.hxx>
#include "xlescher.hxx"
#include "xlocx.hxx"

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

    void                        AddSdrPage( const XclExpRoot& rRoot );
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

protected:  // for access in derived classes
    XclEscherAnchor             maAnchor;       /// The client anchor data.
    sal_uInt16                  mnFlags;        /// Flags for Escher stream export.
};

// ----------------------------------------------------------------------------

/** Represents the position (anchor) of an Escher note object. */
class XclExpEscherNoteAnchor : public XclExpEscherAnchor
{
public:
    explicit                    XclExpEscherNoteAnchor( const XclExpRoot& rRoot, const Rectangle& rRect );
};


// ----------------------------------------------------------------------------

/** Represents the position (anchor) of an Escher cell dropdown object. */
class XclExpEscherDropDownAnchor : public XclExpEscherAnchor
{
public:
    explicit                    XclExpEscherDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rScPos );
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
