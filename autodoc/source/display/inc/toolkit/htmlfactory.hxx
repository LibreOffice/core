/*************************************************************************
 *
 *  $RCSfile: htmlfactory.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:15:18 $
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

#ifndef ADC_DISPLAY_HTMLFACTORY_HXX
#define ADC_DISPLAY_HTMLFACTORY_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "outputstack.hxx"
    // PARAMETERS
#include <udm/xml/xmlitem.hxx>
#include <udm/html/htmlitem.hxx>

namespace Xml = ::csi::xml;
namespace Html = ::csi::html;

/** @resp
    Base class for HTML page creators (factories) for code entites or
    similar items.
*/
template <class ENV>
class HtmlFactory
{
  public:
    // INQUIRY
    ENV &               Env() const             { return *pEnv; }
    Xml::Element &      CurOut() const          { return aDestination.Out(); }

    // ACCESS
    OutputStack &       Out() const             { return aDestination; }

  protected:
                        HtmlFactory(
                            ENV &               io_rEnv,
                            Xml::Element *      o_pOut = 0 )
                            :   pEnv(&io_rEnv)  { if (o_pOut != 0) aDestination.Enter(*o_pOut); }
  private:
    // DATA
    ENV *               pEnv;
    mutable OutputStack aDestination;
};


/** @resp
    Base class for HTML paragraph creators, which are to be put into
    a parent HTML element.
*/
class HtmlMaker
{
  public:

    // INQUIRY
    Xml::Element &      CurOut() const          { return *pOut; }

  protected:
                        HtmlMaker(
                            Xml::Element &      o_rOut )
                            :   pOut(&o_rOut)   {}
  private:
    // DATA
    Xml::Element *      pOut;
};




// IMPLEMENTATION




#endif


