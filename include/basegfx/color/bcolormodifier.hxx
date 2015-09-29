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

#ifndef INCLUDED_BASEGFX_COLOR_BCOLORMODIFIER_HXX
#define INCLUDED_BASEGFX_COLOR_BCOLORMODIFIER_HXX

#include <basegfx/basegfxdllapi.h>
#include <basegfx/color/bcolor.hxx>

#include <osl/diagnose.h>

#include <memory>
#include <vector>

namespace basegfx
{
    /** base class to define color modifications

        The basic idea is to have instances of color modifiers where each
        of these can be asked to get a modified version of a color. This
        can be as easy as to return a fixed color, but may also do any
        other computation based on the given source color and the local
        algorithm to apply.

        This base implementation defines the abstract base class. Every
        derivation offers another color blending effect, when needed with
        parameters for that blending defined as members.

        As long as aw080 is not applied, an operator== is needed to implement
        the operator== of the primitive based on this instances.

        For the exact definitions of the color blending applied refer to the
        implementation of the method getModifiedColor

        BColorModifier is not copyable (no copy constructor, no assigment
        operator); local values cannot be changed after construction. The
        instances are cheap and the idea is to create them on demand. To
        be able to reuse these as much as possible, a define for a
        std::shared_ptr named BColorModifierSharedPtr exists below.
        All usages should handle instances of BColorModifier encapsulated
        into these shared pointers.
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier
    {
    private:
        BColorModifier(const BColorModifier&) SAL_DELETED_FUNCTION;
        BColorModifier& operator=(const BColorModifier&) SAL_DELETED_FUNCTION;
    protected:
        // no one is allowed to incarnate the abstract base class
        // except derivations
        BColorModifier() {}

    public:
        // no one should directly destroy it; all incarnations should be
        // handled in a std::shared_ptr of type BColorModifierSharedPtr
        virtual ~BColorModifier();

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const = 0;
        bool operator!=(const BColorModifier& rCompare) const
        {
            return !(operator==(rCompare));
        }

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const = 0;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** convert color to gray
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_gray : public BColorModifier
    {
    private:
    protected:
    public:
        BColorModifier_gray()
        :   BColorModifier()
        {
        }

        virtual ~BColorModifier_gray();

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** invert color

        returns a color where red green and blue are inverted using 1.0 - n
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_invert : public BColorModifier
    {
    private:
    protected:
    public:
        BColorModifier_invert()
        :   BColorModifier()
        {
        }

        virtual ~BColorModifier_invert();

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** convert to alpha based on luminance

        returns a color where red green and blue are first weighted and added
        to build a luminance value which is then inverted and used for red,
        green and blue. The weights are  r * 0.2125 + g * 0.7154 + b * 0.0721.
        This derivation is used for the svg importer and does exactly what SVG
        defines for this needed case.
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_luminance_to_alpha : public BColorModifier
    {
    private:
    protected:
    public:
        BColorModifier_luminance_to_alpha()
        :   BColorModifier()
        {
        }

        virtual ~BColorModifier_luminance_to_alpha();

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** replace color

        does not use the source color at all, but always returns the
        given color, replacing everything. Useful e.g. for unified shadow
        creation
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_replace : public BColorModifier
    {
    private:
        ::basegfx::BColor           maBColor;

    protected:
    public:
        BColorModifier_replace(const ::basegfx::BColor& rBColor)
        :   BColorModifier(),
            maBColor(rBColor)
        {
        }

        virtual ~BColorModifier_replace();

        // data access
        const ::basegfx::BColor& getBColor() const { return maBColor; }

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** interpolate color

        returns an interpolated color mixed by the given value (f) in the range
        [0.0 .. 1.0] and the given color (col) as follows:

        col * (1 - f) + aSourceColor * f
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_interpolate : public BColorModifier
    {
    private:
        ::basegfx::BColor           maBColor;
        double                      mfValue;

    protected:
    public:
        BColorModifier_interpolate(const ::basegfx::BColor& rBColor, double fValue)
        :   BColorModifier(),
            maBColor(rBColor),
            mfValue(fValue)
        {
        }

        virtual ~BColorModifier_interpolate();

        // data access
        const ::basegfx::BColor& getBColor() const { return maBColor; }
        double getValue() const { return mfValue; }

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** convert color to black and white

        returns black when the luminance of the given color is less than
        the given treshhold value in the range [0.0 .. 1.0], else white
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_black_and_white : public BColorModifier
    {
    private:
        double                      mfValue;

    protected:
    public:
        BColorModifier_black_and_white(double fValue)
        :   BColorModifier(),
            mfValue(fValue)
        {
        }

        virtual ~BColorModifier_black_and_white();

        // data access
        double getValue() const { return mfValue; }

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** gamma correction

        Input is a gamma correction value in the range ]0.0 .. 10.0]; the
        color values get corrected using

        col(r,g,b) = clamp(pow(col(r,g,b), 1.0 / gamma), 0.0, 1.0)
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_gamma : public BColorModifier
    {
    private:
        double                      mfValue;
        double                      mfInvValue;

        /// bitfield
        bool                        mbUseIt : 1;

    protected:
    public:
        BColorModifier_gamma(double fValue);

        virtual ~BColorModifier_gamma();

        // data access
        double getValue() const { return mfValue; }

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /** Red, Green, Blue, Luminance and Contrast correction

        Input are percent values from [-1.0 .. 1-0] which correspond to -100% to 100%
        correction of Red, Green, Blue, Luminance or Contrast. 0.0 means no change of
        the corresponding channel. All these are combined (but can be used single) to
        - be able to cover a bigger change range utilizing the combination
        - allow execution by a small, common, precalculated table
    */
    class BASEGFX_DLLPUBLIC SAL_WARN_UNUSED BColorModifier_RGBLuminanceContrast : public BColorModifier
    {
    private:
        double                      mfRed;
        double                      mfGreen;
        double                      mfBlue;
        double                      mfLuminance;
        double                      mfContrast;

        double                      mfContrastOff;
        double                      mfRedOff;
        double                      mfGreenOff;
        double                      mfBlueOff;

        /// bitfield
        bool                        mbUseIt : 1;

    protected:
    public:
        BColorModifier_RGBLuminanceContrast(double fRed, double fGreen, double fBlue, double fLuminance, double fContrast);

        virtual ~BColorModifier_RGBLuminanceContrast();

        // data access
        double getRed() const { return mfRed; }
        double getGreen() const { return mfGreen; }
        double getBlue() const { return mfBlue; }
        double getLuminance() const { return mfLuminance; }
        double getContrast() const { return mfContrast; }

        // compare operator
        virtual bool operator==(const BColorModifier& rCompare) const SAL_OVERRIDE;

        // compute modified color
        virtual ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const SAL_OVERRIDE;
    };
} // end of namespace basegfx



namespace basegfx
{
    /// typedef to allow working with shared instances of BColorModifier
    /// for the whole mechanism
    typedef std::shared_ptr< BColorModifier > BColorModifierSharedPtr;

    /** Class to hold a stack of BColorModifierSharedPtrs and to get the modified color with
        applying all existing entry changes as defined in the stack. Instances of BColorModifier
        can be pushed and popped to change the stack.

        All references to BColorModifier members use shared pointers, thus instances of
        BColorModifierStack can be copied by the default mechanisms if needed.
    */
    class BASEGFX_DLLPUBLIC BColorModifierStack
    {
    protected:
        ::std::vector< BColorModifierSharedPtr >        maBColorModifiers;

    public:
        sal_uInt32 count() const
        {
            return maBColorModifiers.size();
        }

        const BColorModifierSharedPtr& getBColorModifier(sal_uInt32 nIndex) const
        {
            OSL_ENSURE(nIndex < count(), "BColorModifierStack: Access out of range (!)");
            return maBColorModifiers[nIndex];
        }

        // get the color in it's modified form by applying all existing BColorModifiers,
        // from back to front (the newest first)
        ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& rSource) const;

        void push(const BColorModifierSharedPtr& rNew)
        {
            maBColorModifiers.push_back(rNew);
        }

        void pop()
        {
            maBColorModifiers.pop_back();
        }
    };
} // end of namespace basegfx



#endif // INCLUDED_BASEGFX_COLOR_BCOLORMODIFIER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
