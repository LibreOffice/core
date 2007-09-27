/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tracer.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 10:00:35 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>         //INetURLObject
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>         //SfxMedium
#endif
#ifndef _MS_FILTERTRACER_HXX
#include <svx/msfiltertracer.hxx>   //MSFilterTracer
#endif
#ifndef SW_TRACER_HXX
#include "tracer.hxx"               //sw::log::Tracer
#endif

using rtl::OUString;
#define CAU(X) RTL_CONSTASCII_USTRINGPARAM(X)
#define C2O(X) OUString(CAU(X))
#define COMMENT(X) OUString()

namespace sw
{
    namespace log
    {
        Tracer::Tracer(const SfxMedium &rMed)
            : mpTrace(0)
        {
            using namespace ::com::sun::star::uno;
            using namespace ::com::sun::star::beans;
            Sequence<PropertyValue> aConfig(1);
            PropertyValue aPropValue;
            aPropValue.Value <<=
                OUString(rMed.GetURLObject().GetMainURL(
                    INetURLObject::NO_DECODE));
            aPropValue.Name = C2O("DocumentURL");
            aConfig[0] = aPropValue;
            OUString aTraceConfigPath(CAU("Office.Tracing/Import/Word"));
            mpTrace = new MSFilterTracer(aTraceConfigPath, &aConfig);
            if (mpTrace)
                mpTrace->StartTracing();
        }

        Tracer::~Tracer()
        {
            if (mpTrace)
            {
                mpTrace->EndTracing();
                delete mpTrace;
            }
        }

        void Tracer::Log(Problem eProblem)
        {
            if (!mpTrace)
                return;

            OUString sID(CAU("sw"));
            sID += rtl::OUString::valueOf(static_cast<sal_Int32>(eProblem));
            switch (eProblem)
            {
                case ePrinterMetrics:
                    mpTrace->Trace(sID, COMMENT("PrinterMetrics"));
                    break;
                case eExtraLeading:
                    mpTrace->Trace(sID, COMMENT("Extra Leading"));
                    break;
                case eTabStopDistance:
                    mpTrace->Trace(sID, COMMENT("Minimum Tab Distance"));
                    break;
                case eDontUseHTMLAutoSpacing:
                    mpTrace->Trace(sID, COMMENT("HTML AutoSpacing"));
                    break;
                case eAutoWidthFrame:
                    mpTrace->Trace(sID, COMMENT("AutoWidth"));
                    break;
                case eRowCanSplit:
                    mpTrace->Trace(sID, COMMENT("Splitable Row"));
                    break;
                case eSpacingBetweenCells:
                    mpTrace->Trace(sID, COMMENT("Spacing Between Cells"));
                    break;
                case eTabInNumbering:
                    mpTrace->Trace(sID, COMMENT("Tab In Numbering"));
                    break;
                case eNegativeVertPlacement:
                    mpTrace->Trace(sID,
                        COMMENT("Negative Vertical Placement"));
                    break;
                case eAutoColorBg:
                    mpTrace->Trace(sID,
                        COMMENT("Bad Background for Autocolour"));
                    break;
                case eTooWideAsChar:
                    mpTrace->Trace(sID,
                        COMMENT("Inline wider than TextArea"));
                    break;
                case eAnimatedText:
                    mpTrace->Trace(sID,
                        COMMENT("Animated Text"));
                    break;
                case eDontAddSpaceForEqualStyles:
                    mpTrace->Trace(sID,
                        COMMENT("Don't Add Space between Equal Style"));
                    break;
                case eBorderDistOutside:
                    mpTrace->Trace(sID,
                        COMMENT("Word draws the border outside"));
                    break;
                case eContainsVisualBasic:
                    mpTrace->Trace(sID, COMMENT("Contains VBA"));
                    break;
                case eContainsWordBasic:
                    mpTrace->Trace(sID, COMMENT("Contains Word Basic"));
                    break;
                default:
                    mpTrace->Trace(sID, COMMENT("UNKNOWN"));
                    break;
            }
        }

        rtl::OUString Tracer::GetContext(Environment eContext) const
        {
            rtl::OUString sContext;
            switch (eContext)
            {
                case eMacros:
                case eDocumentProperties:
                    sContext = C2O("Global");
                    break;
                case eMainText:
                    sContext = C2O("Text");
                    break;
                case eSubDoc:
                    sContext = C2O("Text");
                    break;
                case eTable:
                    sContext = C2O("Table");
                    break;
                default:
                    sContext = C2O("UNKNOWN");
                    break;
            }
            return sContext;
        }

        rtl::OUString Tracer::GetDetails(Environment eContext) const
        {
            rtl::OUString sDetails;
            switch (eContext)
            {
                case eDocumentProperties:
                    sDetails = C2O("Document Properties");
                    break;
                case eMacros:
                    sDetails = C2O("Macros");
                    break;
                case eMainText:
                    sDetails = C2O("MainDocument");
                    break;
                case eSubDoc:
                    sDetails = C2O("Unknown Subdocument");
                    break;
                default:
                    sDetails = C2O("UNKNOWN");
                    break;
            }
            return sDetails;
        }

        void Tracer::EnterEnvironment(Environment eContext)
        {
            mpTrace->AddAttribute(GetContext(eContext), GetDetails(eContext));
        }

        void Tracer::EnterEnvironment(Environment eContext,
            const rtl::OUString &rDetails)
        {
            mpTrace->AddAttribute(GetContext(eContext), rDetails);
        }

        void Tracer::LeaveEnvironment(Environment eContext)
        {
            mpTrace->RemoveAttribute(GetContext(eContext));
        }
    }
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
