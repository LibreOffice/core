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
#ifndef INCLUDED_SW_INC_ACCMAP_HXX
#define INCLUDED_SW_INC_ACCMAP_HXX

#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/IAccessibleParent.hxx>
#include <tools/fract.hxx>

#include <svx/AccessibleControlShape.hxx>
#include <svx/AccessibleShape.hxx>
#include "fesh.hxx"
#include <vector>
#include <set>
#include <o3tl/typed_flags_set.hxx>

class SwAccessibleParagraph;
class SwViewShell;
class Rectangle;
class SwFrm;
class SwTextFrm;
class SwPageFrm;
class SwAccessibleContext;
class SwAccessibleContextMap_Impl;
class SwAccessibleEventList_Impl;
class SwAccessibleEventMap_Impl;
class SwShapeList_Impl;
class SdrObject;
namespace accessibility { class AccessibleShape; }
class SwAccessibleShapeMap_Impl;
struct SwAccessibleEvent_Impl;
class SwAccessibleSelectedParas_Impl;
class SwRect;
class MapMode;
class SwAccPreviewData;
struct PreviewPage;
namespace vcl { class Window; }

enum class AccessibleStates
{
    NONE                   = 0x0000,
    // real states for events
    EDITABLE               = 0x0001,
    OPAQUE                 = 0x0002,
    // pseudo states for events
    TEXT_ATTRIBUTE_CHANGED = 0x0200,
    TEXT_SELECTION_CHANGED = 0x0100,
    CARET                  = 0x0080,
    RELATION_FROM          = 0x0040,
    RELATION_TO            = 0x0020,
};
namespace o3tl
{
    template<> struct typed_flags<AccessibleStates> : is_typed_flags<AccessibleStates, 0x3e3> {};
}

class SwAccessibleMap : public ::accessibility::IAccessibleViewForwarder,
                        public ::accessibility::IAccessibleParent
{
    mutable ::osl::Mutex maMutex;
    ::osl::Mutex maEventMutex;
    SwAccessibleContextMap_Impl *mpFrmMap;
    SwAccessibleShapeMap_Impl *mpShapeMap;
    SwShapeList_Impl *mpShapes;
    SwAccessibleEventList_Impl *mpEvents;
    SwAccessibleEventMap_Impl *mpEventMap;
    // #i27301 data structure to keep information about
    // accessible paragraph, which have a selection.
    SwAccessibleSelectedParas_Impl* mpSelectedParas;
    SwViewShell *mpVSh;
    /// for page preview: store preview data, VisArea, and mapping of
    /// preview-to-display coordinates
    SwAccPreviewData* mpPreview;

    css::uno::WeakReference < css::accessibility::XAccessible > mxCursorContext;

    sal_Int32 mnPara;

    bool mbShapeSelected;

    void FireEvent( const SwAccessibleEvent_Impl& rEvent );

    void AppendEvent( const SwAccessibleEvent_Impl& rEvent );

    void InvalidateCursorPosition( const css::uno::Reference<css::accessibility::XAccessible>& rAcc );
    void DoInvalidateShapeSelection(bool bInvalidateFocusMode = false);

    void InvalidateShapeSelection();

    //mpSelectedFrmMap contains the old selected objects.
    SwAccessibleContextMap_Impl *mpSeletedFrmMap;
    //IvalidateShapeInParaSelection() method is responsible for the updating the selected states of the objects.
    void InvalidateShapeInParaSelection();

    void _InvalidateRelationSet( const SwFrm* pFrm, bool bFrom );

    css::uno::Reference<css::accessibility::XAccessible>
            _GetDocumentView( bool bPagePreview );

    /** method to build up a new data structure of the accessible paragraphs,
        which have a selection

        Important note: method has to used inside a mutual exclusive section

        @author OD
    */
    SwAccessibleSelectedParas_Impl* _BuildSelectedParas();

public:

    SwAccessibleMap( SwViewShell *pSh );
    virtual ~SwAccessibleMap();

    css::uno::Reference<css::accessibility::XAccessible> GetDocumentView();

    css::uno::Reference<css::accessibility::XAccessible> GetDocumentPreview(
                            const std::vector<PreviewPage*>& _rPreviewPages,
                            const Fraction&  _rScale,
                            const SwPageFrm* _pSelectedPageFrm,
                            const Size&      _rPreviewWinSize );

    ::rtl::Reference < SwAccessibleContext > GetContextImpl(
                                                 const SwFrm *pFrm,
                                                bool bCreate = true );
    css::uno::Reference<css::accessibility::XAccessible> GetContext(
                                                 const SwFrm *pFrm,
                                                bool bCreate = true );

    ::rtl::Reference < ::accessibility::AccessibleShape > GetContextImpl(
                                        const SdrObject *pObj,
                                        SwAccessibleContext *pParentImpl,
                                        bool bCreate = true );
    css::uno::Reference<css::accessibility::XAccessible> GetContext(
                                        const SdrObject *pObj,
                                        SwAccessibleContext *pParentImpl,
                                        bool bCreate = true );

    inline SwViewShell* GetShell() const
    {
        return mpVSh;
    }
    static bool IsInSameLevel(const SdrObject* pObj, const SwFEShell* pFESh);
    void AddShapeContext(const SdrObject *pObj,
                             css::uno::Reference < css::accessibility::XAccessible > xAccShape);

    void AddGroupContext(const SdrObject *pParentObj,
                    css::uno::Reference < css::accessibility::XAccessible > xAccParent);
    void RemoveGroupContext(const SdrObject *pParentObj, css::uno::Reference < css::accessibility::XAccessible > xAccParent);

    const SwRect& GetVisArea() const;

    /** get size of a dedicated preview page

        @author OD

        @param _nPreviewPageNum
        input parameter - physical page number of page visible in the page preview

        @return an object of class <Size>
    */
    Size GetPreviewPageSize( sal_uInt16 _nPreviewPageNum ) const;

    void RemoveContext( const SwFrm *pFrm );
    void RemoveContext( const SdrObject *pObj );

    // Dispose frame and its children if bRecursive is set
    void Dispose( const SwFrm* pFrm,
                  const SdrObject* pObj,
                  vcl::Window* pWindow,
                  bool bRecursive = false );

    void InvalidatePosOrSize( const SwFrm* pFrm,
                              const SdrObject* pObj,
                              vcl::Window* pWindow,
                              const SwRect& rOldFrm );

    void InvalidateContent( const SwFrm *pFrm );

    void InvalidateAttr( const SwTextFrm& rTextFrm );

    void InvalidateCursorPosition( const SwFrm *pFrm );
    void InvalidateFocus();
    void SetCursorContext(
        const ::rtl::Reference < SwAccessibleContext >& rCursorContext );

    // Invalidate state of whole tree. If an action is open, this call
    // is processed when the last action ends.
    void InvalidateStates( AccessibleStates _nStates,
                           const SwFrm* _pFrm = nullptr );

    void InvalidateRelationSet( const SwFrm* pMaster, const SwFrm* pFollow );

    /** invalidation CONTENT_FLOWS_FROM/_TO relation of a paragraph

        @author OD

        @param _rTextFrm
        input parameter - reference to paragraph, whose CONTENT_FLOWS_FROM/_TO
        has to be invalidated.

        @param _bFrom
        input parameter - boolean indicating, if relation CONTENT_FLOWS_FROM
        (value <true>) or CONTENT_FLOWS_TO (value <false>) has to be invalidated.
    */
    void InvalidateParaFlowRelation( const SwTextFrm& _rTextFrm,
                                     const bool _bFrom );

    /** invalidation of text selection of a paragraph

        @author OD
    */
    void InvalidateParaTextSelection( const SwTextFrm& _rTextFrm );

    /** invalidation of text selection of all paragraphs

        @author OD
    */
    void InvalidateTextSelectionOfAllParas();

    sal_Int32 GetChildIndex( const SwFrm& rParentFrm,
                             vcl::Window& rChild ) const;

    // update preview data (and fire events if necessary)
    void UpdatePreview( const std::vector<PreviewPage*>& _rPreviewPages,
                        const Fraction&  _rScale,
                        const SwPageFrm* _pSelectedPageFrm,
                        const Size&      _rPreviewWinSize );

    void InvalidatePreviewSelection( sal_uInt16 nSelPage );
    bool IsPageSelected( const SwPageFrm *pPageFrm ) const;

    void FireEvents();

    // IAccessibleViewForwarder

    virtual Rectangle GetVisibleArea() const override;
    virtual Point LogicToPixel (const Point& rPoint) const override;
    virtual Size LogicToPixel (const Size& rSize) const override;

    // IAccessibleParent
    virtual bool ReplaceChild (
        ::accessibility::AccessibleShape* pCurrentChild,
        const css::uno::Reference< css::drawing::XShape >& _rxShape,
        const long _nIndex,
        const ::accessibility::AccessibleShapeTreeInfo& _rShapeTreeInfo
    )   throw (css::uno::RuntimeException) override;
    virtual ::accessibility::AccessibleControlShape* GetAccControlShapeFromModel
        (css::beans::XPropertySet* pSet)
        throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::accessibility::XAccessible >   GetAccessibleCaption (
        const css::uno::Reference< css::drawing::XShape > & xShape)
    throw (css::uno::RuntimeException) override;

    // additional Core/Pixel conversions for internal use; also works
    // for preview
    Point PixelToCore (const Point& rPoint) const;
    Rectangle CoreToPixel (const Rectangle& rRect) const;

private:
    /** get mapping mode for LogicToPixel and PixelToLogic conversions

        Replacement method <PreviewAdjust(..)> by new method <GetMapMode>.
        Method returns mapping mode of current output device and adjusts it,
        if the shell is in page/print preview.
        Necessary, because <PreviewAdjust(..)> changes mapping mode at current
        output device for mapping logic document positions to page preview window
        positions and vice versa and doesn't take care to recover its changes.

        @author OD

        @param _rPoint
        input parameter - constant reference to point to determine the mapping
        mode adjustments for page/print preview.

        @param _orMapMode
        output parameter - reference to the mapping mode, which is determined
        by the method
    */
    void GetMapMode( const Point& _rPoint,
                     MapMode&     _orMapMode ) const;
public:
    virtual bool IsDocumentSelAll() override;

    css::uno::WeakReference < css::accessibility::XAccessible >
        GetCursorContext() const { return mxCursorContext; }

    //Para Container for InvalidateCursorPosition
    typedef std::set< SwAccessibleParagraph* >  SET_PARA;
    SET_PARA m_setParaAdd;
    SET_PARA m_setParaRemove;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
