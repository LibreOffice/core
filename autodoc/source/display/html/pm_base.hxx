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



#ifndef ADC_DISPLAY_PM_BASE_HXX
#define ADC_DISPLAY_PM_BASE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include "hdimpl.hxx"



class OuputPage_Environment;
namespace csi
{
    namespace xml
    {
        class Element;
    }
}
class PageDisplay;


/** Interface for making a special kind of HTML-Page
*/
class SpecializedPageMaker
{
  public:
    virtual             ~SpecializedPageMaker() {}

    virtual void        MakePage() = 0;

    virtual void        Write_NavBar();
    virtual void        Write_TopArea();
    virtual void        Write_DocuArea();
//    virtual void        Write_ChildList(
//                            ary::SlotAccessId   i_nSlot,
//                            const char *        i_nListTitle,
//                            const char *        i_nLabel );

  protected:
                        SpecializedPageMaker(
                            PageDisplay &       io_rPage );

    OuputPage_Environment &
                        Env() const             { return *pEnv; }
    csi::xml::Element & CurOut();
    PageDisplay &       Page()                  { return *pPage; }

  private:
    OuputPage_Environment *
                        pEnv;
    csi::xml::Element * pCurOut;
    PageDisplay *       pPage;
};



#endif

