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
