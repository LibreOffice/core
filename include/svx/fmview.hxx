/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_FMVIEW_HXX
#define INCLUDED_SVX_FMVIEW_HXX

#include <svx/view3d.hxx>
#include <comphelper/uno3.hxx>
#include <svx/svxdllapi.h>

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
    TYPEINFO_OVERRIDE();

    FmFormView(FmFormModel* pModel, OutputDevice* pOut = nullptr);
    virtual ~FmFormView();

    /** create a control pair (label/bound control) for the database field description given.
        @param rFieldDesc
            description of the field. see clipboard format SBA-FIELDFORMAT
        @deprecated
            This method is deprecated. Use the version with a ODataAccessDescriptor instead.
    */
    SdrObject*   CreateFieldControl(const OUString& rFieldDesc) const;

    /** create a control pair (label/bound control) for the database field description given.
    */
    SdrObject*   CreateFieldControl( const svx::ODataAccessDescriptor& _rColumnDescriptor );

    /** create a control pair (label/bound control) for the xforms description given.
    */
    SdrObject*   CreateXFormsControl( const svx::OXFormsDescriptor &_rDesc );

    virtual void MarkListHasChanged() override;
    virtual void AddWindowToPaintView(OutputDevice* pNewWin, vcl::Window* pWindow) override;
    virtual void DeleteWindowFromPaintView(OutputDevice* pOldWin) override;

    static void createControlLabelPair(
        OutputDevice* _pOutDev,
        sal_Int32 _nXOffsetMM,
        sal_Int32 _nYOffsetMM,
        const css::uno::Reference< css::beans::XPropertySet >& _rxField,
        const css::uno::Reference< css::util::XNumberFormats >& _rxNumberFormats,
        sal_uInt16 _nControlObjectID,
        const OUString& _rFieldPostfix,
        sal_uInt32 _nInventor,
        sal_uInt16 _nLabelObjectID,
        SdrPage* _pLabelPage,
        SdrPage* _pControlPage,
        SdrModel* _pModel,
        SdrUnoObj*& _rpLabel,
        SdrUnoObj*& _rpControl
    );

    virtual SdrPageView* ShowSdrPage(SdrPage* pPage) override;
    virtual void HideSdrPage() override;

    // for copying complete form structures, not only control models
    virtual SdrModel* GetMarkedObjModel() const override;

    virtual bool MouseButtonDown( const MouseEvent& _rMEvt, vcl::Window* _pWin ) override;

    /** grab the focus to the first form control on the view
        @param _bForceSync
            <TRUE/> if the handling should be done synchronously.
    */
    SVX_DLLPRIVATE void GrabFirstControlFocus( bool _bForceSync = false );

    /** returns the form controller for a given form and a given device
    */
    SVX_DLLPRIVATE css::uno::Reference< css::form::runtime::XFormController >
            GetFormController( const css::uno::Reference< css::form::XForm >& _rxForm, const OutputDevice& _rDevice ) const;

    // SdrView
    bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) override;

    /// shortcut to "GetSdrPageView() ? PTR_CAST( FmFormPage, GetSdrPageView() ) : NULL"
    FmFormPage* GetCurPage();

    SVX_DLLPRIVATE void ActivateControls(SdrPageView*);
    SVX_DLLPRIVATE void DeactivateControls(SdrPageView*);

    SVX_DLLPRIVATE void ChangeDesignMode(bool bDesign);

    SVX_DLLPRIVATE FmXFormView* GetImpl() const { return pImpl; }
    SVX_DLLPRIVATE FmFormShell* GetFormShell() const { return pFormShell; }

    struct FormShellAccess { friend class FmFormShell; private: FormShellAccess() { } };
     void SetFormShell( FmFormShell* pShell, FormShellAccess ) { pFormShell = pShell; }

    struct ImplAccess { friend class FmXFormView; private: ImplAccess() { } };
    void SetMoveOutside( bool _bMoveOutside, ImplAccess ) { E3dView::SetMoveOutside( _bMoveOutside ); }
     void InsertControlContainer(const css::uno::Reference< css::awt::XControlContainer >& xCC);
     void RemoveControlContainer(const css::uno::Reference< css::awt::XControlContainer >& xCC);

    virtual SdrPaintWindow* BeginCompleteRedraw(OutputDevice* pOut) override;
    virtual void EndCompleteRedraw(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer) override;

    SVX_DLLPRIVATE const OutputDevice* GetActualOutDev() const {return mpActualOutDev;}
    SVX_DLLPRIVATE bool checkUnMarkAll(const css::uno::Reference< css::uno::XInterface >& _xSource);

private:
    SVX_DLLPRIVATE FmFormObj* getMarkedGrid() const;
 protected:
    using E3dView::SetMoveOutside;
};

#endif          // _FML_FMVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
