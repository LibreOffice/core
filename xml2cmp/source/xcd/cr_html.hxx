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
