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


package com.sun.star.report;

/**
 * Functions are statefull computation components.
 *
 * Functions are allowed to have a state, but are considered
 * immutable (the same is true for expressions). Functions are
 * allowed to update their state on every call to 'advance', which
 * signals, that the cursor has been moved.
 *
 * Functions have no way to 'reset' their state, if a reset is needed,
 * the report engine is urged to query a new instance from the ReportAddin.
 */
public interface ReportFunction extends ReportExpression
{

    public ReportFunction advance(DataRow row);
}
