/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layoutmanager.hxx,v $
 * $Revision: 1.34 $
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

#ifndef __FRAMEWORK_LAYOUTMANAGER_PANEL_HXX_
#define __FRAMEWORK_LAYOUTMANAGER_PANEL_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <macros/generic.hxx>
#include <stdtypes.h>
#include <properties.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/window.hxx>
#include <vcl/splitwin.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework
{

enum PanelPosition
{
    PANEL_TOP,
    PANEL_LEFT,
    PANEL_RIGHT,
    PANEL_BOTTOM,
    PANEL_COUNT
};

class Panel
{
    public:
        Panel( const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMGR,
               const css::uno::Reference< css::awt::XWindow >& rParent,
               PanelPosition nPanel );
        virtual ~Panel();

    private:
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
        css::uno::Reference< css::awt::XWindow >               m_xPanelWindow;
        PanelPosition                                          m_nPanelPosition;
};

}

#endif // __FRAMEWORK_LAYOUTMANAGER_PANEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
