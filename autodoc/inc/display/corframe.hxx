/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: corframe.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:28:13 $
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

#ifndef ADC_CORFRAME_HXX
#define ADC_CORFRAME_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

class Html_Image;


namespace display
{


class CorporateFrame
{
  public:
    // LIFECYCLE
    virtual             ~CorporateFrame() {}

    // INQUIRY
    virtual DYN Html_Image *
                        LogoSrc() const = 0;
    virtual const char *
                        LogoLink() const = 0;
    virtual const char *
                        CopyrightText() const = 0;

    virtual const char *
                        CssStyle() const = 0;
    virtual const char *
                        CssStylesExplanation() const = 0;
    virtual const char *
                        DevelopersGuideHtmlRoot() const = 0;
    virtual bool        SimpleLinks() const = 0;

    // ACCESS
    virtual void        Set_DevelopersGuideHtmlRoot(
                            const String &      i_directory ) = 0;
    virtual void        Set_SimpleLinks() = 0;
};



// IMPLEMENTATION


}   // namespace display


#endif

