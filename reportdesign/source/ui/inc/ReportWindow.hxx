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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTWINDOW_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTWINDOW_HXX

#include <com/sun/star/report/XSection.hpp>
#include "ReportDefines.hxx"
#include "StartMarker.hxx"
#include <svtools/ruler.hxx>
#include <svx/svdedtv.hxx>
#include <sfx2/zoomitem.hxx>

#include <vector>
#include <comphelper/propmultiplex.hxx>

#include "MarkedSection.hxx"
#include "ViewsWindow.hxx"

namespace rptui
{
    class ODesignView;
    class OScrollWindowHelper;
    class OSectionView;
    class DlgEdFactory;

    class OReportWindow :    public vcl::Window
                            , public IMarkedSection
                            , public ::cppu::BaseMutex
                            , public ::comphelper::OPropertyChangeListener
    {
        VclPtr<Ruler>                  m_aHRuler;
        VclPtr<ODesignView>            m_pView;
        VclPtr<OScrollWindowHelper>    m_pParent;
        VclPtr<OViewsWindow>           m_aViewsWindow;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>   m_pReportListener;
        ::std::unique_ptr<DlgEdFactory>
                                m_pObjFac;

        void ImplInitSettings();

        sal_Int32 GetTotalHeight() const;
        sal_Int32 impl_getRealPixelWidth() const;

        OReportWindow(OReportWindow&) = delete;
        void operator =(OReportWindow&) = delete;
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
        // OPropertyChangeListener
        virtual void    _propertyChanged(const css::beans::PropertyChangeEvent& _rEvent) throw( css::uno::RuntimeException) override;
    public:
        OReportWindow(OScrollWindowHelper* _pParent,ODesignView* _pView);
        virtual ~OReportWindow();
        virtual void dispose() override;

        // Window overrides
        virtual void Resize() override;

        inline ODesignView*         getReportView() const { return m_pView; }
        inline OScrollWindowHelper* getScrollWindow() const { return m_pParent; }

        void            SetMode( DlgEdMode m_eMode );
        void            SetInsertObj( sal_uInt16 eObj,const OUString& _sShapeType = OUString());
        OUString        GetInsertObjString() const;
        void            setGridSnap(bool bOn);
        void            setDragStripes(bool bOn);

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

        Point           getThumbPos() const;

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
                                    ,sal_uInt16 _nPosition = USHRT_MAX);

        sal_uInt16          getSectionCount() const;

        /** turns the grid on or off
        *
        * \param _bVisible
        */
        void            toggleGrid(bool _bVisible);


        /** shows the ruler
        */
        void            showRuler(bool _bShow);

        inline sal_Int32 getRulerHeight() const { return m_aHRuler->GetSizePixel().Height(); }

        /** returns the total width of the first section
        */
        sal_Int32       GetTotalWidth() const;

        /** calculate the max width of the markers
        *
        * @param _bWithEnd  if <TRUE/> the end marker will be used for calculation as well otherwise not.
        * \return the max width
        */
        sal_Int32       getMaxMarkerWidth(bool _bWithEnd) const;

        void            ScrollChildren(const Point& _aThumbPos);

        void            notifySizeChanged();

        /** unmark all objects on the views without the given one.
        *
        * @param _pSectionView The view where the objects should not be unmarked.
        */
        void            unmarkAllObjects(OSectionView* _pSectionView);

        /** triggers the property browser with the report component or section
            @param  _xReportComponent
        */
        void            showProperties( const css::uno::Reference< css::report::XSection>& _xReportComponent);

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
        OSectionWindow* getSectionWindow(const css::uno::Reference< css::report::XSection>& _xSection) const;
        virtual void markSection(const sal_uInt16 _nPos) override;


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
        void fillControlModelSelection(::std::vector< css::uno::Reference< css::uno::XInterface > >& _rSelection) const;

        /** calculates the zoom factor.
            @param  _eType  which kind of zoom is needed
        */
        sal_uInt16 getZoomFactor(SvxZoomType _eType) const;
    };

}   //rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTWINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
