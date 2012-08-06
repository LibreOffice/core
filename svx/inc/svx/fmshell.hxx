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
#ifndef _SVX_FMSHELL_HXX
#define _SVX_FMSHELL_HXX

// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************

#include <sfx2/shell.hxx>
#include <sfx2/module.hxx>
#include <vcl/event.hxx>

#include <svx/svxids.hrc>
#include <svx/fmview.hxx>
#include "svx/svxdllapi.h"

#include <svx/ifaceids.hxx>

//========================================================================
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

//========================================================================
class SVX_DLLPUBLIC FmDesignModeChangedHint : public SfxHint
{
    sal_Bool m_bDesignMode;

public:
    TYPEINFO();
    FmDesignModeChangedHint( sal_Bool bDesMode );
    virtual ~FmDesignModeChangedHint();

    sal_Bool GetDesignMode() const { return m_bDesignMode; }
};

//========================================================================
class SVX_DLLPUBLIC FmFormShell : public SfxShell
{
    friend class FmFormView;
    friend class FmXFormShell;

    FmXFormShell*   m_pImpl;
    FmFormView*     m_pFormView;
    FmFormModel*    m_pFormModel;
    SfxViewShell*   m_pParentShell;

    sal_uInt16  m_nLastSlot;
    sal_Bool    m_bDesignMode : 1;
    sal_Bool    m_bHasForms : 1;    // flag storing if the forms on a page exist,
                                        // only for the DesignMode, see UIFeatureChanged!

    // the marks of a FormView have changed...
    void NotifyMarkListChanged(FmFormView*);
        // (the FormView itself is not a broadcaster, therefore it can't always correctly notify the
        // form explorer who is interested in the event)

    class FormShellWaitObject
    {
        Window* m_pWindow;
    public:
        FormShellWaitObject(const FmFormShell* _pShell);
        ~FormShellWaitObject();
    };
    friend class FormShellWaitObject;

    const OutputDevice* GetCurrentViewDevice() const { return m_pFormView ? m_pFormView->GetActualOutDev() : NULL; }

public:
    SFX_DECL_INTERFACE(SVX_INTERFACE_FORM_SH)
    TYPEINFO();

    FmFormShell(SfxViewShell* pParent, FmFormView* pView = NULL);
    virtual ~FmFormShell();

    virtual void Execute( SfxRequest& );
    virtual void GetState( SfxItemSet& );
    virtual sal_Bool HasUIFeature( sal_uInt32 nFeature );

    void ExecuteTextAttribute( SfxRequest& );
    void GetTextAttributeState( SfxItemSet& );

    sal_Bool GetY2KState(sal_uInt16& nReturn);
    void SetY2KState(sal_uInt16 n);

    void SetView(FmFormView* pView);

    FmFormView*  GetFormView() const { return m_pFormView; }
    FmFormModel* GetFormModel() const { return m_pFormModel; }
    FmFormPage*  GetCurPage() const;
    FmXFormShell* GetImpl() const {return m_pImpl;};

    sal_uInt16  PrepareClose(sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False);

    bool        IsActiveControl() const;
    void        ForgetActiveControl();
    void        SetControlActivationHandler( const Link& _rHdl );

    virtual void    Activate(sal_Bool bMDI);
    virtual void    Deactivate(sal_Bool bMDI);

    // helper methods for implementing XFormLayerAccess
    SdrUnoObj* GetFormControl(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel,
        const SdrView& _rView,
        const OutputDevice& _rDevice,
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _out_rxControl
    ) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > GetFormController(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm,
        const SdrView& _rView,
        const OutputDevice& _rDevice
    ) const;

    /** puts the focus into the document window, if current a form control has the focus. Otherwise, moves the focus
        to the control belonging to the given SdrUnoObj.
    */
    void    ToggleControlFocus(
        const SdrUnoObj& i_rNextCandidate,
        const SdrView& i_rView,
              OutputDevice& i_rDevice
    ) const;

    ::std::auto_ptr< ::svx::ISdrObjectFilter >
            CreateFocusableControlFilter(
                const SdrView& i_rView,
                const OutputDevice& i_rDevice
            ) const;

    sal_Bool    IsDesignMode() const { return m_bDesignMode; }
    void        SetDesignMode( sal_Bool _bDesignMode );

protected:
    void GetFormState(SfxItemSet &rSet, sal_uInt16 nWhich);

    // is there a form on the current page?
    void DetermineForms(sal_Bool bInvalidate);
    void impl_setDesignMode( sal_Bool bDesign);
};

// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************

#endif          // _SVX_FMSHELL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
