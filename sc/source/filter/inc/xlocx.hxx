/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xlocx.hxx,v $
 * $Revision: 1.15 $
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

#ifndef SC_XLOCX_HXX
#define SC_XLOCX_HXX

#include <svx/msocximex.hxx>
#include "xiroot.hxx"
#include "xeroot.hxx"

// 0 = Export TBX form controls, 1 = Export OCX form controls.
#define EXC_EXP_OCX_CTRL 0

// OCX controls ===============================================================

/** Converter base class for import and export of OXC controls.
    @descr  The purpose of this class is to manage all the draw pages occuring in a
    spreadsheet document. Derived classes implement import or export of the controls. */
class XclOcxConverter : protected SvxMSConvertOCXControls
{
protected:
    explicit            XclOcxConverter( const XclRoot& rRoot );
    virtual             ~XclOcxConverter();

    /** Sets the sheet index of the currently processed object. GetDrawPage() needs this. */
    void                SetScTab( SCTAB nScTab );
    /** Calls SetScTab() with the passed sheet index and updates the xDrawPage base class member. */
    void                SetDrawPage( SCTAB nScTab );

private:
    /** Returns the current draw page. */
    virtual const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >&
                        GetDrawPage();

private:
    const XclRoot&      mrRoot;         /// Root data.
    SCTAB               mnCurrScTab;    /// Stores sheet index of an object for GetDrawPage().
    SCTAB               mnCachedScTab;  /// Sheet index of cached draw page.
};

// ----------------------------------------------------------------------------

class Rectangle;
class SdrObject;
class XclImpOleObj;
class XclImpTbxControlObj;
class XclImpControlObjHelper;

/** Converter for import of OXC controls. */
class XclImpOcxConverter : public XclOcxConverter, protected XclImpRoot
{
public:
    explicit            XclImpOcxConverter( const XclImpRoot& rRoot );

    /** Reads the control formatting data for the passed object and creates the SdrUnoObj. */
    SdrObject*          CreateSdrObject( XclImpOleObj& rOcxCtrlObj, const Rectangle& rAnchorRect );
    /** Creates the SdrUnoObj for the passed TBX form control object. */
    SdrObject*          CreateSdrObject( XclImpTbxControlObj& rTbxCtrlObj, const Rectangle& rAnchorRect );

private:
    /** Inserts the passed control rxFComp into the form. */
    virtual sal_Bool    InsertControl(
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::form::XFormComponent >& rxFComp,
                            const ::com::sun::star::awt::Size& rSize,
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape >* pxShape,
                            BOOL bFloatingCtrl );

    /** Returns the SdrObject from the passed shape. Sets the passed anchor rectangle. */
    SdrObject*          FinalizeSdrObject(
                            XclImpControlObjHelper& rCtrlObj,
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape > xShape,
                            const Rectangle& rAnchorRect ) const;
    /** Tries to register a Basic macro for the control. */
    void                RegisterTbxMacro( const XclImpTbxControlObj& rTbxCtrlObj );

private:
    SotStorageStreamRef mxStrm;             /// The 'Ctls' stream in the Excel file.
    sal_Int32           mnLastIndex;        /// Last insertion index of a control.
};

// ----------------------------------------------------------------------------

class SdrObject;
#if EXC_EXP_OCX_CTRL
class XclExpOcxControlObj;
#else
class XclExpTbxControlObj;
#endif
class XclExpCtrlLinkHelper;

/** Converter for export of OXC controls. */
class XclExpOcxConverter : public XclOcxConverter, protected XclExpRoot
{
public:
    explicit            XclExpOcxConverter( const XclExpRoot& rRoot );

#if EXC_EXP_OCX_CTRL
    /** Creates an OCX form control OBJ record from the passed form control.
        @descr  Writes the form control data to the 'Ctls' stream. */
    XclExpOcxControlObj* CreateCtrlObj( ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShape > xShape );

private:
    SotStorageStreamRef  mxStrm;         /// The 'Ctls' stream.
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

// ============================================================================

#endif

