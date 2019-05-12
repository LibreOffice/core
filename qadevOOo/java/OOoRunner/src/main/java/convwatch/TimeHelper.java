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



package convwatch;

public class TimeHelper
{
    /*
      wait a second the caller don't need to handle the interruptexception
      @param _nSeconds how long should we wait
      @param _sReason  give a good reason, why we have to wait
     */
    static void waitInSeconds(int _nSeconds, String _sReason)
        {
            GlobalLogWriter.get().println("Wait " + String.valueOf(_nSeconds) + " sec. Reason: " + _sReason);
            try {
                java.lang.Thread.sleep(_nSeconds * 1000);
            } catch (java.lang.InterruptedException e2) {}
        }
}
