#ifndef RPTUI_SCROLLHELPER_HXX
#define RPTUI_SCROLLHELPER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ScrollHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:48:43 $
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

#ifndef _SV_SCRBAR_HXX
#include <vcl/scrbar.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef RPTUI_REPORT_DEFINES_HXX
#include "ReportDefines.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _SVDEDTV_HXX
#include <svx/svdedtv.hxx>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include "cppuhelper/basemutex.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#include <boost/shared_ptr.hpp>
#include <vcl/dockwin.hxx>
#include <MarkedSection.hxx>

class SdrView;
namespace rptui
{
    class OSectionsWindow;
    class ODesignView;
    class OReportWindow;
    class OSectionView;
    class OReportSection;
    class OReportModel;

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
        Point               m_aPixOffset;       // offset to virtual window (pixel)
        ODesignView*        m_pParent;
        OReportWindow*      m_pChild;
        ::rtl::Reference<comphelper::OPropertyChangeMultiplexer >
                            m_pReportDefintionMultiPlexer; // listener for property changes

        DECL_LINK( ScrollHdl, ScrollBar*);
        Size ResizeScrollBars();
        void ImplInitSettings();
        void impl_initScrollBar( ScrollBar& _rScrollBar ) const;
        void impl_scrollContent( long nDeltaX, long nDeltaY );

        OScrollWindowHelper(OScrollWindowHelper&);
        void operator =(OScrollWindowHelper&);
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        // window
        virtual void            Resize();
        virtual long            Notify( NotifyEvent& rNEvt );
        // OPropertyChangeListener
        virtual void            _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
    public:
        OScrollWindowHelper( ODesignView* _pReportDesignView);
        virtual ~OScrollWindowHelper();

        /** late ctor
        */
        void                    initialize();

        void                    EndScroll( long nDeltaX, long nDeltaY );

        inline Point            getScrollOffset() const { return Point(m_aHScroll.GetThumbPos(),m_aVScroll.GetThumbPos())/*m_aScrollOffset*/; }
        inline OReportWindow*   getReportWindow() const { return m_pChild; }
        void                    setTotalSize(sal_Int32 _nWidth ,sal_Int32 _nHeight);
        inline Size             getTotalSize() const { return m_aTotalPixelSize; }
        inline ScrollBar*       GetHScroll() { return &m_aHScroll; }
        inline ScrollBar*       GetVScroll() { return &m_aVScroll; }

        // forwards
        void                    SetMode( DlgEdMode _eMode );
        void                    SetInsertObj( USHORT eObj,const ::rtl::OUString& _sShapeType = ::rtl::OUString());
        rtl::OUString           GetInsertObjString() const;
        void                    setGridSnap(BOOL bOn);
        void                    setDragStripes(BOOL bOn);
        BOOL                    isDragStripes() const;
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

        /** All objects will be marked.
        */
        void SelectAll();

        /** returns <TRUE/> when a object is marked
        */
        BOOL HasSelection();

        /** removes the section at the given position.
        *
        * \param _nPosition Zero based.
        */
        void                    removeSection(USHORT _nPosition);

        /** adds a new section at position _nPosition.
            If the section is <NULL/> nothing happens.
            If the position is grater than the current elements, the section will be appended.
        */
        void                    addSection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                                    ,const ::rtl::OUString& _sColorEntry
                                    ,USHORT _nPosition = USHRT_MAX);

        USHORT                  getSectionCount() const;

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

        /** the the section as marked or not marked
            @param  _pSectionView   the section where to set the marked flag
            @param  _bMark  the marked flag
        */
        void                    setMarked(OSectionView* _pSectionView,sal_Bool _bMark);
        void                    setMarked(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection,sal_Bool _bMark);
        void                    setMarked(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent> >& _xShape,sal_Bool _bMark);

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

        /** align all marked objects in all sections
        *
        * \param eHor
        * \param eVert
        * \param bBoundRects
        */
        void alignMarkedObjects(sal_Int32 _nControlModification, bool _bAlignAtSection, bool bBoundRects = false);

        sal_uInt32 getMarkedObjectCount() const;
    };
}
#endif // RPTUI_SCROLLHELPER_HXX
