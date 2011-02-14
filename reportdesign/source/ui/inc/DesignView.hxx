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
#ifndef RPTUI_DESIGNVIEW_HXX
#define RPTUI_DESIGNVIEW_HXX

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
#include <svl/hint.hxx>
#include <svl/brdcst.hxx>
#include <comphelper/stl_types.hxx>
#include "ReportDefines.hxx"
#include <svtools/colorcfg.hxx>
#include <boost/shared_ptr.hpp>
#include <svx/svdedtv.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/splitwin.hxx>
#include <MarkedSection.hxx>
#include "ScrollHelper.hxx"

class KeyEvent;
class MouseEvent;
class Timer;
class Window;

namespace rptui
{
    class OReportSection;
    class OUnoObject;
    class OSectionView;
    class OReportController;
    class PropBrw;
    class OAddFieldWindow;
    class OReportModel;
    class ONavigator;
    //==================================================================
    //==================================================================
    class ODesignView : public dbaui::ODataView, public SfxBroadcaster, public IMarkedSection
    {
    private:
        SplitWindow                         m_aSplitWin;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>        m_xReportComponent;
        OReportController&                  m_rReportController;
        OScrollWindowHelper                 m_aScrollWindow;
        Window*                             m_pTaskPane;
        PropBrw*                            m_pPropWin;
        OAddFieldWindow*                    m_pAddField;
        OSectionView*                       m_pCurrentView;
        ONavigator*                         m_pReportExplorer;
        Timer                               m_aMarkTimer;
        Point                               m_aScrollOffset;
        DlgEdMode                           m_eMode;
        sal_uInt16                              m_nCurrentPosition;
        sal_uInt16                              m_eActObj;
        sal_Bool                                m_bFirstDraw;
        Size                                m_aGridSizeCoarse;
        Size                                m_aGridSizeFine;
        sal_Bool                                m_bGridVisible;
        sal_Bool                                m_bGridSnap;
        sal_Bool                                m_bDeleted;


        DECL_LINK( MarkTimeout, Timer * );
        DECL_LINK( SplitHdl, void* );

        void ImplInitSettings();

        ODesignView(ODesignView&);
        void operator =(ODesignView&);
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);
        // return the Rectangle where I can paint myself
        virtual void DataChanged( const DataChangedEvent& rDCEvt );

    public:
        ODesignView(Window* pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
                    OReportController& _rController);
        virtual ~ODesignView();

        // window overloads
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void GetFocus();

        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);

        virtual void initialize();

        inline OReportController&   getController() const { return m_rReportController; }

        void            SetMode( DlgEdMode m_eMode );
        void            SetInsertObj( sal_uInt16 eObj,const ::rtl::OUString& _sShapeType = ::rtl::OUString());
        sal_uInt16          GetInsertObj() const;
        rtl::OUString   GetInsertObjString() const;
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
        sal_Bool IsPasteAllowed() const;

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

#if 0
        /** checks if alignment is possible in the current section
        */
        sal_Bool isAlignPossible() const;
#endif

        /** All objects will be marked.
        */
        void            SelectAll(const sal_uInt16 _nObjectType);

        /// checks if a selection exists
        sal_Bool            HasSelection() const;

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
                                    ,const ::rtl::OUString& _sColorEntry
                                    ,sal_uInt16 _nPosition = USHRT_MAX);

        inline Size     getGridSizeCoarse() const { return m_aGridSizeCoarse; }
        inline Size     getGridSizeFine() const { return m_aGridSizeFine; }
        inline sal_Bool     isGridSnap() const { return m_bGridSnap; }
        void            setGridSnap(sal_Bool bOn);
        void            setDragStripes(sal_Bool bOn);
        /** turns the grid on or off
        *
        * \param _bGridVisible
        */
        void            toggleGrid(sal_Bool _bGridVisible);

        void            togglePropertyBrowser(sal_Bool _bToogleOn);

        sal_Bool            isAddFieldVisible() const;
        void            toggleAddField();

        sal_Bool            isReportExplorerVisible() const;
        void            toggleReportExplorer();

        /** shows or hides the ruler.
        */
        void            showRuler(sal_Bool _bShow);

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
        ::boost::shared_ptr<OSectionWindow> getMarkedSection(NearSectionAccess nsa = CURRENT) const;
        ::boost::shared_ptr<OSectionWindow> getSectionWindow(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) const;
        virtual void markSection(const sal_uInt16 _nPos);

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

        ::rtl::OUString  getCurrentPage() const;
        void             setCurrentPage(const ::rtl::OUString& _sLastActivePage);

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        sal_Bool         handleKeyEvent(const KeyEvent& _rEvent);

        /** set the section as marked or not marked
            @param  _pSectionView   the section where to set the marked flag
            @param  _bMark  the marked flag
        */
        void            setMarked(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection,sal_Bool _bMark);
        void            setMarked(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> >& _xShape,sal_Bool _bMark);

        /** returns if the view handles the event by itself
        *
        * \param _nId the command id
        * \return  <FALSE/> is the event is not handled by the view otherwise <TRUE/>
        */
        sal_Bool        isHandleEvent(sal_uInt16 _nId) const;

        sal_uInt32      getMarkedObjectCount() const;

        /** zoom the ruler and view windows
        */
        void            zoom(const Fraction& _aZoom);

        /** fills the vector with all selected control models
            /param  _rSelection The vector will be filled and will not be cleared before.
        */
        void fillControlModelSelection(::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rSelection) const;

        /** returns the selected field from the add field dialog
        */
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > getSelectedFieldDescriptors();

        /** calculates the zoom factor.
            @param  _eType  which kind of zoom is needed
        */
        sal_uInt16 getZoomFactor(SvxZoomType _eType) const;
    };
//==================================================================
}   //rptui
//==================================================================
#endif // RPTUI_DESIGNVIEW_HXX

