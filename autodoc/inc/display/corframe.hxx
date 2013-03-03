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

    // ACCESS
    virtual void        Set_DevelopersGuideHtmlRoot(
                            const String &      i_directory ) = 0;
};



// IMPLEMENTATION


}   // namespace display


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
