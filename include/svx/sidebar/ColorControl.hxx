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
#include "svx/sidebar/PopupControl.hxx"

#include <svtools/valueset.hxx>
#include <boost/function.hpp>
#include <svx/SvxColorValueSet.hxx>

class Window;
class SfxBindings;
class RedId;
class FloatingWindow;


namespace svx { namespace sidebar {

/** The ColorControl uses a ValueSet control for displaying all named
    colors in a matrix.
*/
class SVX_DLLPUBLIC ColorControl
    : public PopupControl
{
public:
    /** Create a new ColorControl object.
        @param rControlResId
            The resource id for the whole color control.
        @param rNoColorGetter
            A functor for getting the color which will be returned when the
            WB_NONEFIELD is used and got selected
        @param rColorSetter
            A functor for setting the color that is selected by the
            user.
        @param pNoColorStringResId
            Resource id of an optional string for the "no color"
            string.  When a value is given then a
            field/button is created above the color matrix for
            selecting "no color" ie. transparent.
            When zero is given then no such field is created.
    */
    ColorControl (
        Window* pParent,
        SfxBindings* pBindings,
        const ResId& rControlResId,
        const ResId& rValueSetResId,
        const ::boost::function<Color(void)>& rNoColorGetter,
        const ::boost::function<void(OUString&,Color)>& rColorSetter,
        FloatingWindow* pFloatingWindow,
        const ResId* pNoColorStringResId);
    virtual ~ColorControl (void);

    void GetFocus (void);
    void SetCurColorSelect (
        const Color aCol,
        const bool bAvl);

private:
    SvxColorValueSet maVSColor;
    FloatingWindow* mpFloatingWindow;
    const OUString msNoColorString;
    ::boost::function<Color(void)> maNoColorGetter;
    ::boost::function<void(OUString&,Color)> maColorSetter;

    void FillColors (void);
    DECL_LINK(VSSelectHdl, void *);
};

} } // end of namespace svx::sidebar
