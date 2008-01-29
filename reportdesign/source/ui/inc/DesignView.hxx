#ifndef RPTUI_DESIGNVIEW_HXX
#define RPTUI_DESIGNVIEW_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DesignView.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:48:19 $
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

#ifndef DBAUI_DATAVIEW_HXX
#include <dbaccess/dataview.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCOMPONENT_HPP_
#include <com/sun/star/report/XReportComponent.hpp>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _SFXHINT_HXX
#include <svtools/hint.hxx>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef RPTUI_REPORT_DEFINES_HXX
#include "ReportDefines.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#include <boost/shared_ptr.hpp>
#ifndef _SVDEDTV_HXX
#include <svx/svdedtv.hxx>
#endif
#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif
#include <vcl/splitwin.hxx>
#include <MarkedSection.hxx>

class KeyEvent;
class MouseEvent;
class Timer;
class Window;

namespace rptui
{
    class OScrollWindowHelper;
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
        //Splitter                          m_aSplitter;
        SplitWindow*                        m_pSplitWin;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>        m_xReportComponent;
        OReportController*                  m_pReportController;
        OScrollWindowHelper*                m_pScrollWindow;
        Window*                             m_pTaskPane;
        PropBrw*                            m_pPropWin;
        OAddFieldWindow*                    m_pAddField;
        OSectionView*                       m_pCurrentView;
        ONavigator*                         m_pReportExplorer;
        Timer                               m_aMarkTimer;
        Point                               m_aScrollOffset;
        DlgEdMode                           m_eMode;
        USHORT                              m_nCurrentPosition;
        USHORT                              m_eActObj;
        BOOL                                m_bFirstDraw;
        Size                                m_aGridSize;
        BOOL                                m_bGridVisible;
        BOOL                                m_bGridSnap;
        BOOL                                m_bInSplitHandler;


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
                    OReportController* _pController);
        virtual ~ODesignView();

        // window overloads
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void GetFocus();

        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);

        virtual void initialize();

        inline OReportController*   getController() const { return m_pReportController; }

        void            SetMode( DlgEdMode m_eMode );
        void            SetInsertObj( USHORT eObj,const ::rtl::OUString& _sShapeType = ::rtl::OUString());
        USHORT          GetInsertObj() const;
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
        BOOL IsPasteAllowed();

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

        /** checks if alignment is possible in the current section
        */
        sal_Bool isAlignPossible() const;

        /** All objects will be marked.
        */
        void            SelectAll();

        /// checks if a selection exists
        BOOL            HasSelection();

        void            UpdatePropertyBrowserDelayed(OSectionView* _pView);

        USHORT          getSectionCount() const;

        /** removes the section at the given position.
        *
        * \param _nPosition Zero based.
        */
        void            removeSection(USHORT _nPosition);

        /** adds a new section at position _nPosition.
            If the section is <NULL/> nothing happens.
            If the position is grater than the current elements, the section will be appended.
        */
        void            addSection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                                    ,const ::rtl::OUString& _sColorEntry
                                    ,USHORT _nPosition = USHRT_MAX);

        inline Size     getGridSize() const { return m_aGridSize; }
        inline BOOL     isGridSnap() const { return m_bGridSnap; }
        void            setGridSnap(BOOL bOn);
        void            setDragStripes(BOOL bOn);
        BOOL            isDragStripes() const;
        /** turns the grid on or off
        *
        * \param _bGridVisible
        */
        void            toggleGrid(sal_Bool _bGridVisible);

        void            togglePropertyBrowser(sal_Bool _bToogleOn);

        BOOL            isAddFieldVisible() const;
        void            toggleAddField();

        BOOL            isReportExplorerVisible() const;
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
        ::boost::shared_ptr<OReportSection> getMarkedSection(NearSectionAccess nsa = CURRENT) const;
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

        /** calculate the max width of the markers
        *
        * @param _bWithEnd  if <TRUE/> the end marker will be used for calculation as well otherwise not.
        * \return the max width
        */
        sal_Int32        getMaxMarkerWidth(sal_Bool _bWithEnd) const;

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        sal_Bool         handleKeyEvent(const KeyEvent& _rEvent);

        /** set the section as marked or not marked
            @param  _pSectionView   the section where to set the marked flag
            @param  _bMark  the marked flag
        */
        void            setMarked(OSectionView* _pSectionView,sal_Bool _bMark);

        void            setMarked(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection,sal_Bool _bMark);
        void            setMarked(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> >& _xShape,sal_Bool _bMark);

        /** returns if the view handles the event by itself
        *
        * \param _nId the command id
        * \return  <FALSE/> is the event is not handled by the view otherwise <TRUE/>
        */
        sal_Bool        isHandleEvent(sal_uInt16 _nId) const;

        sal_uInt32      getMarkedObjectCount() const;
    };
//==================================================================
}   //rptui
//==================================================================
#endif // RPTUI_DESIGNVIEW_HXX

