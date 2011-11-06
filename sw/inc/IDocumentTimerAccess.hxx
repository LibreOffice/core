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



 #ifndef IDOCUMENTTIMERACCESS_HXX_INCLUDED
 #define IDOCUMENTTIMERACCESS_HXX_INCLUDED

 /** Get information about the current document state
 */
 class IDocumentTimerAccess
 {
 public:
    /**
    Set modus to start, i.e. start timer if block count == 0
    */
    virtual void StartIdling() = 0;

    /**
    Set modus to stopped, i.e. stop timer if running
    */
    virtual void StopIdling() = 0;

    /**
    Increment block count, stop timer if running
    */
    virtual void BlockIdling() = 0;

    /**
    Decrement block count, start timer if block count == 0 AND modus == start
    */
    virtual void UnblockIdling() = 0;

 protected:
    virtual ~IDocumentTimerAccess() {};
 };

 #endif // IDOCUMENTTIMERACCESS_HXX_INCLUDED
