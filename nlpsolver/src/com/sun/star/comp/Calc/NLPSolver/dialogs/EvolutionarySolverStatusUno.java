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

import com.sun.star.comp.Calc.NLPSolver.BaseNLPSolver;
import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XActionListener;
import com.sun.star.comp.Calc.NLPSolver.ResourceManager;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.style.VerticalAlignment;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.comp.Calc.NLPSolver.dialogs.controls.Button;
import com.sun.star.comp.Calc.NLPSolver.dialogs.controls.Label;
import com.sun.star.comp.Calc.NLPSolver.dialogs.controls.ProgressBar;

public class EvolutionarySolverStatusUno extends BaseDialog
    implements IEvolutionarySolverStatusDialog,
               XActionListener {

    private int userState;

    // <editor-fold defaultstate="collapsed" desc="UNO Controls">
    private Label lblSolution;
    private Label lblSolutionValue;
    private Label lblIteration;
    private ProgressBar pbIteration;
    private Label lblIterationValue;
    private Label lblStagnation;
    private ProgressBar pbStagnation;
    private Label lblStagnationValue;
    private Label lblRuntime;
    private Label lblRuntimeValue;
    private Button btnStop;
    private Button btnOK;
    private Button btnContinue;
    // </editor-fold>
    private int defaultTextColor;
    private int maxIterations;
    private int maxStagnation;

    private final ResourceManager resourceManager;

    private final int COLOR_RED = 0xFF0000;

    public EvolutionarySolverStatusUno(XComponentContext xContext) {
        super(xContext, "Solver Status", -1, -1, 170, 95); //center the dialog on the parent

        setCloseable(false);
        userState = IEvolutionarySolverStatusDialog.OK;

        resourceManager = new ResourceManager(xContext, "com.sun.star.comp.Calc.NLPSolver", "/locale", "NLPSolverStatusDialog");

        try {
            setProperty("Title", resourceManager.getLocalizedString("Dialog.Caption"));
        } catch (com.sun.star.resource.MissingResourceException ex) {} //leave the title as it is

        // <editor-fold defaultstate="collapsed" desc="Create UNO Controls">
        int y = 5;
        lblSolution = new Label(this, "lblSolution");
        lblSolution.setPosition(5, y);
        lblSolution.setSize(60, 10);
        lblSolution.setLabel(resourceManager.getLocalizedString("Controls.lblSolution", "Current Solution:"));
        lblSolution.setParentControl(this);

        lblSolutionValue = new Label(this, "lblSolutionValue");
        lblSolutionValue.setPosition(65, y);
        lblSolutionValue.setSize(100, 10);
        lblSolutionValue.setParentControl(this);
        defaultTextColor = lblSolutionValue.getTextColor();
        y += 15;

        lblIteration = new Label(this, "lblIteration");
        lblIteration.setPosition(5, y);
        lblIteration.setSize(60, 15);
        lblIteration.setLabel(resourceManager.getLocalizedString("Controls.lblIteration", "Iteration:"));
        lblIteration.setVerticalAlign(VerticalAlignment.MIDDLE);
        lblIteration.setParentControl(this);

        pbIteration = new ProgressBar(this, "pbIteration");
        pbIteration.setPosition(65, y);
        pbIteration.setSize(100, 15);
        pbIteration.setParentControl(this);

        lblIterationValue = new Label(this, "lblIterationValue");
        lblIterationValue.setPosition(65, y);
        lblIterationValue.setSize(100, 20);
        lblIterationValue.setVerticalAlign(VerticalAlignment.MIDDLE);
        lblIterationValue.setMultiLine(true);
        lblIterationValue.setParentControl(this);
        lblIterationValue.setVisible(false);
        y += 20;

        lblStagnation = new Label(this, "lblStagnation");
        lblStagnation.setPosition(5, y);
        lblStagnation.setSize(60, 15);
        lblStagnation.setLabel(resourceManager.getLocalizedString("Controls.lblStagnation", "Stagnation:"));
        lblStagnation.setVerticalAlign(VerticalAlignment.MIDDLE);
        lblStagnation.setParentControl(this);

        pbStagnation = new ProgressBar(this, "pbStagnation");
        pbStagnation.setPosition(65, y);
        pbStagnation.setSize(100, 15);
        pbStagnation.setParentControl(this);

        lblStagnationValue = new Label(this, "lblStagnationValue");
        lblStagnationValue.setPosition(65, y);
        lblStagnationValue.setSize(100, 20);
        lblStagnationValue.setVerticalAlign(VerticalAlignment.MIDDLE);
        lblStagnationValue.setMultiLine(true);
        lblStagnationValue.setParentControl(this);
        lblStagnationValue.setVisible(false);
        y+= 20;

        lblRuntime = new Label(this, "lblRuntime");
        lblRuntime.setPosition(5, y);
        lblRuntime.setSize(60, 10);
        lblRuntime.setLabel(resourceManager.getLocalizedString("Controls.lblRuntime", "Runtime:"));
        lblRuntime.setParentControl(this);

        lblRuntimeValue = new Label(this, "lblRuntimeValue");
        lblRuntimeValue.setPosition(65, y);
        lblRuntimeValue.setSize(100, 10);
        lblRuntimeValue.setParentControl(this);
        y += 15;

        btnStop = new Button(this, "btnStop");
        btnStop.setPosition(5, y);
        btnStop.setSize(45, 15);
        btnStop.setLabel(resourceManager.getLocalizedString("Controls.btnStop", "Stop"));
        btnStop.setParentControl(this);
        btnStop.addActionListener(this);
        btnStop.setActionCommand("btnStopClick");

        btnOK = new Button(this, "btnOK");
        btnOK.setPosition(65, y);
        btnOK.setSize(40, 15);
        btnOK.setLabel(resourceManager.getLocalizedString("Controls.btnOK", "OK"));
        btnOK.setParentControl(this);
        btnOK.addActionListener(this);
        btnOK.setActionCommand("btnOKClick");
        btnOK.setEnabled(false);

        btnContinue = new Button(this, "btnContinue");
        btnContinue.setPosition(110, y);
        btnContinue.setSize(55, 15);
        btnContinue.setLabel(resourceManager.getLocalizedString("Controls.btnContinue", "Continue"));
        btnContinue.setParentControl(this);
        btnContinue.addActionListener(this);
        btnContinue.setActionCommand("btnContinueClick");
        btnContinue.setEnabled(false);
        y += 15;
        // </editor-fold>
    }

    public int getUserState() {
        return userState;
    }

    public void setBestSolution(double solution, boolean feasible) {
        lblSolutionValue.setLabel(String.format("%.2f", solution));
        if (feasible)
            lblSolutionValue.setTextColor(defaultTextColor);
        else
            lblSolutionValue.setTextColor(COLOR_RED); //red
    }

    public void setMaxIterations(int maxIterations) {
        pbIteration.setRange(0, maxIterations);
        this.maxIterations = maxIterations;
    }

    public void setMaxStagnation(int maxStagnation) {
        pbStagnation.setRange(0, maxStagnation);
        this.maxStagnation = maxStagnation;
    }

    public void setIteration(int iteration) {
        pbIteration.setValue(iteration);
    }

    public void setStagnation(int stagnation) {
        pbStagnation.setValue(stagnation);
    }

    public void setRuntime(long runtime) {
        lblRuntimeValue.setLabel(BaseNLPSolver.nanoTimeToString(resourceManager, runtime));
    }

    public int waitForUser() {
        btnStop.setEnabled(false);
        btnOK.setEnabled(true);
        btnContinue.setEnabled(true);

        if (pbIteration.getValue() >= maxIterations) {
            lblIteration.setTextColor(COLOR_RED);
            if (userState != IEvolutionarySolverStatusDialog.CANCEL)
                lblStagnationValue.setLabel(
                        resourceManager.getLocalizedString("Message.StopIteration",
                        "Maximum iterations reached."));
        }
        
        if (pbStagnation.getValue() >= maxStagnation) {
            lblStagnation.setTextColor(COLOR_RED);
            if (userState != IEvolutionarySolverStatusDialog.CANCEL)
                lblStagnationValue.setLabel(
                        resourceManager.getLocalizedString("Message.StopStagnation",
                        "Process stopped due to stagnation."));
        }

        lblIterationValue.setLabel(String.format(
                resourceManager.getLocalizedString("Message.CurrentIteration",
                "Process stopped at iteration %d of %d."),
                pbIteration.getValue(), maxIterations));
        if (userState == IEvolutionarySolverStatusDialog.CANCEL)
            lblStagnationValue.setLabel(
                    resourceManager.getLocalizedString("Message.StopUser",
                    "Process stopped due to user interruption."));

        pbIteration.setVisible(false);
        pbStagnation.setVisible(false);
        lblIterationValue.setVisible(true);
        lblStagnationValue.setVisible(true);
        
        repaint();

        userState = IEvolutionarySolverStatusDialog.WAITING;
        xDialog.execute();

        lblIteration.setTextColor(defaultTextColor);
        lblStagnation.setTextColor(defaultTextColor);

        lblIterationValue.setVisible(false);
        lblStagnationValue.setVisible(false);
        pbIteration.setVisible(true);
        pbStagnation.setVisible(true);

        btnStop.setEnabled(true);
        btnOK.setEnabled(false);
        btnContinue.setEnabled(false);

        return userState;
    }

    @Override
    public void setVisible(boolean visible) {
        xWindow.setVisible(visible);
    }

    public void dispose() {
        XComponent component = (XComponent) UnoRuntime.queryInterface(XComponent.class, xDialog);
        component.dispose();
    }

    public void actionPerformed(ActionEvent actionEvent) {
        if (userState == IEvolutionarySolverStatusDialog.WAITING) {
            xDialog.endExecute();
            setVisible(true);
        }

        if (actionEvent.ActionCommand.equals("btnStopClick"))
            userState = IEvolutionarySolverStatusDialog.CANCEL;
        else if (actionEvent.ActionCommand.equals("btnOKClick"))
            userState = IEvolutionarySolverStatusDialog.OK;
        else if (actionEvent.ActionCommand.equals("btnContinueClick"))
            userState = IEvolutionarySolverStatusDialog.CONTINUE;
    }

    public void disposing(EventObject eventObject) {
        
    }

}
