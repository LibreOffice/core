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



#ifndef SD_ASYNCHRONOUS_TASK_HXX
#define SD_ASYNCHRONOUS_TASK_HXX

namespace sd { namespace tools {

/** Interface for the asynchronous execution of a task.  This interface
    allows an controller to run the task either timer based with a fixed
    amount of time between the steps or thread based one step right after
    the other.
*/
class AsynchronousTask
{
public:
    /** Run the next step of the task.  After HasNextStep() returns false
        this method should ignore further calls.
    */
    virtual void RunNextStep (void) = 0;

    /** Return <true/> when there is at least one more step to execute.
        When the task has been executed completely then <false/> is
        returned.
    */
    virtual bool HasNextStep (void) = 0;
};

} } // end of namespace ::sd::tools

#endif
