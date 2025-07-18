/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <op.h>
#include <optab.h>
#include "lotfilter.hxx"

// edit function char *X( char * )
const OPCODE_FKT LotusContext::pOpFkt[ FKT_LIMIT ] =
{                       //  Code
    OP_BOF,             //    0
    OP_EOF,             //    1
    NI,                 //    2
    NI,                 //    3
    NI,                 //    4
    NI,                 //    5
    NI,                 //    6
    OP_Window1,         //    7
    OP_ColumnWidth,     //    8
    NI,                 //    9
    NI,                 //   10
    OP_NamedRange,      //   11
    OP_Blank,           //   12
    OP_Integer,         //   13
    OP_Number,          //   14
    OP_Label,           //   15
    OP_Formula,         //   16
    NI,                 //   17
    NI,                 //   18
    NI,                 //   19
    NI,                 //   20
    NI,                 //   21
    NI,                 //   22
    NI,                 //   23
    NI,                 //   24
    NI,                 //   25
    NI,                 //   26
    NI,                 //   27
    NI,                 //   28
    NI,                 //   29
    NI,                 //   30
    NI,                 //   31
    NI,                 //   32
    NI,                 //   33
    NI,                 //   34
    NI,                 //   35
    NI,                 //   36
    OP_Footer,          //   37
    OP_Header,          //   38
    NI,                 //   39
    OP_Margins,         //   40
    NI,                 //   41
    NI,                 //   42
    NI,                 //   43
    NI,                 //   44
    NI,                 //   45
    NI,                 //   46
    NI,                 //   47
    NI,                 //   48
    NI,                 //   49
    NI,                 //   50
    NI,                 //   51
    NI,                 //   52
    NI,                 //   53
    NI,                 //   54
    NI,                 //   55
    NI,                 //   56
    NI,                 //   57
    NI,                 //   58
    NI,                 //   59
    NI,                 //   60
    NI,                 //   61
    NI,                 //   62
    NI,                 //   63
    NI,                 //   64
    NI,                 //   65
    NI,                 //   66
    NI,                 //   67
    NI,                 //   68
    NI,                 //   69
    NI,                 //   70
    OP_SymphNamedRange, //   71
    NI,                 //   72
    NI,                 //   73
    NI,                 //   74
    NI,                 //   75
    NI,                 //   76
    NI,                 //   77
    NI,                 //   78
    NI,                 //   79
    NI,                 //   80
    NI,                 //   81
    NI,                 //   82
    NI,                 //   83
    NI,                 //   84
    NI,                 //   85
    NI,                 //   86
    NI,                 //   87
    NI,                 //   88
    NI,                 //   89
    NI,                 //   90
    NI,                 //   91
    NI,                 //   92
    NI,                 //   93
    NI,                 //   94
    NI,                 //   95
    NI,                 //   96
    NI,                 //   97
    NI,                 //   98
    NI,                 //   99
    OP_HiddenCols,      //  100
};

const OPCODE_FKT LotusContext::pOpFkt123[ FKT_LIMIT123 ] =
{                       //  Code
    OP_BOF123,              //    0
    OP_EOF123,              //    1
    NI,                 //    2
    NI,                 //    3
    NI,                 //    4
    NI,                 //    5
    NI,                 //    6
    NI,         //    7
    NI,     //    8
    NI,                 //    9
    NI,                 //   10
    NI,     //   11
    NI,         //   12
    NI,         //   13
    NI,         //   14
    NI,         //   15
    NI,         //   16
    NI,                 //   17
    NI,                 //   18
    NI,                 //   19
    NI,                 //   20
    NI,                 //   21
    OP_Label123,                //   22
    NI,                 //   23
    NI,                 //   24
    NI,                 //   25
    NI,                 //   26
    OP_CreatePattern123,            //   27
    NI,                 //   28
    NI,                 //   29
    NI,                 //   30
    NI,                 //   31
    NI,                 //   32
    NI,                 //   33
    NI,                 //   34
    OP_SheetName123,    //   35
    NI,                 //   36
    OP_Number123,                   //   37
    OP_Note123,         //   38
    OP_IEEENumber123,                   //   39
    OP_Formula123,          //   40
    NI,                 //   41
    NI,                 //   42
    NI,                 //   43
    NI,                 //   44
    NI,                 //   45
    NI,                 //   46
    NI,                 //   47
    NI,                 //   48
    NI,                 //   49
    NI,                 //   50
    NI,                 //   51
    NI,                 //   52
    NI,                 //   53
    NI,                 //   54
    NI,                 //   55
    NI,                 //   56
    NI,                 //   57
    NI,                 //   58
    NI,                 //   59
    NI,                 //   60
    NI,                 //   61
    NI,                 //   62
    NI,                 //   63
    NI,                 //   64
    NI,                 //   65
    NI,                 //   66
    NI,                 //   67
    NI,                 //   68
    NI,                 //   69
    NI,                 //   70
    NI, //   71
    NI,                 //   72
    NI,                 //   73
    NI,                 //   74
    NI,                 //   75
    NI,                 //   76
    NI,                 //   77
    NI,                 //   78
    NI,                 //   79
    NI,                 //   80
    NI,                 //   81
    NI,                 //   82
    NI,                 //   83
    NI,                 //   84
    NI,                 //   85
    NI,                 //   86
    NI,                 //   87
    NI,                 //   88
    NI,                 //   89
    NI,                 //   90
    NI,                 //   91
    NI,                 //   92
    NI,                 //   93
    NI,                 //   94
    NI,                 //   95
    NI,                 //   96
    NI,                 //   97
    NI,                 //   98
    NI,                 //   99
    NI      //  100
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
