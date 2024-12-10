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
#include <svx/svxdllapi.h>

namespace com::sun::star {
    namespace beans {
        class XPropertySet;
    }
    namespace util {
        class XNumberFormats;
    }
}

class OutputDevice;
class FmFormObj;
class FmFormPage;
class FmFormShell;
class FmXFormView;

namespace svx {
    class ODataAccessDescriptor;
    struct OXFormsDescriptor;
}

class SdrUnoObj;
namespace com::sun::star::form {
    class XForm;
    namespace runtime {
        class XFormController;
    }
}

class SVXCORE_DLLPUBLIC FmFormView : public E3dView
{
    rtl::Reference<FmXFormView> m_pImpl;
    FmFormShell*    m_pFormShell;

    void Init();

public:
    FmFormView(
        SdrModel& rSdrModel,
        OutputDevice* pOut);

    virtual ~FmFormView() override;

    /** create a control pair (label/bound control) for the database field description given.
        @param rFieldDesc
            description of the field. see clipboard format SBA-FIELDFORMAT
        @deprecated
            This method is deprecated. Use the version with an ODataAccessDescriptor instead.
    */
    rtl::Reference<SdrObject> CreateFieldControl(std::u16string_view rFieldDesc) const;

    /** create a control pair (label/bound control) for the database field description given.
    */
    rtl::Reference<SdrObject> CreateFieldControl( const svx::ODataAccessDescriptor& _rColumnDescriptor );

    /** create a control pair (label/bound control) for the xforms description given.
    */
    rtl::Reference<SdrObject> CreateXFormsControl( const svx::OXFormsDescriptor &_rDesc );

    virtual void MarkListHasChanged() override;
    virtual void AddDeviceToPaintView(OutputDevice& rNewDev, vcl::Window* pWindow) override;
    virtual void DeleteDeviceFromPaintView(OutputDevice& rOldDev) override;

    static void createControlLabelPair(
        OutputDevice const * _pOutDev,
        sal_Int32 _nXOffsetMM,
        sal_Int32 _nYOffsetMM,
        const css::uno::Reference< css::beans::XPropertySet >& _rxField,
        const css::uno::Reference< css::util::XNumberFormats >& _rxNumberFormats,
        SdrObjKind _nControlObjectID,
        SdrInventor _nInventor,
        SdrObjKind _nLabelObjectID,

        // tdf#118963 Need a SdrModel for SdrObject creation. To make the
        // demand clear, hand over a SdrMldel&
        SdrModel& _rModel,

        rtl::Reference<SdrUnoObj>& _rpLabel,
        rtl::Reference<SdrUnoObj>& _rpControl
    );

    virtual SdrPageView* ShowSdrPage(SdrPage* pPage) override;
    virtual void HideSdrPage() override;

    virtual bool MouseButtonDown( const MouseEvent& _rMEvt, OutputDevice* _pWin ) override;

    /** grab the focus to the first form control on the view
    */
    SVX_DLLPRIVATE void GrabFirstControlFocus();

    /** returns the form controller for a given form and a given device
    */
    SVX_DLLPRIVATE css::uno::Reference< css::form::runtime::XFormController >
            GetFormController( const css::uno::Reference< css::form::XForm >& _rxForm, const OutputDevice& _rDevice ) const;

    // SdrView
    bool KeyInput(const KeyEvent& rKEvt, vcl::Window* pWin) override;

    /// shortcut to "GetSdrPageView() ? PTR_CAST( FmFormPage, GetSdrPageView() ) : NULL"
    FmFormPage* GetCurPage();

    SVX_DLLPRIVATE void ActivateControls(SdrPageView const *);
    SVX_DLLPRIVATE void DeactivateControls(SdrPageView const *);

    SVX_DLLPRIVATE void ChangeDesignMode(bool bDesign);

    SVX_DLLPRIVATE FmXFormView* GetImpl() const { return m_pImpl.get(); }
    SVX_DLLPRIVATE FmFormShell* GetFormShell() const { return m_pFormShell; }

    struct FormShellAccess { friend class FmFormShell; private: FormShellAccess() { } };
     void SetFormShell( FmFormShell* pShell, FormShellAccess ) { m_pFormShell = pShell; }

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
