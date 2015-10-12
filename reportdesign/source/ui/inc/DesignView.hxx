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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DESIGNVIEW_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DESIGNVIEW_HXX

#include <dbaccess/dataview.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <vcl/split.hxx>
#include <vcl/scrbar.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <svl/hint.hxx>
#include <svl/SfxBroadcaster.hxx>
#include "ReportDefines.hxx"
#include <svtools/colorcfg.hxx>
#include <svx/svdedtv.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/splitwin.hxx>
#include "MarkedSection.hxx"
#include "ScrollHelper.hxx"

class KeyEvent;
class MouseEvent;
class Timer;
namespace vcl { class Window; }

namespace rptui
{
    class OSectionView;
    class OReportController;
    class PropBrw;
    class OAddFieldWindow;
    class ONavigator;


    class ODesignView : public dbaui::ODataView, public SfxBroadcaster, public IMarkedSection
    {
    private:
        VclPtr<SplitWindow>                 m_aSplitWin;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>        m_xReportComponent;
        OReportController&                  m_rReportController;
        VclPtr<OScrollWindowHelper>         m_aScrollWindow;
        VclPtr<vcl::Window>                 m_pTaskPane;
        VclPtr<PropBrw>                     m_pPropWin;
        VclPtr<OAddFieldWindow>             m_pAddField;
        OSectionView*                       m_pCurrentView;
        VclPtr<ONavigator>                  m_pReportExplorer;
        Idle                                m_aMarkIdle;
        DlgEdMode                           m_eMode;
        sal_uInt16                          m_eActObj;
        Size                                m_aGridSizeCoarse;
        Size                                m_aGridSizeFine;
        bool                                m_bGridSnap;
        bool                                m_bDeleted;


        DECL_LINK_TYPED(MarkTimeout, Idle *, void);
        DECL_LINK_TYPED( SplitHdl, SplitWindow*, void );

        void ImplInitSettings();

        ODesignView(ODesignView&) = delete;
        void operator =(ODesignView&) = delete;
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect) override;
        // return the Rectangle where I can paint myself
        virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

    public:
        ODesignView(vcl::Window* pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&,
                    OReportController& _rController);
        virtual ~ODesignView();
        virtual void dispose() override;

        // Window overrides
        virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual bool PreNotify( NotifyEvent& rNEvt ) override;
        virtual void GetFocus() override;

        virtual void initialize() override;

        inline OReportController&   getController() const { return m_rReportController; }

        void            SetMode( DlgEdMode m_eMode );
        void            SetInsertObj( sal_uInt16 eObj,const OUString& _sShapeType = OUString());
        sal_uInt16          GetInsertObj() const { return m_eActObj;}
        OUString   GetInsertObjString() const;
        DlgEdMode       GetMode() const { return m_eMode; }

        /** cuts the current selection in this section
        */
        void Cut();

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

        /** align all marked objects in all sections
        */
        void alignMarkedObjects(sal_Int32 _nControlModification, bool _bAlignAtSection, bool bBoundRects = false);

        /** All objects will be marked.
        */
        void            SelectAll(const sal_uInt16 _nObjectType);

        /// checks if a selection exists
        bool            HasSelection() const;

        void            UpdatePropertyBrowserDelayed(OSectionView& _rView);

        sal_uInt16          getSectionCount() const;

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
                                    ,const OUString& _sColorEntry
                                    ,sal_uInt16 _nPosition = USHRT_MAX);

        inline Size     getGridSizeCoarse() const { return m_aGridSizeCoarse; }
        inline Size     getGridSizeFine() const { return m_aGridSizeFine; }
        inline bool     isGridSnap() const { return m_bGridSnap; }
        void            setGridSnap(bool bOn);
        void            setDragStripes(bool bOn);
        /** turns the grid on or off
        *
        * \param _bGridVisible
        */
        void            toggleGrid(bool _bGridVisible);

        void            togglePropertyBrowser(bool _bToogleOn);

        bool            isAddFieldVisible() const;
        void            toggleAddField();

        bool            isReportExplorerVisible() const;
        void            toggleReportExplorer();

        /** shows or hides the ruler.
        */
        void            showRuler(bool _bShow);

        /** unmark all objects on the views without the given one.
        *
        * @param _pSectionView The view where the objects should not be unmarked.
        */
        void            unmarkAllObjects(OSectionView* _pSectionView);

        /** triggers the property browser with the section
            @param  _xReportComponent the report component
        */
        void            showProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xReportComponent);
        ::com::sun::star::uno::Any getCurrentlyShownProperty() const;

        /** returns the current section or the detail section if no section was selected previously
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getCurrentSection() const;

        /** returns the current control report model or <NULL/>
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent > getCurrentControlModel() const;

        // IMarkedSection
        OSectionWindow* getMarkedSection(NearSectionAccess nsa = CURRENT) const override;
        OSectionWindow* getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const;
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
        void collapseSections(const com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aCollpasedSections);

        OUString  getCurrentPage() const;
        void             setCurrentPage(const OUString& _sLastActivePage);

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        bool         handleKeyEvent(const KeyEvent& _rEvent);

        /** set the section as marked or not marked
            @param  _pSectionView   the section where to set the marked flag
            @param  _bMark  the marked flag
        */
        void            setMarked(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection,bool _bMark);
        void            setMarked(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> >& _xShape,bool _bMark);

        /** returns if the view handles the event by itself
        *
        * \param _nId the command id
        * \return  <FALSE/> is the event is not handled by the view otherwise <TRUE/>
        */
        bool        isHandleEvent(sal_uInt16 _nId) const;

        sal_uInt32      getMarkedObjectCount() const;

        /** zoom the ruler and view windows
        */
        void            zoom(const Fraction& _aZoom);

        /** fills the vector with all selected control models
            /param  _rSelection The vector will be filled and will not be cleared before.
        */
        void fillControlModelSelection(::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rSelection) const;

        /** calculates the zoom factor.
            @param  _eType  which kind of zoom is needed
        */
        sal_uInt16 getZoomFactor(SvxZoomType _eType) const;
    };

}   //rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DESIGNVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
