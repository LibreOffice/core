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

/// @HTML
#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_FIELDS_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_FIELDS_HXX

namespace ww
{
    enum eField
    {
        eNONE = 0,
        eUNKNOWN = 1,
        ePOSSIBLEBOOKMARK = 2,
        eREF = 3,
        eXE = 4,
        eFOOTREF = 5,
        eSET = 6,
        eIF = 7,
        eINDEX = 8,
        eTC = 9,
        eSTYLEREF = 10,
        eRD = 11,
        eSEQ = 12,
        eTOC = 13,
        eINFO = 14,
        eTITLE = 15,
        eSUBJECT = 16,
        eAUTHOR = 17,
        eKEYWORDS = 18,
        eCOMMENTS = 19,
        eLASTSAVEDBY = 20,
        eCREATEDATE = 21,
        eSAVEDATE = 22,
        ePRINTDATE = 23,
        eREVNUM = 24,
        eEDITTIME = 25,
        eNUMPAGE = 26,
        eNUMWORDS = 27,
        eNUMCHARS = 28,
        eFILENAME = 29,
        eTEMPLATE = 30,
        eDATE = 31,
        eTIME = 32,
        ePAGE = 33,
        eEquals = 34,
        eQUOTE = 35,
        eMERGEINC = 36,
        ePAGEREF = 37,
        eASK = 38,
        eFILLIN = 39,
        eMERGEDATA = 40,
        eNEXT = 41,
        eNEXTIF = 42,
        eSKIPIF = 43,
        eMERGEREC = 44,
        eDDEREF = 45,
        eDDEAUTOREF = 46,
        eGLOSSREF = 47,
        ePRINT = 48,
        eEQ = 49,
        eGOTOBUTTON = 50,
        eMACROBUTTON = 51,
        eAUTONUMOUT = 52,
        eAUTONUMLGL = 53,
        eAUTONUM = 54,
        eINCLUDETIFF = 55,
        eLINK = 56,
        eSYMBOL = 57,
        eEMBED = 58,
        eMERGEFIELD = 59,
        eUSERNAME = 60,
        eUSERINITIALS = 61,
        eUSERADDRESS = 62,
        eBARCODE = 63,
        eDOCVARIABLE = 64,
        eSECTION = 65,
        eSECTIONPAGES = 66,
        eINCLUDEPICTURE = 67,
        eINCLUDETEXT = 68,
        eFILESIZE = 69,
        eFORMTEXT = 70,
        eFORMCHECKBOX = 71,
        eNOTEREF = 72,
        eTOA = 73,
        eTA = 74,
        eMERGESEQ = 75,
        eMACRO = 76,
        ePRIVATE = 77,
        eDATABASE = 78,
        eAUTOTEXT = 79,
        eCOMPARE = 80,
        ePLUGIN = 81,
        eSUBSCRIBER = 82,
        eFORMDROPDOWN = 83,
        eADVANCE = 84,
        eDOCPROPERTY = 85,
        eUNKNOWN2 = 86,
        eCONTROL = 87,
        eHYPERLINK = 88,
        eAUTOTEXTLIST = 89,
        eLISTNUM = 90,
        eHTMLCONTROL = 91,
        eBIDIOUTLINE = 92,
        eADDRESSBLOCK = 93,
        eGREETINGLINE = 94,
        eSHAPE = 95,
        // NOTE: values > 95 / 0x5F do not correspond to documented WW8 fields
        // and thus need special handling in WW8Export::OutputField()!
        eBIBLIOGRPAHY=96,
        eCITATION = 97
    };

    /** Find the English Field Name from a winword index

        See OpenOffice.org issue 12831
        (http://www.openoffice.org/issues/show_bug.cgi?id=12831) and MS
        Knowledge Base article 268717
        (http://support.microsoft.com/default.aspx?scid=kb;en-us;268717) for
        details of why to use english field names and not localized ones since
        Word 2000.

        @param
        nIndex the index to search for

        @return 0 if not found, otherwise the fieldname as a C style ASCII
        string

        @author
        <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
    */
    const char *GetEnglishFieldName(eField eIndex) throw();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
