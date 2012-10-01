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
#ifndef RPTUI_VIEWSWINDOW_HXX
#define RPTUI_VIEWSWINDOW_HXX

#include <com/sun/star/report/XSection.hpp>
#include <vcl/window.hxx>
#include <svtools/colorcfg.hxx>
#include "ReportDefines.hxx"
#include "ReportSection.hxx"
#include <comphelper/propmultiplex.hxx>
#include "cppuhelper/basemutex.hxx"
#include <svtools/colorcfg.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <svx/svdedtv.hxx>
#include <SectionView.hxx>
#include <unotools/options.hxx>
#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <MarkedSection.hxx>
#include <SectionWindow.hxx>

class SdrHdl;
namespace rptui
{
    class OReportWindow;
    class OReportSection;
    class OSectionView;


    // -----------------------------------------------------------------------------
    struct RectangleLess : public ::std::binary_function< Rectangle, Rectangle, bool>
    {
        enum CompareMode { POS_LEFT,POS_RIGHT,POS_UPPER,POS_DOWN,POS_CENTER_HORIZONTAL,POS_CENTER_VERTICAL };
        CompareMode m_eCompareMode;
        Point       m_aRefPoint;
        RectangleLess(CompareMode _eCompareMode,const Point& _rRefPoint ) : m_eCompareMode(_eCompareMode),m_aRefPoint(_rRefPoint){}
        bool operator() (const Rectangle& lhs, const Rectangle& rhs) const
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
                return abs(m_aRefPoint.X() - lhs.Center().X()) < abs(m_aRefPoint.X() - rhs.Center().X());
            case POS_CENTER_VERTICAL:
                return abs(lhs.Center().Y() - m_aRefPoint.Y()) < abs(rhs.Center().Y() - m_aRefPoint.Y());
            }
            return false;
        }
    };

    class OWindowPositionCorrector
    {
        ::std::vector< ::std::pair<Window*,Point> > m_aChildren;
        long m_nDeltaX;
        long m_nDeltaY;
    public:
        OWindowPositionCorrector(Window* _pWindow,long _nDeltaX, long _nDeltaY) :m_nDeltaX(_nDeltaX), m_nDeltaY(_nDeltaY)
        {
            sal_uInt16 nCount = _pWindow->GetChildCount();
            m_aChildren.reserve(nCount);
            while( nCount )
            {
                Window* pChild = _pWindow->GetChild(--nCount);
                m_aChildren.push_back(::std::pair<Window*,Point>(pChild,pChild->GetPosPixel()));
            }
        }
        ~OWindowPositionCorrector()
        {
            ::std::vector< ::std::pair<Window*,Point> >::iterator aIter = m_aChildren.begin();
            ::std::vector< ::std::pair<Window*,Point> >::iterator aEnd = m_aChildren.end();
            for (; aIter != aEnd; ++aIter)
            {
                const Point aPos = aIter->first->GetPosPixel();
                if ( aPos == aIter->second )
                    aIter->first->SetPosPixel(Point(m_nDeltaX,m_nDeltaY) + aPos);
            }
        }
    };

    class OViewsWindow :    public Window
                        ,   public utl::ConfigurationListener
                        ,   public IMarkedSection
    {
        typedef ::std::multimap<Rectangle,::std::pair<SdrObject*,OSectionView*>,RectangleLess>      TRectangleMap;
    public:
        typedef ::std::vector< ::boost::shared_ptr<OSectionWindow> >                                TSectionsMap;

        struct TReportPairHelper : public ::std::unary_function< TSectionsMap::value_type, OReportSection >
        {
            OReportSection& operator() (const TSectionsMap::value_type& lhs) const
            {
                return lhs->getReportSection();
            }
        };
        struct TStartMarkerHelper : public ::std::unary_function< TSectionsMap::value_type, OStartMarker >
        {
            OStartMarker& operator() (const TSectionsMap::value_type& lhs) const
            {
                return lhs->getStartMarker();
            }
        };
    private:
        TSectionsMap                            m_aSections;
        svtools::ColorConfig                    m_aColorConfig;
        OReportWindow*                          m_pParent;
        ::rtl::OUString                         m_sShapeType;
        sal_Bool                                m_bInSplitHandler;
        sal_Bool                                m_bInUnmark;

        void ImplInitSettings();
        /** returns the iterator at pos _nPos or the end()
        */
        TSectionsMap::iterator getIteratorAtPos(sal_uInt16 _nPos);
        void collectRectangles(TRectangleMap& _rMap,bool _bBoundRects);
        void collectBoundResizeRect(const TRectangleMap& _rSortRectangles,sal_Int32 _nControlModification,bool _bAlignAtSection,bool _bBoundRects,Rectangle& _rBound,Rectangle& _rResize);
        void impl_resizeSectionWindow(OSectionWindow& _rSectionWindow,Point& _rStartPoint,bool _bSet);

        OViewsWindow(OViewsWindow&);
        void operator =(OViewsWindow&);
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        // windows overload
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual void MouseButtonUp( const MouseEvent& rMEvt );

        virtual void Paint( const Rectangle& rRect );
        virtual void ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
    public:
        OViewsWindow(
            OReportWindow* _pReportWindow);
        virtual ~OViewsWindow();

        // windows overload
        virtual void Resize();

        void resize(const OSectionWindow& _rSectionWindow);

        /** late ctor
        */
        void initialize();

        inline OReportWindow*       getView()           const { return m_pParent; }

        /** removes the section at the given position.
        *
        * \param _nPosition Zero based.
        */
        void            removeSection(sal_uInt16 _nPosition);

        /** adds a new section at position _nPosition.
            If the section is <NULL/> nothing happens.
            If the position is grater than the current elements, the section will be appended.
        */
        void            addSection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                                    ,const ::rtl::OUString& _sColorEntry
                                    ,sal_uInt16 _nPosition = USHRT_MAX);

        sal_uInt16          getSectionCount() const;
        /** return the section at the given position
        *
        * \param _nPos
        * \return the section at this pos or an empty section
        */
        ::boost::shared_ptr<OSectionWindow> getSectionWindow(const sal_uInt16 _nPos) const;

        /** turns the grid on or off
        *
        * \param _bVisible
        */
        void            toggleGrid(sal_Bool _bVisible);
        void            setGridSnap(sal_Bool bOn);
        void            setDragStripes(sal_Bool bOn);

        /** returns the total accumulated height of all sections until _pSection is reached
        */
        sal_Int32       getTotalHeight() const;

        inline bool     empty() const { return m_aSections.empty(); }
        void            SetMode( DlgEdMode m_eMode );
        void            SetInsertObj( sal_uInt16 eObj,const ::rtl::OUString& _sShapeType = ::rtl::OUString());
        rtl::OUString   GetInsertObjString() const;
        /** copies the current selection in this section
        */
        void Copy();

        /** returns if paste is allowed
        *
        * \return <TRUE/> if paste is allowed
        */
        sal_Bool IsPasteAllowed() const;

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
        sal_Bool HasSelection() const;

        /** unmark all objects on the views without the given one.
        *
        * @param _pSectionView The view where the objects should not be unmarked.
        */
        void            unmarkAllObjects(OSectionView* _pSectionView);

        /** returns the report section window for the given xsection
            @param  _xSection   the section
        */
        ::boost::shared_ptr<OSectionWindow> getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const;

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        sal_Bool        handleKeyEvent(const KeyEvent& _rEvent);

        /** the the section as marked or not marked
            @param  _pSectionView   the section where to set the marked flag
            @param  _bMark  the marked flag
        */
        void            setMarked(OSectionView* _pSectionView,sal_Bool _bMark);
        void            setMarked(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection,sal_Bool _bMark);
        void            setMarked(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> >& _xShape,sal_Bool _bMark);

        // IMarkedSection
        ::boost::shared_ptr<OSectionWindow> getMarkedSection(NearSectionAccess nsa = CURRENT) const;
        virtual void markSection(const sal_uInt16 _nPos);

        /** align all marked objects in all sections
        */
        void alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool bBoundRects = false);

        /** creates a default object
        *
        */
        void createDefault();

        /** shows or hides the ruler.
        */
        void showRuler(sal_Bool _bShow);

        /** returns the currently set shape type.
        *
        * \return \member m_sShapeType
        */
        inline ::rtl::OUString getShapeType() const { return m_sShapeType; }

        /** returns the current position in the list
        */
        sal_uInt16 getPosition(const OSectionWindow* _pSectionWindow = NULL) const;

        /** calls on every section BrkAction
        *
        */
        void BrkAction();
        void BegMarkObj(const Point& _aPnt,const OSectionView* _pSection);

    private:
        void BegDragObj_createInvisibleObjectAtPosition(const Rectangle& _aRect, const OSectionView& _rSection);
        void EndDragObj_removeInvisibleObjects();
        Point m_aDragDelta;
        ::std::vector<SdrObject*> m_aBegDragTempList;
        bool isObjectInMyTempList(SdrObject *);
    public:
        void BegDragObj(const Point& _aPnt, SdrHdl* _pHdl,const OSectionView* _pSection);
        void EndDragObj(sal_Bool _bDragIntoNewSection,const OSectionView* _pSection,const Point& _aPnt);

        void EndAction();
        void ForceMarkedToAnotherPage();
        sal_Bool IsAction() const;
        sal_Bool IsDragObj() const;
        void handleKey(const KeyCode& _rCode);
        void stopScrollTimer();

        /** return the section at the given point which is relative to the given section
        *
        * \param _pSection the section which is used as reference point
        * \param _rPnt the point, it will be changed that it is inside the section which will be returned
        * \return the section
        */
        OSectionView* getSectionRelativeToPosition(const OSectionView* _pSection,Point& _rPnt);

        void MovAction(const Point& rPnt,const OSectionView* _pSection,bool _bMove /*= true */, bool _bControlKeySet);

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
        void collapseSections(const com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aCollpasedSections);

        /** zoom the ruler and view windows
        */
        void zoom(const Fraction& _aZoom);

        void scrollChildren(const Point& _aThumbPos);

        /** fills the vector with all selected control models
            /param  _rSelection The vector will be filled and will not be cleared before.
        */
        void fillControlModelSelection(::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rSelection) const;
    };
//==============================================================================
} // rptui
//==============================================================================
#endif // RPTUI_VIEWSWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
