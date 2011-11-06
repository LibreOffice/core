/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
     * @throws java.io.IOException
     * @throws ReportExecutionException
     */
    void execute()
            throws ReportExecutionException, IOException;

    /**
     * Interrupt the job.
     */
    void interrupt();

    /**
     * Queries the jobs execution status.
     *
     * @return true, if the job is currently running, false otherwise.
     */
    boolean isRunning();

    /**
     * Queries the jobs result status.
     *
     * @return true, if the job is finished (or has been interrupted), false
     * if the job waits for activation.
     */
    boolean isFinished();

    void addProgressIndicator(JobProgressIndicator indicator);

    void removeProgressIndicator(JobProgressIndicator indicator);
}
