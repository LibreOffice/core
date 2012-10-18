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

#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#define DBACCESS_UI_DBTREEVIEW_HXX

#include <vcl/window.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

class SvLBoxTreeList;
namespace dbaui
{
    class DBTreeListBox;
    // ------------------
    // - DBTreeView -
    // ------------------

    class DBTreeView : public Window
    {
    private:
        DBTreeListBox*  m_pTreeListBox;
    protected:
        // window overridables
        virtual void Resize();
    public:

        DBTreeView( Window* pParent,
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                WinBits nBits );
        ~DBTreeView();

        /** sets a handler which is called when an list box entry is to be expanded.
            <p>When calling the link, the parameter is an SvTreeListEntry marking the entry to be expanded.
            </p>
        */
        void    SetPreExpandHandler(const Link& _rHdl);
        /// gets the currently set NodeExpansionHandler

        void    setCopyHandler(const Link& _rHdl);


        void                setModel(SvLBoxTreeList* _pTreeModel);
        void                setSelChangeHdl(const Link& _rHdl);

        DBTreeListBox&      getListBox() const { return *m_pTreeListBox; }

        virtual void GetFocus();
    };
}

#endif // DBACCESS_UI_DBTREEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
