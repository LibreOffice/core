/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportJob.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:26:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
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
package com.sun.star.report;

import java.io.IOException;

/**
 * This allows some simple job control. A job can be interrupted (or if it has
 * not yet been started, canceled), and the job status can be queried (scheduled,
 * running, finished).
 *
 * <table border="1">
 * <tr>
 * <th>JobStatus</th><th>running</th><th>finished</th>
 * </tr>
 * <tr>
 * <td>Scheduled</td><td>false</td><td>false</td>
 * </tr>
 * <tr>
 * <td>Running</td><td>true</td><td>false</td>
 * </tr>
 * <tr>
 * <td>Finished</td><td>false</td><td>true</td>
 * </tr>
 * </table>
 *
 * @author Thomas Morgner
 */
public interface ReportJob
{

    /**
     * Although we might want to run the job as soon as it has been
     * created, sometimes it is wiser to let the user add some listeners
     * first. If we execute at once, the user either has to deal with
     * threading code or wont receive any progress information in single
     * threaded environments.
     */
    public void execute()
            throws ReportExecutionException, IOException;

    /**
     * Interrupt the job.
     */
    public void interrupt();

    /**
     * Queries the jobs execution status.
     *
     * @return true, if the job is currently running, false otherwise.
     */
    public boolean isRunning();

    /**
     * Queries the jobs result status.
     *
     * @return true, if the job is finished (or has been interrupted), false
     * if the job waits for activation.
     */
    public boolean isFinished();

    public void addProgressIndicator(JobProgressIndicator indicator);

    public void removeProgressIndicator(JobProgressIndicator indicator);
}
