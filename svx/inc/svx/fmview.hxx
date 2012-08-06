/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <svx/view3d.hxx>
#include <comphelper/uno3.hxx>
#include "svx/svxdllapi.h"

namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertySet;
    }
    namespace util {
        class XNumberFormats;
    }
}}}

class OutputDevice;
class FmFormModel;
class FmFormObj;
class FmFormPage;
class FmFormShell;
class FmXFormView;

namespace svx {
    class ODataAccessDescriptor;
    struct OXFormsDescriptor;
}

class SdrUnoObj;
namespace com { namespace sun { namespace star { namespace form {
    class XForm;
    namespace runtime {
        class XFormController;
    }
} } } }

class SVX_DLLPUBLIC FmFormView : public E3dView
{
    FmXFormView*    pImpl;
    FmFormShell*    pFormShell;

    void Init();

public:
    TYPEINFO();

    FmFormView(FmFormModel* pModel, OutputDevice* pOut = 0L);
    virtual ~FmFormView();

    /** create a control pair (label/bound control) for the database field description given.
        @param rFieldDesc
            description of the field. see clipboard format SBA-FIELDFORMAT
        @deprecated
            This method is deprecated. Use the version with a ODataAccessDescriptor instead.
    */
    SdrObject*   CreateFieldControl(const UniString& rFieldDesc) const;

    /** create a control pair (label/bound control) for the database field description given.
    */
    SdrObject*   CreateFieldControl( const ::svx::ODataAccessDescriptor& _rColumnDescriptor );

    /** create a control pair (label/bound control) for the xforms description given.
    */
    SdrObject*   CreateXFormsControl( const ::svx::OXFormsDescriptor &_rDesc );

    virtual void MarkListHasChanged();
    virtual void AddWindowToPaintView(OutputDevice* pNewWin);
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin);

    static void createControlLabelPair(
        OutputDevice* _pOutDev,
        sal_Int32 _nXOffsetMM,
        sal_Int32 _nYOffsetMM,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >& _rxNumberFormats,
        sal_uInt16 _nControlObjectID,
        const ::rtl::OUString& _rFieldPostfix,
        sal_uInt32 _nInventor,
        sal_uInt16 _nLabelObjectID,
        SdrPage* _pLabelPage,
        SdrPage* _pControlPage,
        SdrModel* _pModel,
        SdrUnoObj*& _rpLabel,
        SdrUnoObj*& _rpControl
    );

    virtual SdrPageView* ShowSdrPage(SdrPage* pPage);
    virtual void HideSdrPage();

    // for copying complete form structures, not only control models
    virtual SdrModel* GetMarkedObjModel() const;
    using E3dView::Paste;
    virtual sal_Bool Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst=NULL, sal_uInt32 nOptions=0);

    virtual sal_Bool MouseButtonDown( const MouseEvent& _rMEvt, Window* _pWin );

    /** grab the focus to the first form control on the view
        @param _bForceSync
            <TRUE/> if the handling should be done synchronously.
    */
    SVX_DLLPRIVATE void GrabFirstControlFocus( sal_Bool _bForceSync = sal_False );

    /** returns the form controller for a given form and a given device
    */
    SVX_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController >
            GetFormController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm, const OutputDevice& _rDevice ) const;

    // SdrView
    sal_Bool KeyInput(const KeyEvent& rKEvt, Window* pWin);

    /// shortcut to "GetSdrPageView() ? PTR_CAST( FmFormPage, GetSdrPageView() ) : NULL"
    FmFormPage* GetCurPage();

    SVX_DLLPRIVATE void ActivateControls(SdrPageView*);
    SVX_DLLPRIVATE void DeactivateControls(SdrPageView*);

     SVX_DLLPRIVATE void ChangeDesignMode(sal_Bool bDesign);

     SVX_DLLPRIVATE FmXFormView* GetImpl() const { return pImpl; }
    SVX_DLLPRIVATE FmFormShell* GetFormShell() const { return pFormShell; }

    struct FormShellAccess { friend class FmFormShell; private: FormShellAccess() { } };
     void SetFormShell( FmFormShell* pShell, FormShellAccess ) { pFormShell = pShell; }

    struct ImplAccess { friend class FmXFormView; private: ImplAccess() { } };
    void SetMoveOutside( bool _bMoveOutside, ImplAccess ) { E3dView::SetMoveOutside( _bMoveOutside ); }
     virtual void InsertControlContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& xCC);
     virtual void RemoveControlContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& xCC);

    virtual SdrPaintWindow* BeginCompleteRedraw(OutputDevice* pOut);
    virtual void EndCompleteRedraw(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer);

    SVX_DLLPRIVATE const OutputDevice* GetActualOutDev() const {return pActualOutDev;}
    SVX_DLLPRIVATE sal_Bool checkUnMarkAll(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xSource);

private:
    SVX_DLLPRIVATE void AdjustMarks(const SdrMarkList& rMarkList);
    SVX_DLLPRIVATE FmFormObj* getMarkedGrid() const;
 protected:
    using E3dView::SetMoveOutside;
};

#endif          // _FML_FMVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
