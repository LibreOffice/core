/*************************************************************************
 *
 *  $RCSfile: JavaThreadPoolFactory.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-09 10:10:28 $
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


package com.sun.star.lib.uno.environments.remote;


import java.io.UnsupportedEncodingException;

import java.util.Enumeration;
import java.util.Hashtable;


import com.sun.star.uno.UnoRuntime;


public class JavaThreadPoolFactory {
    private static final boolean DEBUG = false;

    protected Hashtable _jobQueues  = new Hashtable();


//      public JavaThreadPoolFactory() {
//          new Thread() {
//                  public void run() {
//                      try {
//                          while(true) {
//                              list();

//                              Thread.sleep(2000);
//                          }
//                      }
//                      catch(InterruptedException interruptedException) {
//                          System.err.println("lister interrupted:" + interruptedException);
//                      }
//                  }
//              }.start();
//      }

    public static ThreadId getThreadId() {
        Thread t = Thread.currentThread();
        return t instanceof JobQueue.JobDispatcher
            ? ((JobQueue.JobDispatcher) t).getThreadId()
            : new ThreadId(UnoRuntime.generateOid(t));
    }

    /**
     * For debugging, lists the jobqueues
     */
    synchronized void list() {
        Enumeration elements = _jobQueues.elements();

        System.err.println("##### JavaThreadPoolFactory.list:");
        while(elements.hasMoreElements()) {
            System.err.println(" - " + elements.nextElement());
        }
    }

    void addJobQueue(JobQueue jobQueue) {
        if(DEBUG) System.err.println("##### "+ getClass().getName() + ".addJobQueue:" + jobQueue + " " + ((jobQueue._sync_jobQueue != null) ? jobQueue._sync_jobQueue._threadId : null));

        _jobQueues.put(jobQueue.getThreadId(), jobQueue);
    }

    JobQueue getJobQueue(ThreadId threadId) {
        return (JobQueue)_jobQueues.get(threadId);
    }

    void removeJobQueue(JobQueue jobQueue) {
        _jobQueues.remove(jobQueue.getThreadId());
    }


    JobQueue getAsyncJobQueue(ThreadId threadId) {
        JobQueue jobQueue = (JobQueue)_jobQueues.get(threadId);

        JobQueue async_jobQueue = null;

        if(jobQueue != null)
            async_jobQueue = jobQueue._async_jobQueue;

        if(DEBUG) System.err.println("##### "+ getClass().getName() + ".getAsyncJobQueue:" + jobQueue + " " + async_jobQueue);

        return async_jobQueue;
    }

    public IThreadPool createThreadPool() {
        return new JavaThreadPool(this);
    }

    void dispose(Object disposeId, Throwable throwable) {
        Enumeration elements = _jobQueues.elements();

        while(elements.hasMoreElements())
            ((JobQueue)elements.nextElement()).dispose(disposeId, throwable);
    }
}
