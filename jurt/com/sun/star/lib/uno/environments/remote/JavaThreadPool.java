/*************************************************************************
 *
 *  $RCSfile: JavaThreadPool.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kr $ $Date: 2001-03-08 12:28:12 $
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

/**
 * This class implements a java thread pool.
 * <p>
 * @version     $Revision: 1.6 $ $ $Date: 2001-03-08 12:28:12 $
 * @author      Kay Ramme
 * @see         com.sun.star.uno.UnoRuntime
 * @see         com.sun.star.lib.uno.environments.remote.ThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.IThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.Job
 * @see         com.sun.star.lib.uno.environments.remote.JobQueue
 * @since       UDK1.0
 */
public class JavaThreadPool implements IThreadPool {
    /**
     * When set to true, enables various debugging output.
     */
    public static final boolean DEBUG = false;


    protected Hashtable _jobQueues  = new Hashtable();
      protected Hashtable _disposeIds = new Hashtable();
    protected boolean   _disposed = false;


//      public JavaThreadPool() {
//          new Thread() {
//                  public void run() {
//                      try {
//                          while(true) {
//                              list();

//                              Thread.sleep(5000);
//                          }
//                      }
//                      catch(InterruptedException interruptedException) {
//                          System.err.println("lister interrupted:" + interruptedException);
//                      }
//                  }
//              }.start();
//      }

    /**
     * For debugging, lists the jobqueues
     */
    synchronized void list() {
        Enumeration elements = _jobQueues.elements();

        System.err.println("##### ThreadPool.list:");
        while(elements.hasMoreElements()) {
            System.err.println(" - " + elements.nextElement());
        }
    }

    /**
     * Gets the <code>ThreadID</code> of the given thread.
     * <p>
     * @return   the thread id
     * @param    thread   the thread
     * @see      com.sun.star.lib.uno.environments.remote.ThreadID
     */
    static public ThreadID getThreadId(Thread thread) {
        ThreadID threadId = null;

        if(thread instanceof JobQueue.JobDispatcher)
            threadId = ((JobQueue.JobDispatcher)thread).getThreadId();
        else {
            try {
                threadId = new ThreadID(UnoRuntime.generateOid(thread).getBytes("UTF8"));
            }
            catch(UnsupportedEncodingException unsupportedEncodingException) {
                throw new com.sun.star.uno.RuntimeException("JavaThreadPool.getThreadId - unexpected: " + unsupportedEncodingException.toString());
            }
        }

        if(DEBUG) System.err.println("##### ThreadPool.getThreadId:" + threadId);

        return threadId;
    }

    /**
     * Gets the <code>ThreadID</code> of this thread.
     * Implements the method of <code>IThreadPool</code>
     * <p>
     * @return the thread id
     * @see com.sun.star.lib.uno.environments.remote.IThreadPool#getThreadId
     */
    public ThreadID getThreadId() {
        if(_disposed) throw new RuntimeException("ThreadPool.getThreadId - is disposed");

        return getThreadId(Thread.currentThread());
    }


    public void removeJobQueue(ThreadID threadId) {
        _jobQueues.remove(threadId);
          _disposeIds.remove(threadId);
    }

    public void addJobQueue(ThreadID threadId, JobQueue jobQueue/*, Object disposeId*/) {
        if(_disposed) throw new RuntimeException("ThreadPool.addThread(" + threadId + ") - is disposed");

          if(DEBUG) System.err.println("##### ThreadPool.addThread:" + threadId);

        _jobQueues.put(threadId, jobQueue);
//          _disposeIds.put(threadId, disposeId);
    }

    /**
     * Adds a <code>JobQueue</code> for the given thread under the given <code>ThreadID</code>
     * with the given disposeId.
     * <p>
     * @param  createWorkerThread    create a JobQueue with or without worker thread
     * @param  threadId              the thread id to use
     * @param  disposeId             the dispose id
     */
    public JobQueue addThread(boolean createWorkerThread, ThreadID threadId, Object disposeId, JobQueue syncQueue) throws InterruptedException {
        if(_disposed) throw new RuntimeException("ThreadPool.addThread(" + threadId + ") - is disposed");

          if(DEBUG) System.err.println("##### ThreadPool.addThread:" + threadId);

        JobQueue jobQueue = null;
        synchronized(this) {
            jobQueue = (JobQueue)_jobQueues.get(threadId);
            if(jobQueue == null) {
                if(syncQueue != null)
                    jobQueue = new JobQueue(this, threadId, syncQueue);
                else
                    jobQueue = new JobQueue(this, threadId, createWorkerThread);

                  if(disposeId != null)
                      _disposeIds.put(threadId, disposeId);
            }
        }
        jobQueue.acquire();

        return jobQueue;
    }

    /**
     * Adds a jobQueue for the current thread to the threadpool.
     * Requests are now put into this queue.
     * Implements the method of <code>IThreadPool</code>
     * <p>
     * @param  disposeId    the dipose id with which the thread can be interrupted while staying in the queue
     * @see                 #enter
     * @see com.sun.star.lib.uno.environments.remote.IThreadPool#addThread
     */
    public void addThread(Object disposeId) throws InterruptedException {
        if(_disposed) throw new RuntimeException("ThreadPool.addThread - is disposed");

        addThread(false, getThreadId(Thread.currentThread()), disposeId, null);
    }

    /**
     * Gives the <code>JobQueue</code> for the given threadId.
     * <p>
     * @return  the job queue
     * @param   threadId   the thread id
     * @see com.sun.star.lib.uno.environments.remote.ThreadID
     * @see com.sun.star.lib.uno.environments.remote.JobQueue
     */
    public JobQueue getJobQueue(ThreadID threadId) {
        return (JobQueue)_jobQueues.get(threadId);
    }

    /**
     * Removes the <code>JobQueue</code> for the given threadId.
     * <p>
     * @param   threadId     the thread id
     * @see com.sun.star.lib.uno.environments.remote.ThreadID
     * @see com.sun.star.lib.uno.environments.remote.JobQueue
     */
    public void removeThread(ThreadID threadId) {
        if(_disposed) throw new RuntimeException("ThreadPool.removeThread - is disposed");

          if(DEBUG) System.err.println("##### ThreadPool.removeThread:" + threadId);

        JobQueue jobQueue = (JobQueue)_jobQueues.get(threadId);

        if(jobQueue != null)
            jobQueue.release();
    }

    /**
     * Removes the jobQueue for the current thread.
     * Implements the method of <code>IThreadPool</code>
     * <p>
     * @see com.sun.star.lib.uno.environments.remote.IThreadPool#removeThread
     */
    public void removeThread() {
        if(_disposed) throw new RuntimeException("ThreadPool.removeQueue - is disposed");

        removeThread(getThreadId());
    }

    /**
     * Queues a job into the jobQueue of the thread belonging to the jobs threadId.
     * Implements the method of <code>IThreadPool</code>
     * <p>
     * @param job       the job
     * @param disposeId the dispose id
     * @see com.sun.star.lib.uno.environments.remote.IThreadPool#putJob
     */
    private Object _syncPutJob = new Object();

    public void putJob(Job job, Object disposeId)  throws InterruptedException {
        if(_disposed) throw new RuntimeException("ThreadPool.putJob - is disposed");

        JobQueue jobQueue = null;

        ThreadID threadId = job.getThreadId();

        if(DEBUG) System.err.println("#### ThreadPool.putJob:" + threadId + " " + job + " " + _jobQueues);

        synchronized(_syncPutJob) {
            jobQueue = (JobQueue)_jobQueues.get(threadId);
            if(jobQueue == null) {
                if(job.getOperation() == null) // a reply? and no thread for it?
                    throw new RuntimeException(getClass().getName() + ".putJob - no thread for reply " + threadId);

                jobQueue = new JobQueue(this, threadId, true);
            }
            jobQueue.putJob(job, disposeId);
        }
    }

    /**
     * Enters the <code>ThreadPool</code> under the given thread id.
     * Waits for a reply job or an exception.
     * <p>
     * @result   the result of final reply
     * @param threadId   the thread id to use
     */
    public Object enter(int waitTime, ThreadID threadId) throws java.lang.Exception {
        if(_disposed) throw new RuntimeException("ThreadPool.enter - is disposed");

        JobQueue jobQueue = (JobQueue)_jobQueues.get(threadId);

        Object object = null;

        try {
            object = jobQueue.enter(waitTime, _disposeIds.get(threadId));
        }
        finally {
            removeThread(threadId);
        }

        return object;
    }

    /**
     * Lets the current thread enter the ThreadPool.
     * The thread then dispatches all jobs and leaves
     * the ThreadPool when it gets a reply job.
     * Implements the method of <code>IThreadPool</code>
     * <p>
     * @see com.sun.star.lib.uno.environments.remote.IThreadPool#enter
     */
    public Object enter(int waitTime) throws java.lang.Exception {
        return enter(waitTime, getThreadId());
    }

    public Object enter() throws java.lang.Exception {
        return enter(0);
    }

    /**
     * Interrupts all threads which have associated the dispose id.
     * Implements the method of <code>IThreadPool</code>
     * <p>
     * @param disposeId    the dispose id
     * @see com.sun.star.lib.uno.environments.remote.IThreadPool#dispose
     */
    public void dispose(Object disposeId) {
          if(DEBUG) System.err.println("##### " + getClass().getName() + ".dispose:" + disposeId);
        // clear all jobqueues
        /*synchronized(_jobQueues)*/ {
            Enumeration elements = _jobQueues.elements();
            while(elements.hasMoreElements()) {
                JobQueue jobQueue = (JobQueue)elements.nextElement();
                jobQueue.interrupt(disposeId);
            }
        }
    }

    /**
     * Stops interrupting all jobs queued by the given bridge.
     * Implements the method of <IThreadPool>.
     */
    public synchronized void stopDispose(Object disposeId) {

    }

    synchronized void dispose() {
        if(_disposed) throw new RuntimeException("ThreadPool.dispose - is disposed");

        _disposed = true;

        if(_jobQueues.size() > 0)
            System.err.println("Warning! ThreadPool.dipose - there are active JobQueus:" + _jobQueues.size());

        // clear all jobqueues
        Enumeration elements = _jobQueues.elements();
        while(elements.hasMoreElements())
            ((JobQueue)elements.nextElement()).clear();

        _jobQueues.clear();
        _jobQueues.notifyAll();

        _jobQueues = null;
    }
}
