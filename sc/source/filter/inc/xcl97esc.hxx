/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xcl97esc.hxx,v $
 * $Revision: 1.13.14.3 $
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

#ifndef SC_XCL97ESC_HXX
#define SC_XCL97ESC_HXX

#include <filter/msfilter/escherex.hxx>
#include <tools/table.hxx>
#include <tools/stack.hxx>
#include "xlescher.hxx"
#include "xeroot.hxx"

// 0 = Export TBX form controls, 1 = Export OCX form controls.
#define EXC_EXP_OCX_CTRL 0

namespace utl { class TempFile; }

// --- class XclEscherEx ---------------------------------------------

class SvStream;
class XclObj;
class XclEscherHostAppData;
class XclEscherClientData;
class XclEscherClientTextbox;
#if EXC_EXP_OCX_CTRL
class XclExpOcxControlObj;
#else
class XclExpTbxControlObj;
#endif

class XclEscherEx : public EscherEx, protected XclExpRoot
{
private:
        List                aOffsetMap;
        Stack               aStack;
        utl::TempFile*      pPicTempFile;
        SvStream*           pPicStrm;
        XclObj*             pCurrXclObj;
        XclEscherHostAppData*   pCurrAppData;
        XclEscherClientData*    pTheClientData; // always the same
        XclEscherClientTextbox* pAdditionalText;
        USHORT                  nAdditionalText;

            void                DeleteCurrAppData();

public:
                                XclEscherEx( const XclExpRoot& rRoot, SvStream& rStrm, UINT32 nDrawings );
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

#if EXC_EXP_OCX_CTRL
    /** Creates an OCX form control OBJ record from the passed form control.
        @descr  Writes the form control data to the 'Ctls' stream. */
    XclExpOcxControlObj* CreateCtrlObj( ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShape > xShape );

private:
    SotStorageStreamRef  mxCtlsStrm;         /// The 'Ctls' stream.
#else
    /** Creates a TBX form control OBJ record from the passed form control. */
    XclExpTbxControlObj* CreateCtrlObj( ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShape > xShape );

private:
    /** Tries to get the name of a Basic macro from a control. */
    void                ConvertTbxMacro(
                            XclExpTbxControlObj& rTbxCtrlObj,
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel > xCtrlModel );
#endif
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

class XclEscher : protected XclExpRoot
{
private:
        utl::TempFile*      pTempFile;
        SvStream*           pStrm;
        XclEscherEx*        pEx;

public:
                                XclEscher( const XclExpRoot& rRoot, UINT32 nDrawings );
                                ~XclEscher();

    inline  XclEscherEx*        GetEx() const       { return pEx; }
    inline  SvStream&           GetStrm() const     { return *pStrm; }

    void                        AddSdrPage();
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


// DFF client anchor ==========================================================

class Rectangle;
class SdrObject;
class ScAddress;

/** Represents the position (anchor) of an object in a Calc document. */
class XclExpDffAnchor : public EscherExClientAnchor_Base, protected XclExpRoot
{
public:
    /** Constructs a dummy client anchor. */
    explicit                    XclExpDffAnchor( const XclExpRoot& rRoot, sal_uInt16 nFlags = 0 );
    /** Constructs a client anchor directly from an SdrObject. */
    explicit                    XclExpDffAnchor( const XclExpRoot& rRoot, const SdrObject& rSdrObj );

    /** Sets the flags according to the passed SdrObject. */
    void                        SetFlags( const SdrObject& rSdrObj );

    /** Called from SVX Escher exporter.
        @param rRect  The object anchor rectangle to be exported (in twips). */
    virtual void                WriteData( EscherEx& rEx, const Rectangle& rRect );

    /** Writes the anchor structure with the current anchor position. */
    void                        WriteData( EscherEx& rEx ) const;

protected:  // for access in derived classes
    XclObjAnchor                maAnchor;       /// The client anchor data.
    sal_uInt16                  mnFlags;        /// Flags for DFF stream export.
};

// ----------------------------------------------------------------------------

/** Represents the position (anchor) of a note object. */
class XclExpDffNoteAnchor : public XclExpDffAnchor
{
public:
    explicit                    XclExpDffNoteAnchor( const XclExpRoot& rRoot, const Rectangle& rRect );
};


// ----------------------------------------------------------------------------

/** Represents the position (anchor) of a cell dropdown object. */
class XclExpDffDropDownAnchor : public XclExpDffAnchor
{
public:
    explicit                    XclExpDffDropDownAnchor( const XclExpRoot& rRoot, const ScAddress& rScPos );
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

class XclEscherClientTextbox : public EscherExClientRecord_Base, protected XclExpRoot
{
private:
        const SdrTextObj&       rTextObj;
        XclObj*                 pXclObj;

public:
                                XclEscherClientTextbox(
                                    const XclExpRoot& rRoot,
                                    const SdrTextObj& rObj,
                                    XclObj* pObj
                                    );

                                //! ONLY for the AdditionalText mimic
    inline  void                SetXclObj( XclObj* p )  { pXclObj = p; }

    virtual void                WriteData( EscherEx& rEx ) const;
};



#endif // _XCL97ESC_HXX
