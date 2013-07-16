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
#ifndef _SVX_RADIOBTNBOX_HXX
#define _SVX_RADIOBTNBOX_HXX

#include <svtools/simptabl.hxx>

namespace svx {

// class SvxRadioButtonListBox -------------------------------------------

class SvxRadioButtonListBox : public SvSimpleTable
{
    using SvSimpleTable::SetTabs;

private:
    Point               m_aCurMousePoint;

protected:
    virtual void        SetTabs();
    virtual void        MouseButtonUp( const MouseEvent& _rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );

public:
    SvxRadioButtonListBox(SvSimpleTableContainer& rParent, WinBits nBits);
    ~SvxRadioButtonListBox();

    void                HandleEntryChecked( SvTreeListEntry* _pEntry );

    const Point&        GetCurMousePoint() const;

    long                GetControlColumnWidth() const;
};

} // end of namespace ::svx

#endif // #ifndef _SVX_RADIOBTNBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
