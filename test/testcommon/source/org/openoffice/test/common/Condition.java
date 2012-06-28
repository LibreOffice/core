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



package org.openoffice.test.common;


public abstract class Condition {
    /**
     *
     * @return true, if the condition is true. false, if it's not true.
     */
    public abstract boolean value();


    public boolean test(double iTimeout, double interval) {
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < iTimeout * 1000) {
            if (value())
                return true;
            try {
                Thread.sleep((long) (interval * 1000));
            } catch (InterruptedException e) {
            }
        }

        return value();
    }

    public void waitForTrue(String message, double iTimeout, double interval) {
        if (!test(iTimeout, interval)) {
            throw new RuntimeException(message);
        }
    }
}
