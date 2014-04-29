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



package ifc.lang;


import lib.MultiMethodTest;

import com.sun.star.lang.XMain;


/**
 * Testing <code>com.sun.star.lang.XMain</code>
 * interface methods :
 * <ul>
 *  <li><code> run()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ARGS'</code> (of type <code>String[]</code>):
 *   used as argument for <code>run</code> method. </li>
 * <ul> <p>
 *
 * @see com.sun.star.lang.XMain
 */
public class _XMain extends MultiMethodTest{

    public static XMain oObj = null;

    /**
     * Just calls the method. <p>
     *
     * Has <b> OK </b> status if no runtime exceptions occurred.
     */
    public void _run() {
        String[] args = (String[])tEnv.getObjRelation("ARGS");

        log.println("Running with arguments:");
        for (int i=0; i< args.length; i++)
            log.println("#" + i + ": " + args[i]);

        oObj.run(args);

        tRes.tested("run()", true);
    }

}
