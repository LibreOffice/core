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



#ifndef INCLUDED_RTFSCANNER_HXX
#define INCLUDED_RTFSCANNER_HXX

#include <WriterFilterDllApi.hxx>
#include <vector>
#include <rtftok/RTFInputSource.hxx>
#include <rtftok/RTFScannerHandler.hxx>

namespace writerfilter { namespace rtftok {

class WRITERFILTER_DLLPUBLIC RTFScanner {
public:
        RTFScanner(RTFScannerHandler &eventHandler_) : eventHandler(eventHandler_) {};
        virtual ~RTFScanner()   { }

    const char* YYText()    { return yytext; }
    int YYLeng()        { return yyleng; }

    virtual int yylex() = 0;

    int lineno() const      { return yylineno; }

protected:
    char* yytext;
    int yyleng;
    int yylineno;       // only maintained if you use %option yylineno
     int yy_flex_debug; // only has effect with -d or "%option debug"
        RTFScannerHandler &eventHandler;


public:
  static writerfilter::rtftok::RTFScanner* createRTFScanner(writerfilter::rtftok::RTFInputSource& inputSource, writerfilter::rtftok::RTFScannerHandler &eventHandler);
};

} } /* end namespace writerfilter::rtftok */


#endif /* INCLUDED_RTFSCANNER_HXX */
