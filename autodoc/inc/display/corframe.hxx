/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
