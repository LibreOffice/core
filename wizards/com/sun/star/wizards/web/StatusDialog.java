/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.web;

import com.sun.star.awt.XButton;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XProgressBar;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.IRenderer;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.UnoDialog2;
import com.sun.star.wizards.ui.event.MethodInvocation;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.ui.event.TaskEvent;
import com.sun.star.wizards.ui.event.TaskListener;

/**
 * A Class which displays a Status Dialog with status bars.
 * This can display an X number of bars, to enable the
 * status display of more complex tasks.
 *
 */
public class StatusDialog extends UnoDialog2 implements TaskListener
{

    public static final int STANDARD_WIDTH = 240;
    private XProgressBar progressBar;
    private XFixedText lblTaskName;
    private XFixedText lblCounter;
    private XButton btnCancel;
    private String[] res;
    private IRenderer renderer;
    private boolean enableBreak = false;
    private boolean closeOnFinish = true;
    private MethodInvocation finishedMethod;
    private UnoDialog parent;
    private boolean finished;

    /**
     * Note on the argument resource:
     * This should be a String array containing the followin strings, in the
     * following order:
     * dialog title, cancel, close, counter prefix, counter midfix, counter postfix
     */
    public StatusDialog(XMultiServiceFactory xmsf, int width, String taskName, boolean displayCount, String[] resources, String hid)
    {
        super(xmsf);

        res = resources;
        if (res.length != 6)
        {
            throw new IllegalArgumentException("The resources argument should contain 6 Strings, see Javadoc on constructor.");        //display a close button?
        }
        boolean b = !enableBreak && !closeOnFinish;

        Helper.setUnoPropertyValues(xDialogModel,
                new String[]
                {
                    PropertyNames.PROPERTY_CLOSEABLE, PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_HELPURL, PropertyNames.PROPERTY_MOVEABLE, PropertyNames.PROPERTY_NAME, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_STEP, PropertyNames.PROPERTY_TITLE, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    Boolean.FALSE, new Integer(6 + 25 + (b ? 27 : 7)), hid, Boolean.TRUE, "StatusDialog", 102, 52, 0, res[0], new Integer(width)
                });

        short tabstop = 1;

        lblTaskName = insertLabel("lblTask",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, taskName, 6, 6, new Short(tabstop++), new Integer(width * 2 / 3)
                });
        lblCounter = insertLabel("lblCounter",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    8, PropertyNames.EMPTY_STRING, new Integer(width * 2 / 3), 6, new Short(tabstop++), new Integer(width / 3 - 4)
                });
        progressBar = insertProgressBar("progress",
                new String[]
                {
                    PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                },
                new Object[]
                {
                    10, 6, 16, new Short(tabstop++), new Integer(width - 12)
                });


        if (b)
        {
            btnCancel = insertButton("btnCancel", "performCancel", this,
                    new String[]
                    {
                        PropertyNames.PROPERTY_HEIGHT, PropertyNames.PROPERTY_LABEL, PropertyNames.PROPERTY_POSITION_X, PropertyNames.PROPERTY_POSITION_Y, PropertyNames.PROPERTY_TABINDEX, PropertyNames.PROPERTY_WIDTH
                    },
                    new Object[]
                    {
                        14, res[1], new Integer(width / 2 - 20), new Integer(6 + 25 + 7), new Short(tabstop++), 40
                    });
        }

    }

    private void initProgressBar(Task t)
    {
        progressBar.setRange(0, t.getMax());
        progressBar.setValue(0);
    }

    private void setStatus(int status)
    {
        if (finished)
        {
            return;
        }
        progressBar.setValue(status);
        xReschedule.reschedule();
    }

    public void setLabel(String s)
    {
        Helper.setUnoPropertyValue(UnoDialog.getModel(lblTaskName), PropertyNames.PROPERTY_LABEL, s);
        xReschedule.reschedule();
    }

    /**
     * change the max property of the status bar
     * @param max
     */
    private void setMax(int max)
    {
        if (finished)
        {
            return;
        }
        Helper.setUnoPropertyValue(getModel(progressBar), "ProgressValueMax", new Integer(max));
    }

    /**
     * initialize the status bar according
     * to the given event.
     */
    public void taskStarted(TaskEvent te)
    {
        finished = false;
        initProgressBar(te.getTask());
    }

    /**
     * closes the dialog.
     */
    public void taskFinished(TaskEvent te)
    {
        finished = true;
        if (closeOnFinish)
        {
            parent.xWindow.setEnable(true);
            try
            {
                xWindow.setVisible(false);
                xComponent.dispose();
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }
        else
        {
            Helper.setUnoPropertyValue(getModel(btnCancel), PropertyNames.PROPERTY_LABEL, res[2]);
        }
    }

    /**
     * changes the status display
     */
    public void taskStatusChanged(TaskEvent te)
    {
        setMax(te.getTask().getMax());
        setStatus(te.getTask().getStatus());
    }

    /**
     * changes the displayed text. 
     * A renderer is used to render
     * the task's subtask name to a resource string.
     */
    public void subtaskNameChanged(TaskEvent te)
    {
        if (renderer != null)
        {
            setLabel(renderer.render(te.getTask().getSubtaskName()));
        }
    }

    /**
     * displays the status dialog
     * @param parent_ the parent dialog
     * @param task what to do
     */
    public void execute(final UnoDialog parent_, final Task task, String title)
    {
        try
        {
            this.parent = parent_;
            Helper.setUnoPropertyValue(this.xDialogModel, PropertyNames.PROPERTY_TITLE, title);
            try
            {
                //TODO change this to another execute dialog method.
                task.addTaskListener(StatusDialog.this);
                setMax(10);
                setStatus(0);
                setLabel(task.getSubtaskName());
                parent.xWindow.setEnable(false);
                setVisible(parent);
                if (finishedMethod != null)
                {
                    finishedMethod.invoke();
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    /**
     * not supported !
     */
    public void performCancel()
    {//TODO - implement a thread thing here...
        xWindow.setVisible(false);
    }

    /**
     * @return the subTask renderer object
     */
    public IRenderer getRenderer()
    {
        return renderer;
    }

    /**
     * @param renderer
     */
    public void setRenderer(IRenderer renderer)
    {
        this.renderer = renderer;
    }

    /**
     * sets a method to be invoced when the
     * 
     */
    public void setFinishedMethod(MethodInvocation mi)
    {
        finishedMethod = mi;
    }
}
