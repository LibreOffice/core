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



package org.openoffice.xmerge.converter.xml.sxw.wordsmith;

/**
 *  WordSmith utility class.
 *
 *  @author   David Proulx
 */
class util {

    /**
     *  Convert 2 bytes to an integer.
     *
     *  @param  data   <code>byte</code> data to convert.
     *  @param  index  Index to convert.
     *
     *  @return  Converted integer.
     */
    static int intFrom2bytes(byte[] data, int index) {
        return (((data[index] & 0xFF) << 8)
                | (data[index+1] & 0xFF));

    }


    /**
     *  Convert 4 bytes to an integer.
     *
     *  @param  data   <code>byte</code> data to convert.
     *  @param  index  Index to convert.
     *
     *  @return  Converted integer.
     */
    static int intFrom4bytes(byte[] data, int index) {
        return (((data[index] & 0xFF) << 24)
          | ((data[index + 1] & 0xFF) << 16)
          | ((data[index + 2] & 0xFF) << 8)
                | (data[index+3] & 0xFF));

    }
}

