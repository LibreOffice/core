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



#ifndef ADC_DISPLAY_HF_FUNCDECL_HXX
#define ADC_DISPLAY_HF_FUNCDECL_HXX


// USED SERVICES
    // BASE CLASSES
#include <toolkit/htmlfactory.hxx>
    // COMPONENTS
    // PARAMETERS


#if 0   // old
/** @resp
    Provides three cells to put in a function declaration.
*/
class HF_FunctionDeclaration : public HtmlMaker
{
  public:
                        HF_FunctionDeclaration(
                            Xml::Element &      o_rParent );
    virtual             ~HF_FunctionDeclaration();

    /// Inserts empty line in 2nd and 3rd cell and returns first.
    Xml::Element &      Add_ReturnLine();

    /** Inserts empty line in 1st cell, "raises (" in 2nd
        and returns 3rd.
    */
    Xml::Element &      Add_RaisesLine(
                            const char *        i_sRaisesText,
                            bool                i_bSuppressExtraLine = false );

    Xml::Element &      Front()                 { return *pFront; }
    Xml::Element &      Types()                 { return *pTypes; }
    Xml::Element &      Names()                 { return *pNames; }

  private:
    Xml::Element *      pFront;
    Xml::Element *      pTypes;
    Xml::Element *      pNames;
};
#endif // 0  old

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
