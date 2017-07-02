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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_VIEWSWINDOW_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_VIEWSWINDOW_HXX

#include <com/sun/star/report/XSection.hpp>
#include <vcl/window.hxx>
#include <svtools/colorcfg.hxx>
#include "ReportDefines.hxx"
#include "ReportSection.hxx"
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <svx/svdedtv.hxx>
#include "SectionView.hxx"
#include <unotools/options.hxx>
#include <list>
#include <vector>

#include "MarkedSection.hxx"
#include "SectionWindow.hxx"

class SdrHdl;

namespace rptui
{
    class OReportWindow;
    class OReportSection;
    class OSectionView;
    enum class ControlModification;

    struct RectangleLess
    {
        enum CompareMode { POS_LEFT,POS_RIGHT,POS_UPPER,POS_DOWN,POS_CENTER_HORIZONTAL,POS_CENTER_VERTICAL };
        CompareMode m_eCompareMode;
        Point       m_aRefPoint;
        RectangleLess(CompareMode _eCompareMode,const Point& _rRefPoint ) : m_eCompareMode(_eCompareMode),m_aRefPoint(_rRefPoint){}
        bool operator() (const tools::Rectangle& lhs, const tools::Rectangle& rhs) const
        {
            switch(m_eCompareMode)
            {
            case POS_LEFT:
                return lhs.Left() < rhs.Left();
            case POS_RIGHT:
                return lhs.Right() >= rhs.Right();
            case POS_UPPER:
                return lhs.Top() < rhs.Top();
            case POS_DOWN:
                return lhs.Bottom() >= rhs.Bottom();
            case POS_CENTER_HORIZONTAL:
                return std::abs(m_aRefPoint.X() - lhs.Center().X()) < std::abs(m_aRefPoint.X() - rhs.Center().X());
            case POS_CENTER_VERTICAL:
                return std::abs(lhs.Center().Y() - m_aRefPoint.Y()) < std::abs(rhs.Center().Y() - m_aRefPoint.Y());
            }
            return false;
        }
    };

    class OViewsWindow :    public vcl::Window
                        ,   public utl::ConfigurationListener
                        ,   public IMarkedSection
    {
        typedef ::std::multimap<tools::Rectangle,::std::pair<SdrObject*,OSectionView*>,RectangleLess>      TRectangleMap;
    public:
        typedef ::std::vector< VclPtr<OSectionWindow> >                                TSectionsMap;

    private:
        TSectionsMap                            m_aSections;
        svtools::ColorConfig                    m_aColorConfig;
        VclPtr<OReportWindow>                   m_pParent;
        OUString                                m_sShapeType;
        bool                                    m_bInUnmark;

        void ImplInitSettings();
        /** returns the iterator at pos _nPos or the end()
        */
        TSectionsMap::iterator getIteratorAtPos(sal_uInt16 _nPos);
        void collectRectangles(TRectangleMap& _rMap);
        static void collectBoundResizeRect(const TRectangleMap& _rSortRectangles, ControlModification _nControlModification,bool _bAlignAtSection,tools::Rectangle& _rBound,tools::Rectangle& _rResize);
        void impl_resizeSectionWindow(OSectionWindow& _rSectionWindow,Point& _rStartPoint,bool _bSet);

        OViewsWindow(OViewsWindow&) = delete;
        void operator =(OViewsWindow&) = delete;
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
        // Window overrides
        virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;

        virtual void Paint( vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle& rRect ) override;
        virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;
    public:
        OViewsWindow(
            OReportWindow* _pReportWindow);
        virtual ~OViewsWindow() override;
        virtual void dispose() override;

        // Window overrides
        virtual void Resize() override;

        void resize(const OSectionWindow& _rSectionWindow);

        OReportWindow*       getView()           const { return m_pParent; }

        /** removes the section at the given position.
        *
        * \param _nPosition Zero based.
        */
        void            removeSection(sal_uInt16 _nPosition);

        /** adds a new section at position _nPosition.
            If the section is <NULL/> nothing happens.
            If the position is grater than the current elements, the section will be appended.
        */
        void            addSection(const css::uno::Reference< css::report::XSection >& _xSection
                                    ,const OUString& _sColorEntry
                                    ,sal_uInt16 _nPosition);

        sal_uInt16          getSectionCount() const;
        /** return the section at the given position
        *
        * \param _nPos
        * \return the section at this pos or an empty section
        */
        OSectionWindow* getSectionWindow(const sal_uInt16 _nPos) const;

        /** turns the grid on or off
        *
        * \param _bVisible
        */
        void            toggleGrid(bool _bVisible);
        void            setGridSnap(bool bOn);
        void            setDragStripes(bool bOn);

        /** returns the total accumulated height of all sections until _pSection is reached
        */
        sal_Int32       getTotalHeight() const;

        bool     empty() const { return m_aSections.empty(); }
        void            SetMode( DlgEdMode m_eMode );
        void            SetInsertObj( sal_uInt16 eObj,const OUString& _sShapeType);
        const OUString& GetInsertObjString() const { return m_sShapeType;}
        /** copies the current selection in this section
        */
        void Copy();

        /** returns if paste is allowed
        *
        * \return <TRUE/> if paste is allowed
        */
        bool IsPasteAllowed() const;

        /** paste a new control in this section
        */
        void Paste();

        /** Deletes the current selection in this section
        *
        */
        void Delete();

        /** All objects will be marked.
        */
        void SelectAll(const sal_uInt16 _nObjectType);

        /** returns <TRUE/> when a object is marked
        */
        bool HasSelection() const;

        /** unmark all objects on the views without the given one.
        *
        * @param _pSectionView The view where the objects should not be unmarked.
        */
        void            unmarkAllObjects(OSectionView* _pSectionView);

        /** returns the report section window for the given xsection
            @param  _xSection   the section
        */
        OSectionWindow* getSectionWindow(const css::uno::Reference< css::report::XSection>& _xSection) const;

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        bool            handleKeyEvent(const KeyEvent& _rEvent);

        /** the section as marked or not marked
            @param  _pSectionView   the section where to set the marked flag
            @param  _bMark  the marked flag
        */
        void            setMarked(OSectionView* _pSectionView, bool _bMark);
        void            setMarked(const css::uno::Reference< css::report::XSection>& _xSection, bool _bMark);
        void            setMarked(const css::uno::Sequence< css::uno::Reference< css::report::XReportComponent> >& _xShape, bool _bMark);

        // IMarkedSection
        OSectionWindow* getMarkedSection(NearSectionAccess nsa = CURRENT) const override;
        virtual void markSection(const sal_uInt16 _nPos) override;

        /** align all marked objects in all sections
        */
        void alignMarkedObjects(ControlModification _nControlModification, bool _bAlignAtSection);

        /** creates a default object
        *
        */
        void createDefault();

        /** shows or hides the ruler.
        */
        void showRuler(bool _bShow);

        /** returns the currently set shape type.
        *
        * \return \member m_sShapeType
        */
        const OUString& getShapeType() const { return m_sShapeType; }

        /** returns the current position in the list
        */
        sal_uInt16 getPosition(const OSectionWindow* _pSectionWindow) const;

        /** calls on every section BrkAction
        *
        */
        void BrkAction();
        void BegMarkObj(const Point& _aPnt,const OSectionView* _pSection);

    private:
        void BegDragObj_createInvisibleObjectAtPosition(const tools::Rectangle& _aRect, const OSectionView& _rSection);
        void EndDragObj_removeInvisibleObjects();
        Point m_aDragDelta;
        ::std::vector<SdrObject*> m_aBegDragTempList;
    public:
        void BegDragObj(const Point& _aPnt, SdrHdl* _pHdl,const OSectionView* _pSection);
        void EndDragObj(bool _bDragIntoNewSection,const OSectionView* _pSection,const Point& _aPnt);

        void EndAction();
        void ForceMarkedToAnotherPage();
        bool IsAction() const;
        bool IsDragObj() const;
        void handleKey(const vcl::KeyCode& _rCode);
        void stopScrollTimer();

        /** return the section at the given point which is relative to the given section
        *
        * \param _pSection the section which is used as reference point
        * \param _rPnt the point, it will be changed that it is inside the section which will be returned
        * \return the section
        */
        OSectionView* getSectionRelativeToPosition(const OSectionView* _pSection,Point& _rPnt);

        void MovAction(const Point& rPnt,const OSectionView* _pSection, bool _bControlKeySet);

        sal_uInt32 getMarkedObjectCount() const;

        /** fills the positions of all collapsed sections.
        *
        * \param _rCollapsedPositions Out parameter which holds afterwards all positions of the collapsed sections.
        */
        void fillCollapsedSections(::std::vector<sal_uInt16>& _rCollapsedPositions) const;

        /** collpase all sections given by their position
        *
        * \param _aCollpasedSections The position of the sections which should be collapsed.
        */
        void collapseSections(const css::uno::Sequence< css::beans::PropertyValue>& _aCollpasedSections);

        /** zoom the ruler and view windows
        */
        void zoom(const Fraction& _aZoom);

        void scrollChildren(const Point& _aThumbPos);

        /** fills the vector with all selected control models
            /param  _rSelection The vector will be filled and will not be cleared before.
        */
        void fillControlModelSelection(::std::vector< css::uno::Reference< css::uno::XInterface > >& _rSelection) const;
    };

} // rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_VIEWSWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
