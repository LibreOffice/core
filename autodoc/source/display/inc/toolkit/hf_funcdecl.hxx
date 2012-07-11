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

#ifndef ADC_DISPLAY_HF_FUNCDECL_HXX
#define ADC_DISPLAY_HF_FUNCDECL_HXX


// USED SERVICES
    // BASE CLASSES
#include <toolkit/htmlfactory.hxx>
    // COMPONENTS
    // PARAMETERS

class HF_FunctionDeclaration : public HtmlMaker
{
  public:
                        HF_FunctionDeclaration(
                            Xml::Element &      o_rParent,
                            const String &      i_sRaisesText );
    virtual             ~HF_FunctionDeclaration();

    // OPERATIONS
    Xml::Element &      ReturnCell();
    Xml::Element &      NameCell();
    Xml::Element &      NewParamTypeCell();
    Xml::Element &      ParamNameCell();
    Xml::Element &      ExceptionCell();

  private:
    Html::TableRow &    ParameterLine();

    // DATA
    String              sRaisesText;
    Html::Table *       pTable;
    Xml::Element *      pReturnCell;
    Xml::Element *      pNameCell;
    Html::TableRow *    pParameterLine;
    Xml::Element *      pLastParameterCell;
    Xml::Element *      pExceptionCell;
};


// IMPLEMENTATION



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
