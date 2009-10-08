/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibilityOptTest.hxx,v $
 *
 *  $Revision: 1.1.4.2 $
 *
 *  last change: $Author: as $ $Date: 2008/03/19 11:09:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
