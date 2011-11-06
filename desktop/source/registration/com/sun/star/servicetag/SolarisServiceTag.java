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



package com.sun.star.servicetag;

import java.io.IOException;
import java.util.Set;

/**
 * Utility class to obtain the service tag for the Solaris Operating System.
 */
class SolarisServiceTag {
    private final static String[] SolarisProductURNs = new String[] {
        "urn:uuid:a7a38948-2bd5-11d6-98ce-9d3ac1c0cfd7", /* Solaris 8 */
        "urn:uuid:4f82caac-36f3-11d6-866b-85f428ef944e", /* Solaris 9 */
        "urn:uuid:a19de03b-48bc-11d9-9607-080020a9ed93", /* Solaris 9 sparc */
        "urn:uuid:4c35c45b-4955-11d9-9607-080020a9ed93", /* Solaris 9 x86 */
        "urn:uuid:5005588c-36f3-11d6-9cec-fc96f718e113", /* Solaris 10 */
        "urn:uuid:6df19e63-7ef5-11db-a4bd-080020a9ed93"  /* Solaris 11 */
    };

    /**
     * Returns null if not found.
     *
     * There is only one service tag for the operating system.
     */
    static ServiceTag getServiceTag() throws IOException {
        if (Registry.isSupported()) {
            Registry streg = Registry.getSystemRegistry();
            for (String parentURN : SolarisProductURNs) {
                Set<ServiceTag> instances = streg.findServiceTags(parentURN);
                for (ServiceTag st : instances) {
                    // there should have only one service tag for the OS
                    return st;
                }
            }
        }
        return null;
    }
}
