/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    StreamStr           aBuffer;                // Output buffer, should be transferred into csv::File.
};




#endif
