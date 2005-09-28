/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlocx.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 12:03:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XLOCX_HXX
#define SC_XLOCX_HXX

#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif

#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif
#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif

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
class XclImpCtrlLinkHelper;

/** Converter for import of OXC controls. */
class XclImpOcxConverter : public XclOcxConverter, protected XclImpRoot
{
public:
    explicit            XclImpOcxConverter( const XclImpRoot& rRoot );

    /** Reads the control formatting data for the passed object and creates the SdrUnoObj. */
    SdrObject*          CreateSdrObject( const XclImpOleObj& rOcxCtrlObj, const Rectangle& rAnchorRect );
    /** Creates the SdrUnoObj for the passed TBX form control object. */
    SdrObject*          CreateSdrObject( const XclImpTbxControlObj& rTbxCtrlObj, const Rectangle& rAnchorRect );

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
    SdrObject*          GetSdrObject(
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape >& rxShape,
                            const Rectangle& rAnchorRect ) const;
    /** Tries to set a spreadsheet cell link and source range link at the passed form control. */
    void                ConvertSheetLinks(
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& rxModel,
                            const XclImpCtrlLinkHelper& rControl ) const;
    /** Tries to register a Basic macro for the control. */
    void                RegisterTbxMacro( const XclImpTbxControlObj& rTbxCtrlObj );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        mxDocFactory;       /// The MultiServiceFactory of the Calc document.
    SotStorageStreamRef mxStrm;             /// The 'Ctls' stream in the Excel file.
    sal_Int32           mnLastIndex;        /// Last insertion index of a control.
};

// ----------------------------------------------------------------------------

class SdrObject;
#if EXC_EXP_OCX_CTRL
class XclExpObjOcxCtrl;
#else
class XclExpObjTbxCtrl;
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
    XclExpObjOcxCtrl*   CreateCtrlObj( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShape >& rxShape );
#else
    /** Creates a TBX form control OBJ record from the passed form control. */
    XclExpObjTbxCtrl*   CreateCtrlObj( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::drawing::XShape >& rxShape );
#endif

private:
    /** Tries to get spreadsheet cell link and source range link from the passed model.
        @param rControl  The Excel form control that stores and exports the links. */
    void                ConvertSheetLinks(
                            XclExpCtrlLinkHelper& rControl,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& rxModel ) const;
#if !EXC_EXP_OCX_CTRL
    /** Tries to get the name of a Basic macro from a control. */
    void                ConvertTbxMacro(
                            XclExpObjTbxCtrl& rTbxCtrlObj,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& rxModel );
#endif

private:
#if EXC_EXP_OCX_CTRL
    SotStorageStreamRef  mxStrm;         /// The 'Ctls' stream.
#endif
};

// ============================================================================

#endif

