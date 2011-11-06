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



#ifndef ADC_TOKEN2_HXX
#define ADC_TOKEN2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETRS



/** This is the interface for parser classes, which get a sequence of Token s from
    a text.

    Start() starts to parse the text from the given i_rSource.
    GetNextToken() returns a Token on the heap as long as there are
    still characters in the text left. The last time GetNextToken()
    returns NULL.

    The algorithms for parsing tokens from the text are an issue of
    the derived classes.
*/
class TextToken
{
  public:
    // LIFECYCLE
    virtual             ~TextToken() {}


    // INQUIRY
    virtual const char* Text() const = 0;
};


#endif


