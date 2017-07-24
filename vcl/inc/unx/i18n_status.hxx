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

#ifndef INCLUDED_VCL_INC_UNX_I18N_STATUS_HXX
#define INCLUDED_VCL_INC_UNX_I18N_STATUS_HXX

#include <rtl/ustring.hxx>
#include <salimestatus.hxx>
#include <vcl/vclptr.hxx>

#include <vector>

class SalFrame;
class WorkWindow;
class ListBox;
class FixedText;
class PushButton;
class SalI18N_InputContext;

namespace vcl
{

class StatusWindow;

class X11ImeStatus : public SalI18NImeStatus
{
public:
    X11ImeStatus() {}
    virtual ~X11ImeStatus() override;

    virtual bool canToggle() override;
    virtual void toggle() override;
};

class I18NStatus
{
private:
    SalFrame*                       m_pParent;
    VclPtr<StatusWindow>            m_pStatusWindow;
    OUString                        m_aCurrentIM;

    I18NStatus();
    ~I18NStatus();

    static bool getStatusWindowMode();

public:
    static I18NStatus& get();
    static bool exists();
    static void free();

    void setParent( SalFrame* pParent );
    SalFrame* getParent() const { return  m_pParent; }
    SalFrame* getStatusFrame() const;

    void setStatusText( const OUString& rText );

    enum ShowReason { focus, presentation, contextmap };
    void show( bool bShow, ShowReason eReason );

    // External Control:

    /** Toggle the status window on or off.

        This only works if canToggleStatusWindow returns true (otherwise, any
        calls of this method are ignored).
     */
    void toggleStatusWindow();
};

} // namespace

#endif // INCLUDED_VCL_INC_UNX_I18N_STATUS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
