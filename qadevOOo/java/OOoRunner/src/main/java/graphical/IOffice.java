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



package graphical;

/**
 *
 * @author ll93751
 */
public interface IOffice
{
    /**
     * start an Office, if need
     * @throws graphical.OfficeException
     */
    public void start() throws OfficeException;

    /**
     * Load a document by it's Name
     * @param Name
     * @throws graphical.OfficeException
     */
    public void load(String Name) throws OfficeException;

    /**
     * Create a postscript file in the DOC_COMPARATOR_OUTPUT_DIR directory from a loaded document
     * @throws graphical.OfficeException
     */
    public void storeAsPostscript() throws OfficeException;


    /**
     * Close the background office
     * @throws graphical.OfficeException
     */
    public void close() throws OfficeException;
}
