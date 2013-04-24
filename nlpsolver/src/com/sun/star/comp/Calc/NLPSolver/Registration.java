/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
