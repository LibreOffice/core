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

#ifndef SC_XCL97ESC_HXX
#define SC_XCL97ESC_HXX

#include <memory>
#include <tools/table.hxx>
#include <tools/stack.hxx>
#include <filter/msfilter/escherex.hxx>
#include "xlescher.hxx"
#include "xeroot.hxx"

// 0 = Export TBX form controls, 1 = Export OCX form controls.
#define EXC_EXP_OCX_CTRL 0

namespace utl { class TempFile; }

// ============================================================================

class SvStream;

class XclEscherExGlobal : public EscherExGlobal, protected XclExpRoot
{
public:
    explicit            XclEscherExGlobal( const XclExpRoot& rRoot );

private:
    /** Overloaded to create a new temporary file and return its stream. */
    virtual SvStream*   ImplQueryPictureStream();

private:
    ::std::auto_ptr< ::utl::TempFile > mxPicTempFile;
    ::std::auto_ptr< SvStream > mxPicStrm;
};

// ============================================================================

class XclObj;
class XclExpDffAnchorBase;
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
public:
    explicit            XclEscherEx(
                            const XclExpRoot& rRoot,
                            XclExpObjectManager& rObjMgr,
                            SvStream& rStrm,
                            const XclEscherEx* pParent = 0 );
    virtual             ~XclEscherEx();

    /** Called by MSODRAWING record constructors to initialize the DFF stream
        fragment they will own. returns the DFF fragment identifier. */
    sal_uInt32          InitNextDffFragment();
    /** Called after some data has been written to the DFF stream, to update
        the end position of the DFF fragment owned by an MSODRAWING record. */
    void                UpdateDffFragmentEnd();

    /** Returns the position of the specified DFF stream fragment. */
    sal_uInt32          GetDffFragmentPos( sal_uInt32 nFragmentKey );
    /** Returns the size of the specified DFF stream fragment. */
    sal_uInt32          GetDffFragmentSize( sal_uInt32 nFragmentKey );
    /** Returns true, if there is more data left in the DFF stream than owned
        by the last MSODRAWING record. */
    bool                HasPendingDffData();

    /** Creates a new DFF client anchor object and calculates the anchor
        position of the passed object. Caller takes ownership! */
    XclExpDffAnchorBase* CreateDffAnchor( const SdrObject& rSdrObj ) const;

    virtual EscherExHostAppData* StartShape(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape>& rxShape,
                            const Rectangle* pChildAnchor );
    virtual void                EndShape( sal_uInt16 nShapeType, sal_uInt32 nShapeID );
    virtual EscherExHostAppData*    EnterAdditionalTextGroup();

                                /// Flush and merge PicStream into EscherStream
            void                EndDocument();

#if EXC_EXP_OCX_CTRL
    /** Creates an OCX form control OBJ record from the passed form control.
        @descr  Writes the form control data to the 'Ctls' stream. */
    XclExpOcxControlObj* CreateCtrlObj(
                            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape,
                            const Rectangle* pChildAnchor );

private:
    SotStorageStreamRef  mxCtlsStrm;         /// The 'Ctls' stream.
#else
    /** Creates a TBX form control OBJ record from the passed form control. */
    XclExpTbxControlObj* CreateCtrlObj(
                            ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape,
                            const Rectangle* pChildAnchor );

private:
    /** Tries to get the name of a Basic macro from a control. */
    void                ConvertTbxMacro(
                            XclExpTbxControlObj& rTbxCtrlObj,
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel > xCtrlModel );
#endif

    void                DeleteCurrAppData();

private:
    XclExpObjectManager& mrObjMgr;
        Stack               aStack;
        XclObj*             pCurrXclObj;
        XclEscherHostAppData*   pCurrAppData;
        XclEscherClientData*    pTheClientData; // always the same
        XclEscherClientTextbox* pAdditionalText;
        sal_uInt16                  nAdditionalText;
    sal_uInt32          mnNextKey;
    bool                mbIsRootDff;
};

// --- class XclEscherHostAppData ------------------------------------

class XclEscherHostAppData : public EscherExHostAppData
{
private:
        sal_Bool                bStackedGroup;

public:
                                XclEscherHostAppData() : bStackedGroup( sal_False )
                                    {}
    inline  void                SetStackedGroup( sal_Bool b )   { bStackedGroup = b; }
    inline  sal_Bool                IsStackedGroup() const  { return bStackedGroup; }
};



// ============================================================================

// --- class XclEscherClientData -------------------------------------

class XclEscherClientData : public EscherExClientRecord_Base
{
public:
                        XclEscherClientData() {}
    virtual void        WriteData( EscherEx& rEx ) const;
};


// --- class XclEscherClientTextbox ----------------------------------

class SdrTextObj;

class XclEscherClientTextbox : public EscherExClientRecord_Base, protected XclExpRoot
{
private:
    const SdrTextObj&   rTextObj;
    XclObj*             pXclObj;

public:
                        XclEscherClientTextbox(
                            const XclExpRoot& rRoot,
                            const SdrTextObj& rObj,
                            XclObj* pObj );

                                //! ONLY for the AdditionalText mimic
    inline  void        SetXclObj( XclObj* p )  { pXclObj = p; }

    virtual void        WriteData( EscherEx& rEx ) const;
};



#endif // _XCL97ESC_HXX
