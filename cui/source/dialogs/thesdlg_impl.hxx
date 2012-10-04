/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
