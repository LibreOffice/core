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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_JOINDESIGNVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_JOINDESIGNVIEW_HXX

#include <dbaccess/dataview.hxx>
#include "QEnumTypes.hxx"

namespace dbaui
{
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
        OJoinDesignView(vcl::Window* pParent,
                        OJoinController& _rController,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
        virtual ~OJoinDesignView();

        // set the view readonly or not
        virtual void setReadOnly(bool _bReadOnly);
        // set the statement for representation
        /// late construction
        virtual void Construct() SAL_OVERRIDE;
        virtual void initialize() SAL_OVERRIDE;
        virtual void KeyInput( const KeyEvent& rEvt ) SAL_OVERRIDE;

        virtual void SaveTabWinUIConfig(OTableWindow* pWin);
        OJoinController& getController() const { return m_rController; }
        // called when fields are deleted

        OJoinTableView* getTableView() const { return m_pTableView; }
        OScrollWindowHelper* getScrollHelper() const { return m_pScrollWindow; }
    protected:
        // return the Rectangle where I can paint myself
        virtual void resizeDocumentView(Rectangle& rRect) SAL_OVERRIDE;
        DECL_LINK( SplitHdl, void* );
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_JOINDESIGNVIEW_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
