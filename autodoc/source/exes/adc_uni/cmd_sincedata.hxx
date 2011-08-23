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

#ifndef ADC_CMD_SINCEDATA_HXX
#define ADC_CMD_SINCEDATA_HXX



// USED SERVICES
    // BASE CLASSES
#include "adc_cmd.hxx"
    // COMPONENTS
    // PARAMETERS

namespace autodoc
{
namespace command
{


/** Provides replacements for the contents of the @@since documentation tag.

    Because the @@since tag is part of the source code, it allows only one kind
    of version information there. If this is to be mapped for different products
    (example: from OpenOffice.org versions in the @@since tag to StarOffice or
    StarSuite products), the value of @@since needs a replacement, which is provided
    by this class.

*/
class SinceTagTransformationData : public Context
{
  public:
    /** The key of this map are the version numbers within @since.
        The value is the string to display for each version number.
    */
    typedef std::map<String,String>     Map_Version2Display;

    // LIFECYCLE
                        SinceTagTransformationData();
    virtual             ~SinceTagTransformationData();

    // INQUIRY
    /// False, if no transformation table exists.
    bool                DoesTransform() const;

    /** Gets the string to display for a version number.

        @param i_sVersionNumber
        Usually should be the result of ->StripSinceTagValue().
    */
    const String &      DisplayOf(
                            const String &      i_sVersionNumber ) const;
  private:
    // Interface Context:
    virtual void        do_Init(
                            opt_iter &          i_nCurArgsBegin,
                            opt_iter            i_nEndOfAllArgs );
    // DATA
    Map_Version2Display aTransformationTable;
};


}   // namespace command
}   // namespace autodoc


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
