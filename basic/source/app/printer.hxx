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



#ifndef _BASICPRN_HXX
#define _BASICPRN_HXX

#include "vcl/print.hxx"
#include "vcl/oldprintadaptor.hxx"

class BasicPrinter
{
    boost::shared_ptr<Printer>                      mpPrinter;
    boost::shared_ptr<vcl::OldStylePrintAdaptor>    mpListener;

    short nLine;                        // aktuelle Zeile
    short nPage;                        // aktuelle Seite
    short nLines;                       // Zeilen pro Seite
    short nYoff;                        // Zeilenabstand in Points
    String aFile;                       // Dateiname

    void Header();                      // Seitenkopf drucken
    void StartPage();
public:
    BasicPrinter();
    void Print( const String& rFile, const String& rText, BasicFrame *pFrame );
};

#endif
