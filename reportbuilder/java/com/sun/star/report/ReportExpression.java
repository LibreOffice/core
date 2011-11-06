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
 * Expressions are simple computation components.
 *
 * Expressions are always assumed to be immutable. They are not allowed to
 * change their state and it is not guaranteed, in which order they get called.
 * If the expression has been called before, the last computed value will be
 * available from the datarow.
 *
 * This construct allows us to write expressions in the form
 * "Sum := Sum + Column".
 *
 * Multiple calls to getValue on the same expression instance must return the
 * same value (assuming that the datarow passed in is the same).
 */
public interface ReportExpression
{

    void setParameters(Object[] parameters);

    Object getParameters();

    Object getValue(DataRow row);
}
