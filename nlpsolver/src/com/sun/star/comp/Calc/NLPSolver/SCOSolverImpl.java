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

package com.sun.star.comp.Calc.NLPSolver;

import com.sun.star.comp.Calc.NLPSolver.dialogs.IEvolutionarySolverStatusDialog;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.registry.XRegistryKey;
import net.adaptivebox.sco.SCAgent;
import net.adaptivebox.global.IUpdateCycleEngine;
import net.adaptivebox.knowledge.Library;
import net.adaptivebox.knowledge.SearchPoint;


public final class SCOSolverImpl extends BaseEvolutionarySolver
   implements com.sun.star.lang.XServiceInfo
{
    private static final String m_implementationName = SCOSolverImpl.class.getName();
    private static final String[] m_serviceNames = {
        "com.sun.star.sheet.Solver",
        "com.sun.star.beans.PropertySet"
    };

    public SCOSolverImpl( XComponentContext context )
    {
        super(context, "SCO Evolutionary Algorithm");

        registerProperty(m_librarySize); //SCO allows the user to specify the size of the library
    }

    public static XSingleComponentFactory __getComponentFactory( String sImplementationName ) {
        XSingleComponentFactory xFactory = null;

        if ( sImplementationName.equals( m_implementationName ) )
            xFactory = Factory.createComponentFactory(SCOSolverImpl.class, m_serviceNames);
        return xFactory;
    }

    public static boolean __writeRegistryServiceInfo( XRegistryKey xRegistryKey ) {
        return Factory.writeRegistryServiceInfo(m_implementationName,
                                                m_serviceNames,
                                                xRegistryKey);
    }

    // com.sun.star.lang.XServiceInfo:
    public String getImplementationName() {
         return m_implementationName;
    }

    public boolean supportsService( String sService ) {
        int len = m_serviceNames.length;

        for( int i=0; i < len; i++) {
            if (sService.equals(m_serviceNames[i]))
                return true;
        }
        return false;
    }

    public String[] getSupportedServiceNames() {
        return m_serviceNames;
    }

    // com.sun.star.sheet.XSolver:

    private SCAgent[] m_agents;

    public void solve() {
        initializeSolve();

        //Init:
        int swarmSize = m_swarmSize.getValue();
        m_agents = new SCAgent[swarmSize];
        for (int i = 0; i < swarmSize; i++) {
            m_agents[i] = new SCAgent();
            m_agents[i].setProblemEncoder(m_problemEncoder);
            m_agents[i].setSpecComparator(m_specCompareEngine);
            m_agents[i].setExternalLib(m_library);
        }

        //Learn:
        m_solverStatusDialog.setVisible(true);
        int learningCycles = m_learningCycles.getValue();
        m_solverStatusDialog.setMaxIterations(learningCycles);
        m_solverStatusDialog.setMaxStagnation(m_required.getValue());
        int learningCycle = 1;
        long runtime = 0;
        do {
            long startTime = System.nanoTime();

            if (learningCycle >= m_learningCycles.getValue())
                learningCycle = 1;

            if (m_solverStatusDialog.getUserState() == IEvolutionarySolverStatusDialog.CONTINUE)
                lockDocument();

            m_toleratedCount = 0;
            m_toleratedMin = -1.0 * m_tolerance.getValue();
            m_toleratedMax = m_tolerance.getValue();
            for (; learningCycle <= learningCycles &&
                    m_toleratedCount < m_required.getValue() &&
                    m_solverStatusDialog.getUserState() != IEvolutionarySolverStatusDialog.CANCEL; learningCycle++) {
                for (int i = 0; i < swarmSize; i++) {
                    SearchPoint point = m_agents[i].generatePoint();
                    boolean inRange = (point.getObjectiveValue() >= m_toleratedMin && point.getObjectiveValue() <= m_toleratedMax);
                    if (Library.replace(m_envCompareEngine, point, m_totalBestPoint)) {
                        m_solverStatusDialog.setBestSolution(m_totalBestPoint.getObjectiveValue(), m_totalBestPoint.isFeasible());
                        if (!inRange) {
                            m_toleratedMin = point.getObjectiveValue() - m_tolerance.getValue();
                            m_toleratedMax = point.getObjectiveValue() + m_tolerance.getValue();
                            m_toleratedCount = 0;
                        }
                    }
                }

                for (int i = 0; i < swarmSize; i++)
                    m_agents[i].updateInfo();

                if (m_specCompareEngine instanceof IUpdateCycleEngine)
                    ((IUpdateCycleEngine)m_specCompareEngine).updateCycle(learningCycle);

                m_solverStatusDialog.setIteration(learningCycle);
                m_solverStatusDialog.setStagnation(m_toleratedCount);
                m_solverStatusDialog.setRuntime(runtime + (System.nanoTime() - startTime));
                m_xReschedule.reschedule();
            }

            applySolution(); //show the current solution
            unlockDocument(); //allow the solution to be displayed

            runtime += (System.nanoTime() - startTime);
            m_solverStatusDialog.setRuntime(runtime);
        } while (m_solverStatusDialog.waitForUser() == IEvolutionarySolverStatusDialog.CONTINUE);

        lockDocument();

        finalizeSolve();
    }

}
