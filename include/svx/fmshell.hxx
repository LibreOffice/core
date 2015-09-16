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
#ifndef INCLUDED_SVX_FMSHELL_HXX
#define INCLUDED_SVX_FMSHELL_HXX

// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************

#include <sfx2/shell.hxx>
#include <sfx2/module.hxx>
#include <vcl/event.hxx>

#include <svx/svxids.hrc>
#include <svx/fmview.hxx>
#include <svx/svxdllapi.h>

#include <svx/ifaceids.hxx>


class FmFormModel;
class FmFormPage;
class FmXFormShell;
class FmFormView;
class SdrView;
class SdrPage;
class SdrUnoObj;

namespace com { namespace sun { namespace star { namespace form {
    class XForm;
    namespace runtime {
        class XFormController;
    }
} } } }

namespace svx
{
    class ISdrObjectFilter;
}


class SVX_DLLPUBLIC SAL_WARN_UNUSED FmDesignModeChangedHint : public SfxHint
{
    bool m_bDesignMode;

public:
    FmDesignModeChangedHint( bool bDesMode );
    virtual ~FmDesignModeChangedHint();

    bool GetDesignMode() const { return m_bDesignMode; }
};

class SVX_DLLPUBLIC FmFormShell : public SfxShell
{
    friend class FmFormView;
    friend class FmXFormShell;

    FmXFormShell*   m_pImpl;
    FmFormView*     m_pFormView;
    FmFormModel*    m_pFormModel;
    SfxViewShell*   m_pParentShell;

    sal_uInt16  m_nLastSlot;
    bool        m_bDesignMode : 1;
    bool        m_bHasForms : 1;    // flag storing if the forms on a page exist,
                                        // only for the DesignMode, see UIFeatureChanged!

    // the marks of a FormView have changed...
    void NotifyMarkListChanged(FmFormView*);
        // (the FormView itself is not a broadcaster, therefore it can't always correctly notify the
        // form explorer who is interested in the event)

    class FormShellWaitObject
    {
        VclPtr<vcl::Window> m_pWindow;
    public:
        FormShellWaitObject(const FmFormShell* _pShell);
        ~FormShellWaitObject();
    };
    friend class FormShellWaitObject;

    const OutputDevice* GetCurrentViewDevice() const { return m_pFormView ? m_pFormView->GetActualOutDev() : NULL; }

public:
    SFX_DECL_INTERFACE(SVX_INTERFACE_FORM_SH)
    TYPEINFO_OVERRIDE();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    FmFormShell(SfxViewShell* pParent, FmFormView* pView = NULL);
    virtual ~FmFormShell();

    void Execute( SfxRequest& );
    void GetState( SfxItemSet& );
    virtual bool HasUIFeature( sal_uInt32 nFeature ) SAL_OVERRIDE;

    void ExecuteTextAttribute( SfxRequest& );
    void GetTextAttributeState( SfxItemSet& );

    bool GetY2KState(sal_uInt16& nReturn);
    void SetY2KState(sal_uInt16 n);

    void SetView(FmFormView* pView);

    FmFormView*  GetFormView() const { return m_pFormView; }
    FmFormModel* GetFormModel() const { return m_pFormModel; }
    FmFormPage*  GetCurPage() const;
    FmXFormShell* GetImpl() const {return m_pImpl;};

    bool PrepareClose(bool bUI = true);

    bool        IsActiveControl() const;
    void        ForgetActiveControl();
    void        SetControlActivationHandler( const Link<LinkParamNone*,void>& _rHdl );

    virtual void    Activate(bool bMDI) SAL_OVERRIDE;
    virtual void    Deactivate(bool bMDI) SAL_OVERRIDE;

    // helper methods for implementing XFormLayerAccess
    SdrUnoObj* GetFormControl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel,
        const SdrView& _rView,
        const OutputDevice& _rDevice,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _out_rxControl
    ) const;

    static ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > GetFormController(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
        const SdrView& _rView,
        const OutputDevice& _rDevice
    );

    /** puts the focus into the document window, if current a form control has the focus. Otherwise, moves the focus
        to the control belonging to the given SdrUnoObj.
    */
    void    ToggleControlFocus(
        const SdrUnoObj& i_rNextCandidate,
        const SdrView& i_rView,
              OutputDevice& i_rDevice
    ) const;

    static ::std::unique_ptr< svx::ISdrObjectFilter >
            CreateFocusableControlFilter(
                const SdrView& i_rView,
                const OutputDevice& i_rDevice
            );

    virtual bool IsDesignMode() const SAL_OVERRIDE { return m_bDesignMode; }
    void         SetDesignMode( bool _bDesignMode );

protected:
    void GetFormState(SfxItemSet &rSet, sal_uInt16 nWhich);

    // is there a form on the current page?
    void DetermineForms(bool bInvalidate);
    void impl_setDesignMode( bool bDesign);
};

#endif // INCLUDED_SVX_FMSHELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
