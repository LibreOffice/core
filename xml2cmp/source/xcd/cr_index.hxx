/*************************************************************************
 *
 *  $RCSfile: cr_index.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: np $ $Date: 2001-03-23 13:24:04 $
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

#ifndef X2C_CR_INDEX_HXX
#define X2C_CR_INDEX_HXX

#include "../support/sistr.hxx"
#include "../support/heap.hxx"
#include "../support/list.hxx"
#include "../support/syshelp.hxx"


class ModuleDescription;


class Index
{
  public:
                        Index(
                            const char  *       i_sOutputDirectory,
                            const char *        i_sIdlRootPath,
                            const List<Simstr> &
                                                i_rTagList );
                        ~Index();

    void                GatherData(
                            const List<Simstr> &
                                                i_rInputFileList );
    void                WriteOutput(
                            const char *        i_sOuputFile );

    void                InsertSupportedService(
                            const Simstr &      i_sService );
  private:
    void                ReadFile(
                            const char *        i_sFilename );
    void                CreateHtmlFileName(
                            char *              o_sOutputHtml,
                            const ModuleDescription &
                                                i_rModule );
    void                WriteTableFromHeap(
                            std::ofstream &     o_rOut,
                            Heap &              i_rHeap,
                            const char *        i_sIndexKey,
                            const char *        i_sIndexReference,
                            E_LinkType          i_eLinkType );
    void                WriteHeap(
                            std::ofstream &     o_rOut,
                            Heap &              i_rHeap,
                            E_LinkType          i_eLinkType );

    // DATA
    Heap                aService2Module;
    Heap                aModule2Service;

    Simstr              sOutputDirectory;
    Simstr              sIdlRootPath;

    // Temporary Data
    Simstr              sCurModule;
};




#endif


