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



#ifndef ADC_CFRSTD_HXX
#define ADC_CFRSTD_HXX



// USED SERVICES
    // BASE CLASSES
#include <display/corframe.hxx>
    // COMPONENTS
    // PARAMETERS



class StdFrame : public display::CorporateFrame
{
  public:
    // LIFECYCLE
                        StdFrame();

    // INQUIRY
    virtual DYN Html_Image *
                        LogoSrc() const;
    virtual const char *
                        LogoLink() const;
    virtual const char *
                        CopyrightText() const;
    virtual const char *
                        CssStyle() const;
    virtual const char *
                        CssStylesExplanation() const;
    virtual const char *
                        DevelopersGuideHtmlRoot() const;
    virtual bool        SimpleLinks() const;

    // ACCESS
    virtual void        Set_DevelopersGuideHtmlRoot(
                            const String &      i_directory );
    virtual void        Set_SimpleLinks();

  private:
    String              sDevelopersGuideHtmlRoot;
    bool                bSimpleLinks;
};



// IMPLEMENTATION




#endif

