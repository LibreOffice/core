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

/**
 * @author: ll93751
 * @copyright: Sun Microsystems Inc. 2010
 */

package complex.junitskeleton;

public class justatest /* extends *//* implements */ {
    //public static void main( String[] argv ) {
    //
    //    }
    public void justatest()
        {
            System.out.println("justatest CTor.");
        }

    public void testfkt()
        {
            System.out.println("Test called.");
        }

      /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    public static void shortWait()
    {
        try
        {
            Thread.sleep(500) ;
        }
        catch (InterruptedException e)
        {
            System.out.println("While waiting :" + e) ;
        }
    }

}
