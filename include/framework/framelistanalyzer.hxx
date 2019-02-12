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

#ifndef INCLUDED_FRAMEWORK_FRAMELISTANALYZER_HXX
#define INCLUDED_FRAMEWORK_FRAMELISTANALYZER_HXX

#include <framework/fwedllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <vector>

namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::frame { class XFramesSupplier; }

/** These enums can be combined as flags to enable/disable
    special search algorithm during analyze phase.
    see impl_analyze() for further information.
    But note: To be usable as flags, these enums
    must be values of range [2^n]! */
enum class FrameAnalyzerFlags
{
    Model             =     1,
    Help              =     2,
    BackingComponent  =     4,
    Hidden            =     8,
    All               =    15,
    Zombie            = 32768 // use it for special test scenarios only!!!
};
namespace o3tl {
    template<> struct typed_flags<FrameAnalyzerFlags> : is_typed_flags<FrameAnalyzerFlags, 32768+15> {};
}


namespace framework{

/** analyze and split the current available frame list of a given frames supplier
    into different parts.

    These analyzed information can be used e.g. to decide if it's necessary
    to switch into the backing mode, close the current active frame only or
    exit the whole application explicitly or implicitly.
 */
class FWE_DLLPUBLIC FrameListAnalyzer final
{

    // types

    public:

    // member

    public:

        /** provides access to the frame container, which should be analyzed. */
        const css::uno::Reference< css::frame::XFramesSupplier >& m_xSupplier;

        /** hold the reference frame, which is used e.g. to detect other frames with the same model. */
        const css::uno::Reference< css::frame::XFrame >& m_xReferenceFrame;

        /** enable/disable some special analyzing steps.
            see impl_analyze() for further information. */
        FrameAnalyzerFlags m_eDetectMode;

        /** contains all frames, which uses the same model like the reference frame.
            Will be filled only if m_eDetectMode has set the flag FrameAnalyzerFlags::Model.
            The reference frame is never part of this list! */
        std::vector< css::uno::Reference< css::frame::XFrame > > m_lModelFrames;

        /** contains all frames, which does not contain the same model like the reference frame.
            Filling of it can't be suppressed by m_eDetectMode.
            The reference frame is never part of this list!
            All frames inside this list are visible ones. */
        std::vector< css::uno::Reference< css::frame::XFrame > > m_lOtherVisibleFrames;

        /** contains all frames, which does not contain the same model like the reference frame.
            Filling of it can't be suppressed by m_eDetectMode.
            The reference frame is never part of this list!
            All frames inside this list are hidden ones. */
        std::vector< css::uno::Reference< css::frame::XFrame > > m_lOtherHiddenFrames;

        /** points to the help frame.
            Will be set only, if any other frame (means different from the reference frame)
            contains the help component. If the reference frame itself includes the help module
            it's not set ... but another member m_bIsHelp is used to safe this information.
            See following example code:

            <listing>
            if (m_xReferenceFrame == help)
            {
                m_xHelp   = NULL;
                m_bIsHelp = sal_True;
            }
            else
            if (xOtherFrame == help)
            {
                m_xHelp   = xOtherFrame;
                m_bIsHelp = sal_False;
            }
            </listing>

            Analyzing of the help frame ignores the visible state of any frame.
            But note: a hidden help frame indicates a wrong state!
         */
        css::uno::Reference< css::frame::XFrame > m_xHelp;

        /** points to the frame, which contains the backing component.
            Will be set only, if any other frame (means different from the reference frame)
            contains the backing component. If the reference frame itself includes the
            backing component it's not set ... but another member m_bIsBackingComponent
            will used to safe this information.
            See following example code:

            <listing>
            if (m_xReferenceFrame == backing)
            {
                m_xBackingComponent   = NULL;
                m_bIsBackingComponent = sal_True;
            }
            else
            if (xOtherFrame == backing)
            {
                m_xBackingComponent   = xOtherFrame;
                m_bIsBackingComponent = sal_False      ;
            }
            </listing>

            Analyzing of the help frame ignores the visible state of any frame.
            But note: a hidden backing mode frame indicates a wrong state!
         */
        css::uno::Reference< css::frame::XFrame > m_xBackingComponent;

        /** is set to true only, if the reference frame is a hidden one.
            This value is undefined if m_eDetectMode doesn't have set the flag FrameAnalyzerFlags::Hidden! */
        bool m_bReferenceIsHidden;

        /** is set to true only, if the reference frame contains the help component.
            In this case the member m_xHelp is set to NULL every time.
            This value is undefined if m_eDetectMode doesn't have set the flag FrameAnalyzerFlags::Help! */
        bool m_bReferenceIsHelp;

        /** is set to true only, if the reference frame contains the backing component.
            In this case the member m_xBackingComponent is set to NULL every time.
            This value is undefined if m_eDetectMode doesn't have set the flag FrameAnalyzerFlags::BackingComponent! */
        bool m_bReferenceIsBacking;


    // interface

    public:

        /** starts analyze phase and fill all members with valid information.

            @param xSupplier
                    Must be a valid reference to a frames supplier, which provides
                    access to the frame list for analyzing.

            @param xReferenceFrame
                    This frame must(!) exist inside the analyzed frame list and
                    is used for some comparing functions. Further some member states
                    depends from the current state of this frame.

            @param eDetectMode
                    It represent a flag field, which can enable/disable special
                    analyze steps. Note: Some member values will be undefined, if
                    an analyze step will be disabled.
         */
                 FrameListAnalyzer( const css::uno::Reference< css::frame::XFramesSupplier >& xSupplier       ,
                                    const css::uno::Reference< css::frame::XFrame >&          xReferenceFrame ,
                                          FrameAnalyzerFlags                                  eDetectMode     );
                 ~FrameListAnalyzer();


    // helper

    private:

            void impl_analyze();


}; // class FrameListAnalyzer

} // namespace framework

#endif // INCLUDED_FRAMEWORK_FRAMELISTANALYZER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
