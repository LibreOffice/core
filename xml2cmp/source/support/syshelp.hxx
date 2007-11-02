/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: syshelp.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:55:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef X2C_SYSHELP_HXX
#define X2C_SYSHELP_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <iosfwd>

class Simstr;
template <class XY> class List;


#if defined(WNT) || defined(OS2)
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
                        std::ostream &      o_rFile,
                        const Simstr &      i_rIdlDocuBaseDir,
                        const Simstr &      i_rName,
                        E_LinkType          i_eLinkType );


void                WriteStr(
                        std::ostream &      o_rFile,
                        const char *        i_sStr );
void                WriteStr(
                        std::ostream &      o_rFile,
                        const Simstr &      i_sStr );

void                GatherFileNames(
                        List<Simstr> &      o_sFiles,
                        const char *        i_sSrcDirectory );
void                GatherSubDirectories(
                        List<Simstr> &      o_sSubDirectories,
                        const char *        i_sParentdDirectory );



#endif

