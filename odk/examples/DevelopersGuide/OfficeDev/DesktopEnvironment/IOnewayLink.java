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



import java.util.Vector;

// __________ Implementation __________

/**
 * We need a generic interface to forward any oneway uno interface method
 * by using threads to the original object. Reason:
 * It's not allowed to call synchronoues back to the office if a java object
 * was called in a oneway declared interface method. Then it must be
 * executed asynchronoues. To do so - a thread can be created which use this
 * interface. It get the object, which whis to be called back and the type and
 * parameter of the original request.
 *
 * @author     Andreas Schl&uuml;ns
 * @created    17.07.2002 08:09
 */
public interface IOnewayLink
{
    // _______________________________

    /**
     * @param nRequest
     *          The two user of this callback can define an unique number,
     *          which identify the type of original interface method. So the called
     *          interface object can decide, which action will be necessary.
     *
     * @param lParams
     *          If the original method used parameters, they will be coded here in
     *          a generic way. Only the called interface object know (it depends
     *          from the original request - see nRequest too), how this list must
     *          be interpreted.
     *          Note: Atomic types (e.g. int, long) will be transported as objects
     *          too (Integer, Long)!
     */
    public abstract void execOneway( int nRequest, Vector lParams );
}
