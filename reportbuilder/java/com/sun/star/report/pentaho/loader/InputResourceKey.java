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


package com.sun.star.report.pentaho.loader;

import java.io.Serializable;

/**
 * Creation-Date: Feb 22, 2007, 8:51:42 PM
 *
 * @author Thomas Morgner
 */
public class InputResourceKey implements Serializable
{

    private static final long serialVersionUID = 2819901838705793075L;
    private final Object inputRepositoryId;
    private final String path;

    public InputResourceKey(final Object inputRepositoryId, final String path)
    {
        this.inputRepositoryId = inputRepositoryId;
        this.path = path;
    }

    public Object getInputRepositoryId()
    {
        return inputRepositoryId;
    }

    public String getPath()
    {
        return path;
    }

    public String toString()
    {
        return "InputResourceKey{" + "inputRepositoryId=" + inputRepositoryId + ", path='" + path + '\'' + '}';
    }
}
