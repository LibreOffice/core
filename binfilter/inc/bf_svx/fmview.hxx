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

#ifndef _SVX_FMVIEW_HXX
#define _SVX_FMVIEW_HXX

// HACK to avoid too deep includes and to have some
// levels free in svdmark itself (MS compiler include depth limit)
#ifndef _SVDHDL_HXX
#include <bf_svx/svdhdl.hxx>
#endif

#ifndef _E3D_VIEW3D_HXX
#include <bf_svx/view3d.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

FORWARD_DECLARE_INTERFACE(util,XNumberFormats)
FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
class OutputDevice;
namespace binfilter {
class ExtOutputDevice;
class FmFormModel;
class FmPageViewWinRec;
class FmFormObj;
class FmFormPage;
class FmFormShell;
class FmXFormView;

namespace svx {
    class ODataAccessDescriptor;
}

class FmFormView : public E3dView
{
    friend class FmXFormController;
    friend class FmFormShell;
    friend class FmXFormShell;
    friend class FmXFormView;
    friend class FmExplorer;
    friend class FmFormObj;

    FmXFormView*	pImpl;
    FmFormShell*	pFormShell;

    void Init();
    void SetFormShell( FmFormShell* pShell ) { pFormShell = pShell; }
    FmFormShell* GetFormShell() const { return pFormShell; }

public:
    TYPEINFO();

    FmFormView(FmFormModel* pModel, OutputDevice* pOut);
    virtual ~FmFormView();

    /** create a control pair (label/bound control) for the database field description given.
        @param rFieldDesc
            description of the field. see clipboard format SBA-FIELDFORMAT
        @deprecated
            This method is deprecated. Use the version with a ODataAccessDescriptor instead.
    */

    /** create a control pair (label/bound control) for the database field description given.
    */

    virtual void MarkListHasChanged();
    virtual void DelWin(OutputDevice* pWin1);

    virtual SdrPageView* ShowPage(SdrPage* pPage, const Point& rOffs);
    virtual void HidePage(SdrPageView* pPV);

    // for copying complete form structures, not only control models

    /** grab the focus to the first form control on the view
        @param _bForceSync
            <TRUE/> if the handling should be done synchronously.
    */

    // SdrView
    BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);

protected:
    void ActivateControls(SdrPageView*);
    void DeactivateControls(SdrPageView*);

    // Hinweis an die UI, da?ein Control erzeugt worden ist
    void ChangeDesignMode(sal_Bool bDesign);

public:
    FmXFormView* GetImpl() const {return pImpl;}

private:
    const OutputDevice* GetActualOutDev() const {return pActualOutDev;}
    void AdjustMarks(const SdrMarkList& rMarkList);
    sal_Bool checkUnMarkAll(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xSource);
    FmFormObj* getMarkedGrid() const;
};

}//end of namespace binfilter
#endif          // _FML_FMVIEW_HXX

