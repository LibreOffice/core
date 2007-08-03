#ifndef REPORT_REPORTSECTION_HXX
#define REPORT_REPORTSECTION_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportSection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:44:40 $
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

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _REPORT_RPTUIPAGE_HXX
#include "RptPage.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include "cppuhelper/basemutex.hxx"
#endif
#include "ReportDefines.hxx"
#ifndef _REPORT_RPTUIFUNC_HXX
#include "dlgedfunc.hxx"
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#include <boost/shared_ptr.hpp>
#include <memory>

namespace rptui
{
    class OReportModel;
    class OReportPage;
    class OSectionView;
    class OViewsWindow;

    class OReportSection :   public Window
                        ,   public ::cppu::BaseMutex
                        ,   public ::comphelper::OPropertyChangeListener
                        ,   public DropTargetHelper
    {
        OReportPage*                        m_pPage;
        OSectionView*                       m_pView;
        OViewsWindow*                       m_pParent;
        ::std::auto_ptr<DlgEdFunc>          m_pFunc;
        ::boost::shared_ptr<OReportModel>   m_pModel;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                           m_pMulti;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                           m_pReportListener;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >              m_xSection;

        DlgEdMode                   m_eMode;
        BOOL                        m_bDialogModelChanged;
        sal_Bool                    m_bInDrag;

        /** fills the section with all control from the report section
        */
        void fill();

        OReportSection(OReportSection&);
        void operator =(OReportSection&);
    protected:
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );

        // window overrides
        virtual void        Paint( const Rectangle& rRect );
        virtual void        MouseMove( const MouseEvent& rMEvt );
        virtual void        Command( const CommandEvent& rCEvt );
        virtual void        LoseFocus();
        virtual void        GetFocus();
        virtual void        Resize();

        // OPropertyChangeListener
        virtual void    _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
    public:
        OReportSection(OViewsWindow* _pParent,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
        virtual ~OReportSection();

        // window overrides
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        MouseButtonUp( const MouseEvent& rMEvt );

        /** copies the current selection in this section
            @param  _rAllreadyCopiedObjects This is an out/in put param which contains all already copied objects.
        */
        void Copy(::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rAllreadyCopiedObjects);

        void Copy(::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rAllreadyCopiedObjects,bool _bEraseAnddNoClone);

        /** paste a new control in this section
            @param  _aAllreadyCopiedObjects objects to paste into the section. Only objects are pasted where the name is equal to the section name.
            @param  _bForce If set to <TRUE/> than the objects will be copied into this section. The name is not compared in this case.
        */
        void Paste(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _aAllreadyCopiedObjects,bool _bForce = false);

        /** Deletes the current selection in this section
        *
        */
        void Delete();

        /** All objects will be marked.
        */
        void SelectAll();

        /** makes the grid visible
        *
        * \param _bVisible when <TRUE/> the grid is made visible
        */
        void SetGridVisible(BOOL _bVisible);

        /** adjusat the size of the current page
        *
        * \return <TRUE/> when the page was adjusted
        */
        bool adjustPageSize();

        inline OViewsWindow*        getViewsWindow() const { return m_pParent; }
        inline OSectionView*        getView() const { return m_pView; }
        inline OReportPage*         getPage() const { return m_pPage; }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getSection() const { return m_xSection; }

        BOOL            UnmarkDialog();
        BOOL            RemarkDialog();

        void            SetDialogModelChanged( BOOL bChanged = TRUE ) { m_bDialogModelChanged = bChanged; }
        BOOL            IsDialogModelChanged() const { return m_bDialogModelChanged; }
        DlgEdMode       GetMode() const { return m_eMode; }
        void            SetMode( DlgEdMode m_eMode );

        /** sets the height of the section
            @param  _nHeight    Height is in pixel
        */
        void            setSectionHeightPixel(sal_uInt32 _nHeight);

        /** insert a new SdrObject which belongs to the report component.
            @param  _xObject    the report component
        */
        void            insertObject(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xObject);

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        sal_Bool        handleKeyEvent(const KeyEvent& _rEvent);

        /** returns the current control report model or <NULL/>
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent > getCurrentControlModel() const;

        /** fills the vector with all selected control models
            /param  _rSelection The vector will be filled and will not be cleared before.
        */
        void fillControlModelSelection(::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent > >& _rSelection) const;

        /** creates a default object (custom shape)
        *
        * @param _sType
        */
        void createDefault(const ::rtl::OUString& _sType);

        /** creates a new default custom shape
        *
        * \param _sType
        * \param _pObj
        */
        void createDefault(const ::rtl::OUString& _sType,SdrObject* _pObj);
        void stopScrollTimer();
    };
//==================================================================
}   //rptui
//==================================================================
#endif // REPORT_REPORTSECTION_HXX

