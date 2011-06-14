/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef DBAUI_JOINDESIGNVIEW_HXX
#define DBAUI_JOINDESIGNVIEW_HXX

#include "dataview.hxx"
#ifndef _VECTOR_
#include <vector>
#endif
#include <tools/string.hxx>
#include "QEnumTypes.hxx"

#include <memory>

namespace dbaui
{
    class OTableConnection;
    class OConnectionLineData;
    class OJoinController;
    class OScrollWindowHelper;
    class OJoinTableView;
    class OTableWindow;

    class OJoinDesignView : public ODataView
    {
    protected:
        OScrollWindowHelper*    m_pScrollWindow;    // contains only the scrollbars
        OJoinTableView*         m_pTableView;       // presents the upper window
        OJoinController&        m_rController;

    public:
        OJoinDesignView(Window* pParent,
                        OJoinController& _rController,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~OJoinDesignView();

        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // set the statement for representation
        /// late construction
        virtual void Construct();
        virtual void initialize();
        virtual void KeyInput( const KeyEvent& rEvt );

        virtual void SaveTabWinUIConfig(OTableWindow* pWin);
        OJoinController& getController() const { return m_rController; }
        // called when fields are deleted

        OJoinTableView* getTableView() const { return m_pTableView; }
        OScrollWindowHelper* getScrollHelper() const { return m_pScrollWindow; }
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect);
        DECL_LINK( SplitHdl, void* );
    };
}
#endif // DBAUI_JOINDESIGNVIEW_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
