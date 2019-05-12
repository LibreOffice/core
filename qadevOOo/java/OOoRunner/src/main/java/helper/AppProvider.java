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



package helper;

/**
 *
 * Interface to get a Manager to access the application to check
 */
public interface AppProvider {

    /**
     * Method to get the desired Manager
     */
    public Object getManager(lib.TestParameters param);

    /**
     * Method to dispose the desired Manager
     */
    public boolean disposeManager(lib.TestParameters param);

    /**
     * Close an office.
     * @param param The test parameters.
     * @param closeIfPossible If true, close even if
     * it was running before the test
     * @return True, if close worked.
     */
    public boolean closeExistingOffice(lib.TestParameters param,
                                                boolean closeIfPossible);
}
