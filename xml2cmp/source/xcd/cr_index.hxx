/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
