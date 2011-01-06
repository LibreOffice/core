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

#ifndef __FRAMEWORK_CLASSES_FRAMELISTANALYZER_HXX_
#define __FRAMEWORK_CLASSES_FRAMELISTANALYZER_HXX_

//_______________________________________________
//  interface includes
#include <com/sun/star/frame/XFrame.hpp>

//_______________________________________________
//  other includes
#include <framework/fwedllapi.h>

//_______________________________________________
//  namespace

namespace framework{

//_______________________________________________
//  exported const

//_______________________________________________
//  exported definitions

/** analyze and split the current available frame list of a given frames supplier
    into different parts.

    These analyzed informations can be used e.g. to decide if it's neccessary
    to switch into the backing mode, close the current active frame only or
    exit the whole application explicitly or implicitly.
 */
class FWE_DLLPUBLIC FrameListAnalyzer
{
    //_______________________________________
    // types

    public:

        /** These enums can be combined as flags to enable/disable
            special search algorithm during analyze phase.
            see impl_analyze() for further informations.
            But note: To be useable as flags, these enums
            must be values of range [2^n]! */
        enum EDetect
        {
            E_MODEL             =     1,
            E_HELP              =     2,
            E_BACKINGCOMPONENT  =     4,
            E_HIDDEN            =     8,
            E_ALL               =    15,
            E_ZOMBIE            = 32768 // use it for special test scenarios only!!!
        };

    //_______________________________________
    // member

    public:

        /** provides access to the frame container, which should be analyzed. */
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFramesSupplier >& m_xSupplier;

        /** hold the reference frame, which is used e.g. to detect other frames with the same model. */
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& m_xReferenceFrame;

        /** enable/disable some special analyzing steps.
            see impl_analyze() for further informations. */
        sal_uInt32 m_eDetectMode;

        /** contains all frames, which uses the same model like the reference frame.
            Will be filled only if m_eDetectMode has set the flag E_MODEL.
            The reference frame is never part of this list! */
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > > m_lModelFrames;

        /** contains all frames, which does not contain the same model like the reference frame.
            Filling of it can't be supressed by m_eDetectMode.
            The reference frame is never part of this list!
            All frames inside this list are visible ones. */
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > > m_lOtherVisibleFrames;

        /** contains all frames, which does not contain the same model like the reference frame.
            Filling of it can't be supressed by m_eDetectMode.
            The reference frame is never part of this list!
            All frames inside this list are hidden ones. */
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > > m_lOtherHiddenFrames;

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
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xHelp;

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
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xBackingComponent;

        /** is set to true only, if the reference frame is a hidden one.
            This value is undefined if m_eDetectMode doesn't have set the flag E_HIDDEN! */
        sal_Bool m_bReferenceIsHidden;

        /** is set to true only, if the reference frame contains the help component.
            In this case the member m_xHelp is set to NULL everytimes.
            This value is undefined if m_eDetectMode doesn't have set the flag E_HELP! */
        sal_Bool m_bReferenceIsHelp;

        /** is set to true only, if the reference frame contains the backing component.
            In this case the member m_xBackingComponent is set to NULL everytimes.
            This value is undefined if m_eDetectMode doesn't have set the flag E_BACKINGCOMPONENT! */
        sal_Bool m_bReferenceIsBacking;

    //_______________________________________
    // interface

    public:

        /** starts analyze phase and fille all members with valid informations.

            @param xSupplier
                    Must be a valid reference to a frames supplier, which provies
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
                 FrameListAnalyzer( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFramesSupplier >& xSupplier       ,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >&          xReferenceFrame ,
                                          sal_uInt32                                          eDetectMode     );
        virtual ~FrameListAnalyzer();

    //_______________________________________
    // helper

    private:

            void impl_analyze();


}; // class FrameListAnalyzer

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_FRAMELISTANALYZER_HXX_
