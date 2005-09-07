/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlfile.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:57:18 $
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
    void                SetInlineStyle(
                            const char *        i_sStyle );
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


