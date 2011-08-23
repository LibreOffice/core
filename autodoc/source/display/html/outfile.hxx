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

#ifndef ADC_DISPLAY_HTML_OUTFILE_HXX
#define ADC_DISPLAY_HTML_OUTFILE_HXX

// USED SERVICES
#include <udm/html/htmlitem.hxx>
#include <cosv/ploc.hxx>


namespace csv
{
    class File;
}




class HtmlDocuFile
{
  public:
    // LIFECYCLE
                        HtmlDocuFile();

    void				SetLocation(
                            const csv::ploc::Path &
                                                i_rFilePath,
                            uintt               i_depthInOutputTree );
    void                SetTitle(
                            const char *        i_sTitle );
    void                SetCopyright(
                            const char *        i_sCopyright );
    void                EmptyBody();

    Html::Body &  		Body()					{ return aBodyData; }
    bool          		CreateFile();

    static void         WriteCssFile(
                            const csv::ploc::Path &
                                                i_rFilePath );
  private:
    void	 			WriteHeader(
                            csv::File &			io_aFile );
    void				WriteBody(
                            csv::File &			io_aFile );

    // DATA
    String 				sFilePath;
    String  			sTitle;
    String  			sLocation;
    String              sCopyright;
    uintt               nDepthInOutputTree;

    Html::Body			aBodyData;
    StreamStr           aBuffer;                // Output buffer, should be transfered into csv::File.
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
