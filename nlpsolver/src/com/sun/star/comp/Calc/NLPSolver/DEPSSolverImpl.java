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
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.registry.XRegistryKey;
import java.util.logging.Level;
import java.util.logging.Logger;
import net.adaptivebox.deps.DEPSAgent;
import net.adaptivebox.deps.behavior.DEGTBehavior;
import net.adaptivebox.deps.behavior.PSGTBehavior;
import net.adaptivebox.global.IUpdateCycleEngine;
import net.adaptivebox.knowledge.ILibEngine;
import net.adaptivebox.knowledge.Library;
import net.adaptivebox.knowledge.SearchPoint;


public final class DEPSSolverImpl extends BaseEvolutionarySolver
   implements com.sun.star.lang.XServiceInfo
{
    private static final String m_implementationName = DEPSSolverImpl.class.getName();
    private static final String[] m_serviceNames = {
        "com.sun.star.sheet.Solver",
        "com.sun.star.beans.PropertySet"
    };

    public DEPSSolverImpl( XComponentContext context )
    {
        super(context, "DEPS Evolutionary Algorithm");

        registerProperty(m_agentSwitchRate);
        registerProperty(m_factor);
        registerProperty(m_CR);
        registerProperty(m_c1);
        registerProperty(m_c2);
        registerProperty(m_weight);
        registerProperty(m_CL);
    }

    public static XSingleComponentFactory __getComponentFactory( String sImplementationName ) {
        XSingleComponentFactory xFactory = null;

        if ( sImplementationName.equals( m_implementationName ) )
            xFactory = Factory.createComponentFactory(DEPSSolverImpl.class, m_serviceNames);
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
    private DEPSAgent[] m_agents;

    private PropertyInfo<Double> m_agentSwitchRate = new PropertyInfo<Double>("AgentSwitchRate", 0.5, "Agent Switch Rate (DE Probability)");
    // --DE
    private PropertyInfo<Double> m_factor = new PropertyInfo<Double>("DEFactor", 0.5, "DE: Scaling Factor (0-1.2)");
    private PropertyInfo<Double> m_CR = new PropertyInfo<Double>("DECR", 0.9, "DE: Crossover Probability (0-1)");
    // --PS
    private PropertyInfo<Double> m_c1 = new PropertyInfo<Double>("PSC1", 1.494, "PS: Cognitive Constant");
    private PropertyInfo<Double> m_c2 = new PropertyInfo<Double>("PSC2", 1.494, "PS: Social Constant");
    private PropertyInfo<Double> m_weight = new PropertyInfo<Double>("PSWeight", 0.729, "PS: Constriction Coefficient");
    private PropertyInfo<Double> m_CL = new PropertyInfo<Double>("PSCL", 0.0, "PS: Mutation Probability (0-0.005)");

    public void solve() {
        try {
            m_librarySize.setValue(m_swarmSize.getValue()); //DEPS' library is as large as the swarm
        } catch (IllegalArgumentException ex) {
            Logger.getLogger(DEPSSolverImpl.class.getName()).log(Level.SEVERE, null, ex);
        }
        initializeSolve();

        //Init:
        m_agents = new DEPSAgent[m_swarmSize.getValue()];
        for (int i = 0; i < m_swarmSize.getValue(); i++) {
            m_agents[i] = new DEPSAgent();
            m_agents[i].setProblemEncoder(m_problemEncoder);
            m_agents[i].setPbest(m_library.getSelectedPoint(i));

            DEGTBehavior deGTBehavior = new DEGTBehavior();
            deGTBehavior.FACTOR = m_factor.getValue();
            deGTBehavior.CR = m_CR.getValue();

            PSGTBehavior psGTBehavior = new PSGTBehavior();
            psGTBehavior.c1 = m_c1.getValue();
            psGTBehavior.c2 = m_c2.getValue();
            psGTBehavior.CL = m_CL.getValue();
            psGTBehavior.weight = m_weight.getValue();

            m_agents[i].switchP = m_agentSwitchRate.getValue();
            m_agents[i].setGTBehavior(deGTBehavior);
            m_agents[i].setGTBehavior(psGTBehavior);

            m_agents[i].setSpecComparator(m_specCompareEngine);
            m_agents[i].setLibrary(m_library);
        }

        //Learn:
        m_solverStatusDialog.setVisible(true);
        m_solverStatusDialog.setMaxIterations(m_learningCycles.getValue());
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
            for (; learningCycle <= m_learningCycles.getValue() &&
                    m_toleratedCount < m_required.getValue() &&
                    m_solverStatusDialog.getUserState() != IEvolutionarySolverStatusDialog.CANCEL; learningCycle++) {
                m_library.refreshGbest(m_specCompareEngine);

                for (int i = 0; i < m_swarmSize.getValue(); i++)
                    m_agents[i].generatePoint();

                for (int i = 0; i < m_swarmSize.getValue(); i++)
                    m_agents[i].learn();

                for (int i = 0; i < m_swarmSize.getValue(); i++) {
                    SearchPoint agentPoint = m_agents[i].getMGState();
                    boolean inRange = (agentPoint.getObjectiveValue() >= m_toleratedMin && agentPoint.getObjectiveValue() <= m_toleratedMax);
                    if (Library.replace(m_envCompareEngine, agentPoint, m_totalBestPoint)) {
                        m_solverStatusDialog.setBestSolution(m_totalBestPoint.getObjectiveValue(), m_totalBestPoint.isFeasible());
                        if (!inRange) {
                            m_toleratedMin = agentPoint.getObjectiveValue() - m_tolerance.getValue();
                            m_toleratedMax = agentPoint.getObjectiveValue() + m_tolerance.getValue();
                            m_toleratedCount = 0;
                        }
                    }
                }

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
