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

import java.awt.Dimension;

import java.io.InputStream;

/**
 *
 * @author oj93728
 */
public interface ImageService
{

    /**
     * @param image
     * @return the mime-type of the image as string.
     * @throws ReportExecutionException
     */
    String getMimeType(final InputStream image) throws ReportExecutionException;

    /**
     * @param image
     * @return the mime-type of the image as string.
     * @throws ReportExecutionException
     */
    String getMimeType(final byte[] image) throws ReportExecutionException;

    /**
     * @param image
     * @returns the dimension in 100th mm.
     *
     * @throws ReportExecutionException
     * @return*/
    Dimension getImageSize(final InputStream image) throws ReportExecutionException;

    /**
     * @param image
     * @returns the dimension in 100th mm.
     *
     * @throws ReportExecutionException
     * @return*/
    Dimension getImageSize(final byte[] image) throws ReportExecutionException;
}

