/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outfile.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:30:09 $
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

    void                SetLocation(
                            const csv::ploc::Path &
                                                i_rFilePath,
                            uintt               i_depthInOutputTree );
    void                SetTitle(
                            const char *        i_sTitle );
    void                SetBodyAttr(
                            const char *        i_sAttrName,
                            const char *        i_sAttrValue );
    void                SetCopyright(
                            const char *        i_sCopyright );
    void                EmptyBody();

    Html::Body &        Body()                  { return aBodyData; }
    bool                CreateFile();

    static void         WriteCssFile(
                            const csv::ploc::Path &
                                                i_rFilePath );
  private:
    void                WriteHeader(
                            csv::File &         io_aFile );
    void                WriteBody(
                            csv::File &         io_aFile );

    // DATA
    String              sFilePath;
    String              sTitle;
    String              sLocation;
    String              sCopyright;
    uintt               nDepthInOutputTree;

    Html::Body          aBodyData;
    StreamStr           aBuffer;                // Output buffer, should be transfered into csv::File.
};




#endif
