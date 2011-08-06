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

#ifndef X2C_CR_HTML_HXX
#define X2C_CR_HTML_HXX


#include <string.h>
#include <fstream>
#include "../support/sistr.hxx"
#include "../support/syshelp.hxx"


class XmlElement;
class SglTextElement;
class MultipleTextElement;


class HtmlCreator
{
  public:
                        HtmlCreator(
                            const char *        i_pOutputFileName,
                            const XmlElement &  i_rDocument,
                            const Simstr &      i_sIDL_BaseDirectory );
                        ~HtmlCreator();

    void                Run();

    void                StartTable();
    void                FinishTable();
    void                StartBigCell(
                            const char *        i_sTitle );
    void                FinishBigCell();

    void                Write_SglTextElement(
                            const SglTextElement &
                                                i_rElement,
                            bool                i_bStrong = false );
    void                Write_MultiTextElement(
                            const MultipleTextElement &
                                                i_rElement );
    void                Write_SglText(
                            const Simstr &      i_sName,
                            const Simstr &      i_sValue );
    void                Write_ReferenceDocu(
                            const Simstr &      i_sName,
                            const Simstr &      i_sRef,
                            const Simstr &      i_sRole,
                            const Simstr &      i_sTitle );
  private:
    void                StartRow();
    void                FinishRow();
    void                StartCell(
                            const char *        i_pWidth );
    void                FinishCell();

    void                WriteElementName(
                            const Simstr &      i_sName,
                            bool                i_bStrong );
    void                WriteStr(
                            const char *        i_sStr )
                                                { aFile.write( i_sStr, (int) strlen(i_sStr) ); }
    // DATA
    std::ofstream       aFile;
    const XmlElement &  rDocument;
    Simstr              sIdl_BaseDirectory;
};




#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
