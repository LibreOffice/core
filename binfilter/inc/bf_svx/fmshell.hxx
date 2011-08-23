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

#ifndef _SFX_SHELL_HXX //autogen
#include <bf_sfx2/shell.hxx>
#endif

#ifndef _SFXMODULE_HXX //autogen
#include <bf_sfx2/module.hxx>
#endif

#ifndef _SV_EVENT_HXX //autogen
#include <vcl/event.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <bf_svx/svxids.hrc>
#endif

#ifndef _SVX_FMVIEW_HXX
#include <bf_svx/fmview.hxx>
#endif

#include <bf_svx/ifaceids.hxx>
namespace binfilter {

//========================================================================
class FmFormModel;
class FmFormPage;
class SvxFmTabWin;
class FmXFormShell;
class FmFormView;

//========================================================================
class FmDesignModeChangedHint : public SfxHint
{
    sal_Bool m_bDesignMode;

public:
    TYPEINFO();
    FmDesignModeChangedHint( sal_Bool bDesMode );
    virtual ~FmDesignModeChangedHint();

    sal_Bool GetDesignMode() const { return m_bDesignMode; }
};

//========================================================================
class FmFormShell : public SfxShell
{
    friend class FmFormView;
    friend class FmXFormShell;

    sal_uInt16	m_nLastSlot;
    sal_Bool	m_bDesignMode : 1;
    sal_Bool	m_bHasForms	: 1;	// Flag welches festhaelt, ob Formulare auf einer Seite
                                // vorhanden sind, nur für den DesignMode, siehe UIFeatureChanged!

    FmXFormShell*	m_pImpl;
    FmFormView*		m_pFormView;
    FmFormModel*	m_pFormModel;
    SfxViewShell*	m_pParentShell;

    // die Markierungen einer FormView haben sich geaendert ...
    void NotifyMarkListChanged(FmFormView*);
        // (die FormView selber ist kein Broadcaster, deshalb kann sie den Formular-Explorer, den dieses
        // Ereignis interesiert, nicht sauber verstaendigen (sie koennte sich lediglich von der Application
        // das Navigator-Fenster geben lassen, aber das wollen wir ja nicht, ge ? ...))

//#ifdef NOOLDSV
//	class FormShellWaitObject
//	{
//		Window*	m_pWindow;
//	public:
//		FormShellWaitObject(const FmFormShell* _pShell);
//		~FormShellWaitObject();
//	};
//	friend class FormShellWaitObject;
//
//	const OutputDevice* GetCurrentViewDevice() const { return m_pFormView ? m_pFormView->GetActualOutDev() : NULL; }
//#endif

public:
    SFX_DECL_INTERFACE(SVX_INTERFACE_FORM_SH);
    TYPEINFO();

    FmFormShell(SfxViewShell* pParent, FmFormView* pView = NULL);
    virtual ~FmFormShell();

    virtual void Execute( SfxRequest& );
    virtual void GetState( SfxItemSet& );
    virtual sal_Bool HasUIFeature( sal_uInt32 nFeature );


    void SetView(FmFormView* pView);

    FmFormView*  GetFormView() const { return m_pFormView; }
    FmFormModel* GetFormModel() const { return m_pFormModel; }
    FmFormPage*  GetCurPage() const;
    FmXFormShell* GetImpl() const {return m_pImpl;};

    sal_uInt16 PrepareClose(sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False);
    sal_Bool IsDesignMode() const {return m_bDesignMode;}

    virtual void	Activate(sal_Bool bMDI);
    virtual void	Deactivate(sal_Bool bMDI);

protected:

    // gibt es ein Formular auf der aktuellen Seite?
    void DetermineForms(sal_Bool bInvalidate);
    void SetDesignMode( sal_Bool bDesign);
};

// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************

}//end of namespace binfilter
#endif          // _SVX_FMSHELL_HXX
