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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
