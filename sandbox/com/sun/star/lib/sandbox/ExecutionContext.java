/*************************************************************************
 *
 *  $RCSfile: ExecutionContext.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.sandbox;

import java.awt.Image;
import java.awt.Dimension;
import java.awt.Container;
import java.awt.BorderLayout;

import java.applet.Applet;
import java.applet.AppletStub;
import java.applet.AppletContext;
import java.applet.AudioClip;

import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;

import java.net.URL;
import java.net.MalformedURLException;

import java.text.MessageFormat;

import java.util.Hashtable;
import java.util.Observable;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

public abstract class ExecutionContext extends Observable {
    private static final boolean DEBUG = false;

    private static int instances;

    /* message ids */
    protected final static int CMD_LOAD    = 1;
    protected final static int CMD_INIT    = 2;
    protected final static int CMD_START   = 3;
    protected final static int CMD_STOP    = 4;
    protected final static int CMD_DESTROY = 5;
    protected final static int CMD_DISPOSE = 6;

    protected final static int LOADED    = 1;
    protected final static int INITED    = 2;
    protected final static int STARTED   = 3;
    protected final static int STOPPED   = 4;
    protected final static int DESTROYED = 5;
    protected final static int DISPOSED  = 6;

    private int status = DISPOSED;

    protected ClassContext classContext;

    private Thread dispatchThread = null;
    private SandboxThreadGroup threadGroup = null;

    private String name;

    protected ResourceBundle resourceBundle;

    private Object synObj = new Object();
    private Message head;
    private Message tail;
    private boolean loop = true;

    protected ExecutionContext() {
        instances ++;
    }

    public void finalize() {
        instances --;
    }

    public int getStatus() {
        return status;
    }

    Object getSynObject() {
        return synObj;
    }

    class Message {
        Message next;
        int id;

        Message(int id) {
            this.id = id;
        }
    }

    public void init(String name, ClassContext classContext) throws MissingResourceException {
        this.name = name;

        resourceBundle = ResourceBundle.getBundle("sun.applet.resources.MsgAppletViewer");

        this.classContext = classContext;

        threadGroup = new SandboxThreadGroup(classContext.getThreadGroup(), name, classContext.getClassLoader());
        threadGroup.setDaemon(true);

        dispatchThread = new Thread( threadGroup, new Runnable() {
            public void run() {
                while( loop ) {
                    synchronized( getSynObject() ) {
                        if (head == null) {
                            try {
                                getSynObject().wait();
                            }
                            catch (InterruptedException e ) {
                                if (DEBUG) System.err.println("#### ExecutionContext - dispatchThread " + dispatchThread.toString()  + " -interrupted");
                                  break;
                            }
                        }
                    }
                    if (DEBUG) System.err.println("#### ExecutionContext dispatchThread " + dispatchThread.toString() + " -dispatching: " + head.id);
                    dispatch( head.id );
                    if (DEBUG) System.err.println("#### ExecutionContext dispatchThread " + dispatchThread.toString() + " get next head - current state is " +head.id );
                    synchronized( getSynObject() ) {
                        head = head.next;
                        getSynObject().notify();
                    }
                }
                  if(DEBUG) System.err.println("#### ExecutionContext - dispatchThread  -terminating");
            }
        });

        dispatchThread.start();
    }
    public void sendEvent(int id) {
        sendEvent(id, 0);
    }

    public void sendEvent(int id, int timeout) {
          synchronized( getSynObject() ) {
            try {
                Message message = new Message(id);
                if(tail != null)
                    tail.next = message;

                tail = message;

                if(head == null)
                    head = tail;

                getSynObject().notify();

                if ( timeout != 0 )
                    getSynObject().wait( timeout );
            }
            catch( InterruptedException e ) {
            }
          }
    }

    public void dispose() {
          //if(DEBUG) System.err.println("#### ExecutionContext.disposing");
        dispose(1000);
    }

    public void dispose( long timeout ) {
        if(DEBUG) System.err.println("#### ExecutionContext "+ dispatchThread.toString() +"disposing:" + timeout);
        try {
            try {
                synchronized( getSynObject() ) {
                    while( head != null )
                        getSynObject().wait( timeout ); // wait at most one second for each queued command
                }
            }
            catch(InterruptedException ee) {
                if(DEBUG) System.err.println("#### ExecutionContext " + dispatchThread.toString() + " - dispose 1:" + ee);
            }

            loop = false;

            getSynObject().notifyAll();
            // dispatchThread.interrupt();
            dispatchThread.join(timeout);

            if(DEBUG) threadGroup.list();

//              threadGroup.stop();

            if ( !threadGroup.isDestroyed() )
                threadGroup.destroy();
        }
        catch (Exception ie) {
            if(DEBUG) System.err.println("#### ExecutionContext "+ threadGroup.toString() + " - destroyThreadGroup:" + ie);
            try {
                threadGroup.stop();
            } catch (Exception se) {
                if(DEBUG) System.err.println("#### ExecutionContext "+ threadGroup.toString() + " - stop ThreadGroup:" + se);
            }
        }

        classContext = null;

        dispatchThread = null;
        threadGroup.dispose();
        threadGroup = null;

        name = null;

        resourceBundle = null;

        synObj = null;
        head = null;
        tail = null;
    }


    protected void showStatus(String status) {
        if (DEBUG) System.err.println("#### ExecutionContext.showStatus:" + status);
          setChanged();
          notifyObservers(resourceBundle.getString("appletpanel." + status));
    }

    protected void showStatus(String status, String arg1) {
        if(DEBUG) System.err.println("#### ExecutionContext.showStatus" + status + " " + arg1);
        try {
            Object args[] = new Object[1];
            args[0] = arg1;
            setChanged();
            try {
                notifyObservers(MessageFormat.format(resourceBundle.getString("appletpanel." + status), args));
            }
            catch(MissingResourceException me) {}
        }
        catch(Exception ee) {
            if(DEBUG)System.err.println("#### ExecutionContext.showStatus:" + ee);
        }
    }


    public ThreadGroup getThreadGroup() {
        return threadGroup;
    }

    /**
     * Send an event. Queue it for execution by the handler thread.
     */
    public void dispatch(int id) {
        try {
            switch(id) {
            case CMD_LOAD:
                if (status == DISPOSED) {
                    xload();
                    status = LOADED;
                    showStatus("loaded");
                }
                else
                    showStatus("notdisposed");
                break;

            case CMD_INIT:
                if(status == LOADED || status == DESTROYED) {
                    xinit();
                    status = INITED;
                    showStatus("inited");
                }
                else
                    showStatus("notloaded");
                break;

            case CMD_START:
                if (status == INITED || status == STOPPED) {
                    xstart();
                    status = STARTED;
                    showStatus("started");
                }
                else
                    showStatus("notinited");
                break;

            case CMD_STOP:
                if (status == STARTED) {
                    xstop();
                    status = STOPPED;
                    showStatus("stopped");
                }
                else
                    showStatus("notstarted");
                break;

            case CMD_DESTROY:
                if(status == INITED || status == STOPPED) {
                    xdestroy();
                    status = DESTROYED;
                    showStatus("destroyed");
                }
                else
                    showStatus("notstopped");
                break;

            case CMD_DISPOSE:
                if (status == LOADED || status == DESTROYED) {
                    xdispose();
                    //  baseResourceLoader.flush();
                    status = DISPOSED;
                    showStatus("disposed");
                }
                else
                    showStatus("notdestroyed");
                break;

            default:
                xExtended(id);
            }
        }
        catch (ClassNotFoundException classNotFoundException) {
            showStatus("notfound", name);
            if(DEBUG) classNotFoundException.printStackTrace();
        }
        catch (InstantiationException instantiationException) {
            showStatus("nocreate", name);
            if(DEBUG) instantiationException.printStackTrace();
        }
        catch (IllegalAccessException illegalAccessException) {
            showStatus("noconstruct", name);
            if(DEBUG) illegalAccessException.printStackTrace();
        }
        catch (Exception exception) {
            showStatus("exception", exception.getMessage());
            if(DEBUG) exception.printStackTrace();
        }
        catch (ThreadDeath threadDeath) {
            showStatus("death");
            if(DEBUG) threadDeath.printStackTrace();

            throw threadDeath;
        }
        catch (Error error) {
            showStatus("error", error.getMessage());
            if(DEBUG) error.printStackTrace();
        }
    }

    protected abstract void xload() throws ClassNotFoundException, InstantiationException, IllegalAccessException;
    protected abstract void xinit();
    protected abstract void xstart();
    protected abstract void xstop();
    protected abstract void xdestroy();
    protected abstract void xdispose();

    protected void xExtended(int id) {
    }

    /*
    **
    */
    public void sendLoad() {
        sendEvent(CMD_LOAD);
    }

    public void sendInit() {
        sendEvent(CMD_INIT);
    }

    public void sendStart() {
          sendEvent(CMD_START);
    }

    public void sendStop() {
        sendEvent(CMD_STOP);
    }

    public void sendDestroy() {
        sendEvent(CMD_DESTROY);
    }

    public void sendDispose() {
        sendEvent(CMD_DISPOSE);
    }

    public void startUp() {
        sendLoad();
        sendInit();
        sendStart();
    }

    public void shutdown() {
        sendStop();
        sendDestroy();
        sendDispose();
    }

    public void restart() {
        sendStop();
        sendDestroy();
        sendInit();
        sendStart();
    }

    public void reload() {
        sendStop();
        sendDestroy();
        sendDispose();
        sendLoad();
        sendInit();
        sendStart();
    }
}

