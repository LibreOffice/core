/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmview.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:52:11 $
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

#ifndef _SVX_FMVIEW_HXX
#define _SVX_FMVIEW_HXX

#ifndef _E3D_VIEW3D_HXX
#include <svx/view3d.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

FORWARD_DECLARE_INTERFACE(util,XNumberFormats)
FORWARD_DECLARE_INTERFACE(beans,XPropertySet)

class OutputDevice;
class XOutputDevice;
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

    // SdrView
    BOOL KeyInput(const KeyEvent& rKEvt, Window* pWin);

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
private:
    SVX_DLLPRIVATE const OutputDevice* GetActualOutDev() const {return pActualOutDev;}
    SVX_DLLPRIVATE void AdjustMarks(const SdrMarkList& rMarkList);
    SVX_DLLPRIVATE sal_Bool checkUnMarkAll(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xSource);
    SVX_DLLPRIVATE FmFormObj* getMarkedGrid() const;
};

#endif          // _FML_FMVIEW_HXX

