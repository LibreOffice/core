/*************************************************************************
 *
 *  $RCSfile: fields.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-25 07:40:29 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/// @HTML
#ifndef WW_FIELDS_HXX
#define WW_FIELDS_HXX

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
        eSHAPE = 95
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
};

#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
