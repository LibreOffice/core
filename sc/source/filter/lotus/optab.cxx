/*************************************************************************
 *
 *  $RCSfile: optab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop


#include "op.h"
#include "optab.h"


// Bearbeitungsfunktion sal_Char *X( sal_Char * )
OPCODE_FKT pOpFkt[ FKT_LIMIT ] =
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


