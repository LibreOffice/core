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



#ifndef INCLUDED_RTFSCANNERHANDLER_HXX
#define INCLUDED_RTFSCANNERHANDLER_HXX

namespace writerfilter { namespace rtftok {

class RTFScannerHandler
{
public:
    virtual void dest(char*token, char*value)=0;
    virtual void ctrl(char*token, char*value)=0;
    virtual void lbrace(void)=0;
    virtual void rbrace(void)=0;
    virtual void addSpaces(int count)=0;
    virtual void addBinData(unsigned char data)=0;
    virtual void addChar(char ch) =0;
    virtual void addCharU(sal_Unicode ch) =0;
    virtual void addHexChar(char* hexch) =0;

};

} } /* end namespace writerfilter::rtftok */

#endif /* INCLUDED_RTFSCANNERHANDLER_HXX */
