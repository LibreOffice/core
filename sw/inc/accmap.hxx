/*************************************************************************
 *
 *  $RCSfile: accmap.hxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: mib $ $Date: 2002-05-27 12:37:59 $
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
#ifndef _ACCMAP_HXX
#define _ACCMAP_HXX

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <drafts/com/sun/star/accessibility/XAccessible.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_IACCESSIBLE_VIEW_FORWARDER_HXX
#include <svx/IAccessibleViewForwarder.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_IACCESSIBLE_PARENT_HXX
#include <svx/IAccessibleParent.hxx>
#endif
#ifndef _VIEWSH_HXX
#include "viewsh.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _FRACT_HXX
#include <tools/fract.hxx>
#endif

#include <vector>

class Rectangle;
class SwFrm;
class SwRootFrm;
class SwAccessibleContext;
class SwAccessibleContextMap_Impl;
class SwAccessibleEventList_Impl;
class SwAccessibleEventMap_Impl;
class SwShapeList_Impl;
class SdrObject;
namespace accessibility { class AccessibleShape; }
class SwAccessibleShapeMap_Impl;
struct SwAccessibleEvent_Impl;
class SwRect;
class ViewShell;
class SwFrmOrObj;
class SwAccPreviewData;

// real states for events
#define ACC_STATE_EDITABLE 0x01
#define ACC_STATE_OPAQUE 0x02

// pseudo states for events
#define ACC_STATE_CARET 0x80
#define ACC_STATE_RELATION_FROM 0x40
#define ACC_STATE_RELATION_TO 0x20

#define ACC_STATE_RELATION_MASK 0x60

#define ACC_STATE_MASK 0x1F

class SwAccessibleMap : public accessibility::IAccessibleViewForwarder,
                        public accessibility::IAccessibleParent
{
    ::vos::OMutex maMutex;
    ::vos::OMutex maEventMutex;
    SwAccessibleContextMap_Impl *mpFrmMap;
    SwAccessibleShapeMap_Impl *mpShapeMap;
    SwShapeList_Impl *mpShapes;
    SwAccessibleEventList_Impl *mpEvents;
    SwAccessibleEventMap_Impl *mpEventMap;
    ViewShell *mpVSh;

    ::com::sun::star::uno::WeakReference < ::drafts::com::sun::star::accessibility::XAccessible > mxCursorContext;

    sal_Int32 mnPara;
    sal_Int32 mnFootnote;
    sal_Int32 mnEndnote;

    /// for page preview: store preview data, VisArea, and mapping of
    /// preview-to-display coordinates
    SwAccPreviewData* mpPreview;

    sal_Bool mbShapeSelected;

    void FireEvent( const SwAccessibleEvent_Impl& rEvent );

    void AppendEvent( const SwAccessibleEvent_Impl& rEvent );

    void InvalidateCursorPosition(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::accessibility::XAccessible>& rAcc );
    void DoInvalidateShapeSelection();
    void DoInvalidateShapeFocus();
    void InvalidateShapeSelection();

    void _InvalidateRelationSet( const SwFrm* pFrm, sal_Bool bFrom );

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>
            _GetDocumentView( sal_Bool bPagePreview );

public:

    SwAccessibleMap( ViewShell *pSh );
    ~SwAccessibleMap();

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> GetDocumentView();

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> GetDocumentPreview(
                sal_uInt8 nRow, sal_uInt8 nColumn, sal_Int16 nStartPage,
                const Size& rPageSize, const Point& rFreePoint,
                const Fraction& rScale );

    ::vos::ORef < SwAccessibleContext > GetContextImpl(
                                                 const SwFrm *pFrm,
                                                sal_Bool bCreate = sal_True );
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> GetContext(
                                                 const SwFrm *pFrm,
                                                sal_Bool bCreate = sal_True );

    ::vos::ORef < ::accessibility::AccessibleShape > GetContextImpl(
                                        const SdrObject *pObj,
                                        SwAccessibleContext *pParentImpl,
                                        sal_Bool bCreate = sal_True );
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible> GetContext(
                                        const SdrObject *pObj,
                                        SwAccessibleContext *pParentImpl,
                                        sal_Bool bCreate = sal_True );

    ViewShell *GetShell() const { return mpVSh; }
    inline const SwRect& GetVisArea() const;

    void RemoveContext( const SwFrm *pFrm );
    void RemoveContext( const SdrObject *pObj );

    // Dispose frame and its children if bRecursive is set
    void Dispose( const SwFrm *pFrm, const SdrObject *pObj,
                  sal_Bool bRecursive=sal_False );

    void InvalidatePosOrSize( const SwFrm *pFrm, const SdrObject *pObj,
                              const SwRect& rOldFrm );

    void InvalidateContent( const SwFrm *pFrm );

    void InvalidateCursorPosition( const SwFrm *pFrm );
    void InvalidateFocus();

    void SetCursorContext(
        const ::vos::ORef < SwAccessibleContext >& rCursorContext );

    // Invalidate state of whole tree. If an action is open, this call
    // is processed when the last action ends.
    void InvalidateStates( sal_uInt8 nStates );

    void InvalidateRelationSet( const SwFrm* pMaster, const SwFrm* pFollow );

    // update preview data (and fire events if necessary)
    void UpdatePreview( sal_uInt8 nRow, sal_uInt8 nColumn,
                        sal_Int16 nStartPage,
                        const Size& rPageSize, const Point& rFreePoint,
                        const Fraction& rScale );

    void FireEvents();


    // IAccessibleViewForwarder

    virtual sal_Bool IsValid() const;
    virtual Rectangle GetVisibleArea() const;
    virtual Point LogicToPixel (const Point& rPoint) const;
    virtual Size LogicToPixel (const Size& rSize) const;
    virtual Point PixelToLogic (const Point& rPoint) const;
    virtual Size PixelToLogic (const Size& rSize) const;

    // IAccessibleParent
    virtual sal_Bool ReplaceChild (
        ::accessibility::AccessibleShape* pCurrentChild,
        ::accessibility::AccessibleShape* pReplacement)
        throw (::com::sun::star::uno::RuntimeException);

    // additional Core/Pixel conversions for internal use; also works
    // for preview
    Point CoreToPixel (const Point& rPoint) const;
    Point PixelToCore (const Point& rPoint) const;
    Rectangle CoreToPixel (const Rectangle& rRect) const;
    Rectangle PixelToCore (const Rectangle& rRect) const;

private:
    inline PreviewAdjust(const Point& rPoint, sal_Bool bFromPreview) const;
};



// helper class that stores preview data
class SwAccPreviewData
{
    SwRect maVisArea;
    Fraction maScale;

    typedef std::vector<Rectangle> Rectangles;
    Rectangles maPreviewRects;
    Rectangles maLogicRects;

public:
    SwAccPreviewData();
    ~SwAccPreviewData();

    void Update( sal_uInt8 nRow,        // # rows in preview
                 sal_uInt8 nCol,        // # columns in preview
                 sal_uInt16 nStartPage, // start page (0 is before first page)
                 const Size& rPageSize, // size of an empty page
                 const Point& rFreePoint, // free space between pages (x,y)
                 const Fraction& rScale,// scale factor for preview
                 ViewShell* pShell );

    const SwRect& GetVisArea() const;
    Point PreviewToLogic(const Point& rPoint) const;
    Point LogicToPreview(const Point& rPoint) const;

    MapMode GetMapModeForPreview( ) const;

    /** Adjust the MapMode so that the preview page appears at the
     * proper position. rPoint identifies the page for which the
     * MapMode should be adjusted. If bFromPreview is true, rPoint is
     * a preview coordinate; else it's a document coordinate. */
    void AdjustMapMode( MapMode& rMapMode,
                        const Point& rPoint,
                        sal_Bool bFromPreview ) const;

    void AdjustMapMode( MapMode& rMapMode ) const;
};



inline const SwRect& SwAccessibleMap::GetVisArea() const
{
    DBG_ASSERT( !mpVSh->IsPreView() || (mpPreview != NULL),
                "preview without preview data?" );
    return mpVSh->IsPreView() ? mpPreview->GetVisArea() : mpVSh->VisArea();
}


#endif
