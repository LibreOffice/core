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
