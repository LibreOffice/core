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
#ifndef REPORT_REPORTSECTION_HXX
#define REPORT_REPORTSECTION_HXX

#include <vcl/window.hxx>
#include "RptPage.hxx"
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/propmultiplex.hxx>
#include "cppuhelper/basemutex.hxx"
#include "ReportDefines.hxx"
#include "dlgedfunc.hxx"
#include <svtools/transfer.hxx>
#include <rtl/ref.hxx>
#include <boost/shared_ptr.hpp>
#include <memory>

namespace rptui
{
    class OReportModel;
    class OReportPage;
    class OSectionView;
    class OSectionWindow;

    class OReportSection :   public Window
                        ,   public ::cppu::BaseMutex
                        ,   public ::comphelper::OPropertyChangeListener
                        ,   public DropTargetHelper
    {
        OReportPage*                        m_pPage;
        OSectionView*                       m_pView;
        OSectionWindow*                     m_pParent;
        ::std::auto_ptr<DlgEdFunc>          m_pFunc;
        ::boost::shared_ptr<OReportModel>   m_pModel;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                           m_pMulti;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                           m_pReportListener;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >              m_xSection;
        sal_Int32                   m_nPaintEntranceCount;

        DlgEdMode                   m_eMode;
        sal_Bool                        m_bDialogModelChanged;
        sal_Bool                    m_bInDrag;

        /** fills the section with all control from the report section
        */
        void fill();
        /** checks all objects if they fit in the new paper width.
        */
        void impl_adjustObjectSizePosition(sal_Int32 i_nPaperWidth,sal_Int32 i_nLeftMargin,sal_Int32 i_nRightMargin);

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
        virtual void        Resize();

        // OPropertyChangeListener
        virtual void    _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
    public:
        OReportSection(OSectionWindow* _pParent,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
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
        void SelectAll(const sal_uInt16 _nObjectType);

        /** makes the grid visible
        *
        * \param _bVisible when <TRUE/> the grid is made visible
        */
        void SetGridVisible(sal_Bool _bVisible);

        inline OSectionWindow*      getSectionWindow() const { return m_pParent; }
        inline OSectionView&        getSectionView() const { return *m_pView; }
        inline OReportPage*         getPage() const { return m_pPage; }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getSection() const { return m_xSection; }

        void            SetDialogModelChanged( sal_Bool bChanged = sal_True ) { m_bDialogModelChanged = bChanged; }
        sal_Bool            IsDialogModelChanged() const { return m_bDialogModelChanged; }
        DlgEdMode       GetMode() const { return m_eMode; }
        void            SetMode( DlgEdMode m_eMode );

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
        void fillControlModelSelection(::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& _rSelection) const;

        /** creates a default object (custom shape)
        *
        * @param _sType
        */
        void createDefault(const OUString& _sType);

        /** creates a new default custom shape
        *
        * \param _sType
        * \param _pObj
        */
        void createDefault(const OUString& _sType,SdrObject* _pObj);
        void stopScrollTimer();

        /** deactivate the currect active ole object if any
        */
        void    deactivateOle();

        /** returns true when an ole object is currently active
        */
        bool isUiActive() const;
    };

}   //rptui

#endif // REPORT_REPORTSECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
