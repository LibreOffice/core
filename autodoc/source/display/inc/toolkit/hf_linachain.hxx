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



#ifndef ADC_DISPLAY_HF_LINACHAIN_HXX
#define ADC_DISPLAY_HF_LINACHAIN_HXX

// BASE CLASSES
#include "htmlfactory.hxx"
#include "out_position.hxx"




class HF_LinkedNameChain : public HtmlMaker
{
  public:
    /** F_LinkMaker makes a link out of the name of the
        parent position.

        Returns true, if there is a link, false if not.
    */
    typedef String (*F_LinkMaker)(const char *);


                        HF_LinkedNameChain(
                            Xml::Element &      o_rOut );
    virtual             ~HF_LinkedNameChain();

    void                Produce_CompleteChain(
                            const output::Position &
                                                i_curPosition,
                            F_LinkMaker         i_linkMaker ) const;
    void                Produce_CompleteChain_forModule(
                            const output::Position &
                                                i_curPosition,  /// current Module's node
                            F_LinkMaker         i_linkMaker ) const;
  private:
    void                produce_Level(
                            output::Node &      i_levelNode,
                            const output::Position &
                                                i_startPosition,
                            F_LinkMaker         i_linkMaker ) const;
};




#endif
