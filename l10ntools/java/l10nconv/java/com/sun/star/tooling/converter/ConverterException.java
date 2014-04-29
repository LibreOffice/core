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


/*
 * a simple exception
 * just to separate
 * it from other
 * exceptions
 */
package com.sun.star.tooling.converter;

/**
 * @author Christian Schmidt
 *
 * Thrown if an error occurs during converting from one file format to another
 * that does not belong to an other exception
 */
public class ConverterException extends Exception {

    /**
     *
     */
    public ConverterException() {
        super();

    }

    /**
     * @param arg0
     */
    public ConverterException(String arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     */
    public ConverterException(Throwable arg0) {
        super(arg0);

    }

    /**
     * @param arg0
     * @param arg1
     */
    public ConverterException(String arg0, Throwable arg1) {
        super(arg0, arg1);

    }

}