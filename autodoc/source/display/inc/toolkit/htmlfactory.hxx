/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
                        ~HtmlFactory() {}
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


