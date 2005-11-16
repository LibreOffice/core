/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tracer.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:03 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef SW_TRACER_HXX
#define SW_TRACER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>      //rtl::OUString
#endif

class MSFilterTracer;
class SfxMedium;

namespace sw
{
    namespace log
    {

        enum Problem
        {
            ePrinterMetrics = 1,
            eExtraLeading,
            eTabStopDistance,
            eDontUseHTMLAutoSpacing,
            eAutoWidthFrame,
            eRowCanSplit,
            eSpacingBetweenCells,
            eTabInNumbering,
            eNegativeVertPlacement,
            eAutoColorBg,
            eTooWideAsChar,
            eAnimatedText,
            eDontAddSpaceForEqualStyles,
            eBorderDistOutside,
            eContainsVisualBasic,
            eContainsWordBasic
        };

        enum Environment
        {
            eDocumentProperties,
            eMacros,
            eMainText,
            eSubDoc,
            eTable
        };

        class Tracer
        {
        private:
            MSFilterTracer *mpTrace;
            rtl::OUString GetContext(Environment eContext) const;
            rtl::OUString GetDetails(Environment eContext) const;
        public:
            Tracer(const SfxMedium &rMedium);
            MSFilterTracer *GetTrace() const { return mpTrace; }
            void EnterEnvironment(Environment eContext);
            void EnterEnvironment(Environment eContext,
                const rtl::OUString &rDetails);
            void Log(Problem eProblem);
            void LeaveEnvironment(Environment eContext);
            ~Tracer();
        };
    }
}
#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
