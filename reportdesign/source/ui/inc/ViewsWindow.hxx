#ifndef RPTUI_VIEWSWINDOW_HXX
#define RPTUI_VIEWSWINDOW_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewsWindow.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:49:05 $
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

#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef RPTUI_REPORT_DEFINES_HXX
#include "ReportDefines.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include "cppuhelper/basemutex.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _SVDEDTV_HXX
#include <svx/svdedtv.hxx>
#endif
#include <SectionView.hxx>

#include <list>
#include <vector>
#include <boost/shared_ptr.hpp>

#include <MarkedSection.hxx>

class Splitter;
class SdrHdl;
namespace comphelper
{
    class OPropertyChangeMultiplexer;
}
namespace rptui
{
    class OReportWindow;
    class ODesignView;
    class OEndMarker;
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

    class OViewsWindow :    public Window
                        ,   public ::cppu::BaseMutex
                        ,   public ::comphelper::OPropertyChangeListener
                        ,   public SfxListener
                        ,   public IMarkedSection
    {
        typedef ::std::multimap<Rectangle,::std::pair<SdrObject*,OSectionView*>,RectangleLess>              TRectangleMap;
    public:
        typedef ::std::pair< ::boost::shared_ptr<OEndMarker>,::boost::shared_ptr<Splitter> >                TSplitterPair;
        typedef ::std::pair< ::boost::shared_ptr<OReportSection> , ::rtl::Reference< comphelper::OPropertyChangeMultiplexer> >
                                                                                                            TReportPair;
        typedef ::std::pair< TReportPair, TSplitterPair  >                                                  TSectionPair;
        typedef ::std::vector< TSectionPair >                                                               TSectionsMap;

        struct TReportPairHelper : public ::std::unary_function< TSectionsMap::value_type, ::boost::shared_ptr<OReportSection> >
        {
            const ::boost::shared_ptr<OReportSection>& operator() (const TSectionsMap::value_type& lhs) const
            {
                return lhs.first.first;
            }
        };
    private:
        TSectionsMap                            m_aSections;
        svtools::ColorConfig                    m_aColorConfig;
        OReportWindow*                          m_pParent;
        ::rtl::OUString                         m_sShapeType;
        Point                                   m_aPoint;
        sal_Bool                                m_bInSplitHandler;
        sal_Bool                                m_bInUnmark;

        void ImplInitSettings();
        /** returns the iterator at pos _nPos or the end()
        */
        TSectionsMap::iterator getIteratorAtPos(USHORT _nPos);
        void collectRectangles(TRectangleMap& _rMap,bool _bBoundRects);
        void collectBoundResizeRect(const TRectangleMap& _rSortRectangles,sal_Int32 _nControlModification,bool _bAlignAtSection,bool _bBoundRects,Rectangle& _rBound,Rectangle& _rResize);

        DECL_LINK(StartSplitHdl, Splitter*);
        DECL_LINK(SplitHdl, Splitter*);
        DECL_LINK(EndSplitHdl, Splitter*);

        OViewsWindow(OViewsWindow&);
        void operator =(OViewsWindow&);
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        // windows overload
        virtual void MouseButtonDown( const MouseEvent& rMEvt );
        virtual void MouseButtonUp( const MouseEvent& rMEvt );
        using Window::Notify;
        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint);
        // OPropertyChangeListener
        virtual void    _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
    public:
        OViewsWindow( Window* _pParent,OReportWindow* _pReportWindow);
        virtual ~OViewsWindow();

        // windows overload
        virtual void Resize();

        /** late ctor
        */
        void initialize();

        inline OReportWindow*       getView()           const { return m_pParent; }

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

        USHORT          getSectionCount() const;
        /** return the section at the given position
        *
        * \param _nPos
        * \return the section at this pos or an empty section
        */
        ::boost::shared_ptr<OReportSection> getSection(const USHORT _nPos) const;

        void            showView(USHORT _nPos,BOOL _bShow);

        /** turns the grid on or off
        *
        * \param _bVisible
        */
        void            toggleGrid(sal_Bool _bVisible);
        void            setGridSnap(BOOL bOn);
        void            setDragStripes(BOOL bOn);
        BOOL            isDragStripes() const;

        /** returns the total accumulated height of all sections until _pSection is reached
            @param  _pSection Defines the end of accumulation, can be <NULL/>
        */
        sal_Int32       getTotalHeight(const OReportSection* _pSection = NULL) const;

        /** returns the height of the first spliiter.
        */
        sal_Int32       getSplitterHeight() const;

        inline bool     empty() const { return m_aSections.empty(); }
        void            SetMode( DlgEdMode m_eMode );
        void            SetInsertObj( USHORT eObj,const ::rtl::OUString& _sShapeType = ::rtl::OUString());
        rtl::OUString   GetInsertObjString() const;
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

        void            SectionHasFocus(OReportSection* _pSection,BOOL _bHasFocus);

        /** unmark all objects on the views without the given one.
        *
        * @param _pSectionView The view where the objects should not be unmarked.
        */
        void            unmarkAllObjects(OSectionView* _pSectionView);

        /** returns the report section window for the given xsection
            @param  _xSection   the section
        */
        ::boost::shared_ptr<OReportSection> getReportSection(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);

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

        /** get section from point inside.
            @param  _aPosition
        */
        ::boost::shared_ptr<OReportSection> getSection(const Point& _aPosition);

        // IMarkedSection
        ::boost::shared_ptr<OReportSection> getMarkedSection(NearSectionAccess nsa = CURRENT) const;
        virtual void markSection(const sal_uInt16 _nPos);

        /** align all marked objects in all sections
        */
        void alignMarkedObjects(sal_Int32 _nControlModification,bool _bAlignAtSection, bool bBoundRects = false);

        /** shows the properties of the section corresponding to the end marker
        *
        * \param _pEndMarker must be not <NULL/>
        */
        void showProperties(const OEndMarker* _pEndMarker);

        /** creates a default object
        *
        */
        void createDefault();

        /** returns the currently set shape type.
        *
        * \return \member m_sShapeType
        */
        inline ::rtl::OUString getShapeType() const { return m_sShapeType; }

        /** returns the current position in the list
        */
        USHORT getPosition(const OReportSection* _pSection = NULL) const;

        /** calls on every section BrkAction
        *
        */
        void BrkAction();
        void BegMarkObj(const Point& _aPnt,const OSectionView* _pSection);
    private:
        void BegDragObj_createInvisibleObjectAtPosition(const Rectangle& _aRect, const OSectionView* _pSection);
        void EndDragObj_removeInvisibleObjects();
        Point m_aDragDelta;
        ::std::vector<SdrObject*> m_aBegDragTempList;
        bool isObjectInMyTempList(SdrObject *);
    public:
        void BegDragObj(const Point& _aPnt, SdrHdl* _pHdl,const OSectionView* _pSection);
        void EndDragObj(BOOL _bDragIntoNewSection,const OSectionView* _pSection,const Point& _aPnt);

        void EndAction();
        void ForceMarkedToAnotherPage();
        BOOL IsAction() const;
        BOOL IsDragObj() const;
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
        // void MovAction2(const Point& rPnt,const OSectionView* _pSection);
        void setPoint(const Point& _aPnt);
        inline Point getPoint() const { return m_aPoint; }

        sal_uInt32 getMarkedObjectCount() const;
    };
//==============================================================================
} // rptui
//==============================================================================
#endif // RPTUI_VIEWSWINDOW_HXX

