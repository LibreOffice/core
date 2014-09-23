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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYTEXTVIEW_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYTEXTVIEW_HXX

#include "queryview.hxx"
#include "querycontainerwindow.hxx"

namespace dbaui
{
    class OSqlEdit;
    class OQueryTextView : public vcl::Window
    {
        friend class OQueryViewSwitch;
        OSqlEdit*   m_pEdit;
    public:
        OQueryTextView( OQueryContainerWindow* pParent );
        virtual ~OQueryTextView();

        virtual void GetFocus() SAL_OVERRIDE;

        virtual bool isCutAllowed();
        virtual bool isPasteAllowed();
        virtual bool isCopyAllowed();
        virtual void copy();
        virtual void cut();
        virtual void paste();
        // clears the whole query
        virtual void clear();
        // set the view readonly or not
        virtual void setReadOnly(bool _bReadOnly);
        // check if the statement is correct when not returning false
        virtual bool checkStatement();
        // set the statement for representation
        virtual void setStatement(const OUString& _rsStatement);
        virtual OUString getStatement();
        // allow access to our edit
        OSqlEdit* getSqlEdit() const { return m_pEdit; }

        OQueryContainerWindow*  getContainerWindow() { return static_cast< OQueryContainerWindow* >( GetParent() ); }
    protected:
        virtual void Resize() SAL_OVERRIDE;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYTEXTVIEW_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
