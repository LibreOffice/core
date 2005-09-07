/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlfactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:56:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


