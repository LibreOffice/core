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



#ifndef _HSTREAM_H_
#define _HSTREAM_H_

typedef unsigned char byte;
/**
 * Stream class
 */
class HStream
{
    public:
        HStream();
        virtual ~HStream();

/**
 *
 */
        void addData( const byte *buf, int aToAdd);
/**
 * Read some byte to buf as given size
 */
        int readBytes( byte *buf, int aToRead);
/**
 * Skip some byte from stream as given size
 */
        int skipBytes( int aToSkip );
/**
 * @returns Size of remained stream
 */
        int available();
/**
 * remove the stream from this object.
 */
        void closeInput();
    private:
        int size;
        byte *seq;
        int pos;
};
#endif
