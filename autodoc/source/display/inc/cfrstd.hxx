/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfrstd.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:54:03 $
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

#ifndef ADC_CFRSTD_HXX
#define ADC_CFRSTD_HXX



// USED SERVICES
    // BASE CLASSES
#include <display/corframe.hxx>
    // COMPONENTS
    // PARAMETERS



class StdFrame : public display::CorporateFrame
{
  public:
    // LIFECYCLE
                        StdFrame();

    // INQUIRY
    virtual DYN Html_Image *
                        LogoSrc() const;
    virtual const char *
                        LogoLink() const;
    virtual const char *
                        CopyrightText() const;
    virtual const char *
                        CssStyle() const;
    virtual const char *
                        CssStylesExplanation() const;
    virtual const char *
                        DevelopersGuideHtmlRoot() const;
    virtual bool        SimpleLinks() const;

    // ACCESS
    virtual void        Set_DevelopersGuideHtmlRoot(
                            const String &      i_directory );
    virtual void        Set_SimpleLinks();

  private:
    String              sDevelopersGuideHtmlRoot;
    bool                bSimpleLinks;
};



// IMPLEMENTATION




#endif

