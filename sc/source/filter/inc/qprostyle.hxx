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




#ifndef SC_QPROSTYLE_HXX
#define SC_QPROSTYLE_HXX

#include <sal/config.h>
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include <tools/string.hxx>

#include <tools/color.hxx>
#include "flttypes.hxx"
#include "ftools.hxx"

class ScQProStyle
{
    enum limits { maxsize = 256 };
    sal_uInt8  maAlign[ maxsize ];
    sal_uInt8  maFont[ maxsize ];
    sal_uInt16 maFontRecord[ maxsize ];
    sal_uInt16 maFontHeight[ maxsize ];
    String     maFontType[ maxsize ];

    public:
    ScQProStyle();
    void SetFormat( ScDocument *pDoc, sal_uInt8 nCol, sal_uInt16 nRow, SCTAB nTab, sal_uInt16 nStyle );
    void setFontRecord(sal_uInt16 nIndex, sal_uInt16 nData, sal_uInt16 nPtSize)
    {
        if (nIndex < maxsize)
        {
            maFontRecord[ nIndex ] = nData;
            maFontHeight[ nIndex ] = nPtSize;
        }
    }
    void setFontType( sal_uInt16 nIndex, String &aLabel )
        { if (nIndex < maxsize) maFontType[ nIndex ] = aLabel; }
    void setAlign( sal_uInt16 nIndex, sal_uInt8 nData )
        { if (nIndex < maxsize) maAlign[ nIndex ] = nData; }
    void setFont( sal_uInt16 nIndex, sal_uInt8 nData )
        { if (nIndex < maxsize) maFont[ nIndex ] = nData; }
};
#endif
