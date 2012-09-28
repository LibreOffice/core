/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2011 Novell, Inc. (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
package com.sun.star.comp.Calc.NLPSolver;

import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;

public final class Registration {

    private static final String[] m_serviceNames = {
        "com.sun.star.beans.PropertySet",
        "com.sun.star.sheet.Solver"
    };

    public static XSingleComponentFactory __getComponentFactory( String sImplementationName )
    {
        XSingleComponentFactory xFactory = null;

	    System.out.println("Get component '" + sImplementationName + "'");

        if ( sImplementationName.equals( "com.sun.star.comp.Calc.NLPSolver.DEPSSolverImpl" ) )
            xFactory = Factory.createComponentFactory( com.sun.star.comp.Calc.NLPSolver.DEPSSolverImpl.class,
                                                       m_serviceNames );
        if ( sImplementationName.equals( "com.sun.star.comp.Calc.NLPSolver.SCOSolverImpl" ) )
            xFactory = Factory.createComponentFactory( com.sun.star.comp.Calc.NLPSolver.SCOSolverImpl.class,
                                                       m_serviceNames );

	    System.out.println("Return factory " + xFactory);

        return xFactory;
    }
    private Registration() {}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
