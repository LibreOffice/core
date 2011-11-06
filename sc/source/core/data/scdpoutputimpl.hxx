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


#ifndef SCDPOUTPUTIMPL_HXX
#define SCDPOUTPUTIMPL_HXX

#include "document.hxx"

#define SC_DP_FRAME_INNER_BOLD      20
#define SC_DP_FRAME_OUTER_BOLD      40

#define SC_DP_FRAME_COLOR           Color(0,0,0) //( 0x20, 0x40, 0x68 )

class OutputImpl
{
    ScDocument*         mpDoc;
    sal_uInt16              mnTab;
    ::std::vector< bool > mbNeedLineCols;
    ::std::vector< SCCOL > mnCols;

    ::std::vector< bool > mbNeedLineRows;
    ::std::vector< SCROW > mnRows;

    SCCOL   mnTabStartCol;
    SCROW   mnTabStartRow;
    SCCOL   mnMemberStartCol;
    SCROW   mnMemberStartRow;

    SCCOL   mnDataStartCol;
    SCROW   mnDataStartRow;
    SCCOL   mnTabEndCol;
    SCROW   mnTabEndRow;

public:
    OutputImpl( ScDocument* pDoc, sal_uInt16 nTab,
        SCCOL   nTabStartCol,
        SCROW   nTabStartRow,
        SCCOL   nMemberStartCol,
        SCROW   nMemberStartRow,
        SCCOL nDataStartCol,
        SCROW nDataStartRow,
        SCCOL nTabEndCol,
        SCROW nTabEndRow );
    sal_Bool AddRow( SCROW nRow );
    sal_Bool AddCol( SCCOL nCol );

    void OutputDataArea();
    void OutputBlockFrame ( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Bool bHori = sal_False );

};

#endif
