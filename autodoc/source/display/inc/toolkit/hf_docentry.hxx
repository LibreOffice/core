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

#ifndef ADC_DISPLAY_HF_DOCENTRY_HXX
#define ADC_DISPLAY_HF_DOCENTRY_HXX



// USED SERVICES
    // BASE CLASSES
#include "htmlfactory.hxx"
    // COMPONENTS
    // PARAMETERS

/** @resp
    Produces a list of <DT>..</DT> and <DD>.
*/
class HF_DocEntryList : public HtmlMaker
{
  public:

                        HF_DocEntryList(
                            Xml::Element &      o_rOut );
    virtual             ~HF_DocEntryList();

    Xml::Element &      Produce_Term(
                            const char *        i_sTerm = 0 );
    Xml::Element &      Produce_NormalTerm(     /// Font will not be bold.
                            const char *        i_sTerm = 0 );
    Xml::Element &      Produce_Definition();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
