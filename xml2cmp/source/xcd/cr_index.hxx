/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cr_index.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:01:41 $
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
                            std::ostream &      o_rOut,
                            Heap &              i_rHeap,
                            const char *        i_sIndexKey,
                            const char *        i_sIndexReference,
                            E_LinkType          i_eLinkType );
    void                WriteHeap(
                            std::ostream &      o_rOut,
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


