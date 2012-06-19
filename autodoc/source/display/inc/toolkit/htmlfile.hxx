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

#ifndef ADC_DISPLAY_HTMLFILE_HXX
#define ADC_DISPLAY_HTMLFILE_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <udm/html/htmlitem.hxx>
    // PARAMETERS
#include <cosv/ploc.hxx>

namespace csv
{
     class File;
}

/** Represents an HTML output file.
*/
class DocuFile_Html
{
  public:
    // LIFECYCLE
                        DocuFile_Html();

    void                SetLocation(
                            const csv::ploc::Path &
                                                i_rFilePath );
    void                SetTitle(
                            const char *        i_sTitle );
    void                SetRelativeCssPath(
                            const char *        i_sCssFile_relativePath );
    void                SetCopyright(
                            const char *        i_sCopyright );
    void                EmptyBody();

    Html::Body &        Body()                  { return aBodyData; }
    bool                CreateFile();

  private:
    void                WriteHeader(
                            csv::File &         io_aFile );
    void                WriteBody(
                            csv::File &         io_aFile );
    // DATA
    String              sFilePath;
    String              sTitle;
    String              sLocation;
    String              sStyle;
    String              sCssFile;
    String              sCopyright;

    Html::Body          aBodyData;
    StreamStr           aBuffer;
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
