/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatusDialog.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:57:50 $
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
package com.sun.star.wizards.web;

import com.sun.star.awt.XButton;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XProgressBar;
import com.sun.star.awt.XWindowListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.Renderer;
import com.sun.star.wizards.ui.UnoDialog;
import com.sun.star.wizards.ui.UnoDialog2;
import com.sun.star.wizards.ui.event.MethodInvocation;
import com.sun.star.wizards.ui.event.Task;
import com.sun.star.wizards.ui.event.TaskEvent;
import com.sun.star.wizards.ui.event.TaskListener;

/**
 * @author rpiterman
 * A Class which displays a Status Dialog with status bars.
 * This can display an X number of bars, to enable the
 * status display of more complex tasks.
 *
 */
public class StatusDialog extends UnoDialog2 implements TaskListener {

    public static final int STANDARD_WIDTH = 240;

    private XProgressBar progressBar;
    private XFixedText lblTaskName;
    private XFixedText lblCounter;
    private XButton btnCancel;

    private String[] res;
    private Renderer renderer;
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
    public StatusDialog(XMultiServiceFactory xmsf, int width, String taskName, boolean displayCount,String[] resources, String hid) {
        super(xmsf);

        res = resources;
        if (res.length != 6)
          throw new IllegalArgumentException("The resources argument should contain 6 Strings, see Javadoc on constructor.");

        //display a close button?
        // if enableBreak == false and closeOnFinsih == false;
        boolean b = !enableBreak && !closeOnFinish;

        Helper.setUnoPropertyValues(xDialogModel,
             new String[] { "Closeable","Height","HelpURL","Moveable","Name","PositionX","PositionY","Step","Title","Width"},
             new Object[] { Boolean.FALSE, new Integer(6 + 25 + (b ? 27 : 7)), hid, Boolean.TRUE,"StatusDialog",new Integer(102),new Integer(52), new Integer(0),res[0],new Integer(width)}
        );

        short tabstop = 1;

        lblTaskName = insertLabel("lblTask" ,
                  new String[] {"Height","Label","PositionX","PositionY","TabIndex","Width"},
                  new Object[] { new Integer(8),taskName,new Integer(6),new Integer(6),new Short(tabstop++), new Integer(width * 2 / 3)});
        lblCounter = insertLabel("lblCounter",
                  new String[] {"Height","Label","PositionX","PositionY","TabIndex","Width"},
                  new Object[] { new Integer(8),"",new Integer(width * 2 / 3),new Integer(6),new Short(tabstop++), new Integer(width / 3 - 4)});
        progressBar = insertProgressBar("progress",
                  new String[] {"Height","PositionX","PositionY","TabIndex","Width"},
                new Object[] { new Integer(10),new Integer(6),new Integer(16),new Short(tabstop++), new Integer(width -12)});


        if (b) {
            btnCancel = insertButton("btnCancel","performCancel",this,
            new String[] {"Height","Label","PositionX","PositionY","TabIndex","Width"},
            new Object[] { new Integer(14),res[1] ,new Integer(width / 2 - 20 ),new Integer(6+25+ 7 ),new Short(tabstop++), new Integer(40)});
        }

//      xWindow.addWindowListener((XWindowListener)guiEventListener);
//      guiEventListener.add("StatusDialog" ,EVENT_WINDOW_SHOWN, "performRunnable",this);

    }

    private void initProgressBar(Task t) {
        progressBar.setRange(0,t.getMax());
        progressBar.setValue(0);
    }

    private void setStatus(int status) {
        if (finished) return;
        progressBar.setValue(status);
        xReschedule.reschedule();
    }

    public void setLabel(String s) {
//      lblTaskName.setText(s);
        Helper.setUnoPropertyValue( UnoDialog.getModel(lblTaskName), "Label", s);
        xReschedule.reschedule();
    }

    /**
     * change the max property of the status bar
     * @param max
     */
    private void setMax(int max) {
        if (finished) return;
        Helper.setUnoPropertyValue(getModel(progressBar),"ProgressValueMax", new Integer(max));
    }

    /**
     * initialize the status bar according
     * to the given event.
     */
    public void taskStarted(TaskEvent te) {
        finished = false;
        initProgressBar(te.getTask());
    }

    /**
     * closes the dialog.
     */
    public void taskFinished(TaskEvent te) {
        finished = true;
        if (closeOnFinish) {
//          xDialog.endExecute();
            parent.xWindow.setEnable(true);
            try {
                xWindow.setVisible(false);
                xComponent.dispose();
               //System.out.println("disposed");
            }
            catch (Exception ex) {ex.printStackTrace();}
         }
         else
               Helper.setUnoPropertyValue(getModel(btnCancel),"Label",res[2]);

    }

    /**
     * changes the status display
     */
    public void taskStatusChanged(TaskEvent te) {
        setMax(te.getTask().getMax());
        setStatus(te.getTask().getStatus());
    }

    /**
     * changes the displayed text.
     * A renderer is used to render
     * the task's subtask name to a resource string.
     */
    public void subtaskNameChanged(TaskEvent te) {
        if (renderer != null)
            setLabel(renderer.render(te.getTask().getSubtaskName()));
    }

    /**
     * displays the status dialog
     * @param parent the parent dialog
     * @param r what to do
     */
    public void execute(final UnoDialog parent_, final Task task,  String title) {
        try {
            this.parent = parent_;
            Helper.setUnoPropertyValue( this.xDialogModel, "Title", title );
            try {
                //TODO change this to another execute dialog method.
                task.addTaskListener(StatusDialog.this);
                setMax(10);
                setStatus(0);
                setLabel(task.getSubtaskName());
                parent.xWindow.setEnable(false);
                setVisible(parent);
                if (finishedMethod != null)
                    finishedMethod.invoke();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * not supported !
     */
    public void performCancel() {//TODO - implement a thread thing here...
        xWindow.setVisible(false);
    }


    /**
     * @return the subTask renderer object
     */
    public Renderer getRenderer() {
        return renderer;
    }

    /**
     * @param renderer
     */
    public void setRenderer(Renderer renderer) {
        this.renderer = renderer;
    }

    /**
     * sets a method to be invoced when the
     *
     */
    public void setFinishedMethod(MethodInvocation mi) {
        finishedMethod = mi;
    }


}
