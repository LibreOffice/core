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

#ifndef _SBX_BRWVIEW_HXX
#define _SBX_BRWVIEW_HXX

#include <vcl/window.hxx>

#include <tools/resid.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <dbaccess/dataview.hxx>
#include <unotools/eventlisteneradapter.hxx>

namespace com { namespace sun { namespace star { namespace awt {
    class XControl;
    class XControlContainer;
    class XControlModel;
}}}}

class Splitter;

namespace dbaui
{
    class DBTreeView;
    class SbaGridControl;

    class UnoDataBrowserView : public ODataView, public ::utl::OEventListenerAdapter
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >                 m_xGrid;            // our grid's UNO representation
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >        m_xMe;              // our own UNO representation
        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        mutable SbaGridControl* m_pVclControl;  // our grid's VCL representation
        Window*                 m_pStatus;

        DECL_LINK( SplitHdl, void* );
    // attribute access
    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >             getGridControl() const  { return m_xGrid; }
        SbaGridControl*         getVclControl() const;

    public:
        UnoDataBrowserView( Window* pParent,
                            IController& _rController,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
        virtual ~UnoDataBrowserView();

        /// late construction
        virtual void Construct(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel);

        /** as columns may be hidden there is a difference between a columns model pos and its view pos
            so we you may use these translation function
        */
        sal_uInt16 View2ModelPos(sal_uInt16 nPos) const;
        /// for the same reason the view column count isn't the same as the model column count

        void setSplitter(Splitter* _pSplitter);
        void setTreeView(DBTreeView* _pTreeView);

        void    showStatus( const OUString& _rStatus );
        void    hideStatus();

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >     getContainer() { return m_xMe; }

    protected:
        virtual long PreNotify( NotifyEvent& rNEvt );
        virtual void GetFocus();
        virtual void resizeDocumentView(Rectangle& rRect);
        virtual void _disposing( const ::com::sun::star::lang::EventObject& _rSource );

    private:
        using ODataView::Construct;
    };

    class BrowserViewStatusDisplay
    {
    protected:
        UnoDataBrowserView*     m_pView;

    public:
        BrowserViewStatusDisplay( UnoDataBrowserView* _pView, const OUString& _rStatus );
        ~BrowserViewStatusDisplay( );
    };
}
#endif // _SBX_BRWVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
