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



#ifndef SC_HTMLIMP_HXX
#define SC_HTMLIMP_HXX

#include "eeimport.hxx"

class ScHTMLParser;

class ScHTMLImport : public ScEEImport
{
private:
    static void         InsertRangeName( ScDocument* pDoc, const String& rName, const ScRange& rRange );

public:
    ScHTMLImport( ScDocument* pDoc, const String& rBaseURL, const ScRange& rRange, sal_Bool bCalcWidthHeight = sal_True );
    virtual ~ScHTMLImport();
    const ScHTMLParser* GetParser() const { return (ScHTMLParser*)mpParser; }

    virtual void        WriteToDocument( sal_Bool bSizeColsRows = sal_False, double nOutputFactor = 1.0,
                                         SvNumberFormatter* pFormatter = NULL, bool bConvertDate = true );

    static String       GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName );
};


#endif
