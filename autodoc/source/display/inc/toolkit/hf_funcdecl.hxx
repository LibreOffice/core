/*************************************************************************
 *
 *  $RCSfile: hf_funcdecl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 13:35:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
