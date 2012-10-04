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

#ifndef _SVX_THESDLG_IMPL_HXX
#define _SVX_THESDLG_IMPL_HXX

#include "thesdlg.hxx"

#include <svtools/ehdl.hxx>
#include <svx/checklbx.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/image.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/wrkwin.hxx>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>

#include <stack>
#include <map>
#include <algorithm>

using namespace ::com::sun::star;
using ::rtl::OUString;

class SvLBoxEntry;
class ThesaurusAlternativesCtrl;

class AlternativesString : public SvLBoxString
{
    ThesaurusAlternativesCtrl&    m_rControlImpl;
public:

    AlternativesString( ThesaurusAlternativesCtrl &rControl,
        SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& rStr );

    virtual void Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
