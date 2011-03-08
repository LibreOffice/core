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

#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#define DBACCESS_UI_DBTREEVIEW_HXX

#include <vcl/window.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

class SvTreeListBox;
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
            <p>When calling the link, the parameter is an SvLBoxEntry marking the entry to be expanded.
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
