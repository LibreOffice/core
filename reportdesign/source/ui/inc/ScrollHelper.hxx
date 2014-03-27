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
#ifndef RPTUI_SCROLLHELPER_HXX
#define RPTUI_SCROLLHELPER_HXX

#include <vcl/scrbar.hxx>
#include <com/sun/star/report/XSection.hpp>
#include <comphelper/propmultiplex.hxx>
#include "ReportDefines.hxx"
#include <svtools/colorcfg.hxx>
#include <svx/svdedtv.hxx>
#include "cppuhelper/basemutex.hxx"
#include <rtl/ref.hxx>
#include <boost/shared_ptr.hpp>
#include <vcl/dockwin.hxx>
#include <MarkedSection.hxx>
#include "ReportWindow.hxx"

namespace rptui
{
    class ODesignView;
    class OReportWindow;
    class OSectionView;

    /** This class defines the scrollable area of the report design. It includes
        the h-ruler and the sections, and end marker. Not the start marker.
    */
    typedef Window OScrollWindowHelper_BASE;
    class OScrollWindowHelper :     public ::cppu::BaseMutex
                                ,   public OScrollWindowHelper_BASE/*TabPage*/
                                ,   public ::comphelper::OPropertyChangeListener
                                    , public IMarkedSection
    {
    private:
        ScrollBar           m_aHScroll;
        ScrollBar           m_aVScroll;
        ScrollBarBox        m_aCornerWin;       // window in the bottom right corner
        Size                m_aTotalPixelSize;
        ODesignView*        m_pParent;
        OReportWindow       m_aReportWindow;
        ::rtl::Reference<comphelper::OPropertyChangeMultiplexer >
                            m_pReportDefintionMultiPlexer; // listener for property changes

        DECL_LINK( ScrollHdl, ScrollBar*);
        Size ResizeScrollBars();
        void ImplInitSettings();
        void impl_initScrollBar( ScrollBar& _rScrollBar ) const;

        OScrollWindowHelper(OScrollWindowHelper&);
        void operator =(OScrollWindowHelper&);
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
        // window
        virtual void            Resize() SAL_OVERRIDE;
        virtual bool            Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
        // OPropertyChangeListener
        virtual void            _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    public:
        OScrollWindowHelper( ODesignView* _pReportDesignView);
        virtual ~OScrollWindowHelper();

        /** late ctor
        */
        void                    initialize();

        inline Point            getThumbPos() const { return Point(m_aHScroll.GetThumbPos(),m_aVScroll.GetThumbPos())/*m_aScrollOffset*/; }
        inline const OReportWindow& getReportWindow() const { return m_aReportWindow; }
        void                    setTotalSize(sal_Int32 _nWidth ,sal_Int32 _nHeight);
        inline Size             getTotalSize() const { return m_aTotalPixelSize; }
        inline ScrollBar*       GetHScroll() { return &m_aHScroll; }
        inline ScrollBar*       GetVScroll() { return &m_aVScroll; }

        // forwards
        void                    SetMode( DlgEdMode _eMode );
        void                    SetInsertObj( sal_uInt16 eObj,const OUString& _sShapeType = OUString());
        OUString           GetInsertObjString() const;
        void                    setGridSnap(sal_Bool bOn);
        void                    setDragStripes(sal_Bool bOn);
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

        /** removes the section at the given position.
        *
        * \param _nPosition Zero based.
        */
        void                    removeSection(sal_uInt16 _nPosition);

        /** adds a new section at position _nPosition.
            If the section is <NULL/> nothing happens.
            If the position is grater than the current elements, the section will be appended.
        */
        void                    addSection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                                    ,const OUString& _sColorEntry
                                    ,sal_uInt16 _nPosition = USHRT_MAX);

        sal_uInt16                  getSectionCount() const;

        /** turns the grid on or off
        *
        * \param _bVisible
        */
        void                    toggleGrid(sal_Bool _bVisible);

        /** unmark all objects on the views without the given one.
        *
        * @param _pSectionView The view where the objects should not be unmarked.
        */
        void                    unmarkAllObjects(OSectionView* _pSectionView);

        /** shows or hides the ruler.
        */
        void                    showRuler(sal_Bool _bShow);

        /** calculate the max width of the markers
        *
        * @param _bWithEnd  if <TRUE/> the end marker will be used for calculation as well otherwise not.
        * \return the max width
        */
        sal_Int32               getMaxMarkerWidth(sal_Bool _bWithEnd) const;

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        sal_Bool                handleKeyEvent(const KeyEvent& _rEvent);

        /** the section as marked or not marked
            @param  _pSectionView   the section where to set the marked flag
            @param  _bMark  the marked flag
        */
        void                    setMarked(OSectionView* _pSectionView,sal_Bool _bMark);
        void                    setMarked(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection,sal_Bool _bMark);
        void                    setMarked(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> >& _xShape,sal_Bool _bMark);

        // IMarkedSection
        ::boost::shared_ptr<OSectionWindow> getMarkedSection(NearSectionAccess nsa = CURRENT) const SAL_OVERRIDE;
        ::boost::shared_ptr<OSectionWindow> getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const;
        virtual void markSection(const sal_uInt16 _nPos) SAL_OVERRIDE;


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

        /** align all marked objects in all sections
        *
        * \param eHor
        * \param eVert
        * \param bBoundRects
        */
        void alignMarkedObjects(sal_Int32 _nControlModification, bool _bAlignAtSection, bool bBoundRects = false);

        sal_uInt32 getMarkedObjectCount() const;

        /** zoom the ruler and view windows
        */
        void zoom(const Fraction& _aZoom);

        /** fills the vector with all selected control models
            /param  _rSelection The vector will be filled and will not be cleared before.
        */
        void fillControlModelSelection(::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rSelection) const;

        /** calculates the zoom factor.
            @param  _eType  which kind of zoom is needed
        */
        sal_uInt16 getZoomFactor(SvxZoomType _eType) const;
    };
}
#endif // RPTUI_SCROLLHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
