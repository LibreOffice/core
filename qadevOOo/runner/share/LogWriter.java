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



package share;

public interface LogWriter {

    /**
     * Method to print
     */
    public void println(String msg);

    /**
     * initialization
     *
     */

    public boolean initialize(share.DescEntry entry, boolean active);

    /**
     * will mostly be used by outproducers to sum up
     * the information, maybe write them to a db
     */

    public boolean summary(share.DescEntry entry);


    /**
     * Returns the <CODE>Watcher</CODE> which is associated with this logger
     * @see share.Watcher
     * @return the associated <CODE>Watcher</CODE>
     */
    public Object getWatcher();

    /**
     * Set a <CODE>Watcher</CODE> to the <CODE>LogWriter</CODE>
     * This is useful if a test starts a new office instance by itself. In this cases
     * the <CODE>LogWritter</CODE> could retrigger the <CODE>Watcher</CODE>
     * @see share.Watcher
     * @param watcher the new <CODE>Watcher</CODE>
     */
    public void setWatcher(Object watcher);

}
