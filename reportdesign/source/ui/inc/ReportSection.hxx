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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTSECTION_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTSECTION_HXX

#include <vcl/window.hxx>
#include "RptPage.hxx"
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/propmultiplex.hxx>
#include <cppuhelper/basemutex.hxx>
#include "ReportDefines.hxx"
#include "dlgedfunc.hxx"
#include <svtools/transfer.hxx>
#include <rtl/ref.hxx>
#include <memory>

namespace rptui
{
    class OReportModel;
    class OReportPage;
    class OSectionView;
    class OSectionWindow;

    class OReportSection :   public vcl::Window
                        ,   public ::cppu::BaseMutex
                        ,   public ::comphelper::OPropertyChangeListener
                        ,   public DropTargetHelper
    {
        OReportPage*                        m_pPage;
        OSectionView*                       m_pView;
        VclPtr<OSectionWindow>              m_pParent;
        ::std::unique_ptr<DlgEdFunc>        m_pFunc;
        std::shared_ptr<OReportModel>       m_pModel;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                           m_pMulti;
        ::rtl::Reference< comphelper::OPropertyChangeMultiplexer>                           m_pReportListener;
        css::uno::Reference< css::report::XSection >              m_xSection;
        sal_Int32                   m_nPaintEntranceCount;

        DlgEdMode                   m_eMode;

        /** fills the section with all control from the report section
        */
        void fill();
        /** checks all objects if they fit in the new paper width.
        */
        void impl_adjustObjectSizePosition(sal_Int32 i_nPaperWidth,sal_Int32 i_nLeftMargin,sal_Int32 i_nRightMargin);

        OReportSection(OReportSection&) = delete;
        void operator =(OReportSection&) = delete;
    protected:
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt ) override;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt ) override;

        // window overrides
        virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
        virtual void        MouseMove( const MouseEvent& rMEvt ) override;
        virtual void        Command( const CommandEvent& rCEvt ) override;
        virtual void        Resize() override;

        // OPropertyChangeListener
        virtual void    _propertyChanged(const css::beans::PropertyChangeEvent& _rEvent) throw( css::uno::RuntimeException) override;
    public:
        OReportSection(OSectionWindow* _pParent,const css::uno::Reference< css::report::XSection >& _xSection);
        virtual ~OReportSection();
        virtual void dispose() override;

        // window overrides
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;

        /** copies the current selection in this section
            @param  _rAllreadyCopiedObjects This is an out/in put param which contains all already copied objects.
        */
        void Copy(css::uno::Sequence< css::beans::NamedValue >& _rAllreadyCopiedObjects);

        void Copy(css::uno::Sequence< css::beans::NamedValue >& _rAllreadyCopiedObjects,bool _bEraseAnddNoClone);

        /** paste a new control in this section
            @param  _aAllreadyCopiedObjects objects to paste into the section. Only objects are pasted where the name is equal to the section name.
            @param  _bForce If set to <TRUE/> than the objects will be copied into this section. The name is not compared in this case.
        */
        void Paste(const css::uno::Sequence< css::beans::NamedValue >& _aAllreadyCopiedObjects,bool _bForce = false);

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
        void SetGridVisible(bool _bVisible);

        inline OSectionWindow*      getSectionWindow() const { return m_pParent; }
        inline OSectionView&        getSectionView() const { return *m_pView; }
        inline OReportPage*         getPage() const { return m_pPage; }
        inline css::uno::Reference< css::report::XSection > getSection() const { return m_xSection; }

        DlgEdMode       GetMode() const { return m_eMode; }
        void            SetMode( DlgEdMode m_eMode );

        /** checks if the keycode is known by the child windows
            @param  _rCode  the keycode
            @return <TRUE/> if the keycode is handled otherwise <FALSE/>
        */
        bool            handleKeyEvent(const KeyEvent& _rEvent);

        /** returns the current control report model or <NULL/>
        */
        css::uno::Reference< css::report::XReportComponent > getCurrentControlModel() const;

        /** fills the vector with all selected control models
            /param  _rSelection The vector will be filled and will not be cleared before.
        */
        void fillControlModelSelection(::std::vector< css::uno::Reference< css::uno::XInterface > >& _rSelection) const;

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
        static void createDefault(const OUString& _sType,SdrObject* _pObj);
        void stopScrollTimer();

        /** deactivate the current active ole object if any
        */
        void    deactivateOle();

        /** returns true when an ole object is currently active
        */
        bool isUiActive() const;
    };

}   //rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_REPORTSECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
