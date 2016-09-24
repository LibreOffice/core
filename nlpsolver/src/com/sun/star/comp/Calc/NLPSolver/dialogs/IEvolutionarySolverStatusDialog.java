/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2009 by Sun Microsystems, Inc.
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

package com.sun.star.comp.Calc.NLPSolver.dialogs;

public interface IEvolutionarySolverStatusDialog {
    int WAITING = 0;
    int OK = 1;
    int CONTINUE = 2;
    int CANCEL = 3;

    int getUserState();

    void setBestSolution(double solution, boolean feasible);
    void setMaxIterations(int maxIterations);
    void setMaxStagnation(int maxStagnation);
    void setIteration(int iteration);
    void setStagnation(int stagnation);
    void setRuntime(long runtime);
    int waitForUser();

    void setVisible(boolean visible);
    void dispose();
}
