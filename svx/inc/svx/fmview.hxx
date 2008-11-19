/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fmview.hxx,v $
 * $Revision: 1.7 $
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

FORWARD_DECLARE_INTERFACE(util,XNumberFormats)
FORWARD_DECLARE_INTERFACE(beans,XPropertySet)

class OutputDevice;
class FmFormModel;
class FmPageViewWinRec;
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
    class XFormController;
} } } }

class SVX_DLLPUBLIC FmFormView : public E3dView
{
    friend class FmXFormController;
    friend class FmFormShell;
    friend class FmXFormShell;
    friend class FmXFormView;
    friend class FmExplorer;
    friend class FmFormObj;

    FmXFormView*    pImpl;
    FmFormShell*    pFormShell;

    void Init();
    void SetFormShell( FmFormShell* pShell ) { pFormShell = pShell; }
    FmFormShell* GetFormShell() const { return pFormShell; }

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
        SdrView* _pView,
        OutputDevice* _pOutDev,
        sal_Int32 _nXOffsetMM,
        sal_Int32 _nYOffsetMM,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >& _rxNumberFormats,
        sal_uInt16 _nObjID,
        const ::rtl::OUString& _rFieldPostfix,
        UINT32 _nInventor,
        UINT16 _nIndent,
        SdrPage* _pLabelPage,
        SdrPage* _pPage,
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

    virtual BOOL MouseButtonDown( const MouseEvent& _rMEvt, Window* _pWin );

    /** grab the focus to the first form control on the view
        @param _bForceSync
            <TRUE/> if the handling should be done synchronously.
    */
    void    GrabFirstControlFocus( sal_Bool _bForceSync = sal_False );

    /** returns the form controller for a given form and a given device
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormController >
            GetFormController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& _rxForm, const OutputDevice& _rDevice ) const;

    // SdrView
    BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);

    /// shortcut to "GetSdrPageView() ? PTR_CAST( FmFormPage, GetSdrPageView() ) : NULL"
    FmFormPage* GetCurPage();

protected:
    void ActivateControls(SdrPageView*);
    void DeactivateControls(SdrPageView*);

    // Hinweis an die UI, daﬂ ein Control erzeugt worden ist
    void ObjectCreated(FmFormObj* pObj);
    void ChangeDesignMode(sal_Bool bDesign);

public:
    FmXFormView* GetImpl() const {return pImpl;}

    virtual void InsertControlContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& xCC);
    virtual void RemoveControlContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >& xCC);

    // the following is a hack, only to be used on the 3.0.1 branch, to prevent becoming
    // incompatible there
    // #i94033# / 2008-10-16 / frank.schoenheit@sun.com
    void    onBeginCompleteRedraw();
    void    onEndCompleteRedraw();

private:
    SVX_DLLPRIVATE const OutputDevice* GetActualOutDev() const {return pActualOutDev;}
    SVX_DLLPRIVATE void AdjustMarks(const SdrMarkList& rMarkList);
    SVX_DLLPRIVATE sal_Bool checkUnMarkAll(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xSource);
    SVX_DLLPRIVATE FmFormObj* getMarkedGrid() const;
};

#endif          // _FML_FMVIEW_HXX

