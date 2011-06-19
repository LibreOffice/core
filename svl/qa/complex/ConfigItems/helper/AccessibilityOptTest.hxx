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

#ifndef  SVTOOLS_ACCESSIBILITYOPTTEST_HXX
#define  SVTOOLS_ACCESSIBILITYOPTTEST_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <svl/accessibilityoptions.hxx>

namespace css = ::com::sun::star;

class AccessibilityOptTest
{
public:
    AccessibilityOptTest();
    ~AccessibilityOptTest();

    void impl_checkAccessibilityOptions();

private:
    void impl_checkGetAutoDetectSystemHC();
    void impl_checkGetIsForPagePreviews();
    void impl_checkGetIsHelpTipsDisappear();
    void impl_checkGetIsAllowAnimatedGraphics();
    void impl_checkGetIsAllowAnimatedText();
    void impl_checkGetIsAutomaticFontColor();
    void impl_checkGetIsSystemFont();
    void impl_checkGetHelpTipSeconds();
    void impl_checkIsSelectionInReadonly();

    void impl_checkSetAutoDetectSystemHC();
    void impl_checkSetIsForPagePreviews();
    void impl_checkSetIsHelpTipsDisappear();
    void impl_checkSetIsAllowAnimatedGraphics();
    void impl_checkSetIsAllowAnimatedText();
    void impl_checkSetIsAutomaticFontColor();
    void impl_checkSetIsSystemFont();
    void impl_checkSetHelpTipSeconds();
    void impl_checkSetSelectionInReadonly();

private:
    css::uno::Reference< css::container::XNameAccess > m_xCfg;
    SvtAccessibilityOptions aAccessibilityOpt;
};

#endif // #ifndef  SVTOOLS_ACCESSIBILITYOPTTEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
