/*************************************************************************
 *
 *  $RCSfile: xlocx.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-14 12:13:23 $
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

class XclImpEscherOle;
class XclImpEscherTbxCtrl;
class XclImpCtrlLinkHelper;

/** Converter for import of OXC controls. */
class XclImpOcxConverter : public XclOcxConverter, protected XclImpRoot
{
public:
    explicit            XclImpOcxConverter( const XclImpRoot& rRoot );

    /** Reads the control formatting data for the passed object and creates the SdrUnoObj.
        @return  true = SdrUnoObj successfully created. */
    bool                CreateSdrUnoObj( XclImpEscherOle& rOleObj );

    /** Creates the SdrUnoObj for the passed TBX form control object.
        @return  true = SdrUnoObj successfully created. */
    bool                CreateSdrUnoObj( XclImpEscherTbxCtrl& rTbxCtrl );

private:
    /** Inserts the passed control rxFComp into the document. */
    virtual sal_Bool    InsertControl(
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::form::XFormComponent >& rxFComp,
                            const ::com::sun::star::awt::Size& rSize,
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShape >* pxShape,
                            BOOL bFloatingCtrl );

    /** Tries to set a spreadsheet cell link and source range link at the passed form control. */
    void                ConvertSheetLinks(
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::awt::XControlModel >& rxModel,
                            const XclImpCtrlLinkHelper& rControl ) const;
    /** Tries to register a Basic macro for the control. */
    void                RegisterTbxMacro( XclImpEscherTbxCtrl& rTbxCtrl );

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
                            XclExpObjTbxCtrl& rTbxCtrl,
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

