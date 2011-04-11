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
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#define DBAUI_QUERYVIEW_TEXT_HXX

#include "queryview.hxx"
#include "querycontainerwindow.hxx"

class Splitter;
namespace dbaui
{
    class OSqlEdit;
    class OQueryTextView : public Window
    {
        friend class OQueryViewSwitch;
        OSqlEdit*   m_pEdit;
    public:
        OQueryTextView( OQueryContainerWindow* pParent );
        virtual ~OQueryTextView();

        virtual void GetFocus();

        virtual sal_Bool isCutAllowed();
        virtual sal_Bool isPasteAllowed();
        virtual sal_Bool isCopyAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // clears the whole query
        virtual void clear();
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly);
        // check if the statement is correct when not returning false
        virtual sal_Bool checkStatement();
        // set the statement for representation
        virtual void setStatement(const ::rtl::OUString& _rsStatement);
        virtual ::rtl::OUString getStatement();
        // allow access to our edit
        OSqlEdit* getSqlEdit() const { return m_pEdit; }

        OQueryContainerWindow*  getContainerWindow() { return static_cast< OQueryContainerWindow* >( GetParent() ); }
    protected:
        virtual void Resize();
    };
}
#endif // DBAUI_QUERYVIEW_TEXT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
