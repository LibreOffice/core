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
    public final static int WAITING = 0;
    public final static int OK = 1;
    public final static int CONTINUE = 2;
    public final static int CANCEL = 3;

    public int getUserState();

    public void setBestSolution(double solution, boolean feasible);
    public void setMaxIterations(int maxIterations);
    public void setMaxStagnation(int maxStagnation);
    public void setIteration(int iteration);
    public void setStagnation(int stagnation);
    public void setRuntime(long runtime);
    public int waitForUser();

    public void setVisible(boolean visible);
    public void dispose();
}
