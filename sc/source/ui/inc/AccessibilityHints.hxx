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

#ifndef SC_ACCESSIBILITYHINTS_HXX
#define SC_ACCESSIBILITYHINTS_HXX

#include "viewdata.hxx"
#include <com/sun/star/uno/XInterface.hpp>
#include <svl/smplhint.hxx>
#include <svl/hint.hxx>

#define SC_HINT_ACC_SIMPLE_START    SFX_HINT_USER00
#define SC_HINT_ACC_TABLECHANGED    SC_HINT_ACC_SIMPLE_START + 1
#define SC_HINT_ACC_CURSORCHANGED   SC_HINT_ACC_SIMPLE_START + 2
#define SC_HINT_ACC_VISAREACHANGED  SC_HINT_ACC_SIMPLE_START + 3
#define SC_HINT_ACC_ENTEREDITMODE   SC_HINT_ACC_SIMPLE_START + 4
#define SC_HINT_ACC_LEAVEEDITMODE   SC_HINT_ACC_SIMPLE_START + 5
#define SC_HINT_ACC_MAKEDRAWLAYER   SC_HINT_ACC_SIMPLE_START + 6
#define SC_HINT_ACC_WINDOWRESIZED   SC_HINT_ACC_SIMPLE_START + 7

class ScAccWinFocusLostHint : public SfxHint
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                xOldAccessible;
public:
                TYPEINFO();
                ScAccWinFocusLostHint(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xOld );
                ~ScAccWinFocusLostHint();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                GetOldAccessible() const { return xOldAccessible; }
};

class ScAccWinFocusGotHint : public SfxHint
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                xNewAccessible;
public:
                TYPEINFO();
                ScAccWinFocusGotHint(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xNew );
                ~ScAccWinFocusGotHint();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                GetNewAccessible() const { return xNewAccessible; }
};

class ScAccGridWinFocusLostHint : public ScAccWinFocusLostHint
{
    ScSplitPos  eOldGridWin;
public:
                TYPEINFO();
                ScAccGridWinFocusLostHint( ScSplitPos eOldGridWin,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xOld );
                ~ScAccGridWinFocusLostHint();

    ScSplitPos  GetOldGridWin() const { return eOldGridWin; }
};

class ScAccGridWinFocusGotHint : public ScAccWinFocusGotHint
{
    ScSplitPos  eNewGridWin;
public:
                TYPEINFO();
                ScAccGridWinFocusGotHint( ScSplitPos eNewGridWin,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xNew );
                ~ScAccGridWinFocusGotHint();

    ScSplitPos  GetNewGridWin() const { return eNewGridWin; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
