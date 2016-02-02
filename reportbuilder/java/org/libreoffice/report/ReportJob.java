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
package org.libreoffice.report;

import java.io.IOException;

/**
 * This allows some simple job control. A job can be interrupted (or if it has
 * not yet been started, canceled), and the job status can be queried (scheduled,
 * running, finished).
 *
 * <table summary="Job status values" border="1">
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
 */
public interface ReportJob
{

    /**
     * Although we might want to run the job as soon as it has been
     * created, sometimes it is wiser to let the user add some listeners
     * first. If we execute at once, the user either has to deal with
     * threading code or won't receive any progress information in single
     * threaded environments.
     * @throws java.io.IOException
     * @throws ReportExecutionException
     */
    void execute()
            throws ReportExecutionException, IOException;

}
