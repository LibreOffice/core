/*************************************************************************
#*
#*    $Workfile:        syshelp.hxx  $
#*
#*    class
#*
#*    Beschreibung
#*
#*    Ersterstellung    NP
#*    Letzte Aenderung  $Author: np $
#*                      $Date: 2001-03-09 15:22:59 $
#*    $Revision: 1.1 $
#*
#*    $Logfile:         T:/solar/source/...  $
#*
#*    Copyright (c) 2000, Star Office Gmbh
#*
#**************************************************************************/

#ifndef X2C_SYSHELP_HXX
#define X2C_SYSHELP_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
class ofstream;
class Simstr;



#ifdef WNT
const char   C_sSLASH[] = "\\";
const char   C_cSLASH = '\\';
#elif defined(UNX)
const char   C_sSLASH[] = "/";
const char   C_cSLASH = '/';
#else
#error Must run under unix or windows, please define UNX or WNT.
#endif

enum E_LinkType
{
    lt_nolink = 0,
    lt_idl,
    lt_html
};


void                WriteName(
                        ofstream &          o_rFile,
                        const Simstr &      i_rIdlDocuBaseDir,
                        const Simstr &      i_rName,
                        E_LinkType          i_eLinkType );


void                WriteStr(
                        ofstream &          o_rFile,
                        const char *        i_sStr );
void                WriteStr(
                        ofstream &          o_rFile,
                        const Simstr &      i_sStr );


#endif

